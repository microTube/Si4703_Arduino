#ifndef SI4703_ARDUINO_h
#define SI4703_ARDUINO_h

#include <Arduino.h> 

#include <stdio.h>
#include <stdlib.h>
#define PGMSTR(x) (__FlashStringHelper*)(x)

//RDS Europe...
//Link: https://www.electronics-notes.com/articles/audio-video/broadcast-audio/rds-radio-data-system-pty-codes.php
const char Si4703_PTY_0 []  PROGMEM  = { "Unknown\0" };
const char Si4703_PTY_1 []  PROGMEM  = { "News\0" };
const char Si4703_PTY_2 []  PROGMEM  = { "Current Aff.\0" };
const char Si4703_PTY_3 []  PROGMEM  = { "Information\0" };
const char Si4703_PTY_4 []  PROGMEM  = { "Sport\0" };
const char Si4703_PTY_5 []  PROGMEM  = { "Education\0" };
const char Si4703_PTY_6 []  PROGMEM  = { "Drama\0" };
const char Si4703_PTY_7 []  PROGMEM  = { "Culture\0" };
const char Si4703_PTY_8 []  PROGMEM  = { "Science\0" };
const char Si4703_PTY_9 []  PROGMEM  = { "Varied\0" };
const char Si4703_PTY_10[]  PROGMEM  = { "Pop Music\0" };
const char Si4703_PTY_11[]  PROGMEM  = { "Rock Music\0" };
const char Si4703_PTY_12[]  PROGMEM  = { "Easy Listen\0" };
const char Si4703_PTY_13[]  PROGMEM  = { "Light Class\0" };
const char Si4703_PTY_14[]  PROGMEM  = { "Ser. Class\0" };
const char Si4703_PTY_15[]  PROGMEM  = { "Other Music\0" };
const char Si4703_PTY_16[]  PROGMEM  = { "Weather\0" };
const char Si4703_PTY_17[]  PROGMEM  = { "Finance\0" };
const char Si4703_PTY_18[]  PROGMEM  = { "Children\0" };
const char Si4703_PTY_19[]  PROGMEM  = { "Social Aff.\0" };
const char Si4703_PTY_20[]  PROGMEM  = { "Relig Talk\0" };
const char Si4703_PTY_21[]  PROGMEM  = { "Phone-in\0" };
const char Si4703_PTY_22[]  PROGMEM  = { "Travel/Publ\0" };
const char Si4703_PTY_23[]  PROGMEM  = { "Leisure\0" };
const char Si4703_PTY_24[]  PROGMEM  = { "Jazz Music\0" };
const char Si4703_PTY_25[]  PROGMEM  = { "Countr Music\0" };
const char Si4703_PTY_26[]  PROGMEM  = { "Nation Music\0" };
const char Si4703_PTY_27[]  PROGMEM  = { "Oldies Music\0" };
const char Si4703_PTY_28[]  PROGMEM  = { "Folk Music\0" };
const char Si4703_PTY_29[]  PROGMEM  = { "Documentary\0" };
const char Si4703_PTY_30[]  PROGMEM  = { "Alarm Test!!0" };
const char Si4703_PTY_31[]  PROGMEM  = { "Alarm\0" };
const char* const string_table_si4703[] PROGMEM = {
Si4703_PTY_0,  Si4703_PTY_1,  Si4703_PTY_2,  Si4703_PTY_3, 
Si4703_PTY_4,  Si4703_PTY_5,  Si4703_PTY_6,  Si4703_PTY_7, 
Si4703_PTY_8,  Si4703_PTY_9,  Si4703_PTY_10, Si4703_PTY_11, 
Si4703_PTY_12, Si4703_PTY_13, Si4703_PTY_14, Si4703_PTY_15, 
Si4703_PTY_16, Si4703_PTY_17, Si4703_PTY_18, Si4703_PTY_19, 
Si4703_PTY_20, Si4703_PTY_21, Si4703_PTY_22, Si4703_PTY_23, 
Si4703_PTY_24, Si4703_PTY_25, Si4703_PTY_26, Si4703_PTY_27, 
Si4703_PTY_28, Si4703_PTY_29, Si4703_PTY_30, Si4703_PTY_31};

class Si4703_Arduino
{
  public:
  uint8_t				resetPin;
  uint8_t				rdsPin;
  uint8_t				regionEuropa;
  uint8_t	 			rssi;
  boolean				validChannel;
  uint8_t 				stereo;
  uint8_t				afc;
  uint8_t				BLTF;
  uint8_t				STCF;
  uint16_t				fmChannel;
  uint8_t				volume;
  uint8_t 				SMUTER 		 = 3;
  uint8_t 				SMUTEA 		 = 0;
  
  boolean				i2cError	 = false;
  
  uint8_t 				PSflag   	 = false;
  uint8_t 				RTflag   	 = false;
  uint8_t 				PTYflag  	 = false;
  uint8_t 				CTflag   	 = false;
  uint8_t 				RDSflag      = false;
  uint8_t				AFflag		 = false;
  const uint16_t 		channelBTM 	 = 875;
  const uint16_t 		channelTOP 	 = 1080;
  volatile uint8_t 		updateTime   = false;
  uint8_t 				Si4703_Addr  = 0x10;

  uint16_t 				si4703_registers[16];
  
  uint8_t				RDSData[10];
  char 					PS[10];
  char 					PSTemp[10];
  char 					RT[65];
  char 					RTTemp[65];
  char 					PTY[20];
  uint8_t				PICODE[2];
  uint8_t				REG_PI[2];
  uint8_t				AF_PICODE[2];
  uint8_t				CLOCK[5];
  uint8_t				DATE[5];
  uint16_t				AF[50];
  uint8_t 				AF_Zaehler;
  uint16_t 				_Puffer0;
  uint16_t 				_Puffer1;
  uint8_t 				_AF_scan_a;
  uint8_t 				_AF_scan_b;
  uint8_t				AF_trying;
   int8_t  				Timezone;
  uint16_t  			_Utcoffset;
  uint16_t  			_Utcoffsetdate;
  uint16_t  			_Lo;
  uint16_t  			_Hi;
  uint16_t  			_Stunden;
  uint16_t  			_Minuten;
  uint16_t  			_Sekunden;
  uint16_t  			_STD;
  uint16_t 				_MJD;
  uint16_t  			_d;
  uint16_t  			_m;
  uint16_t  			_y;
  uint16_t  			_Wochentag;
  uint8_t 				BLER[4];
  uint8_t 				GroupType;
  uint8_t 				GroupWord;
  uint8_t 				RDSSync;
  uint8_t 				RDSReady;
  uint8_t  				oldRTcount;
  uint8_t  				RT_step;
  boolean				runDecodePS;
  unsigned     			_PTY, _PTY1, _PTY2;
  uint8_t				TC;
  const int     		_nd[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int16_t       		_j;
  
	   Si4703_Arduino	();
  void initRadio 		(uint8_t Power);
  void reset 			(void);
  void initOsc 			(void);
  void setPower 		(int Power);
  void setMute			(int muteState);
  void softMute 		(int muteState);
  void initRDS 			(void);
  void setProperties 	(void);
  void setSterMonBL 	(uint8_t SterMonBL);
  boolean seekStation 	(int seekDir,boolean (*Display)(),uint8_t rssi=26);
  void setChannel 		(int channel, boolean clearRDS_=true);
  void tuneData 		(void);
  void setVolume 		(int volume);
  void softVolume 		(uint8_t volume, int dir);
  byte updateRegisters	(void);
  void readRegisters	(void);
  boolean readRDS 		(void);
  int  checkRDSErr 		(void);
  void decodePS 		(void);
  void decodePI 		(void);
  void decodeRT 		(void);
  void decodePTY		(void);
  char*fStr				(char* str);
  void decodeCT 		(void);
  void decodeTC 		(void);
  void convertMJD 		(unsigned long MJD);
  int  decodeAF 		(void);
  void seekAF 			(boolean (*Display)(int));
  void clearRDS 		(void);
  
  private:

  const uint16_t  FAIL 				= 0;
  const uint16_t  SUCCESS 			= 1;
  const uint16_t  DEVICEID 			= 0x00;
  const uint16_t  CHIPID 			= 0x01;
  const uint16_t  POWERCFG 			= 0x02;
  const uint16_t  GPIO2_RDSIEN 		= 0x02;
  const uint16_t  CHANNEL 			= 0x03;
  const uint16_t  SYSCONFIG1 		= 0x04;
  const uint16_t  SYSCONFIG2 		= 0x05;
  const uint16_t  SYSCONFIG3 		= 0x06;
  const uint16_t  STATUSRSSI 		= 0x0A;
  const uint16_t  READCHAN 			= 0x0B;
  const uint16_t  RDSA 				= 0x0C;
  const uint16_t  RDSB 				= 0x0D;
  const uint16_t  RDSC 				= 0x0E;
  const uint16_t  RDSD 				= 0x0F;
  const uint16_t  RDSIEN 			= 0x0F;
	
  const uint16_t  RDSAH 			= 0x00;
  const uint16_t  RDSAL 			= 0x01;
  const uint16_t  RDSBH 			= 0x02;
  const uint16_t  RDSBL 			= 0x03;
  const uint16_t  RDSCH 			= 0x04;
  const uint16_t  RDSCL 			= 0x05;
  const uint16_t  RDSDH 			= 0x06;
  const uint16_t  RDSDL 			= 0x07;
	
  //Register 0x02 - POWERCFG
  const uint16_t  SMUTE 			= 15;
  const uint16_t  DMUTE 			= 14;
  const uint16_t  SKMODE 			= 10;
  const uint16_t  SEEKUP 			= 9;
  const uint16_t  SEEK 				= 8;
  const uint16_t  RDS_VERB 			= 11;
  const uint16_t  MONO 				= 1;
  
  //Register 0x03 - CHANNEL
  const uint16_t  TUNE 				= 15;

  //Register 0x04 - SYSCONFIG1
  const uint16_t  RDS 				= 12;
  const uint16_t  DE 				= 11;

  //Register 0x05 - SYSCONFIG2
  const uint16_t  SPACE1 			= 5;
  const uint16_t  SPACE0 			= 4;
	
  //Register 0x06 - SYSCONFIG 3
  const uint16_t  VOLEXT 			= 8;
	
  //Register 0x0A - STATUSRSSI
  const uint16_t  RDSR 				= 15;
  const uint16_t  STC 				= 14;
  const uint16_t  SFBL 				= 13;
  const uint16_t  AFCRL 			= 12;
  const uint16_t  RDSS 				= 11;
  const uint16_t  STEREO 			= 8;
};
#endif
