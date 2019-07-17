/*
 * SolarInverterToWeb.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include <Arduino.h>

#include "Logger.h"
#include "Inverter.h"
#include "WebServer.h"

void setup() {
	Logger::init();
	Logger::info("Starting solar monitor...");

	Inverter::getInstance()->init();
	WebServer::getInstance()->init();
}

void loop() {
	Inverter::getInstance()->loop();
	WebServer::getInstance()->loop();
}
