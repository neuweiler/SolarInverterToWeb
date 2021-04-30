/*
 * WLAN.h
 *
 *  Created on: 4 Aug 2019
 *      Author: Michael Neuweiler
 */

#ifndef WLAN_H_
#define WLAN_H_

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>

#include "Logger.h"
#include "Inverter.h"

#define LED_STATION D5
#define LED_AP D6

class WLAN
{
public:
    WLAN();
    virtual ~WLAN();
    void init();
    void loop();

private:
    void checkConnection();
    void setupOTA();

    uint32_t lastConnectionAttempt;
    bool apConnected, stationConnected;
};

extern WLAN wlan;

#endif /* WLAN_H_ */
