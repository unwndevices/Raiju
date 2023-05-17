#ifndef _PARAMETER_QUEUE
#define _PARAMETER_QUEUE
#include <Arduino.h>

#define ARDUINOJSON_USE_LONG_LONG 0
#define ARDUINOJSON_USE_DOUBLE 0
#include <ArduinoJson.h>
#include "FS.h"
#include "SPIFFS.h"

#define VERSION 100

class Parameter;

class Parameter
{
public:
    float value;
    float cv = 0.0f;
    bool hasChanged = true;
    bool isLocked = true;
};

#define INTERFACE_AMT 10

enum paramType
{
    ATTACK = 0,
    DECAY,
    SUSTAIN,
    RELEASE,
    SHAPE,
    COLOR,
    FREQ,
    PITCHENV,
    ENVSHAPE,
    ENVOFFSET
};

struct ParamStruct
{
    Parameter interface[INTERFACE_AMT];
};

struct ConfigStruct
{
    /// Calibration Configuration
    volatile bool id;
    volatile int version;

    volatile bool default_page;
    volatile int led_brightness;
    volatile bool cv_routing[4];

    String ssid;
    String password;
    String ip;

    volatile bool hasChanged;
    volatile uint8_t stageOut;

    volatile bool modShape;
    volatile float modRatio;
    volatile float pEnvSemitones;
    volatile float pEnvDecay;
};

struct CalibrationStruct
{
    volatile bool isCalibrated;
    volatile uint16_t calibration[4];
    volatile uint16_t minVal[8];
    volatile uint16_t maxVal[8];
    volatile uint8_t envOffset;
};

class Configuration
{
public:
    static ParamStruct mParamStruct;
    static ConfigStruct mConfigStruct;
    static CalibrationStruct mCalibrationStruct;
    static Configuration *instance;
    static Configuration *getInstance();

    static void init();

    static void loadParameters();
    static void saveParametersToFile();

    static void loadCalibration();
    static void saveCalibration(String obj);
    static void saveCalibrationToFile();

    static void loadConfiguration();
    static void saveConfiguration(String obj);
    static void saveConfigurationToFile();

    static void printParameters();
    static void printConfiguration();
    static void merge(JsonVariant dst, JsonVariantConst src);

    static void lock(bool lock);
    static void lock(uint8_t channel, bool lock);
    static void hasChanged(bool change);

    static QueueHandle_t parameter_queue;

    static const char *configuration;
    static const char *parameters;
    static const char *calibration;
};

#endif // !PARAMETER_QUEUE