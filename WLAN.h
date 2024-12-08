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
#include <lwip/napt.h>
#include <lwip/dns.h>
//#include <dhcpserver.h>

#include "Logger.h"
#include "Inverter.h"

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
    bool mode(WiFiMode_t m);

    void checkConnection();
	void setupStation();
	void setupAccessPoint();
	void setupNAT();

    uint32_t lastConnectionAttempt;
    bool isConnected;
};

extern WLAN wlan;

#endif /* WLAN_H_ */
