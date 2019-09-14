/*
 * WebServer.cpp
 *
 * Provides a small webserver to read out data as pure JSON or in a dashboard.
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "WebServer.h"

#define DASHBOARD_FILE "dashboard.json"
#define CONFIG_FILE "config.json"

WebServer::WebServer()
{
    server = new ESP8266WebServer(80);
}

WebServer::~WebServer()
{
    delete server;
}

/**
 * Get singleton instance
 */
WebServer* WebServer::getInstance()
{
    static WebServer instance;
    return &instance;
}

/**
 * Initialize the WiFi as soft AP and configure the web server.
 */
void WebServer::init()
{
    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    WiFi.hostname("solar");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(Config::serverSsid.c_str(), Config::serverPassword.c_str());
    Logger::info("started WiFi AP %s on ip %s", Config::serverSsid.c_str(), WiFi.softAPIP().toString().c_str());

    MDNS.begin(Config::serverSsid.c_str());

    server->addHandler(this);
    server->serveStatic("/", SPIFFS, "/");
    server->begin();
    Logger::info("started webserver");
}

/**
 * The main processing logic.
 */
void WebServer::loop()
{
    server->handleClient();
    MDNS.update();
}

/**
 * Find out if we can handle the request.
 */
bool WebServer::canHandle(HTTPMethod requestMethod, String requestUri)
{
    if (Logger::isDebug())
        Logger::debug("http request: %d, url: %s", requestMethod, requestUri.c_str());

    if (requestMethod == HTTP_GET && (requestUri.equals("/data") || requestUri.equals("/list"))) {
        return true;
    }
    if (requestMethod == HTTP_POST && requestUri.equals("/upload")) {
        return true;
    }
    return false;
}

bool WebServer::canUpload(String requestUri) {
    if (!canHandle(HTTP_POST, requestUri))
        return false;

    return true;
}

/**
 * Handle a request and send the inverter data.
 */
bool WebServer::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri)
{
    if (requestUri.equals("/data"))
        server.send(200, "application/json", Inverter::getInstance()->toJSON());
    else if (requestUri.equals("/list"))
        handleFileList();
    else if (requestUri.equals("/upload"))
        server.send(200, "text/plain", "Upload successful");
    else
        return false;

    return true;
}

void WebServer::upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload) {
    handleFileUpload();
}

/**
 * Create file list and send as response.
 */
void WebServer::handleFileList()
{
    Dir dir = SPIFFS.openDir("/");
    String output = "[";
    while (dir.next()) {
        File entry = dir.openFile("r");
        if (output != "[") {
            output += ',';
        }
        bool isDir = false;
        output += "{\"type\":\"";
        output += (isDir) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += String(entry.name()).substring(1);
        output += "\"}";
        entry.close();
    }
    output += "]";
    server->send(200, "text/json", output);
}

/**
 * Receive a new version of the config file.
 */
void WebServer::handleFileUpload()
{
    HTTPUpload& upload = server->upload();

    if (!upload.filename.equals(CONFIG_FILE) && !upload.filename.equals(DASHBOARD_FILE)) {
        return;
    }

    if (upload.status == UPLOAD_FILE_START) {
        fsUploadFile = SPIFFS.open("/" + upload.filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
        }
        Config::load(); // re-load the config from new file
    }
}
