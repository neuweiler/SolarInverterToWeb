/*
 * WLAN.cpp
 *
 * This class handles the connection to a consumer's WLAN and sends
 * the maximum available power to it. The purpose is to keep it's
 * consumption at or below what the PV panels are able to deliver.
 * So no power from a connected battery or grid is used.
 * This is useful e.g. when charging an electric car.
 *
 * When a client is connected to our AP, the output D5 goes high,
 * allowing an LED to signal a connected client. When this device is
 * connected to the consumer's WLAN, D6 will go high.
 *
 *  Created on: 4 Aug 2019
 *      Author: Michael Neuweiler
 */

#include "WLAN.h"

/**
 * Constructor
 */
WLAN::WLAN()
{
    lastConnectionAttempt = 0;
    apConnected = false;
    stationConnected = false;
}

WLAN::~WLAN()
{
}

/**
 * Initialize the WiFi client and output ports.
 */
void WLAN::init()
{
    pinMode(PIN_LED_STATION, OUTPUT);
    pinMode(PIN_LED_AP, OUTPUT);

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    if (config.wifiStationSsid[0]) {
		setupStation();
    } else {
        WiFi.mode(WIFI_AP); // set as AP only
    }

    WiFi.hostname(config.wifiHostname);

	setupAccessPoint();
}

/**
 * The main processing logic.
 */
void WLAN::loop()
{
    checkConnection();
}

/**
 * Check if a connection to or from a device is established
 * and set digital ports high/low accordingly. Tries to (re)establish
 * a connection to the target WLAN.
 *
 * As in dual mode (WIFI_AP_STA) the auto reconnect has to be
 * disabled, we need to manually try to connect to the AP every 15sec.
 */
void WLAN::checkConnection()
{
    if (WiFi.isConnected()) {
        stationConnected = true;
    } else {
        stationConnected = false;
        // we try to (re)establish connection every 15sec, this allows softAP to work (although it gets blocked for 1-2sec)
        if (config.wifiStationSsid[0] && millis() - lastConnectionAttempt > config.wifiStationReconnectInterval) {
            logger.info(F("attempting to (re)connect to %s"), config.wifiStationSsid);
            WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);
            lastConnectionAttempt = millis();
        }
    }
    apConnected = WiFi.softAPgetStationNum() > 0;

    // indicate connection status via LEDs
    digitalWrite(PIN_LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(PIN_LED_AP, (apConnected ? HIGH : LOW));
}

void WLAN::setupStation() {
	WiFi.mode(WIFI_AP_STA); // set as AP and as client
	WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
	WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);
	logger.info(F("started WiFi Station: %s"), WiFi.localIP().toString().c_str());
}

void WLAN::setupAccessPoint() {
	IPAddress localIp;
	localIp.fromString(config.wifiApAddress);
	IPAddress gateway;
	gateway.fromString(config.wifiApGateway);
	IPAddress subnet;
	subnet.fromString(config.wifiApNetmask);

	WiFi.softAPConfig(localIp, gateway, subnet);
	WiFi.softAP(config.wifiApSsid, config.wifiApPassword, config.wifiApChannel);
	delay(100); // wait for SYSTEM_EVENT_AP_START

	logger.info(F("started WiFi AP %s on ip %s, channel %d"), config.wifiApSsid, WiFi.softAPIP().toString().c_str(), config.wifiApChannel);
}

WLAN wlan;
