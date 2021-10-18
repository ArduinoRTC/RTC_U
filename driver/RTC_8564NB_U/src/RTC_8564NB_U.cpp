/*******************************************************************************
*  RTC8564NB - エプソン8564NB(リアルタイムクロック)ライブラリ                  *
*                                                                              *
*                                                                              *
* ============================================================================ *
*   VERSION  DATE        BY             CHANGE/COMMENT                         *
* ---------------------------------------------------------------------------- *
*   1.00     2012-01-15  きむ茶工房     Create                                 *
*   1.01     2012-01-24  きむ茶工房     rTime()のバグ修正(ans=0の行追加)       *
*   1.10     2012-09-13  きむ茶工房     ArduinoIDE1.0.1に対応して変更          *
*******************************************************************************/
/*
 *  RTC用ライブラリも含めて，大幅変更したため，原型をあまり
 *  とどめていないが，きむ茶工房さんのものを引き継いでます．
 */

#include "RTC_8564NB_U.h"

#define WRITE_FIX

// Constructors
RTC_8564NB_U::RTC_8564NB_U(TwoWire * theWire, int32_t id)
{
  _i2c_if=theWire;
  _rtcID=id;
}

void RTC_8564NB_U::getRtcInfo(rtc_info_t *info){
  info->type=EPSON8564NB;
  info->numOfInteruptPin=RTC_EPSON_8564NB_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_EPSON_8564NB_NUM_OF_ALARM;
  info->numOfTimer=RTC_EPSON_8564NB_NUM_OF_TIMER;
  info->numOfClockOut=RTC_EPSON_8564NB_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_EPSON_8564NB_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_EPSON_8564NB_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_EPSON_8564NB_HAVE_MILLISEC;
  info->independentSQW=RTC_EPSON_8564NB_INDEPENDENT_SQW;
}

bool RTC_8564NB_U::begin(uint32_t addr){
  _i2c_addr=addr;
  _i2c_if->begin();
  delay(RTC_EPSON_8564NB_INIT_DELAY);
  if ( -1 == initRTC() ) return false;
  controlTimer(0,0);
  controlAlarm(0,0);
  return true;
}


bool RTC_8564NB_U::getTime(rtc_date_t* rst){
  byte tm[7];
  rTime(tm);
  rst->year      = bcd2bin(tm[6]) + 2000;
  rst->month     = bcd2bin(tm[5]);
  rst->wday      = bcd2bin(tm[4]);
  rst->mday      = bcd2bin(tm[3]);
  rst->hour      = bcd2bin(tm[2]);
  rst->minute    = bcd2bin(tm[1]);
  rst->second    = bcd2bin(tm[0]);
  rst->millisecond = 0;
  return true;
}

bool RTC_8564NB_U::setTime(rtc_date_t* date){
  byte year=(byte) (date->year - 2000);
#ifdef DEBUG
  Serial.print(year);Serial.print("/");Serial.print(date->month);Serial.print("/");Serial.print(date->mday);Serial.print(" ");
  switch(date->wday) {
    case SUN : Serial.print("SUN");break;
    case MON : Serial.print("MON");break;
    case TUE : Serial.print("TUE");break;
    case WED : Serial.print("WED");break;
    case THU : Serial.print("THU");break;
    case FRI : Serial.print("FRI");break;
    case SAT : Serial.print("SAT");break;
  }
  Serial.print(" ");
  Serial.print(date->hour);Serial.print(":");Serial.print(date->minute);Serial.print(":");Serial.print(date->second);Serial.print(" ");Serial.print(date->millisecond);
  Serial.println("");
#endif /* DEBUG */
  if (0 == sTime((byte) year,(byte)date->month,(byte)date->mday,(byte)date->wday,(byte)date->hour,(byte)date->minute,(byte)date->second)) {
    return true;
  }
  return false;
}

/*
 * アラームの設定
 */
int  RTC_8564NB_U::setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  if (0== SetAlarm(timing->hour, timing->minute, timing->mday, timing->wday) ) {
    if (RTC_U_SUCCESS==setAlarmMode(num, mode)) return RTC_U_SUCCESS;
  }
  return RTC_U_FAILURE;
}


/*
 * アラームの動作モード設定
 *   コントロールレジスタ2のAIEビットを変更
 *   (割り込みピンを制御するか否かの設定)
 *  引数modeの要素がNULL場合は変更しない
 */

int  RTC_8564NB_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  if (mode == NULL) return RTC_U_UNSUPPORTED;
  byte pinBits, control2_reg;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2, &control2_reg)) {
    return RTC_U_FAILURE;
  }
  switch(mode->useInteruptPin) {
  case 0  : pinBits = 0b11111101;break;
  case 1  : pinBits = 0b11111111;break;
  default :
    if (0==(control2_reg & 0b00000010)) {
      pinBits = 0b11111101;
    } else {
      pinBits = 0b11111111;
    }
  }
  byte writeVal = control2_reg & pinBits ;
  if (!writeReg(RTC_EPSON_8564NB_REG_CONTROL2,writeVal)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * アラームの動作変更(start/stop/resumeなど)
 * タイマコントールレジスタ(アドレス0x0E)のTEビット(最上位ビット)で制御
 * 0 : 停止
 * 停止したアラームを再度有効にする際は，再セットアップが必要
 */
int  RTC_8564NB_U::controlAlarm(uint8_t num, uint8_t action) {
  if (action != 0 ) return RTC_U_UNSUPPORTED;
  if (num >= RTC_EPSON_8564NB_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  byte minute_alarm_reg, hour_alarm_reg, day_alarm_reg, weekday_alarm_reg;
  byte minute_alarm_bits, hour_alarm_bits, day_alarm_bits, weekday_alarm_bits;
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_MINUITE, &minute_alarm_reg)) {
    return RTC_U_FAILURE;
  }
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_HOUR, &hour_alarm_reg)) {
    return RTC_U_FAILURE;
  }
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_DAY, &day_alarm_reg)) {
    return RTC_U_FAILURE;
  }
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY, &weekday_alarm_reg)) {
    return RTC_U_FAILURE;
  }
  minute_alarm_bits  = minute_alarm_reg  | 0b10000000;
  hour_alarm_bits    = hour_alarm_reg    | 0b10000000;
  day_alarm_bits     = day_alarm_reg     | 0b10000000;
  weekday_alarm_bits = weekday_alarm_reg | 0b10000000;

  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_MINUITE,minute_alarm_bits)) return RTC_U_FAILURE;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_HOUR,hour_alarm_bits)) return RTC_U_FAILURE;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_DAY,day_alarm_bits)) return RTC_U_FAILURE;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY,weekday_alarm_bits)) return RTC_U_FAILURE;

  return RTC_U_SUCCESS;
}

/*
 * タイマの設定
 */
int  RTC_8564NB_U::setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_TIMER) return RTC_U_UNSUPPORTED;
  int rst = setTimerMode(num, mode);
  if (rst != 0) return RTC_U_FAILURE;
  if (0== SetTimer(mode->interval, multi) ) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
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

int  RTC_8564NB_U::setTimerMode(uint8_t num, timer_mode_t * mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_TIMER) return RTC_U_UNSUPPORTED;
  if (mode == NULL) return RTC_U_UNSUPPORTED;
  byte repeatBits, pinBits, intervalBits, control2_reg, timerControl_reg;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2, &control2_reg)) {
    return RTC_U_FAILURE;
  }
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL, &timerControl_reg)) {
    return RTC_U_FAILURE;
  }
  switch(mode->repeat) {
  case 0  : repeatBits = 0b11101111;break;
  case 1  : repeatBits = 0b11111111;break;
  default :
    if (0==(control2_reg & 0b00010000)) {
      repeatBits = 0b11101111;
    } else {
      repeatBits = 0b11111111;
    }
  }
  switch(mode->useInteruptPin) {
  case 0  : pinBits = 0b11111110;break;
  case 1  : pinBits = 0b11111111;break;
  default :
    if (0==(control2_reg & 0b00000001)) {
      pinBits = 0b11111110;
    } else {
      pinBits = 0b11111111;
    }
  }
  switch(mode->interval) {
  case 0  : intervalBits = 0b11111100;break;
  case 1  : intervalBits = 0b11111101;break;
  case 2  : intervalBits = 0b11111110;break;
  case 3  : intervalBits = 0b11111111;break;
  default :
    byte tmp = timerControl_reg & 0b00000011;
    switch(tmp) {
    case 0  : intervalBits = 0b11111100;break;
    case 1  : intervalBits = 0b11111101;break;
    case 2  : intervalBits = 0b11111110;break;
    case 3  : intervalBits = 0b11111111;break;
    }
  }
  byte writeVal = control2_reg & repeatBits & pinBits ;
  if (!writeReg(RTC_EPSON_8564NB_REG_CONTROL2,writeVal)) return RTC_U_FAILURE;
  writeVal = timerControl_reg | intervalBits;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL,writeVal)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * タイマの動作変更(start/stop/resumeなど)
 * タイマコントールレジスタ(アドレス0x0E)のTEビット(最上位ビット)で制御
 * 0 : 停止
 * 1 : 動作
 */
int  RTC_8564NB_U::controlTimer(uint8_t num, uint8_t action){
  if (num >= RTC_EPSON_8564NB_NUM_OF_TIMER) return RTC_U_UNSUPPORTED;
  byte timerControl_reg, TEbits;
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL, &timerControl_reg)) {
    return RTC_U_UNSUPPORTED;
  }
  switch(action){
  case 0 : TEbits = 0b01111111;break;
  case 1 : TEbits = 0b11111111;break;
  default : return RTC_U_UNSUPPORTED;
  }
  byte writeVal = timerControl_reg & TEbits;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL,writeVal)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * 割り込み発生時のレジスタの内容をチェック
 */
uint16_t RTC_8564NB_U::checkInterupt(void){
  byte reg1 ;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&reg1)) return -1;
  reg1 = reg1 & 0b00001100;
  reg1 = reg1 >> 2;
  uint8_t alarms, timers;
  alarms = reg1 & 0b00000001;
  reg1 = reg1 >> 1;
  timers = reg1 & 0b00000001;
  uint16_t ret = (uint16_t) (timers << 8) | alarms;
  return ret;
}

/* レジスタの割り込みフラグのクリア */
bool RTC_8564NB_U::clearInterupt(uint16_t type){
  uint8_t timers, alarms;
  alarms=(uint8_t) 0x00FF & type;
  timers=(uint8_t)(type >> 16);
  if ((alarms > RTC_EPSON_8564NB_NUM_OF_ALARM) || (timers > RTC_EPSON_8564NB_NUM_OF_TIMER)) return false;
  uint8_t mask;
  if ((alarms == 0) && (timers == 1)) { // タイマのフラグを消す
    mask = 0b11111011;
  } else if ((alarms == 1) && (timers == 0)) { // アラームのフラグを消す
    mask = 0b11110111;
  } else { // 両方消す
    mask = 0b11110011;
  }
  byte reg1;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&reg1)) return false;
  reg1 = reg1 & mask;
  _i2c_if->beginTransmission(_i2c_addr) ;  // 通信の開始処理
  _i2c_if->write(0x01) ;                  // レジスターアドレスは01hを指定する
  _i2c_if->write(reg1);
  int flag = _i2c_if->endTransmission();
  if (flag == 0) return true;
  return false;
}

/*
 * クロック信号出力設定と出力開始
 * クロック出力レジスタ(0x0D)のFD1(1bit目)とFD0(最下位)ビットを設定(周波数)
 *                        FE(最上位bit)を1に設定
 *                        ピン引数はCLKOE端子の番号
 */
int  RTC_8564NB_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  _clkoe_pin = pin;
  if (freq > 3) return RTC_U_UNSUPPORTED;
  byte clockOutReg = 0b10000000 | freq;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
  if (_clkoe_pin >= 0) {
    pinMode(_clkoe_pin,OUTPUT);
    digitalWrite(_clkoe_pin,HIGH);
  }
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の設定
 * クロック出力レジスタ(0x0D)のFD1(1bit目)とFD0(最下位)ビットを設定(周波数)
 */
int RTC_8564NB_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  if (freq > 3) return RTC_U_UNSUPPORTED;
  byte clockOutReg;
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,&clockOutReg)) return RTC_U_FAILURE;
  clockOutReg = (clockOutReg &0b11111100) | freq;
  if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の制御
 * クロック出力レジスタ(0x0D)の最上位ビットを 0 : 発信停止 1 : 発信開始
 * ピン番号が非負の場合はソフトとピンと両方対応
 */
int  RTC_8564NB_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_EPSON_8564NB_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  byte clockOutReg ;
  if (!readReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,&clockOutReg)) return RTC_U_FAILURE;
  if (mode == 0) { // 止める
    clockOutReg = clockOutReg & 0b01111111;
    if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
    if (_clkoe_pin >= 0) {
      digitalWrite(_clkoe_pin,LOW);
    }
  } else if (mode == 1) {
    clockOutReg = clockOutReg & 0b11111111;
    if (!writeReg(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT,clockOutReg)) return RTC_U_FAILURE;
    if (_clkoe_pin >= 0) {
      digitalWrite(_clkoe_pin,HIGH);
    }
  } else {
    return RTC_U_FAILURE;
  }
  return RTC_U_SUCCESS;
}

// ＢＣＤをバイナリ(10進数)に変換する処理(BCDは２桁まで対応)
unsigned int RTC_8564NB_U::bcd2bin(byte dt)
{
  return ((dt >> 4) * 10) + (dt & 0xf) ;
}

// バイナリ(10進数)をＢＣＤに変換する処理(0-255までの変換に対応)
unsigned int RTC_8564NB_U::bin2bcd(unsigned int num) 
{
  return ((num/100) << 8) | ( ((num%100) / 10) << 4 ) | (num%10) ;
}

/*
 * レジスタの読みとり
 */
bool RTC_8564NB_U::readReg(byte addr, byte *reg){
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
bool RTC_8564NB_U::writeReg(byte addr, byte val) {
  _i2c_if->beginTransmission(_i2c_addr) ;  // 通信の開始処理
  _i2c_if->write((uint8_t)addr) ;                  // レジスターアドレスを指定する
  _i2c_if->write(val);                  // カウントダウンタイマー値の設定
  int ans=_i2c_if->endTransmission() ;      // データの送信と終了処理
  if (ans==0) return true;
  return false;
}

/*********************************************************************************
*  ans = initRTC(void)                                                           *
*    ans    : 戻り値、0=正常終了(電源が落ちていないので何もしない)               *
*                     1=電源が落ちていたので初期化した                           *
*                    -1=失敗                                                     *
*********************************************************************************/

// 電圧が落ちていた場合は時刻を初期化
//  0 : 
//  1 : 電源が落ちていたので初期化
// -1 : 失敗
int RTC_8564NB_U::initRTC(void) {
  int ans=0;
  byte reg1 , reg2;
  _i2c_if->beginTransmission(_i2c_addr);                              // 通信の開始処理（初期化状態のチェック）
  _i2c_if->write((uint8_t) RTC_EPSON_8564NB_REG_CONTROL2);            // レジスターアドレスは01hを指定する
  ans = _i2c_if->endTransmission();                                   // データの送信と終了処理
  if (ans == 0) {
    ans = _i2c_if->requestFrom(_i2c_addr,2);                          // ＲＴＣにデータ送信要求をだす(2バイト)
    if (ans == 2) {
      reg1 = _i2c_if->read();                                         // Control1 レジスタを読み出し
      reg2 = _i2c_if->read();                                         // Control2 レジスタを読み出し
      if (reg2 & 0x80) {                                              // VLビット(Control2レジスタ最上位ビット)がＯＮなら初期化する
        _i2c_if->beginTransmission(_i2c_addr) ;                       // 通信の開始処理（設定の書き込み）
        _i2c_if->write((uint8_t)RTC_EPSON_8564NB_REG_CONTROL1);       // Control1 レジスタを指定する
        _i2c_if->write((uint8_t)0x20) ;                               // Control1レジスタの設定(TEST=0,STOP=1)
        _i2c_if->write(0x11);                                         // Control2(Reg01)の設定(割込み禁止)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_SECONDS));  // Seconds(Reg02)の設定(時刻の秒0-59,VL=0)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_MINUTES));  // Minutes(Reg03)の設定(時刻の分0-59)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_HOURS));    // Hours(Reg04)の設定(時刻の時0-23)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_DAYS));     // Days(Reg05)の設定(カレンダの日1-31)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_WEEKDAYS)); // WeekDays(Reg06)の設定(カレンダの曜日0-6)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_MONTHS));   // Months(Reg07)の設定(カレンダの月1-12)
        _i2c_if->write((byte)bin2bcd(RTC_EPSON_8564NB_REG_YEARS));    // Years(Reg08)の設定(カレンダの年00-99)
        _i2c_if->write((uint8_t)0x80);                                // MinuteAlarm(Reg09)の設定(アラームの分無効)
        _i2c_if->write((uint8_t)0x80);                                // HourAlarm(Reg0A)の設定(アラームの時無効)
        _i2c_if->write((uint8_t)0x80);                                // HourAlarm(Reg0B)の設定(アラームの日無効)
        _i2c_if->write((uint8_t)0x80);                                // WeekDayAlarm(Reg0C)の設定(アラームの曜日無効)
        _i2c_if->write((uint8_t)0x83);                                // CLKOUT(Reg0D)の設定(1Hzで出力する)
        _i2c_if->write((uint8_t)0x00);                                // TimerControl(Reg0E)の設定(タイマ機能は無効)
        _i2c_if->write((uint8_t)0x00);                                // Timer(Reg0F)の設定(タイマ初期値は０)
        ans = _i2c_if->endTransmission();                             // データの送信と終了処理
        if (ans == 0) {
          _i2c_if->beginTransmission(_i2c_addr);                      // 通信の開始処理（時刻のカウント開始）
          _i2c_if->write((uint8_t)RTC_EPSON_8564NB_REG_CONTROL1);     // Control1 レジスタ(00h)を指定
          _i2c_if->write((uint8_t)0x00);                              // Control1(Reg00)の設定(TEST=0,STOP=0)
          ans = _i2c_if->endTransmission();                           // データの送信と終了処理
          delay(RTC_EPSON_8564NB_INIT_DELAY);                         // カウント開始を待つ
        }
        ans = RTC_U_FAILURE;
      } else {
        ans = RTC_U_SUCCESS;
      }
    } else ans = RTC_U_FAILURE;
  }
  return ans;
}

/*******************************************************************************
*  ans = sTime(Year,Mon,mDay,wDay,Hour,Min,Sec)                                *
*    ＲＴＣに日付と時刻を書き込む処理                                          *
*                                                                              *
*    Year : 日付の年(0-99)を指定する(西暦の下２ケタ 2000年-2099年)             *
*    Mon  : 日付の月(1-12)を指定する                                           *
*    mDay : 日付の日(1-31)を指定する(在りえない日を指定したら動作が不定らしい) *
*    wDay : 曜日の指定(日[0] 月[1] 火[2] 水[3] 木[4] 金[5] 土[6])              *
*    Hour : 時刻の時(0-23)を指定する                                           *
*    Min  : 時刻の分(0-59)を指定する                                           *
*    Sec  : 時刻の秒(0-59)を指定する                                           *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::sTime(byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec)
{
  int ans ;

  _i2c_if->beginTransmission(_i2c_addr);      // 通信の開始処理（時刻の設定）
  _i2c_if->write((uint8_t)0x00);              // レジスターアドレスは00hを指定する
  _i2c_if->write((uint8_t)0x20);              // Control1(Reg00)の設定(TEST=0,STOP=1)
  ans = _i2c_if->endTransmission();           // データの送信と終了処理
  if (ans == 0) {
    _i2c_if->beginTransmission(_i2c_addr);    // 通信の開始処理（時刻の設定）
    _i2c_if->write((uint8_t)0x02);            // レジスターアドレスは02hを指定する
    _i2c_if->write((byte)bin2bcd(Sec));       // Seconds(Reg02)の設定(時刻の秒0-59,VL=0)
    _i2c_if->write((byte)bin2bcd(Min));       // Minutes(Reg03)の設定(時刻の分0-59)
    _i2c_if->write((byte)bin2bcd(Hour));      // Hours(Reg04)の設定(時刻の時0-23)
    _i2c_if->write((byte)bin2bcd(mDay));      // Days(Reg05)の設定(カレンダの日1-31)
    _i2c_if->write((byte)bin2bcd(wDay));      // WeekDays(Reg06)の設定(カレンダの曜日0-6)
    _i2c_if->write((byte)bin2bcd(Mon));       // Months(Reg07)の設定(カレンダの月1-12)
    _i2c_if->write((byte)bin2bcd(Year));      // Years(Reg08)の設定(カレンダの年00-99)
    ans = _i2c_if->endTransmission();         // データの送信と終了処理
    if (ans == 0) {
      _i2c_if->beginTransmission(_i2c_addr);  // 通信の開始処理（時刻のカウント開始）
      _i2c_if->write((uint8_t)0x00);          // レジスターアドレスは00hを指定する
      _i2c_if->write((uint8_t)0x00);          // Control1(Reg00)の設定(TEST=0,STOP=0)
      ans = _i2c_if->endTransmission();       // データの送信と終了処理
      delay(RTC_EPSON_8564NB_INIT_DELAY);     // カウント開始を待つ
    }
  }
  return ans ;
}
/*******************************************************************************
*  ans = rTime(*tm)                                                            *
*    ＲＴＣから現在の日付と時刻を読み取る処理                                  *
*                                                                              *
*    *tm  : 読み取ったデータを保存する配列変数を指定する(配列は7バイト必要)    *
*           配列にはＢＣＤデータで秒・分・時・日・曜日・月・年の順で保存する   *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::rTime(byte *tm)
{
  int ans ;

  _i2c_if->beginTransmission(_i2c_addr);      // 通信の開始処理（時刻の受信）
  _i2c_if->write((uint8_t)0x02);              // レジスターアドレスは02hを指定する
  ans = _i2c_if->endTransmission();           // データの送信と終了処理
  if (ans == 0) {
    ans = _i2c_if->requestFrom(_i2c_addr,7);  // ＲＴＣにデータ送信要求をだす
    if (ans == 7) {
      *tm = _i2c_if->read() & 0x7f ;          // 秒
      tm++ ;
      *tm = _i2c_if->read() & 0x7f ;          // 分
      tm++ ;
      *tm = _i2c_if->read() & 0x3f ;          // 時
      tm++ ;
      *tm = _i2c_if->read() & 0x3f ;          // 日
      tm++ ;
      *tm = _i2c_if->read() & 0x07 ;          // 曜日
      tm++ ;
      *tm = _i2c_if->read() & 0x1f ;          // 月
      tm++ ;
      *tm = _i2c_if->read() ;                 // 年
      ans = 0 ;
    } else ans = 5 ;
  }
  return ans ;
}

/*******************************************************************************
*  ans = SetTimer(sclk,count)                                                  *
*    定周期タイマーの設定をする処理（タイマーの開始）                          *
*                                                                              *
*    sclk : ソースクロックの指定(244.14us[0] 15.625ms[1] 1sec[2] 1min[3])      *
*    count: カウント値の指定(1-255) sclk=2 count=10 なら10秒周期となる         *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::SetTimer(byte sclk,byte count)
{
  int ans ;

  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理
  _i2c_if->write((uint8_t)0x0f) ;         // レジスターアドレスは0fhを指定する
  _i2c_if->write(count);                  // カウントダウンタイマー値の設定
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理
  _i2c_if->write((uint8_t)0x0e) ;         // レジスターアドレスは0ehを指定する
  _i2c_if->write(sclk | 0x80) ;           // ソースクロックの設定とタイマーの開始
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  return ans ;
}
/*******************************************************************************
*  ans = StopTimer()                                                           *
*    定周期タイマー機能を無効にする処理（タイマーの終了）                      *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::StopTimer()
{
  int ans ;
  byte Ctrl2;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&Ctrl2)) return -1;
  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理
  _i2c_if->write((uint8_t)0x0e) ;         // レジスターアドレスは0ehを指定する
  _i2c_if->write((uint8_t)0x00) ;         // タイマーの終了
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理（アラームの開始）
  _i2c_if->write((uint8_t)0x01) ;         // レジスターアドレスは01hを指定する
  Ctrl2 = Ctrl2 & 0xfb ;                  // タイマーフラグをクリア(TF=0)
  _i2c_if->write(Ctrl2);                  // Control2(Reg01)の設定
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  return ans ;
}

/*******************************************************************************
*  ans = SetAlarm(Hour,Min,mDay,wDay)                                          *
*    アラームの日時を設定する処理（アラームの開始）                            *
*                                                                              *
*    Hour : 時刻の時(0-23)を指定する、0xff指定でHour設定は無効                 *
*    Min  : 時刻の分(0-59)を指定する、0xff指定でMin設定は無効                  *
*    mDay : 日付の日(1-31)を指定する、0xff指定でmDay設定は無効                 *
*    wDay : 曜日の指定(日[0] 月[1] 火[2] 水[3] 木[4] 金[5] 土[6])              *
*           0xff指定でwDay設定は無効                                           *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::SetAlarm(byte Hour,byte Min,byte mDay,byte wDay)
{
  int ans ;
  byte Ctrl2;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&Ctrl2)) return -1;
  _i2c_if->beginTransmission(_i2c_addr) ;             // 通信の開始処理（アラーム時刻の設定）
  _i2c_if->write((uint8_t)0x09) ;                     // レジスターアドレスは09hを指定する
  if (Min == 0xff)  _i2c_if->write((uint8_t)0x80)   ; // MinuteAlarm(Reg09)の設定(分は無効)
  else _i2c_if->write((byte)bin2bcd(Min))  ;          // 0-59(分は有効)
  if (Hour == 0xff) _i2c_if->write((uint8_t)0x80)   ; // HoursAlarm(Reg0A)の設定(時は無効)
  else _i2c_if->write((byte)bin2bcd(Hour)) ;          // 0-23(時は有効)
  if (mDay == 0xff) _i2c_if->write((uint8_t)0x80)   ; // DaysAlarm(Reg0B)の設定(日は無効)
  else _i2c_if->write((byte)bin2bcd(mDay)) ;          // 1-31(日は有効)
  if (wDay == 0xff) _i2c_if->write((uint8_t)0x80)   ; // WeekDaysAlarm(Reg0C)の設定(曜日は無効)
  else _i2c_if->write((byte)bin2bcd(wDay)) ;          // 0-6(曜日は有効)
  ans = _i2c_if->endTransmission() ;                  // データの送信と終了処理
  _i2c_if->beginTransmission(_i2c_addr) ;             // 通信の開始処理（アラームの開始）
  _i2c_if->write((uint8_t)0x01) ;                     // レジスターアドレスは01hを指定する
  Ctrl2 = (Ctrl2 | 0x02) & 0xf7 ;                     // アラームを有効にする(AIE=1 AF=0)
  _i2c_if->write(Ctrl2);                              // Control2(Reg01)の設定(割込み)
  ans = _i2c_if->endTransmission() ;                  // データの送信と終了処理
  return ans ;
}
/*******************************************************************************
*  ans = StopAlarm()                                                           *
*    アラーム機能を無効にする処理（アラーム終了）                              *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::StopAlarm()
{
  int ans ;
  byte Ctrl2;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&Ctrl2)) return -1;
  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理（アラームの停止）
  _i2c_if->write((uint8_t)0x01) ;         // レジスターアドレスは01hを指定する
  Ctrl2 = Ctrl2 & 0xf5 ;                  // アラームを無効にする(AIE=0 AF=0)
  _i2c_if->write(Ctrl2);                  // Control2(Reg01)の設定(割込み)
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  return ans ;
}
/*******************************************************************************
*  ans = offAlarm()                                                            *
*    アラームの発生を解除する処理（アラームは引き続き有効）                    *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int RTC_8564NB_U::offAlarm()
{
  int ans ;
  byte Ctrl2;
  if (!readReg(RTC_EPSON_8564NB_REG_CONTROL2,&Ctrl2)) return -1;
  _i2c_if->beginTransmission(_i2c_addr) ; // 通信の開始処理（アラーム出力の解除）
  _i2c_if->write((uint8_t)0x01) ;         // レジスターアドレスは01hを指定する
  Ctrl2 = Ctrl2 & 0xf7 ;                  // アラームの出力を解除する(AF=0)
  _i2c_if->write(Ctrl2);                  // Control2(Reg01)の設定(割込み)
  ans = _i2c_if->endTransmission() ;      // データの送信と終了処理
  return ans ;
}
