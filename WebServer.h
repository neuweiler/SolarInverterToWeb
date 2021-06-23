/*
 * WebServer.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <FS.h>
#include "Logger.h"
#include "Inverter.h"
#include "Config.h"

class WebServer : public RequestHandler {
public:
	WebServer();
	virtual ~WebServer();
	void init();
	void loop();

private:
    bool canHandle(HTTPMethod requestMethod, String requestUri) override;
    bool canUpload(String requestUri);
    bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) override;
    void upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload);

    void handleFileList();
    void handleFileUpload();
	ESP8266WebServer *server;
	File fsUploadFile;
    String uploadPath;
};

extern WebServer webServer;

#endif /* WEBSERVER_H_ */
