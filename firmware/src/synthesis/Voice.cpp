#include "Voice.h"
#include "utils/utils.h"
#include "drivers/Pinout.h"
#include "utils/Easing.h"

float IRAM_ATTR Voice::nextOscSample()
{
    return mWavefolder.Process(mOscillator.Process());
}

float IRAM_ATTR Voice::nextEnvSample()
{
    if (processEnv)
    {
        currentEnvValue = mEnvelope.Process();
        processEnv = false;
    }
    else
    {
        currentPEnvValue = mPitchEnvelope.Process();
        processEnv = true;
    }
    return currentEnvValue;
}

void IRAM_ATTR Voice::getFrames(Frame_t *frames, int number_frames)
{
    applyParams();
    for (int i = 0; i < number_frames; i++)
    {
        frames[i].valueLeft = (uint16_t)map((nextEnvSample() * (float)UINT16_MAX), UINT16_MAX, 0, 22000, 49000);
        frames[i].valueRight = (uint16_t)(((1.0f + (nextOscSample() * 0.5f)) * 0.48f) * (float)UINT16_MAX); /// offset, gain, scale, max_int
    }
}

// TODO move to proper location
float soft_quantizer(float input, float step, float threshold)
{
    float rounded = roundf(input / step) * step; // round input to nearest multiple of the quantization step
    float diff = fabsf(input - rounded);         // calculate difference between input and rounded value
    if (diff >= threshold)
    {                                                             // check if the difference exceeds the threshold
        float sign = (input > rounded) ? 1.0f : -1.0f;            // determine whether to add or subtract from the rounded value
        float eased = rounded + sign * (diff - threshold * step); // ease the value towards the input
        return eased;
    }
    else
    {
        return rounded;
    }
}

void Voice::applyParams()
{
    Configuration *tParams;
    xQueueReceive(Configuration::parameter_queue, &tParams, 15); // TODO test value
    mEnvelope.setAttack(fclamp(tParams->mParamStruct.interface[0].value + tParams->mParamStruct.interface[0].cv, 0.01f, 2.0f));
    mEnvelope.setDecay(fclamp(tParams->mParamStruct.interface[1].value + tParams->mParamStruct.interface[1].cv, 0.01f, 2.0f));
    mEnvelope.setSustain(tParams->mParamStruct.interface[2].value + tParams->mParamStruct.interface[2].cv);
    mEnvelope.setRelease(fclamp(tParams->mParamStruct.interface[3].value + tParams->mParamStruct.interface[3].cv, 0.01f, 2.0f));
    // TODO better curve
    mEnvelope.setShape(1.0f - fclamp(tParams->mParamStruct.interface[8].value * tParams->mParamStruct.interface[8].value));
    //  PITCH ENVELOPE
    mPitchEnvelope.SetDecay(fclamp(tParams->mParamStruct.interface[1].value + tParams->mParamStruct.interface[1].cv) * pEnvRatio + 0.01f);
    // VOICE
    mOscillator.SetShape(fclamp(Easing::EaseInOutQuad(tParams->mParamStruct.interface[4].value + tParams->mParamStruct.interface[4].cv), 0.01f, 1.0f));
    mWavefolder.SetFold(fmap(fclamp((Easing::EaseInOutQuad(tParams->mParamStruct.interface[5].value + tParams->mParamStruct.interface[5].cv))), 1.0f, 10.0f));
    SetEnvAmount(fmap(tParams->mParamStruct.interface[7].value + tParams->mParamStruct.interface[7].cv, -1.0f, 1.0f));
    setFrequency(abs(volts_to_frequency(1.0f + (soft_quantizer(tParams->mParamStruct.interface[6].value * freqRange, 0.08333333333f, 0.02f)) + (tParams->mParamStruct.interface[6].cv) + (currentPEnvValue * pEnvAmount))));
    if (tParams->mParamStruct.interface[9].hasChanged)
    {
        dacWrite(OFFSET_ENV, (int)fmap((tParams->mParamStruct.interface[9].value), 119.0f, 140.0f));
        tParams->mParamStruct.interface[9].hasChanged = false;
    }
}