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

//#define DEBUG_MEM

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

#ifdef DEBUG_MEM
    printHeapInfo();
#endif
}

void printHeapInfo() {
	logger.debug(F("free: %u, frag: %u, maxfree: %u"), ESP.getFreeHeap(), ESP.getHeapFragmentation(), ESP.getMaxFreeBlockSize());
    delay(500);
}
