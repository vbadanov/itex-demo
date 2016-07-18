
#include <chrono>
#include <thread>
#include <mutex>
#include <iostream>
#include <aux/utils.hpp>
#include <http/server.hpp>

#ifndef __APPLE__
#include <gperftools/heap-checker.h>
#include <gperftools/malloc_extension.h>
#endif

namespace http
{

//===============================================================================
Server::Server(const ServerSettings& settings)
    : io_service_(),
      acceptor_(io_service_, settings.endpoint),
      settings_(settings),
      running_(false),
      scheduled_connections_delete_timer_(io_service_, std::chrono::milliseconds(settings_.sheduled_connection_delete_period_msec))
{
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
}

//===============================================================================
Server::~Server()
{
    { //lock scope
        std::lock_guard<aux::spinlock> lg(connection_pool_lock_);

        for(auto elt : connection_pool_)
        {
            if(elt.second != nullptr)
            {
                delete elt.second;
            }
        }
        connection_pool_.clear();
    }

    process_sheduled_connection_delete();
}

//===============================================================================
void Server::start()
{
    running_ = true;
    acceptor_.listen();

    unsigned long num_threads = settings_.num_threads;
    if(num_threads == 0)
    {
        num_threads = std::thread::hardware_concurrency();
    }

    for (unsigned long i = 0; i < num_threads; ++i)
    {
        boost::asio::spawn(io_service_, [&](boost::asio::yield_context yield)
        {
            while(running_)
            {
                try
                {
                    boost::system::error_code ec;
                    boost::asio::ip::tcp::socket socket(io_service_);
                    acceptor_.async_accept(socket, yield[ec]);
                    if (!ec)
                    {
                    	// disable Nagle algorithm
                    	socket.set_option(boost::asio::ip::tcp::no_delay(true));
                    	socket.set_option(boost::asio::socket_base::reuse_address(true));

                        Connection* conn = nullptr;

                        { //lock scope
                            std::lock_guard<aux::spinlock> lg(connections_to_delete_lock_);
                            if(!connections_to_delete_.empty())
                            {
                                conn = connections_to_delete_.front();
                                connections_to_delete_.pop();
                            }
                        }

                        if(conn != nullptr)
                        {
                            // reuse connection object
                            conn->reinit(std::move(socket));
                        }
                        else
                        {
                            // create new connection object
                            conn = new Connection(this, std::move(socket), settings_.connection_settings);
                        }

                        size_t current_pool_size = 0;
                        { //lock scope
                            std::lock_guard<aux::spinlock> lg(connection_pool_lock_);
                            connection_pool_[conn->get_id()] = conn;
                            current_pool_size = connection_pool_.size();
                        }

                        conn->start();

                        if (current_pool_size > 0 && current_pool_size % 1000UL == 0)
                        {
                            std::cout << current_pool_size << std::endl;
                        }
                    }
                }
                catch (std::exception& e)
                {
                    //TODO: add logging
                    std::cout << "ACCEPTOR Got exception: " << e.what() << std::endl;
                    throw e;
                    //pass
                }
            }
        });
    };

    boost::asio::spawn(io_service_, [&](boost::asio::yield_context yield)
    {
        while(running_)
        {
            boost::system::error_code ec;
            scheduled_connections_delete_timer_.expires_from_now(std::chrono::milliseconds(settings_.sheduled_connection_delete_period_msec));
            scheduled_connections_delete_timer_.async_wait(yield[ec]);
            if (ec != boost::asio::error::operation_aborted)
            {
                process_sheduled_connection_delete();
            }
        }
    });

    std::vector<std::thread> thread_pool;
    for (unsigned long i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back([&]
        {
            //Assign each thread to particular processor core
        	aux::set_cpu_affinity(i);

            //Start I/O loop
            io_service_.run();
        });
    }
    for (auto& t : thread_pool)
    {
        t.join();
    }
}

//===============================================================================
void Server::stop()
{
    running_ = false;

    { // lock scope
        std::lock_guard<aux::spinlock> lg_pool(connection_pool_lock_);
        while(!connection_pool_.empty())
        {
            Connection* conn = connection_pool_.begin()->second;
            conn->stop();

            { // lock scope
                std::lock_guard<aux::spinlock> lg_del(connections_to_delete_lock_);
                move_connection_to_delete_queue_nolock(conn->get_id());
            }
        }
    }
    scheduled_connections_delete_timer_.cancel();
    acceptor_.close();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    io_service_.stop();
    process_sheduled_connection_delete();
}

//===============================================================================
boost::asio::io_service& Server::get_io_service()
{
    return io_service_;
}

//===============================================================================
void Server::move_connection_to_delete_queue_nolock(const size_t& connection_id)
{
    auto elt = connection_pool_.find(connection_id);
    if(elt != connection_pool_.end())
    {
        connections_to_delete_.push(elt->second);
        connection_pool_.erase(elt);
    }
}
//===============================================================================
void Server::schedule_connection_delete(const size_t& connection_id)
{
    std::lock_guard<aux::spinlock> lg_pool(connection_pool_lock_);
    std::lock_guard<aux::spinlock> lg_del(connections_to_delete_lock_);

    move_connection_to_delete_queue_nolock(connection_id);

    size_t current_pool_size = connection_pool_.size();
    if (current_pool_size > 0 && current_pool_size % 1000UL == 0)
    {
        std::cout << current_pool_size << std::endl;
    }
}

//===============================================================================
void Server::process_sheduled_connection_delete()
{
    { //lock scope
        std::lock_guard<aux::spinlock> lg(connections_to_delete_lock_);

        const size_t num_conn = connections_to_delete_.size();
        if(num_conn > 0)
        {
            std::cout << "DELETING " << num_conn << " CONNECTIONS" << std::endl;
        }

        while (!connections_to_delete_.empty())
        {
            Connection* conn = connections_to_delete_.front();
            connections_to_delete_.pop();
            if(conn != nullptr)
            {
                delete conn;
                conn = nullptr;
            }
        }
    }

#ifndef __APPLE__
    MallocExtension::instance()->ReleaseFreeMemory();
#endif

}


}
