
#ifndef AUX_RANDOM_HPP_
#define AUX_RANDOM_HPP_

#include <cstdint>
#include <chrono>

#include <aux/rng.hpp>

typedef double REAL;

namespace aux
{

//==============================================================================
// This is a fast, good generator if you're short on memory, but otherwise
// we rather suggest to use a xorshift128+ or xorshift1024* (for a very
// long period) generator.
// The state must be seeded with a nonzero value.
class Xorshift64star
{
    public:
        Xorshift64star()
		{
			// generate seed from timestamp
			x = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()) + reinterpret_cast<uint64_t>(this);
		}

    	inline void seed(uint64_t seed)
    	{
    		x = seed;
    	}

        inline uint64_t generate()
        {
        	x ^= x >> 12; // a
        	x ^= x << 25; // b
        	x ^= x >> 27; // c
        	return x * 2685821657736338717LL;
        }

    private:
        uint64_t x;
};

//==============================================================================
// This is a fast, top-quality generator. If 1024 bits of state are too
// much, try a xorshift128+ or generator.
// The state must be seeded so that it is not everywhere zero. If you have
// a 64-bit seed,  we suggest to seed a xorshift64* generator and use its
// output to fill s.
class Xorshift1024star
{
    public:
        Xorshift1024star()
			: p(0)
		{
			//generate seed from timestamp (see xorshift64star internals)
			Xorshift64star seed_rng;
			for(int i = 0; i < 16; i++)
			{
				s[i] = seed_rng.generate();
			}
		}

        inline void seed(uint64_t seed[16])
        {
        	for(int i = 0; i < 16; i++)
        	{
        		s[i] = seed[i];
        	}
        }

        inline uint64_t generate()
        {
        	register uint64_t s0 = s[ p ];
        	register uint64_t s1 = s[ p = ( p + 1 ) & 15 ];
        	s1 ^= s1 << 31; // a
        	s1 ^= s1 >> 11; // b
        	s0 ^= s0 >> 30; // c
        	return ( s[ p ] = s0 ^ s1 ) * 1181783497276652981LL;
        }

    private:
        uint64_t s[16];
        int p;
};

//==============================================================================
class RandomGenerator
{
	public:
		RandomGenerator()
		{
			unsigned long seed = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()) + reinterpret_cast<uint64_t>(this);
			prng2_.norm_rand_init(seed);
			prng2_.norm_rand_init(seed);
			prng2_.ran2_rand_init(seed);
		}

		// generates uniformly distributed 64-bit unsigned integer
		inline uint64_t uniform_uint64()
		{
			return prng_.generate();
		}

		// generates uniformly distributed double in interval [0,1]
		inline double uniform_double_0to1()
		{
			return static_cast<double>(prng_.generate()) * (1.0/18446744073709551616.0);
		}

		// generates uniformly distributed double in interval [-1,1]
		inline double uniform_double_m1to1()
		{
			return (uniform_double_0to1() - 0.5) * 2;
		}

		inline double uniform_double()
		{
			register DoubleToFromUint64 val;
			val.ui64 = prng_.generate();
			return val.d;
		}

		// generate random number with normal distribution (mean = 0, sigma = 1)
		inline double norm_double()
		{
			return prng2_.norm_rand_real();
		}

	private:
		Xorshift1024star prng_;
		RandomNumberGenerator prng2_;

		union DoubleToFromUint64
		{
			double d;
			uint64_t ui64;
		};

};

}
#endif //AUX_RANDOM_HPP_

