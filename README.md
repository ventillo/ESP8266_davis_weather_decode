# ESP8266 Ddavis weather decoder
Take in data from an Arduino + CC1101 via Serial and send them to InlfuxDB via WiFi

## Input
Serial data from an arduino device, capable of receiving data from the Davis Vantage Vue
via CC1101.

## Output
Data written into database (currently only influxdb) via WiFi. Basically a bridge between
an arduino and a Wifi network - with some additional checks of the Davis packets

# Hardware
You will need:
- Arduino Mini Pro (3.3v)
- CC1101 from Texas instruments
- ESP8266 development board
- couple wires
- FTDI USB dongle

# Software
The arduino stuff is written in C of course, but the rest is done in MircoPython.
Memory isses forced me to pre-compile the main module to .mpy (more on that later)

# Step by step
### Arduino part
Take a look at the post I made about the Arduino configuration.
I'll include the INO and CPP files here, just for reference.
https://spoton.cz/2017/11/24/davis-vantague-vue-arduino-and-a-raspberry-pi-3/
This should be the result:
![Arduino and CC1101](https://spoton.cz/wp-content/uploads/2017/11/DSC_0026_s.jpg)

### ESP8266 part
Using a NodeMCU board, so wires can be attached directly to the board, or you can still
use breadboard
Together with the ESP8266.
![esp8226 and arduino](https://spoton.cz/wp-content/uploads/2020/05/DSC_0403-scaled.jpg)

First, you need to flash your ESP8266 with a Mircopython image. Where to get it
from and how to do that -> https://docs.micropython.org/en/latest/esp8266/tutorial/intro.html


