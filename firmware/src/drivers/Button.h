#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>
#include "Pinout.h"
#include "utils/utils.h"
#include "Multiplexer.h"

#include <AceButton.h>
using namespace ace_button;

class MuxButtonConfig : public ButtonConfig
{
public:
    virtual int readButton(uint8_t pin)
    {
        return _mux->muxValues[pin];
    }
    void assignMultiplexer(MuxChannel multiplexer)
    {
        _mux = &multiplexer;
    }
    virtual unsigned long getClock() { return millis(); }

private:
    static MuxChannel *_mux;
};

MuxChannel *MuxButtonConfig::_mux = &Multiplexer::mux[1];

class Button
{
public:
    Button() : // trigIn(&trigConfig),
               // gateIn(&trigConfig),
               trigButton(&muxConfigT),
               stageButton(&muxConfig),
               highToggle(&muxConfigT),
               lowToggle(&muxConfigT)
    {
    }

    ButtonConfig trigConfig;
    MuxButtonConfig muxConfig;
    MuxButtonConfig muxConfigT;

    // AceButton trigIn;
    // AceButton gateIn;

    AceButton trigButton;
    AceButton stageButton;
    AceButton highToggle;
    AceButton lowToggle;

    void init();
    void update();

    bool isShift = false;
};

void Button::init()
{
    // Button uses the built-in pull up register.
    // pinMode(TRIG_SW, INPUT_PULLUP);

    //pinMode(TRIG, INPUT_PULLUP);
    //pinMode(GATE, INPUT_PULLUP);
    // trigIn.init(TRIG, HIGH);
    // gateIn.init(GATE, HIGH);
    trigButton.init(TRIG_SW, HIGH);
    stageButton.init(STAGE_SW, HIGH);
    highToggle.init(TOGGLE_H, HIGH);
    lowToggle.init(TOGGLE_L, HIGH);
    ///
    trigConfig.setDebounceDelay(1);
    muxConfigT.setDebounceDelay(30);
    muxConfig.setDebounceDelay(30);
    muxConfig.setFeature(ButtonConfig::kFeatureLongPress);
    muxConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
    muxConfig.setLongPressDelay(600);
}

void Button::update()
{
    trigButton.check();
    // trigIn.check();
    // gateIn.check();
    stageButton.check();
    highToggle.check();
    lowToggle.check();
}

#endif