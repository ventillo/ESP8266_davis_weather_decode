
/*
    Davis logger with Wifi esp8266 as bridge.
*/

int height = 450;

#include "Davis.h"
#include "SPI.h"
//
byte temp;
String data;
int incByte = 0;

void setup()
{
  delay(100);
  Serial.begin(9600);
  delay(100);
  Radio.begin();
  Radio.rx();
  Serial.println("Done Setup");
}

void loop()
{
  if (Radio.available() > 0)
    printPacket();
  if (hopping)
    {
      hopping = 0;
      Radio.hop();
      Radio.rx();  // Un-comment if not using Timer 2.
    }
}


void printPacket(void)
{
  byte header = Radio.read();     // Where's the header?
  byte package[16];

  package[0] = hopIndex;          // Which frequency are we at?
  package[1] = header;
  for (int i = 2; i < 11; i++)    //  The received packet.
  {
    package[i] = Radio.read();
  }
  package[11] = pktRssi;
  package[12] = pktLqi;
  package[13] = freqError;
  package[14] = next;
  package[15] = pktCount;

  String data = "{'hop':" + String(package[0]) + "," +
    "'h':" + String(package[1]) + "," +
    "'b0':" + String(package[2]) + "," +
    "'b1':" + String(package[3]) + "," +
    "'b2':" + String(package[4]) + "," +
    "'b3':" + String(package[5]) + "," +
    "'b4':" + String(package[6]) + "," +
    "'b5':" + String(package[7]) + "," +
    "'b6':" + String(package[8]) + "," +
    "'b7':" + String(package[9]) + "," +
    "'b8':" + String(package[10]) + "," +
    "'b9':" + String(package[11]) + "," +
    "'rssi':" + String(package[12]) + "," +
    "'lqi':" + String(package[13]) + "," +
    "'nxt':" + String(package[14]) + "," +
    "'cnt':" + String(package[15]) + "}";
  Serial.println(data);
  hopping = 1;                  //  Turn on hopping.

}
