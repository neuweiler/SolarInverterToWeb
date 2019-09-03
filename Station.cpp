/*
 * Station.cpp
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

#include "Station.h"

/**
 * Constructor
 */
Station::Station()
{
    timestamp = 0;
    lastConnectionAttempt = 0;
    connectionError = 0;
    apConnected = false;
    stationConnected = false;
    oldOutput = 0;
}

Station::~Station()
{
}

/**
 * Get singleton instance.
 */
Station* Station::getInstance()
{
    static Station instance;
    return &instance;
}

/**
 * Initialize the WiFi client and output ports.
 */
void Station::init()
{
    pinMode(LED_STATION, OUTPUT);
    pinMode(LED_AP, OUTPUT);

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    WiFi.mode(WIFI_AP_STA); // set as AP and as client
    WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
    WiFi.begin(Config::stationSsid.c_str(), Config::stationPassword.c_str());
    Logger::info("started WiFi Station for SSID %s", Config::stationSsid.c_str());
}

/**
 * The main processing logic.
 */
void Station::loop()
{
    if (millis() - timestamp < Config::stationUpdateInterval) {
        return;
    }

    checkConnection();
    if (stationConnected) {
        sendUpdate();
    }
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
void Station::checkConnection()
{
    if (WiFi.isConnected()) {
        stationConnected = true;
    } else {
        stationConnected = false;
        // we try to (re)establish connection every 15sec, this allows softAP to work (although it gets blocked for 1-2sec)
        if (millis() - lastConnectionAttempt > Config::stationReconnectInterval) {
            Logger::info("attempting to (re)connect to %s", Config::stationSsid.c_str());
            WiFi.begin(Config::stationSsid.c_str(), Config::stationPassword.c_str());
            lastConnectionAttempt = millis();
        }
    }
    apConnected = WiFi.softAPgetStationNum() > 0;

    // indicate connection status via LEDs
    digitalWrite(LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(LED_AP, (apConnected ? HIGH : LOW));
}

/**
 * Create GET request and send maximum solar current to target device.
 */
void Station::sendUpdate()
{
    Inverter *inverter = Inverter::getInstance();
    inverter->calculateMaximumSolarPower();
    uint16_t maxOutput = (Config::consumerOutputAsCurrent ? inverter->getMaximumSolarCurrent() : inverter->getMaximumSolarPower());
    if (oldOutput == maxOutput) {
        return;
    }
    oldOutput = maxOutput;

    HTTPClient http;
    String requestUrl;

    requestUrl.concat(Config::stationRequestPrefix);
    requestUrl.concat(maxOutput);
    requestUrl.concat(Config::stationRequestPostfix);
    http.begin(requestUrl);

    Logger::debug("sending request %s", requestUrl.c_str());
    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            Logger::debug("repsonse: OK");
        }
    } else {
        Logger::error("request failed: %s", http.errorToString(httpCode).c_str());
        connectionError++;
        if (connectionError > 3) {
            WiFi.disconnect(false); // otherwise WiFi doesn't realize that the connection was lost
            WiFi.mode(WIFI_AP_STA);
            connectionError = 0;
        }
    }
    http.end();
}
