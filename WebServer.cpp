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
    uploadPath = "";
    server = new ESP8266WebServer(80);
}

WebServer::~WebServer()
{
    delete server;
}

/**
 * Initialize the web server.
 */
void WebServer::init()
{
    pinMode(PIN_POWER_OVERRIDE, INPUT);

    server->addHandler(this);
    server->serveStatic("/", LittleFS, "/");
    server->begin();
    logger.info(F("started webserver"));
}

/**
 * The main processing logic.
 */
void WebServer::loop()
{
    server->handleClient();
}

/**
 * Find out if we can handle the request.
 */
bool WebServer::canHandle(HTTPMethod method, String uri)
{
    if (logger.isDebug())
        logger.debug(F("http request: %d, url: %s"), method, uri.c_str());

    if (method == HTTP_GET && (uri.equals(F("/data")) || uri.equals(F("/list"))
    		|| uri.equals(F("/maxCurrent")))) {
        return true;
    }
    if (method == HTTP_POST && uri.equals(F("/upload"))) {
        return true;
    }
    if (method == HTTP_POST && uri.equals(F("/grid"))) {
        return true;
    }
    return false;
}

bool WebServer::canUpload(String uri)
{
    if (!canHandle(HTTP_POST, uri))
        return false;

    return true;
}

/**
 * Handle a request and send the inverter data.
 */
bool WebServer::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri)
{
    if (requestUri.equals(F("/data"))) {
    	server.send(200, F("application/json"), inverter.toJSON());
    } else if (requestUri.equals(F("/maxCurrent"))) {
    	uint16_t maxCurrent = digitalRead(PIN_POWER_OVERRIDE) == HIGH ? 0xffff : inverter.getMaximumSolarCurrent();
        server.send(200, F("application/json"), String(F("{\"maxCurrent\": ")) + maxCurrent + "}");
    } else if (requestUri.equals(F("/list"))) {
        handleFileList();
    } else if (requestUri.equals(F("/upload"))) {
    	server.sendHeader(F("Location"), String("/list?dir=" + uploadPath), true);
    	server.send(302, F("text/plain"), "");
    } else if (requestUri.equals(F("/grid"))) {
    	inverter.switchToGrid(3600);
    	server.send(200, F("text/plain"), F("Switched to grid mode for 1h"));
    } else {
        return false;
    }

    return true;
}

void WebServer::upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload)
{
    handleFileUpload();
}

/**
 * Create file list and send as response.
 */
void WebServer::handleFileList()
{
	String path = server->hasArg("dir") ? server->arg("dir") : "/";
	uploadPath = path;

	Dir dir = LittleFS.openDir(path);
    String output = "<html><body>";
    output += "<h3>" + path + "</h3><table>";

    if (path.length() > 1) {
        String parent = path.substring(0, path.lastIndexOf('/'));
        if (parent.length() == 0) {
            parent = "/";
        }
        output += "<tr><td colspan='4'><a href='/list?dir=" + parent + "'>..</a></td></tr>";
    }
    if (!path.endsWith("/")) {
    	path += "/";
    }
    
    while (dir.next()) {
        File file = dir.openFile("r");
        time_t time = file.getLastWrite();
        String prefix = (file.isDirectory() ? "/list?dir=" : "");
        output += F("<tr>");
        output += "<td><a href='" + prefix + path + file.name() + "'>" + file.name() + F("</a></td>");
        output += "<td style='text-align: right;'>" + (file.isDirectory() ? F("(dir)") : String(file.size())) + F("</td>");
        output += "<td style='text-align: right;'>" + String(ctime(&time)) + F("</td>");
        output += F("</tr>");
        file.close();
    }
    output += F("</table>");
    output += F("<form action='/upload' method='post' enctype='multipart/form-data'>");
    output += F("Upload File: <input type='file' id='uploadFile' name='filename'>");
    output += F("<input type='submit'>");
    output += F("</form>");
    output += F("</body></html>");

    server->send(200, F("text/html"), output);
}

/**
 * Receive a new version of the config file.
 */
void WebServer::handleFileUpload()
{
    HTTPUpload& upload = server->upload();

    if (upload.status == UPLOAD_FILE_START) {
        fsUploadFile = LittleFS.open(uploadPath + "/" + upload.filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
        }
        config.load(); // re-load the config from new file
    }
}

WebServer webServer;
