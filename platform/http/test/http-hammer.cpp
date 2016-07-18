
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

#include <aux/utils.hpp>

#include <http/parser.hpp>
#include <http/response.hpp>


static constexpr unsigned long READ_BUFFER_SIZE = 32768;

bool running = false;
std::atomic<size_t> total_requests;

//===============================================================================
struct params_t
{
    std::string host;
    unsigned int port;
    std::string uri_string;
    size_t num_threads;
    size_t load_period_seconds;
    size_t max_connections;
    bool keep_alive;
};


//===============================================================================
class Worker
{
    public:
        Worker(boost::asio::io_service& io_service, const params_t& params, const std::string& request_string)
            : params_(params), strand_(io_service), socket_(io_service), request_string_(request_string)
        {
        	//pass
        }

        void run()
        {
            boost::asio::spawn(strand_, [&](boost::asio::yield_context yield)
            {
                boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(params_.host), params_.port);
                boost::system::error_code ec;
                http::Response response;
                http::Parser parser(&response);
                char data_buffer_[READ_BUFFER_SIZE];

                while(running)
                {
                    try
                    {
                        if(!socket_.is_open())
                        {
                            socket_.async_connect(endpoint, yield);
                        }

                        boost::asio::async_write(socket_, boost::asio::buffer(request_string_), yield/*[ec]*/);

                        ec.clear();
                        parser.clear();
                        response.clear();

                        while(!parser.complete())
                        {
                            std::size_t n = socket_.async_read_some(boost::asio::buffer(data_buffer_, READ_BUFFER_SIZE), yield/*[ec]*/);
                            parser.feed(data_buffer_, n);
                        }

                        if(!params_.keep_alive)
                        {
                            socket_.close();
                        }

                        if(response.status_code() == http::StatusCode::CODE_200_OK && response.body().size() > 0)
                        {
                            ++total_requests;
                        }
                    }
                    catch (std::exception& e)
                    {
                        //std::cout << e.what() << std::endl;
                        socket_.close();
                    }
                }
            });
        }

    private:
        params_t params_;
        boost::asio::io_service::strand strand_;
        boost::asio::ip::tcp::socket socket_;
        std::string request_string_;
};


//===============================================================================
int main(int argc, char* argv[])
{
    if (argc != 8)
    {
        std::cerr << "Usage: http-hammer <host> <port> <uri_string> <num_threads> <load_period_seconds> <max_connections> <keep_alive_bool>\n";
        return 1;
    }

    boost::asio::io_service io_service;
    params_t params;

    params.host = std::string(argv[1]);
    params.port = std::atoi(argv[2]);
    params.uri_string = std::string(argv[3]);
    params.num_threads = std::atoll(argv[4]);
    params.load_period_seconds = atoll(argv[5]);
    params.max_connections = atoll(argv[6]);
    params.keep_alive = static_cast<bool>(atol(argv[7]));

    if(params.num_threads == 0)
    {
        params.num_threads = std::thread::hardware_concurrency();
    }

    total_requests = 0;
    running = true;

    std::string request_string =
            std::string("GET ") + params.uri_string + std::string(" HTTP/1.1\r\n") +
            std::string("Host: ") + params.host + std::string("\r\n") +
            std::string(params.keep_alive ? "Connection: Keep-Alive\r\n" : "") +
            //std::string("Content-Length: 0\r\n\r\n");
            std::string("\r\n");

    for(size_t i = 0; i < params.max_connections; ++i)
    {
        Worker* worker = new Worker(io_service, params, request_string);
        worker->run();
    }

    boost::asio::spawn(io_service, [&](boost::asio::yield_context yield)
    {
        boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
        signals.async_wait(yield);
        running = false;
        io_service.stop();
        std::cout << "Shutting down... please wait..." << std::endl;
    });

    boost::asio::steady_timer completion_timer(io_service, std::chrono::seconds(params.load_period_seconds));
    boost::asio::spawn(io_service, [&](boost::asio::yield_context yield)
    {
        boost::system::error_code ec;
        completion_timer.async_wait(yield[ec]);
        if (ec != boost::asio::error::operation_aborted)
        {
            running = false;

            std::cout << "Finishing... please wait..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            io_service.stop();
        }
    });

    boost::asio::steady_timer stats_display_timer(io_service, std::chrono::seconds(1));
    boost::asio::spawn(io_service, [&](boost::asio::yield_context yield)
    {
        while(running)
        {
            boost::system::error_code ec;
            stats_display_timer.expires_from_now(std::chrono::seconds(1));
            stats_display_timer.async_wait(yield[ec]);
            if (ec != boost::asio::error::operation_aborted)
            {
                std::cout << total_requests << std::endl;
            }
        }
    });

    std::vector<std::thread> thread_pool;
    for (size_t i = 0; i < params.num_threads; ++i)
    {
        thread_pool.emplace_back([&]
        {
            //Assign each thread to particular processor core
        	aux::set_cpu_affinity(i);

            std::cout << "START THREAD " << std::this_thread::get_id() << std::endl;

            //Start I/O loop
            io_service.run();
        });
    }

    for (auto &t : thread_pool)
    {
        t.join();
    }

    std::cout << "\nTotal requests: " << total_requests << std::endl;
    std::cout << "Req per second: " << total_requests / params.load_period_seconds << std::endl;

    return 0;
}
