/*
 * Inverter.cpp
 *
 * Communicates with the inverter and parses its response.
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "Inverter.h"

const char * Inverter::modeString[] = { "ON", "STAND_BY", "LINE", "BATTERY", "BYPASS", "ECO", "FAULT", "POWER_SAVE", "UNKNOWN" };

/**
 * Constructor
 */
Inverter::Inverter()
{
    mode = UNKNOWN;
    status = 0;
    warning = 0;
    faultCode = 0;
    gridVoltage = 0;
    gridFrequency = 0;
    outVoltage = 0;
    outFrequency = 0;
    outPowerApparent = 0;
    outPowerActive = 0;
    outLoad = 0;
    busVoltage = 0;
    batteryVoltage = 0;
    batteryVoltageSCC = 0;
    batteryCurrent = 0;
    batterySOC = 0;
    batteryPower = 0;
    pvCurrent = 0;
    pvVoltage = 0;
    pvChargingPower = 0;
    temperature = 0;
    fanCurrent = 0;
    eepromVersion = 0;

    queryMode = STATUS;
    timestamp = 0;
    maxSolarPower = 1000;
}

Inverter::~Inverter()
{
}

/**
 * Get singleton instance.
 */
Inverter* Inverter::getInstance()
{
    static Inverter instance;
    return &instance;
}

/**
 * Initialize the Inverter.
 */
void Inverter::init()
{
    Serial.begin(2400);

    timestamp = millis();

    maxSolarPower = Config::initialMaxSolarPower;
}

/**
 * The main processing logic, called by the program's loop().
 */
void Inverter::loop()
{
    if (timestamp + Config::inverterUpdateInterval > millis())
        return;

    if (readResponse()) {
        switch (queryMode) {
        case MODE:
            parseModeResponse(input);
            queryMode = STATUS;
            break;
        case STATUS:
            parseStatusResponse(input);
            queryMode = WARNING;
            break;
        case WARNING:
            parseWarningResponse(input);
            queryMode = STATUS;
            break;
        }
    }

    sendQuery();

    timestamp = millis();
}

/**
 * Send a command to the inverter with a checksum.
 */
void Inverter::sendCommand(String command)
{
    String crc = CRCUtil::getCRC(command);

    Logger::info("sending command: %s", command.c_str());

    Serial.print(command);
    Serial.print(crc);
    Serial.write(13);
}

/**
 * Check if a response is available on the serial port and read it to a buffer.
 */
bool Inverter::readResponse()
{
    if (Serial.available()) {
        byte size = Serial.readBytes(input, INPUT_BUFFER_SIZE);
        input[size - 1] = 0; // drop the last char, it's a CR and not useful for CRC calculation
        return CRCUtil::checkCRC(String(input));
    }
    return false;
}

/**
 * Query the actual data and status from the inverter.
 */
void Inverter::sendQuery()
{
    switch (queryMode) {
    case MODE:
        sendCommand("QMOD");
        break;
    case STATUS:
        sendCommand("QPIGS");
        break;
    case WARNING:
        sendCommand("QPIWS");
        break;
    }
}

/**
 * Parse the inverter's response to a mode request.
 *
 * Example: (S<CRC>
 */
void Inverter::parseModeResponse(char *input)
{
    if (input[0] != '(' || strlen(input) < 2 || strstr(input, "(NAK") != NULL) {
        Logger::info("unable to parse '%s", input);
    }
    input++; // skip the (

    switch (input[0]) {
    case 'P':
        mode = ON;
        break;
    case 'S':
        mode = STAND_BY;
        break;
    case 'L':
        mode = LINE;
        break;
    case 'B':
        mode = BATTERY;
        break;
    case 'Y':
        mode = BYPASS;
        break;
    case 'E':
        mode = ECO;
        break;
    case 'F':
        mode = FAULT;
        break;
    case 'H':
        mode = POWER_SAVE;
        break;
    default:
        mode = UNKNOWN;
        break;
    }
}

/**
 * Parse the inverter's response to a status request.
 *
 * Example: (235.3 49.9 229.9 49.9 1800 1810 050 348 25.10 000 085 0040 00.0 117.4 00.00 00000 00010110 00 00 00000 110<CRC>
 */
void Inverter::parseStatusResponse(char *input)
{
    if (input[0] != '(' || strlen(input) < 10 || strchr(input, ' ') == NULL) {
        Logger::info("unable to parse '%s", input);
    }
    input++; // skip the (

    char *token = strtok(input, " ");
    if (token != NULL) {
        uint16_t batteryChargeCurrent;
        uint16_t batteryDischargeCurrent;

        gridVoltage = atof(token);
        processFloat(&gridFrequency);
        processFloat(&outVoltage);
        processFloat(&outFrequency);
        processInt(&outPowerApparent);
        processInt(&outPowerActive);
        processShort(&outLoad);
        processInt(&busVoltage);
        processFloat(&batteryVoltage);
        processInt(&batteryChargeCurrent);
        processShort(&batterySOC);
        processFloat(&temperature);
        processInt(&pvCurrent);
        processFloat(&pvVoltage);
        processFloat(&batteryVoltageSCC);
        processInt(&batteryDischargeCurrent);
        processStatus1();
        processInt(&fanCurrent);
        fanCurrent *= 10;
        processShort(&eepromVersion);
        processInt(&pvChargingPower);
        processStatus2();
        batteryCurrent = (batteryChargeCurrent > 0 ? batteryChargeCurrent : -1 * batteryDischargeCurrent);
        batteryPower = batteryCurrent * batteryVoltage;
    }
}

/**
 * Parse the inverter's response to a warning request.
 *
 * Example: (0110000000000000000000000000000022<CRC>
 */
void Inverter::parseWarningResponse(char *input)
{
    if (input[0] != '(' || strlen(input) < 2 || strstr(input, "(NAK") != NULL) {
        Logger::info("unable to parse '%s", input);
    }
    input++; // skip the (

    warning = 0;
    if (input[1] == '1')
        warning |= INVERTER_FAULT;
    if (input[2] == '1')
        warning |= BUS_OVERVOLTAGE;
    if (input[3] == '1')
        warning |= BUS_UNDERVOLTAGE;
    if (input[4] == '1')
        warning |= BUS_SOFT_FAIL;
    if (input[5] == '1')
        warning |= GRID_FAIL;
    if (input[6] == '1')
        warning |= OPV_SHORT;
    if (input[7] == '1')
        warning |= INVERTER_UNDERVOLTAGE;
    if (input[8] == '1')
        warning |= INVERTER_OVERVOLTAGE;
    if (input[9] == '1')
        warning |= OVER_TEMPERATURE;
    if (input[10] == '1')
        warning |= FAN_LOCKED;
    if (input[11] == '1')
        warning |= BATTERY_OVERVOLTAGE;
    if (input[12] == '1')
        warning |= BATTERY_UNDERVOLTAGE;
    if (input[13] == '1')
        warning |= BATTERY_OVERCHARGE;
    if (input[14] == '1')
        warning |= BATTERY_SHUTDOWN;
    if (input[15] == '1')
        warning |= BATTERY_DERATING;
    if (input[16] == '1')
        warning |= OVER_LOAD;
    if (input[17] == '1')
        warning |= EEPROM_FAULT;
    if (input[18] == '1')
        warning |= INVERTER_OVER_CURRENT;
    if (input[19] == '1')
        warning |= INVERTER_SOFT_FAIL;
    if (input[20] == '1')
        warning |= SELF_TEST_FAIL;
    if (input[21] == '1')
        warning |= OP_DC_OVER_VOLTAGE;
    if (input[22] == '1')
        warning |= BATTERY_OPEN;
    if (input[23] == '1')
        warning |= CURRENT_SENSOR_FAIL;
    if (input[24] == '1')
        warning |= BATTERY_SHORT;
    if (input[25] == '1')
        warning |= POWER_LIMIT;
    if (input[26] == '1')
        warning |= PV_VOLTAGE_HIGH;
    if (input[27] == '1')
        warning |= MPPT_OVERLOAD_FAULT;
    if (input[28] == '1')
        warning |= MPPT_OVERLOAD_WARNING;
    if (input[29] == '1')
        warning |= BATTER_TOO_LOW_TO_CHARGE;
    if (input[30] == '1')
        warning |= DC_DC_OVERCURRENT;

    input[34] = 0;
    faultCode = atoi(&input[32]);
}

/**
 * Read the next token, parse and put into a float variable.
 */
void Inverter::processFloat(float *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atof(token);
    }
}

/**
 * Read the next token, parse and put into a uint16_t variable.
 */
void Inverter::processInt(uint16_t *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atol(token);
    }
}

/**
 * Read the next token, parse and put into a uint8_t variable.
 */
void Inverter::processShort(uint8_t *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atoi(token);
    }
}

void Inverter::processStatus1()
{
    char *token = strtok(0, " ");
    status = 0;

    if (token[3] == '1')
        status |= LOAD;
    if (token[4] == '1')
        status |= BATTERY_VOLTAGE_TOO_STEADY;
    if (token[5] == '1')
        status |= CHARGING;
    if (token[6] == '1')
        status |= CHARGING_SOLAR;
    if (token[7] == '1')
        status |= CHARGING_GRID;
}

void Inverter::processStatus2()
{
    char *token = strtok(0, " ");

    if (token[0] == '1')
        status |= CHARGING_FLOATING;
    if (token[1] == '1')
        status |= SWITCHED_ON;
}

/**
 * Convert the actual values into a JSON string.
 */
String Inverter::toJSON()
{
    StaticJsonDocument<2048> jsonDoc;

    jsonDoc["time"] = millis() / 1000;

    JsonObject grid = jsonDoc.createNestedObject("grid");
    grid["voltage"] = gridVoltage;
    grid["frequency"] = gridFrequency;

    JsonObject out = jsonDoc.createNestedObject("out");
    out["voltage"] = outVoltage;
    out["frequency"] = outFrequency;
    out["powerApparent"] = outPowerApparent;
    out["powerActive"] = outPowerActive;
    out["load"] = outLoad;
    out["source"] = evalLoadSource();

    JsonObject battery = jsonDoc.createNestedObject("battery");
    battery["voltage"] = batteryVoltage;
    battery["voltageSCC"] = batteryVoltageSCC;
    battery["current"] = batteryCurrent;
    battery["power"] = batteryPower;
    battery["soc"] = batterySOC;
    battery["source"] = evalChargeSource();
    battery["floatCharge"] = (status & CHARGING_FLOATING ? "on" : "off");

    JsonObject pv = jsonDoc.createNestedObject("pv");
    pv["voltage"] = pvVoltage;
    pv["current"] = pvCurrent;
    pv["power"] = pvChargingPower;
    pv["maxPower"] = getMaximumSolarPower();
    pv["maxCurrent"] = getMaximumSolarCurrent();

    JsonObject system = jsonDoc.createNestedObject("system");
    system["version"] = eepromVersion;
    system["mode"] = modeString[mode];
    system["switch"] = (status & SWITCHED_ON ? "on" : "off");
    system["voltage"] = busVoltage;
    system["temperature"] = temperature;
    system["fanCurrent"] = fanCurrent;
    system["faultCode"] = faultCode;
    JsonArray warn = system.createNestedArray("warning");
    evalWarning(warn);

    String str;
    serializeJson(jsonDoc, str);
    return str;
}

String Inverter::evalChargeSource()
{
    if (status & CHARGING) {
        if ((status & CHARGING_SOLAR) && (status & CHARGING_GRID)) {
            return "Solar and Grid";
        } else if (status & CHARGING_SOLAR) {
            return "Solar";
        } else if (status & CHARGING_GRID) {
            return "Grid";
        }
    }
    return "-";
}

String Inverter::evalLoadSource()
{
    if (status & LOAD) {
        if (mode & LINE) {
            return "Grid";
        } else if (mode & BATTERY) {
            return "Battery";
        }
    }
    return "-";
}

void Inverter::evalWarning(JsonArray &array)
{
    if (warning & INVERTER_FAULT)
        array.add("Inverter fault");
    if (warning & BUS_OVERVOLTAGE)
        array.add("Bus over-voltage");
    if (warning & BUS_UNDERVOLTAGE)
        array.add("Bus under-voltage");
    if (warning & BUS_SOFT_FAIL)
        array.add("Bus soft fail");
    if (warning & GRID_FAIL)
        array.add("Grid fail");
    if (warning & OPV_SHORT)
        array.add("OPV short");
    if (warning & INVERTER_UNDERVOLTAGE)
        array.add("Inverter under-voltage");
    if (warning & INVERTER_OVERVOLTAGE)
        array.add("Inverter over-voltage");
    if (warning & OVER_TEMPERATURE)
        array.add("Over temperature");
    if (warning & FAN_LOCKED)
        array.add("Fan locked");
    if (warning & BATTERY_OVERVOLTAGE)
        array.add("Battery over-voltage");
    if (warning & BATTERY_UNDERVOLTAGE)
        array.add("Battery under-voltage");
    if (warning & BATTERY_OVERCHARGE)
        array.add("Battery over-charge");
    if (warning & BATTERY_SHUTDOWN)
        array.add("Battery shutdown");
    if (warning & BATTERY_DERATING)
        array.add("Battery derating");
    if (warning & OVER_LOAD)
        array.add("Over-load");
    if (warning & EEPROM_FAULT)
        array.add("EEPROM fault");
    if (warning & INVERTER_OVER_CURRENT)
        array.add("Inverter over-current");
    if (warning & INVERTER_SOFT_FAIL)
        array.add("Inverter soft fail");
    if (warning & SELF_TEST_FAIL)
        array.add("Self-test fail");
    if (warning & OP_DC_OVER_VOLTAGE)
        array.add("OP DC over-voltage");
    if (warning & BATTERY_OPEN)
        array.add("Battery open");
    if (warning & CURRENT_SENSOR_FAIL)
        array.add("Current sensor fail");
    if (warning & BATTERY_SHORT)
        array.add("Battery short");
    if (warning & POWER_LIMIT)
        array.add("Power limit");
    if (warning & PV_VOLTAGE_HIGH)
        array.add("PV voltage high");
    if (warning & MPPT_OVERLOAD_FAULT)
        array.add("MPPT over-load fault");
    if (warning & MPPT_OVERLOAD_WARNING)
        array.add("MPPT over-load warning");
    if (warning & BATTER_TOO_LOW_TO_CHARGE)
        array.add("Battery voltage to low to charge");
    if (warning & DC_DC_OVERCURRENT)
        array.add("DC/DC converter over-current");
}

/**
 * Calculate the maximum available solar power.
 * The goal is to use only PV input, no battery and no grid power.
 */
void Inverter::calculateMaximumSolarPower()
{
    if (outPowerActive > pvChargingPower + Config::pvOutPowerTolerance ||
            batteryCurrent < Config::maxBatteryDischargeCurrent ||
            busVoltage < Config::minBusVoltage ||
            pvVoltage < Config::minPvVoltage) {
        if (maxSolarPower >= Config::powerAdjustment && maxSolarPower > Config::minSolarPower) {
            maxSolarPower -= Config::powerAdjustment;
        } else {
            maxSolarPower = 0;
        }
    } else if (pvVoltage > Config::maxPvVoltage) {
        if (maxSolarPower < Config::maxSolarPower - Config::powerAdjustment) {
            maxSolarPower += Config::powerAdjustment;
        } else {
            maxSolarPower = Config::maxSolarPower;
        }
    }
}

/**
 * Return the calculated maximum power to restrict power input to PV (in Watt)
 */
uint16_t Inverter::getMaximumSolarPower()
{
    return maxSolarPower;
}

/**
 * Get the maximum applicable solar current in 0.1A
 */
uint16_t Inverter::getMaximumSolarCurrent()
{
    return maxSolarPower * 10 / (outVoltage > 0 ? outVoltage : 230);
}

