#include "ADSREnv.h"
#include "FastMath.h"
#include "utils/dsp.h"

volatile float ADSREnv::fs;

ADSREnv::ADSREnv()
{

    reset();
    setSampleRate(39100.0f);
    setAttack(3.0f);
    setDecay(1.0f);
    setRelease(3.0f);
    setSustain(1.0f);
    setTargetRatioA(1.0f);
    setTargetRatioDR(0.0001f);
}

ADSREnv::~ADSREnv()
{
}

void ADSREnv::setShape(float shape)
{
    setTargetRatioA(daisysp::fmap(shape, 1.0f, 4.0f));
    setTargetRatioDR(daisysp::fmap(shape, 0.001f, 4.0f));
}

void ADSREnv::setAttack(float rate)
{
    rate = rate * timeMultiplier + 0.005f;
    attackRate = rate * fs;
    attackCoef = calcCoef(rate, targetRatioA);
    attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
}

void ADSREnv::setDecay(float rate)
{
    rate = rate * timeMultiplier + 0.005f;
    decayRate = rate * fs;
    decayCoef = calcCoef(rate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
}

void ADSREnv::setRelease(float rate)
{
    rate = rate * timeMultiplier + 0.005f;
    releaseRate = rate * fs;
    releaseCoef = calcCoef(rate, targetRatioDR);
    releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

float ADSREnv::calcCoef(float rate, float targetRatio)
{
    return (rate <= 0) ? 0.0f : exp(-log((1.0f + targetRatio) / targetRatio) / rate);
}

void ADSREnv::setSustain(float level)
{
    level = daisysp::fclamp(level, 0.0f, 1.0f);
    level = daisysp::pow10f(level * 2.0f - 1.0f);
    level = daisysp::fclamp(level, 0.001f, 1.0f);

    sustainLevel = level;
    decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
}

void ADSREnv::setTargetRatioA(float targetRatio)
{
    if (targetRatio < 0.0000001f)
        targetRatio = 0.0000001f; // -180 dB
    targetRatioA = targetRatio;
    attackCoef = calcCoef(attackRate, targetRatioA);
    attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
}

void ADSREnv::setTargetRatioDR(float targetRatio)
{
    if (targetRatio < 0.0000001f)
        targetRatio = 0.0000001f; // -180 dB
    targetRatioDR = targetRatio;
    decayCoef = calcCoef(decayRate, targetRatioDR);
    releaseCoef = calcCoef(releaseRate, targetRatioDR);
    decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
    releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

void ADSREnv::setSampleRate(float fs_hz)
{
    fs = fs_hz;
}

bool ADSREnv::isIdle()
{
    return state == env_idle;
}

void ADSREnv::setModeLin()
{
    setTargetRatioA(10.f);
    setTargetRatioDR(10.f);
}

void ADSREnv::setModeExp()
{
    setTargetRatioA(1.0f);
    setTargetRatioDR(0.001f);
}

void ADSREnv::setMultiplier(timeMult mult)
{
    switch (mult)
    {
    case HIGH_M:
        timeMultiplier = 0.3f;
        break;
    case MID_M:
        timeMultiplier = 1.0f;
        break;
    case LOW_M:
        timeMultiplier = 10.0f;
        break;
    default:
        timeMultiplier = 0.3f;
        break;
    }
    log_d("%f", timeMultiplier);
}

void ADSREnv::hold()
{
    if (state == env_release)
    {
        state = env_sustain;
    }
}

uint8_t ADSREnv::getState()
{
    return state;
}

void ADSREnv::reset()
{
    state = env_idle;
    output = 0.0f;
    readyToTrigger = false;
    previousState = state;
    timeMultiplier = 1.0f;
}

float ADSREnv::getOutput()
{
    return output;
}

bool ADSREnv::ProcessStageBegin(envState evalState)
{
    if (evalState == state && readyToTrigger)
    {
        readyToTrigger = false;
        return true;
    }
    else if (evalState != state)
    {
        readyToTrigger = true;
    }
    return false;
}
envState ADSREnv::ProcessStageBegin()
{
    if (previousState != state)
    {
        previousState = state;
        return state;
    }
    else
    {
        return env_idle;
    }
}

bool ADSREnv::ProcessStageEnd(envState evalState)
{
    if (evalState == state && !readyToTrigger)
    {
        readyToTrigger = true;
    }
    else if (evalState != state && readyToTrigger)
    {
        readyToTrigger = false;
        return true;
    }
    return false;
}

envState ADSREnv::ProcessStageEnd()
{
    /// se lo stato cambia, manda stage precedente
    if (previousState == state && !readyToTrigger)
    {
        readyToTrigger = true;
        return env_idle;
    }

    else if (previousState != state && readyToTrigger)
    {
        envState outStage = previousState;
        previousState = state;
        return outStage;
    }

    else
        return env_idle;
}

bool ADSREnv::ProcessStageGate(envState evalState)
{
    if (evalState == state)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool ADSREnv::ProcessStageGate()
{
    if (state != env_idle)
    {
        return true;
    }
    else
    {
        return false;
    }
}