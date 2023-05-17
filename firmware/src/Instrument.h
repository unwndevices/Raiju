#ifndef INSTRUMENT_H
#define INSTRUMENT_H
#include "drivers/Drivers.h"
#include "utils/utils.h"
#include "utils/IChangeHandler.h"

#include "Configuration.h"

// TODO this is ugly
void trig(bool state);
void gate(bool state);

enum interfaceMode
{
    ENVELOPE_MODE = 0,
    VOICE_MODE
};

class Instrument : public IEventHandler, public IChangeHandler
{
public:
    Instrument() : triggerIn(TRIG, trig),
                   gateIn(GATE, gate){};

    void begin();
    void update();                                                                        // TODO switch to tasks
    void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState) override; /// button scenarios
    void handleChangeEvent() override;
    void handleCalibrationEvent() override;

    static Leds leds;

private:
    void setMode(interfaceMode mode);
    void setLedMode(interfaceMode mode);
    void invertMode();
    void paramUpdate(Parameter *_par, uint8_t input);
    void cvUpdate(Parameter *_par, uint8_t input);
    void interfaceUpdate();

    bool getRouting(uint8_t index);

    void flashLed(uint8_t input);
    uint8_t currentEnvState = 0;

    interfaceMode currentMode = ENVELOPE_MODE;

    static float lockThreshold;
    static float threshold;
    Analog analog;
    InterruptButton triggerIn, gateIn;
    static Button buttons;
    static DigitalOut stageOut;
    Configuration *pConfig = nullptr;
};

Leds Instrument::leds;
Button Instrument::buttons;
DigitalOut Instrument::stageOut;
float Instrument::lockThreshold = 0.15f;
// float Instrument::threshold = 0.001f;

void Instrument::begin()
{
    log_d("Leds Init");
    leds.init();
    leds.stages[0].set(LED_ON, 300);
    pConfig = Configuration::getInstance();
    pConfig->init();
    pConfig->printConfiguration();
    log_d("Audio Init");
    leds.stages[1].set(LED_ON, 300);
    Audio::init();
    Audio::output.setIEventHandler(this);

    /// Do before buttons and analog initialization
    leds.stages[2].set(LED_ON, 300);
    log_d("Mux Init");
    Multiplexer::init();
    /// Interrupt Buttons
    log_d("Interrupt Buttons Init");
    triggerIn.Enable();
    gateIn.Enable();

    /// Buttons
    log_d("Buttons Init");
    buttons.init();
    buttons.trigConfig.setIEventHandler(this);
    buttons.muxConfigT.setIEventHandler(this);
    buttons.muxConfig.setIEventHandler(this);
    /// Analog Input
    log_d("Analog Init");
    leds.stages[3].set(LED_ON, 300);
    analog.setIEventHandler(this);
    analog.init();
    log_d("Stage out Init");
    stageOut.init(STAGE_OUT);
    Multiplexer::updateAll();
    leds.setStageLeds(LED_ON, 750);
    setMode((interfaceMode)pConfig->mConfigStruct.default_page);

    // TODO momentarily called here.
    Audio::mVoice->mOscillator.SetRatio(pConfig->mConfigStruct.modRatio);
    Audio::mVoice->mOscillator.SetModWaveform(pConfig->mConfigStruct.modShape);
    Audio::mVoice->SetEnvRatio(pConfig->mConfigStruct.pEnvDecay);
    Audio::mVoice->SetEnvSemi(pConfig->mConfigStruct.pEnvSemitones);
}

void Instrument::update()
{
    Multiplexer::updateAndNext();
    buttons.update();
    analog.updateVO();
    interfaceUpdate();
}

void Instrument::paramUpdate(Parameter *_par, uint8_t input)
{
    float _temp = analog.getF(input);
    if (input == POT_SHAPE && _par->isLocked && lockThreshold > abs(_par->value - _temp))
    {
        _par->isLocked = false;
        flashLed(input);
    }
    else if (_par->isLocked && lockThreshold > abs(_par->value - _temp))
    {
        _par->isLocked = false;
        flashLed(input);
    }

    if (!(_par->isLocked) && _par->value != _temp)
    {
        _par->value = _temp;
        _par->hasChanged = true;
    }
}

void Instrument::flashLed(uint8_t input)
{

    switch (input)
    {
    case POT_ATTACK:
        input = 0;
        break;
    case POT_DECAY:
        input = 1;
        break;
    case POT_SUSTAIN:
        input = 2;
        break;
    case POT_RELEASE:
        input = 3;
        break;
    case POT_SHAPE:
        leds.setStageLeds(LED_ON, 300);

        return;
    default:
        break;
    }
    leds.stages[input].set(LED_ON, 300);
}

void Instrument::cvUpdate(Parameter *_par, uint8_t input)
{
    if (input == CV_SUSTAIN)
    {
        _par->cv = analog.getVO();
    }
    else
    {
        _par->cv = analog.getF(input);
    }
    _par->hasChanged = true;
}

bool Instrument::getRouting(uint8_t index)
{
    return !pConfig->mConfigStruct.cv_routing[index];
}

void Instrument::interfaceUpdate()
{
    if (pConfig->mConfigStruct.hasChanged)
    {
        pConfig->loadConfiguration();
        Audio::mVoice->mOscillator.SetRatio(pConfig->mConfigStruct.modRatio);
        Audio::mVoice->mOscillator.SetModWaveform(pConfig->mConfigStruct.modShape);
        Audio::mVoice->SetEnvRatio(pConfig->mConfigStruct.pEnvDecay);
        Audio::mVoice->SetEnvSemi(pConfig->mConfigStruct.pEnvSemitones);
        pConfig->saveParametersToFile();
        pConfig->mConfigStruct.hasChanged = false;
    }
    if (!currentMode)
    {
        paramUpdate(&pConfig->mParamStruct.interface[0], POT_ATTACK);
        paramUpdate(&pConfig->mParamStruct.interface[1], POT_DECAY);
        paramUpdate(&pConfig->mParamStruct.interface[2], POT_SUSTAIN);
        paramUpdate(&pConfig->mParamStruct.interface[3], POT_RELEASE);
        paramUpdate(&pConfig->mParamStruct.interface[8], POT_SHAPE);
    }
    else
    {
        paramUpdate(&pConfig->mParamStruct.interface[4], POT_ATTACK);
        paramUpdate(&pConfig->mParamStruct.interface[5], POT_DECAY);
        paramUpdate(&pConfig->mParamStruct.interface[6], POT_SUSTAIN);
        paramUpdate(&pConfig->mParamStruct.interface[7], POT_RELEASE);
    }
    if (buttons.isShift)
    {
        paramUpdate(&pConfig->mParamStruct.interface[9], POT_SHAPE);
    }

    if (getRouting(0))
        cvUpdate(&pConfig->mParamStruct.interface[0], CV_ATTACK);
    else
        cvUpdate(&pConfig->mParamStruct.interface[4], CV_ATTACK);
    if (getRouting(1))
        cvUpdate(&pConfig->mParamStruct.interface[1], CV_DECAY);
    else
        cvUpdate(&pConfig->mParamStruct.interface[5], CV_DECAY);
    if (getRouting(2))
        cvUpdate(&pConfig->mParamStruct.interface[2], CV_SUSTAIN);
    else
        cvUpdate(&pConfig->mParamStruct.interface[6], CV_SUSTAIN);
    if (getRouting(3))
        cvUpdate(&pConfig->mParamStruct.interface[3], CV_RELEASE);
    else
        cvUpdate(&pConfig->mParamStruct.interface[7], CV_RELEASE);

    xQueueSend(Configuration::parameter_queue, &pConfig, 50);
}

void Instrument::handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    uint8_t id = button->getPin();
    log_d("ID: %d", id);
    log_d("TYPE: %d", eventType);
    switch (eventType)
    {
    case AceButton::kEventPressed:
        if (id == GATE || (id == TRIG_SW && !buttons.isShift))
        {
            Audio::mVoice->mEnvelope.Gate(true);
            Audio::mVoice->mPitchEnvelope.Trigger();
        }
        else if (id == TRIG)
        {
            Audio::mVoice->mEnvelope.Trig();
            Audio::mVoice->mPitchEnvelope.Trigger();
        }
        else if (id == TOGGLE_H)
            Audio::mVoice->mEnvelope.setMultiplier(ADSREnv::timeMult::HIGH_M);

        else if (id == TOGGLE_L)
        {
            Audio::mVoice->mEnvelope.setMultiplier(ADSREnv::timeMult::LOW_M);
            pConfig->saveParametersToFile();
        }
        if (id == TRIG || (id == TRIG_SW && !buttons.isShift) || id == GATE)
            leds.trig.directWrite(LED_ON);

        if ((id == TRIG_SW) && buttons.isShift)
            invertMode();
        break;

    case AceButton::kEventReleased:
        if (id == GATE || id == TRIG_SW)
            Audio::mVoice->mEnvelope.Gate(false);
        else if (id == TOGGLE_H || id == TOGGLE_L)
        {
            Audio::mVoice->mEnvelope.setMultiplier(ADSREnv::timeMult::MID_M);
        }

        if (id == GATE || id == TRIG || id == TRIG_SW)
            leds.trig.directWrite(LED_OFF);
        if (id == STAGE_SW)
        {
            if (pConfig->mConfigStruct.stageOut < 4)
            {
                leds.stages[(uint8_t)pConfig->mConfigStruct.stageOut].set(LED_ON, 200);
                pConfig->mConfigStruct.stageOut++;
            }
            else
            {
                pConfig->mConfigStruct.stageOut = 0;
                leds.setStageLeds(LED_ON, 200);
            }
        }
        break;
    case AceButton::kEventLongPressed:
        if (id == STAGE_SW)
        {
            invertMode();
            buttons.isShift = true;
        }
        break;
    case AceButton::kEventLongReleased:
        if (id == STAGE_SW)
        {
            invertMode();
            buttons.isShift = false;
        }
        break;
    }
}

void Instrument::handleChangeEvent()
{
    envState stage = (envState)pConfig->mConfigStruct.stageOut;
    if (stage != 0)
    {
        log_d("stage reached");
        if (Audio::mVoice->mEnvelope.ProcessStageEnd(stage))
        {

            stageOut.directWrite(true);
            leds.stages[(uint8_t)stage - 1].set(LED_ON, 50);
        }
    }
    else
    {
        envState currentStage = Audio::mVoice->mEnvelope.ProcessStageEnd();
        if (currentStage != env_idle)
        {
            stageOut.directWrite(true);
            leds.stages[(uint8_t)currentStage - 1].set(LED_ON, 50);
        }
    }
}

void Instrument::invertMode()
{
    setMode((interfaceMode)!currentMode);
}

void Instrument::setMode(interfaceMode mode)
{
    currentMode = mode;
    log_d("locking all Configuration");
    pConfig->lock(true);
    setLedMode(mode);
}

void Instrument::setLedMode(interfaceMode mode)
{
    if (mode)
    {
        leds.setStageLeds(150);
    }
    else
    {
        leds.setStageLeds(LED_OFF);
    }
}

void Instrument::handleCalibrationEvent()
{
    uint8_t phase = 0;
    uint16_t buffer[8];
    uint16_t averageValue;
    uint16_t num_elements = sizeof(buffer) / sizeof(buffer[0]);
    /// start calibration, 0V
    leds.stages[phase].set(LED_ON);
    Serial.println("-3V Calibration");
    while (phase == 0)
    {
        averageValue = 0;
        for (int i = 0; i < 8; i++)
        {
            analog.updateVO();
            buffer[i] = analog.getVORaw();
            averageValue += buffer[i];
        }
        averageValue = averageValue / num_elements;

        if (!digitalRead(MUX_D))
        {
            pConfig->mCalibrationStruct.calibration[phase] = averageValue;
            delay(400);
            leds.stages[phase].set(LED_OFF);
            phase++;
        }
    }

    Serial.println("0V Calibration");
    leds.stages[phase].set(LED_ON);
    while (phase == 1)
    {
        for (int i = 0; i < 8; i++)
        {
            analog.updateVO();
            buffer[i] = analog.getVORaw();
            averageValue += buffer[i];
        }
        averageValue = averageValue / num_elements;

        if (!digitalRead(MUX_D))
        {
            pConfig->mCalibrationStruct.calibration[phase] = UINT12_MAX - analogRead(CV_SUSTAIN);
            delay(400);
            leds.stages[phase].set(LED_OFF);
            phase++;
        }
    }

    Serial.println("3V Calibration");
    leds.stages[phase].set(LED_ON);
    while (phase == 2)
    {
        for (int i = 0; i < 8; i++)
        {
            analog.updateVO();
            buffer[i] = analog.getVORaw();
            averageValue += buffer[i];
        }
        averageValue = averageValue / num_elements;

        if (!digitalRead(MUX_D))
        {
            pConfig->mCalibrationStruct.calibration[phase] = UINT12_MAX - analogRead(CV_SUSTAIN);
            delay(400);
            leds.stages[phase].set(LED_OFF);
            phase++;
        }
    }

    Serial.println("5V Calibration");
    leds.stages[phase].set(LED_ON);
    while (phase == 3)
    {
        for (int i = 0; i < 8; i++)
        {
            analog.updateVO();
            buffer[i] = analog.getVORaw();
            averageValue += buffer[i];
        }
        averageValue = averageValue / num_elements;

        if (!digitalRead(MUX_D))
        {

            pConfig->mCalibrationStruct.calibration[phase] = UINT12_MAX - analogRead(CV_SUSTAIN);
            delay(400);
            leds.stages[phase].set(LED_OFF);
            phase++;
        }
    }

    leds.setStageLeds(300);
    Serial.println("Min Calibration");
    while (phase == 4)
    {
        Multiplexer::updateAll();
        if (!digitalRead(MUX_D))
        {
            for (int i = 0; i < 8; i++)
            {
                pConfig->mCalibrationStruct.maxVal[i] = analog.getRaw(i) - 4;
            }
            phase++;
            delay(400);
        }
    }

    leds.setStageLeds(LED_ON);
    Serial.println("Max Calibration");
    while (phase == 5)
    {
        Multiplexer::updateAll();
        if (!digitalRead(MUX_D))
        {
            for (int i = 0; i < 8; i++)
            {
                pConfig->mCalibrationStruct.minVal[i] = analog.getRaw(i) + 4;
            }
            phase++;
            delay(400);
        }
    }

    leds.setStageLeds(600);
    Serial.println("Envelope Offset");
    uint8_t env_offset = 0;
    while (phase == 6)
    {
        Multiplexer::updateAll();
        env_offset = 50 + (uint8_t)(analog.getRaw(POT_SHAPE) / 30);
        Serial.println(env_offset);
        dacWrite(OFFSET_ENV, env_offset);
        delay(50);
        if (!digitalRead(MUX_D))
        {
            pConfig->mCalibrationStruct.envOffset = env_offset;
            phase++;
            delay(400);
        }
    }

    leds.setStageLeds(300);
    while (phase == 7)
    {
        Multiplexer::updateAll();
        /// print cv pin values
        analog.printValues();
        if (!digitalRead(MUX_D))
        {
            phase++;
        }
    }

    pConfig->mCalibrationStruct.isCalibrated = true;
    pConfig->saveCalibrationToFile();
}

void trig(bool state)
{
    if (state)
    {
        Audio::mVoice->mEnvelope.Trig();
        Audio::mVoice->mPitchEnvelope.Trigger();
        Instrument::leds.trig.directWrite(LED_ON);
    }
    else
    {
        Instrument::leds.trig.directWrite(LED_OFF);
    }
}

void gate(bool state)
{
    if (state)
    {
        Audio::mVoice->mEnvelope.Gate(true);
        Audio::mVoice->mPitchEnvelope.Trigger();
        Instrument::leds.trig.directWrite(LED_ON);
    }
    else
    {
        Audio::mVoice->mEnvelope.Gate(false);
        Instrument::leds.trig.directWrite(LED_OFF);
    }
}
#endif // !INSTRUMENT_H