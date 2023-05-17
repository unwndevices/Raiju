#ifndef INTERRUPT_BUTTON_HPP
#define INTERRUPT_BUTTON_HPP

#include <Arduino.h>
#include "Pinout.h"
#include "utils/utils.h"

#define MAX_BUTTONS 2

class InterruptButton
{
public:
    typedef void (*callback_t)(bool);

    InterruptButton(){};
    InterruptButton(uint8_t pin, callback_t callback, uint8_t mode = CHANGE);
    void Enable();
    void Disable();

private:
    static void Isr0();
    static void Isr1();

    uint8_t id;
    uint8_t m_pin;
    callback_t m_callback;
    uint8_t m_mode;
    bool m_enabled;

    static InterruptButton *s_buttons[MAX_BUTTONS];
    static uint8_t s_numButtons;
};

#endif // !INTERRUPT_BUTTON_HPP