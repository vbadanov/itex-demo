
#include <thread>
#include <iostream>
#include <gtest/gtest.h>
#include <quant/workset.hpp>
#include <aux/utils.hpp>


//==============================================================================
TEST(QuantTest, WorksetTest)
{
	const size_t NUM_ITERATIONS = 100000000;
	const size_t WORKSET_SIZE = 100000;
	typedef itex::quant::Workset<size_t> Workset;
	Workset workset(WORKSET_SIZE);
	std::atomic<bool> running;
	running = false;

	std::thread s1(
			[&]()
			{
				aux::set_cpu_affinity(0);
				Workset::Subscriber* subscriber = workset.get_subscriber();
				while(!running) {};
				for(size_t i = 0; i < NUM_ITERATIONS; ++i)
				{
					size_t value = 0;
					if(!running)
					{
						std::cout << "subscriber #1 forced to stop at iteration: " << i << std::endl;
						break;
					}
					while(!subscriber->try_pop(value)) {}
					EXPECT_EQ(i, value);
				}
				subscriber->release();
			}
	);

	std::thread s2(
				[&]()
				{
					aux::set_cpu_affinity(1);
					Workset::Subscriber* subscriber = workset.get_subscriber();
					while(!running) {};
					for(size_t i = 0; i < NUM_ITERATIONS; ++i)
					{
						size_t value = 0;
						if(!running)
						{
							std::cout << "subscriber #2 forced to stop at iteration: " << i << std::endl;
							break;
						}
						while(!subscriber->try_pop(value)) {}
						EXPECT_EQ(i, value);
					}
					subscriber->release();
				}
		);

	std::thread s3(
			[&]()
			{
				aux::set_cpu_affinity(2);
				Workset::Subscriber* subscriber = workset.get_subscriber();
				while(!running) {};
				for(size_t i = 0; i < NUM_ITERATIONS; ++i)
				{
					size_t value = 0;
					if(!running)
					{
						std::cout << "subscriber #3 forced to stop at iteration: " << i << std::endl;
						break;
					}
					while(!subscriber->try_pop(value)) {}
					EXPECT_EQ(i, value);
				}
				subscriber->release();
			}
	);

	std::thread s4(
				[&]()
				{
					aux::set_cpu_affinity(3);
					Workset::Subscriber* subscriber = workset.get_subscriber();
					while(!running) {};
					for(size_t i = 0; i < NUM_ITERATIONS; ++i)
					{
						size_t value = 0;
						if(!running)
						{
							std::cout << "subscriber #4 forced to stop at iteration: " << i << std::endl;
							break;
						}
						while(!subscriber->try_pop(value)) {}
						EXPECT_EQ(i, value);
					}
					subscriber->release();
				}
		);

	std::thread s5(
				[&]()
				{
					aux::set_cpu_affinity(4);
					Workset::Subscriber* subscriber = workset.get_subscriber();
					while(!running) {};
					for(size_t i = 0; i < NUM_ITERATIONS; ++i)
					{
						size_t value = 0;
						if(!running)
						{
							std::cout << "subscriber #5 forced to stop at iteration: " << i << std::endl;
							break;
						}
						while(!subscriber->try_pop(value)) {}
						EXPECT_EQ(i, value);
					}
					subscriber->release();
				}
		);

	std::thread s6(
				[&]()
				{
					aux::set_cpu_affinity(5);
					Workset::Subscriber* subscriber = workset.get_subscriber();
					while(!running) {};
					for(size_t i = 0; i < NUM_ITERATIONS; ++i)
					{
						size_t value = 0;
						if(!running)
						{
							std::cout << "subscriber #6 forced to stop at iteration: " << i << std::endl;
							break;
						}
						while(!subscriber->try_pop(value)) {}
						EXPECT_EQ(i, value);
					}
					subscriber->release();
				}
		);

	std::thread s7(
				[&]()
				{
					aux::set_cpu_affinity(6);
					Workset::Subscriber* subscriber = workset.get_subscriber();
					while(!running) {};
					for(size_t i = 0; i < NUM_ITERATIONS; ++i)
					{
						size_t value = 0;
						if(!running)
						{
							std::cout << "subscriber #7 forced to stop at iteration: " << i << std::endl;
							break;
						}
						while(!subscriber->try_pop(value)) {}
						EXPECT_EQ(i, value);
					}
					subscriber->release();
				}
		);

	std::thread p(
			[&]()
			{
				aux::set_cpu_affinity(7);
				Workset::Publisher* publisher = workset.get_publisher();
				while(!running) {};
				for(size_t i = 0; i < NUM_ITERATIONS; ++i)
				{
					publisher->push(i);
					if(i % 1000000 == 0)
					{
						std::cout << "producer: " << (double)i / (double)NUM_ITERATIONS * 100 << "% done"<< std::endl;
					}

					// busy loop delay to wait consumers
					volatile double tmp = 1;
					for(size_t j = 0; j < 150; ++j)
					{
						tmp *= (double)j;
					}
				}
				publisher->release();
				running = false;
			}
	);


	running = true;
	s1.join();
	s2.join();
	s3.join();
	p.join();
}

//============================================================================//
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
