
#include <aux/rng.hpp>
#include <math.h>
#include <memory.h>
#include <cstdlib>

namespace aux
{

/*===========================================================================*/
RandomNumberGenerator::RandomNumberGenerator()
{
    _mt_left = 1;
    _mt_initf = 0;

    _ran2_idum2=123456789;
    _ran2_iy=0;
    _ran2_idum=0;
}

/*===========================================================================*/
RandomNumberGenerator::~RandomNumberGenerator()
{
}

/*===========================================================================*/
/* initializes state[N] with a seed */
void RandomNumberGenerator::uni_rand_init(unsigned long seed)
{
    int j;
    _mt_state[0]= seed & 0xffffffffUL;
    for (j=1; j<N; j++) {
        _mt_state[j] = (1812433253UL * (_mt_state[j-1] ^ (_mt_state[j-1] >> 30)) + j);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array state[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        _mt_state[j] &= 0xffffffffUL;  /* for >32 bit machines */
    }
    _mt_left = 1; _mt_initf = 1;
}

/*===========================================================================*/
/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void RandomNumberGenerator::uni_rand_init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    this->uni_rand_init(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        _mt_state[i] = (_mt_state[i] ^ ((_mt_state[i-1] ^ (_mt_state[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        _mt_state[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { _mt_state[0] = _mt_state[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        _mt_state[i] = (_mt_state[i] ^ ((_mt_state[i-1] ^ (_mt_state[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        _mt_state[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { _mt_state[0] = _mt_state[N-1]; i=1; }
    }

    _mt_state[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
    _mt_left = 1; _mt_initf = 1;
}

/*===========================================================================*/
inline void RandomNumberGenerator::mt_next_state(void)
{
    unsigned long *p=_mt_state;
    int j;

    /* if init_uni_rand() has not been called, */
    /* a default initial seed is used         */
    if (_mt_initf==0) this->uni_rand_init(5489UL);

    _mt_left = N;
    _mt_next = _mt_state;

    for (j=N-M+1; --j; p++)
        *p = p[M] ^ TWIST(p[0], p[1]);

    for (j=M; --j; p++)
        *p = p[M-N] ^ TWIST(p[0], p[1]);

    *p = p[M-N] ^ TWIST(p[0], _mt_state[0]);
}

/*===========================================================================*/
/* generates a random number on [0,0xffffffff]-interval */
unsigned long RandomNumberGenerator::uni_rand_int32(void)
{
    unsigned long y;

    if (--_mt_left == 0) this->mt_next_state();
    y = *_mt_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/*===========================================================================*/
/* generates a random number on [0,0x7fffffff]-interval */
long RandomNumberGenerator::uni_rand_int31(void)
{
    unsigned long y;

    if (--_mt_left == 0) this->mt_next_state();
    y = *_mt_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return (long)(y>>1);
}

/* These real versions are due to Isaku Wada, 2002/01/09 added */
/*===========================================================================*/
/* generates a random number on [0,1]-real-interval */
REAL RandomNumberGenerator::uni_rand_real1(void)
{
    unsigned long y;

    if (--_mt_left == 0) this->mt_next_state();
    y = *_mt_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    //return (REAL)y * (1.0/4294967295.0);
    return (REAL)y * (0.00000000023283064370807973754315);
    /* divided by 2^32-1 */
}

/*===========================================================================*/
/* generates a random number on [0,1)-real-interval */
REAL RandomNumberGenerator::uni_rand_real2(void)
{
    unsigned long y;

    if (--_mt_left == 0) this->mt_next_state();
    y = *_mt_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    //return (REAL)y * (1.0/4294967296.0);
    return (REAL)y * (0.00000000023283064365386962890625);
    /* divided by 2^32 */
}

/*===========================================================================*/
/* generates a random number on (0,1)-real-interval */
REAL RandomNumberGenerator::uni_rand_real3(void)
{
    unsigned long y;

    if (--_mt_left == 0) this->mt_next_state();
    y = *_mt_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    //return ((REAL)y + 0.5) * (1.0/4294967296.0);
    return ((REAL)y + 0.5) * (0.00000000023283064365386962890625);
    /* divided by 2^32 */
}

/*===========================================================================*/
/* generates a random number on [0,1] with 53-bit resolution*/
double RandomNumberGenerator::uni_rand_real_res53(void)
{
    unsigned long a = this->uni_rand_int32()>>5, b = this->uni_rand_int32()>>6;
    //return(a*67108864.0+b)*(1.0/9007199254740991.0);
    return(a*67108864.0+b) * (1.1102230246251566636831481088739e-16);
}


/*===========================================================================*/
/* This procedure sets the seed and creates the tables */
void RandomNumberGenerator::norm_rand_init(unsigned long seed)
{
   const REAL m1 = 2147483648.0;
   const REAL m2 = 4294967296.0;
   REAL dn=3.442619855899;
   REAL tn=dn;
   REAL vn=9.91256303526217e-3;
   REAL q;
   REAL de=7.697117470131487;
   REAL te=de;
   REAL ve=3.949659822581572e-3;
   int i;

   this->uni_rand_init(seed);

   /* Set up tables for norm_rand_real() */
   q=vn/exp(-.5*dn*dn);
   _zig_kn[0]=(unsigned int)((dn/q)*m1);
   _zig_kn[1]=0;

   _zig_wn[0]=q/m1;
   _zig_wn[127]=dn/m1;

   _zig_fn[0]=1.;
   _zig_fn[127]=exp(-.5*dn*dn);

    for(i=126;i>=1;i--)
    {
        dn = sqrt(-2.*log(vn/dn+exp(-.5*dn*dn)));
        _zig_kn[i+1] = (unsigned int)((dn/tn)*m1);
        tn = dn;
        _zig_fn[i] = exp(-.5*dn*dn);
        _zig_wn[i] = dn/m1;
    }
}

/*===========================================================================*/
/* Generate random number with normal distribution (mean = 0, sigma = 1) */
REAL RandomNumberGenerator::norm_rand_real()
{
    _zig_hz=this->uni_rand_int32();
    _zig_iz = _zig_hz & 127;
    return ((unsigned int)abs(_zig_hz) < _zig_kn[_zig_iz]) ? _zig_hz*_zig_wn[_zig_iz] : this->zig_nfix();
}

/*===========================================================================*/
/* generates variates from the residue when rejection in norm_rand_real() occurs */
inline REAL RandomNumberGenerator::zig_nfix(void)
{
    const REAL r = 3.442619855899; //3.442620f;     /* The start of the right tail */
    REAL x, y;
    for(;;)
    {
        x = _zig_hz*_zig_wn[_zig_iz];      /* iz==0, handles the base strip */
        if(_zig_iz==0)
        {
            do
            {
                x = -log(this->uni_rand_real3())*(1/r);
                y = -log(this->uni_rand_real3());
            } while(y+y < x*x);

            return (_zig_hz > 0) ? r+x : -r-x;
        }

        /* iz>0, handle the wedges of other strips */
        if( _zig_fn[_zig_iz]+this->uni_rand_real3()*(_zig_fn[_zig_iz-1]-_zig_fn[_zig_iz]) < exp(-.5*x*x) )
        {
            return x;
        }

        /* initiate, try to exit for(;;) for loop*/
        _zig_hz = this->uni_rand_int32();
        _zig_iz = _zig_hz & 127;
        if((unsigned int)abs(_zig_hz) < _zig_kn[_zig_iz]) return (_zig_hz*_zig_wn[_zig_iz]);
    }
}

/*===========================================================================*/
// The ran2 pseudo-random number generator.  It has a period of 2 * 10^18 and
// returns a uniform random deviate on the interval (0.0, 1.0) excluding the
// end values.  idum initializes the sequence, so we create a separate seeding
// function to set the seed.  If you reset the seed then you re-initialize the
// sequence.
void RandomNumberGenerator::ran2_rand_init(unsigned long seed)
{
    int j;
    long k;

    _ran2_idum = static_cast<long>(seed);
    if (_ran2_idum == 0) _ran2_idum=1;
    if (_ran2_idum < 0) _ran2_idum = -_ran2_idum;
    _ran2_idum2=(_ran2_idum);
    for (j=NTAB+7;j>=0;j--)
    {
        k=(_ran2_idum)/IQ1;
        _ran2_idum=IA1*(_ran2_idum-k*IQ1)-k*IR1;
        if (_ran2_idum < 0) _ran2_idum += IM1;
        if (j < NTAB) _ran2_iv[j] = _ran2_idum;
    }
    _ran2_iy=_ran2_iv[0];
}

/*===========================================================================*/
REAL RandomNumberGenerator::ran2_rand_real(void)
{
    int j;
    long k;
    REAL temp;

    k=(_ran2_idum)/IQ1;
    _ran2_idum=IA1*(_ran2_idum-k*IQ1)-k*IR1;
    if (_ran2_idum < 0) _ran2_idum += IM1;
    k=_ran2_idum2/IQ2;
    _ran2_idum2=IA2*(_ran2_idum2-k*IQ2)-k*IR2;
    if (_ran2_idum2 < 0) _ran2_idum2 += IM2;
    j=_ran2_iy/NDIV;
    _ran2_iy=_ran2_iv[j]-_ran2_idum2;
    _ran2_iv[j] = _ran2_idum;
    if (_ran2_iy < 1) _ran2_iy += IMM1;
    if ((temp=AM*_ran2_iy) > RNMX) return RNMX;
    else return temp;
}

}
