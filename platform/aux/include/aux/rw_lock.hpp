
#ifndef RW_LOCK_HPP_
#define RW_LOCK_HPP_

#include <atomic>

namespace aux
{

class rw_lock
{
    public:
		//===============================================================================
		rw_lock()
    		: lock_(0)
    	{
			//pass
    	}

		//===============================================================================
		inline bool reader_try_lock()
		{
			bool res = false;
			int64_t val = lock_.load(std::memory_order_consume);
			size_t num_attempts = 1000;
			int64_t expected = 0;
			if(val < 0)
			{
				for(size_t i = 0; i < num_attempts && res == false; ++i)
				{
					expected = 0;
					res = lock_.compare_exchange_strong(expected, 1, std::memory_order_consume);
				}
			}
			else
			{
				for(size_t i = 0; i < num_attempts && res == false; ++i)
				{
					expected = val;
					res = lock_.compare_exchange_strong(expected, val + 1, std::memory_order_consume);
				}
			}
			return res;
		}

		//===============================================================================
		inline void reader_lock()
		{
			int64_t val = lock_.load(std::memory_order_consume);
			int64_t expected = 0;
			if(val < 0)
			{
				do
				{
					expected = 0;
				}
				while(!lock_.compare_exchange_strong(expected, 1, std::memory_order_consume));
			}
			else
			{
				do
				{
					expected = val;
				}
				while(!lock_.compare_exchange_strong(expected, val + 1, std::memory_order_consume));
			}
		}

		//===============================================================================
		inline void reader_unlock()
		{
			auto res = lock_.fetch_sub(1, std::memory_order_release);
		}

		//===============================================================================
		inline void writer_lock()
		{
			int64_t expected = 0;
			do
			{
				expected = 0;
			}
			while(!lock_.compare_exchange_strong(expected, -1, std::memory_order_acquire));
		}

		//===============================================================================
		inline void writer_unlock()
		{
			lock_.store(0, std::memory_order_release);
		}

    private:
        std::atomic<int64_t> lock_;

};

}
#endif //RW_LOCK_HPP_
