
#ifndef REAL_NUMBER_H_
#define REAL_NUMBER_H_

#include <float.h>

namespace aux
{

#ifdef SINGLE_PRECISION_REAL_NUMBER
typedef float REAL;
#define REAL_MIN_EPSILON FLT_EPSILON
#define REAL_MAX_POSITIVE FLT_MAX
#define REAL_MAX_NEGATIVE -FLT_MAX
#else
typedef double REAL;
#define REAL_MIN_EPSILON DBL_EPSILON
#define REAL_MAX_POSITIVE DBL_MAX
#define REAL_MAX_NEGATIVE -DBL_MAX
#endif

}

#endif // REAL_NUMBER_H_
