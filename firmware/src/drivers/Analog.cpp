#include "Analog.h"
MuxChannel *Analog::mux = &Multiplexer::mux[0];
uint16_t Analog::vOct = 0.0f;

float Analog::cvOffset = 0.22f;
uint16_t Analog::minVal[8] = {};
uint16_t Analog::maxVal[8] = {};
uint16_t Analog::zeroVal = 0;
float Analog::zeroValRec = 0.0f;

uint16_t Analog::calibration[4] = {};
void *Analog::mEventHandler = nullptr;

void Analog::init()
{
    dacWrite(OFFSET_CV, 37); /// ~1.78V
    adcAttachPin(CV_SUSTAIN);
    getCalibration();
    getZero();
}

void Analog::setRange(uint8_t channel, uint16_t min, uint16_t max)
{
    minVal[channel] = min;
    maxVal[channel] = max;
}

void Analog::calibrate()
{
    dispatchEvent();
}

void Analog::getCalibration()
{

    Configuration *pConfig = Configuration::getInstance();
    if (!pConfig->mCalibrationStruct.isCalibrated)
    {
        calibrate();
    }
    for (int i = 0; i < 4; i++)
    {
        calibration[i] = pConfig->mCalibrationStruct.calibration[i];
        // Serial.println(calibration[i]);
    }
    // Serial.println(" ");
    for (int i = 0; i < 8; i++)
    {
        minVal[i] = pConfig->mCalibrationStruct.minVal[i];
        maxVal[i] = pConfig->mCalibrationStruct.maxVal[i];
        // Serial.println(minVal[i]);
        // Serial.println(maxVal[i]);
    }
}

/// do after getCalibration
void Analog::getZero()
{
    zeroVal = calibration[1];
    zeroValRec = (float)(1.0f / zeroVal);
}

float Analog::convertToVolt(uint16_t value)
{
    float volts = 0.0f;
    if (value < calibration[0]) /// -5v < val < -3v
    {
        volts = unwn::lerp_int_to_float(value, -5.0f, -3.0f, 0, calibration[0]);
    }
    else if (value >= calibration[0] && value < calibration[1])
    {
        volts = unwn::lerp_int_to_float(value, -3.0f, 0.0f, calibration[0], calibration[1]);
    }
    else if (value >= calibration[1] && value < calibration[2])
    {
        volts = unwn::lerp_int_to_float(value, 0.0f, 3.0f, calibration[1], calibration[2]);
    }
    else
    {
        volts = unwn::lerp_int_to_float(value, 3.0f, 5.0f, calibration[2], calibration[3]);
    }
    return volts;
}

void Analog::dispatchEvent()
{
    IChangeHandler *eventHandler = reinterpret_cast<IChangeHandler *>(mEventHandler);
    eventHandler->handleCalibrationEvent();
}

void Analog::printValues()
{
    // Serial.println("Analog IN Values:");
    for (int i = 0; i < 8; i++)
    {
        if (i == 0 || i == 2 || i == 4 || i == 5 || i == 7)
        {
        }
        else
        {
            Serial.print(getRaw(i));
            Serial.print("  ");
        }
    }
    Serial.println("");
}