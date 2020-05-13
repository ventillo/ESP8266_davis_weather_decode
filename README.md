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
(TBA)

### ESP8266 part
Using a NodeMCU board, so wires can be attached directly to the board, or you can still
use breadboard

First, you need to flash your ESP8266 with a Mircopython image. Where to get it
from and how to do that -> https://docs.micropython.org/en/latest/esp8266/tutorial/intro.html


