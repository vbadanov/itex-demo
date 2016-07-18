
#ifndef HTTP_SERVER_HANDLER_HPP_
#define HTTP_SERVER_HANDLER_HPP_

#include <string>

#include <http/server.hpp>
#include <http/router.hpp>

#include <sqldb/database.hpp>
#include <sqldb/transaction.hpp>

#include <orderbook-queue-writer.hpp>
#include <quant/workset.hpp>
#include <quant/orderbook-entry.hpp>

namespace itex
{

namespace demo_manager
{

class HTTPServerHandler : public http::IRequestHandler
{
    public:

		struct Dependencies
		{
			sqldb::Database* db;
			OrderbookQueueWriter* oqw;
			itex::quant::Workset<itex::quant::OrderbookEntry>* ws;
		};

		struct Configuration
		{
			bool persist_workset_to_database;
		};

		HTTPServerHandler(Dependencies& deps, Configuration& cfg);
        virtual void reinit(boost::asio::yield_context& yield);
        virtual ~HTTPServerHandler();
        virtual http::Response process_request(http::Connection& conn, http::Request& req, boost::asio::yield_context& yield);

        http::Response add_orderbook(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield);
        http::Response get_workset_png(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield);
        http::Response get_work_set(http::Router::URLPathParameters& url_parameters, http::Connection& conn, http::Request& req, boost::asio::yield_context& yield);


    private:
        http::Router router_;
        Dependencies& deps_;
        Configuration& cfg_;
        itex::quant::Workset<itex::quant::OrderbookEntry>::Publisher* ws_publisher_;
};


}
}

#endif //HTTP_SERVER_HANDLER_HPP_
