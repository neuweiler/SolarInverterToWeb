# SolarInverterToWeb
Read data via RS-232 interface from inverter and present it as JSON via WiFi/HTTP. The solution is implemented on an ESP8266
but is likely to work on other ESP or Arduino boards with small modifications.

Use TX/RX pins and a RS-232-to-TTL converter to connect to many of the standard solar inverters from China. The data is presented
as JSON on the root of the webserver, e.g. http://192.168.4.1

Example output: 
```
{
  "grid": {
    "voltage": 235.5,
    "frequency": 49.9
  },
  "out": {
    "voltage": 235.5,
    "frequency": 49.9,
    "powerApparent": 1818,
    "powerActive": 1818,
    "load": 60
  },
  "battery": {
    "voltage": 2.4,
    "voltageSCC": 0,
    "currentCharge": 0,
    "currentDischarge": 0,
    "soc": 0
  },
  "pv": {
    "voltage": 208.4,
    "current": 5,
    "power": 1159
  },
  "system": {
    "status": 16,
    "mode": 10,
    "version": 0,
    "voltage": 368,
    "temperature": 28,
    "fanCurrent": 0
  }
}
```
