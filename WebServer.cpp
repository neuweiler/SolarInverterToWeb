/*
 * WebServer.cpp
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

void WebServer::loop()
{
    server->handleClient();
    MDNS.update();
}

bool WebServer::canHandle(HTTPMethod requestMethod, String requestUri)
{
    if (requestMethod == HTTP_GET && (requestUri.equals("/data") || requestUri.equals("/list"))) {
        return true;
    }
    return false;
}

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

void WebServer::handleFileList()
{
    if (!server->hasArg("dir")) {
        server->send(500, "text/plain", "BAD ARGS");
        return;
    }

    String path = server->arg("dir");
    Dir dir = SPIFFS.openDir(path);
    path = String();

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
