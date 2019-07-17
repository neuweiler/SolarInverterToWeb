/*
 * WebServer.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <ESP8266WebServer.h>
#include "Logger.h"
#include "Inverter.h"

class HTTPHandler : public RequestHandler {
public:
	bool canHandle(HTTPMethod requestMethod, String requestUri) override;
	bool handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) override;
};

class WebServer {
public:
	static WebServer* getInstance();
	void init();
	void loop();
	void handleRootPath();

private:
	WebServer();
	virtual ~WebServer();
	void operator=(WebServer const&);

	ESP8266WebServer *server;
	HTTPHandler *handler;
};

#endif /* WEBSERVER_H_ */
