/*
 * Inverter.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include <Arduino.h>
#include "Logger.h"

#define INPUT_BUFFER_SIZE 250

class Inverter {
public:
	enum Status {
		acCharging = 0,
		sccCharging = 1,
		charging = 2,
		batteryVoltageSteady = 3,
		load = 4
	};

	enum Mode {
		off = 0, on = 1, chargeFloating = 2
	};

	static Inverter* getInstance();
	void init();
	void loop();
	String toJSON();

private:
	Inverter();
	virtual ~Inverter();
	void operator=(Inverter const&);
	bool readResponse();
	void queryStatus();
	void parseStatusResponse(char *input);
	void processFloat(float *value);
	void processInt(uint16_t *value);
	void processShort(uint8_t *value);
    String jsonElement(String name, uint8_t value);
    String jsonElement(String name, uint16_t value);
    String jsonElement(String name, float value);
    bool prefix;
    char buffer[80];

	char input[INPUT_BUFFER_SIZE + 1];
	uint32_t timestamp;

	uint8_t mode;
	uint8_t status;
	float gridVoltage; // in V
	float gridFrequency; // in Hz
	float outVoltage; // in V
	float outFrequency; // in Hz
	uint16_t outPowerApparent; // in VA
	uint16_t outPowerActive; // in W
	uint8_t outLoad; // in percent
	uint16_t busVoltage; // in V
	float batteryVoltage; // in V
	float batteryVoltageSCC; // in V
	uint16_t batteryChargeCurrent; // in A
	uint16_t batteryDischargeCurrent; // in A
	uint8_t batterySOC; // in percent
	uint16_t pvCurrent; // in A
	float pvVoltage; // in V
	uint16_t pvChargingPower; // in W
	float temperature; // in degree Celsius
	uint16_t fanCurrent; // in mW
	uint8_t eepromVersion;
};

#endif /* INVERTER_H_ */
