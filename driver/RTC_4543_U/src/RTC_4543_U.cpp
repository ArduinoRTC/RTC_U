#include "RTC_4543_U.h"

RTC_4543_U::RTC_4543_U(uint8_t _dataPin, uint8_t _clkPin, uint8_t _wrPin, uint8_t _cePin, uint8_t _fsel, int32_t _rtcID) {
  dataPin = _dataPin;
  clkPin  = _clkPin;
  wrPin   = _wrPin;
  cePin   = _cePin;
  fsel    = _fsel;
  rtcID   = _rtcID;
}

void  RTC_4543_U::getRtcInfo(rtc_info_t *info) {
  info->type=EPSON_4543SA;
  info->numOfInteruptPin=RTC_EPSON_4543_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_EPSON_4543_NUM_OF_ALARM;
  info->numOfTimer=RTC_EPSON_4543_NUM_OF_TIMER;
  info->numOfClockOut=RTC_EPSON_4543_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_EPSON_4543_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_EPSON_4543_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_EPSON_4543_HAVE_MILLISEC;
  info->independentSQW=RTC_EPSON_4543_INDEPENDENT_SQW;
  info->detectLowBattery=RTC_EPSON_4543_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_EPSON_4543_OSCILLATOR_CONTROL;
}

void RTC_4543_U::beginTransmission(bool mode_write) {
  digitalWrite(cePin, LOW);
  digitalWrite(clkPin, LOW);
  RTC_4543_DELAY;

  if(mode_write){
    pinMode(dataPin, OUTPUT);
    digitalWrite(wrPin, HIGH);    
  } else {
    pinMode(dataPin, INPUT);
    digitalWrite(wrPin, LOW);
  }

  RTC_4543_DELAY;
  digitalWrite(cePin, HIGH);
  RTC_4543_DELAY;
}

void RTC_4543_U::endTransmission(void) {
  digitalWrite(clkPin, LOW);
  digitalWrite(cePin, LOW);
}

uint8_t RTC_4543_U::readData(bool half) {
  uint8_t result=0;
  uint8_t size;
  if (half) size=4;
  else size=8;
  for(int i = 0; i < size; i++){
    digitalWrite(clkPin,HIGH);
    RTC_4543_DELAY;
    result = result | ((digitalRead(dataPin)) << i);
    digitalWrite(clkPin,LOW);
    RTC_4543_DELAY;
  }
  return result;
}

#define bitVal(data, pos) ((data >> pos) & 0b1)? HIGH: LOW

void RTC_4543_U::writeData(uint8_t data, bool half){
  uint8_t size;
  if (half) size=4;
  else size=8;
  for(int i = 0; i < size; i++){
    digitalWrite(dataPin, bitVal(data,i) );
    RTC_4543_DELAY;
    digitalWrite(clkPin, HIGH);
    RTC_4543_DELAY;
    digitalWrite(clkPin, LOW);
    RTC_4543_DELAY;
  }
}

void RTC_4543_U::getAllData(void) {
  uint8_t val[7];
  beginTransmission(false); // read start
  for (int i=0;i<7;i++) {
    val[i]=0;
    if (i==3) {
      val[i]=readData(true);
    } else {
      val[i]=readData(false);
    }
  }
  endTransmission(); // end
  if (0 == (val[0] & 0b10000000)) {
    fdt=false;
  } else {
    fdt = true;
  }
  sec    = bcdToInt(val[0]&0b01111111);
  minute = bcdToInt(val[1]&0b01111111);
  hour   = bcdToInt(val[2]&0b00111111);
  wday   = bcdToInt(val[3]&0b00000111)-1;
  mday   = bcdToInt(val[4]&0b00111111);
  month  = bcdToInt(val[5]&0b00011111);
  year   = bcdToInt(val[6]);
}

void RTC_4543_U::setAllData(void) {
  uint8_t b_sec, b_min, b_hour, b_wday, b_mday, b_month, b_year;
  b_sec   = intToBCD(sec);
  b_min   = intToBCD(minute);
  b_hour  = intToBCD(hour);
  b_wday  = intToBCD(wday+1);
  b_mday  = intToBCD(mday);
  b_month = intToBCD(month);
  b_year  = intToBCD(year);
  beginTransmission(true); // write start
  writeData(b_sec,false);
  writeData(b_min,false);
  writeData(b_hour,false);
  writeData(b_wday,true);
  writeData(b_mday,false);
  writeData(b_month,false);
  writeData(b_year,false);
  endTransmission(); // end
}

bool  RTC_4543_U::begin(bool init , uint32_t addr) {
  pinMode(clkPin,OUTPUT);
  digitalWrite(clkPin,LOW);
  pinMode(wrPin,OUTPUT);
  digitalWrite(wrPin,LOW);
  pinMode(cePin,OUTPUT);
  digitalWrite(cePin,LOW);
  if (!init) return true;
  date_t date;
  date.year=2000+RTC_EPSON_4543_DEFAULT_YEAR;
  date.month=RTC_EPSON_4543_DEFAULT_MONTH;
  date.mday=RTC_EPSON_4543_DEFAULT_DAY;
  date.wday=RTC_EPSON_4543_DEFAULT_DAY_OF_WEEK;
  date.hour=RTC_EPSON_4543_DEFAULT_HOUR;
  date.minute=RTC_EPSON_4543_DEFAULT_MIN;
  date.second=RTC_EPSON_4543_DEFAULT_SECOND;
  return setTime(&date);
}

bool  RTC_4543_U::setTime(date_t* date) {
  if (date->year < 2000) return false;
  year = date->year-2000;
  month = date->month;
  mday = date->mday;
  wday = date->wday;
  hour = date->hour;
  minute = date->minute;
  sec = date->second;
  setAllData();
  return true;
}

bool  RTC_4543_U::getTime(date_t* date) {
  getAllData();
  date->year   = year+2000;
  date->month  = month;
  date->mday   = mday;
  date->wday   = wday;
  date->hour   = hour;
  date->minute = minute;
  date->second = sec;
  return true;
}

int   RTC_4543_U::checkLowPower(void) {
  getAllData();
  if (fdt) return 1;
  return 0;
}

int   RTC_4543_U::clearPowerFlag(void) {
  getAllData();
  fdt=false;
  return RTC_U_SUCCESS;
}

int RTC_4543_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_EPSON_4543_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  foe  = pin;
  pinMode(fsel,OUTPUT);
  if (freq == 0) digitalWrite(fsel,LOW); // 32.768kHz
  else  digitalWrite(fsel,HIGH);         // 1Hz
  pinMode(foe,OUTPUT);
  digitalWrite(foe,HIGH);
  return RTC_U_SUCCESS;
}

int RTC_4543_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_EPSON_4543_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  pinMode(fsel,OUTPUT);
  if (freq == 0) digitalWrite(fsel,LOW); // 32.768kHz
  else  digitalWrite(fsel,HIGH);         // 1Hz
  return RTC_U_SUCCESS;
}

int RTC_4543_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_EPSON_4543_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  pinMode(foe,OUTPUT);
  if (mode == 0) digitalWrite(foe,LOW); // stop
  else  digitalWrite(foe,HIGH);         // start
  return RTC_U_SUCCESS;
}
