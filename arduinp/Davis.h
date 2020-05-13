/*
  Davis.h
    
  Arduino library that implements the Davis Instruments Vantage Vue 
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
//
#ifndef Davis_h
#define Davis_h
//
#include "Arduino.h"
//
//  Arduino Definitions.
//
#define SS                       10        // Arduino Digital Pin used for Slave Select - Out
#define GDO0                      2        // Arduino Digital Pin used for INT0 - In
#define GDO2                      3        // Arduino Digital Pin used for INT1 - In
//
#ifndef NO_INTERRUPTS
#define NO_INTERRUPTS()          uint8_t sreg = SREG; cli()
#define INTERRUPTS()             SREG = sreg
#endif
//
#ifndef TIMER2_RUN
#define TIMER2_RUN()             TCNT2 = 0x00; TIMSK2 = (1 << OCIE2A)
#define TIMER2_RESET()           TCNT2 = 0x00; timer = 0  
#define TIMER2_STOP()            TIMSK2 &= ~(1 << OCIE2A)
#endif
//
#define BUFFER_SIZE                16        // TX & RX Buffer Size
//
//  Davis Packet Length.
//
#define DAVIS_PACKET_LENGTH        12        // The actual packet length including RSSI & LQI
//
//  Davis Register Configuration Settings.
//
#define DAVIS_IOCFG2             0x2E        // GDO2 Output Pin Configuration
#define DAVIS_IOCFG1             0x2E        // GDO1 Output Pin Configuration
#define DAVIS_IOCFG0             0x01        // GDO0 Output Pin Configuration
//
#define DAVIS_FIFOTHR            0x42        // RX FIFO and TX FIFO Thresholds
//
#define DAVIS_SYNC1              0xCB        // Synchronization word, high byte
#define DAVIS_SYNC0              0x89        // Synchronization word, low byte
//
#define DAVIS_PKTLEN             0x0A        // Packet Length 
#define DAVIS_PKTCTRL1           0xC4        // Packet Automation Control
#define DAVIS_PKTCTRL0           0x00        // Packet Automation Control
//
#define DAVIS_ADDR               0x00        // Device Address
#define DAVIS_CHANNR             0x00        // Channel Number
#define DAVIS_FSCTRL1            0x06        // Frequency Synthesizer Control
#define DAVIS_FSCTRL0            0xF0        // Frequency Synthesizer Control
// 
#define DAVIS_FREQ2              0x23        // Frequency Control Word, High Byte
#define DAVIS_FREQ1              0x0D        // Frequency Control Word, Middle Byte
#define DAVIS_FREQ0              0x97        // Frequency Control Word, Low Byte
//
#define DAVIS_MDMCFG4            0xC9        // Modem Configuration
#define DAVIS_MDMCFG3            0x83        // Modem Configuration
#define DAVIS_MDMCFG2            0x12        // Modem Configuration
#define DAVIS_MDMCFG1            0x21        // Modem Configuration 
#define DAVIS_MDMCFG0            0xF9        // Modem Configuration
//
#define DAVIS_DEVIATN            0x24        // Modem Deviation Setting
//
#define DAVIS_MCSM2              0x07        // Main Radio Control State Machine Configuration
#define DAVIS_MCSM1              0x00        // Main Radio Control State Machine Configuration
#define DAVIS_MCSM0              0x18        // Main Radio Control State Machine Configuration
//
#define DAVIS_FOCCFG             0x16        // Frequency Offset Compensation Configuration
#define DAVIS_BSCFG              0x6C        // Bit Synchronization Configuration
//
#define DAVIS_AGCCTRL2           0x43        // AGC Control
#define DAVIS_AGCCTRL1           0x40        // AGC Control
#define DAVIS_AGCCTRL0           0x91        // AGC Control
//
#define DAVIS_WOREVT1            0x87        // High Byte Event0 Timeout
#define DAVIS_WOREVT0            0x6B        // Low Byte Event0 Timeout
#define DAVIS_WORCTRL            0xF8        // Wake On Radio Control
//
#define DAVIS_FREND1             0x56        // Front End RX Configuration
#define DAVIS_FREND0             0x10        // Front End TX Configuration
//
#define DAVIS_FSCAL3             0xEF        // Frequency Synthesizer Calibration
#define DAVIS_FSCAL2             0x2B        // Frequency Synthesizer Calibration
#define DAVIS_FSCAL1             0x2F        // Frequency Synthesizer Calibration
#define DAVIS_FSCAL0             0x1F        // Frequency Synthesizer Calibration
//
#define DAVIS_RCCTRL1            0x00        // RC Oscillator Configuration
#define DAVIS_RCCTRL0            0x00        // RC Oscillator Configuration
//
#define DAVIS_FSTEST             0x59        // Frequency Synthesizer Calibration Control
#define DAVIS_PTEST              0x7F        // Production Test
#define DAVIS_AGCTEST            0x3F        // AGC Test
//
#define DAVIS_TEST2              0x81        // Various Test Settings
#define DAVIS_TEST1              0x35        // Various Test Settings
#define DAVIS_TEST0              0x09        // Various Test Settings
//
#define DAVIS_DISABLED           0x2E        // Disables GPIO's 
//
//  Table for FREQ2 settings.  (Vantage VUE - EU 868 mHz)
//
static const uint8_t __attribute__ ((progmem)) FREQ_2[5] =
{
  0x21, 0x21, 0x21, 0x21, 0x21
};  
//
//  Table for FREQ1 settings.
//
static const uint8_t __attribute__ ((progmem)) FREQ_1[5] =
{
  0x62, 0x65, 0x67, 0x64, 0x66
};  
//
//  Table for FREQ0 settings.
//
static const uint8_t __attribute__ ((progmem)) FREQ_0[5] =
{
  0xE2, 0x40, 0x9D, 0x11, 0x6F
};
//
//  CRC Lookup Table.
//
static const uint16_t __attribute__ ((progmem)) CRC_TABLE[256] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
//
//  Global Variables.
//
extern volatile uint8_t hopIndex;
extern volatile int  pktRssi;
extern volatile uint8_t pktLqi;
extern volatile int freqError;
extern volatile uint16_t pktCount;
extern volatile uint16_t pktMiss;
extern volatile uint8_t timer;
extern volatile uint8_t now;
extern volatile uint8_t next;
extern volatile uint8_t rxing;
extern volatile uint8_t hopping;
//
//  DAVIS Class.
//
class DAVIS
{
  private:
  
    static uint8_t rxBuffer[];
    static volatile uint8_t rxBufferIndex;
    static volatile uint8_t rxBufferLength;
    static volatile uint8_t freqComp[];
                  
  public:
    
    void begin(void);
    void reset(void);    
    void setRegisters(void);
    void cmdStrobe(uint8_t command);
    void writeRegister(uint8_t regAddr, uint8_t value);
    void writeBurst(uint8_t regAddr, uint8_t *buffer, uint8_t length);
    uint8_t readRegister(uint8_t regAddr);
    void readBurst(uint8_t regAddr, uint8_t *buffer, uint8_t length);
    uint8_t readStatus(uint8_t regAddr);
    void idle(void);
    void rx(void);
    void hop(void);
    void setFrequency(uint8_t index);
    void rxData(void);
    int available(void);
    uint8_t read(void);
    void flush(void);
    uint16_t calcCrc(uint8_t *buffer, uint8_t length);
    char calcRssi(uint8_t value);
    int calcFreqError(uint8_t value);
};

extern DAVIS Radio;
//
//
//

#endif
