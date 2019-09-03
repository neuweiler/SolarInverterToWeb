/*
 * Config.h
 *
 *  Created on: 3 Sep 2019
 *      Author: Michael Neuweiler
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Logger.h"

class Config
{
public:
    static void init();

    static const char *CONFIG_FILE;

    // Inverter
    static uint16_t initialMaxSolarPower; // initial Power to start consumer with (in W)
    static uint16_t inverterUpdateInterval; // at which interval the next data is requested from inverter (in ms)
    static uint16_t pvOutPowerTolerance; // tolerance of higher out power against PV input (in W)
    static int16_t maxBatteryDischargeCurrent; // allowed discharge current before throttling down consumer power (in A)
    static int16_t minBusVoltage; // minimum bus voltage allowed before throttling down consumer power (in V)
    static float minPvVoltage; // minimum PV voltage allowed before throttling down consumer power (in V)
    static float maxPvVoltage; // PV voltage at which the consumer power can be increased (in V)
    static uint16_t powerAdjustment; // amount of power increased/decreased when adjusting consumer power (in W)
    static uint16_t minSolarPower; // minimum solar power to provide to the consumer (in W)
    static uint16_t maxSolarPower; // maximum solar power to provide to the consumer (in W)

    // Station
    static String stationSsid;
    static String stationPassword;
    static uint16_t stationUpdateInterval; // at which interval the next update is sent to the consumer (in ms)
    static uint16_t stationReconnectInterval; // at which interval the next connection attempt to the station is made (in ms)
    static bool consumerOutputAsCurrent; // if true the consumer output is sent in Amps, otherwise in Watts
    static String stationRequestPrefix; // the prefix for the HTTP request to the station
    static String stationRequestPostfix; // the postfix for the HTTP request to the station

    // Webserver
    static String serverSsid;
    static String serverPassword;

};

#endif /* CONFIG_H_ */
