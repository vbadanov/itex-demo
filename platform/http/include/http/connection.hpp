
#ifndef HTTP_CONNECTION_HPP_
#define HTTP_CONNECTION_HPP_

#include <functional>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/server.hpp>
#include <http/settings.hpp>
#include <aux/spinlock.hpp>

namespace http
{

class Server;

class Connection
{
    friend Server;

    public:
        Connection() = delete;
        Connection(Server* server, boost::asio::ip::tcp::socket&& socket, const ConnectionSettings& settings);
        Connection(const Connection& connection) = delete;
        Connection(Connection&& connection) = delete;
        Connection& operator=(const Connection& connection) = delete;
        Connection& operator=(Connection&& connection) = delete;
        virtual ~Connection();

        void reinit(boost::asio::ip::tcp::socket&& socket);
        void start();
        void stop();
        void kill();
        bool is_running();
        bool is_timeout_occurred();
        bool is_ok();

        size_t get_id();
        Server& get_server();

    protected:
        void timeout_handler(const boost::system::error_code& ec);
        void check_timeout_occurred();

    private:
        Server* server_;
        boost::asio::ip::tcp::socket socket_;
        boost::asio::steady_timer timer_;
        boost::asio::io_service::strand strand_;
        ConnectionSettings settings_;
        IRequestHandler* handler_;
        volatile bool running_;
        bool timeout_occurred_;
        bool deletion_sheduled_;
        size_t id_;
        char* data_buffer_;
};

}

#endif //HTTP_CONNECTION_HPP_
