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
#include "WLAN.h"

void setup() {
	logger.init();
	logger.info("Starting solar monitor...");

	config.init();
	wlan.init();
	webServer.init();
	battery.init();
	inverter.init();
}

void loop() {
	webServer.loop();
	inverter.loop();
	wlan.loop();
}
