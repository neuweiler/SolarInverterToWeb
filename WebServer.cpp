/*
 * WebServer.cpp
 *
 * Provides a small webserver to read out data as pure JSON or in a dashboard.
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "WebServer.h"

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
    if (requestMethod == HTTP_GET && (requestUri.equals("/data") || requestUri.equals("/list"))) {
        return true;
    }
    if (requestMethod == HTTP_POST && requestUri.equals(Config::CONFIG_FILE)) {
        return true;
    }
    return false;
}

/**
 * Handle a request and send the inverter data.
 */
bool WebServer::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri)
{
    if (canHandle(requestMethod, requestUri)) {
        if (requestUri.equals("/data"))
            server.send(200, "application/json", Inverter::getInstance()->toJSON());
        else if (requestUri.equals("/list"))
            handleFileList();
        else if (requestUri.equals(Config::CONFIG_FILE) && requestMethod == HTTP_POST)
            handleConfigFileUpload();
        return true;
    }
    return false;
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
void WebServer::handleConfigFileUpload()
{
    if (!server->uri().equals(Config::CONFIG_FILE)) {
      return;
    }

    HTTPUpload& upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
      fsUploadFile = SPIFFS.open(Config::CONFIG_FILE, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) {
        fsUploadFile.close();
      }
    }
}
