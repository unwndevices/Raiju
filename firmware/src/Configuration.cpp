#include "Configuration.h"
#include "drivers/Pinout.h"

QueueHandle_t Configuration::parameter_queue;

ParamStruct Configuration::mParamStruct;
ConfigStruct Configuration::mConfigStruct;
CalibrationStruct Configuration::mCalibrationStruct;
Configuration *Configuration::instance = nullptr;

const char *Configuration::configuration = "/configuration.json";
const char *Configuration::parameters = "/parameters.json";
const char *Configuration::calibration = "/calibration.json";

Configuration *Configuration::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Configuration();
    }
    return instance;
}

void Configuration::init()
{
    SPIFFS.begin();
    parameter_queue = xQueueCreate(1, sizeof(Configuration *));
    if (parameter_queue == NULL)
    {
        log_d("Error initializing the queue");
    }

    mParamStruct.interface[ATTACK].value = 0.01f;
    mParamStruct.interface[DECAY].value = 0.01f;
    mParamStruct.interface[SUSTAIN].value = 0.01f;
    mParamStruct.interface[RELEASE].value = 0.01f;
    mParamStruct.interface[SHAPE].value = 0.0f;
    mParamStruct.interface[COLOR].value = 0.0f;
    mParamStruct.interface[FREQ].value = 0.4f;
    mParamStruct.interface[PITCHENV].value = 0.0f;
    mParamStruct.interface[ENVSHAPE].value = 1.0f;
    mParamStruct.interface[ENVOFFSET].value = 0.0f;
    for (int i = 0; i < 4; i++)
    {
        mConfigStruct.cv_routing[i] = false;
    }
    mConfigStruct.hasChanged = false;
    loadConfiguration();
    loadParameters();
    loadCalibration();
}

void Configuration::lock(bool lock)
{
    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        mParamStruct.interface[i].isLocked = lock;
    }
}
void Configuration::lock(uint8_t channel, bool lock)
{
    mParamStruct.interface[channel].isLocked = lock;
}
void Configuration::hasChanged(bool change)
{
    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        mParamStruct.interface[i].hasChanged = change;
    }
}

void Configuration::loadConfiguration()
{
    File file = SPIFFS.open(configuration);
    // Allocate a temporary JsonDocument
    StaticJsonDocument<1024> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        log_d("Failed to read file, using default configuration");
    }

    // Copy values from the JsonDocument to the params
    /// ID
    mConfigStruct.id = doc["id"];
    /// Version
    mConfigStruct.version = doc["version"];
    /// Panel Configuration
    mConfigStruct.default_page = doc["default_page"];
    mConfigStruct.led_brightness = doc["led_brightness"];
    // mConfigStruct.stageOut = doc["stage_out"];
    /// Voice Configuration
    mConfigStruct.modShape = doc["mod_shape"];
    mConfigStruct.modRatio = doc["mod_ratio"];
    mConfigStruct.pEnvSemitones = doc["penv_semitones"];
    mConfigStruct.pEnvDecay = doc["penv_decay"];

    /// Routing Configuration
    for (int i = 0; i < 4; i++)
    {
        mConfigStruct.cv_routing[i] = doc["cv_routing"][i];
    }
    /// Wifi Configuration
    mConfigStruct.ssid = doc["ssid"].as<String>();
    mConfigStruct.password = doc["password"].as<String>();
    mConfigStruct.ip = doc["ip"].as<String>();

    // check the version number after loading the config
    const int newSoftwareVersion = VERSION; // replace this with your new software version
    if (mConfigStruct.version < newSoftwareVersion)
    {
        Serial.println("Updating configuration to new version");
        mConfigStruct.version = newSoftwareVersion;

        file.close();
        File file = SPIFFS.open(calibration);
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            log_d("Failed to read file");
        }

        // check for missing parameters and add default values
        if (!doc.containsKey("env_offset"))
        {
            Serial.println("env_offset missing, adding default value");

            mCalibrationStruct.envOffset = 132; // added in version 100
        }
        else
        {
            Serial.println("env_offset already present");
        }
        file.close();
        // save updated configuration
        saveConfigurationToFile();
        saveCalibrationToFile();
    }
}

void Configuration::saveConfiguration(String obj)
{
    StaticJsonDocument<850> newData, oldData;
    deserializeJson(newData, obj);
    File prevFile = SPIFFS.open(configuration, FILE_READ);
    deserializeJson(oldData, prevFile);
    merge(oldData, newData);

    SPIFFS.remove(configuration);
    File file = SPIFFS.open(configuration, FILE_WRITE);
    // Serialize JSON to file
    if (serializeJson(oldData, file) == 0)
    {
        log_d("Failed to write to file");
    }
    // Close the file
    file.close();
}
void Configuration::saveCalibration(String obj)
{
    StaticJsonDocument<750> newData, oldData;
    deserializeJson(newData, obj);
    File prevFile = SPIFFS.open(calibration, FILE_READ);
    deserializeJson(oldData, prevFile);
    merge(oldData, newData);

    SPIFFS.remove(calibration);
    File file = SPIFFS.open(calibration, FILE_WRITE);
    // Serialize JSON to file
    if (serializeJson(oldData, file) == 0)
    {
        log_d("Failed to write to file");
    }
    // Close the file
    file.close();
    printConfiguration();
}

void Configuration::loadParameters()
{
    File file = SPIFFS.open(parameters);
    // Allocate a temporary JsonDocument
    StaticJsonDocument<1024> doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        log_d("Failed to read file, using default configuration");
    // Copy values from the JsonDocument to the params
    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        mParamStruct.interface[i].value = doc["parameters"][i] | 0.01;
    }
}

void Configuration::saveParametersToFile()
{
    SPIFFS.remove(parameters);
    File file = SPIFFS.open(parameters, FILE_WRITE);
    StaticJsonDocument<1024> doc;

    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        doc["parameters"][i] = mParamStruct.interface[i].value;
    }

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        log_d("Failed to write to file");
    }

    // Close the file
    file.close();
}

void Configuration::saveConfigurationToFile()
{
    SPIFFS.remove(configuration);
    File file = SPIFFS.open(configuration, FILE_WRITE);
    StaticJsonDocument<1024> doc;

    doc["id"] = mConfigStruct.id;
    doc["version"] = mConfigStruct.version;
    doc["default_page"] = mConfigStruct.default_page;
    doc["led_brightness"] = mConfigStruct.led_brightness;
    for (int i = 0; i < 4; i++)
    {
        doc["cv_routing"][i] = mConfigStruct.cv_routing[i];
    }
    doc["stage_out"] = mConfigStruct.stageOut;
    doc["mod_shape"] = mConfigStruct.modShape;
    doc["mod_ratio"] = mConfigStruct.modRatio;
    doc["penv_semitones"] = mConfigStruct.pEnvSemitones;
    doc["penv_decay"] = mConfigStruct.pEnvDecay;
    doc["ssid"] = mConfigStruct.ssid;
    doc["password"] = mConfigStruct.password;
    doc["ip"] = mConfigStruct.ip;
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        log_d("Failed to write to file");
    }

    // Close the file
    file.close();
}

void Configuration::printParameters()
{
    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        Serial.print(mParamStruct.interface[i].value);
        Serial.print(" ");
    }
    Serial.println();
    Serial.println();
    for (int i = 0; i < INTERFACE_AMT; i++)
    {
        Serial.print(mParamStruct.interface[i].cv);
        Serial.print(" ");
    }
    Serial.println();
}

void Configuration::printConfiguration()
{
    // Open file for reading
    File file = SPIFFS.open(parameters);
    if (!file)
    {
        log_d("Failed to read file");
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        Serial.print((char)file.read());
    }
    Serial.println();

    // Close the file
    file.close();
    // Open file for reading
    file = SPIFFS.open(configuration);
    if (!file)
    {
        log_d("Failed to read file");
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        Serial.print((char)file.read());
    }
    Serial.println();

    // Close the file
    file.close();

    // Open file for reading
    file = SPIFFS.open(calibration);
    if (!file)
    {
        log_d("Failed to read file");
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        Serial.print((char)file.read());
    }
    Serial.println();

    // Close the file
    file.close();
}

void Configuration::loadCalibration()
{
    File file = SPIFFS.open(calibration);

    // Allocate a temporary JsonDocument
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<512> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        log_d("Failed to read file, using default configuration");
    // Copy values from the JsonDocument to the params

    mCalibrationStruct.isCalibrated = doc["calibrated"];

    for (int i = 0; i < 4; i++)
    {
        mCalibrationStruct.calibration[i] = doc["calibration"][i];
    }
    for (int i = 0; i < 8; i++)
    {
        mCalibrationStruct.minVal[i] = doc["min_values"][i];
        mCalibrationStruct.maxVal[i] = doc["max_values"][i];
    }

    mCalibrationStruct.envOffset = doc["env_offset"];
    dacWrite(OFFSET_ENV, mCalibrationStruct.envOffset);
}

void Configuration::saveCalibrationToFile()
{
    SPIFFS.remove(calibration);
    File file = SPIFFS.open(calibration, FILE_WRITE);
    StaticJsonDocument<512> doc;

    doc["calibrated"] = mCalibrationStruct.isCalibrated;

    for (int i = 0; i < 4; i++)
    {
        doc["calibration"][i] = mCalibrationStruct.calibration[i];
    }
    for (int i = 0; i < 8; i++)
    {
        doc["min_values"][i] = mCalibrationStruct.minVal[i];
        doc["max_values"][i] = mCalibrationStruct.maxVal[i];
    }

    doc["env_offset"] = mCalibrationStruct.envOffset;
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
        log_d("Failed to write to file");
    }

    // Close the file
    file.close();
}
void Configuration::merge(JsonVariant dst, JsonVariantConst src)
{
    if (src.is<JsonObjectConst>())
    {
        for (JsonPairConst kvp : src.as<JsonObjectConst>())
        {
            if (dst[kvp.key()])
                merge(dst[kvp.key()], kvp.value());
            else
                dst[kvp.key()] = kvp.value();
        }
    }
    else
    {
        dst.set(src);
    }
}
