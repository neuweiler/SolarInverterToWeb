# SolarInverterToWeb
Read data via RS-232 interface from inverter and present it as JSON via WiFi/HTTP. The solution is implemented on an ESP8266
but is likely to work on other ESP or Arduino boards with small modifications.

Use TX/RX pins and a RS-232-to-TTL converter to connect to many of the standard solar inverters from China. The data is presented
as JSON on the root of the webserver, e.g. http://192.168.4.1
