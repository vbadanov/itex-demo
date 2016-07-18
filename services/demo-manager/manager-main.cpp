
#include <iostream>
#include <atomic>

#include <boost/asio/signal_set.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>


#define RAPIDJSON_ASSERT(x) if(!(x)) throw std::runtime_error("JSON assertion failed: " #x);
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <http/server.hpp>
#include <http/router.hpp>
#include <http-server-handler.hpp>
#include <quant/utils.hpp>

namespace itex
{

namespace demo_manager
{

//===============================================================================
class RequestHandlerFactory : public http::IRequestHandlerFactory
{
    public:
		RequestHandlerFactory(HTTPServerHandler::Configuration cfg)
    		: cfg_(cfg), ready_(true)
    	{
			//pass
    	}

        virtual http::IRequestHandler* create_handler(boost::asio::yield_context& yield)
        {
        	if(ready_.load(std::memory_order_acquire))
        	{
        		return new itex::demo_manager::HTTPServerHandler(deps_, cfg_);
        	}
        	else
        	{
        		return nullptr;
        	}
        }

        virtual ~RequestHandlerFactory()
        {
            //pass
        }

        HTTPServerHandler::Dependencies& deps()
        {
        	return deps_;
        }

        void set_ready(bool ready)
        {
        	ready_.store(ready, std::memory_order_release);
        }

    private:
        HTTPServerHandler::Dependencies deps_;
        HTTPServerHandler::Configuration cfg_;
        std::atomic<bool> ready_;
};

}
}

//===============================================================================
using namespace itex::demo_manager;

//===============================================================================
int main(int argc, char **argv)
{
	try
	{
		auto stage_dir_path = boost::filesystem::system_complete( boost::filesystem::path( argv[0] ) ).normalize().parent_path().parent_path();
		auto config_file_path = stage_dir_path;
		config_file_path += "/etc/demo-manager.cfg";

		boost::property_tree::ptree pt;
		boost::property_tree::ini_parser::read_ini(config_file_path.normalize().native(), pt);

		HTTPServerHandler::Configuration http_server_handler_cfg;
		http_server_handler_cfg.persist_workset_to_database = pt.get<bool>("Workset.persist_to_database");

		RequestHandlerFactory rh_factory(http_server_handler_cfg);

		http::ServerSettings http_server_settings;
		http_server_settings.endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), pt.get<int>("HTTPServer.port"));
		http_server_settings.num_threads = pt.get<int>("HTTPServer.num_threads");
		http_server_settings.sheduled_connection_delete_period_msec = pt.get<int>("HTTPServer.sheduled_connection_delete_period_msec");
		http_server_settings.connection_settings.buffer_size_bytes = pt.get<int>("HTTPServer.connection_buffer_size_bytes");
		http_server_settings.connection_settings.timeout_msec = pt.get<int>("HTTPServer.connection_timeout_msec");
		http_server_settings.connection_settings.request_handler_factory = &rh_factory;

		http::Server server(http_server_settings);

		sqldb::ConnectionPoolSettings db_conn_pool_settings;
		db_conn_pool_settings.pool_shrink_period_msec = pt.get<int>("Database.pool_shrink_period_msec");
		db_conn_pool_settings.max_connections_limit = pt.get<int>("Database.max_connections_limit");
		db_conn_pool_settings.connection_settings.connection_timeout_msec = pt.get<int>("Database.connection_timeout_msec");
		db_conn_pool_settings.connection_settings.command_timeout_msec = pt.get<int>("Database.command_timeout_msec");

		std::string db_host = pt.get<std::string>("Database.host");
		std::string db_port = pt.get<std::string>("Database.port");
		std::string db_database_name = pt.get<std::string>("Database.database_name");
		std::string db_user = pt.get<std::string>("Database.user");
		//std::string db_password = pt.get<std::string>("Database.password");
		sqldb::Database db(server.get_io_service(), std::string("postgresql://") + db_user + "@" + db_host + ":" + db_port + "/" + db_database_name, db_conn_pool_settings);
		rh_factory.deps().db = &db;

		OrderbookQueueWriter oqw(server.get_io_service(), db);
		oqw.start();
		rh_factory.deps().oqw = &oqw;

		itex::quant::OrderbookWorkset ws(pt.get<size_t>("Workset.size"));
		rh_factory.set_ready(false);
		boost::asio::spawn(server.get_io_service(), [&](boost::asio::yield_context yield)
		{
			itex::quant::Utils::preload_workset_async(ws, 0, 0.0, server.get_io_service(), db, true, yield);
			rh_factory.set_ready(true);
		});
		rh_factory.deps().ws = &ws;

		boost::asio::spawn(server.get_io_service(), [&](boost::asio::yield_context yield)
		{
			boost::asio::signal_set signals(server.get_io_service(), SIGINT, SIGTERM);
			std::cout << "Created signal_set" << std::endl;

			signals.async_wait(yield);
			std::cout << "Received signal..." << std::endl;

			server.stop();
			oqw.stop();

			std::cout << "Service is shutting down... please wait..." << std::endl;
		});

		std::cout << "Starting server" << std::endl;
		server.start();
		std::cout << "Server stopped" << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: [" << e.what() << "] Exiting." << std::endl;
	}

    return 0;
}



