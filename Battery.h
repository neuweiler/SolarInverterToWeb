/*
 * Battery.h
 *
 *  Created on: 19 Mar 2021
 *      Author: michael
 */

#ifndef BATTERY_H_
#define BATTERY_H_

#include <Arduino.h>
#include "Logger.h"
#include "Config.h"

class Battery {
public:
	Battery();
	virtual ~Battery();
	void init();
	void loop();
	bool isFullyCharged();
	bool isEmpty();

	void setSOC(uint16_t soc);
	uint16_t getSOC();
	uint16_t getAmpereHours();
	void setCurrent(int16_t current);
	int16_t getCurrent();
	void setVoltage(float voltage);
	float getVoltage();
	int16_t getPower();
	void setVoltageSCC(float voltageSCC);
	float getVoltageSCC();
private:
	uint32_t timestamp;
	uint32_t restTimestamp;
	uint16_t soc; // in 0.1%
	int32_t ampereMilliseconds;
	uint16_t ampereHours; // in 0.1Ah
	int16_t current; // in A
	float voltage; // in V
	float voltageSCC; // in V

	void checkBatteryResting();
	void updateSoc();
};

extern Battery battery;

#endif /* BATTERY_H_ */
