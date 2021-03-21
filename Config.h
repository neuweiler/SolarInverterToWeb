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
    uint16_t inverterUpdateInterval; // at which interval the next data is requested from inverter (in ms)
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

    // Battery
    uint16_t batteryCapacity; // the capacity of the battery (in Ah)
    BatteryType batteryType; // the type of battery used
    float batteryVoltageFullCharge; // the voltage at which the battery pack is fully charged and charge should stop (in V)
    float batteryVoltageNominal; // the nominal (resting) voltage of the fully charged battery pack (in V)
    float batteryVoltageEmpty; // the battery voltage at which a resting battery is to be considered fully discharged (in V)
    float batteryVoltageFloat; // the default float voltage to set to avoid trickle charging Li-Ion batteries (in V)
    bool batterySocCalculateInternally; // if true we'll display the SOC / Ah by counting ourselfes, if fals we'll use the inverter's SOC (true/false)
    uint8_t batterySocTriggerFloatOverride; // state of charge at which a float voltage charge will be triggered (in %, 0 to disable)
    uint8_t batteryRestDuration; // if voltage < batteryVoltageEmpty this is the duration where load has to be below restCurrent before we declare the battery empty (in sec)
    uint8_t batteryRestCurrent; // max current where we still consider the battery to be at rest with no signifikant load (in A)

    // Station
    String stationSsid;
    String stationPassword;
    uint16_t stationUpdateInterval; // at which interval the next update is sent to the consumer (in ms)
    uint16_t stationReconnectInterval; // at which interval the next connection attempt to the station is made (in ms)
    bool outputAsCurrent; // if true the consumer output is sent in Amps, otherwise in Watts
    String stationRequestPrefix; // the prefix for the HTTP request to the station
    String stationRequestPostfix; // the postfix for the HTTP request to the station

    // Webserver
    String serverSsid;
    String serverPassword;

private:
    void print();
};

extern Config config;

#endif /* CONFIG_H_ */
