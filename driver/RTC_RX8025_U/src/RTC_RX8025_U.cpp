#include "RTC_RX8025_U.h"

RTC_RX8025_U::RTC_RX8025_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
  timerSetting=0xFF;
}

void RTC_RX8025_U::getRtcInfo(rtc_u_info_t *info){
  info->type=EPSON_RX8025;
  info->numOfInteruptPin=RTC_EPSON_RX8025_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_EPSON_RX8025_NUM_OF_ALARM;
  info->numOfTimer=RTC_EPSON_RX8025_NUM_OF_TIMER;
  info->numOfClockOut=RTC_EPSON_RX8025_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_EPSON_RX8025_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_EPSON_RX8025_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_EPSON_RX8025_HAVE_MILLISEC;
  info->independentSQW=RTC_EPSON_RX8025_INDEPENDENT_SQW;
  info->detectLowBattery=RTC_EPSON_RX8025_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_EPSON_RX8025_OSCILLATOR_CONTROL;
}

bool RTC_RX8025_U::setTimeReg(uint8_t sec, uint8_t minute, uint8_t hour, uint8_t wday, uint8_t day, uint8_t mon, uint8_t year){
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);       // select reg no.0
  _i2c_if->write(sec);        // second
  _i2c_if->write(minute);     // min
  _i2c_if->write(hour);       // hour
  _i2c_if->write(wday);       // day of week
  _i2c_if->write(day);        // day
  _i2c_if->write(mon);        // month
  _i2c_if->write(year);       // year
  uint8_t rst = _i2c_if->endTransmission();
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  if (0!=rst) return false;
  return true;
}

bool RTC_RX8025_U::begin(bool force, uint32_t addr){
  _i2c_addr=addr;
  _i2c_if->begin();
  delay(RTC_EPSON_RX8025_INIT_DELAY);

  int result = getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (result < 0) return false;
  uint8_t control1 = (uint8_t) result;
  control1 = control1 & 0b11011111;
  control1 = control1 | 0b00100000;
  result = setRegValue(0xE0, control1);
  if (result < 0) return false;
  if (!force) return true;
  // Set control registers.
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0xE0);
  _i2c_if->write(RTC_EPSON_RX8025_CONTROL1_REG_CFG); // Control1 reg
  _i2c_if->write(0x00);                             // Control2 reg (all off)
  _i2c_if->endTransmission();
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);

  //Serial.println("begin pos 1.");

  // Set time
  if (!setTimeReg(intToBCD(RTC_EPSON_RX8025_DEFAULT_SECOND), intToBCD(RTC_EPSON_RX8025_DEFAULT_MIN),
                    intToBCD(RTC_EPSON_RX8025_DEFAULT_HOUR), RTC_EPSON_RX8025_DEFAULT_DAY_OF_WEEK, 
                    intToBCD(RTC_EPSON_RX8025_DEFAULT_DAY), intToBCD(RTC_EPSON_RX8025_DEFAULT_MONTH), 
                    intToBCD(RTC_EPSON_RX8025_DEFAULT_YEAR))) return false;
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);

  // unset all alarm
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x8E);  // select reg no.8
  _i2c_if->write(0x00);  //  8:Alarm_W ; Minute
  _i2c_if->write(0x00);  //  9:Alarm_W ; Hour
  _i2c_if->write(0x00);  //  A:Alarm_W ; Weekday
  _i2c_if->write(0x00);  //  B:Alarm_D ; Minute
  _i2c_if->write(0x00);  //  C:Alarm_D ; Hour
  _i2c_if->endTransmission();
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);

  return true;
}

bool RTC_RX8025_U::checkYearOverflow(uint8_t m){
  if (m >=127) return true;
  return false;
}

bool RTC_RX8025_U::getTime(date_t* time){
  uint8_t y, m, w, d, h, mi, s;
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);
  int result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return false;

  if (8!=_i2c_if->requestFrom(_i2c_addr, 8)) return false;
  _i2c_if->read();
  s  = _i2c_if->read();
  mi = _i2c_if->read();
  h  = _i2c_if->read();
  w  = _i2c_if->read();
  d  = _i2c_if->read();
  m  = _i2c_if->read();
  y  = _i2c_if->read();
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  time->year=bcdToInt(y);
  bool flag=checkYearOverflow(m);
  m = 0b01111111 & m;
  if (time->year > 99) return false;
  if (flag) { // 2099年対応(2199年まで対応可能)
    time->year+=2100;
  } else {
    time->year+=2000;
  }
  time->month=bcdToInt(m);
  if ((time->month < 1) || (time->month > 12)) return false;
  time->mday=bcdToInt(d);
  //                    1月 2月 3月  4月 5月  6月 7月 8月  9月 10月 11月 12月
  uint8_t mday_list[]={ 31, 29, 31, 30, 31, 30, 31, 31, 30,  31, 30, 31 };
  uint8_t mday_max=mday_list[time->month-1];
  if (mday_max < time->mday) return false;
  if (w>6) return false;
  time->wday=w;
  time->hour=bcdToInt(h);
  if (time->hour>23) return false;
  time->minute=bcdToInt(mi);
  if (time->minute>59) return false;
  time->second=bcdToInt(s);
  if (time->second >59) return false;
  return true;
}

bool RTC_RX8025_U::setTime(date_t* time){
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

int RTC_RX8025_U::getRegValue(uint8_t addr){
  if (addr > 0x0F) return RTC_U_FAILURE;
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  addr=(addr<<4) & 0b11110000;
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  int result = _i2c_if->endTransmission() ;     // データの送信と終了処理
  if (result == 0) {
    result = _i2c_if->requestFrom(_i2c_addr,1) ; // ＲＴＣにデータ送信要求をだす
    if (result == 1) {
      return _i2c_if->read()  ; // Regを受信
    }
  }
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  return RTC_U_FAILURE;
}

bool RTC_RX8025_U::setRegValue(uint8_t addr, uint8_t val) {
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  _i2c_if->write(val);
  int flag = _i2c_if->endTransmission();
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  if (flag == 0) return true;
  return false;
}
/* ================================================================ */
/* =====  fout ピンに32.768kHzの信号を出す機能に関する関数 ===== */
/*
 * foeピンを登録して，output設定の上，HIGHに設定した上で，numとfreqは無視
 * /CLEN2ビットは0に設定 (0x0e ctrl1レジスタの下から5bit目を0にする 0b1101111とor)
 */
int RTC_RX8025_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin){
  foePin=pin;
  pinMode(foePin,OUTPUT);
  digitalWrite(foePin,HIGH);
  // ctrl1レジスタ(0x0E)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl1=(uint8_t) regValue;
  // ctrl1レジスタ(0x0E)設定  /CLEN2ビットを0にする (0x0e ctrl1レジスタの下から5bit目 0b10000)
  ctrl1=ctrl1 & 0b11101111;
  if (setRegValue(0xE0, ctrl1)) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}

/*
 * 登録されているfoeピンをmode(0:止める，1:動かす)に従ってHIGH, LOWに制御
 * numは無視(周波数設定はないため)
 */
int RTC_RX8025_U::controlClockOut(uint8_t num, uint8_t mode){
  if (num >= RTC_EPSON_RX8025_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;
  switch(mode) {
    case 0: digitalWrite(foePin,LOW);return RTC_U_SUCCESS;
    case 1: digitalWrite(foePin,HIGH);return RTC_U_SUCCESS;
  }
  return RTC_U_FAILURE;
}



/* ================================================================ */
/*
 * num    - 0:Alarm_W , 1:Alarm_D
 * アラームには動作モードがない．
 * timing
 *    uint16_t  year;
 *    uint8_t   month;
 *    uint8_t   mday;
 *    uint8_t   wday;
 *    uint8_t   hour;
 *    uint8_t   minute;
 *    uint8_t   second;
 *    int16_t   millisecond;
 * Alarm_W : 0x08(min bcd), 0x09(hour bcd), 0x0A(wday), 0x0E(WALE 7bit目), 0x0F(WAFG 下から2bit目)
 * Alarm_D : 0x0B(min bcd), 0x0C(hour bcd), 0x0E(DALE 6bit目), 0x0F(DAFG 最下位bit)
 */
int RTC_RX8025_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing){
  if (num >= RTC_EPSON_RX8025_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  // numで指定されたalarmを止める
  int flag=controlAlarm( num,  0);
  if (flag!=RTC_U_SUCCESS) return flag;
  // レジスタ値を計算
  uint8_t min, hour, wday;
  min=intToBCD(timing->minute);
  hour=intToBCD(timing->hour);
  //wday=timing->wday;
  switch(timing->wday) {
    case 0: wday=0b00000001;break;
    case 1: wday=0b00000010;break;
    case 2: wday=0b00000100;break;
    case 3: wday=0b00001000;break;
    case 4: wday=0b00010000;break;
    case 5: wday=0b00100000;break;
    case 6: wday=0b01000000;break;
    default:wday=(timing->wday)&0b01111111;
  }
  
  // レジスタ値を設定
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  if (num==0) { // alarm_W設定, レジスタ 0x08, 0x09, 0x0Aに書き込み
    _i2c_if->beginTransmission(_i2c_addr);
    _i2c_if->write(0x80);       // select reg no.0
    _i2c_if->write(min);        // second
    _i2c_if->write(hour);       // hour
    _i2c_if->write(wday);       // day of week
    uint8_t rst = _i2c_if->endTransmission();
    if (0!=rst) return RTC_U_FAILURE;
  } else { // alarm_D設定, レジスタ 0x0B, 0x0Cに書き込み
    _i2c_if->beginTransmission(_i2c_addr);
    _i2c_if->write(0xB0);       // select reg no.0
    _i2c_if->write(min);        // second
    _i2c_if->write(hour);       // hour
    uint8_t rst = _i2c_if->endTransmission();
    if (0!=rst) return RTC_U_FAILURE;
  }
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  // numで指定されたalarmを有効にする
  return controlAlarm( num,  1);
}

/*
 * num    - 0:Alarm_W , 1:Alarm_D
 * action - 0:off, 1:on
 * Alarm_W : 0x08(min bcd), 0x09(hour bcd), 0x0A(wday), 0x0E(WALE 7bit目), 0x0F(WAFG 下から2bit目)
 * Alarm_D : 0x0B(min bcd), 0x0C(hour bcd), 0x0E(DALE 6bit目), 0x0F(DAFG 最下位bit)
 */
int RTC_RX8025_U::controlAlarm(uint8_t num, uint8_t action){
  if (num >= RTC_EPSON_RX8025_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  delayMicroseconds(RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL);
  // ctrl1レジスタ(0x0E)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl1=(uint8_t) regValue;
  // ctrl2レジスタ(0x0F)の値を取得
  regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl2=(uint8_t) regValue;
  if (0==action) { // アラームOFF, ctrl1の(W|D)ALEビットを0に変更
    uint8_t mask;
    if (num==0) {
      mask=0b01111111;
    } else {
      mask=0b10111111;
    };
    ctrl1=ctrl1&mask;
    // ctrl1レジスタに値を書き込み
    if (!setRegValue(0xE0, ctrl1)) return RTC_U_FAILURE;
  } else if (1==action) {  // アラームON
    uint8_t mask;
    // ctrl2の(W|D)AFGビットを0に変更
    if (num==0) {
      mask=0b11111101;
    } else {
      mask=0b11111110;
    }
    ctrl2=ctrl2&mask;
    // ctrl2レジスタに値を書き込み
    if (!setRegValue(0xF0, ctrl2)) return RTC_U_FAILURE;
    // ctrl1の(W|D)ALEビットを1に変更
    if (num==0) {
      mask=0b10000000;
    } else {
      mask=0b01000000;
    }
    ctrl1=ctrl1|mask;
    // ctrl1レジスタに値を書き込み
    if (!setRegValue(0xE0, ctrl1)) return RTC_U_FAILURE;
  } else { // エラー
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/* ================================================================ */
/*
 * RX8025はタイマは1つしかないため，numは実質無意味．
 * modeは3つのメンバ変数の意味は以下の通り
 *    int8_t  repeat;          ← 意味無し
 *    int8_t  useInteruptPin;  ← 意味無し
 *    int8_t  interval;        ← ctrl1レジスタ(0x0e)の下3bitを変更
 * RX8025はmodeで設定した時間の整数倍を設定する機能はないため，multiは無意味
 */
int RTC_RX8025_U::setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi){
  if (num >= RTC_EPSON_RX8025_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;
  if (7< mode->interval) return RTC_U_ILLEGAL_PARAM;
  // ctrl1レジスタを読み出し
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl1  =(uint8_t) regValue;
  // ctrl1レジスタ値の下3bitを書き換え
  ctrl1 = ctrl1 & 0b11110000;
  ctrl1 = ctrl1 | mode->interval;
  // ctrl1レジスタに値を書き込み
  if (!setRegValue(0xE0, ctrl1)) return RTC_U_FAILURE;
  // バックアップがないことを記録
  timerSetting=0xFF;
  return RTC_U_SUCCESS;
}

/*
 * RX8025はタイマは1つしかないため，numは実質無意味．
 * modeは3つのメンバ変数の意味は以下の通り
 *    int8_t  repeat;          ← 意味無し
 *    int8_t  useInteruptPin;  ← 意味無し
 *    int8_t  interval;        ← ctrl1レジスタ(0x0e)の下3bitを変更
 * タイマがOFFの場合は，バックアップされているモードの値のみを書き換え
 * タイマがONの場合は，レジスタ値を書き換え
 */
int RTC_RX8025_U::setTimerMode(uint8_t num, rtc_timer_mode_t * mode){
  if (num >= RTC_EPSON_RX8025_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;
  if (7< mode->interval) return RTC_U_ILLEGAL_PARAM;
  if (0xFF!=timerSetting) { // タイマOFFの場合
    timerSetting=mode->interval;
    return RTC_U_SUCCESS;
  }
  // タイマONの場合(もしくは未設定の場合)
  // ctrl1レジスタ(0x0E)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl1  =(uint8_t) regValue;
  ctrl1 = ctrl1 & 0b11110000;
  ctrl1 = ctrl1 | mode->interval;
  // ctrl1レジスタを書き換え
  if (setRegValue(0xE0, ctrl1)) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}

/*
 * RX8025はタイマは1つしかないため，numは実質無意味．
 * actionは0:off, 1:on
 * RX8025はタイマを一時停止する設定が存在しないため，設定をバックアップ，復元する方法を用いる．
 *       設定 : ctrl1レジスタ(0x0f)の下3bit
 */
int RTC_RX8025_U::controlTimer(uint8_t num, uint8_t action){
  if (num >= RTC_EPSON_RX8025_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;
  // ctrl1レジスタ(0x0E)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL1);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl1  =(uint8_t) regValue;
  uint8_t tail   = ctrl1 & 0b00000111;
  uint8_t header = ctrl1 & 0b11110000;
  if (0==action) {// タイマを一時OFF
    // レジスタのtail値をバックアップ
    timerSetting=tail;
    // レジスタのtail部分をall 0に書き換え
    ctrl1 = ctrl1 & 0b11110000;
  } else if(1==action) {// タイマをON
    if (timerSetting==0xFF) { // バックアップがない場合
      return RTC_U_FAILURE;
    }
    // レジスタのtail値をバックアップした値に書き戻し
    ctrl1 = ctrl1 & 0b11110000;
    ctrl1 = ctrl1 | timerSetting;
    timerSetting=0xFF;
  } else {// エラー
    return RTC_U_ILLEGAL_PARAM;
  }
  // ctrl1レジスタを書き換え
  if (setRegValue(0xE0, ctrl1)) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}
/* ================================================================ */
int RTC_RX8025_U::checkLowPower(void){
  // ctrl2レジスタ(0x0F)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl2  =(uint8_t) regValue;
  uint8_t mask=0b01110000;
  ctrl2 = mask & ctrl2;
  ctrl2 = ctrl2 >> 4;
  return (int) ctrl2;
}

int RTC_RX8025_U::clearPowerFlag(void) {
  // ctrl2レジスタ(0x0F)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl2  =(uint8_t) regValue;
  uint8_t mask=0b10001111;
  uint8_t flag=0b00100000;
  ctrl2 = mask & ctrl2;
  ctrl2 = ctrl2 | flag;
  // ctrl2レジスタを書き換え
  if (setRegValue(0xF0, ctrl2)) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}

int RTC_RX8025_U::setLowPower(uint8_t mode){
  // ctrl2レジスタ(0x0F)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl2  =(uint8_t) regValue;
  switch(mode) {
    case 0 : ctrl2 = ctrl2 & 0b01111111;break;
    case 1 : ctrl2 = ctrl2 | 0b10000000;break;
    default: return RTC_U_ILLEGAL_PARAM;
  }
  if (setRegValue(0xF0, ctrl2)) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}
/* ================================================================ */
/*
 * アラームやタイマが動作したことを示す情報
 * アラーム
 * alarm_W : WAFGビット ctrl2レジスタ'0x0F'の下から2bit目, /INTB端子
 * alarm_D : DAFGビット ctrl2レジスタ'0x0F'の最下位ビット, /INTA端子
 * タイマ
 * CTFGビット ctrl2レジスタ'0x0F'の下から3bit目, /INTA端子
 */

int RTC_RX8025_U::checkInterupt(void){
  // ctrl2レジスタ(0x0F)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return regValue;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t mask=0b111;
  uint16_t rst=(uint16_t) (regValue & mask);
  return rst;
}

int RTC_RX8025_U::clearInterupt(uint16_t type){
  //if (type > 7) return RTC_U_ILLEGAL_PARAM;
  // ctrl2レジスタ(0x0F)の値を取得
  int regValue=getRegValue(RTC_EPSON_RX8025_REG_CONTROL2);
  if (regValue < 0) return RTC_U_FAILURE;
  if (regValue > 255) return RTC_U_FAILURE;
  uint8_t ctrl2=(uint8_t) regValue;
  // マスク値の計算 (下8bitを取得し，bit反転)
  uint8_t mask=(uint8_t) type;
  mask=~mask;
  // ctrl2レジスタ(0x0F)の値を計算
  ctrl2=ctrl2 & mask;
  // ctrl2レジスタに値を書き込み
  if (!setRegValue(0xF0, ctrl2)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/* ================================================================ */
/*
 * 計時に使う発振器の周波数を制御する機能
 */
int RTC_RX8025_U::setOscillator(uint8_t mode) {
  if (mode > 127) return RTC_U_ILLEGAL_PARAM;
  if (!setRegValue(0x70, mode)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

int RTC_RX8025_U::getOscillator(void) {
  return getRegValue(RTC_EPSON_RX8025_REG_DIGITAL_OFFSET);
}
/* ================================================================ */
#ifdef DEBUG
void RTC_RX8025_U::dumpReg(void) {
  String regName[]={
    "sec            ",
    "min            ",
    "hour           ",
    "week           ",
    "day            ",
    "month          ",
    "year           ",
    "digital offset ",
    "alarm_w min    ",
    "alarm_w hour   ",
    "alarm_w weekday",
    "alarm_d min    ",
    "alarm_d hour   ",
    "reserved       ",
    "control1       ",
    "control2       "
  };
  Serial.println("register name   | addr | value");
  Serial.println("----------------+------+------------------");
  for (int i=0; i< RTC_EPSON_RX8025_REG_NUM; i++) {
    Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
  }
}

bool  RTC_RX8025_U::backupRegValues(void) {
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(0x00);
  int result = _i2c_if->endTransmission();     // データの送信と終了処理
  if (result!=0) return false;
  if ((RTC_EPSON_RX8025_REG_NUM+1)!=_i2c_if->requestFrom(_i2c_addr, (RTC_EPSON_RX8025_REG_NUM+1))) return false;
  _i2c_if->read();
  for (int i=0; i<RTC_EPSON_RX8025_REG_NUM;i++){
    regValue[i]=_i2c_if->read();
  }
  return true;
}

bool RTC_RX8025_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num >= RTC_EPSON_RX8025_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}

#endif /* DEBUG */
/* ================================================================ */
