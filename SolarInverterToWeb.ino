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

void printHeapInfo(uint8_t i) {
	Serial.printf("%d: free: %u, frag: %u, maxfree: %u\n", i,
	ESP.getFreeHeap(), ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize());
}

void setup() {
	logger.init();
	logger.info(F("Starting solar monitor..."));

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
//printHeapInfo(9);
//delay(100);
}
