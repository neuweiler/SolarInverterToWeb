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

#define AP_SSID "solar"
#define AP_PASSWORD "inverter"

class WebServer : public RequestHandler {
public:
	static WebServer* getInstance();
	void init();
	void loop();
    bool canHandle(HTTPMethod requestMethod, String requestUri) override;
    bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) override;
    void handleFileList();

private:
	WebServer();
	virtual ~WebServer();
	void operator=(WebServer const&);

	ESP8266WebServer *server;
};

#endif /* WEBSERVER_H_ */
