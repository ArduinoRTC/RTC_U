

#include "RTC_DS3234_U.h"


// Constructors
RTC_DS3234_U::RTC_DS3234_U(uint8_t cs,  SPIClass *spi, int32_t id) :
  _cs(cs),
  _sck(0xFF),
  _spi(spi),
  _rtcID(id),
  _ds3234(cs,spi)
{}

RTC_DS3234_U::RTC_DS3234_U(uint8_t cs, uint8_t miso, uint8_t mosi, uint8_t sck, int32_t id) :
  _cs(cs),
  _mosi(mosi),
  _miso(miso),
  _sck(sck),
  _rtcID(id),
  _ds3234(cs, mosi, miso, sck)
{}

void RTC_DS3234_U::getRtcInfo(rtc_info_t *info){
  info->type=DS3234;
  info->numOfInteruptPin=RTC_DS3234_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_DS3234_NUM_OF_ALARM;
  info->numOfTimer=RTC_DS3234_NUM_OF_TIMER;
  info->numOfClockOut=RTC_DS3234_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_DS3234_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_DS3234_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_DS3234_HAVE_MILLISEC;
  info->independentSQW=RTC_DS3234_INDEPENDENT_SQW;
}

bool RTC_DS3234_U::begin(void){
  _ds3234.begin();
  if (!_ds3234.validateTimeData()) _ds3234.autoTime();
  _ds3234.controlAlarmInterrupt(0 , 0);
  _ds3234.alarm1(true);
  _ds3234.alarm2(true);
  return true;
}


bool RTC_DS3234_U::getTime(rtc_date_t* rst){
  _ds3234.update();
  rst->year      = _ds3234.getYear() + 2000;
  rst->month     = _ds3234.getMonth();
  rst->wday      = _ds3234.getDay();
  rst->mday      = _ds3234.getDate();
  rst->hour      = _ds3234.getHour();
  rst->minute    = _ds3234.getMinute();
  rst->second    = _ds3234.getSecond();
  rst->millisecond = 0;
  if (rst->wday == 7) rst->wday=0;
  return true;
}

bool RTC_DS3234_U::setTime(rtc_date_t* date){
  uint8_t year=(uint8_t) (date->year - 2000);
  uint8_t tmp=date->wday;
  if (tmp==0) tmp=7;
  _ds3234.setTime( date->second,  date->minute,  date->hour,    tmp,  date->mday,  date->month,  year);
  return true;
}

/*
 * アラームの設定
 * ベースとなるsparkfunのドライバが設定の成功/失敗を返さないため，常時成功(RTC_U_SUCCESS)を返す．
 */
int  RTC_DS3234_U::setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing) {
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  uint8_t day, hour, min, sec, wday;
  uint8_t pinModeVal=0;
  if (mode->useInteruptPin == 1) pinModeVal=1;
  wday = timing->wday;
  if (wday == 7) wday=8;
  if (wday == 0) wday=7;
  bool use_wday=true;
  if (timing->mday >31) {
    day=0xFF;
  } else {
    day=timing->mday;
  }
  if (wday > 7) {
    use_wday=false;
    wday=0xFF;
  } else {
    use_wday=true;
    wday=timing->wday;
  }
  if (timing->hour > 23) {
    hour = 0xFF;
  } else {
    hour = timing->hour;
  }
  if (timing->minute > 59) {
    min = 0xFF;
  } else {
    min = timing->minute;
  }
  if (timing->second > 60) {
    sec=0xFF;
  } else {
    sec = timing->second;
  }
  if (num == 0) {
    if (day==0xFF) {
      _ds3234.setAlarm1( sec, min, hour, wday,use_wday);
    } else {
      _ds3234.setAlarm1( sec, min, hour, day, false);
    }
    _ds3234.controlAlarmInterrupt(pinModeVal, -1);
  } else {
    if (day==0xFF) {
      _ds3234.setAlarm2( min, hour, wday,use_wday);
    } else {
      _ds3234.setAlarm2( min, hour, day, false);
    }
    _ds3234.enableAlarmInterrupt(-1, pinModeVal);
  }
  return RTC_U_SUCCESS;
}


/*
 * アラームの動作モード設定
 *   コントロールレジスタ(0Eh/8Eh)のAxIEビットを変更
 *  引数modeの要素がNULL場合は変更しない
 */
int  RTC_DS3234_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  if (mode==NULL) return RTC_U_UNSUPPORTED;
  int alarm1=-1, alarm2=-1;
  if (num==0) {
    if (mode->useInteruptPin == 1) {
      alarm1=1;
    } else {
      alarm1=0;
    }
  } else {
    if (mode->useInteruptPin == 1) {
      alarm2=1;
    } else {
      alarm2=0;
    }
  }
  _ds3234.controlAlarmInterrupt(alarm1 , alarm2);
  return RTC_U_SUCCESS;
}

/*
 * アラームの動作変更(start/stop/resumeなど)
 * タイマコントールレジスタ(アドレス0x0E)のTEビット(最上位ビット)で制御
 * 0 : 成功(RTC_U_SUCCESS)
 * 2 : 未知のアクション (未使用)
 * -1 : 存在しないアラームを制御しようとした．
 * 【注意事項】停止したアラームを再度有効にする際は，再セットアップが必要
 */
int  RTC_DS3234_U::controlAlarm(uint8_t num, uint8_t action) {
  //if (action != 0 ) return 2;
  if (action != 0 ) return RTC_U_UNSUPPORTED;
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_UNSUPPORTED;
  if (num == 0 ) {
    _ds3234.controlAlarmInterrupt(0, -1);
  } else {
    _ds3234.controlAlarmInterrupt(-1, 0);
  }
  return RTC_U_SUCCESS;
}

/*
 * タイマの設定
 * タイマは無いので，未サポート(-1:RTC_U_UNSUPPORTED)を返す．
 */
int  RTC_DS3234_U::setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi) {
  return RTC_U_UNSUPPORTED;
}

/*
 * タイマの動作モード設定
 * タイマは無いので，未サポート(-1:RTC_U_UNSUPPORTED)を返す．
 */
int  RTC_DS3234_U::setTimerMode(uint8_t num, timer_mode_t * mode) {
  return RTC_U_UNSUPPORTED;
}

/*
 * タイマの動作変更(start/stop/resumeなど)
 * タイマは無いので，未サポート(-1:RTC_U_UNSUPPORTED)を返す．
 */
int  RTC_DS3234_U::controlTimer(uint8_t num, uint8_t action){
  return RTC_U_UNSUPPORTED;
}

/*
 * 割り込み発生時のレジスタの内容をチェック
 * 返り値
 * 0 : 割り込みは発生していない
 * 1 : アラーム1番が発生した
 * 2 : アラーム2番が発生した
 * 3 : アラーム1,2の両方が割り込み発生した
 */
uint16_t RTC_DS3234_U::checkInterupt(void){
  int rst=0;
  if (_ds3234.alarm1(false)) rst=1;
  if (_ds3234.alarm2(false)) rst+=2;
  return rst;
}

/*
 * レジスタの割り込みフラグのクリア
 */
bool RTC_DS3234_U::clearInterupt(uint16_t type){
  bool alarm1=false, alarm2=false, rst=true;
  if (type == 0) {
    if (!_ds3234.alarm1(true)) rst=false;
    if (!_ds3234.alarm2(true)) rst=false;
    return rst;
  }
  if ((type & 0b1) > 0) alarm1=true;
  if (((type >> 1) & 0b1) > 0) alarm2=true;
  if (alarm1) {
    if (!_ds3234.alarm1(true)) rst=false;
  }
  if (alarm2) {
    if (!_ds3234.alarm2(true)) rst=false;
  }
  return rst;
}

/*
 * クロック信号出力設定と出力開始
 */
int  RTC_DS3234_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  if (freq > 3) return RTC_U_UNSUPPORTED;
  if (pin > -1) return RTC_U_UNSUPPORTED;
  switch(freq) {
    case 0 : _ds3234.writeSQW(SQW_SQUARE_1);break;
    case 1 : _ds3234.writeSQW(SQW_SQUARE_1K);break;
    case 2 : _ds3234.writeSQW(SQW_SQUARE_4K);break;
    case 3 : _ds3234.writeSQW(SQW_SQUARE_8K);break;
  }
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の設定
 */
int RTC_DS3234_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  if (freq > 3) return RTC_U_UNSUPPORTED;
  switch(freq) {
    case 0 : _ds3234.writeSQW(SQW_SQUARE_1);break;
    case 1 : _ds3234.writeSQW(SQW_SQUARE_1K);break;
    case 2 : _ds3234.writeSQW(SQW_SQUARE_4K);break;
    case 3 : _ds3234.writeSQW(SQW_SQUARE_8K);break;
  }
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の制御
 * クロック出力レジスタ(0x0D)の最上位ビットを 0 : 発信停止 1 : 発信開始
 * ピン番号が非負の場合はソフトとピンと両方対応
 */
int  RTC_DS3234_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_UNSUPPORTED;
  _ds3234.controlAlarmInterrupt(-1,-1);
  return RTC_U_SUCCESS;
}
