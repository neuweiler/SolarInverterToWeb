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
    batteryChargeCurrent = 0;
    batteryDischargeCurrent = 0;
    batterySOC = 0;
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

    Logger::debug("initializing inverter communication");
    Serial.print("QPI\r");
    delay(100);
    readResponse();
    Serial.print("QPIRI\r");
    delay(100);
    readResponse();
    Serial.print("QPI\r");
    delay(100);
    readResponse();
    Serial.print("QMN\r");
    delay(100);
    readResponse();
    Serial.print("QPIWS\r");
    delay(100);
    readResponse();

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

bool Inverter::readResponse()
{
    if (Serial.available()) {
        byte size = Serial.readBytes(input, INPUT_BUFFER_SIZE);
        input[size] = 0;
        Logger::debug("received %d bytes: %s", size, input);
        return true;
    }
    return false;
}

void Inverter::queryStatus()
{
    Logger::debug("query inverter status");
    Serial.print("QPIGS\r");
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
    String str;

    str += "{";
    str += "\"grid\": {";
    prefix = false;
    str += jsonElement("voltage", gridVoltage);
    str += jsonElement("frequency", gridFrequency);
    str += "},\"out\": {";
    prefix = false;
    str += jsonElement("voltage", outVoltage);
    str += jsonElement("frequency", outFrequency);
    str += jsonElement("powerApparent", outPowerApparent);
    str += jsonElement("powerActive", outPowerActive);
    str += jsonElement("load", outLoad);
    str += "},\"battery\": {";
    prefix = false;
    str += jsonElement("voltage", batteryVoltage);
    str += jsonElement("voltageSCC", batteryVoltageSCC);
    str += jsonElement("currentCharge", batteryChargeCurrent);
    str += jsonElement("currentDischarge", batteryDischargeCurrent);
    str += jsonElement("soc", batterySOC);
    str += "},\"pv\": {";
    prefix = false;
    str += jsonElement("voltage", pvVoltage);
    str += jsonElement("current", pvCurrent);
    str += jsonElement("power", pvChargingPower);
    str += "},\"system\": {";
    prefix = false;
    str += jsonElement("status", status);
    str += jsonElement("mode", mode);
    str += jsonElement("version", eepromVersion);
    str += jsonElement("voltage", busVoltage);
    str += jsonElement("temperature", temperature);
    str += jsonElement("fanCurrent", fanCurrent);
    str += "}";
    str += "}";

    return str;
}

String Inverter::jsonElement(String name, uint8_t value)
{
    snprintf(buffer, 79, "%s\"%s\": %d", (prefix ? "," : ""), name.c_str(), value);
    prefix = true;
    return buffer;
}

String Inverter::jsonElement(String name, uint16_t value)
{
    snprintf(buffer, 79, "%s\"%s\": %d", (prefix ? "," : ""), name.c_str(), value);
    prefix = true;
    return buffer;
}

String Inverter::jsonElement(String name, float value)
{
    snprintf(buffer, 79, "%s\"%s\": %.1f", (prefix ? "," : ""), name.c_str(), value);
    prefix = true;
    return buffer;
}

