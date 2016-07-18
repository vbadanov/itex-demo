
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <aux/utils.hpp>
#include <sqldb/database.hpp>
#include <sqldb/transaction.hpp>
#include <sqldb/result.hpp>

using boost::asio::ip::tcp;

#define TS std::setprecision(20) << std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1,1>>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() << " "

static const int TIMEOUT_SECONDS = 600;

std::atomic<int> total_number_of_connections(0);
volatile bool service_running = true;

class session: public std::enable_shared_from_this<session>
{
    public:
        explicit session(tcp::socket socket, sqldb::Database& db)
                : socket_(std::move(socket)), timer_(socket_.get_io_service()), strand_(socket_.get_io_service()), db_(db)
        {
        }

        void go()
        {
            auto self(shared_from_this());
            boost::asio::spawn(strand_,
                    [this, self](boost::asio::yield_context yield)
                    {
                        try
                        {
                            char data[1024];
                            while(service_running)
                            {
                                try
                                {
                                    timer_.expires_from_now(std::chrono::seconds(TIMEOUT_SECONDS));
                                    std::size_t n = socket_.async_read_some(boost::asio::buffer(data), yield);
                                    std::string query_str(data, n);
                                    boost::algorithm::trim_right(query_str);

                                    std::cout << TS << "Start query" << std::endl;
                                    std::cout << TS << "Get transaction" << std::endl;
                                    sqldb::Transaction tr(db_);
                                    sqldb::Results results = tr.async_exec(query_str, yield);

                                    int id;
                                    std::string name;
                                    size_t cnt_q = 0;
                                    for (auto& query_result : results)
                                    {
                                        size_t cnt_r = 0;
                                        for(auto row : query_result)
                                        {
                                            row.copy_to(id, name);
                                            std::string result_row = (boost::format("DATA[%1%, %2%]: %3%, %4%\n") % cnt_q % cnt_r % id % name).str();
                                            timer_.expires_from_now(std::chrono::seconds(TIMEOUT_SECONDS));
                                            boost::asio::async_write(socket_, boost::asio::buffer(result_row, result_row.size()), yield);
                                            ++cnt_r;
                                        }
                                        ++cnt_q;
                                    }

                                    std::cout << TS << "Commit transaction" << std::endl;
                                    tr.async_commit(yield);
                                }
                                catch (sqldb::Exception& e)
                                {
                                    boost::asio::async_write(socket_, boost::asio::buffer(std::string(e.what())), yield);
                                }
                            }
                        }
                        catch (std::exception& e)
                        {
                            socket_.close();
                            timer_.cancel();
                            --total_number_of_connections;
                            std::cout << TS << "Total connections: " << total_number_of_connections << std::endl;
                        }
                    });

            		boost::asio::spawn(strand_,
                    [this, self](boost::asio::yield_context yield)
                    {
                        while (socket_.is_open() && service_running)
                        {
                            boost::system::error_code ignored_ec;
                            timer_.async_wait(yield[ignored_ec]);
                            if (timer_.expires_from_now() <= std::chrono::seconds(0))
                            {
                                socket_.close();
                                --total_number_of_connections;
                                std::cout << TS << "Total connections: " << total_number_of_connections << std::endl;
                            }
                        }
                    });
        }

        private:
        tcp::socket socket_;
        boost::asio::steady_timer timer_;
        boost::asio::io_service::strand strand_;
        sqldb::Database& db_;
    };

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: boost_asio_async_query_service <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;
        std::cout << TS<< "Created io_service" << std::endl;

        sqldb::Database db(io_service, "postgresql://postgres@localhost:5432/test", {60000, 1000, {10000, 1000}});
        std::cout << TS<< "Created DB Pool" << std::endl;

        tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
        std::cout << TS << "Created acceptor" << std::endl;

        boost::asio::spawn(io_service, [&](boost::asio::yield_context yield)
        {
            while(service_running)
            {
                boost::system::error_code ec;
                tcp::socket socket(io_service);
                acceptor.async_accept(socket, yield[ec]);
                if (!ec) std::make_shared<session>(std::move(socket), db)->go();
                ++total_number_of_connections;
                std::cout << TS << "Total connections: " << total_number_of_connections << std::endl;
            }
        });
        std::cout << TS<< "Spawned acceptor" << std::endl;


        boost::asio::spawn(io_service, [&](boost::asio::yield_context yield)
        {
            boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
            std::cout << TS << "Created signal_set" << std::endl;

            signals.async_wait(yield);
            service_running = false;
            acceptor.cancel();
            acceptor.close();
            io_service.stop();

            std::cout << TS << "Service is shutting down... please wait..." << std::endl;
        });

        unsigned long num_threads = std::thread::hardware_concurrency();
        std::cout << TS<< "Starting " << num_threads << " io_service.run() threads" << std::endl;
        std::vector<std::thread> thread_pool;
        for (unsigned long i = 0; i < num_threads; ++i)
        {
            thread_pool.emplace_back([&]
            {
                //Assign each thread to particular processor core
            	aux::set_cpu_affinity(i);

                //Start I/O loop
                    io_service.run();
                });
        }
        for (auto &t : thread_pool)
        {
            t.join();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << TS<< "Exception: " << e.what() << "\n";
    }

    return 0;
}
