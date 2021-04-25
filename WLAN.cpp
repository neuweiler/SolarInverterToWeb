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
    timestamp = 0;
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
    pinMode(LED_STATION, OUTPUT);
    pinMode(LED_AP, OUTPUT);

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    WiFi.mode(WIFI_AP_STA); // set as AP and as client
    WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    logger.info("started WiFi Station for SSID %s", config.wifiSsid.c_str());

    // setup own AccessPoint
    WiFi.hostname("solar");
    WiFi.softAP(config.wifiApSsid.c_str(), config.wifiApPassword.c_str());
    logger.info("started WiFi AP %s on ip %s", config.wifiApSsid.c_str(), WiFi.softAPIP().toString().c_str());

    MDNS.begin(config.wifiApSsid.c_str());
}

/**
 * The main processing logic.
 */
void WLAN::loop()
{
    if (millis() - timestamp < config.wifiUpdateInterval) {
        return;
    }

    checkConnection();
    MDNS.update();

    timestamp = millis();
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
        if (millis() - lastConnectionAttempt > config.wifiReconnectInterval) {
            logger.info("attempting to (re)connect to %s", config.wifiSsid.c_str());
            WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
            lastConnectionAttempt = millis();
        }
    }
    apConnected = WiFi.softAPgetStationNum() > 0;

    // indicate connection status via LEDs
    digitalWrite(LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(LED_AP, (apConnected ? HIGH : LOW));
}

WLAN wlan;

