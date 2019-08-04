/*
 * Inverter.cpp
 *
 *  Created on: 13 Jul 2019
 *      Author: Michael Neuweiler
 */

#include "Inverter.h"

Inverter::Inverter()
{
    mode = off;
    status = acCharging;
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
    prefix = false;

    timestamp = 0;
}

Inverter::~Inverter()
{
}

/**
 * Get singleton instance
 */
Inverter* Inverter::getInstance()
{
    static Inverter instance;
    return &instance;
}

void Inverter::init()
{
    Serial.begin(2400);

    timestamp = millis();
}

void Inverter::loop()
{
    if (timestamp + 1000 > millis())
        return;

    if (readResponse()) {
        parseStatusResponse(input);
    }
    queryStatus();

    timestamp = millis();
}

void Inverter::sendCommand(String command) {
    String crc = CRCUtil::getCRC(command);

    Logger::info("sending command: %s", command.c_str());

    Serial.print(command);
    Serial.print(crc);
    Serial.write(13);
}

bool Inverter::readResponse()
{
    if (Serial.available()) {
        byte size = Serial.readBytes(input, INPUT_BUFFER_SIZE);
        input[size] = 0;
        return true;
//        return CRCUtil::checkCRC(String(input));
    }
    return false;
}

void Inverter::queryStatus()
{
   sendCommand("QPIGS");
}

/**
 * Example input: "(231.0 49.9 222.2 51.0 1800 1810 050 400 12.11 005 090 123 0003 430.1 12.22 11 22 33 99 00044 2"
 */
void Inverter::parseStatusResponse(char *input)
{
    if (input[0] != '(' || strlen(input) < 10 || strchr(input, ' ') == NULL) {
        Logger::info("unable to parse '%s", input);
    }

    char *token = strtok(input, " ");
    if (token != NULL) {
        uint16_t batteryChargeCurrent;
        uint16_t batteryDischargeCurrent;

        token++; // skip the "("
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
        processShort(&status);
        processInt(&fanCurrent);
        fanCurrent *= 10;
        processShort(&eepromVersion);
        processInt(&pvChargingPower);
        token = strtok(0, " ");
        if (token != NULL) {
            token[3] = 0;
            mode = atol(token);
        }
        batteryCurrent = (batteryDischargeCurrent > 0 ? batteryDischargeCurrent * -1 : batteryChargeCurrent);
        batteryPower = batteryCurrent * batteryVoltage;
    }
}

void Inverter::processFloat(float *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atof(token);
    }
}

void Inverter::processInt(uint16_t *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atol(token);
    }
}

void Inverter::processShort(uint8_t *value)
{
    char *token = strtok(0, " ");
    if (token != NULL && value != NULL) {
        *value = atoi(token);
    }
}

String Inverter::toJSON()
{
    StaticJsonDocument<1024> jsonDoc;

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

    JsonObject battery = jsonDoc.createNestedObject("battery");
    battery["voltage"] = batteryVoltage;
    battery["voltageSCC"] = batteryVoltageSCC;
    battery["current"] = batteryCurrent;
    battery["power"]= batteryPower;
    battery["soc"] = batterySOC;

    JsonObject pv = jsonDoc.createNestedObject("pv");
    pv["voltage"] = pvVoltage;
    pv["current"] = pvCurrent;
    pv["power"] = pvChargingPower;

    JsonObject system = jsonDoc.createNestedObject("system");
    system["status"] = status;
    system["mode"] = mode;
    system["version"] = eepromVersion;
    system["voltage"] = busVoltage;
    system["temperature"] = temperature;

    String str;
    serializeJson(jsonDoc, str);
    return str;
}
