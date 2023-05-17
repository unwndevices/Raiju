#ifndef WEBAPP_H
#define WEBAPP_H
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Configuration.h"
#include <SPIFFS.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <Update.h>
#include "drivers/Analog.h"
class WebApp
{
public:
    WebApp(){};

    //! init after SPIFFS
    void init();
    void stop();

    void update();
    void handleStatus();

    static void task(void *handle);

private:
    static bool apMode;
    static WebServer server;
    static IPAddress staticIP;
    static StaticJsonDocument<1024> jsonDocument;

    void connectToWiFi();
    void configureWebPages();
    void configureUpdateFW();

    bool exists(String path);

    String getContentType(String filename);
    bool handleFileRead(String path);
    bool handlePut(bool type);
    void handleUpload();

    Configuration *pConfig = nullptr;

    TaskHandle_t taskApp;
    bool state = true;
    Analog analog;
};

#endif // !WEBAPP_H