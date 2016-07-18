
#include <cmath>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/p_square_cumul_dist.hpp>
#include <boost/circular_buffer.hpp>

#include <mgl2/mgl.h>
#include <lodepng.h>

#include <aux/utils.hpp>
#include <quant/utils.hpp>


namespace itex
{

namespace quant

{
using AccumulatorPDF = boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::density>>;
using AccumulatorCDF = boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::p_square_cumulative_distribution>>;
using HistogramType = boost::iterator_range<std::vector<std::pair<double, double>>::iterator>;

double Utils::preload_workset_async(OrderbookWorkset& ows, size_t offset, double after_timestamp, boost::asio::io_service& is, sqldb::Database& db, bool draw_histogram, boost::asio::yield_context yield)
{
	double start_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	size_t capacity = ows.capacity();
	OrderbookWorkset::Publisher* publisher = ows.get_publisher();
	sqldb::Transaction tr(db);
	sqldb::Results orderbook_results = tr.async_exec(R"(
		            SELECT * FROM
		    		(
		                SELECT ol.id, s.alias_id, ol.timestamp FROM orderbook_log AS ol
		                INNER JOIN securities AS s ON ol.security_id = s.id
		                WHERE ol.timestamp > $1
		                ORDER BY id DESC LIMIT $1 OFFSET $2
		            ) AS t
		            WHERE timestamp::numeric > $3::numeric
		            ORDER BY id ASC;)", sqldb::make_params(capacity, offset, after_timestamp), yield);

	std::vector<double> avg_volumes;
	std::pair<double, double> max_diff = std::make_pair(0.0, 0.0);
	double avg_diff = 0.0;
	double prev_bid = -1.0;
	double prev_ask = -1.0;

	//create an accumulator
	AccumulatorPDF acc( boost::accumulators::tag::density::num_bins = 20, boost::accumulators::tag::density::cache_size = 2);
	acc(-10);
	acc(10);
	const size_t MAX_DELAY = 100;
	boost::circular_buffer<double> delays_bids(MAX_DELAY, 0.0);
	boost::circular_buffer<double> delays_asks(MAX_DELAY, 0.0);


	size_t num_rows = orderbook_results[0].num_rows();
	size_t current_row = 0;

	if(num_rows == 0)
	{
		return after_timestamp;
	}

	std::cout << "PRELOADING WORKSET SIZE: " << num_rows << std::endl;
	for(auto orderbook_log_record : orderbook_results[0])
	{
		unsigned long long id = 0;
		unsigned long long security_id = 0;
		double timestamp = 0.0;
		orderbook_log_record.copy_to(id, security_id, timestamp);

		sqldb::Results orderbook_details_results = tr.async_exec("SELECT price, volume FROM orderbook_log_details WHERE orderbook_log_id = $1 ORDER BY price ASC;", sqldb::make_params(id), yield);

		size_t orderbook_size = orderbook_details_results[0].num_rows();
		std::vector<std::pair<double, double>> bids;
		std::vector<std::pair<double, double>> asks;
		bids.reserve(orderbook_size / 2);
		asks.reserve(orderbook_size / 2);

		if(avg_volumes.size() < orderbook_size / 2)
		{
			avg_volumes.resize(orderbook_size / 2, 0.0);
		}

		for(auto orderbook_detail_record : orderbook_details_results[0])
		{
			double price = 0.0;
			double volume = 0.0;
			orderbook_detail_record.copy_to(price, volume);

			if(volume > 0)
			{
				bids.emplace_back(std::make_pair(price, volume));
			}
			else if(volume < 0)
			{
				asks.emplace_back(std::make_pair(price, volume));
			}
		}

		std::sort(bids.begin(), bids.end(), [](std::pair<double, double> a, std::pair<double, double> b)
		{
			return a.first > b.first;
		});

		std::sort(asks.begin(), asks.end(), [](std::pair<double, double> a, std::pair<double, double> b)
		{
			return a.first < b.first;
		});

		itex::quant::OrderbookEntry workset_entry(orderbook_size / 2);
		workset_entry.security_id() = security_id;
		workset_entry.timestamp() = timestamp;
		size_t bids_size = bids.size();
		size_t asks_size = asks.size();

		for(size_t i = 0; i < bids_size; ++i)
		{
			workset_entry.bids()[i*2] = bids[i].first;
			workset_entry.bids()[i*2 + 1] = bids[i].second;
			avg_volumes[i] += abs(bids[i].second);
		}

		for(size_t i = 0; i < asks_size; ++i)
		{
			workset_entry.asks()[i*2] = asks[i].first;
			workset_entry.asks()[i*2 + 1] = asks[i].second;
			avg_volumes[i] += abs(asks[i].second);
		}

		publisher->push(std::move(workset_entry));


		for(size_t i = 0; i < delays_bids.size(); ++i)
		{
			double val = workset_entry.bids()[0] - delays_bids[i];
			val = val < 0 ? std::max(-10.0, val) : std::min(10.0, val);
			acc(val);
			val = workset_entry.asks()[0] - delays_asks[i];
			val = val < 0 ? std::max(-10.0, val) : std::min(10.0, val);
			acc(val);
		}
		delays_bids.push_back(workset_entry.bids()[0]);
		delays_asks.push_back(workset_entry.asks()[0]);

		// calc max diff
		if(prev_bid < 0.0 && prev_ask < 0.0)
		{
			prev_bid = workset_entry.bids()[0];
			prev_ask = workset_entry.asks()[0];
		}
		else
		{
			double diff = (workset_entry.bids()[0] - prev_bid);
			avg_diff += diff;
			diff = std::abs(diff);
			if(max_diff.first < diff)
			{
				max_diff.first = diff;
			}
			prev_bid = workset_entry.bids()[0];

			diff = (workset_entry.asks()[0] - prev_ask);
			avg_diff += diff;
			diff = std::abs(diff);
			if(max_diff.second < diff)
			{
				max_diff.second = diff;
			}
			prev_ask = workset_entry.asks()[0];
		}

		++current_row;
	}
	avg_diff /= (2*num_rows);
	publisher->release();

	std::cout << "AVERAGE VOLUMES: ";
	size_t avg_volumes_size = avg_volumes.size();
	avg_volumes_size = avg_volumes_size > 10 ? 10 : avg_volumes_size;
	double coeff = static_cast<double>(num_rows*2);
	for(size_t i = 0; i < avg_volumes_size; ++i)
	{
		avg_volumes[i] = avg_volumes[i] / coeff;
		std::cout << avg_volumes[i] << " ";
	}
	std::cout << std::endl;

	std::cout << "MAX DIFF: bid: " << max_diff.first << " ask: " << max_diff.second << std::endl;
	std::cout << "AVG DIFF: " << avg_diff << std::endl;

	// draw histogram
	if(draw_histogram)
	{
		std::cout << "HISTOGRAM: " << std::endl;
		HistogramType hist = boost::accumulators::density(acc);
		double sum = 0.0;
		for( size_t i = 0; i < hist.size(); ++i)
		{
			std::cout << "Diff lower bound: ";
			std::cout.width(5);
			std::cout << hist[i].first << ", Value: ";
			std::cout.width(15);
			std::cout << hist[i].second << " |";
			size_t length = (size_t)(std::sqrt(hist[i].second) * 100);
			if(length > 0)
			{
				std::cout << std::string(length, 'o');
			}
			std::cout << std::endl;

			sum += (hist[i].first * hist[i].second);
		}
		std::cout << "SUM: " << sum << std::endl;

	}


	double last_timestamp = num_rows > 0 ? orderbook_results[0].get_value<double>(num_rows - 1, "timestamp") : after_timestamp;

	double end_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "TOTAL WORKSET SIZE: " << ows.size() << " ELAPLSED TIME: " << end_time - start_time << std::endl << std::endl;
	return last_timestamp;
}


//===============================================================================
// TODO: draw_workset_png() needs optimizations!
std::string Utils::draw_workset_png(OrderbookWorkset& ows)
{
	if (ows.empty())
	{
		return std::string();
	}

	double start_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	double end_time_temp = 0.0;

	OrderbookWorkset::Subscriber* subscriber = ows.get_subscriber();
	size_t x_size = subscriber->num_available();

	struct SecurityWorksetData
	{
		double* x;
		double* y;
		double* z;
		size_t y_size;
		double price_origin;
		double last_non_zero_price;
	};

	std::map<unsigned long long, SecurityWorksetData> sec_map;
	OrderbookWorkset workset(x_size);
	OrderbookWorkset::Publisher* publisher = workset.get_publisher();

	while(subscriber->num_available() > 0)
	{
		itex::quant::OrderbookEntry e;
		while(!subscriber->try_pop(e)) {};
		publisher->push(e);

		unsigned long long sec_id = static_cast<unsigned long long>(e.security_id());
		if(sec_map.find(sec_id) == sec_map.end())
		{
			SecurityWorksetData swd;
			swd.y_size = e.orderbook_depth() * 2;
			swd.price_origin =  (e.bids()[0] + e.asks()[0]) / 2.0;
			swd.last_non_zero_price = swd.price_origin;
			swd.x = aux::allocate_buffer<double>(x_size * swd.y_size * 1);
			swd.y = aux::allocate_buffer<double>(x_size * swd.y_size * 1);
			swd.z = aux::allocate_buffer<double>(x_size * swd.y_size * 1);
			std::fill(swd.x, swd.x + (x_size * swd.y_size * 1), 1000000.0);
			std::fill(swd.y, swd.y + (x_size * swd.y_size * 1), 1000000.0);
			std::fill(swd.z, swd.z + (x_size * swd.y_size * 1), 1000000.0);
			sec_map.emplace(sec_id, swd);
		}
	}

	publisher->release();
	subscriber->release();
	subscriber = workset.get_subscriber();

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 1: " << end_time_temp - start_time << std::endl;

	for(size_t i = 0; i < x_size; ++i)
	{
		itex::quant::OrderbookEntry e;
		while(!subscriber->try_pop(e)) {};

		for(auto& elt : sec_map)
		{
			if(elt.first == static_cast<unsigned long long>(e.security_id()))
			{
				continue;
			}

			SecurityWorksetData& swd = elt.second;
			for(size_t j = 0; j < swd.y_size; ++j)
			{
				swd.x[i + x_size * j] = (double)i;
				if(i > 0)
				{
					swd.y[i + x_size * j] = swd.y[(i-1) + x_size * j];
					swd.z[i + x_size * j] = swd.z[(i-1) + x_size * j];
				}
				else
				{
					swd.y[i + x_size * j] = 0.0;
					swd.z[i + x_size * j] = 0.0;
				}
			}
		}

		SecurityWorksetData& swd = sec_map[static_cast<unsigned long long>(e.security_id())];
		for(size_t j = 0; j < swd.y_size; ++j)
		{
			double* data = e.data_ptr();
			double price = data[2 + (j < 50 ? 49 - j : j) * 2];
			double volume = data[2 + (j < 50 ? 49 - j : j) * 2 + 1];


			if(abs(price) < 0.001)
			{
				price = swd.last_non_zero_price;
			}
			else
			{
				swd.last_non_zero_price = price;
			}

			volume = abs(volume);

			const double VOLUME_LIMIT = 500;
			if(volume >= 0)
			{
				volume = ((abs(volume) < VOLUME_LIMIT) ? volume : VOLUME_LIMIT);
			}
			else
			{
				volume = ((abs(volume) < VOLUME_LIMIT) ? volume : -VOLUME_LIMIT);
			}

			swd.x[i + x_size * j] = (double)i;
			swd.y[i + x_size * j] = price - swd.price_origin;
			swd.z[i + x_size * j] = (volume >= 0.001) ? log2(volume) : 0.0;
		}
	}

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 2: " << end_time_temp - start_time << std::endl;

	int width = 2000;
	int height = 1000;
	mglGraph gr(0, width, height);
	gr.DefaultPlotParam();
	gr.SetQuality(MGL_DRAW_FAST | MGL_DRAW_LMEM | MGL_DRAW_DOTS);

	size_t num_securities = sec_map.size();
	size_t sec_idx = 0;
	for(auto& elt : sec_map)
	{
		SecurityWorksetData& swd = elt.second;

		mglData x;
		mglData y;
		mglData z;
		x.Link(swd.x, x_size, swd.y_size, 1);
		y.Link(swd.y, x_size, swd.y_size, 1);
		z.Link(swd.z, x_size, swd.y_size, 1);

		auto x_min_max = std::make_pair(x.Minimal(), x.Maximal());
		auto y_min_max = std::make_pair(y.Minimal(), y.Maximal());
		auto z_min_max = std::make_pair(z.Minimal(), z.Maximal());

		std::cout<< "x min: " << x_min_max.first << " x max: " << x_min_max.second << std::endl;
		std::cout<< "y min: " << y_min_max.first << " y max: " << y_min_max.second << std::endl;
		std::cout<< "z min: " << z_min_max.first << " z max: " << z_min_max.second << std::endl;

		gr.SubPlot(1, num_securities, sec_idx);

		gr.SetRanges(x_min_max.first, x_min_max.second, y_min_max.first, y_min_max.second, z_min_max.first, z_min_max.second);
		gr.SetOrigin(0.0, 0.0, 0.0);
		gr.Title((std::string("security id: ") + std::to_string(elt.first) + " price origin: " + std::to_string(elt.second.price_origin)).c_str());
		gr.Dens(x, y, z); //, "", ".");

		gr.Box();
		gr.Axis();
		gr.Colorbar("_");

		++sec_idx;
	}

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 3: " << end_time_temp - start_time << std::endl;

	const unsigned char* image = gr.GetRGBA();
	std::cout << "WARNINGS: [" << gr.Message() << "] [" << gr.GlobalWarn() << "]" << std::endl;

	std::vector<unsigned char> png;
	lodepng::encode(png, image, gr.GetWidth(), gr.GetHeight());

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 4: " << end_time_temp - start_time << std::endl;

	size_t png_len = png.size();
	std::string png_str(png_len, 0);
	for(size_t i = 0; i < png_len; ++i)
	{
		png_str[i] = png[i];
	}

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 5: " << end_time_temp - start_time << std::endl;


	for(auto& elt : sec_map)
	{
		SecurityWorksetData& swd = elt.second;
		free(swd.x);
		free(swd.y);
		free(swd.z);
	}

	end_time_temp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG DRAW ELAPSED TIME STEP 6: " << end_time_temp - start_time << std::endl;

	return png_str;
}


}
}

