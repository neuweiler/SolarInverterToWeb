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
    pinMode(LED_STATION, OUTPUT);
    pinMode(LED_AP, OUTPUT);

    WiFi.persistent(false); // prevent flash memory wear ! (https://github.com/esp8266/Arduino/issues/1054)
    if (config.wifiStationSsid[0]) {
		setupStation();
    } else {
        WiFi.mode(WIFI_AP); // set as AP only
    }

    WiFi.hostname(config.wifiHostname);

	setupAccessPoint();
	setupNAT();
    setupOTA();
}

/**
 * The main processing logic.
 */
void WLAN::loop()
{
    ArduinoOTA.handle();
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
    digitalWrite(LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(LED_AP, (apConnected ? HIGH : LOW));
}

void WLAN::setupStation() {
	WiFi.mode(WIFI_AP_STA); // set as AP and as client
	WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
	WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);

	uint8_t i = 60;
	while (!WiFi.isConnected() && i-- > 0) {
		logger.console(".");
		delay(500);
	}

	if (WiFi.isConnected()) {
	    // give DNS servers to AP side (for NAT)
		dhcps_set_dns(0, WiFi.dnsIP(0));
		dhcps_set_dns(1, WiFi.dnsIP(1));
	}

	logger.info(F("started WiFi Station: %s (dns: %s / %s)\n"), WiFi.localIP().toString().c_str(),
			WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str());
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
}

void WLAN::setupOTA()
{
	ArduinoOTA.setHostname(config.wifiHostname);

	ArduinoOTA.onStart([]() {
        logger.info(F("Start updating %s"), (ArduinoOTA.getCommand() == U_FLASH ? "flash" : "filesystem"));
    });
	ArduinoOTA.onEnd([]() {
        logger.info(F("Update finished"));
    });
	ArduinoOTA.onError([](ota_error_t error) {
        logger.error("Update Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) logger.error("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) logger.error("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) logger.error("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) logger.error("Receive Failed");
        else if (error == OTA_END_ERROR) logger.error("End Failed");
    });

	ArduinoOTA.begin(true);
    logger.info(F("OTA initialized"));
}

WLAN wlan;
