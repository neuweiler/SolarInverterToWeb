/*
 * Config.h
 *
 *  Created on: 3 Sep 2019
 *      Author: Michael Neuweiler
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define FS_MAX_OPEN_FILES 20

#define PIN_LED_WIFI_CONNECTED D5 // green LED
#define PIN_LED_CLIENT_CONNECTED D6  // yellow LED
#define PIN_POWER_OVERRIDE D7

#include <Arduino.h>
#include <LittleFS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Logger.h"

// uncomment to enable memory stats being sent every 0.5sec
//#define DEBUG_MEM

// uncomment to redirect all log output to Serial (USB) and set speed to 115200 - only works with no inverter connected, use only during dev
//#define DEBUG_LOG

class Config
{
public:
    enum BatteryType
    {
        LeadAcid = 0,
        NiMh = 1,
        LiIon = 2,
        Other = 99
   };

    void init();
    void load();

    static const char *CONFIG_FILE;

    // Inverter
    uint16_t initialSolarPower; // initial Power to start consumer with (in W)
    uint16_t inverterInterval; // at which interval the next data is requested from inverter (in ms)
    uint16_t pvOutPowerTolerance; // tolerance of higher out power against PV input (in W)
    int16_t maxBatteryDischargeCurrent; // allowed discharge current before throttling down consumer power (in A)
    int16_t minBusVoltage; // minimum bus voltage allowed before throttling down consumer power (in V)
    float minPvVoltage; // minimum PV voltage allowed before throttling down consumer power (in V)
    float maxPvVoltage; // PV voltage at which the consumer power can be increased (in V)
    uint16_t powerAdjustment; // amount of power increased/decreased when adjusting consumer power (in W)
    uint16_t minSolarPower; // minimum solar power to provide to the consumer (in W)
    uint16_t maxSolarPower; // maximum solar power to provide to the consumer (in W)
    uint32_t cutoffRetryTime; // time until a retry is started after a power cutoff due to minSolarPower (in sec)
    uint8_t cutoffRetryMinBatterySoc; // minimum battery soc to try a restart after power cutoff (in %)
    uint8_t inputOverrideActivateSOC; // SOC at which input prio will switch to SUB (in 1%, 0 = disabled)
    uint8_t inputOverrideDeactivateSOC; // SOC at which input prio will switch back to SBU (in 1%)

    // Battery
    uint16_t batteryCapacity; // the capacity of the battery (in Ah)
    BatteryType batteryType; // the type of battery used
    float batteryVoltageFullCharge; // the voltage at which the battery pack is fully charged and charge should stop (in V)
    float batteryVoltageNominal; // the nominal (resting) voltage of the fully charged battery pack (in V)
    float batteryVoltageEmpty; // the battery voltage at which a resting battery is to be considered fully discharged (in V)
    float batteryVoltageFloat; // the default float voltage to set to avoid trickle charging Li-Ion batteries (in V)
    bool batteryOverDischargeProtection; // even when switched to utility in SBU mode, the inverter still may drain the battery, if true this switches to SUB mode and enables grid charge until battery voltage is at nominal voltage
    bool batterySocCalculateInternally; // if true we'll display the SOC / Ah by counting ourselfes, if fals we'll use the inverter's SOC (true/false)
    uint8_t batterySocTriggerFloatOverride; // state of charge at which a float voltage charge will be triggered (in %, 0 to disable)
    uint8_t batteryRestDuration; // if voltage < batteryVoltageEmpty this is the duration where load has to be below restCurrent before we declare the battery empty (in sec)
    uint8_t batteryRestCurrent; // max current where we still consider the battery to be at rest with no signifikant load (in A)

    // Wifi
    const char *wifiHostname; // the host name
    const char *wifiStationSsid; // the ssid of the network we want to connect to, if empty no connection is attempted
    const char *wifiStationPassword; // the password of the network we want to connect to
    uint16_t wifiStationReconnectInterval; // at which interval the next connection attempt to the network is made (in ms)
    const char *wifiApSsid; // the ssid of the network we provide
    const char *wifiApPassword; // the password of the network we provide
    uint8_t wifiApChannel; // the channel of the network we provide
    const char *wifiApAddress; // the ip address of the network we provide
    const char *wifiApGateway; // the gateway address of the network we provide
    const char *wifiApNetmask; // the netmask of the network we provide
    bool wifiApNAT; // if NAT should be enabled in AP/Station mode to forward traffic to foreign AP
};

extern Config config;

#endif /* CONFIG_H_ */
