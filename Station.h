/*
 * Station.h
 *
 *  Created on: 4 Aug 2019
 *      Author: Michael Neuweiler
 */

#ifndef STATION_H_
#define STATION_H_

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "Logger.h"
#include "Inverter.h"

#define STATION_SSID "GEVCU"
#define STATION_PASSWORD "verysecret"
#define BASE_URL "http://192.168.3.10/?maximumSolarCurrent="

#define LED_STATION D5
#define LED_AP D6

class Station
{
public:
    static Station* getInstance();
    void init();
    void loop();

private:
    Station();
    virtual ~Station();
    void operator=(Station const&);
    void sendUpdate();
    void checkConnection();

    uint32_t timestamp;
    uint32_t lastConnectionAttempt;
    uint8_t connectionError;
    bool apConnected, stationConnected;
};

#endif /* STATION_H_ */
