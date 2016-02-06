#ifndef UTIL_H
#define UTIL_H

#include <QtGlobal>

#include <math.h>

#define STR1(x) #x
#define STR(x) STR1(x)

#ifndef MIN
#define MIN(a,b) ( (a) <= (b) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) >= (b) ? (a) : (b) )
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef DEG2RAD
#define DEG2RAD(x) (x*(M_PI/180.0))
#endif

#ifndef RAD2DEG
#define RAD2DEG(x) (x*(180.0/M_PI))
#endif

#define EPSILON  0.0000001
#define EPSILONf 0.000001f
// since == isn't reliable for floats, use this instead
#define eqf(x,y) (fabs(x-y) < EPSILON)
#define feqf(x,y) (fabsf(x-y) < EPSILONf)

/// retrieve a time value from the system's high resolution timer, in seconds
extern double getTime();
/// \brief std::rand() based random number from [min, max].
extern double getRand(double min = 0., double max = 1.);


#endif // UTIL_H

