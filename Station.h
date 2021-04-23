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
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include "Logger.h"
#include "Inverter.h"

#define LED_STATION D5
#define LED_AP D6

class Station
{
public:
    Station();
    virtual ~Station();
    void init();
    void loop();

private:
    void checkConnection();

    uint32_t timestamp;
    uint32_t lastConnectionAttempt;
    bool apConnected, stationConnected;
};

extern Station station;

#endif /* STATION_H_ */
