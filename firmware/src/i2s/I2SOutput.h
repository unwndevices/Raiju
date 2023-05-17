#ifndef __sampler_base_h__
#define __sampler_base_h__

#include <Arduino.h>
#include "driver/i2s.h"
#include "synthesis/Voice.h"
#include "utils/utils.h"
#include "utils/IChangeHandler.h"

class SampleSource;

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SOutput
{
private:
    static TaskHandle_t m_i2sWriterTaskHandle;
    static QueueHandle_t m_i2sQueue;
    static i2s_port_t m_i2sPort;

    static Voice *m_sample_generator;
    static void *mEventHandler;
    static void dispatchEvent();

public:
    static void start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, Voice *sample_generator);
    static void setIEventHandler(IChangeHandler *eventHandler) { mEventHandler = reinterpret_cast<void *>(eventHandler); };
    friend void i2sWriterTask(void *param);
};

#endif