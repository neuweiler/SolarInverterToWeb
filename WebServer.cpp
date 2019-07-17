/*
 * WebServer.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "WebServer.h"

const char *ssid = "solar";
const char *password = "inverter";

WebServer::WebServer() {
	server = new ESP8266WebServer(80);
	handler = new HTTPHandler();
}

WebServer::~WebServer() {
	delete server;
	delete handler;
}

/**
 * Get singleton instance
 */
WebServer* WebServer::getInstance() {
	static WebServer instance;
	return &instance;
}

void WebServer::init() {
	WiFi.softAP(ssid, password);
	Logger::info("started WiFi AP %s on ip %s", ssid,
			WiFi.softAPIP().toString().c_str());

	server->addHandler(handler);
	server->begin();
	Logger::info("started webserver");
}

void WebServer::loop() {
	server->handleClient();
}

bool HTTPHandler::canHandle(HTTPMethod requestMethod, String requestUri) {
	if (requestMethod != HTTP_GET)
		return false;
	return true;
}

bool HTTPHandler::handle(ESP8266WebServer &server, HTTPMethod requestMethod, String requestUri) {
	if (!canHandle(requestMethod, requestUri))
		return false;
	server.send(200, "application/json", Inverter::getInstance()->toJSON());
	return true;
}
