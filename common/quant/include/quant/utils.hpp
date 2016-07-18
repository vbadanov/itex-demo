
#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>
#include <boost/asio/spawn.hpp>

#include <sqldb/database.hpp>
#include <sqldb/transaction.hpp>
#include <quant/workset.hpp>
#include <quant/orderbook-entry.hpp>

namespace itex
{

namespace quant
{

using OrderbookWorkset = itex::quant::Workset<itex::quant::OrderbookEntry>;

//===============================================================================
class Utils
{
	public:
		static double preload_workset_async(OrderbookWorkset& ows, size_t offset, double after_timestamp, boost::asio::io_service& is, sqldb::Database& db, bool draw_histogram, boost::asio::yield_context yield);
		static std::string draw_workset_png(OrderbookWorkset& ows);
};

}
}

#endif /* UTILS_HPP_ */
