#ifndef ANALOG_H
#define ANALOG_H
#include <Arduino.h>
#include "utils/utils.h"
#include "utils/dsp.h"
#include "utils/conversions.h"
#include "Multiplexer.h"
#include "utils/IChangeHandler.h"

#include "Configuration.h"

#define MAX_CHANNELS 8
class Analog
{
public:
    static void init();
    static void setRange(uint8_t channel, uint16_t min, uint16_t max);
    inline static void updateVO() { vOct = UINT12_MAX - analogRead(CV_SUSTAIN); }
    inline static float getVO();
    inline static uint16_t getVORaw();
    inline static uint16_t getRaw(uint8_t channel) { return mux->read(channel); }
    inline static uint16_t get(uint8_t channel);
    inline static float getF(uint8_t channel);
    static void assignMultiplexer(MuxChannel multiplexer) { mux = &multiplexer; }

    /// Calibration
    static void calibrate();
    static void setIEventHandler(IChangeHandler *eventHandler) { mEventHandler = reinterpret_cast<void *>(eventHandler); };
    static void printValues();

private:
    inline static uint16_t remap(uint8_t channel, uint16_t value);
    static float convertToVolt(uint16_t value);

    /// Calibration
    static void getZero();
    static void getCalibration();

    static void *mEventHandler;
    static void dispatchEvent();

    static MuxChannel *mux;
    static uint16_t vOct;

    static float cvOffset;

    static uint16_t zeroVal;
    static float zeroValRec;
    static uint16_t calibration[4];
    static uint16_t minVal[8];
    static uint16_t maxVal[8];
};

inline float Analog::getF(uint8_t channel)
{
    float value = 0.0f;
    switch (channel)
    {
    case CV_ATTACK:
    case CV_DECAY:
    case CV_RELEASE:
        value = (float)((get(channel) - zeroVal) * zeroValRec);
        break;

    default:
        value = unwn::float_range_from_uint12_t(get(channel));
        break;
    }
    return (value);
}

inline uint16_t Analog::remap(uint8_t ch, uint16_t value)
{
    return UINT12_MAX - DSY_CLAMP(map(value, minVal[ch], maxVal[ch], 0, UINT12_MAX), 0, UINT12_MAX);
}

inline uint16_t Analog::get(uint8_t channel)
{
    switch (channel)
    {
    case POT_ATTACK:
    case POT_DECAY:
    case POT_SUSTAIN:
    case POT_RELEASE:
    case POT_SHAPE:
        return remap(channel, mux->read(channel));
    default:
        return UINT12_MAX - mux->read(channel);
    }
}

inline float Analog::getVO()
{
    return convertToVolt(vOct);
}

inline uint16_t Analog::getVORaw()
{
    return vOct;
}

#endif // !ANALOG_H