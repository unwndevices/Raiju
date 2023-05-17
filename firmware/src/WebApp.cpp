#include "WebApp.h"

// AP name and password
const char *ssid = "Raiju";
const char *pass = "unknowndevices";

// Hostname for WiFi access point
String hostname = "raiju";

// File paths to server files
const char *indexHTML = "/index.html.gz";
const char *calibrationHTML = "/calibration.html.gz";
const char *updateHTML = "/update.html.gz";
const char *configJSON = "/configuration.json";
const char *calibrationJSON = "/calibration.json";

// Static IP address for WiFi access point
IPAddress WebApp::staticIP(192, 168, 1, 1);

// Web server instance
WebServer WebApp::server(80);
void WebApp::init()
{
    // Get configuration instance
    pConfig = Configuration::getInstance();

    connectToWiFi();
    configureWebPages();
    configureUpdateFW();
    xTaskCreatePinnedToCore(this->task, "WebApp", 2048 * 2, this, 1, &taskApp, 0); /// pinned on core 0
}

void WebApp::connectToWiFi()
{
    // Disable WiFi power saving mode
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.setHostname(hostname.c_str());

    // Get SSID and password from configuration
    const char *ssidSta = pConfig->mConfigStruct.ssid.c_str();
    const char *passSta = pConfig->mConfigStruct.password.c_str();

    if (!pConfig->mConfigStruct.ip.isEmpty())
    {
        staticIP.fromString(pConfig->mConfigStruct.ip);
    }

    if ((ssidSta != NULL) && (ssidSta[0] != '\0'))
    {
        uint8_t attempts = 0;
        WiFi.mode(WIFI_STA);
        log_d("connecting in STA mode");
        if (!staticIP.toString().isEmpty())
        {
            log_d("configuring IP");
            if (!WiFi.config(staticIP, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0)))
                log_d("failed to config IP");
        }
        WiFi.begin(ssidSta, passSta);
        while ((WiFi.status() != WL_CONNECTED) & (attempts < 5))
        {
            Serial.print('.');
            delay(1000);
            attempts++;
        }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        log_d("connecting in AP mode");
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(staticIP, staticIP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(ssid, pass); // 1 client limit
    }

    // MDNS.begin("raiju");
}

void WebApp::configureWebPages()
{
    // Handle GET requests
    server.on("/", HTTP_GET, [this]()
              {
        if (!this->handleFileRead(indexHTML))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
    server.on("/index.html", HTTP_GET, [this]()
              {
        if (!this->handleFileRead(indexHTML))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
    server.on("/calibration.html", HTTP_GET, [this]()
              {
        if (!this->handleFileRead(calibrationHTML))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
    server.on("/update.html", HTTP_GET, [this]()
              {
        if (!this->handleFileRead(updateHTML))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
    server.on(configJSON, HTTP_GET, [this]()
              {
        if (!this->handleFileRead(configJSON)) {
            server.send(404, "text/plain", "Filenotfound");
        } });
    server.on(calibrationJSON, HTTP_GET, [this]()
              {
        if (!this->handleFileRead(calibrationJSON))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });

    server.on("/voct", HTTP_GET, [this]()
              {
        int analogValue = analog.getVORaw(); // Assuming you're using GPIO34 for analog reading
        String analogValueString = String(analogValue);
        server.send(200, "text/plain", analogValueString); });

    // Handle POST requests
    server.on(configJSON, HTTP_PUT, [this]()
              {
        if (!this->handlePut(true))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
    server.on(calibrationJSON, HTTP_PUT, [this]()
              {
        if (!this->handlePut(false))
        {
            server.send(404, "text/plain", "FileNotFound");
        } });
}

void WebApp::stop()
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    server.stop();
    vTaskDelete(taskApp);
}

void WebApp::handleStatus()
{
    // state = pConfig->mConfigStruct.wifiStatus;
    if (!state)
    {
        stop();
    }
}

void WebApp::update()
{
    server.handleClient();
}

void WebApp::task(void *handle)
{
    WebApp *pThis = (WebApp *)handle;
    server.begin();
    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            pThis->update();
        }
        // vTaskDelay(10); // TODO test delay
    }
}

void WebApp::configureUpdateFW()
{
    /*handling uploading firmware file */
    server.on(
        "/update", HTTP_POST, []()
        {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
        []()
        {
            HTTPUpload &upload = server.upload();
            if (upload.status == UPLOAD_FILE_START)
            {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                { // start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                { // true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else
                {
                    Update.printError(Serial);
                }
            }
        });
}

///////////////////////////////////////////////
/////////////////// HELPERS ///////////////////
///////////////////////////////////////////////

bool WebApp::exists(String path)
{
    bool yes = false;
    File file = SPIFFS.open(path, "r");
    if (!file.isDirectory())
    {
        yes = true;
    }
    file.close();
    return yes;
}

String WebApp::getContentType(String filename)
{
    if (server.hasArg("download"))
    {
        return "application/octet-stream";
    }
    else if (filename.endsWith(".json"))
    {
        return "application/json";
    }
    else if (filename.endsWith(".htm"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".html"))
    {
        return "text/html";
    }
    else if (filename.endsWith(".css"))
    {
        return "text/css";
    }
    else if (filename.endsWith(".js"))
    {
        return "application/javascript";
    }
    else if (filename.endsWith(".png"))
    {
        return "image/png";
    }
    else if (filename.endsWith(".gif"))
    {
        return "image/gif";
    }
    else if (filename.endsWith(".jpg"))
    {
        return "image/jpeg";
    }
    else if (filename.endsWith(".ico"))
    {
        return "image/x-icon";
    }
    else if (filename.endsWith(".xml"))
    {
        return "text/xml";
    }
    else if (filename.endsWith(".pdf"))
    {
        return "application/x-pdf";
    }
    else if (filename.endsWith(".zip"))
    {
        return "application/x-zip";
    }
    else if (filename.endsWith(".html.gz"))
    {
        return "text/html";
    }
    return "text/plain";
}

bool WebApp::handleFileRead(String path)
{
    log_d("handleFileRead: %s", path);
    if (path.endsWith("/"))
    {
        path += "index.html";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (exists(pathWithGz) || exists(path))
    {
        if (exists(pathWithGz))
        {
            path += ".gz";
        }
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

bool WebApp::handlePut(bool type)
{
    if (!server.hasArg("plain"))
    {
        log_d("error");
        return false;
    }

    String body = server.arg("plain");
    if (type == true)
    {
        // Serial.println("Calling saveConfiguration");
        pConfig->saveConfiguration(body);
        pConfig->mConfigStruct.hasChanged = true;
    }

    else if (type == false)
    {
        // Serial.println("Calling saveCalibration");
        pConfig->saveCalibration(body);
        ESP.restart();
    }
    server.send(200, "application/json", "{}");
    return true;
}

void WebApp::handleUpload()
{
}