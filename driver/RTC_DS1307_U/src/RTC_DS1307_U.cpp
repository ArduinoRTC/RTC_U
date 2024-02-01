#include "RTC_DS1307_U.h"


RTC_DS1307_U::RTC_DS1307_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
}

void RTC_DS1307_U::getRtcInfo(rtc_u_info_t *info){
  info->type=DS1307;
  info->numOfInteruptPin=RTC_DS1307_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_DS1307_NUM_OF_ALARM;
  info->numOfTimer=RTC_DS1307_NUM_OF_TIMER;
  info->numOfClockOut=RTC_DS1307_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_DS1307_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_DS1307_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_DS1307_HAVE_MILLISEC;
  info->detectLowBattery=RTC_DS1307_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_DS1307_OSCILLATOR_CONTROL;
}

bool RTC_DS1307_U::begin(bool init, uint32_t addr) {
  _i2c_addr=addr;
  _i2c_if->begin();
  if (!init) return true;
  if ( RTC_U_FAILURE == initRTC() ) return false;
  return true;
}

int RTC_DS1307_U::initRTC(void) {
  byte data;
  // Reset the register pointer
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->endTransmission();  
  _i2c_if->requestFrom(_i2c_addr, 7);
  // A few of these need masks because certain bits are control bits
  data = _i2c_if->read();
  data = _i2c_if->read();
  data = _i2c_if->read();
  data = _i2c_if->read();
  data = _i2c_if->read();
  data = _i2c_if->read();
  data = _i2c_if->read();
  if (0 == data) { // 時刻が消えている
    date_t data;
    data.year=RTC_DS1307_DEFAULT_YEAR;
    data.month=RTC_DS1307_DEFAULT_MONTH;
    data.mday=RTC_DS1307_DEFAULT_DAY;
    data.wday=RTC_DS1307_DEFAULT_DAY_OF_WEEK;
    data.hour=RTC_DS1307_DEFAULT_HOUR;
    data.minute=RTC_DS1307_DEFAULT_MIN;
    data.second=RTC_DS1307_DEFAULT_SECOND;
    if (!setTime(&data)) return RTC_U_FAILURE;
  }
  return RTC_U_SUCCESS;
}

/****************************************************************/
/*Function: Read time and date from RTC */
bool  RTC_DS1307_U::getTime(date_t*rst) {
  // Reset the register pointer
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->endTransmission();  
  _i2c_if->requestFrom(_i2c_addr, 7);
  // A few of these need masks because certain bits are control bits
  rst->second = bcdToInt(_i2c_if->read() & 0x7f);
  rst->minute = bcdToInt(_i2c_if->read());
  rst->hour   = bcdToInt(_i2c_if->read() & 0x3f);// Need to change this if 12 hour am/pm
  rst->wday   = bcdToInt(_i2c_if->read());
  rst->mday   = bcdToInt(_i2c_if->read());
  rst->month  = bcdToInt(_i2c_if->read());
  rst->year   = bcdToInt(_i2c_if->read())+2000;
  rst->millisecond = 0;
  if (rst->wday==7) { // このRTCは日曜が0ではなく，7であるため．
    rst->wday=0;
  }
  return true;
}

/*******************************************************************/
/*Frunction: Write the time that includes the date to the RTC chip */
bool RTC_DS1307_U::setTime(date_t *data) {
  uint8_t dayVal = data->wday;
  if (dayVal == 0) dayVal=7;
  byte year=(byte) (data->year - 2000);
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->write(intToBCD(data->second));// 0 to bit 7 starts the clock
  _i2c_if->write(intToBCD(data->minute));
  _i2c_if->write(intToBCD(data->hour));  // If you want 12 hour am/pm you need to set bit 6 
  _i2c_if->write(intToBCD(dayVal));
  _i2c_if->write(intToBCD(data->mday));
  _i2c_if->write(intToBCD(data->month));
  _i2c_if->write(intToBCD(year));
  int flag = _i2c_if->endTransmission();
  if (flag == 0) return true;
  return false;
}

/*
 * クロック信号出力設定と出力開始
 * pinは無視
 */
int  RTC_DS1307_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  return setClockOutMode( num,  freq);
}

/*
 * クロック出力の設定
 */
int RTC_DS1307_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_DS1307_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 5) return RTC_U_ILLEGAL_PARAM;
  byte clockOutReg;
  switch(freq) {
    case 0 : clockOutReg = RTC_DS1307_SQW_1HZ;break;
    case 1 : clockOutReg = RTC_DS1307_SQW_4kHZ;break;
    case 2 : clockOutReg = RTC_DS1307_SQW_8kHZ;break;
    case 3 : clockOutReg = RTC_DS1307_SQW_32kHZ;break;
    case 4 : clockOutReg = RTC_DS1307_OFF;break;
    case 5 : clockOutReg = RTC_DS1307_ON;break;
  }
  if (!writeReg(RTC_DS1307_REG_CLOCK_CONTROL,clockOutReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の制御 : modeが0の時はclock出力をOFF, 1の時はclock出力をON
 */
int  RTC_DS1307_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_DS1307_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  byte clockOutReg;
  switch(mode) {
    case 0: clockOutReg = RTC_DS1307_OFF;break;
    case 1: clockOutReg = RTC_DS1307_ON;break;
    default: return RTC_U_ILLEGAL_PARAM;
  }
  if (!writeReg(RTC_DS1307_REG_CLOCK_CONTROL,clockOutReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}
/* ================================================================ */
int RTC_DS1307_U::clockHaltStatus(void) {
  byte reg;
  if (!readReg(0, &reg)) return RTC_U_FAILURE;
  byte mask=0b10000000;
  reg=reg & mask;
  if (reg>0) return 1;
  return 0;
}

int RTC_DS1307_U::controlClockHalt(uint8_t mode) {
  byte reg;
  if (!readReg(0, &reg)) return RTC_U_FAILURE;
  switch(mode) {
    case 1: reg = reg & 0b01111111;break;
    case 0: reg = reg | 0b10000000;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  if (!writeReg(0, reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}
/* ================================================================ */
int RTC_DS1307_U::getSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_DS1307_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  addr = addr + RTC_DS1307_SRAM_BASE_ADDR;
  for (int i=0 ; i< len ; i++) {
    if (!readReg(addr+i, data+i)) return RTC_U_FAILURE;
  }
  return RTC_U_SUCCESS;
}

int RTC_DS1307_U::setSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_DS1307_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  addr = addr + RTC_DS1307_SRAM_BASE_ADDR;
  for (int i=0 ; i< len ; i++) {
    if (!writeReg(addr+i, *(data+i))) return RTC_U_FAILURE;
  }
  return RTC_U_SUCCESS;
}
/* ================================================================ */
/*
 * レジスタの読みとり
 */
bool RTC_DS1307_U::readReg(byte addr, byte *reg){
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr) ;
  int ans = _i2c_if->endTransmission() ;     // データの送信と終了処理
  if (ans == 0) {
    ans = _i2c_if->requestFrom(_i2c_addr,1) ; // ＲＴＣにデータ送信要求をだす
    if (ans == 1) {
      (*reg) = _i2c_if->read()  ; // Regを受信
      return true;
    }
  }
  return false;
}

/*
 * レジスタへの書き込み
 */
bool RTC_DS1307_U::writeReg(byte addr, byte val) {
  _i2c_if->beginTransmission(_i2c_addr) ;  // 通信の開始処理
  _i2c_if->write((uint8_t)addr) ;                  // レジスターアドレスを指定する
  _i2c_if->write(val);                  // カウントダウンタイマー値の設定
  int ans=_i2c_if->endTransmission() ;      // データの送信と終了処理
  if (ans==0) return true;
  return false;
}
/* ================================================================ */

#ifdef DEBUG
void RTC_DS1307_U::dumpReg(void) {
  String regName[]={
    "sec            ",
    "min            ",
    "hour           ",
    "week           ",
    "day            ",
    "month          ",
    "year           ",
    "control        "
  };
  Serial.println("register name   | addr | value");
  Serial.println("----------------+------+------------------");
  for (int i=0; i< RTC_DS1307_REG_NUM; i++) {
    Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
  }
}

bool RTC_DS1307_U::backupRegValues(void) {
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00) ;
  int ans = _i2c_if->endTransmission() ;     // データの送信と終了処理
  if (ans == 0) {
    ans = _i2c_if->requestFrom(_i2c_addr,RTC_DS1307_REG_NUM) ; // ＲＴＣにデータ送信要求をだす
    if (ans == RTC_DS1307_REG_NUM) {
      for (int i=0 ; i< RTC_DS1307_REG_NUM ; i++) {
        regValue[i]=_i2c_if->read()  ; // Regを受信
      }
      return true;
    }
  }
  return false;
}

bool RTC_DS1307_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num >= RTC_DS1307_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
