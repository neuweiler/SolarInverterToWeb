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
			WiFi.begin(config.wifiStationSsid, config.wifiStationPassword);
			lastConnectionAttempt = millis();
		}
	}
//    apConnected = WiFi.softAPgetStationNum() > 0;

	// indicate connection status via LED
	digitalWrite(PIN_LED_WIFI_CONNECTED, (isConnected ? HIGH : LOW));
}

void WLAN::setupStation() {
	WiFi.setAutoReconnect(false); // auto-reconnect tries every 1sec, messes up soft-ap (can't connect)
//WiFi.setAutoReconnect(true);
	logger.info(F("Wifi: connecting to access point %s"), config.wifiStationSsid);
	char stationSsid[32];
	memcpy(stationSsid, config.wifiStationSsid, sizeof(stationSsid));
	WiFi.begin(stationSsid, config.wifiStationPassword);

	uint8_t i = 60;
	while (!WiFi.isConnected() && i-- > 0) {
		logger.console(F("waiting (%d)..."), i);
		delay(500);
	}
	logger.info(F("started WiFi Station: %s"), WiFi.localIP().toString().c_str());
}


/***** Note: This code is only included from ESP8266WiFiGenericClass::mode() for debugging purposes ******/

extern "C" char* wifi_station_hostname; // sdk's hostname location

bool WLAN::mode(WiFiMode_t m) {
    if (m & ~(WIFI_STA | WIFI_AP)) {
        // any other bits than legacy disallowed
        return false;
    }

    if(wifi_get_opmode() == (uint8) m){
        return true;
    }

    char backup_hostname [33] { 0 }; // hostname is 32 chars long (RFC)

    if (m != WIFI_OFF && wifi_fpm_get_sleep_type() != NONE_SLEEP_T) {
        memcpy(backup_hostname, wifi_station_hostname, sizeof(backup_hostname));
        // wifi starts asleep by default
        wifi_fpm_do_wakeup();
        wifi_fpm_close();
    }

    logger.debug("a pw: %s ssid: %s, ch: %d", config.wifiApPassword, config.wifiApSsid, config.wifiApChannel);
    bool ret = false;
    ETS_UART_INTR_DISABLE();
    ret = wifi_set_opmode_current(m);
    ETS_UART_INTR_ENABLE();
    logger.debug("b pw: %s ssid: %s, ch: %d", config.wifiApPassword, config.wifiApSsid, config.wifiApChannel);

    if(!ret)
      return false; //calling wifi_set_opmode failed

    //Wait for mode change, which is asynchronous.
    //Only wait if in CONT context. If this were called from SYS, it's up to the user to serialize
    //tasks to wait correctly.
    constexpr unsigned int timeoutValue = 1000; //1 second
    if(can_yield()) {
        // check opmode every 100ms or give up after timeout
        esp_delay(timeoutValue, [m]() { return wifi_get_opmode() != m; }, 100);

        //if at this point mode still hasn't been reached, give up
        if(wifi_get_opmode() != (uint8) m) {
            return false; //timeout
        }
    }

    if (backup_hostname[0])
        memcpy(wifi_station_hostname, backup_hostname, sizeof(backup_hostname));

    return ret;
}

/****** ********/

void WLAN::setupAccessPoint() {
	IPAddress localIp;
	localIp.fromString(config.wifiApAddress);
	IPAddress gateway;
	gateway.fromString(config.wifiApGateway);
	IPAddress subnet;
	subnet.fromString(config.wifiApNetmask);

	/***** Note: This code is only included from ESP8266WiFiGenericClass::mode() for debugging purposes ******/

WiFiMode_t currentMode = WiFi.getMode();
bool isEnabled = ((currentMode & WIFI_AP) != 0);

if(!isEnabled) {
	logger.debug("03ap pw: %s ssid: %s, ch: %d", config.wifiApPassword, config.wifiApSsid, config.wifiApChannel);
  mode((WiFiMode_t)(currentMode | WIFI_AP));
  logger.debug("04ap pw: %s ssid: %s, ch: %d", config.wifiApPassword, config.wifiApSsid, config.wifiApChannel);
}

	/****** *******/

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
