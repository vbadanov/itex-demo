
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <sched.h>
#include <random>

using boost::asio::ip::tcp;

static constexpr unsigned long READ_BUFFER_SIZE = 1*1024*1024;

void run(const char* host, const char* port, const long num_iterations, const long num_queries_to_send)
{
    try
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> before_connect_distribution(0,10000);
        std::uniform_int_distribution<int> before_query_distribution(0,1000);

        for (long i = 0; i < num_iterations; ++i)
        {
            boost::asio::io_service io_service;

            tcp::resolver resolver(io_service);
            tcp::resolver::query query(tcp::v4(), host, port);
            tcp::resolver::iterator iterator = resolver.resolve(query);

            tcp::socket s(io_service);

            std::this_thread::sleep_for(std::chrono::milliseconds(before_connect_distribution(generator)));

            boost::asio::connect(s, iterator);

            std::string response;
            boost::asio::streambuf response_streambuf;

            std::string query_str("select * from table1 as t1 inner join table2 as t2 on t1.name = t2.name");

            char* read_buffer = new char[READ_BUFFER_SIZE];

            size_t total_bytes_read = 0;
            size_t cnt = 1;
			for (long j = 0; j < num_queries_to_send; ++j)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(before_query_distribution(generator)));
				boost::asio::write(s, boost::asio::buffer(query_str.c_str(), query_str.size()));

				do
				{
					size_t read_bytes = s.read_some(boost::asio::buffer(read_buffer, READ_BUFFER_SIZE));
					total_bytes_read += read_bytes;
					++cnt;
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				} while(s.available());
			}
			std::cout << "| " << i << " | " << total_bytes_read/cnt << " | " << total_bytes_read << " | " << std::endl;

			delete[] read_buffer;

			boost::system::error_code ec;
			s.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
			s.close();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
};


int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cerr << "Usage: boost_asio_query_client <host> <port> <num_iterations> <num_queries_to_send> <num_threads>\n";
        return 1;
    }
    long num_threads = atol(argv[5]);
    std::vector<std::thread> thread_pool;
    for (long i = 0; i < num_threads; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        thread_pool.emplace_back([&]
                                  {
                                        run(argv[1], argv[2], atol(argv[3]), atol(argv[4]));
                                  });
    }
    for (auto &t : thread_pool)
    {
        t.join();
    }

    return 0;
}

