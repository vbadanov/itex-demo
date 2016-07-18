
#include <iostream>
#include <chrono>
#include <thread>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <http-server-handler.hpp>
#include <quant/utils.hpp>


namespace itex
{

namespace demo_manager
{

//===============================================================================
bool is_local_net_address(http::Request& req, http::Response& resp)
{
	std::string subnet_start = req.remote_endpoint().address.substr(0, 4);
	if(subnet_start.substr(0,3) != "10." &&
	   subnet_start != "127." &&
	   subnet_start != "172." &&
	   subnet_start != "192."
	  )
	{
		resp.status_code(http::StatusCode::CODE_403_FORBIDDEN);
		resp.body("Request from " + req.remote_endpoint().address + ":" + std::to_string(req.remote_endpoint().port) + " forbidden");
		return false;
	}
	return true;
}


//===============================================================================
HTTPServerHandler::HTTPServerHandler(Dependencies& deps, Configuration& cfg)
	: deps_(deps), cfg_(cfg)
{
	ws_publisher_ = deps_.ws->get_publisher();
	router_.set("/itex-demo/securities/:id/orderbook", http::Method::POST, std::bind(&HTTPServerHandler::add_orderbook, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	router_.set("/itex-demo/workset.png", http::Method::GET, std::bind(&HTTPServerHandler::get_workset_png, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	router_.set("/itex-demo/workset.png/:limit/:offset", http::Method::GET, std::bind(&HTTPServerHandler::get_workset_png, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

//===============================================================================
void HTTPServerHandler::reinit(boost::asio::yield_context& yield)
{
	//pass
}

//===============================================================================
HTTPServerHandler::~HTTPServerHandler()
{
	ws_publisher_->release();
}

//===============================================================================
http::Response HTTPServerHandler::process_request(http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
	return router_.route_request(conn, req, yield);
}

//===============================================================================
http::Response HTTPServerHandler::add_orderbook(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
	double start_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	http::Response response;

	if(!is_local_net_address(req, response))
	{
		return response;
	}

	//copy orderbook rows from json to internal struct
	OrderbookQueueWriter::QueueEntry entry;
	entry.security_id = std::stoull(url_parameters["id"]);
	entry.timestamp = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	rapidjson::Document d;
	d.Parse(req.body().c_str());
	rapidjson::Value& prices = d["prices"];
	rapidjson::Value& volumes = d["volumes"];

	size_t orderbook_size = prices.Size();
	entry.prices.resize(orderbook_size);
	entry.volumes.resize(orderbook_size);

	std::vector<std::pair<double, double>> bids;
	std::vector<std::pair<double, double>> asks;
	bids.reserve(orderbook_size / 2);
	asks.reserve(orderbook_size / 2);

	for(size_t i = 0; i < orderbook_size; ++i)
	{
		entry.prices[i] = prices[i].GetDouble();
		entry.volumes[i] = volumes[i].GetDouble();
		if(entry.volumes[i] > 0)
		{
			bids.emplace_back(std::make_pair(entry.prices[i], entry.volumes[i]));
		}
		else if(entry.volumes[i] < 0)
		{
			asks.emplace_back(std::make_pair(entry.prices[i], entry.volumes[i]));
		}
	}

	// queue entry for writing to database
	if(cfg_.persist_workset_to_database)
	{
		deps_.oqw->push(entry);
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
	workset_entry.security_id() = static_cast<double>(entry.security_id);
	workset_entry.timestamp() = entry.timestamp;
	size_t bids_size = bids.size();
	size_t asks_size = asks.size();

	for(size_t i = 0; i < bids_size; ++i)
	{
		workset_entry.bids()[i*2] = bids[i].first;
		workset_entry.bids()[i*2 + 1] = bids[i].second;
	}

	for(size_t i = 0; i < asks_size; ++i)
	{
		workset_entry.asks()[i*2] = asks[i].first;
		workset_entry.asks()[i*2 + 1] = asks[i].second;
	}

	//add to workset (circular buffer)
	ws_publisher_->push(std::move(workset_entry));

	//process orderbook
	//return current set of positions


	response.status_code(http::StatusCode::CODE_200_OK);
	response.headers()["Content-Type"] = "text/json";

	double end_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	return response;


}

//===============================================================================
http::Response HTTPServerHandler::get_workset_png(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
	double start_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	http::Response response;
	response.status_code(http::StatusCode::CODE_200_OK);
	response.headers()["Content-Type"] = "image/png";

	if(url_parameters.find("limit") != url_parameters.end() && url_parameters.find("offset") != url_parameters.end())
	{
		// Load workset from history and draw
		itex::quant::Workset<itex::quant::OrderbookEntry> ws(std::stoull(url_parameters["limit"]));
		itex::quant::Utils::preload_workset_async(ws, std::stoull(url_parameters["offset"]), 0.0, conn.get_server().get_io_service(), *deps_.db, false, yield);

		//wait
		response.body(itex::quant::Utils::draw_workset_png(ws));
	}
	else
	{
		// Draw current workset
		response.body(itex::quant::Utils::draw_workset_png(*deps_.ws));
	}

	double end_time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	std::cout << "PNG REQUEST ELAPSED TIME: " << end_time - start_time << std::endl;

	return response;
}

//===============================================================================
http::Response HTTPServerHandler::get_work_set(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield)
{
	http::Response response;
	response.status_code(http::StatusCode::CODE_200_OK);

	return response;
}


}
}

