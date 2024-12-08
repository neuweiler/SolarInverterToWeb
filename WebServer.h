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
    bool canHandle(HTTPMethod method, const String& uri) override;
    bool canUpload(const String& uri) override;
    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, const String& requestUri) override;
    void upload(ESP8266WebServer& server, const String& requestUri, HTTPUpload& upload) override;

private:
    void replyServerError(String msg);
    void handleFileList();
	ESP8266WebServer *server;
	File fsUploadFile;
    String uploadPath;
};

extern WebServer webServer;

#endif /* WEBSERVER_H_ */
