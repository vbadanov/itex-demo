
#ifndef AUX_HASH_HPP_
#define AUX_HASH_HPP_

#include <string>
#include <stdexcept>
#include <cstdint>
#include <citycrc.h>

namespace aux
{

//===============================================================================
inline uint128 hash(const std::string& str)
{
    return CityHashCrc128(str.c_str(), str.size());
}

//===============================================================================
inline std::string to_string(const uint128& hash_value)
{
    return std::to_string(hash_value.first).append(std::to_string(hash_value.second));
}

//===============================================================================
class hash_of_uint128 {
	public:
		inline std::size_t operator()(const uint128& value) const
		{
			return static_cast<size_t>(Hash128to64(value));
		}
};


}
#endif //AUX_SPINLOCK_HPP_
