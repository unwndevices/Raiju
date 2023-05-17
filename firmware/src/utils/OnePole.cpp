#include "OnePole.h"

#define V_PI 3.14159265358979323846264338327950288

OnePole::OnePole()
{
}

OnePole::OnePole(float cutoff, float samplingRate, Type type)
{
}

int OnePole::Init(float cutoff, float samplingRate, Type type)
{
    if (cutoff > samplingRate / 2)
        return -1;
    ym1 = 0.0; // Reset filter state
    _samplingRate = samplingRate;
    setFilter(cutoff, type);
    return 0;
}

void OnePole::setFilter(float cutoff, Type type)
{
    _type = type;
    setFc(cutoff);
}

void OnePole::setFc(float cutoff)
{
    switch (_type)
    {
    case LOWPASS:
        b1 = expf(-2.0f * (float)V_PI * cutoff / _samplingRate);
        a0 = 1.0f - b1;
        break;
    case HIGHPASS:
        b1 = -expf(-2.0f * (float)V_PI * (0.5f - cutoff / _samplingRate));
        a0 = 1.0f + b1;
    }
}

float OnePole::Process(float input)
{
    return ym1 = input * a0 + ym1 * b1;
}