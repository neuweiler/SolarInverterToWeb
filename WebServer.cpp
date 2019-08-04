/*
 * WebServer.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "WebServer.h"

#define SSID "solar"
#define PASSWORD "inverter"

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
    WiFi.softAP(SSID, PASSWORD);
    Logger::info("started WiFi AP %s on ip %s", SSID, WiFi.softAPIP().toString().c_str());

    MDNS.begin("solar");

    server->addHandler(this);
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
    if (requestMethod != HTTP_GET)
        return false;
    return true;
}

bool WebServer::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri)
{
    if (!canHandle(requestMethod, requestUri))
        return false;
    if (requestUri.equals("/data")) {
        server.send(200, "application/json", Inverter::getInstance()->toJSON());
    } else if (requestUri.equals("/list")) {
        handleFileList();
    } else {
        if (!handleFileRead(requestUri)) {
            server.send(404, "text/plain", "File Not Found: " + requestUri);
        }
    }
    return true;
}

bool WebServer::handleFileRead(String path)
{
    Logger::info("serving file: %s", path.c_str());
    if (path.endsWith("/")) {
        path += "index.htm";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if (SPIFFS.exists(pathWithGz)) {
            path += ".gz";
        }
        File file = SPIFFS.open(path, "r");
        server->streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

String WebServer::getContentType(String filename)
{
    if (server->hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
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

