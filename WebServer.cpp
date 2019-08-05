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
    SPIFFS.begin();

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    WiFi.hostname("solar");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Logger::info("started WiFi AP %s on ip %s", AP_SSID, WiFi.softAPIP().toString().c_str());

    MDNS.begin("solar");

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
