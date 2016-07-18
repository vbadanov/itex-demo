
#ifndef AUX_SPINLOCK_HPP_
#define AUX_SPINLOCK_HPP_

#include <atomic>

namespace aux
{

class spinlock
{
    public:
		//===============================================================================
		inline void lock()
		{
			while(lock_.test_and_set(std::memory_order_acquire)) {};
		}

		//===============================================================================
		inline void unlock()
		{
			lock_.clear(std::memory_order_release);
		}

		inline bool is_locked()
		{
			bool res = lock_.test_and_set(std::memory_order_acquire);
			if(!res)
			{
				lock_.clear(std::memory_order_release);
			}
			return res;
		}

    private:
        std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};

}
#endif //AUX_SPINLOCK_HPP_
