#ifndef VOICE_H
#define VOICE_H
#include "fm2.h"
#include "ADSREnv.h"
#include "ctagADEnv.h"
#include "waveshaper.h"
#include "i2s/SampleSource.h"
#include "Configuration.h"
#include "utils/conversions.h"

#define BASE_NOTE_FREQ 27.5f

class Voice : public SampleSource
{
public:
    Voice()
        : sr_(39100.0f),
          mFrequency(200.0f),
          isActive(false),
          pEnvRatio(0.25f),
          pEnvSemitones(12.00f),
          pEnvAmount(0.0f),
          processEnv(false){};

    inline void setFrequency(float freq)
    {
        mFrequency = freq;
        mOscillator.SetFreq(mFrequency);
    };

    virtual int getSampleRate() { return (int)sr_; };
    virtual void init(float sr)
    {

        mOscillator.Init(sr);
        mOscillator.SetIndex(5.0);
        mWavefolder.Init(sr);
        mEnvelope.setSampleRate(sr / 2.0f);
        mPitchEnvelope.SetSampleRate(sr / 2.0f);
        mPitchEnvelope.SetAttack(0.01f);
    };
    virtual void getFrames(Frame_t *frames, int number_frames);

    inline void SetEnvRatio(float ratio) { pEnvRatio = fclamp(ratio, 0.0f, 10.0f); }
    inline void SetEnvAmount(float amt) { pEnvAmount = fclamp(amt * pEnvSemitones * volts_per_semitone, -5.0f, 5.0f); }
    inline void SetEnvSemi(float semitones) { pEnvSemitones = semitones; }

    daisysp::Fm2 mOscillator;
    b259 mWavefolder;
    ADSREnv mEnvelope;
    ctagADEnv mPitchEnvelope;

private:
    void SetPitchEnv(Configuration *tParams);

    void applyParams();
    float nextOscSample();
    float nextEnvSample();
    float sr_;
    float mFrequency;
    float currentOscValue = 0.0f;
    float currentEnvValue = 0.0f;
    float currentPEnvValue = 0.0f;
    bool isActive;
    /// ratio applied to the pitch envelope rates,
    float pEnvRatio;
    float pEnvSemitones;
    float pEnvAmount;

    bool processEnv;

    float freqBase = 0.0f;
    float freqRange = 3.0f;
};
#endif // !VOICE_H