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
        WiFi.mode(WIFI_AP_STA); // set as AP and as client
        WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
        WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);
        logger.info("started WiFi Station for SSID %s", config.wifiStationSsid);
    } else {
        WiFi.mode(WIFI_AP); // set as AP only
    }

    // setup own AccessPoint
    WiFi.hostname(config.wifiHostname);
    WiFi.softAP(config.wifiApSsid, config.wifiApPassword, config.wifiApChannel);
	delay(100); // wait for SYSTEM_EVENT_AP_START

    IPAddress localIp;
    localIp.fromString(config.wifiApAddress);
    IPAddress gateway;
    gateway.fromString(config.wifiApGateway);
    IPAddress subnet;
    subnet.fromString(config.wifiApNetmask);
    WiFi.softAPConfig(localIp, gateway, subnet);
    logger.info("started WiFi AP %s on ip %s, channel %d", config.wifiApSsid, WiFi.softAPIP().toString().c_str(), config.wifiApChannel);

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
            logger.info("attempting to (re)connect to %s", config.wifiStationSsid);
            WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);
            lastConnectionAttempt = millis();
        }
    }
    apConnected = WiFi.softAPgetStationNum() > 0;

    // indicate connection status via LEDs
    digitalWrite(LED_STATION, (stationConnected ? HIGH : LOW));
    digitalWrite(LED_AP, (apConnected ? HIGH : LOW));
}

void WLAN::setupOTA()
{
	ArduinoOTA.setHostname(config.wifiHostname);

	ArduinoOTA.onStart([]() {
        logger.info("Start updating %s", (ArduinoOTA.getCommand() == U_FLASH ? "flash" : "filesystem"));
    });
	ArduinoOTA.onEnd([]() {
        logger.info("Update finished");
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
    logger.info("OTA initialized");
}

WLAN wlan;
