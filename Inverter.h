/*
 * Inverter.h
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Logger.h"
#include "CRCUtil.h"
#include "Config.h"
#include "Battery.h"

#define INPUT_BUFFER_SIZE 512

class Inverter
{
public:
    enum Status
    {
        LOAD = 1 << 0,
        BATTERY_VOLTAGE_TOO_STEADY = 1 << 2,
        CHARGING = 1 << 3,
        CHARGING_SOLAR = 1 << 4,
        CHARGING_GRID = 1 << 5,
        CHARGING_FLOATING = 1 << 6,
        SWITCHED_ON = 1 << 7
    };

    enum Mode
    {
        ON,
        STAND_BY,
        LINE,
        BATTERY,
        BYPASS,
        ECO,
        FAULT,
        POWER_SAVE,
        UNKNOWN
    };
    static const char *modeString[];

    enum Warning
    {
        INVERTER_FAULT = 1 << 0,
        BUS_OVERVOLTAGE = 1 << 1,
        BUS_UNDERVOLTAGE = 1 << 2,
        BUS_SOFT_FAIL = 1 << 3,
        GRID_FAIL = 1 << 4,
        OPV_SHORT = 1 << 5,
        INVERTER_UNDERVOLTAGE = 1 << 6,
        INVERTER_OVERVOLTAGE = 1 << 7,
        OVER_TEMPERATURE = 1 << 8,
        FAN_LOCKED = 1 << 9,
        BATTERY_OVERVOLTAGE = 1 << 10,
        BATTERY_UNDERVOLTAGE = 1 << 11,
        BATTERY_OVERCHARGE = 1 << 12,
        BATTERY_SHUTDOWN = 1 << 13,
        BATTERY_DERATING = 1 << 14,
        OVER_LOAD = 1 << 15,
        EEPROM_FAULT = 1 << 16,
        INVERTER_OVER_CURRENT = 1 << 17,
        INVERTER_SOFT_FAIL = 1 << 18,
        SELF_TEST_FAIL = 1 << 19,
        OP_DC_OVER_VOLTAGE = 1 << 20,
        BATTERY_OPEN = 1 << 21,
        CURRENT_SENSOR_FAIL = 1 << 22,
        BATTERY_SHORT = 1 << 23,
        POWER_LIMIT = 1 << 24,
        PV_VOLTAGE_HIGH = 1 << 25,
        MPPT_OVERLOAD_FAULT = 1 << 26,
        MPPT_OVERLOAD_WARNING = 1 << 27,
        BATTER_TOO_LOW_TO_CHARGE = 1 << 28,
        DC_DC_OVERCURRENT = 1 << 29
    };

    Inverter();
    virtual ~Inverter();
    void init();
    void loop();
    String toJSON();
    void calculateMaximumSolarPower();
    uint16_t getMaximumSolarPower();
    uint16_t getMaximumSolarCurrent();
    void switchToGrid(uint16_t duration);

private:
    enum QueryMode
    {
        STATUS,
        MODE,
        WARNING,
		IGNORE
    };

    void setFloatVoltage(float voltage);
    void sendCommand(String command);bool readResponse();
    void sendQuery();
    void parseStatusResponse(char *input);
    void parseModeResponse(char *input);
    void parseWarningResponse(char *input);
    float parseFloat();
    uint16_t parseInt();
    uint8_t parseShort();
    uint8_t parseStatus1();
    uint8_t parseStatus2();
    String evalChargeSource();
    String evalLoadSource();
    void evalWarning(JsonArray &array);
	void processResponse();
	bool adjustFloatVoltage();
	bool overDischargeProtection();

    char input[INPUT_BUFFER_SIZE + 1];
    char buffer[20];
    uint32_t timestamp;
    uint32_t cutoffTime;
    uint16_t maxSolarPower;

    QueryMode queryMode;
    Mode mode;
    uint8_t status;
    uint32_t warning;
    float gridVoltage; // in V
    float gridFrequency; // in Hz
    float outVoltage; // in V
    float outFrequency; // in Hz
    uint16_t outPowerApparent; // in VA
    uint16_t outPowerActive; // in W
    uint8_t outLoad; // in percent
    uint16_t busVoltage; // in V
    uint16_t pvCurrent; // in A
    float pvVoltage; // in V
    uint16_t pvChargingPower; // in W
    float temperature; // in degree Celsius
    uint16_t fanCurrent; // in mW
    uint8_t eepromVersion;
    uint8_t faultCode;
    bool floatOverrideActive;
    bool overDischargeProtectionActive;
    float floatVoltage; // in V
	StaticJsonDocument<2048> jsonDoc;
};

extern Inverter inverter;

#endif /* INVERTER_H_ */
