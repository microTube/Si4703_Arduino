/**************************************************
 * --------FM Radio with Si4703 DSP Tuner-------- *
 * Frequency range:   87.5 - 108.0MHz (in 100kHz) *
 * Volume range:      0 - 15                      *
 * Stations:          20                          *
 * RDS:               PS,RT,PTY,AF,CT,Traffic     *
 *                                                *
 * -------------Description of Sketch------------ *
 * Programmed for Arduino, runs for example on an *
 * Arduino Pro Mini with 3,3V @8MHz (ATMega328P)  *
 * Required Memory for Flash:  15096 Bytes ( 49%) *
 * Required Memory for SRAM:    1059 Bytes ( 51%) *
 * ----------Arduino Ports & Connections--------- *
 * Radio Si4703                                   *
 * RESET = 2                                      *
 * GPIO2 = 3                                      *
 * SDIO  = A4 (Only by Arduino Pro Mini)          *
 * SCLK  = A5 (Only by Arduino Pro Mini)          *
 * -----------------Conditions...---------------- *
 * - All what you do, with this code and this     *
 *   parts is at your own risk, be careful!       *
 *   I don't take any responsibility              *
 *   (Have fun, enjoy :)                          *
 *   Kind regards, ArduinoTube                    *
***************************************************/ 
#include <Si4703_Arduino.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <stdbool.h>

Si4703_Arduino Radio;
int volume      = 15;
int channel     = 980;
bool      Power = true;

const int UP    = +1;
const int DOWN  = -1;

const int ChannelLOEEP = 0;
const int ChannelHIEEP = 1;
const int VolumeEEP    = 2;

const int seekDX	     = false;

void setup() 
{
  Serial_writeBegin();
  EEPROM_readData();
  Radio_init();
  Serial_Data();
}

void EEPROM_readData (void)
{
  channel = EEPROM.read(ChannelHIEEP) << 8;
  channel+= EEPROM.read(ChannelLOEEP)&0xFF;
  volume  = EEPROM.read(VolumeEEP);
  if((volume>30)||(volume<0))       volume=0;
  if((channel>1080)||(channel<875)) channel=875;
}

void Radio_init (void)
{
  Radio.resetPin = 19;
  Radio.rdsPin   = 18;
  Radio.regionEuropa = true; //If your region is EU
  Radio.initRadio(Power);
  Radio.setChannel(channel);
  Radio.setVolume(volume);  
}

void loop()
{
  Serial_Command();
  Radio_loopRDS();
}

void Serial_Command (void)
{
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'u')Radio_tuneStation( UP );
    if (ch == 'd')Radio_tuneStation(DOWN);
    if (ch == 'w'){Radio.seekStation( UP ,Serial_Channel,seekDX);EEPROM_saveChannel();}
    if (ch == 'q'){Radio.seekStation(DOWN,Serial_Channel,seekDX);EEPROM_saveChannel();}
    if (ch == 't'){if(Radio.AF_Zaehler){Radio.seekAF(Serial_Channel);Serial_Tune();}}
    if (ch == 'f')Radio_tuneFrequency();
    if (ch == 'i')Serial_Data();
    if (ch == '+')Radio_setVolume( UP );
    if (ch == '-')Radio_setVolume(DOWN);
    if (ch == 'l')Radio_listFavourite();
    if (ch == 'x')Radio_setFavourite();
    if (ch == '?')Serial_writeBegin();
          Radio_tuneFavourite(ch);
  }  
}

void Serial_Data (void)
{
  Radio.tuneData();
  Serial_Tune();
  Serial_Volume();
}

void Serial_Tune (void)
{
  Serial_Channel();
  Serial.print(F("TUNE: "));
  Serial.print(Radio.rssi);Serial.print(F("dB - "));
   
  if(Radio.stereo)Serial.println(F("STEREO"));
  else              Serial.println(F("MONO"));
}

int Serial_Channel (void)
{
  Radio.tuneData();
  channel=Radio.fmChannel;
  Serial.print(("FM: "));
  Serial.print(channel/10);
  Serial.print(F("."));
  Serial.print(channel%10);Serial.println(F(" MHz"));
  return(Serial.available());
}

void Serial_Volume (void)
{
  Serial.print(F("VOLUME: "));Serial.println(volume);
}

void Serial_RDS (void)
{
  char Buffer[32];
  if(Radio.PSflag==true)
  {
    Serial.print(F("\nPS:"));
    Serial.print(Radio.PS);
    Serial.print(F(" | "));
    Serial.print(Radio.PTY);
    sprintf(Buffer,"%02X%02X\0",Radio.PICODE[0],Radio.PICODE[1]);
    Serial.print(F("\nPI: "));
    Serial.println(Buffer);
    Radio.PSflag=false;
  }
  if(Radio.RTflag==true)
  {
    Serial.print(F("RT:"));
    Serial.println(Radio.RT);    
    Radio.RTflag=false;
  }
  if(Radio.CTflag==true)
  {
    setTime(Radio.CLOCK[0],Radio.CLOCK[1],0,Radio.DATE[0],Radio.DATE[1],Radio.DATE[2]);
    Radio.CTflag=false;
    Radio.updateTime=true;
    sprintf(Buffer,"\nClock:%02d:%02d\0",hour(),minute());
    Serial.println(Buffer);
    sprintf(Buffer,"Date:%02d.%02d.%04d\n\0",day(),month(),year());
    Serial.println(Buffer);
  }
}

void Radio_loopRDS (void)
{
  char Buffer[20];
  bool RDSavailable = Radio.readRDS();
  if(RDSavailable)
  {
    Radio.decodeRT();
    Radio.decodePS();
    Radio.decodePTY();
    Radio.decodeAF();
    Radio.decodeCT();
    Serial_RDS();
  }
}

void Serial_writeBegin (void)
{
  Serial.begin(9600);
  Serial.println(F("Si4703 DSP Radio"));
  Serial.println(F("COMMAND | FUNCTION"));
  Serial.println(F("+ and - | set Volume up/down"));
  Serial.println(F("u and d | set Frequency up/down"));
  Serial.println(F("q and w | seek auto for a next available Station"));
  Serial.println(F("t       | tune for a alternative frequency by RDS"));
  Serial.println(F("f       | Directly Tuning")); 
  Serial.println(F("i       | Show any Informations"));
  Serial.println(F("1...0   | Your Favourite Stations"));
  Serial.println(F("l       | list all your Favourite Stations"));
  Serial.println(F("x       | set your Favourite Station"));  
  Serial.println(F("?       | List Commands and Functions again\n"));  
}

void Radio_listFavourite (void)
{
  char Buffer[16];
  Serial.println(F("\nYour Favourite Stations:\n"));
  for(int ls = 1; ls <= 10; ls++)
  {
    unsigned lsStation;
    lsStation = EEPROM_getFavourite(ls);
    sprintf(Buffer, "FM%2d: %3d.%d MHz", ls, lsStation/10,lsStation%10);  
    Serial.println(Buffer);
  }
}

void Radio_setFavourite (void)
{
  Serial.println(F("\nSet your Favourite Number between 1...20\n"));
  while(Serial.available()==0)delay(1000);
  while(Serial.available())
  {
    char Station_;
    int Station;
    while(!isdigit(Station_))Station_=Serial.read();
    Station = Station_ - '0';
    if((Station>=0)&&(Station<=10))
    {
      if(Station)
      {
        EEMPROM_setFavourite(Station,channel);
      }
      else
      {
        EEMPROM_setFavourite(10,channel);
      }
      Serial.println(F("Done!"));break;
    }
    else
    {
      Serial.println(F("Invalid Station Number!"));
      Serial.println(F("Please Try again!"));
    }
  }
  Serial_Tune();
}

unsigned EEPROM_getFavourite (int Station)
{
  unsigned Temp = 0;
  Temp += (EEPROM.read(2*Station+10) << 8);
  Temp += (EEPROM.read(2*Station+11)&0xFF);
  if((Temp<875)||(Temp>1080))Temp=875;
  return (Temp);
}

void EEMPROM_setFavourite (int Station, unsigned channel)
{
  EEPROM.write(((2*Station)+10),channel >> 8);
  EEPROM.write(((2*Station)+11),channel&0xFF);
}


void EEPROM_saveChannel (void)
{
  EEPROM.write(ChannelHIEEP,channel >> 8);
  EEPROM.write(ChannelLOEEP,channel&0xFF);
}

void Radio_tuneFavourite (char ch)
{
  if((ch>='0')&&(ch<='9'))
  {
    Radio.softVolume(volume,-1);
    if(ch=='0') channel=EEPROM_getFavourite(10);
    else        channel=EEPROM_getFavourite(ch-'0');
    Radio_tuneStation(0);
    Radio.softVolume(volume,+1);
  }
}

void Radio_setVolume (int Dir)
{
  volume += Dir;
  if(volume==0)volume=0;
  if(volume>30)volume=30;
  Radio.setVolume(volume);
  Serial_Volume();
  delay(100);
  EEPROM.write(VolumeEEP,volume);
}

void Radio_tuneStation (signed Dir)
{
  if(Dir)channel+=Dir;
  if(channel>Radio.channelTOP)channel=Radio.channelBTM;
  if(channel<Radio.channelBTM)channel=Radio.channelTOP;
  Radio.setChannel(channel);
  Radio.clearRDS();
  Serial_Tune();
  EEPROM_saveChannel();
}

void Radio_tuneFrequency (void)
{
  unsigned Number=0;
  Serial.println(F("\nEnter Frequency: \nFor FM: 1080 --> 108.0 MHz\0"));
  
  while(Number==0)                //Wait For Input...
  if(Serial.available())Number = Serial.parseInt();
  
  if((Number>875)&&(Number<1080))
  {

    channel=Number;
    Radio.softVolume(volume,-1);
    Radio.setChannel(channel);
    Radio.softVolume(volume,+1);
    Radio.clearRDS();
    EEPROM.write(channel,ChannelHIEEP >> 8);
    EEPROM.write(channel,ChannelLOEEP&0xFF);
    Serial_Tune();          
  }
  else Serial.println(F("Invalid Frequency!\nPlease Try Again!"));  
}
