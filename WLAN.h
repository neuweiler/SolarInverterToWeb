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
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <dhcpserver.h>

#include "Logger.h"
#include "Inverter.h"

#define LED_STATION D5
#define LED_AP D6
#define NAPT 1000
#define NAPT_PORT 10

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
	void setupStation();
	void setupAccessPoint();
	void setupNAT();

    uint32_t lastConnectionAttempt;
    bool apConnected, stationConnected;
};

extern WLAN wlan;

#endif /* WLAN_H_ */
