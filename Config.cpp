/*
 * Config.cpp
 *
 *  Created on: 3 Sep 2019
 *      Author: Michael Neuweiler
 */

#include "Config.h"

const char *Config::CONFIG_FILE = "/config.json";

void Config::init()
{
    SPIFFS.begin();
    load();
    print();
}

void Config::load() {
    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        logger.error("Failed to open %s", CONFIG_FILE);
    }

    StaticJsonDocument < 1024 > doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        logger.error("Failed to parse file, using default configuration");

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


    batteryCapacity = doc["battery"]["capacity"] | 100;
    batteryType = doc["battery"]["type"] | BatteryType::LiIon;
    batteryVoltageFullCharge = doc["battery"]["voltage"]["full"] | 28.4f;
    batteryVoltageNominal = doc["battery"]["voltage"]["nominal"] | 25.6f;
    batteryVoltageEmpty = doc["battery"]["voltage"]["empty"] | 21.6f;
    batteryVoltageFloat = doc["battery"]["voltage"]["float"] | 24.5f;
    overDischargeProtection = doc["battery"]["overDischargeProtection"] | false;
    batterySocCalculateInternally = doc["battery"]["soc"]["calculateInternally"] | true;
    batteryRestDuration = doc["battery"]["soc"]["restDuration"] | 5;
    batteryRestCurrent = doc["battery"]["soc"]["restCurrent"] | 10;
    batterySocTriggerFloatOverride = doc["battery"]["soc"]["triggerFloatOverride"] | 0;

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
    logger.console("inverterUpdateInterval : %dms", inverterUpdateInterval);
    logger.console("initialSolarPower: %dW", initialSolarPower);
    logger.console("pvOutPowerTolerance: %dW", pvOutPowerTolerance);
    logger.console("minSolarPower: %dW", minSolarPower);
    logger.console("maxSolarPower: %dW", maxSolarPower);
    logger.console("powerAdjustment: %dW", powerAdjustment);
    logger.console("minPvVoltage: %fV", minPvVoltage);
    logger.console("maxPvVoltage: %fV", maxPvVoltage);
    logger.console("maxBatteryDischargeCurrent: %dA", maxBatteryDischargeCurrent);
    logger.console("minBusVoltage: %dV", minBusVoltage);
    logger.console("cutoffRetryTime: %dsec", cutoffRetryTime);
    logger.console("cutoffRetryMinBatterySoc: %d%%", cutoffRetryMinBatterySoc);

    logger.console("batteryCapacity: %dAh", batteryCapacity);
    logger.console("batteryType: %d", batteryType);
    logger.console("batteryVoltageFullCharge: %fV", batteryVoltageFullCharge);
    logger.console("batteryVoltageNominal: %fV", batteryVoltageNominal);
    logger.console("batteryVoltageEmpty: %fV", batteryVoltageEmpty);
    logger.console("batteryVoltageFloat: %fV", batteryVoltageFloat);
    logger.console("batterySocTriggerFloatOverride: %f%%", batterySocTriggerFloatOverride);
    logger.console("batteryRestDuration: %dsec", batteryRestDuration);
    logger.console("batteryRestCurrent: %dA", batteryRestCurrent);

    logger.console("stationSsid: %s", stationSsid.c_str());
    logger.console("stationPassword: %s", stationPassword.c_str());
    logger.console("stationUpdateInterval: %d", stationUpdateInterval);
    logger.console("stationReconnectInterval: %d", stationReconnectInterval);
    logger.console("outputAsCurrent: %d", outputAsCurrent);
    logger.console("stationRequestPrefix: %s", stationRequestPrefix.c_str());
    logger.console("stationRequestPostfix: %s", stationRequestPostfix.c_str());

    logger.console("serverSsid: %s", serverSsid.c_str());
    logger.console("serverPassword: %s", serverPassword.c_str());
}

Config config;

