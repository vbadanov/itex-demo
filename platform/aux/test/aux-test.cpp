
#include <thread>
#include <mutex>
#include <deque>
#include <sched.h>
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>

#include <aux/spinlock.hpp>
#include <aux/hash.hpp>
#include <aux/timeout-guard.hpp>
#include <aux/random.hpp>
#include <aux/utils.hpp>
#include <aux/array3d.hpp>

//==============================================================================
struct TestStruct
{
	TestStruct(double a = 0.0, int* b = nullptr) : a(a), b(b) {}
	TestStruct(const TestStruct& rhs) : a(rhs.a), b(rhs.b) {}
	TestStruct(TestStruct&& rhs) : a(rhs.a), b(rhs.b) { rhs.a=0; rhs.b=nullptr; }
	TestStruct& operator=(const TestStruct& rhs) { a=rhs.a; b=rhs.b; return *this; }
	TestStruct& operator=(TestStruct&& rhs) { a=rhs.a; rhs.a=0; b=rhs.b; rhs.b=nullptr; return *this; }
	bool operator==(const TestStruct& rhs)
	{
		return (a == rhs.a && b == rhs.b);
	}
	double a;
	int* b;
};
bool operator==(const TestStruct& lhs, const TestStruct& rhs)
{
	return (lhs.a == rhs.a && lhs.b == rhs.b);
}

TEST(Array3dTest, AuxTest)
{
	double test[] = {0, 1, 2, 3, 4, 5, 6, 7};
	aux::Array3d<double> a({2, 2, 2}, 0.0, test);
	EXPECT_EQ(a(0,0,0), 0);
	EXPECT_EQ(a(1,0,0), 1);
	EXPECT_EQ(a(0,1,0), 2);
	EXPECT_EQ(a(1,1,0), 3);
	EXPECT_EQ(a(0,0,1), 4);
	EXPECT_EQ(a(1,0,1), 5);
	EXPECT_EQ(a(0,1,1), 6);
	EXPECT_EQ(a(1,1,1), 7);

	EXPECT_EQ(a(-1,-1,-1), a(1,1,1));

	int val = 123;
	TestStruct test_struct[] = {TestStruct(0, nullptr), TestStruct(1, &val), TestStruct(2, nullptr), TestStruct(3, &val), TestStruct(4, nullptr), TestStruct(5, &val), TestStruct(6, nullptr), TestStruct(7, &val)};
	aux::Array3d<TestStruct> a_struct({2, 2, 2}, TestStruct(), test_struct);
	EXPECT_EQ(a_struct(0,0,0), TestStruct(0, nullptr));
	EXPECT_EQ(a_struct(1,0,0), TestStruct(1, &val));
	EXPECT_EQ(a_struct(0,1,0), TestStruct(2, nullptr));
	EXPECT_EQ(a_struct(1,1,0), TestStruct(3, &val));
	EXPECT_EQ(a_struct(0,0,1), TestStruct(4, nullptr));
	EXPECT_EQ(a_struct(1,0,1), TestStruct(5, &val));
	EXPECT_EQ(a_struct(0,1,1), TestStruct(6, nullptr));
	EXPECT_EQ(a_struct(1,1,1), TestStruct(7, &val));

	EXPECT_EQ(a_struct(-1,-1,-1), a_struct(1,1,1));

}


//==============================================================================
TEST(GetMemoryUsageTest, AuxTest)
{
	std::cout << "Current memory usage: " << aux::system_memory_usage_ratio() * 100.0 << "%" << std::endl;
}

//==============================================================================
TEST(SpinlockTest, AuxTest)
{
    typedef aux::spinlock lock_t;
    aux::spinlock lock;

    const size_t QUEUE_SIZE = 1000;
    const size_t NUM_ITERATIONS = 1000000;
    unsigned long num_threads = std::thread::hardware_concurrency();
    std::cout << "Starting " << num_threads << " threads" << std::endl;
    std::vector<std::thread> thread_pool;
    std::deque<size_t> queue(QUEUE_SIZE, 0);
    for (unsigned long i = 0; i < num_threads; ++i)
    {
        thread_pool.emplace_back([&]
        {
            //Assign each thread to particular processor core
            aux::set_cpu_affinity(i);

            //Start I/O loop
            for(unsigned long long i = 0; i < NUM_ITERATIONS; ++i)
            {
                std::lock_guard<lock_t> lg(lock);
                size_t val = queue.back();
                queue.pop_back();
                queue.push_front(++val);
            }
        });
    }
    for (auto &t : thread_pool)
    {
        t.join();
    }

    size_t expected_value = NUM_ITERATIONS * num_threads / QUEUE_SIZE;
    for(auto i : queue)
    {
        EXPECT_EQ(i, expected_value);
    }
    std::cout << std::endl;
}

//==============================================================================
TEST(HashTest, AuxTest)
{
    std::string h001 = aux::to_string(aux::hash("Hello World!"));
    EXPECT_EQ(h001, std::string("61341628404781883098687500903828582500"));

    std::string h002 = aux::to_string(aux::hash(""));
    EXPECT_EQ(h002, std::string("44632409380718249394374473821787594281"));

    std::string h003 = aux::to_string(aux::hash("sldfkjsldfkjslk;gdsfg lkjfg;lsdfjgeprot ppodkf;sdlfgklsdfkg'sdfg'dfgdfglk"));
    EXPECT_EQ(h003, std::string("27588791792834030352407490897104294260"));

    std::string statement_name;
    const size_t NUM_ITERATIONS = 1000000;
    for(unsigned long long i = 0; i < NUM_ITERATIONS; ++i)
    {
        statement_name = aux::to_string(aux::hash("insert into table1 as t1 inner join table2 as t2 on t1.name = t2.name; select something in another query;"));
        statement_name.clear();
    }
}


//==============================================================================
void timeout_handler(const boost::system::error_code& ec, int type)
{
    std::cout << "timeout_handler: entered; type: " << type << std::endl;
    if (ec == boost::asio::error::operation_aborted)
    {
        std::cout << "timeout_handler: ec = operation_aborted" << std::endl;
        return;
    }

    switch(type)
    {
        case 0:
            std::cout << "timeout_handler: timeout type 0" << std::endl;
            break;

        case 1:
            std::cout << "timeout_handler: timeout type 1" << std::endl;
            break;

        default:
            std::cout << "timeout_handler: timeout type unknown" << std::endl;
            break;
    }
}

TEST(TimerGuardTest, AuxTest)
{
    std::cout << "Create io_service object" << std::endl;
    boost::asio::io_service io_service;

    std::cout << "Create timer object" << std::endl;
    boost::asio::steady_timer timer(io_service);

    size_t TIMEOUT = 1000;
    std::cout << "Create timer_guard object" << std::endl;
    aux::timeout_guard tg(timer,
                       std::chrono::milliseconds(TIMEOUT),
                       std::bind(timeout_handler, std::placeholders::_1, 1)
                      );

    std::cout << "Run io_service" << std::endl;
    io_service.run();

    std::cout << "Sleep 2*TIMEOUT" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT * 2));
}

//==============================================================================
TEST(RandomTest, AuxTest)
{
    aux::Xorshift64star xor64;
    aux::Xorshift1024star xor1024;
    aux::RandomGenerator prng;

    for(auto i = 0; i < 10; i++)
    {
    	std::cout << "[" << xor64.generate() << " / " << xor1024.generate() << " / " << prng.uniform_uint64() << " / " << prng.uniform_double_0to1() << " / " << prng.uniform_double() << "]" << std::endl;
    }


    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    double acc = 0.0;
    double min = 1.0;
    double max = 0.0;
    double val = 0.0;
    uint64_t num_iterations = 0;
    std::cout << "start time: " << start.time_since_epoch().count() << std::endl;
    while(end - start < std::chrono::seconds(3))
    {
    	for (uint64_t i = 0; i < 100000000; i++)
    	{
    		val = prng.uniform_double_0to1();
    		acc += val;
    		if(val > max)
    		{
    			max = val;
    		}
    		if(val < min)
    		{
    			min = val;
    		}
    		num_iterations++;
    	}
        end = std::chrono::system_clock::now();
    }
    std::cout << "start time: " << end.time_since_epoch().count() << std::endl;

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Random generator test. Min: " << min << "; Max: " << max << "; Performance: " << static_cast<double>(num_iterations)/elapsed_seconds.count()/1000000.0 << " Millions generations per second" << std::endl;
}


//============================================================================//
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
