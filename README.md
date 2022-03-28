# Inuits IoT scale

## Repository info

- `scale/` Contains program for ESP8266
- `WWW/` Contains Web UI written in JavaScript using jQuery framework

## ESP8266

### Required libraries

- [HX711](https://github.com/bogde/HX711)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets)
- [ESPAsynsWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP)

### Description

Weight sensor is connected to the ESP8266 using HX711 amplifier to pins `5` and `4`.

Pin `5` is used as `DT`, pin `4` as `SCK`.

ESP8266 is polling the weight sensor every 0.5 seconds to see if there is any change in the weight.

When change occurs, ESP8266 will send measured weight to serial console and to the WebSocket client.

### Weight sensor application

![weight_sensor](https://i.ibb.co/RB2850z/SEN0160-Dimension.jpg)

## Web UI

Web UI is standalone, serverless application writen in JavaScript using jQuery framework.

This app is using mDNS to try to find ESP8266 in local network using AJAX request to location where ESP is supposed to be listening.

After successful response app will try to connect to the WebSocket server running on the ESP.

When the connection is successful, app will change the state in the bottom of the page to `Connected`.

When not, app will try to find ESP every 5 seconds.

App is testing if connection is still active every 7 seconds. If not, app wil try to recconect using same system as on first connecting.
