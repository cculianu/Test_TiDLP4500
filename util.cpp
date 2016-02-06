#include "Util.h"

#ifdef Q_OS_WIN
#include <windows.h>

double getTime()
{
    static __int64 freq = 0;
    static __int64 t0 = 0;
    __int64 ct;

    if (!freq) {
        QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&ct);   // reads the current time (in system units)
    if (!t0) t0 = ct;
    return double(ct-t0) / double(freq);
}

#elif defined(Q_OS_DARWIN)
#include <mach/mach_time.h>
double getTime()
{
    double t = static_cast<double>(mach_absolute_time());
    struct mach_timebase_info info;
    mach_timebase_info(&info);
    return t * (1e-9 * static_cast<double>(info.numer) / static_cast<double>(info.denom) );
}
#else
#include <QTime>
double getTime()
{
    static QTime t;
    static bool started = false;
    if (!started) { t.start(); started = true; }
    return double(t.elapsed())/1000.0;
}
#endif


double getRand(double min, double max)
{
    static bool seeded = false;
    if (!seeded) { seeded = true; qsrand(/*std::time(0)*/1234);  }
    int r = qrand();
    return double(r)/double(RAND_MAX-1) * (max-min) + min;
}
