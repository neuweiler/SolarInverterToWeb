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
	logger.init();
	logger.info("Starting solar monitor...");

	config.init();
	station.init();
	webServer.init();
	inverter.init();
}

void loop() {
	webServer.loop();
	inverter.loop();
	station.loop();
}
