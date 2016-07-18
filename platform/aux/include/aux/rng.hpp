
#ifndef RANDOM_NUMBER_GENERATOR_H_
#define RANDOM_NUMBER_GENERATOR_H_

#include <aux/real-number.hpp>

namespace aux
{

// Mersenne twister implementation is taken from mt19937ar-cok.c
// Ziggurat implementation is taken from rnorrexp.c

// Mersenne twister period parameters
const int N = 624;
const int M = 397;
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

// ran2 parameters
#define IM1 2147483563L
#define IM2 2147483399L
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014L
#define IA2 40692L
#define IQ1 53668L
#define IQ2 52774L
#define IR1 12211L
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

// Class for random number generation: uniform and normal
class RandomNumberGenerator
{
    public:
        RandomNumberGenerator();
        ~RandomNumberGenerator();
        // Mersenne twister
        void uni_rand_init(unsigned long seed);
        void uni_rand_init_by_array(unsigned long init_key[], int key_length);
        unsigned long uni_rand_int32(void);
                 long uni_rand_int31(void);
                 REAL uni_rand_real1(void);
                 REAL uni_rand_real2(void);
                 REAL uni_rand_real3(void);
               double uni_rand_real_res53(void);

        // Ziggurat
        void norm_rand_init(unsigned long seed);
        REAL norm_rand_real(void);

        // ran2
        void ran2_rand_init(unsigned long seed);
        REAL ran2_rand_real(void);

    private:
        // Mersenne twister state
        unsigned long _mt_state[N]; /* the array for the state vector  */
        int _mt_left;
        int _mt_initf;
        unsigned long *_mt_next;
        void mt_next_state(void);

        // Ziggurat state
        int _zig_hz;
        unsigned int _zig_iz, _zig_kn[128], _zig_ke[256];
        REAL _zig_wn[128],_zig_fn[128], _zig_we[256],_zig_fe[256];
        REAL zig_nfix(void);

        // ran2 state
        long _ran2_idum2;
        long _ran2_iy;
        long _ran2_iv[NTAB];
        long _ran2_idum;
};

}

#endif // RANDOM_NUMBER_GENERATOR_H_
