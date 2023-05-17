#ifndef DIGITALOUT_H
#define DIGITALOUT_h
#include <Arduino.h>
#include "utils/utils.h"

class DigitalOut
{
public:
    void init(uint8_t pin);

    void set(bool value);
    void directWrite(bool value);
    void update();
    static void task(void *handle);

private:
    volatile uint8_t _pin;
    volatile bool _bool;
    volatile uint16_t holdTime = 5;
    volatile bool isHold = false;
    volatile ullong holdTimeStart = 0;

    void write(bool value);
    bool read();
    TaskHandle_t taskDOut;
};

void DigitalOut::directWrite(bool value)
{
    isHold = true;
    holdTimeStart = millis();
    write(value);
}
void DigitalOut::write(bool value)
{
    if (value)
        GPIO.out_w1tc = (1 << _pin);
    else
        GPIO.out_w1ts = (1 << _pin);
}

void DigitalOut::init(uint8_t pin)
{
    _pin = pin;
    _bool = false;
    pinMode(_pin, OUTPUT);
    write(_bool);
    xTaskCreatePinnedToCore(this->task, "DOut", 1024, this, 5, &taskDOut, 0); /// pinned on core 0
}
void DigitalOut::set(bool value)
{
    _bool = value;
    isHold = true;
}

void DigitalOut::update()
{
    if (isHold && !holdTimeStart)
    {
        holdTimeStart = millis();
        write(_bool);
    }
    else if (isHold && holdTimeStart)
    {
        if (holdTime <= millis() - holdTimeStart)
        {
            isHold = false;
            holdTimeStart = 0;
            write(false);
        }
    }

    else if (!isHold)
    {
        write(false);
    }
}

void DigitalOut::task(void *handle)
{
    while (1)
    {
        DigitalOut *pThis = (DigitalOut *)handle;
        for (int i = 0; i < 4; i++)
        {
            pThis->update();
        }
        vTaskDelay(pdMS_TO_TICKS(2)); // TODO test delay values
    }
}

#endif // !DIGITALOUT_H