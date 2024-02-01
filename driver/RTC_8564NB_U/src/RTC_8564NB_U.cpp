
#include "RTC_8564NB_U.h"

RTC_8564NB_U::RTC_8564NB_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
}

void RTC_8564NB_U::getRtcInfo(rtc_u_info_t *info){
  info->type=EPSON8564NB;
  info->numOfInteruptPin=RTC_EPSON_8564NB_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_EPSON_8564NB_NUM_OF_ALARM;
  info->numOfTimer=RTC_EPSON_8564NB_NUM_OF_TIMER;
  info->numOfClockOut=RTC_EPSON_8564NB_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_EPSON_8564NB_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_EPSON_8564NB_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_EPSON_8564NB_HAVE_MILLISEC;
  info->independentSQW=RTC_EPSON_8564NB_INDEPENDENT_SQW;
  info->detectLowBattery=RTC_EPSON_8564NB_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_EPSON_8564NB_OSCILLATOR_CONTROL;
}

bool RTC_8564NB_U::begin(bool force, uint32_t addr){
  _i2c_addr=addr;
  _i2c_if->begin();
  delay(RTC_EPSON_8564NB_INIT_DELAY);
  if (!force) return true;
  date_t date;
  date.year = RTC_EPSON_8564NB_DEFAULT_YEAR;
  date.month = RTC_EPSON_8564NB_DEFAULT_MONTH;
  date.mday = RTC_EPSON_8564NB_DEFAULT_DAY;
  date.wday = RTC_EPSON_8564NB_DEFAULT_DAY_OF_WEEK;
  date.hour = RTC_EPSON_8564NB_DEFAULT_HOUR;
  date.minute = RTC_EPSON_8564NB_DEFAULT_MIN;
  date.second = RTC_EPSON_8564NB_DEFAULT_SECOND;
  if (0 > setTime(&date)) return false;
  if (0 > controlAlarm(0,0)) return false;
  if (0 > controlTimer(0,0)) return false;
  return true;
}

bool RTC_8564NB_U::getTime(date_t* date){
  uint8_t data[7];
  int result;
  for (int i=RTC_EPSON_8564NB_REG_SECONDS; i < RTC_EPSON_8564NB_REG_ALARM_MINUITE; i++) {
    result = getRegValue(i);
    if (0 > result) return false;
    data[i-RTC_EPSON_8564NB_REG_SECONDS] = (uint8_t) result;
    delayMicroseconds(2);
  }
  date->second = bcdToInt(data[0] & 0x7F);
  date->minute = bcdToInt(data[1] & 0x7F);
  date->hour   = bcdToInt(data[2] & 0x3F);
  date->mday   = bcdToInt(data[3] & 0x3F);
  date->wday   = data[4] & 0x07;
  date->month  = bcdToInt(data[5] & 0x1F);
  date->year   = bcdToInt(data[6]) + 2000;
  return true;
}

bool RTC_8564NB_U::setTime(date_t* date){
  uint8_t second_reg = intToBCD((date->second) % 60);
  uint8_t minute_reg = intToBCD((date->minute) % 60);
  uint8_t hours_reg = intToBCD((date->hour) % 24);
  uint8_t date_reg = intToBCD((date->mday) % 32);
  uint8_t wday_reg = intToBCD((date->wday) % 7);
  uint8_t month_reg = intToBCD((date->month) % 13);
  uint8_t year_reg = intToBCD((date->year) % 100);
  if (!setRegValue(RTC_EPSON_8564NB_REG_SECONDS,second_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_MINUTES,minute_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_HOURS,hours_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_DAYS,date_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_WEEKDAYS,wday_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_MONTHS,month_reg)) return false;
  if (!setRegValue(RTC_EPSON_8564NB_REG_YEARS,year_reg)) return false;
  return true;
}

/*
 * アラームの設定
 */
int  RTC_8564NB_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (timing->minute > 59) {
    alarm_minute=0b10000000;
  } else {
    alarm_minute=intToBCD(timing->minute);
  }
  if (timing->hour > 23) {
    alarm_hour=0b10000000;
  } else {
    alarm_hour = intToBCD(timing->hour);
  }
  if (timing->mday > 31) {
    alarm_date=0b10000000;
  } else {
    alarm_date=intToBCD(timing->mday);
  }
  if (timing->wday > 6) {
    alarm_wday=0b10000000;
  } else {
    alarm_wday=timing->wday;
  }

  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_MINUITE,alarm_minute)) return RTC_U_FAILURE;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_HOUR,alarm_hour)) return RTC_U_FAILURE;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_DAY,alarm_date)) return RTC_U_FAILURE;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY,alarm_wday)) return RTC_U_FAILURE;

  int result = setAlarmMode(num, mode);
  if (0 > result) return result;
  result = controlAlarm(num, 1);
  if (0 > result) return result;
  return RTC_U_SUCCESS;
}


/*
 * アラームの動作モード設定
 *   コントロールレジスタ2のAIEビットを変更
 *   (割り込みピンを制御するか否かの設定)
 *  引数modeの要素がNULL場合は変更しない
 */

int  RTC_8564NB_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (mode == NULL) return RTC_U_ILLEGAL_PARAM;

  // control2レジスタ(0x01)の値を取得
  int result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
  if (0> result) return result;
  uint8_t control2_reg = (uint8_t) result;
  if (mode->useInteruptPin==0) {  // AIEビット(bit1)を0
    control2_reg = control2_reg & 0b11111101;
  } else if (mode->useInteruptPin==1) {  // AIEビット(bit1)を1
    control2_reg = control2_reg | 0b00000010;
  } else { // エラー
    return RTC_U_ILLEGAL_PARAM;
  }

  //  control2レジスタに書き込み
  if (!setRegValue(RTC_EPSON_8564NB_REG_CONTROL2,control2_reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * アラームの動作変更(start/stop/resumeなど)
 * 0 : 停止
 * 1 : 動作開始
 */
int  RTC_8564NB_U::controlAlarm(uint8_t num, uint8_t action) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;

  if (action ==0) {
    // アラームの各レジスタ(0x09から0x0CまでのAEビットのみを1にする)
    uint8_t ae = 0b10000000;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_MINUITE,ae)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_HOUR,ae)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_DAY,ae)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY,ae)) return RTC_U_FAILURE;
  } else if (action == 1) {
    // control2レジスタ(0x01)のAFビット(bit3)を0クリア
    int result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
    if (0> result) return result;
    uint8_t control2_reg = (uint8_t) result;
    control2_reg = control2_reg & 0b11110111;
    if (!setRegValue(RTC_EPSON_8564NB_REG_CONTROL2,control2_reg)) return RTC_U_FAILURE;
    // アラームの各レジスタ(0x09から0x0Cまでをバックアップから書き戻し
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_MINUITE,alarm_minute)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_HOUR,alarm_hour)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_DAY,alarm_date)) return RTC_U_FAILURE;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY,alarm_wday)) return RTC_U_FAILURE;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/*
 * タイマの設定
 * タイマレジスタ(0x0F)を書き換えて，モード変更の後，タイマを有効化
 */
int  RTC_8564NB_U::setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi) {
  if (multi > 0xFF) return RTC_U_ILLEGAL_PARAM;
  uint8_t timerCounter_reg = (uint8_t) multi;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_TIMER,timerCounter_reg)) return RTC_U_FAILURE;
  int result = setTimerMode(num, mode);
  if (0>result) return result;
  return controlTimer(num, 1);
}

/*
 * タイマの動作モード設定
 *   コントロールレジスタ2の設定
 *     TI/TPモードの指定
 *     TIEビットの指定
 *   タイマ制御レジスタの設定
 *     TD1 TD0
 *     0    0   : 4096Hz
 *     0    1   : 64Hz
 *     1    0   : 1秒
 *     1    1   : 1分
 *  引数modeの要素が負の場合は変更しない
 */
int  RTC_8564NB_U::setTimerMode(uint8_t num, rtc_timer_mode_t * mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;
  if (mode == NULL) return RTC_U_ILLEGAL_PARAM;

  int result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
  if (0> result) return result;
  uint8_t control2_reg = (uint8_t) result;

  result = getRegValue(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL);
  if (0> result) return result;
  uint8_t timerControl_reg = (uint8_t) result;


  // コントロール2 レジスタ
  // TI/TPビットの設定
  if (mode->repeat ==0) {
    control2_reg = control2_reg & 0b11101111;
  } else if (mode->repeat ==1) {
    control2_reg = control2_reg | 0b00010000;                   
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  // TIEビットの設定
  if (mode->useInteruptPin ==0) {
    control2_reg = control2_reg & 0b11111110;
  } else if (mode->useInteruptPin ==1) {
    control2_reg = control2_reg | 0b00000001;                   
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  // timer controlレジスタ
  // TD1, TD0 ビットの設定
  if (mode->interval>3) return RTC_U_ILLEGAL_PARAM;
  timerControl_reg = (timerControl_reg & 0b11111100)| mode->interval;

  // レジスタに書き込み
  if (!setRegValue(RTC_EPSON_8564NB_REG_CONTROL2,control2_reg)) return RTC_U_FAILURE;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL,timerControl_reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * タイマの動作変更(start/stop/resumeなど)
 * タイマコントールレジスタ(アドレス0x0E)のTEビット(最上位ビット)で制御
 * 0 : 停止
 * 1 : 動作
 */
int  RTC_8564NB_U::controlTimer(uint8_t num, uint8_t action){
  if (num >= RTC_EPSON_8564NB_NUM_OF_TIMER) return RTC_U_ILLEGAL_PARAM;

  // タイマコントロールレジスタ
  int result = getRegValue(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL);
  if (0> result) return result;
  // TEビットを書き換え
  uint8_t timerControl_reg = (uint8_t) result;
  switch(action){
  case 0 : timerControl_reg = timerControl_reg & 0b01111111;break;
  case 1 : timerControl_reg = timerControl_reg | 0b10000000;break;
  default : return RTC_U_ILLEGAL_PARAM;
  }

  // コントロール2 レジスタ
  result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
  if (0> result) return result;
  uint8_t control2_reg = (uint8_t) result;
  // TFビットは強制的に消す
  control2_reg = control2_reg & 0b11111011;

  // レジスタに書き込み
  if (!setRegValue(RTC_EPSON_8564NB_REG_CONTROL2,control2_reg)) return RTC_U_FAILURE;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL,timerControl_reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * 割り込み発生時のレジスタの内容をチェック
 * control2レジスタ(0x01)のAF, TFビットの値を返す
 */
int RTC_8564NB_U::checkInterupt(void){
  int result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
  if (0> result) return result;
  uint8_t intr = (uint8_t) result;
  intr = intr & 0b00001100;
  intr = intr >>2;
  return intr;
}

/*
 * レジスタの割り込みフラグのクリア
 * control2レジスタ(0x01)のAF, TFビット(2,3bit)の0クリア
 */
int RTC_8564NB_U::clearInterupt(uint16_t type){
  type=type&0b11;
  uint8_t mask;
  switch(type) {
    case 0: return RTC_U_SUCCESS;
    case 1: mask = 0b11111011;break;
    case 2: mask = 0b11110111;break;
    default: mask = 0b11110011;
  }
  uint8_t control2;
  int result = getRegValue(RTC_EPSON_8564NB_REG_CONTROL2);
  if (0> result) return result;
  control2 = (uint8_t) result;
  control2 = control2 & mask;
  if (!setRegValue(RTC_EPSON_8564NB_REG_CONTROL2,control2)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * クロック信号出力設定と出力開始
 * クロック出力レジスタ(0x0D)のFD1(1bit目)とFD0(最下位)ビットを設定(周波数)
 *                        FE(最上位bit)を1に設定
 *                        ピン引数はCLKOE端子の番号
 */
int  RTC_8564NB_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  _clkoe_pin = pin;
  int result = setClockOutMode(num ,freq);
  if (RTC_U_SUCCESS!=result) return result;
  result = controlClockOut(num, 1);
  if (RTC_U_SUCCESS!=result) return result;
  pinMode(_clkoe_pin,OUTPUT);
  digitalWrite(_clkoe_pin,HIGH);
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の設定
 * クロック出力レジスタ(0x0D)のFD1(1bit目)とFD0(最下位)ビットを設定(周波数)
 */
int RTC_8564NB_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 3) return RTC_U_ILLEGAL_PARAM;
  int result = getRegValue(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT);
  if (0> result) return result;
  uint8_t clockOutReg=(uint8_t) result;
  clockOutReg = (clockOutReg &0b11111100) | freq;
  if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の制御
 * クロック出力レジスタ(0x0D)の最上位ビットを 0 : 発信停止 1 : 発信開始
 * ピン番号が非負の場合はソフトとピンと両方対応
 */
int  RTC_8564NB_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  int result = getRegValue(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT);
  if (0> result) return result;
  uint8_t clockOutReg=(uint8_t) result;
  if (mode == 0) { // 止める
    clockOutReg = clockOutReg & 0b01111111;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
    if (_clkoe_pin >= 0) {
      digitalWrite(_clkoe_pin,LOW);
    }
  } else if (mode == 1) {
    clockOutReg = clockOutReg | 0b10000000;
    if (!setRegValue(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
    if (_clkoe_pin >= 0) {
      digitalWrite(_clkoe_pin,HIGH);
    }
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/*
 * 電圧低下の検出
 * 秒レジスタ(0x02)のVLビット(bit 7)が1か否かで判定
 */
int RTC_8564NB_U::checkLowPower(void) {
  int result = getRegValue(RTC_EPSON_8564NB_REG_SECONDS);
  if (0> result) return result;
  uint8_t second_reg=(uint8_t) result;
  second_reg = (second_reg & 0b10000000) >> 7;
  return second_reg;
}

/*
 * 電圧低下検出ビットのクリア
 *  秒レジスタ(0x02)のVLビット(bit 7)を0でクリア
 */
int RTC_8564NB_U::clearPowerFlag(void) {
  int result = getRegValue(RTC_EPSON_8564NB_REG_SECONDS);
  if (0> result) return result;
  uint8_t second_reg=(uint8_t) result;
  second_reg = second_reg & 0b01111111;
  if (!setRegValue(RTC_EPSON_8564NB_REG_SECONDS,second_reg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * レジスタの読みとり
 */
int RTC_8564NB_U::getRegValue(uint8_t addr){
  if (addr > 0x0F) return RTC_U_ILLEGAL_PARAM;
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  int result = _i2c_if->endTransmission() ;
  if (result == 0) {
    result = _i2c_if->requestFrom(_i2c_addr,1) ;
    if (result == 1) {
      return (int) _i2c_if->read()  ; // Regを受信
    }
  }
  delayMicroseconds(2);
  return RTC_U_FAILURE;
}

/*
 * レジスタへの書き込み
 */
bool RTC_8564NB_U::setRegValue(uint8_t addr, uint8_t val) {
  _i2c_if->beginTransmission(_i2c_addr);
  _i2c_if->write(addr);
  _i2c_if->write(val);
  int flag = _i2c_if->endTransmission();
  delayMicroseconds(2);
  if (flag == 0) return true;
  return false;
}

/* ================================================================ */
#ifdef DEBUG
bool  RTC_8564NB_U::backupRegValues(void) {
  int result;
  for (int i=0; i<RTC_EPSON_8564NB_REG_NUM;i++){
    result = getRegValue(i);
    if (0>result) return false;
    regValue[i]=(uint8_t) result;
  }
  return true;
}


bool  RTC_8564NB_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value) {
  if (num >= RTC_EPSON_8564NB_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}

void  RTC_8564NB_U::dumpReg(void) {
  String regName[]={
    "Control 1      ",
    "Control 2      ",
    "Second         ",
    "Minute         ",
    "Hour           ",
    "Day            ",
    "Weekday        ",
    "Month          ",
    "Year           ",
    "Minute Alarm   ",
    "Hour Alarm     ",
    "Day Alarm      ",
    "WeekDay Alarm  ",
    "Clkout freq    ",
    "Timer control  ",
    "Timer          "
  };
  Serial.println("register name   | addr | value");
  Serial.println("----------------+------+------------------");
  for (int i=0; i< RTC_EPSON_8564NB_REG_NUM; i++) {
    Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
  }
}

#endif /* DEBUG */
