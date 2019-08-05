/*
 * Station.cpp
 *
 *  Created on: 4 Aug 2019
 *      Author: Michael Neuweiler
 */

#include "Station.h"

Station::Station()
{
    timestamp = 0;
    lastConnectionAttempt = 0;
    connectionError = 0;
    apConnected = false;
    stationConnected = false;
}

Station::~Station()
{
}

/**
 * Get singleton instance
 */
Station* Station::getInstance()
{
    static Station instance;
    return &instance;
}

void Station::init()
{
    pinMode(LED_STATION, OUTPUT);
    pinMode(LED_AP, OUTPUT);

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    WiFi.mode(WIFI_AP_STA); // set as AP and as client
    WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
    WiFi.begin(STATION_SSID, STATION_PASSWORD);
    Logger::info("started WiFi Station for SSID %s", STATION_SSID);
}

void Station::loop()
{
    if (millis() - timestamp < 2000) {
        return;
    }

    checkConnection();
    if (stationConnected) {
        sendUpdate();
    }
    timestamp = millis();
}

void Station::checkConnection()
{
    if (WiFi.isConnected()) {
        stationConnected = true;
    } else {
        stationConnected = false;
        // we try to (re)establish connection every 15sec, this allows softAP to work (although it gets blocked for 1-2sec)
        if (millis() - lastConnectionAttempt > 15000) {
            Logger::info("attempting to (re)connect to %s", STATION_SSID);
            WiFi.begin(STATION_SSID, STATION_PASSWORD);
            lastConnectionAttempt = millis();
        }
    }
    apConnected = WiFi.softAPgetStationNum() > 0;

    // indicate connection status via LEDs
    digitalWrite(LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(LED_AP, (apConnected ? HIGH : LOW));
}

void Station::sendUpdate()
{
    HTTPClient http;
    String requestUrl;

    requestUrl.concat(BASE_URL);
    requestUrl.concat(Inverter::getInstance()->getMaximumSolarCurrent());
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
