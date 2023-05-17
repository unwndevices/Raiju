#include "Multiplexer.h"

void MuxChannel::init(uint8_t muxPin, InputType type)
{
    _muxPin = muxPin;
    _type = type;
    // different mode for analog and digital
    if (_type == InputType::ANALOG_IN)
    {
        analogSetAttenuation(ADC_0db);
        analogReadResolution(12);
        adcAttachPin(_muxPin);
    }
    else
    {
        pinMode(_muxPin, INPUT_PULLUP);
    }
}

uint16_t MuxChannel::read(uint8_t channel)
{
    return muxValues[channel];
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

uint8_t Multiplexer::lastPin = (uint8_t)0;
uint8_t Multiplexer::maxPin = (uint8_t)8;
uint8_t Multiplexer::muxAmount = (uint8_t)0;

uint8_t Multiplexer::_S0 = (uint8_t)S0;
uint8_t Multiplexer::_S1 = (uint8_t)S1;
uint8_t Multiplexer::_S2 = (uint8_t)S2;
uint8_t Multiplexer::v0 = (uint8_t)0;
uint8_t Multiplexer::v1 = (uint8_t)0;
uint8_t Multiplexer::v2 = (uint8_t)0;

MuxChannel Multiplexer::mux[2];

void Multiplexer::init()
{
    pinMode(_S0, OUTPUT);
    pinMode(_S1, OUTPUT);
    pinMode(_S2, OUTPUT);

    digitalWrite(_S0, v0);
    digitalWrite(_S1, v1);
    digitalWrite(_S2, v2);

    mux[0].init(MUX_A, ANALOG_IN);
    mux[1].init(MUX_D, DIGITAL_IN);
    muxAmount = 2;
}

void Multiplexer::updateAll()
{

    if (lastPin)
    {
        v0 = 0;
        v1 = 0;
        v2 = 0;
        lastPin = 0;
        bitSelect(lastPin);
    }

    for (lastPin = 0; lastPin < maxPin; lastPin++)
    {
        update();
        bitSelectNext(lastPin);
        delay(4);
    }
    lastPin = 0;
}

void Multiplexer::update()
{
    uint16_t value = 0;
    for (int i = 0; i < muxAmount; i++)
    {
        if (mux[i]._type == InputType::ANALOG_IN)
            value = analogRead(mux[i]._muxPin);
        else if(mux[i]._type == InputType::DIGITAL_IN)
                value = digitalRead(mux[i]._muxPin);

        if (value != mux[i].muxValues[lastPin])
        {
            mux[i].muxValues[lastPin] = value;
        }
    }
}

void Multiplexer::updateAndNext()
{
    update();
    if (lastPin < maxPin - 1)
        lastPin++;
    else
        lastPin = 0;
    bitSelect(lastPin);
}

void Multiplexer::bitSelect(uint8_t pin)
{
    v0 = bitRead(pin, 0);
    v1 = bitRead(pin, 1);
    v2 = bitRead(pin, 2);
    digitalWrite(_S0, v0);
    digitalWrite(_S1, v1);
    digitalWrite(_S2, v2);
}

void Multiplexer::bitSelectNext(uint8_t pin)
{
    v0 = bitRead((pin + 1), 0);
    v1 = bitRead((pin + 1), 1);
    v2 = bitRead((pin + 1), 2);
    digitalWrite(_S0, v0);
    digitalWrite(_S1, v1);
    digitalWrite(_S2, v2);
}