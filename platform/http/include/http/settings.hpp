
#ifndef HTTP_SETTINGS_HPP_
#define HTTP_SETTINGS_HPP_

#include <cstddef>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <http/connection.hpp>
#include <http/request.hpp>
#include <http/response.hpp>

namespace http
{

class Connection;
class Request;
class Response;


//===============================================================================
class IRequestHandler
{
    public:
        virtual Response process_request(Connection& conn, Request& req, boost::asio::yield_context& yield) = 0;
        virtual void reinit(boost::asio::yield_context& yield) = 0; // called when HTTP connection object is reused for new connection
        virtual ~IRequestHandler() {};
};

//===============================================================================
class IRequestHandlerFactory
{
    public:
        virtual IRequestHandler* create_handler(boost::asio::yield_context& yield) = 0;
        virtual ~IRequestHandlerFactory() {};
};

//===============================================================================
struct ConnectionSettings
{
    size_t timeout_msec;       // timeout for HTTP request processing
    size_t buffer_size_bytes;  // receive buffer size
    IRequestHandlerFactory* request_handler_factory;    // HTTP requests handler factory (new handler instance per new connection)
};

//===============================================================================
struct ServerSettings
{
    boost::asio::ip::tcp::endpoint endpoint;        // endpoint to listen
    size_t num_threads;                             // number of worker threads, 0 - auto detection (by hardware concurrency)
    size_t sheduled_connection_delete_period_msec;  // time period check and delete connections which are dead (which have been already scheduled to be deleted later)
    ConnectionSettings connection_settings;         // settings for every new connection
};



}

#endif //HTTP_SETTINGS_HPP_
