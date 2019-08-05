/*
 * SolarInverterToWeb.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include <Arduino.h>

#include "Logger.h"
#include "Inverter.h"
#include "Station.h"
#include "WebServer.h"

void setup() {
	Logger::init();
	Logger::info("Starting solar monitor...");

    Station::getInstance()->init();
    WebServer::getInstance()->init();
	Inverter::getInstance()->init();
}

void loop() {
	WebServer::getInstance()->loop();
    Inverter::getInstance()->loop();
    Station::getInstance()->loop();
}
