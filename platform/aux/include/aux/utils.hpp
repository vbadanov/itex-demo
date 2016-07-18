
#ifndef AUX_UTILS_HPP_
#define AUX_UTILS_HPP_

#include <cstdlib>
#include <stdexcept>
#include <cerrno>
#include <iostream>
#include <signal.h>

namespace aux
{

//===============================================================================
template<class T>
T* allocate_buffer(size_t size)
{
	T* tmp = nullptr;
	const size_t ALIGNMENT = 64;
	int error = posix_memalign(reinterpret_cast<void**>(&tmp), ALIGNMENT, size * sizeof(T));

	switch(error)
	{
		case EINVAL:
		{
			throw std::runtime_error("posix_memalign - the alignment argument was not a power of two, or was not a multiple of sizeof(void *)");
			break;
		}
		case ENOMEM:
		{
			throw std::runtime_error(std::string("posix_memalign - there was insufficient memory to fulfill the allocation request of ") + std::to_string(size) + std::string(" elements of ") + std::to_string(sizeof(T)) + " bytes each");
			break;
		}

		case 0:
		{
			break;
		}

		default:
		{
			throw std::runtime_error(std::string("posix_memalign - unknown error code: ") + std::to_string(error));
		}

	}
	return tmp;
}

//===============================================================================
void set_cpu_affinity(size_t cpu_number);

//===============================================================================
double system_memory_usage_ratio();

//===============================================================================
struct StackSentinel
{
#ifdef STACK_SENTINEL_ON
	private:
		static const size_t buffer_size = 16 * 1024;
		uint8_t buffer_[buffer_size];

	public:
		StackSentinel()
		{
			for(size_t i = 0; i < buffer_size; ++i)
			{
				buffer_[i] = 0xDD;
			}
		}

		~StackSentinel()
		{
			for(size_t i = 0; i < buffer_size; ++i)
			{
				if(buffer_[i] != 0xDD)
				{
					std::cout << std::endl << "!!! STACK SENTINEL - BAD !!!" << std::endl;
					raise(SIGTRAP);
				}
			}
		}
#endif
};




}

#endif //AUX_UTILS_HPP_
