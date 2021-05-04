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
	LittleFS.begin();
    load();
    print();
}

void Config::load() {
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        logger.error("Failed to open %s", CONFIG_FILE);
    }

    StaticJsonDocument < 2048 > doc;
    DeserializationError error = deserializeJson(doc, file);
	if (error) {
		logger.error("could not parse config: %s", error.f_str());
	}

    inverterInterval = doc["inverter"]["interval"] | 300;
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
    batteryOverDischargeProtection = doc["battery"]["overDischargeProtection"] | false;
    batterySocCalculateInternally = doc["battery"]["soc"]["calculateInternally"] | true;
    batteryRestDuration = doc["battery"]["soc"]["restDuration"] | 5;
    batteryRestCurrent = doc["battery"]["soc"]["restCurrent"] | 10;
    batterySocTriggerFloatOverride = doc["battery"]["soc"]["triggerFloatOverride"] | 0;

    wifiHostname = doc["wifi"]["hostname"] | "solar";
    wifiStationSsid = doc["wifi"]["station"]["ssid"] | "";
    wifiStationPassword = doc["wifi"]["station"]["password"] | "";
    wifiStationReconnectInterval = doc["wifi"]["station"]["reconnectInterval"] | 15000;
    wifiApSsid = doc["wifi"]["ap"]["ssid"] | "solar";
    wifiApPassword = doc["wifi"]["ap"]["password"] | "inverter";
    wifiApChannel = doc["wifi"]["ap"]["channel"] | 13;
    wifiApAddress= doc["wifi"]["ap"]["address"] | "192.168.4.1";
    wifiApGateway= doc["wifi"]["ap"]["gateway"] | "192.168.4.1";
    wifiApNetmask= doc["wifi"]["ap"]["netmask"] | "255.255.255.0";

    file.close();
}

void Config::print()
{
    logger.console("inverterInterval : %dms", inverterInterval);
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

    logger.console("wifiHostname: %s", wifiHostname);
    logger.console("wifiStationSsid: %s", wifiStationSsid);
    logger.console("wifiStationPassword: %s", wifiStationPassword);
    logger.console("wifiStationReconnectInterval: %d", wifiStationReconnectInterval);
    logger.console("wifiApSsid: %s", wifiApSsid);
    logger.console("wifiApPassword: %s", wifiApPassword);
    logger.console("wifiApChannel: %d", wifiApChannel);
    logger.console("wifiApAddress: %s", wifiApAddress);
    logger.console("wifiApGateway: %s", wifiApGateway);
    logger.console("wifiApNetmask: %s", wifiApNetmask);
}

Config config;
