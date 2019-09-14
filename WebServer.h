/*
 * WebServer.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include "Logger.h"
#include "Inverter.h"
#include "Config.h"

class WebServer : public RequestHandler {
public:
	static WebServer* getInstance();
	void init();
	void loop();
    bool canHandle(HTTPMethod requestMethod, String requestUri) override;
    bool canUpload(String requestUri);
    bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) override;
    void upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload);
    void handleFileList();
    void handleFileUpload();

private:
	WebServer();
	virtual ~WebServer();
	void operator=(WebServer const&);

	ESP8266WebServer *server;
	File fsUploadFile;
};

#endif /* WEBSERVER_H_ */
