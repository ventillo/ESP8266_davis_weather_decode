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
### Step 1, Arduino part
Take a look at the post I made about the Arduino configuration.
I'll include the INO and CPP files here, just for reference.
https://spoton.cz/2017/11/24/davis-vantague-vue-arduino-and-a-raspberry-pi-3/
This should be the result:
![Arduino and CC1101](https://spoton.cz/wp-content/uploads/2017/11/DSC_0026_s.jpg)

### Step 2, cpnnect to ESP8266
Using a NodeMCU board, so wires can be attached directly to the board, or you can still
use breadboard together with the ESP8266.

![esp8226 and arduino](https://spoton.cz/wp-content/uploads/2020/05/DSC_0403-scaled-e1589367211334.jpg)
Quite simple, you need the VCC and GND  to power the Arduino (Grren + Orange on
the picture), then connect the RX and TX.
| Arduino Pin | ESP Pin |
|-------------|---------|
| Vcc         | Vin     |
| GND         | GND     |
| RX          | TX      |
| TX          | RX      |

### Step 3 - flash your ESP with the latest MircoPython
You need to flash your ESP8266 with a Mircopython image. Where to get it
from and how to do that -> https://docs.micropython.org/en/latest/esp8266/tutorial/intro.html

### Step 4 - freeze main.py
You can do this with mpy-cross main.py
(Get the mpy-cross via AUR, `yay -S mpy-cross`, or via pip, `pip install mpy-cross`)

### Step 5 - modify inet.conf
You'll need to add your WiFi name (SSID) and password. I anticipate WPA
encryption. Also type in the IP you have influxdb installed.
There is no user and password currently in effect, so whatever you type in will
be ignored, the influxdb has to be set for this as well.
As the last thing, add the port n which the influxdb is running

### Step 5 - upload the files
You'll need o upload inet.conf, boot.py and main.mpy
```
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put inet.conf
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put boot.py
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put main.mpy
```
Again, ampy can be found on AUR, or pip install ampy
```
yay -S aur/adafruit-ampy
```
or
```
pip install adafruit-ampy
```

### Step 6 -

