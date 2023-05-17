#include "fm2.h"

using namespace daisysp;

void Fm2::Init(float samplerate)
{
    // init oscillators
    car_.Init(samplerate);
    mod_.Init(samplerate);
    filter.Init(samplerate);
    filter.SetRes(0.77f);
    filter.SetDrive(0.77f);
    // set some reasonable values
    lfreq_ = 440.f;
    lratio_ = 5.0f;
    famt_ = 1.0f;
    fout_ = 0.0f;
    out_ = 0.0f;
    SetFreq(lfreq_);
    SetRatio(lratio_);

    car_.SetAmp(1.f);
    mod_.SetAmp(1.f);
    car_.SetWaveform(Oscillator::WAVE_TRI);
    mod_.SetWaveform(Oscillator::WAVE_NOISE);
    idx_ = 0.0f;
}

void Fm2::SetShape(float amount)
{
    amount = fclamp(amount, 0.0f, 1.0f);
    if (amount < 0.5f)
    {
        SetIndex(0.0f);
        SetFilterAmt(fclamp(1.0f - (amount * 2.0f), 0.0f, 1.0f));
    }
    else if (amount >= 0.5f)
    {
        SetFilterAmt(0.0f);
        SetIndex(fclamp(((amount - 0.5f) * 0.055f), 0.0f, 1.0f));
    }
}

float Fm2::Process()
{
    if (lratio_ != ratio_ || lfreq_ != freq_)
    {
        lratio_ = ratio_;
        lfreq_ = freq_;
        car_.SetFreq(lfreq_);
        mod_.SetFreq(lfreq_ * lratio_);
    }
    float modval = mod_.Process();
    car_.PhaseAdd(modval * idx_);
    out_ = car_.Process();

    filter.Process(out_);
    fout_ = filter.Band();

    float output = ((out_ * (1.0f - famt_)) + (fout_ * famt_));
    return output;
}

void Fm2::SetFreq(float freq)
{
    freq_ = fabsf(freq);
    filter.SetFreq(freq_);
}

void Fm2::SetRatio(float ratio)
{
    ratio_ = fabsf(ratio);
}

void Fm2::SetIndex(float index)
{
    idx_ = index * kIdxScalar;
}

float Fm2::GetIndex()
{
    return idx_ * kIdxScalarRecip;
}

void Fm2::Reset()
{
    car_.Reset();
}
