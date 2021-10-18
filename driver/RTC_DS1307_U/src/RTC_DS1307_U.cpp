#include "RTC_DS1307_U.h"


RTC_DS1307_U::RTC_DS1307_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
}

void RTC_DS1307_U::getRtcInfo(rtc_info_t *info){
  info->type=DS1307;
  info->numOfInteruptPin=RTC_DS1307_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_DS1307_NUM_OF_ALARM;
  info->numOfTimer=RTC_DS1307_NUM_OF_TIMER;
  info->numOfClockOut=RTC_DS1307_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_DS1307_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_DS1307_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_DS1307_HAVE_MILLISEC;
}

bool RTC_DS1307_U::begin(uint32_t addr) {
  _i2c_addr=addr;
  _i2c_if->begin();
  if ( -1 == initRTC() ) return false;
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
    rtc_date_t data;
    data.year=RTC_DS1307_DEFAULT_YEAR;
    data.month=RTC_DS1307_DEFAULT_MONTH;
    data.mday=RTC_DS1307_DEFAULT_DAY;
    data.wday=RTC_DS1307_DEFAULT_DAY_OF_WEEK;
    data.hour=RTC_DS1307_DEFAULT_HOUR;
    data.minute=RTC_DS1307_DEFAULT_MIN;
    data.second=RTC_DS1307_DEFAULT_SECOND;
    if (!setTime(&data)) return -1;
  }
  startClock();
  return 0;
}

/****************************************************************/
/*Function: Read time and date from RTC */
bool  RTC_DS1307_U::getTime(rtc_date_t*rst) {
  // Reset the register pointer
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->endTransmission();  
  _i2c_if->requestFrom(_i2c_addr, 7);
  // A few of these need masks because certain bits are control bits
  rst->second = bcdToDec(_i2c_if->read() & 0x7f);
  rst->minute = bcdToDec(_i2c_if->read());
  rst->hour   = bcdToDec(_i2c_if->read() & 0x3f);// Need to change this if 12 hour am/pm
  rst->wday   = bcdToDec(_i2c_if->read());
  rst->mday   = bcdToDec(_i2c_if->read());
  rst->month  = bcdToDec(_i2c_if->read());
  rst->year   = bcdToDec(_i2c_if->read())+2000;
  rst->millisecond = 0;
  if (rst->wday==7) { // このRTCは日曜が0ではなく，7であるため．
    rst->wday=0;
  }
  return true;
}

/*******************************************************************/
/*Frunction: Write the time that includes the date to the RTC chip */
bool RTC_DS1307_U::setTime(rtc_date_t *data) {
#ifdef DEBUG
  Serial.println("try to set time data to registers ... done");
#endif
  uint8_t dayVal = data->wday;
  if (dayVal == 0) dayVal=7;
  byte year=(byte) (data->year - 2000);
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->write(decToBcd(data->second));// 0 to bit 7 starts the clock
  _i2c_if->write(decToBcd(data->minute));
  _i2c_if->write(decToBcd(data->hour));  // If you want 12 hour am/pm you need to set bit 6 
  _i2c_if->write(decToBcd(dayVal));
  _i2c_if->write(decToBcd(data->mday));
  _i2c_if->write(decToBcd(data->month));
  _i2c_if->write(decToBcd(year));
  int flag = _i2c_if->endTransmission();
#ifdef DEBUG
  Serial.println("read registers ... done");
#endif
  startClock();
#ifdef DEBUG
  Serial.println("start clock ... done");
#endif
  if (flag == 0) return true;
  return false;
}

/*
 * アラームの設定 : このRTCは機能がないので-1を返す
 */
int  RTC_DS1307_U::setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing) {
  return -1;
}

/*
 * アラームの動作モード設定 : このRTCは機能がないので-1を返す
 */

int  RTC_DS1307_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  return -1;
}

/*
 * アラームの動作変更(start/stop/resumeなど)  : このRTCは機能がないので-1を返す
 */
int  RTC_DS1307_U::controlAlarm(uint8_t num, uint8_t action) {
  return -1;
}

/*
 * タイマの設定 : このRTCは機能がないので-1を返す
 */
int  RTC_DS1307_U::setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi) {
  return -1;
}

/*
 * タイマの動作モード設定 : このRTCは機能がないので-1を返す
 */

int  RTC_DS1307_U::setTimerMode(uint8_t num, timer_mode_t * mode) {
  return -1;
}

/*
 * タイマの動作変更(start/stop/resumeなど) : このRTCは機能がないので-1を返す
 */
int  RTC_DS1307_U::controlTimer(uint8_t num, uint8_t action){
  return -1;
}

/*
 * 割り込み発生時のレジスタの内容をチェック : このRTCは機能がないので-1を返す
 */
uint16_t RTC_DS1307_U::checkInterupt(void){
  return -1;
}

/* レジスタの割り込みフラグのクリア */
bool RTC_DS1307_U::clearInterupt(uint16_t type){
  return -1;
}

/*
 * クロック信号出力設定と出力開始
 */
int  RTC_DS1307_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  _clkoe_pin = pin;
  return setClockOutMode( num,  freq);
}

/*
 * クロック出力の設定
 */
int RTC_DS1307_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_DS1307_NUM_OF_CLOCKOUT) return -1;
  if (freq > 5) return -1;
  byte clockOutReg;
  switch(freq) {
    case 0 : clockOutReg = RTC_DS1307_SQW_1HZ;break;
    case 1 : clockOutReg = RTC_DS1307_SQW_4kHZ;break;
    case 2 : clockOutReg = RTC_DS1307_SQW_8kHZ;break;
    case 3 : clockOutReg = RTC_DS1307_SQW_32kHZ;break;
    case 4 : clockOutReg = RTC_DS1307_OFF;break;
    case 5 : clockOutReg = RTC_DS1307_ON;break;
  }
  if (!writeReg(RTC_DS1307_REG_CLOCK_CONTROL,clockOutReg)) return 1;
  return 0;
}

/*
 * クロック出力の制御 : modeが0の時はclock出力をOFF, 1の時はclock出力をON
 */
int  RTC_DS1307_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_DS1307_NUM_OF_CLOCKOUT) return -1;
  byte clockOutReg;
  switch(mode) {
    case 0: clockOutReg = RTC_DS1307_OFF;break;
    case 1: clockOutReg = RTC_DS1307_ON;break;
    default: return -1;
  }
  if (!writeReg(RTC_DS1307_REG_CLOCK_CONTROL,clockOutReg)) return 1;
  return 0;
}

uint8_t RTC_DS1307_U::decToBcd(uint8_t val)
{
  return ( (val/10*16) + (val%10) );
}

//Convert binary coded decimal to normal decimal numbers
uint8_t RTC_DS1307_U::bcdToDec(uint8_t val)
{
  return ( (val/16*10) + (val%16) );
}

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

/*Function: The clock timing will start */
void RTC_DS1307_U::startClock(void)        // set the ClockHalt bit low to start the rtc
{
  byte second;
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);                      // Register 0x00 holds the oscillator start/stop bit
  _i2c_if->endTransmission();
  _i2c_if->requestFrom(_i2c_addr, 1);
  second = _i2c_if->read() & 0x7f;       // save actual seconds and AND sec with bit 7 (sart/stop bit) = clock started
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->write((uint8_t)second);                    // write seconds back and start the clock
  _i2c_if->endTransmission();
}

/*Function: The clock timing will stop */
void RTC_DS1307_U::stopClock(void)         // set the ClockHalt bit high to stop the rtc
{
  byte second;
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);                      // Register 0x00 holds the oscillator start/stop bit
  _i2c_if->endTransmission();
  _i2c_if->requestFrom(_i2c_addr, 1);
  second = _i2c_if->read() | 0x80;       // save actual seconds and OR sec with bit 7 (sart/stop bit) = clock stopped
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write((uint8_t)RTC_DS1307_REG_SECOND);
  _i2c_if->write((uint8_t)second);                    // write seconds back and stop the clock
  _i2c_if->endTransmission();
}
