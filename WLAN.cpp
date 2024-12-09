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
WLAN::WLAN() {
	lastConnectionAttempt = 0;
	isConnected = false;
}

WLAN::~WLAN() {
}

/**
 * Initialize the WiFi client and output ports.
 */
void WLAN::init() {
	pinMode(PIN_LED_WIFI_CONNECTED, OUTPUT);

	uint8_t wifiMode = WIFI_AP_STA; // act as AccessPoint or Station
	if (config.wifiStationSsid[0] == 0) {
		wifiMode = WIFI_AP; // act as AccessPoint only
	}
	if (config.wifiApSsid[0] == 0) {
		wifiMode = WIFI_STA; // act as Station only
	}

	WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
	WiFi.hostname(config.wifiHostname);

	if (wifiMode == WIFI_AP_STA || wifiMode == WIFI_STA) {
		setupStation();
	}
	if (wifiMode == WIFI_AP_STA || wifiMode == WIFI_AP) {
		setupAccessPoint();
	}

	if (config.wifiApNAT && wifiMode == WIFI_AP_STA) {
		setupNAT();
	}
}

/**
 * The main processing logic.
 */
void WLAN::loop() {
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
void WLAN::checkConnection() {
	if (WiFi.isConnected()) {
		isConnected = true;
	} else {
		isConnected = false;

		// we try to (re)establish connection every 15sec, this allows softAP to work (although it gets blocked for 1-2sec)
		if (config.wifiStationSsid[0] && millis() - lastConnectionAttempt > config.wifiStationReconnectInterval) {
			logger.info(F("attempting to (re)connect to %s"), config.wifiStationSsid);
			WiFi.reconnect();
			lastConnectionAttempt = millis();
		}
	}

	// indicate connection status via LED
	digitalWrite(PIN_LED_WIFI_CONNECTED, (isConnected ? HIGH : LOW));
}

void WLAN::setupStation() {
	WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
	logger.info(F("Wifi: connecting to access point %s"), config.wifiStationSsid);
	WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);

	uint8_t i = 60;
	while (!WiFi.isConnected() && i-- > 0) {
		logger.console(F("waiting (%d)..."), i);
		delay(500);
	}
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

	logger.info(F("started WiFi AP %s on ip %s, channel %d"), config.wifiApSsid, WiFi.softAPIP().toString().c_str(),
			config.wifiApChannel);
}

/**
 * Enable the forwarding of traffic between this AccessPoint and the Station's AccessPoint so the ESP can also be
 * used as an extender.
 *
 * Warning: Operation over longer period of time not stable - not recommended !
 */
void WLAN::setupNAT() {
	err_t ret = ip_napt_init(NAPT, NAPT_PORT);
	logger.debug(F("ip_napt_init: %d (OK=%d)"), ret);
	if (ret == ERR_OK) {
		ret = ip_napt_enable_no(SOFTAP_IF, 1);
		logger.debug(F("ip_napt_enable_no: %d"), ret);
	}
	if (ret != ERR_OK) {
		logger.error(F("NAPT initialization failed\n"));
	}

	if (WiFi.isConnected()) { // give station's DNS servers to AP side (for NAT)
//		dhcps_set_dns(0, WiFi.dnsIP(0));
//		dhcps_set_dns(1, WiFi.dnsIP(1));
	}
}

WLAN wlan;
