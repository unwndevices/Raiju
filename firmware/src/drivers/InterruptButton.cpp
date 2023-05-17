#include "InterruptButton.hpp"
InterruptButton *InterruptButton::s_buttons[2] = {nullptr};
uint8_t InterruptButton::s_numButtons = 0;

InterruptButton::InterruptButton(uint8_t pin, callback_t callback, uint8_t mode)
    : m_pin(pin), m_callback(callback), m_mode(mode), m_enabled(false)
{
    if (s_numButtons < MAX_BUTTONS)
    {
        s_buttons[s_numButtons] = this;
        log_d("Button %d registered", s_numButtons);
        id = s_numButtons;
        s_numButtons++;
    }
}

void InterruptButton::Enable()
{
    if (!m_enabled)
    {
        pinMode(m_pin, INPUT_PULLUP);
        if (id == 0)
        {
            attachInterrupt(digitalPinToInterrupt(m_pin), Isr0, m_mode);
        }
        else if (id == 1)
        {
            attachInterrupt(digitalPinToInterrupt(m_pin), Isr1, m_mode);
        }
        m_enabled = true;
    }
}

void InterruptButton::Disable()
{
    if (m_enabled)
    {
        detachInterrupt(digitalPinToInterrupt(m_pin));
        m_enabled = false;
    }
}

void InterruptButton::Isr0()
{
    if (digitalRead(s_buttons[0]->m_pin) == LOW)
    {
        s_buttons[0]->m_callback(true);
    }
    else if (digitalRead(s_buttons[0]->m_pin) == HIGH)
    {
        s_buttons[0]->m_callback(false);
    }
}
void InterruptButton::Isr1()
{
    if (digitalRead(s_buttons[1]->m_pin) == LOW)
    {
        s_buttons[1]->m_callback(true);
    }
    else if (digitalRead(s_buttons[1]->m_pin) == HIGH)
    {
        s_buttons[1]->m_callback(false);
    }
}