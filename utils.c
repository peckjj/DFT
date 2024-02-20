#include <stdint.h>
#include <complex.h>
#include <math.h>

#include "utils.h"

double map(int32_t x, int32_t in_min, int32_t in_max, double out_min, double out_max)
{
    double inRange = (in_max + 0.0) - (in_min + 0.0);
    double outRange = out_max - out_min;

    return ((x + 0.0) - in_min) * (outRange) / (inRange) + out_min;
}

int sinWave(double complex *out, double freq, double amp, double phase, double res)
{
    for (long i = 0; i < res; i++)
    {
        out[i] = CMPLX(i / res, amp * sin(freq * 2 * M_PI * i / res));
    }

    return 0;
}