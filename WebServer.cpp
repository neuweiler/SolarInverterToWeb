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
    server->addHandler(this);
    server->serveStatic("/", LittleFS, "/");
    server->begin();
    logger.info("started webserver");
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
bool WebServer::canHandle(HTTPMethod requestMethod, String requestUri)
{
    if (logger.isDebug())
        logger.debug("http request: %d, url: %s", requestMethod, requestUri.c_str());

    if (requestMethod == HTTP_GET && (requestUri.equals("/data") || requestUri.equals("/list")
    		|| requestUri.equals("/maxCurrent"))) {
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
        server.send(200, "application/json", inverter.toJSON());
    if (requestUri.equals("/maxCurrent"))
        server.send(200, "application/json", String("{\"maxCurrent\": ") + inverter.getMaximumSolarCurrent() + "}");
    else if (requestUri.equals("/list"))
        handleFileList();
    else if (requestUri.equals("/upload")) {
    	server.sendHeader("Location", String("/list?dir=" + uploadPath), true);
    	server.send(302, "text/plain", "");
    }
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
        output += "<tr>";
        output += "<td><a href='" + prefix + path + file.name() + "'>" + file.name() + "</a></td>";
        output += "<td style='text-align: right;'>" + (file.isDirectory() ? "(dir)" : String(file.size())) + "</td>";
        output += "<td style='text-align: right;'>" + String(ctime(&time)) + "</td>";
        output += "</tr>";
        file.close();
    }
    output += "</table>";
    output += "<form action='/upload' method='post' enctype='multipart/form-data'>";
    output += "Upload File: <input type='file' id='uploadFile' name='filename'>";
    output += "<input type='submit'>";
    output += "</form>";
    output += "</body></html>";

    server->send(200, "text/html", output);
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

