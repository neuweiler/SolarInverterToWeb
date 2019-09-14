/*
 * Config.cpp
 *
 *  Created on: 3 Sep 2019
 *      Author: Michael Neuweiler
 */

#include "Config.h"

const char *Config::CONFIG_FILE = "/config.json";
uint16_t Config::initialSolarPower;
uint16_t Config::inverterUpdateInterval;
uint16_t Config::pvOutPowerTolerance;
int16_t Config::maxBatteryDischargeCurrent;
int16_t Config::minBusVoltage;
float Config::minPvVoltage;
float Config::maxPvVoltage;
uint16_t Config::powerAdjustment;
uint16_t Config::minSolarPower;
uint16_t Config::maxSolarPower;
uint32_t Config::cutoffRetryTime;
uint8_t Config::cutoffRetryMinBatterySoc;
String Config::stationSsid;
String Config::stationPassword;
uint16_t Config::stationUpdateInterval;
uint16_t Config::stationReconnectInterval;
bool Config::outputAsCurrent;
String Config::stationRequestPrefix;
String Config::stationRequestPostfix;
String Config::serverSsid;
String Config::serverPassword;

void Config::init()
{
    SPIFFS.begin();
    load();
    print();
}

void Config::load() {
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        Logger::error("Failed to open %s", CONFIG_FILE);
    }

    StaticJsonDocument < 1024 > doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Logger::error("Failed to parse file, using default configuration");

    inverterUpdateInterval = doc["inverter"]["interval"] | 300;
    initialSolarPower = doc["inverter"]["pv"]["power"]["initial"] | 1000;
    pvOutPowerTolerance = doc["inverter"]["pv"]["power"]["tolerance"] | 100;
    minSolarPower = doc["inverter"]["pv"]["power"]["min"] | 400;
    maxSolarPower = doc["inverter"]["pv"]["power"]["max"] | 3000;
    powerAdjustment = doc["inverter"]["pv"]["power"]["adjustmentStep"] | 25;
    minPvVoltage = doc["inverter"]["pv"]["voltage"]["min"] | 320.0f;
    maxPvVoltage = doc["inverter"]["pv"]["voltage"]["max"] | 325.0f;
    maxBatteryDischargeCurrent = (doc["inverter"]["battery"]["dischargeCurrent"]["max"] | 4) * -1;
    minBusVoltage = doc["inverter"]["bus"]["voltage"]["min"] | 390;
    cutoffRetryTime = doc["inverter"]["cutoffRetry"]["time"] | 300;
    cutoffRetryMinBatterySoc = doc["inverter"]["cutoffRetry"]["minBatterySoc"] | 50;

    stationSsid = String(doc["station"]["ssid"] | "myStation");
    stationPassword = String(doc["station"]["password"] | "stationPasswd");
    stationUpdateInterval = doc["station"]["interval"]["update"] | 2000;
    stationReconnectInterval = doc["station"]["interval"]["reconnect"] | 15000;
    outputAsCurrent = doc["station"]["request"]["outputAsCurrent"] | true;
    stationRequestPrefix = String(doc["station"]["request"]["prefix"] | "http://192.168.3.10/?maximumSolarCurrent=");
    stationRequestPostfix = String(doc["station"]["request"]["postfix"] | "");

    serverSsid = String(doc["server"]["ssid"] | "solar");
    serverPassword = String(doc["server"]["password"] | "inverter");

    file.close();
}

void Config::print()
{
    Logger::console("inverterUpdateInterval : %d", inverterUpdateInterval);
    Logger::console("initialSolarPower: %d", initialSolarPower);
    Logger::console("pvOutPowerTolerance: %d", pvOutPowerTolerance);
    Logger::console("minSolarPower: %d", minSolarPower);
    Logger::console("maxSolarPower: %d", maxSolarPower);
    Logger::console("powerAdjustment: %d", powerAdjustment);
    Logger::console("minPvVoltage: %f", minPvVoltage);
    Logger::console("maxPvVoltage: %f", maxPvVoltage);
    Logger::console("maxBatteryDischargeCurrent: %d", maxBatteryDischargeCurrent);
    Logger::console("minBusVoltage: %d", minBusVoltage);
    Logger::console("cutoffRetryTime: %d", cutoffRetryTime);
    Logger::console("cutoffRetryMinBatterySoc: %d", cutoffRetryMinBatterySoc);

    Logger::console("stationSsid: %s", stationSsid.c_str());
    Logger::console("stationPassword: %s", stationPassword.c_str());
    Logger::console("stationUpdateInterval: %d", stationUpdateInterval);
    Logger::console("stationReconnectInterval: %d", stationReconnectInterval);
    Logger::console("outputAsCurrent: %d", outputAsCurrent);
    Logger::console("stationRequestPrefix: %s", stationRequestPrefix.c_str());
    Logger::console("stationRequestPostfix: %s", stationRequestPostfix.c_str());

    Logger::console("serverSsid: %s", serverSsid.c_str());
    Logger::console("serverPassword: %s", serverPassword.c_str());
}
