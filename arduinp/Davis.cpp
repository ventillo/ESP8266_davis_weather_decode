/*
  Davis.cpp
  
  Arduino library that implements the Davis Instruments wireless 
  weather station protocol for the RFBee.
  
  Version 0.8
  
  Copyright (c) 2012 by Ray H. Dees

  NOTE: This code is currently below version 1.0, and therefore is
  lacking some functionality or documentation, or may not be fully
  tested.

  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version. 

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details. 

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>. 
  
*/
#include "CC1101.h"
#include "Davis.h"
#include "SPI.h"
//
//  Initialize the Global Variables.
//
volatile uint8_t hopIndex = 0;
volatile int pktRssi;
volatile uint8_t pktLqi;
volatile int freqError;
volatile uint16_t pktCount = 0;
volatile uint16_t pktMiss = 0;
volatile uint8_t timer = 0;
volatile uint8_t now = 0;
volatile uint8_t next = 64;
volatile uint8_t rxing = 0;
volatile uint8_t hopping = 0;
//
//  Intialize Class Variables.
//
uint8_t DAVIS::rxBuffer[BUFFER_SIZE]; 
volatile uint8_t DAVIS::rxBufferIndex = 0;
volatile uint8_t DAVIS::rxBufferLength = 0;
volatile uint8_t DAVIS::freqComp[51];
//
// Implements the standard Arduino begin() function.
//
void DAVIS::begin(void)
{
  NO_INTERRUPTS();                      // Disable Interrupts while changes are made.
  
  ADCSRA = 0x00;                        //  Disable the analog comparator.
    
  EICRA = 0x00;
  EICRA |= (1 << ISC01 | 1 << ISC00);   // Setup Interrupt 0 for Rising edge.
  EIFR  |= (1 << INTF0);                // Clear pending interrupts.
  EIMSK |= (1 << INT0);                 // Enable Interrupt 0.

  //TCCR1B = 0x00;                        // Disable Timer 1. 
  
  //ASSR = 0x00;                                      //  Set Timer 2 to System Clock.
  //TCCR2A = 0x00;                                    //  Reset TCCR2A.  
  //TCCR2A = (1 << WGM21);                            //  Set CTC mode.
  //TCCR2B = 0x00;                                    //  Reset TCCR2B.
  //TCCR2B = (1 << CS22 | 1 << CS21 | 1 << CS20);     //  Prescale by 1024. 
  //TIFR2 = (1 << OCF2B | 1 << OCF2A | 1 << TOV2);    //  Clear pending interrupts.
  //OCR2A = 0x4D;                                     //  Set Compare Match for .01 seconds.
  //TIMSK2 = 0x00;                                    //  Reset TIMSK2.  TIMSK2 &= ~(1 << OCIE2A) Defined as TIMER2_STOP.
  //TIMSK2 = (1 << OCIE2A);                         //  Timer 2 Compare Match A Interrupt Enable. Defined as TIMER2_RUN.
  
  pinMode(SS, OUTPUT);                  // Set Slave Select as an OUTPUT.
  digitalWrite(SS, HIGH);               // Set it HIGH.
  pinMode(GDO0, INPUT);                 // Set INT0 as an INPUT.
  pinMode(GDO2, INPUT);                 // Set INT1 as an INPUT (Future Use).
  //    
  //  These pins correspond to LED's located on the Uart Bee.  
  //
  pinMode(9, OUTPUT);                   // RSSI Pin - Indicates Hopping.
  digitalWrite(9, LOW);                 // Set it LOW.
  pinMode(14, OUTPUT);                  // ASSOC Pin - Indicates Sync.
  digitalWrite(14, LOW);                // Set it LOW.
  pinMode(15, OUTPUT);                  // ON Pin (Not currently used.)
  digitalWrite(15, LOW);                // Set it LOW.
  
  SPI.begin();                          // Start the SPI.
  SPCR |= (1 << SPR0);  
  
  reset();                              // Reset the CC1101.
  setRegisters();                       // Configure the CC1101.
  
  for (uint8_t i = 0; i < 6; i++)      // Preset the frequency compensation.
    freqComp[i] = DAVIS_FSCTRL0;
  
  INTERRUPTS();                         // Enable Interrupts.
}  
//
//  Reset the CC1101.
//
void DAVIS::reset(void) 
{
  digitalWrite(SS, HIGH);               // Deselect the CC1101.
  delayMicroseconds(10);
  digitalWrite(SS, LOW);                // Select the CC1101.
  delayMicroseconds(10);
  digitalWrite(SS, HIGH);               // Deselect the CC1101. 
  delayMicroseconds(50);
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(CC1101_SRES);            // Send reset command strobe.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.
}
//
//  Write the CC1101 Configuration Registers.
//
void DAVIS::setRegisters(void)
{ 
  writeRegister(CC1101_IOCFG2, DAVIS_IOCFG2);
  writeRegister(CC1101_IOCFG1, DAVIS_IOCFG1);
  writeRegister(CC1101_IOCFG0,	DAVIS_IOCFG0);
  writeRegister(CC1101_FIFOTHR, DAVIS_FIFOTHR);
  writeRegister(CC1101_SYNC1, DAVIS_SYNC1);
  writeRegister(CC1101_SYNC0, DAVIS_SYNC0);
  writeRegister(CC1101_PKTLEN, DAVIS_PKTLEN);
  writeRegister(CC1101_PKTCTRL1, DAVIS_PKTCTRL1);
  writeRegister(CC1101_PKTCTRL0, DAVIS_PKTCTRL0);
  writeRegister(CC1101_ADDR,	DAVIS_ADDR);
  writeRegister(CC1101_CHANNR, DAVIS_CHANNR);
  writeRegister(CC1101_FSCTRL1, DAVIS_FSCTRL1);
  writeRegister(CC1101_FSCTRL0, DAVIS_FSCTRL0);
  writeRegister(CC1101_FREQ2, DAVIS_FREQ2);
  writeRegister(CC1101_FREQ1, DAVIS_FREQ1);
  writeRegister(CC1101_FREQ0, DAVIS_FREQ0);
  writeRegister(CC1101_MDMCFG4, DAVIS_MDMCFG4);
  writeRegister(CC1101_MDMCFG3, DAVIS_MDMCFG3);
  writeRegister(CC1101_MDMCFG2, DAVIS_MDMCFG2);
  writeRegister(CC1101_MDMCFG1, DAVIS_MDMCFG1);
  writeRegister(CC1101_MDMCFG0, DAVIS_MDMCFG0);
  writeRegister(CC1101_DEVIATN, DAVIS_DEVIATN);
  writeRegister(CC1101_MCSM2, DAVIS_MCSM2);
  writeRegister(CC1101_MCSM1, DAVIS_MCSM1);
  writeRegister(CC1101_MCSM0, DAVIS_MCSM0);
  writeRegister(CC1101_FOCCFG,	DAVIS_FOCCFG);
  writeRegister(CC1101_BSCFG, DAVIS_BSCFG);
  writeRegister(CC1101_AGCCTRL2, DAVIS_AGCCTRL2);
  writeRegister(CC1101_AGCCTRL1, DAVIS_AGCCTRL1);
  writeRegister(CC1101_AGCCTRL0, DAVIS_AGCCTRL0);
  writeRegister(CC1101_WOREVT1, DAVIS_WOREVT1);
  writeRegister(CC1101_WOREVT0, DAVIS_WOREVT0);
  writeRegister(CC1101_WORCTRL, DAVIS_WORCTRL);
  writeRegister(CC1101_FREND1,	DAVIS_FREND1);
  writeRegister(CC1101_FREND0,	DAVIS_FREND0);
  writeRegister(CC1101_FSCAL3,	DAVIS_FSCAL3);
  writeRegister(CC1101_FSCAL2,	DAVIS_FSCAL2);
  writeRegister(CC1101_FSCAL1,	DAVIS_FSCAL1);
  writeRegister(CC1101_FSCAL0,	DAVIS_FSCAL0);
  writeRegister(CC1101_RCCTRL1, DAVIS_RCCTRL1);
  writeRegister(CC1101_RCCTRL0, DAVIS_RCCTRL0);
  writeRegister(CC1101_FSTEST, DAVIS_FSTEST);
  writeRegister(CC1101_PTEST, DAVIS_PTEST);
  writeRegister(CC1101_AGCTEST, DAVIS_AGCTEST);
  writeRegister(CC1101_TEST2, DAVIS_TEST2);
  writeRegister(CC1101_TEST1, DAVIS_TEST1);
  writeRegister(CC1101_TEST0, DAVIS_TEST0);
  writeBurst(CC1101_PATABLE, (uint8_t*)PA_TABLE, 8);
  setFrequency(hopIndex);
  digitalWrite(15, HIGH);
}
//
//  Write a strobe command.
//
void DAVIS::cmdStrobe(uint8_t command) 
{
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(command);                // Send strobe command.
  digitalWrite(SS, HIGH);               // Deselect the CC1101 .
}
//
//  Write to a single register.
//
void DAVIS::writeRegister(uint8_t regAddr, uint8_t value)
{
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(regAddr);                // Send register address.
  SPI.transfer(value);                  // Send value.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.
}
//
//  Write to sequential registers.
//
void DAVIS::writeBurst(uint8_t regAddr, uint8_t *buffer, uint8_t length)
{
  uint8_t addr, i;
  
  addr = regAddr | WRITE_BURST;         // Enable burst transfer.
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(addr);                   // Send register base address.
  for(i = 0; i < length; i++)
    SPI.transfer(buffer[i]);            // Send values byte by byte.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.  
}
//
//  Read from a single register.
//
uint8_t DAVIS::readRegister(uint8_t regAddr) 
{
  uint8_t addr, value;

  addr = regAddr | READ_SINGLE;
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(addr);                   // Send register address.
  value = SPI.transfer(0x00);           // Read value.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.

  return value;
}
//
//  Read from sequential registers.
//
void DAVIS::readBurst(uint8_t regAddr, uint8_t *buffer, uint8_t length) 
{
  uint8_t addr, i;
  
  addr = regAddr | READ_BURST;
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(addr);                   // Send register base address.
  for(i = 0; i < length; i++)
    buffer[i] = SPI.transfer(0x00);     // Read registers byte by byte.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.
}
//
//  Read the status from a register.
//
uint8_t DAVIS::readStatus(uint8_t regAddr) 
{
  uint8_t addr, value;

  addr = regAddr | READ_BURST;
  digitalWrite(SS, LOW);                // Select the CC1101.
  SPI.transfer(addr);                   // Send register address.
  value = SPI.transfer(0x00);           // Read status.
  digitalWrite(SS, HIGH);               // Deselect the CC1101.

  return value;
}
//
//  Sets Idle Mode.
//
void DAVIS::idle(void)
{
  setIdleState();
}
//
//  Sets Rx Mode.
//
void DAVIS::rx(void)
{
  setRxState();
  
  while ((Radio.readStatus(CC1101_MARCSTATE) & 0x1F) != CC1101_STATE_RX)
    delayMicroseconds(900);
    
  rxing = 1;

  writeRegister(CC1101_IOCFG0,	DAVIS_IOCFG0);
}  
//
//  Controls frequency hopping.
//
void DAVIS::hop(void)
{
  digitalWrite(9, HIGH);                // Turn on hopping LED.
          
  hopIndex++;                           // Increment the index
  
  if (hopIndex > 4)                    // Clamp at 51.
    hopIndex = 0;      
  
  writeRegister(CC1101_FSCTRL0, freqComp[hopIndex]);            // Write FSCTRL0.
  
  setFrequency(hopIndex);               //  Set the frequency.
  
  digitalWrite(9, LOW);                 // Turn off hopping LED.
}
//
//  Sets the RX or TX frequency from the lookup table.
//
void DAVIS::setFrequency(uint8_t index)
{
  idle();                                                       //  Make sure we are in idle state.
  
  writeRegister(CC1101_FREQ2, pgm_read_byte(&FREQ_2[index]));   // Write FREQ2.
  writeRegister(CC1101_FREQ1, pgm_read_byte(&FREQ_1[index]));   // Write FREQ1.
  writeRegister(CC1101_FREQ0, pgm_read_byte(&FREQ_0[index]));   // Write FREQ0.
    
  flush();                                                      // Flush everything.
}
//
//  Receive the data packet.
//
void DAVIS::rxData(void)
{
  uint8_t pktVerify, pktLength, freqEst, addr, i;
  uint16_t pktCrc = 0xFFFF;
////////////////////////////////////////////  See Errata Note for this section.      
  pktVerify = readStatus(CC1101_RXBYTES) & 0x7F; 
  
  do
    {
      pktLength = pktVerify;
      pktVerify = readStatus(CC1101_RXBYTES) & 0x7F;
    }  
  
  while (pktLength != pktVerify); 
////////////////////////////////////////////    
    
  if ((readStatus(CC1101_MARCSTATE) & 0x1F) == CC1101_STATE_RXFIFO_ERROR)  // Check for Rx FIFO Overrun. 
    pktLength = 0;
      
    
  if (pktLength == DAVIS_PACKET_LENGTH)
    {
      addr = CC1101_RXFIFO | READ_BURST;
      
      digitalWrite(SS, LOW);                // Select the CC1101.
      
      SPI.transfer(addr);                   // Send register base address.
      
      SPI.setBitOrder(LSBFIRST);            // Reverse the bit order.
      
      for(i = 0; i < 10; i++)
        rxBuffer[i] = SPI.transfer(0x00);   // Read rx data into the buffer.
      
      SPI.setBitOrder(MSBFIRST);            // Set the bit order back. 
      
      for(i = 10; i < 12; i++)
        rxBuffer[i] = SPI.transfer(0x00);   // Read RSSI & LQI into the buffer.
      
      digitalWrite(SS, HIGH);               // Deselect the CC1101.
      
      pktCrc = calcCrc(rxBuffer, 8);        // Get the CRC for first eight bytes.
      
      //pktCrc = 0x0000;                    // Uncomment to disable CRC checking.
      
      if (pktCrc == 0x0000)                 // If CRC = 0, valid data is now available.
        {
          now = timer = 0;
          pktRssi = calcRssi(rxBuffer[10]);          
          pktLqi = (rxBuffer[11] & 0x7F);
          freqEst = readStatus(CC1101_FREQEST);
          freqError = calcFreqError(freqEst);
          freqComp[hopIndex] = freqComp[hopIndex] + freqEst;
          pktCount++;
          rxBufferLength = pktLength;
          rxing = 0;
          writeRegister(CC1101_IOCFG0,	DAVIS_DISABLED);  //  Disable CC1101 Rx interrupts.
          digitalWrite(14, HIGH);
          return;
        }
    }
      
  idle();
  flushRxFifo();
  rx();
}   
//
//  Implements the standard Arduino available() function.
//
int DAVIS::available(void) 
{
  if (rxBufferIndex == rxBufferLength)
    return -1;
  
  else
    return rxBufferLength - rxBufferIndex;
}  
//
//  Implements the standard Arduino read() function.
//
uint8_t DAVIS::read(void)  
{
  uint8_t value = 0x00;
     
  if (rxBufferIndex < rxBufferLength)
    {
      value = rxBuffer[rxBufferIndex];
      rxBufferIndex++;
    }
  
  else
    rxBufferIndex = rxBufferLength = 0;
      
  return value;
}
//
//  Flushes FIFO's and the receive buffer.
//
void DAVIS::flush(void)
{
  flushRxFifo();                        // Flush both CC1101 FIFO's. 
  flushTxFifo();
  
  for (int i = 0; i < BUFFER_SIZE; i++) // Flush the rxBuffer.
    rxBuffer[i] = 0x00;
  
  rxBufferLength = rxBufferIndex = 0;
}
//
//  Calculates the 16 bit CRC using the Davis method.
//
uint16_t DAVIS::calcCrc(uint8_t *buffer, uint8_t length)
{ 
  uint16_t crc = 0x0000; 

  while (length-- > 0)
    { 
      crc = (crc << 8) ^ pgm_read_word(&CRC_TABLE[(crc >> 8) ^ (*buffer++)]); 
    }; 
 
  return crc; 
}     
//
//  Calculates the RSSI in dBm.
//
char DAVIS::calcRssi(uint8_t value)
{
  int rssi;
  
  if (value >= 128)
    rssi = ((value - 256) >> 1) - 74;  //  CC1101 RSSI offset value.
  
  else
    rssi = (value >> 1) - 74;
  
  if (rssi < -128)
    rssi = -128;
    
  return rssi;
}  
//
//  Calculates the Frequency Error for display purposes.
//
int DAVIS::calcFreqError(uint8_t value)
{
  int error;
  
  if (value >= 128)
    error = ((value - 256) >> 1);
  
  else
    error = (value >> 1);
      
  return error;
}  
//
//  INT0 Interrupt Service Routine.
//
ISR(INT0_vect)
{
  Radio.rxData();
}  
//
//  Timer 2 Compare Match A Interrupt Service Routine. 
//
ISR(TIMER2_COMPA_vect)
{
  timer++;
  if (timer >= 4)
    {
      timer = 0;
      now++;
      next++;
    }  
  if (now >= 44 && rxing == 0)
    Radio.rx();
}    
//
//
//
DAVIS Radio;
