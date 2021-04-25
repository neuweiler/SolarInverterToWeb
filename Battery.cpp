/*
 * Battery.cpp
 *
 *  Created on: 19 Mar 2021
 *      Author: michael
 */

#include "Battery.h"

/**
 * Constructor
 */
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

/**
 * Destructor
 */
Battery::~Battery() {
}

/**
 * Initialize the class
 */
void Battery::init() {
	timestamp = millis();
}

/**
 * the main loop
 */
void Battery::loop() {
	updateSoc();
	checkBatteryResting();
}

/**
 * Calculate the state of charge based on current and elapsed time since last measurement.
 * The calculation is done in an Ampere-miliseconds buffer and moved over from there to the Ah counter
 * (which counts in 0.1Ah)
 *
 * 1Ah = 60Am = 3600As = 3600000Ams
 */
void Battery::updateSoc() {
	ampereMilliseconds += current * (millis() - timestamp);
	timestamp = millis();

	if (ampereMilliseconds > 360000) {
		if (ampereHours < config.batteryCapacity * 10) {
			ampereHours++;
		}
		ampereMilliseconds -= 360000;
	} else if (ampereMilliseconds < -3600000) {
		if (ampereHours > 0) {
			ampereHours--;
		}
		ampereMilliseconds += 360000;
	}

	if (isEmpty()) {
		ampereHours = 0;
	}
	if (isFullyCharged()) {
		ampereHours = config.batteryCapacity * 10;
	}

	if (config.batterySocCalculateInternally) {
		soc = ampereHours * 100 / config.batteryCapacity;
	}
}

void Battery::checkBatteryResting() {
	if (current <= config.batteryRestCurrent * -1) {
		restTimestamp = millis(); // we're not resting, update the timestamp
	}
}

bool Battery::isFullyCharged() {
	return voltage >= config.batteryVoltageFullCharge && current < config.batteryRestCurrent;
}

bool Battery::isEmpty() {
	return voltage <= config.batteryVoltageEmpty
			&& (restTimestamp + config.batteryRestDuration * 1000) < millis();
}

/**
 * Set the current state of charge in 0.1% (if we don't calc it by ourselves)
 */
void Battery::setSOC(uint16_t soc) {
	if (!config.batterySocCalculateInternally) {
		this->soc = soc;
	}
}

/**
 * Get the current state of charge in 0.1%
 */
uint16_t Battery::getSOC() {
	return soc;
}

/**
 * Return the remaining ampere hours in the battery (in 0.1A)
 */
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
