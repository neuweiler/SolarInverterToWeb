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
}

void Config::load() {
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        logger.error("Failed to open %s", CONFIG_FILE);
    }

    StaticJsonDocument < 2048 > doc;
    DeserializationError error = deserializeJson(doc, file);
	if (error) {
		logger.error(F("could not parse config: %s"), error.f_str());
	}

    inverterInterval = doc[F("inverter")][F("interval")] | 300;
    initialSolarPower = doc[F("inverter")][F("pv")][F("power")][F("initial")] | 1000;
    pvOutPowerTolerance = doc[F("inverter")][F("pv")][F("power")][F("tolerance")] | 100;
    minSolarPower = doc[F("inverter")][F("pv")][F("power")][F("min")] | 400;
    maxSolarPower = doc[F("inverter")][F("pv")][F("power")][F("max")] | 3000;
    powerAdjustment = doc[F("inverter")][F("pv")][F("power")][F("adjustmentStep")] | 25;
    minPvVoltage = doc[F("inverter")][F("pv")][F("voltage")][F("min")] | 320.0f;
    maxPvVoltage = doc[F("inverter")][F("pv")][F("voltage")][F("max")] | 325.0f;
    maxBatteryDischargeCurrent = (doc[F("inverter")][F("battery")][F("dischargeCurrent")][F("max")] | 4) * -1;
    minBusVoltage = doc[F("inverter")][F("bus")][F("voltage")][F("min")] | 390;
    cutoffRetryTime = doc[F("inverter")][F("cutoffRetry")][F("time")] | 300;
    inputOverrideActivateSOC = doc[F("inverter")][F("inputOverride")][F("activateSoc")] | 20;
    inputOverrideDeactivateSOC = doc[F("inverter")][F("inputOverride")][F("deactivateSoc")] | 50;

    batteryCapacity = doc[F("battery")][F("capacity")] | 100;
    batteryType = doc[F("battery")][F("type")] | BatteryType::LiIon;
    batteryVoltageFullCharge = doc[F("battery")][F("voltage")][F("full")] | 28.4f;
    batteryVoltageNominal = doc[F("battery")][F("voltage")][F("nominal")] | 25.6f;
    batteryVoltageEmpty = doc[F("battery")][F("voltage")][F("empty")] | 21.6f;
    batteryVoltageFloat = doc[F("battery")][F("voltage")][F("float")] | 24.5f;
    batteryOverDischargeProtection = doc[F("battery")][F("overDischargeProtection")] | false;
    batterySocCalculateInternally = doc[F("battery")][F("soc")][F("calculateInternally")] | true;
    batteryRestDuration = doc[F("battery")][F("soc")][F("restDuration")] | 5;
    batteryRestCurrent = doc[F("battery")][F("soc")][F("restCurrent")] | 10;
    batterySocTriggerFloatOverride = doc[F("battery")][F("soc")][F("triggerFloatOverride")] | 0;

    wifiHostname = doc[F("wifi")][F("hostname")] | "solar";
    wifiStationSsid = doc[F("wifi")][F("station")][F("ssid")] | "";
    wifiStationPassword = doc[F("wifi")][F("station")][F("password")] | "";
    wifiStationReconnectInterval = doc[F("wifi")][F("station")][F("reconnectInterval")] | 15000;
    wifiApSsid = doc[F("wifi")][F("ap")][F("ssid")] | "solar";
    wifiApPassword = doc[F("wifi")][F("ap")][F("password")] | "inverter";
    wifiApChannel = doc[F("wifi")][F("ap")][F("channel")] | 13;
    wifiApAddress = doc[F("wifi")][F("ap")][F("address")] | "192.168.4.1";
    wifiApGateway = doc[F("wifi")][F("ap")][F("gateway")] | "192.168.4.1";
    wifiApNetmask = doc[F("wifi")][F("ap")][F("netmask")] | "255.255.255.0";
    wifiApNAT= doc[F("wifi")][F("ap")][F("NAT")] | false;

    file.close();
}

Config config;
