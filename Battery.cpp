/*
 * Battery.cpp
 *
 *  Created on: 19 Mar 2021
 *      Author: michael
 */

#include "Battery.h"

Battery::Battery() {
	timestamp = 0;
	restTimestamp = 0;
	soc = 0;
	ampereMilliseconds = 0;
	ampereHours = 0;
	current = 0;
	voltage = 0;
	voltageSCC = 0;

}

Battery::~Battery() {
}

void Battery::init() {
	timestamp = millis();
}

void Battery::loop() {
	updateSoc();
	checkBatteryResting();
}

/**
 *
 * 1Ah = 60Am = 3600As = 3600000Ams
 */
void Battery::updateSoc() {
	ampereMilliseconds += current * (millis() - timestamp);
	timestamp = millis();

	if (ampereMilliseconds > 3600000) {
		if (ampereHours < config.batteryCapacity) {
			ampereHours++;
		}
		ampereMilliseconds -= 3600000;
	} else if (ampereMilliseconds < -3600000) {
		if (ampereHours > 0) {
			ampereHours--;
		}
		ampereMilliseconds += 3600000;
	}

	if (isEmpty()) {
		ampereHours = 0;
	}
	if (isFullyCharged()) {
		ampereHours = config.batteryCapacity;
	}

	if (config.batterySocCalculateInternally) {
		soc = ampereHours * 100 / config.batteryCapacity;
	}
}

void Battery::checkBatteryResting() {
	if (current <= config.batteryRestCurrent * -1) {
		restTimestamp = millis(); // we're not resting, update the timestanp
	}
}

bool Battery::isFullyCharged() {
	return voltage >= config.batteryVoltageFullCharge && current < config.batteryRestCurrent;
}

bool Battery::isEmpty() {
	return voltage <= config.batteryVoltageEmpty
			&& (restTimestamp + config.batteryRestDuration * 1000) < millis();
}

void Battery::setSOC(uint8_t soc) {
	if (!config.batterySocCalculateInternally) {
		this->soc = soc;
	}
}

uint8_t Battery::getSOC() {
	return soc;
}

uint16_t Battery::getAmpereHours() {
	return ampereHours;
}

void Battery::setCurrent(int16_t current) {
	this->current = current;
}

int16_t Battery::getCurrent() {
	return current;
}

void Battery::setVoltage(float voltage) {
	this->voltage = voltage;
}

float Battery::getVoltage() {
	return voltage;
}

int16_t Battery::getPower() {
	return current * voltage;
}

void Battery::setVoltageSCC(float voltageSCC) {
	this->voltageSCC = voltageSCC;
}

float Battery::getVoltageSCC() {
	return voltageSCC;
}

Battery battery;
