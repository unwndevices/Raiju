#ifndef AUDIO_H
#define AUDIO_H
#include <Arduino.h>
#include "drivers/Pinout.h"
#include "i2s/I2SOutput.h"
#include "synthesis/Voice.h"
#include "utils/utils.h"

#define SAMPLERATE 39100.0f

i2s_pin_config_t i2sPins = {
    .bck_io_num = DAC_BCLK,  // The bit clock connectiom, goes to pin 27 of ESP32
    .ws_io_num = DAC_LRCLK,  // Word select, also known as word select or left right clock
    .data_out_num = DAC_DIN, // Data out from the ESP32, connect to DIN on 38357A
    .data_in_num = -1        // we are not interested in I2S data into the ESP32
};

class Audio
{
public:
    static I2SOutput output;
    static Voice *mVoice;
    static void init();
};

I2SOutput Audio::output;
Voice *Audio::mVoice = nullptr;

void Audio::init()
{
    mVoice = new Voice;
    mVoice->init(SAMPLERATE);
    output.start(I2S_NUM_0, i2sPins, mVoice);
}
#endif // !AUDIO_H