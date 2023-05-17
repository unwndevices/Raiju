#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H
#include <Arduino.h>
#include "drivers/Pinout.h"


enum InputType
{
    DIGITAL_IN = 0,
    ANALOG_IN
};

class MuxChannel
{
public:
    void init(uint8_t muxPin, InputType type);
    uint16_t read(uint8_t channel);
    uint16_t muxValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t _type;
    uint8_t _muxPin;
};

class Multiplexer
{
public:
    Multiplexer();

    static void init();
    static void update();
    static void updateAndNext();
    static void updateAll();

    static MuxChannel mux[2];

private:
    static uint8_t lastPin;
    static uint8_t maxPin;
    static uint8_t muxAmount;

    static void bitSelect(uint8_t pin);
    static void bitSelectNext(uint8_t pin); // useful to allow the pins to set before reading the this->value

    static uint8_t _S0;
    static uint8_t _S1;
    static uint8_t _S2;
    static uint8_t v0;
    static uint8_t v1;
    static uint8_t v2;
};

#endif