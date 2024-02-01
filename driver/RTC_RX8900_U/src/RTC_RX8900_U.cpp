#include "RTC_RX8900_U.h"

/*
 * クラスオブジェクト生成
 */
RTC_RX8900_U::RTC_RX8900_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
}

/*
 * 初期化
 */
bool RTC_RX8900_U::begin(bool init, uint32_t addr){
  _i2c_addr=addr;
  _i2c_if->begin();
  delay(RTC_EPSON_RX8900_INIT_DELAY);
  if (!init) return true;
  clearRegisters();
  uint8_t y, m, w, d, h, mi, s;
  y  = RTC_EPSON_RX8900_DEFAULT_YEAR;
  m  = RTC_EPSON_RX8900_DEFAULT_MONTH;
  d  = RTC_EPSON_RX8900_DEFAULT_DAY;
  w  = RTC_EPSON_RX8900_DEFAULT_DAY_OF_WEEK;
  h  = RTC_EPSON_RX8900_DEFAULT_HOUR;
  mi = RTC_EPSON_RX8900_DEFAULT_MIN;
  s  = RTC_EPSON_RX8900_DEFAULT_SECOND;
  if (setTimeReg(s, mi, h, w, d, m, y)) return true;
  return false;
}

/*
 * レジスタ値読み取り
 */
int RTC_RX8900_U::getRegValue(uint8_t addr){
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  int result = _i2c_if->endTransmission();
  if (result == 0) {
    result = _i2c_if->requestFrom(_i2c_addr,1);
    if (result == 1) {
      return _i2c_if->read();
    }
  }
  return RTC_U_FAILURE;
}

/*
 * レジスタ値書き込み
 */
bool RTC_RX8900_U::setRegValue(uint8_t addr, uint8_t val) {
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  _i2c_if->write(val);
  int flag = _i2c_if->endTransmission();
  if (flag == 0) return true;
  return false;
}

/*
 * 時刻設定
 */
bool RTC_RX8900_U::setTime(date_t* time){
  uint8_t y, m, w, d, h, mi, s;
  if (time->year > 2099) return false;
  if ((time->month > 12)||(time->month<1)) return false;
  if (time->wday > 6) return false;
  //                    1月 2月 3月  4月 5月  6月 7月 8月  9月 10月 11月 12月
  uint8_t mday_list[]={ 31, 29, 31, 30, 31, 30, 31, 31, 30,  31, 30, 31 };
  uint8_t mday_max=mday_list[time->month-1];
  if (time->mday > mday_max) return false;
  if (time->hour > 23) return false;
  if (time->minute > 59) return false;
  if (time->second > 59) return false;
  y  = intToBCD(time->year%100);
  m  = intToBCD(time->month);
  w  = time->wday;
  d  = intToBCD(time->mday);
  h  = intToBCD(time->hour);
  mi = intToBCD(time->minute);
  s  = intToBCD(time->second);
  if (!(setTimeReg(s, mi, h, w, d, m, y))) return false;
  return true;
}

/*
 * 時刻レジスタ書き込み
 */
bool RTC_RX8900_U::setTimeReg(uint8_t sec, uint8_t minute, uint8_t hour, uint8_t wday, uint8_t day, uint8_t mon, uint8_t year){
  uint8_t w;
  switch(wday) {
    case 0 : w=1;break;
    case 1 : w=2;break;
    case 2 : w=4;break;
    case 3 : w=8;break;
    case 4 : w=16;break;
    case 5 : w=32;break;
    case 6 : w=64;break;
  }
  if (!setRegValue(0x00,sec))    return false; // sec
  if (!setRegValue(0x01,minute)) return false; // minute
  if (!setRegValue(0x02,hour))   return false; // hour
  //if (!setRegValue(0x03,wday))   return false; // wday
  if (!setRegValue(0x03,w))   return false; // wday
  if (!setRegValue(0x04,day))    return false; // day
  if (!setRegValue(0x05,mon))    return false; // mon
  if (!setRegValue(0x06,year))   return false; // year
  delay(RTC_EPSON_RX8900_INIT_DELAY);
  return true;
}

/*
 * 時刻読み取り
 */
bool RTC_RX8900_U::getTime(date_t* time){
  uint8_t y, m, w, d, h, mi, s;
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);
  int result = _i2c_if->endTransmission();
  if (result!=0) return false;
  if (7!=_i2c_if->requestFrom(_i2c_addr, 7)) return false;
  s  = _i2c_if->read();
  mi = _i2c_if->read();
  h  = _i2c_if->read();
  w  = _i2c_if->read();
  d  = _i2c_if->read();
  m  = _i2c_if->read();
  y  = _i2c_if->read();

  time->year=bcdToInt(y)+2000;

  time->month=bcdToInt(m);
  if ((time->month < 1) || (time->month > 12)) return false;
  time->mday=bcdToInt(d);
  //                    1月 2月 3月  4月 5月  6月 7月 8月  9月 10月 11月 12月
  uint8_t mday_list[]={ 31, 29, 31, 30, 31, 30, 31, 31, 30,  31, 30, 31 };
  uint8_t mday_max=mday_list[time->month-1];
  if (mday_max < time->mday) return false;
  switch(w) {
    case  1: time->wday=0;break;
    case  2: time->wday=1;break;
    case  4: time->wday=2;break;
    case  8: time->wday=3;break;
    case 16: time->wday=4;break;
    case 32: time->wday=5;break;
    case 64: time->wday=6;break;
    default: return false;
  }
  time->hour=bcdToInt(h);
  if (time->hour>23) return false;
  time->minute=bcdToInt(mi);
  if (time->minute>59) return false;
  time->second=bcdToInt(s);
  if (time->second >59) return false;
  return true;
}

/*
 * RTC情報取得
 */
void RTC_RX8900_U::getRtcInfo(rtc_u_info_t *info) {
  info->type=EPSON_RX8900;
  info->numOfInteruptPin=RTC_EPSON_RX8900_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_EPSON_RX8900_NUM_OF_ALARM;
  info->numOfTimer=RTC_EPSON_RX8900_NUM_OF_TIMER;
  info->numOfClockOut=RTC_EPSON_RX8900_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_EPSON_RX8900_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_EPSON_RX8900_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_EPSON_RX8900_HAVE_MILLISEC;
  info->independentSQW=RTC_EPSON_RX8900_INDEPENDENT_SQW;
  info->detectLowBattery=RTC_EPSON_RX8900_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_EPSON_RX8900_OSCILLATOR_CONTROL;
}

/*
 * 電圧降下フラグ読み取り
 */
int RTC_RX8900_U::checkLowPower(void) {
  int rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b00000011;
  val=val & mask;
  return val;
}

/*
 * 電圧低下フラグクリア
 */
int RTC_RX8900_U::clearPowerFlag(void) {
  int rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b11111100;
  val=val & mask;
  if (!setRegValue(0x0E, val)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * 割り込みフラグ読み取り
 */
int RTC_RX8900_U::checkInterupt(void) {
  int rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b00111000;
  val=val & mask;
  return (val>>3);
}

/*
 * 割り込みフラグクリア
 */
int RTC_RX8900_U::clearInterupt(uint16_t type) {
  // read flag register
  int rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=(uint8_t) type;
  mask=~mask;
  mask=mask<<3;
  mask=0b11000111 | mask;
  val=val & mask;
  bool flag=setRegValue(0x0E, val);
  if (flag) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}

/*
 * num  - 出力は1つなので0限定
 * freq - extension register(0x0D)の2,3bit目の値をそのまま指定
 *          0b00 : 32.768kHz , 0b01 : 1024Hz , 0b10 : 1Hz , 0b11 : 32.768kHz
 * pin - foeピンに接続したArduino側のピン番号
 */
int RTC_RX8900_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (3<freq) return RTC_U_ILLEGAL_PARAM;
  foePin=pin;
  pinMode(foePin,OUTPUT);
  digitalWrite(foePin,HIGH);
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b11110011;
  val = val & mask;
  val = val | (freq<<2);
  if (!setRegValue(0x0D, val)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * num  - 出力は1つなので0限定
 * freq - extension register(0x0D)の2,3bit目の値をそのまま指定
 *          0b00 : 32.768kHz , 0b01 : 1024Hz , 0b10 : 1Hz , 0b11 : 32.768kHz
 */
int RTC_RX8900_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b11110011;
  val = val & mask;
  val = val | freq;
  if (!setRegValue(0x0D, val)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * num  - 出力は1つなので0限定
 * mode - 0 : off , 1 : on
 */
int RTC_RX8900_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (mode > 1) return RTC_U_ILLEGAL_PARAM;
  if (mode==0) {
    pinMode(foePin,OUTPUT);
    digitalWrite(foePin,LOW);
  } else {
    pinMode(foePin,OUTPUT);
    digitalWrite(foePin,HIGH);
  }
  return RTC_U_SUCCESS;
}

/* ================================================================ */
/*
 * タイマ設定前に，各種フラグをOFFにする必要あり
 * 以下のレジスタのbitを順に0に設定(アプリケーションマニュアルに記載)
 *  - TE  : extentionレジスタ 4bit目
 *  - TF  : flagレジスタ 4bit目
 *  - TIE : コントールレジスタ 4bit目
 */
int RTC_RX8900_U::setTTimerFlag(void){
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t val=(uint8_t) rst;
  uint8_t mask=0b11101111;
  val = val & mask;
  if (!setRegValue(0x0D, val)) return RTC_U_FAILURE;
  rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  val=(uint8_t) rst;
  val = val & mask;
  if (!setRegValue(0x0E, val)) return RTC_U_FAILURE;
  rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  val=(uint8_t) rst;
  val = val & mask;
  if (!setRegValue(0x0F, val)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * multi - 下8bit : timerカウンタ0の値に割当て
 *       - 上8bitのうち下bit : timerカウンタ1の値に割当て
 * mode - repeat         : 無視
 *      - useInteruptPin : コントールレジスタ 4bit目(TIE bit)に割当て (0 ピン出力なし, 1 ピン出力あり)
 *      - interval       : extentionレジスタの0,1bit目(TSEL0,1)に割当て
 */
int RTC_RX8900_U::setTTimer(rtc_timer_mode_t * mode, uint16_t multi) {
  uint8_t timer_counter0, timer_counter1;
  timer_counter0 = (uint8_t) multi;
  timer_counter1 = (uint8_t) ((multi >>8) & 0b00001111);
  if (1 < mode->useInteruptPin) return RTC_U_ILLEGAL_PARAM;
  if (3 < mode->interval) return RTC_U_ILLEGAL_PARAM;
  int rst = setTTimerFlag();
  if (RTC_U_SUCCESS!=rst) return rst;
  uint8_t timerCounter0, timerCounter1, extentionReg, controlReg;
  timerCounter0=timer_counter0;
  timerCounter1=timer_counter1;
  rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  extentionReg=((uint8_t) rst) | 0b10000;
  extentionReg=extentionReg&0b11111100;
  extentionReg=extentionReg|mode->interval;
  rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  if (mode->useInteruptPin==0) {
    controlReg=(uint8_t) rst & 0b11101111;
  } else {
    controlReg=((uint8_t) rst) | 0b00010000;
  }
  if (!setRegValue(0x0B, timerCounter0)) return RTC_U_FAILURE;
  if (!setRegValue(0x0C, timerCounter1)) return RTC_U_FAILURE;
  if (!setRegValue(0x0D, extentionReg)) return RTC_U_FAILURE;
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * mode - repeat         : 無視
 *      - useInteruptPin : コントールレジスタ 4bit目(TIE bit)に割当て (0 ピン出力なし, 1 ピン出力あり)
 *      - interval       : extentionレジスタの0,1bit目(TSEL0,1)に割当て
 */
int RTC_RX8900_U::setTTimerMode(rtc_timer_mode_t * mode) {
  if (1 < mode->useInteruptPin) return RTC_U_ILLEGAL_PARAM;
  if (3 < mode->interval) return RTC_U_ILLEGAL_PARAM;
  int rst = setTTimerFlag();
  if (RTC_U_SUCCESS!=rst) return rst;
  uint8_t extentionReg, controlReg;
  rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  extentionReg=(uint8_t) rst;
  extentionReg=extentionReg & 0b11111100;
  extentionReg=extentionReg | mode->interval;
  rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  if (mode->useInteruptPin==0) {
    controlReg=(uint8_t) rst & 0b11101111;
  } else {
    controlReg=((uint8_t) rst) | 0b00010000;
  }
  if (!setRegValue(0x0D, extentionReg)) return RTC_U_FAILURE;
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * action : extensionレジスタの4bit目(TE bit)に代入
 */
int RTC_RX8900_U::controlTTimer(uint8_t action) {
  if (1<action) return RTC_U_ILLEGAL_PARAM;
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t mask, extentionReg;
  if (action==0) {
    mask=0b11101111;
    extentionReg=((uint8_t) rst) & mask;
  } else {
    mask=0b00010000;
    extentionReg=((uint8_t) rst) | mask;
  }
  if (!setRegValue(0x0D, extentionReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}
/* ================================================================ */

/*
 * controlレジスタの5bit目(UIE bit)に0を代入
 * flagレジスタ5bit目(UF bit)に0を代入
 */
int RTC_RX8900_U::setUTimerFlag(void){
  uint8_t flagReg, controlReg;
  int rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  flagReg=(uint8_t) rst;
  uint8_t mask=0b11011111;
  flagReg = flagReg & mask;
  rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  controlReg=(uint8_t) rst;
  mask=0b11011111;
  controlReg = controlReg & mask;
  if (!setRegValue(0x0E, flagReg)) return RTC_U_FAILURE;
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * multiは無視
 * 処理内容はsetUTimerMode()と同じなので，setUTimerMode()に任せる
 */
int RTC_RX8900_U::setUTimer(rtc_timer_mode_t * mode, uint16_t multi) {
  return setUTimerMode(mode);
}

/*
 * mode - repeat         : 無視
 *      - useInteruptPin : 無視
 *      - interval       : extentionレジスタの5bit目(USEL)に代入
 */
int RTC_RX8900_U::setUTimerMode(rtc_timer_mode_t * mode) {
  if (3< mode->interval) return RTC_U_ILLEGAL_PARAM;
  uint8_t extensionReg;
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  extensionReg=(uint8_t) rst;
  if (0==mode->interval) {
    extensionReg = extensionReg & 0b11011111;
  } else {
    extensionReg = extensionReg | 0b00100000;
  }
  if (!setRegValue(0x0D, extensionReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * action : controlレジスタの5bit目(UIE bit)に代入
 */
int RTC_RX8900_U::controlUTimer(uint8_t action) {
  if (1 < action) return RTC_U_ILLEGAL_PARAM;
  uint8_t controlReg;
  int rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  controlReg=(uint8_t) rst;
  if (0==action) {
    controlReg = controlReg & 0b11011111;
  } else {
    controlReg = controlReg | 0b00100000;
  }
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/* ================================================================ */

/*
 * num
 *  - 1 : 定周期タイマ
 *  - 0 : カウントダウンタイマ(割り込み)
 */
int RTC_RX8900_U::setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi) {
  switch(num) {
    case 0 : return setTTimer(mode, multi);
    case 1 : return setUTimer(mode, multi);
  }
  return RTC_U_ILLEGAL_PARAM;
}

/*
 * num
 *  - 1 : 定周期タイマ
 *  - 0 : カウントダウンタイマ(割り込み)
 */
int RTC_RX8900_U::setTimerMode(uint8_t num, rtc_timer_mode_t * mode) {
  switch(num) {
    case 0 : return setTTimerMode(mode);
    case 1 : return setUTimerMode(mode);
  }
  return RTC_U_ILLEGAL_PARAM;
}

/*
 * num
 *  - 1 : 定周期タイマ
 *  - 0 : 時刻更新タイマ(割り込み)
 */
int RTC_RX8900_U::controlTimer(uint8_t num, uint8_t action) {
  switch(num) {
    case 0 : return controlTTimer(action);
    case 1 : return controlUTimer(action);
  }
  return RTC_U_ILLEGAL_PARAM;
}

/* ================================================================ */
/*
 * Controlレジスタ(0x0F)の3bit目(AIEビット)を0に設定
 * Flagレジスタ(0x0E)の3bit目(AFビット)を0に設定
 */
int RTC_RX8900_U::setAlarmFlag(void) {
  uint8_t flagReg, controlReg;
  int rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  controlReg=(uint8_t) rst;
  uint8_t mask=0b11110111;
  controlReg = controlReg & mask;
  rst=getRegValue(0x0E);
  if (rst < 0) return RTC_U_FAILURE;
  flagReg=(uint8_t) rst;
  flagReg = flagReg & mask;
  if (!setRegValue(0x0E, flagReg)) return RTC_U_FAILURE;
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * num : アラームは1つしかないため，0以外無効
 * mode - useInteruptPin
 *      - type  extentionレジスタ(0x0D)の6bit目(WADA)に代入 0x0Aレジスタの解釈が変更 0:曜日 , 1:日付け
 * timing - 各要素の値が0xFFの場合はAE bitを立てるとみなす
 */
int RTC_RX8900_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if ((mode->type!=0) && (mode->type!=1)) return RTC_U_ILLEGAL_PARAM;
  if ( (timing->minute>59) && (timing->minute!=0xFF) ) return RTC_U_ILLEGAL_PARAM;
  if ( (timing->hour>23) && (timing->hour!=0xFF) ) return RTC_U_ILLEGAL_PARAM;
  uint8_t extensionReg, controlReg, minAlarm, hourAlarm, weekdayAlarm;
  if (timing->minute==0xFF) {
    minAlarm=0b10000000;
  } else {
    minAlarm=intToBCD(timing->minute);
  }
  if (timing->hour==0xFF) {
    hourAlarm=0b10000000;
  } else {
    hourAlarm=intToBCD(timing->hour);
  }
  if (mode->type==0) {
    switch(timing->wday) {
      case 0: weekdayAlarm=0b1;break;
      case 1: weekdayAlarm=0b10;break;
      case 2: weekdayAlarm=0b100;break;
      case 3: weekdayAlarm=0b1000;break;
      case 4: weekdayAlarm=0b10000;break;
      case 5: weekdayAlarm=0b100000;break;
      case 6: weekdayAlarm=0b1000000;break;
      case 0xFF: weekdayAlarm=0b10000000;break;
      default: return RTC_U_ILLEGAL_PARAM;
    }
  } else {
    if (timing->mday==255) {
      weekdayAlarm=0b10000000;
    } else if (32>timing->mday) {
      weekdayAlarm=intToBCD(timing->mday);
    } else {
      return RTC_U_ILLEGAL_PARAM;
    }
  }
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  extensionReg=(uint8_t) rst;
  if (mode->type==0) {
    extensionReg = extensionReg & 0b10111111;
  } else {
    extensionReg = extensionReg | 0b01000000;
  }
  rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  controlReg=(uint8_t) rst;
  controlReg=controlReg|0b1000;
  if (!setRegValue(0x08, minAlarm)) return RTC_U_FAILURE;
  if (!setRegValue(0x09, hourAlarm)) return RTC_U_FAILURE;
  if (!setRegValue(0x0A, weekdayAlarm)) return RTC_U_FAILURE;
  if (!setRegValue(0x0D, extensionReg)) return RTC_U_FAILURE;
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * num : アラームは1つしかないため，0以外無効
 * mode - useInteruptPin
 *      - type  extentionレジスタ(0x0D)の6bit目(WADA)に代入 0x0Aレジスタの解釈が変更 0:曜日 , 1:日付け
 */
int RTC_RX8900_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  int rst=getRegValue(0x0D);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t extensionReg=(uint8_t) rst;
  if (mode->type==0) {
    extensionReg = extensionReg & 0b10111111;
  } else {
    extensionReg = extensionReg | 0b01000000;
  }
  if (!setRegValue(0x0D, extensionReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * num : アラームは1つしかないため，0以外無効
 * action : controlレジスタ(0x0F)の3bit目(AIE)に代入
 */
int RTC_RX8900_U::controlAlarm(uint8_t num, uint8_t action) {
  if (num >= RTC_EPSON_RX8900_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  int rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t controlReg=(uint8_t) rst;
  if (action==0) {
    controlReg=controlReg & 0b11110111;
  } else if (action==1) {
    controlReg=controlReg | 0b00001000;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  if (!setRegValue(0x0F, controlReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}


/* ================================================================ */
void RTC_RX8900_U::clearRegisters(void) {
  for (int i=0; i<16;i++){
    setRegValue(i, 0);
  }
}

/* ================================================================ */
float RTC_RX8900_U::getTemperature(uint8_t kind) {
  int rst=getRegValue(0x17);
  if (rst < 0) return RTC_U_TEMPERATURE_FAILURE;
  uint8_t val=(uint8_t) rst;
  float celcius, kelvin, fahrenheit;
  celcius = (val*2 - 187.19)/3.218;
  kelvin = celcius + 273.15;
  fahrenheit = celcius*1.8 + 32.0;
  switch (kind) {
    case RTC_U_TEMPERATURE_KELVIN:return kelvin;
    case RTC_U_TEMPERATURE_CELCIUS:return celcius;
    case RTC_U_TEMPERATURE_FAHRENHEIT: return fahrenheit;
  }
  return RTC_U_TEMPERATURE_FAILURE;
}

int   RTC_RX8900_U::setTemperatureFunction(uint8_t mode) {
  if (mode > 3) return RTC_U_ILLEGAL_PARAM;
  mode = mode << 6;
  int rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t reg=((uint8_t) rst)&0b00111000;
  reg=reg | mode;
  if (!setRegValue(0x0F, reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}
/* ================================================================ */
/*
 * コントロールレジスタ(0x0F)の最下位bitに1を書き込む
 */
int RTC_RX8900_U::controlClock(void) {
  int rst=getRegValue(0x0F);
  if (rst < 0) return RTC_U_FAILURE;
  uint8_t reg=((uint8_t) rst)&0b11111110;
  if (!setRegValue(0x0F, reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/* ======================= SRAM アクセス ============================== */
int RTC_RX8900_U::getSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_EPSON_RX8900_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  addr = addr + RTC_EPSON_RX8900_SRAM_BASE_ADDR;
  int regValue = getRegValue(addr);
  if (regValue < 0) return regValue;
  (*data) = (uint8_t) regValue;
  return RTC_U_SUCCESS;
}

int RTC_RX8900_U::setSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_EPSON_RX8900_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  if (!setRegValue(RTC_EPSON_RX8900_SRAM_BASE_ADDR, *data)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/* ================================================================ */
#ifdef DEBUG
void RTC_RX8900_U::dumpReg(void) {
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);
  int result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return ;
  if (16!=_i2c_if->requestFrom(_i2c_addr, 16)) return ;
  uint8_t reg[18];
  for (int i=0; i<16;i++){
    reg[i]=_i2c_if->read();
  }
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x17);
  result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return ;
  if (2!=_i2c_if->requestFrom(_i2c_addr, 2)) return;
  reg[17]=_i2c_if->read();
  reg[18]=_i2c_if->read();
  String regName[]={
    "sec",
    "min",
    "hour",
    "week",
    "day",
    "month",
    "year",
    "ram",
    "min alarm",
    "hour alarm",
    "week/day alarm",
    "timer counter 0",
    "timer counter 1",
    "extension",
    "flag",
    "control",
    "temperature",
    "backup"
  };
  for (uint8_t i=0;i< 16; i++) {
    Serial.print(" Reg No.0x");Serial.print(i,HEX);Serial.print(" : ");Serial.print(regName[i]);Serial.print(" = ");Serial.println(reg[i],BIN);
  }
  Serial.print(" Reg No.0x");Serial.print(0x17,HEX);Serial.print(" : ");Serial.print(regName[16]);Serial.print(" = ");Serial.println(reg[16],BIN);
  Serial.print(" Reg No.0x");Serial.print(0x18,HEX);Serial.print(" : ");Serial.print(regName[17]);Serial.print(" = ");Serial.println(reg[17],BIN);
}

bool RTC_RX8900_U::backupRegValues(void){
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);
  int result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return false;
  if (16!=_i2c_if->requestFrom(_i2c_addr, 16)) return false;
  uint8_t reg[18];
  for (int i=0; i<16;i++){
    regValue[i]=_i2c_if->read();
  }
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x17);
  result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return false;
  if (2!=_i2c_if->requestFrom(_i2c_addr, 2)) return false;
  regValue[16]=_i2c_if->read();
  regValue[17]=_i2c_if->read();
  return true;
}

bool RTC_RX8900_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if ((num > 0x0F) && (num != 17) && (num != 18)) return false;
  //
  uint8_t reg;
  if (num==17) {
    reg=regValue[16] & mask;
  } else if (num==18) {
    reg=regValue[17] & mask;
  } else {
    reg=regValue[num] & mask;
  }
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
