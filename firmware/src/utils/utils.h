#ifndef UTILS_H
#define UTILS_H
#include <Arduino.h>
#define UINT12_MAX 0xfff
#define UINT12_REC 0.0002442f
#define UINT16_REC 1.5259e-05f

#define LOW_PRIORITY 1
#define MED_PRIORITY 10
#define HIGH_PRIORITY 20

typedef void (*cB)(void *param);

typedef unsigned long long ullong;

namespace unwn
{

    inline uint16_t float_range_to_uint16_t(float value, float min_ = 0, float max_ = 1)
    {
        return ((value - min_) / (max_ - min_)) * UINT16_MAX;
    }

    inline float float_range_from_uint16_t(uint16_t value, float min_ = 0, float max_ = 1)
    {
        return (((float)value) * UINT16_REC) * (max_ - min_) + min_;
    }

    inline float float_range_from_uint12_t(uint16_t value, float min_ = 0.0f, float max_ = 1.0f)
    {
        return (((float)value) * UINT12_REC) * (max_ - min_) + min_;
    }

    inline float lerp_int_to_float(uint16_t value, float fmin, float fmax, uint16_t imin, uint16_t imax)
    {
        return (fmin + ((fmax - fmin) / (imax - imin)) * (value - imin));
    }
}

#endif // !UTILS_H