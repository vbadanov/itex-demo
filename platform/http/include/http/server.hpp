
#ifndef HTTP_SERVER_HPP_
#define HTTP_SERVER_HPP_

#include <unordered_map>
#include <queue>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <http/request.hpp>
#include <http/response.hpp>
#include <http/connection.hpp>
#include <http/settings.hpp>
#include <aux/spinlock.hpp>

namespace http
{

//===============================================================================
class Connection;

//===============================================================================
class Server
{
    friend Connection;

    public:
        Server() = delete;
        Server(const ServerSettings& settings);
        Server(const Server& server) = delete;
        Server(Server&& server) = delete;
        Server& operator=(const Server& server) = delete;
        Server& operator=(Server&& server) = delete;
        virtual ~Server();

        void start();
        void stop();

        boost::asio::io_service& get_io_service();

    protected:
        void move_connection_to_delete_queue_nolock(const size_t& connection_id);
        void schedule_connection_delete(const size_t& connection_id);
        void scheduled_connections_delete_handler(const boost::system::error_code& ec);
        void process_sheduled_connection_delete();

    private:
        boost::asio::io_service io_service_;
        boost::asio::ip::tcp::acceptor acceptor_;
        ServerSettings settings_;
        volatile bool running_;
        aux::spinlock connection_pool_lock_;
        std::unordered_map<size_t, Connection*> connection_pool_;
        aux::spinlock connections_to_delete_lock_;
        std::queue<Connection*> connections_to_delete_;
        boost::asio::steady_timer scheduled_connections_delete_timer_;

};

}

#endif //HTTP_SERVER_HPP_
