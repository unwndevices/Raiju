#include "Led.h"
#include "Led_LUT.h"

uint8_t Led::ledAmount = 0;
uint16_t Leds::maxBrightness = 1024;

Led::Led(uint8_t pin)
{
    uint8_t _pin = pin;
    _ledId = ledAmount; // used to assign a different pwm channel to each led
    ++ledAmount;
#ifdef DEBUG
    if (pin == LED_TRIG)
    {
        return;
    }
#endif
    pinMode(_pin, OUTPUT);
    ledcAttachPin(_pin, _ledId);
    ledcSetup(_ledId, 4000, BIT_PWM);
}

void Led::set(uint16_t value)
{
    baseValue = constrain(value, 0, Leds::maxBrightness);
    // write(baseValue);
}

void Led::set(float value)
{
    baseValue = (uint16_t)(constrain(value, 0.0f, 1.0f) * Leds::maxBrightness);
}

void Led::set(uint16_t value, uint16_t time)
{
    holdValue = constrain(value, 0, Leds::maxBrightness);
    holdTime = time;
    isHold = true;
}

void Led::set(float value, uint16_t time)
{
    holdValue = (uint16_t)(constrain(value, 0.0f, 1.0f) * Leds::maxBrightness);
    holdTime = time;
    isHold = true;
}

void Led::write(uint16_t value)
{
    ledcWrite(_ledId, brightnessLut[value]);
}

void Led::directWrite(uint16_t value)
{
#ifdef DEBUG
    log_d("Triggered");
    return;
#endif
    ledcWrite(_ledId, value);
}
uint16_t Led::read()
{
    return ledcRead(_ledId);
}

void Led::update()
{
    if (isHold && !holdTimeStart)
    {
        holdTimeStart = millis();
        write(holdValue);
    }
    else if (isHold && holdTimeStart)
    {
        if (holdTime <= millis() - holdTimeStart)
        {
            isHold = false;
            holdTimeStart = 0;
            write(baseValue);
        }
    }

    else if (!isHold)
    {
        if (baseValue != read())
            write(baseValue);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

Led Leds::stages[4]{LED_ATTACK,
                    LED_DECAY,
                    LED_SUSTAIN,
                    LED_RELEASE};
Led Leds::trig(LED_TRIG);

void Leds::init()
{
    xTaskCreatePinnedToCore(this->task, "Leds", 2024, this, LOW_PRIORITY, &taskLed, 0); /// pinned on core 0
}

void Leds::setStageLeds(uint16_t value)
{
    for (int i = 0; i < 4; i++)
    {
        stages[i].set(value);
    }
}
void Leds::setStageLeds(uint16_t value, uint16_t time)
{
    for (int i = 0; i < 4; i++)
    {
        stages[i].set(value, time);
    }
}

void Leds::task(void *handle)
{
    while (1)
    {
        Leds *pThis = (Leds *)handle;
        for (int i = 0; i < 4; i++)
        {
            pThis->stages[i].update();
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // TODO test delay values
    }
}