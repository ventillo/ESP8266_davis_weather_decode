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
You can do this with
```
mpy-cross main.py
```
(Get the mpy-cross via AUR, `yay -S mpy-cross`, or via pip, `pip install mpy-cross`)

### Step 5 - modify inet.conf
You'll need to add your WiFi name (SSID) and password. I anticipate WPA
encryption. Also type in the IP you have influxdb installed.
There is no user and password currently in effect, so whatever you type in will
be ignored, the influxdb has to be set for this as well.
As the last thing, add the port n which the influxdb is running

### Step 5 - upload the files
For this, you **need to disconnect the RX/TX pins from the ESP8266 temporarily**
You'll need o upload inet.conf, boot.py and main.mpy
Again, ampy can be found on AUR, or pip install ampy
```
yay -S aur/adafruit-ampy
```
or
```
pip install adafruit-ampy
```
And do the actual copy
```
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put inet.conf
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put boot.py
/usr/bin/ampy -b 115200 -p /dev/ttyUSB0 put main.mpy
```
Wait for the copy to finish, it can take some time.
Make sure you do not connec to the ESP dureing the time of copy with a console
(picocom etc...)

### Step 6 - reconnect and run
Reconnect the RX/TX pins as they were and launch a serial console. I use
picocom.
I first run the console @ 115200 baud, then switch to 9600 (duh, stupid, I know).
After hitting the reset ont he ESP, you should see some startup messages
```
picocom -b 115200 /dev/ttyUSB0

b'Disabling AP'
b'Activating INFRA'
b'Connecting to infra'
b"Let's wait for the network to come up"
b'Trying... 15 more times'
b'Trying... 14 more times'
b'Trying... 13 more times'
b'Trying... 12 more times'
b'Trying... 11 more times'
b'Trying... 10 more times'
b'Trying... 9 more times'
b'Trying... 8 more times'
b'Trying... 7 more times'
b'Trying... 6 more times'
b'Trying... 5 more times'
Connected:
b'  IP: 192.168.1.101'
b'  MASK: 255.255.255.0'
b'  GW: 192.168.1.254'
b'  DNS: 192.168.1.4\n'
b'Network section done'
stack: 2720 out of 8192
GC: total: 37952, used: 19968, free: 17984
 No. of 1-blocks: 398, 2-blocks: 53, max blk sz: 264, max free sz: 1124
...
```
If it times out, waiting for wifi, try to reboot, move closer to AP, or doublecheck the SSID/PASS
At this point, you can press Ctrl + A + D 4 times in a row, so the baud rate gets down to 9600.
At this point, you should see some debug output already
```
...
{'hop':0,'h':80,'b0':6,'b1':87,'b2':255,'b3':113,'b4':1,'b5':125,'b6':45,'b7':255,'b8':255,'b9':188,'rssi':44,'lqi':0,'nxt':64,'cnt':66}
ID: 0, PKT: 5, B_LOW: 0:
W_SPD: 9.7, W_DIR: 121.8
NETCONF: 192.168.1.2:8086
 WIND:{'speed': 9.7, 'direction': 121.8}
 MEASUREMENT: rain
 MEASURE NAME: value
 VALUE: 0.0
 TAGS: {'type': 'rainrate'}
SENDING TO: http://192.168.1.2:8086/write?db=weather_v2
POST_DATA: rain,type=rainrate value=0.0
                                        wind,type=speed value=9.7
                                                                  wind,type=direction value=121.8
DATA SEND: 204
{'hop':1,'h':128,'b0':6,'b1':105,'b2':31,'b3':10,'b4':12,'b5':242,'b6':21,'b7':255,'b8':255,'b9':187,'rssi':48,'lqi':0,'nxt':64,'cnt':67}
ID: 0, PKT: 8, B_LOW: 0:
W_SPD: 9.7, W_DIR: 147.0
NETCONF: 192.168.1.2:8086
 WIND:{'speed': 9.7, 'direction': 147.0}
 MEASUREMENT: temphumi
 MEASURE NAME: temperature
 VALUE: 9.8
 TAGS: {'type': 'external'}
SENDING TO: http://192.168.1.2:8086/write?db=weather_v2
POST_DATA: temphumi,type=external temperature=9.8
                                                  wind,type=speed value=9.7
                                                                            wind,type=direction value=147.0
...
```
First line is the packet as it comes from the arduino, just for debug.
```
{'hop':0,'h':80,'b0':6,'b1':87,'b2':255,'b3':113,'b4':1,'b5':125,'b6':45,'b7':255,'b8':255,'b9':188,'rssi':44,'lqi':0,'nxt':64,'cnt':66}
```
Some additional debug about the packet data (davis ID, packet ID and if the battery is low in the external davis vue)
Plus decoded info about the payload of the packet
```
ID: 0, PKT: 5, B_LOW: 0:
W_SPD: 9.7, W_DIR: 121.8
```
Then network configuration of the influx DB
```
NETCONF: 192.168.1.2:8086
```
Finally, what is going to be written and the real POST to influx.
```
 WIND:{'speed': 9.7, 'direction': 147.0}
 MEASUREMENT: temphumi
 MEASURE NAME: temperature
 VALUE: 9.8
 TAGS: {'type': 'external'}
```
POST write to
```
SENDING TO: http://192.168.1.2:8086/write?db=weather_v2
```
with data
```
temphumi,type=external temperature=9.8
wind,type=speed value=9.7
wind,type=direction value=147.0
```
Wind data is always present, as the first two bytes represent the speed and direction
The rest is payload of other data.
Finally a HTTP response code of how the write went:
```
DATA SEND: 204
```
