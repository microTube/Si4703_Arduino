#include "Arduino.h"
#include <Wire.h>
#include "Si4703_Arduino.h"

/*
Si4703_Arduino Bibliothek
Version:   0.1-16042017
Ersteller: ArduinoTube

###########################FM Radio###########################
:: Empfang von 87.50 MHz bis 108.00 MHz                     ::
:: Volle RDS unterstützung (PS,PTY,RT,CT,TP/TA,AF)          ::
:: Signalstärke und -rauschabstand können ausgelesen werden ::
:: ...                                                      ::
**************************************************************
*/

/**********************************************************
*-----------------------Konstruktor-----------------------*
**********************************************************/
Si4703_Arduino::Si4703_Arduino ()
{
	
}

/*******************************************************
*----------Si4703_Arduino Initialisieren fuer Empfang----------* 
********************************************************/
void Si4703_Arduino::initRadio (uint8_t Power)
{
  int oldVolume = volume;
  Wire.begin();
  if(!Power)softVolume(volume,-1);
  pinMode(rdsPin,INPUT_PULLUP);
  reset();
  initOsc();
  setPower(Power);
  initRDS();
  setProperties();
  if(Power)softVolume(oldVolume,+1);
  delay(110);
}

/*******************************************************
*---------------------Si4703_Arduino Mute----------------------*
*******************************************************/
void Si4703_Arduino::setMute (int muteState)
{
	readRegisters();
	if(muteState)	si4703_registers[POWERCFG] &= ~(1<<DMUTE);
	else		 	si4703_registers[POWERCFG] |=  (1<<DMUTE);
	updateRegisters();
}

/*******************************************************
*-------------------Si4703_Arduino Soft Mute-------------------*
*******************************************************/
void Si4703_Arduino::softMute (int muteState)
{
	readRegisters();
	if(!muteState)	si4703_registers[POWERCFG] |=  (1<<SMUTE);
	else		 	si4703_registers[POWERCFG] &= ~(1<<SMUTE);
	updateRegisters();
}

/*******************************************************
*---------------------Si4703_Arduino Reset---------------------* 
********************************************************/
void Si4703_Arduino::reset (void)
{
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(1);
  digitalWrite(resetPin, HIGH);
}

/*******************************************************
*----------------Si4703_Arduino Init Oscillator----------------* 
********************************************************/
void Si4703_Arduino::initOsc (void)
{
  readRegisters();
  si4703_registers[0x07] = 0x8100;
  updateRegisters();
  delay(800);
}

/*******************************************************
*------------------Si4703_Arduino Power On/Off-----------------* 
********************************************************/
void Si4703_Arduino::setPower (int Power)
{
  if(Power)si4703_registers[POWERCFG] = 0x4001;
  else	   si4703_registers[POWERCFG] = 0x00;
}

/*******************************************************
*-------------------Si4703_Arduino RDS Init--------------------* 
********************************************************/
void Si4703_Arduino::initRDS (void)
{
  si4703_registers[POWERCFG]   |= (1<<RDS_VERB);
  si4703_registers[SYSCONFIG1] |= (1<<RDSIEN);
  si4703_registers[SYSCONFIG1] |= (1<<RDS);
  si4703_registers[SYSCONFIG1] |= (1<<GPIO2_RDSIEN);
}

/*******************************************************
*----------------Si4703_Arduino set Properties-----------------* 
********************************************************/
void Si4703_Arduino::setProperties (void)
{
  if(regionEuropa)
  si4703_registers[SYSCONFIG1] |= (1<<DE);
  si4703_registers[SYSCONFIG2] |= (1<<SPACE0);
  si4703_registers[SYSCONFIG3] |= ((SMUTEA<<12) | (SMUTER<<14));
  updateRegisters();
}

/**********************************************************
*-------------Si4703_Arduino set St/Mo Blend--------------*
// Quality_ = 0 --> RSSI: 31 - 49 dBµV :: Default value  //
// Quality_ = 1 --> RSSI: 37 - 55 dBµV :: +6 dBµV higher //
// Quality_ = 2 --> RSSI: 19 - 37 dBµV :: -12dBµV lower  //
// Quality_ = 3 --> RSSI: 25 - 43 dBµV :: -6 dBµV lower  //
// Quality_ = 4 --> RSSI: xx - xx dBµV :: always Mono    //
*---------------------------------------------------------* 
**********************************************************/
void Si4703_Arduino::setSterMonBL (uint8_t SterMonBL)
{
  readRegisters();
  int Quality_ = 0;
  if(SterMonBL==0)Quality_=2;
  if(SterMonBL==1)Quality_=3;
  if(SterMonBL==3)Quality_=1;

  si4703_registers[SYSCONFIG1] &= ~(2<<6);
  if(SterMonBL<4)
  {
	  si4703_registers[SYSCONFIG1] |= ((Quality_&0x03)<<6);
	  si4703_registers[POWERCFG] &= ~(MONO<<13);
  }
  if(SterMonBL==4)
  {
	  si4703_registers[POWERCFG] |= (MONO<<13);
  }
  updateRegisters();
}

/*******************************************************
*-----------------Si4703_Arduino Seek Station------------------* 
********************************************************/
boolean Si4703_Arduino::seekStation (int seekDir, boolean (*Display)(),uint8_t rssi=26)
{
  bool seekStation =  true;
  boolean limit    = false;
  int oldVolume    = volume;
  int oldChannel   = fmChannel;
  softVolume(volume,-1);
  clearRDS();
  int breakSeek=false;
  while(seekStation==true)
  {
    if(seekDir>0)fmChannel++;
    if(seekDir<0)fmChannel--;
    if(fmChannel>channelTOP){fmChannel=channelBTM;limit=true;}
	if(fmChannel<channelBTM){fmChannel=channelTOP;limit=true;}
    setChannel(fmChannel);
	breakSeek=Display();
	if((breakSeek)||(oldChannel==fmChannel))seekStation=false;
    if((!afc)&&(this->rssi>rssi))
    {delay(500);tuneData();if((afc==0)&&(this->rssi>rssi))seekStation=false;delay(150);}
  }
  softVolume(oldVolume,+1);
  return(limit);
}

/*******************************************************
*---------------------Si4703_Arduino Tune----------------------* 
********************************************************/
void Si4703_Arduino::setChannel (int channel, boolean clearRDS_=true)
{
  if(clearRDS_)clearRDS();
  readRegisters();
  if(channel>channelTOP)channel=channelBTM;
  if(channel<channelBTM)channel=channelTOP;  
  int newChannel  = channel * 10;
      newChannel -= 8750;
	  newChannel /= 10;
  si4703_registers[CHANNEL] &= 0xFE00;
  si4703_registers[CHANNEL] |= newChannel;
  si4703_registers[CHANNEL] |= (1<<TUNE);
  updateRegisters();
  delay(60);
  tuneData();
  si4703_registers[CHANNEL] &= ~(1<<TUNE);
  updateRegisters();
}

/*******************************************************
*--------------------Si4703_Arduino Volume---------------------* 
********************************************************/
void Si4703_Arduino::setVolume (int volume)
{
  readRegisters();
  if(volume < 0)   volume = 0;
  if(volume > 30) volume = 30;
  if(volume <= 15)
  {
	si4703_registers[SYSCONFIG2] &= 0xFFF0;
    si4703_registers[SYSCONFIG2] |= volume;
	si4703_registers[SYSCONFIG3] |= (1<<VOLEXT);
  }
  if(volume > 15)
  {
	si4703_registers[SYSCONFIG2] &= 0xFFF0;
    si4703_registers[SYSCONFIG2] |= volume-15;
	si4703_registers[SYSCONFIG3] &= (~(1<<VOLEXT));
  }
  this->volume = volume;
  updateRegisters();
}

/*******************************************************
*----------------Si4703_Arduino Soft Volume Up-----------------* 
********************************************************/
void Si4703_Arduino::softVolume (uint8_t volume, int dir)
{
  if(dir>0)
  for(int v = 0; v<=volume; v++){setVolume(v);delay(11);}

  else
  for(int v = volume; v>=0; v--){setVolume(v);delay(5);}
}

/*******************************************************
*---------------------Si4703_Arduino Update--------------------* 
********************************************************/
byte Si4703_Arduino::updateRegisters(void) 
{
  Wire.beginTransmission(Si4703_Addr);
  for(int regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) 
  {
    byte high_byte = si4703_registers[regSpot] >> 8;
    byte low_byte  = si4703_registers[regSpot] & 0x00FF;
    Wire.write(high_byte);
    Wire.write(low_byte);
  }
  byte ack = Wire.endTransmission();
  i2cError = ack ? true : false;
  if(ack != 0)return(false);
  return(true);
}

/*******************************************************
*---------------------Si4703_Arduino Read----------------------* 
********************************************************/
void Si4703_Arduino::readRegisters(void)
{
  int x = 0x0A;
  Wire.requestFrom(Si4703_Addr, 32);
  while(true)
  {
	if(x == 0x10) x = 0; 
    si4703_registers[x] = Wire.read() << 8;
    si4703_registers[x] |= Wire.read();
	x++;
	if(x == 0x0A)break;
  }
}

/*******************************************************
*------------------Si4703_Arduino get Tune Data----------------* 
********************************************************/
void Si4703_Arduino::tuneData (void)
{
	readRegisters();
    RDSSync   	= si4703_registers[STATUSRSSI] & (1<<RDSS) ? true : false;
	RDSReady  	= si4703_registers[STATUSRSSI] & (1<<RDSR) ? true : false;
	stereo 		= ((si4703_registers[STATUSRSSI]>>STEREO)&0x01);
	afc 	 	= ((si4703_registers[STATUSRSSI]>>AFCRL)&0x01);
	rssi	 	=   si4703_registers[STATUSRSSI] & 0xFF;
	fmChannel	=   si4703_registers[READCHAN] & 0x03FF;
	fmChannel	+=  875;
	validChannel= ((rssi>20)&&(!afc));
	updateRegisters();
}

/*******************************************************
*--------------------Si4703_Arduino read RDS-------------------* 
********************************************************/
boolean Si4703_Arduino::readRDS (void)
{ 
	int RDS_intState = digitalRead(rdsPin);
	if(RDS_intState==LOW)
    {
	   readRegisters();
	   BLER[0] = ((si4703_registers[STATUSRSSI] & 0x0600) >> 9); //Mask in BLERA (4...5)
	   BLER[1] = ((si4703_registers[READCHAN] & 0xC000) >> 14);  //Mask in BLERB (6...7)
	   BLER[2] = ((si4703_registers[READCHAN] & 0x3000) >> 12);  //Mask in BLERC (8...9)
	   BLER[3] = ((si4703_registers[READCHAN] & 0xC00) >> 10);   //Mask in BLERD (10...11)
	   if(si4703_registers[STATUSRSSI] & (1<<RDSS))
	   {
		 if(checkRDSErr())
		 {
		 // Block A 16 Bit
		 RDSData[RDSAH] = (si4703_registers[RDSA] & 0xFF00) >> 8; // MSB
		 RDSData[RDSAL] = (si4703_registers[RDSA] & 0x00FF);      // LSB
		 
		 // Block B 16 Bit
		 RDSData[RDSBH] = (si4703_registers[RDSB] & 0xFF00) >> 8; // MSB
		 RDSData[RDSBL] = (si4703_registers[RDSB] & 0x00FF);      // LSB
		 
		 // Block C 16 Bit
		 RDSData[RDSCH] = (si4703_registers[RDSC] & 0xFF00) >> 8; // MSB
		 RDSData[RDSCL] = (si4703_registers[RDSC] & 0x00FF);      // LSB
		 
		 // Block D 16 Bit
		 RDSData[RDSDH] = (si4703_registers[RDSD] & 0xFF00) >> 8; // MSB
		 RDSData[RDSDL] = (si4703_registers[RDSD] & 0x00FF);      // LSB
		 
		 // Block B 16 Bit
		 GroupType = (RDSData[RDSBH]>>4);
		 GroupWord = (RDSData[RDSBH]>>7);
		 RDSSync   = si4703_registers[STATUSRSSI] & (1<<RDSS) ? true : false;
		 RDSReady  = si4703_registers[STATUSRSSI] & (1<<RDSR) ? true : false;		  
		  return(true);
		 }
		 runDecodePS = false;// Hiermit wird verhindert dass beim dynamischen Sendernamen
							 // es zu Überschneidungen kommt (for no overcrosses of dynamic PS)
	   }
   }
   return (false);
}

/*******************************************************
*----------------Überprüfe nach Fehlern----------------* 
********************************************************/
int Si4703_Arduino::checkRDSErr (void)
{
	if((BLER[0]==0)&&(BLER[1]==0)&&(BLER[2]==0)&&(BLER[3]==0))return(true);
	return(false);
}

/*******************************************************
*---------------Decodiere RDS-Sendernamen--------------* 
********************************************************/
void Si4703_Arduino::decodePS (void)
{
  int PSAdress  	= 0;
  int rdsErrors 	= checkRDSErr();
  int lengthOfTmp	= strlen(PSTemp);
  if((GroupType==0)&&(PSflag==false)&&(rdsErrors))
  {
	
	PSAdress=RDSData[RDSBL]&3;
	PSAdress*=2;
	if((runDecodePS==false)&&(PSAdress==0))runDecodePS=true;
	if((runDecodePS==false)&&(lengthOfTmp))for(int clearPS = 0; clearPS<=8; clearPS++)PSTemp[clearPS]=0;
	if(runDecodePS)
	{
		PSTemp[PSAdress]   = RDSData[RDSDH];
		PSTemp[PSAdress+1] = RDSData[RDSDL];
		PSTemp[PSAdress+2] = '\0';	
	}
	if(strlen(PSTemp)==8)
    {
	  runDecodePS = false;
      PSflag  	  = true;
	  for(int copyPS = 0; copyPS<8; copyPS++){PS[copyPS]=PSTemp[copyPS];PS[copyPS+1]=0;}
      for(int clearPS = 0; clearPS<=8; clearPS++)PSTemp[clearPS]=0;
    }
  }
}

/*******************************************************
*---------------Decodiere RDS-Sendernamen--------------* 
********************************************************/
void Si4703_Arduino::decodePI (void)
{
    int rdsErrors = checkRDSErr();
	if(GroupType==0)
	{
		if((RDSData[RDSAH]&0xFF)&&(RDSData[RDSAL]&0xFF)&&(rdsErrors))
		{
			PICODE[0] 		   = RDSData[RDSAH]&0xFF;
			PICODE[1]		   = RDSData[RDSAL]&0xFF;
			REG_PI[0]		   = PICODE[0]&0xF0;// xxxx0000
			REG_PI[1]		   = PICODE[1];
		}	
	}
}

/*******************************************************
*----------------Decodiere RDS-Radiotext---------------* 
********************************************************/
void Si4703_Arduino::decodeRT (void)
{
  volatile uint8_t RTAdress  = 0;
  volatile uint8_t LZA       = 0;
  volatile uint8_t LZE       = 0;
  volatile uint8_t strl      = 0;
  volatile uint8_t rdsErrors = checkRDSErr();
  if((GroupType==2)&&(RT_step<2)&&(RTflag==false)&&(rdsErrors))
  {
      RTAdress=RDSData[RDSBL]&15;
      RTAdress*=4;
      if((RT_step==0)&&(RTAdress==0))
      {
         RT_step=1;
         for(int Clear=0; Clear<=64; Clear++)RTTemp[Clear]= 0;
      }
	  
	  strl=strlen(RTTemp);
      if((RT_step==1)&&(strl)&&(RTAdress<oldRTcount))
      {
		  for(int Clear=0; Clear<=64; Clear++)RT[Clear] = 0;
          for(LZA=0;RTTemp[LZA]<=32; LZA++);
          for(LZE=64;RTTemp[LZE]<=32; LZE--);
		  
          for(int copy = LZA; copy<=LZE; copy++)
		  {
			  RT[copy-LZA] = RTTemp[copy];
			  RT[copy-LZA+1] ='\0';
		  }
          for(int Clear=0; Clear<=64; Clear++)RTTemp[Clear]= 0;
          RT_step=0;
          RTflag = true;
		  oldRTcount=0;
      }
	  
	  oldRTcount=RTAdress;
      if(RT_step==1)
      {	  
          RTTemp[RTAdress]   = RDSData[RDSCH];
          RTTemp[RTAdress+1] = RDSData[RDSCL];
          RTTemp[RTAdress+2] = RDSData[RDSDH];
          RTTemp[RTAdress+3] = RDSData[RDSDL];
		  RTTemp[RTAdress+4] = '\0';
      }
  }
}

/*******************************************************
*----------------------RDS TC--------------------------*
*******************************************************/
void Si4703_Arduino::decodeTC (void)
{
  int rdsErrors = checkRDSErr();
  if((GroupType==0)&&(rdsErrors))
  {
	  if(((RDSData[RDSBH]&(1<<2))==0)&&((RDSData[RDSBL]&(1<<4)))) 		TC=1;	// EON is available
	  else if(((RDSData[RDSBH]&(1<<2)))&&((RDSData[RDSBL]&(1<<4))==0)) 	TC=2;	// TP Signal is available
	  else if(((RDSData[RDSBH]&(1<<2)))&&((RDSData[RDSBL]&(1<<4))))  	TC=3;	// Traffic announcement is now
	  else TC=0;
  }
}

/*******************************************************
*---------------Decodiere RDS-Programmtyp--------------* 
********************************************************/
void Si4703_Arduino::decodePTY (void)
{
  int rdsErrors = checkRDSErr();
  if((GroupType==0)&&(rdsErrors))
  {
    _PTY1 = ((RDSData[RDSBL]&0xE0)>>5);
    _PTY2 = (((RDSData[RDSBH]&0x3))<<3);
    _PTY = _PTY1+_PTY2;
	strcpy_P(PTY, (char*)pgm_read_word(&(string_table_si4703[_PTY])));
  }
}

//### Flash String Conversion #################################################
#define MAX_STRING 60
char* Si4703_Arduino::fStr(char* str) // Return Ram bufferd String
{
  char stringBuffer[MAX_STRING+1];
  strcpy_P(stringBuffer, (char*)str);
  return (stringBuffer);
}

/*******************************************************
*------------Decodiere RDS-Uhrzeit und Datum-----------* 
********************************************************/
void Si4703_Arduino::decodeCT (void)
{
  int rdsErrors = checkRDSErr();
  if((GroupType==4)&&(CTflag==false)&&(rdsErrors))
  {
	_MJD=0;
	_Utcoffset = RDSData[RDSDL]&31;                 
	_Utcoffset = _Utcoffset / 2;
	_Lo = RDSData[RDSDL] / 64;                      
	_Hi = RDSData[RDSDH]&15;                        
	_Hi = _Hi * 4;
	_Minuten = _Hi + _Lo;
	_Lo = RDSData[RDSDH] / 16;                      
		
	_Hi = RDSData[RDSCL]&1;                          
	_Hi *= 16;
	_STD = _Hi + _Lo;
	if(RDSData[RDSDL]>>5==1)_STD -= _Utcoffset;
	else					_STD += _Utcoffset;
	_Stunden=_STD;
	_Utcoffsetdate=_Utcoffset;
	Timezone=_Utcoffsetdate;
	_MJD+=(((RDSData[RDSCH]<<7)+(RDSData[RDSCL]>>1)+((RDSData[RDSBL]&0b11)<<15)));
	if(_STD>=24){_STD-=24;_MJD++;}
	if((_STD!=32)&&(_Minuten!=32)&&((_Stunden!=0)||(_Minuten!=0)))
	{
		CLOCK[0]=_Stunden;
		CLOCK[1]=_Minuten;
		_Sekunden=0;
	}
	if(_MJD)
	{
		convertMJD(_MJD);
		DATE[0]=_d;
		DATE[1]=_m;
		DATE[2]=_y;
		DATE[3]=_Wochentag;
		_MJD=0;
		if((CLOCK[0]<=23)&&(CLOCK[0]>=0)&&(CLOCK[1]<=59)&&(CLOCK[1]>=0)&&(DATE[0]>0)&&(DATE[1]>0)&&(DATE[2]>0))
		CTflag = true;
	updateTime = true;
	}
  }
}

/*******************************************************
*------------Wandle den RDS-MJD zum Datum um-----------* 
********************************************************/
void Si4703_Arduino::convertMJD (unsigned long MJD)
{
	_d = 0;
	_m = 0;
	_y = 14;
	_j = MJD-56658+1;  // 56658 = MJD of 01.01.2014
	_Wochentag=2;
	for(int zaheler=_Wochentag; zaheler<=(_j); zaheler++)
	{
		_Wochentag++;
		if(_Wochentag>6)_Wochentag=0;
	}
	while (_j>0)
	{
		_d = _j;
		if (_m>=12) {_m = 0; _y++;} // Wenn das Jahr vollendet ist, dann Jahreswechsel und Monatsbeginn "Januar"

		_j = _j-_nd[_m];
		
		_m++;
		if (_y%4==0 && _m==2) _j--;
	}
}

/*******************************************************
*----------Decodiere RDS-Alternativ Frequenzen---------* 
********************************************************/
int Si4703_Arduino::decodeAF (void)
{
	 int rdsErrors = checkRDSErr();
	 if((AF_Zaehler<40)&&(GroupType==0)&&(rdsErrors))
	 {
		 _Puffer0=((RDSData[RDSCH]+channelBTM));
		 _Puffer1=((RDSData[RDSCL]+channelBTM));
		 if ((_Puffer0>channelBTM)&&(_Puffer0<=channelTOP)&&(_Puffer1>channelBTM)&&(_Puffer1<=channelTOP))
		 {
			 for (_AF_scan_a = 0; ((_AF_scan_a <= 40) && ((AF[_AF_scan_a] != _Puffer0)) && (_Puffer0 != _Puffer1)); _AF_scan_a ++);
			 for (_AF_scan_b = 0; ((_AF_scan_b <= 40) && ((AF[_AF_scan_b] != _Puffer1)) && (_Puffer0 != _Puffer1)); _AF_scan_b ++);
			
			 if (_AF_scan_a >= 40){AF[AF_Zaehler]=_Puffer0;AF_Zaehler++;AF_trying=0;}
			 if (_AF_scan_b >= 40){AF[AF_Zaehler]=_Puffer1;AF_Zaehler++;AF_trying=0;}
			 AFflag	= true;
		 }
	 }
	 return (AF_Zaehler);
}

/*******************************************************
*----------------------AF Suchlauf---------------------* 
********************************************************/
void Si4703_Arduino::seekAF (boolean (*Display)(int))
{
	char oldPS[10];
	uint16_t oldFmChannel = fmChannel;
	boolean  breakSeek	  = false;
	uint8_t  oldVolume = volume;
	AF_PICODE[0]=REG_PI[0];
	AF_PICODE[1]=REG_PI[1];
	REG_PI[0]=PICODE[0]=RDSData[RDSAH]=0;
	REG_PI[1]=PICODE[1]=RDSData[RDSAL]=0;
	strcpy(oldPS,PS);
	softVolume(volume,-1);
	for(int AFcount = 0; AFcount <= AF_Zaehler; AFcount++)
	{	
		breakSeek = Display(0);
		while((oldFmChannel==AF[AFcount])&&(AFcount!=AF_Zaehler))AFcount++;
		if(AFcount!=AF_Zaehler)
		{
			setChannel(AF[AFcount],false);
			REG_PI[0]=REG_PI[1]=0;
			if(validChannel)
			for(int countRDS = 0; ((countRDS <= 60)&&(!breakSeek)); countRDS++)
			{
				breakSeek = Display(0+countRDS>30?4:0);
				readRDS();
				decodePI();
				if((REG_PI[0])&&(REG_PI[1])) break;
				else if(countRDS==50)break;
			}
			
			if((AF_PICODE[0]==REG_PI[0])&&(AF_PICODE[1]==REG_PI[1]))
			{
				softVolume(oldVolume,+1);
				Display(1);delay(1000);
				AF_trying=0;
				fmChannel=AF[AFcount];
				break;
			}
		}
		if((AFcount==AF_Zaehler)||(breakSeek))
		{
			softVolume(oldVolume,+1);
			if(!breakSeek){AF_trying++;Display(2);}
			else		   Display(3);
			REG_PI[0]=AF_PICODE[0];
			REG_PI[1]=AF_PICODE[1];
			setChannel(oldFmChannel,false);
			delay(1000);
			break;
		}
	}
	strcpy(PS,oldPS);
	AF_PICODE[0]=0;
	AF_PICODE[1]=0;
}

void Si4703_Arduino::clearRDS (void)
{
	PSflag   	 = false;
	RTflag   	 = false;
	PTYflag  	 = false;
	CTflag   	 = false;
	RDSflag      = false;
	CTflag 		 = false;
	updateTime   = false;
	AFflag		 = false;
	AF_trying	 = 0;
	AF_Zaehler	 = 0;
	TC			 = 0;
	for(int _clear=0; _clear<64; _clear++)
	{
		 RT[_clear]=0;
		 RTTemp[_clear]=0;
		 if(_clear<8)PSTemp[_clear]=0;
		 if(_clear<8)PS[_clear]=0;
		 if(_clear<10)PTY[_clear]=0;
		 if(_clear<41)AF[_clear]=0;
		 if(_clear<2)PICODE[_clear]=0;
	}
}
