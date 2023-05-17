#include "fold.h"
#include <math.h>
#include "utils/dsp.h"

using namespace daisysp;

void Fold::Init(float sr)
{
    incr_ = 0.0f;
    sample_index_ = 0;
    index_ = 0.0f;
    value_ = 0.0f;
    tone_ = 1400.0f;
    gain_ = 1.0f;
    filter.Init(tone_, sr);
}

float Fold::Process(float in)
{
    float out;

    if (index_ < sample_index_)
    {
        index_ += incr_;
        out = value_ = in;
    }
    else
    {
        out = value_;
    }

    out = daisysp::SoftLimit(out) * gain_;

    sample_index_++;
    return filter.Process(out);
}
