#pragma once
#include <Arduino.h>

typedef enum
{
    env_idle = 0,
    env_attack,
    env_decay,
    env_sustain,
    env_release
} envState;

class ADSREnv
{
public:
    ADSREnv();

    ~ADSREnv();

    inline float Process();

    bool ProcessStageBegin(envState evalState);
    envState ProcessStageBegin();
    bool ProcessStageEnd(envState evalState);
    envState ProcessStageEnd();
    bool ProcessStageGate(envState evalState);
    bool ProcessStageGate();

    float getOutput();

    uint8_t getState();

    inline void Gate(bool gate);

    inline void Trig();

    void setAttack(float rate);

    void setDecay(float rate);

    void setRelease(float rate);

    void setSustain(float level);

    void setShape(float shape);

    void setTargetRatioA(float targetRatio);

    void setTargetRatioDR(float targetRatio);

    void reset();

    static void setSampleRate(float fs_hz);

    bool isIdle();

    void setModeLin();

    void setModeExp();

    void hold();

    // typedef void (*StateHandler)(uint8_t eventType, float value);

    typedef enum
    {
        LOW_M = 0,
        MID_M,
        HIGH_M
    } timeMult;

    void setMultiplier(timeMult mult);

private:
    float calcCoef(float rate, float targetRatio);

    envState state;
    envState previousState;
    static volatile float fs;
    float output;
    volatile float attackRate;
    volatile float decayRate;
    volatile float releaseRate;
    volatile float attackCoef;
    volatile float decayCoef;
    volatile float releaseCoef;
    volatile float sustainLevel;
    volatile float targetRatioA;
    volatile float targetRatioDR;
    volatile float attackBase;
    volatile float decayBase;
    volatile float releaseBase;
    volatile bool isTrig;
    volatile float timeMultiplier;

    volatile bool readyToTrigger;
};

inline float ADSREnv::Process()
{
    switch (state)
    {
    case env_idle:
        break;
    case env_attack:
        output = attackBase + output * attackCoef;
        if (output >= 1.0f)
        {
            output = 1.0f;
            state = env_decay;
        }
        break;
    case env_decay:
        output = decayBase + output * decayCoef;
        if (output <= sustainLevel)
        {
            output = sustainLevel;
            state = env_sustain;
        }
        break;
    case env_sustain:
        if (isTrig)
        {
            state = env_release;
        }
        break;
    case env_release:
        output = releaseBase + output * releaseCoef;
        if (output <= 0.000001f)
        {
            output = 0.0f;
            state = env_idle;
        }
    }
    return output;
}

inline void ADSREnv::Gate(bool gate)
{
    if (gate)
    {
        isTrig = false;

        if (state == env_idle || state == env_release)
        {
            state = env_attack;
        }
    }
    else if (state != env_idle)
    {
        state = env_release;
    }
}

inline void ADSREnv::Trig()
{
    state = env_attack;
    isTrig = true;
}