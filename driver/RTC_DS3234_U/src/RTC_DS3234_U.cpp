

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
  info->detectLowBattery=RTC_DS3234_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_DS3234_OSCILLATOR_CONTROL;
}

bool RTC_DS3234_U::begin(bool init){
  _ds3234.begin();
  if (!init) return true;
  if (!_ds3234.validateTimeData()) _ds3234.autoTime();
  _ds3234.controlAlarmInterrupt(0 , 0);
  _ds3234.alarm1(true);
  _ds3234.alarm2(true);
  return true;
}


bool RTC_DS3234_U::getTime(date_t* rst){
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

bool RTC_DS3234_U::setTime(date_t* date){
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
int  RTC_DS3234_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
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
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (mode==NULL) return RTC_U_ILLEGAL_PARAM;
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
 * 0 : 成功(RTC_U_SUCCESS)
 * 【注意事項】停止したアラームを再度有効にする際は，再セットアップが必要
 */
int  RTC_DS3234_U::controlAlarm(uint8_t num, uint8_t action) {
  //if (action != 0 ) return 2;
  if (action != 0 ) return RTC_U_ILLEGAL_PARAM;
  if (num >= RTC_DS3234_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (num == 0 ) {
    _ds3234.controlAlarmInterrupt(0, -1);
  } else {
    _ds3234.controlAlarmInterrupt(-1, 0);
  }
  return RTC_U_SUCCESS;
}


/*
 * 割り込み発生時のレジスタの内容をチェック
 * 返り値
 * 0 : 割り込みは発生していない
 * 1 : アラーム1番が発生した
 * 2 : アラーム2番が発生した
 * 3 : アラーム1,2の両方が割り込み発生した
 */
int RTC_DS3234_U::checkInterupt(void){
  int rst=0;
  if (_ds3234.alarm1(false)) rst=1;
  if (_ds3234.alarm2(false)) rst+=2;
  return rst;
}

/*
 * レジスタの割り込みフラグのクリア
 */
int RTC_DS3234_U::clearInterupt(uint16_t type){
  bool alarm1=false, alarm2=false;
  uint8_t tmp;
  // alarm1のチェック
  tmp = (uint8_t) (type & 0b1);
  if (tmp > 0) alarm1=true;
  tmp = (uint8_t) ((type >>1) & 0b1);
  if (tmp > 0) alarm2=true;
  if (alarm1) {
    _ds3234.alarm1(true);
  }
  if (alarm2) {
    _ds3234.alarm2(true);
  }
  return RTC_U_SUCCESS;
}

/*
 * クロック信号出力設定と出力開始
 * Controlレジスタ(0x0E)のINTCN bit(2bit目)をクリア
 *   RS2, RS1 (3,4bit目)に値を代入
 * num=0 : INT/SQW端子への出力
 * num=1 : 32KHz端子への出力
 */
int  RTC_DS3234_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 3) return RTC_U_ILLEGAL_PARAM;
  if (pin != -1) return RTC_U_ILLEGAL_PARAM;
  return setClockOutMode(num, freq);
}

/*
 * クロック出力の設定
 * setClockOut()と同じ
 */
int RTC_DS3234_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 3) return RTC_U_ILLEGAL_PARAM;
  if (num == 0) {
    switch(freq) {
      case 0 : _ds3234.writeSQW(SQW_SQUARE_1);break;
      case 1 : _ds3234.writeSQW(SQW_SQUARE_1K);break;
      case 2 : _ds3234.writeSQW(SQW_SQUARE_4K);break;
      case 3 : _ds3234.writeSQW(SQW_SQUARE_8K);break;
    }
    return RTC_U_SUCCESS;
  } else {
    return RTC_U_UNSUPPORTED;
  }
}

/*
 * クロック出力の制御
 * num=0 : コントロールレジスタ(0x0E)のINTCNビット(下から3bit目)を書き換え
 * num=1 : ステータスレジスタ(0x0F)のEN32kHzビット(bit 3)を書き換え
 */
int RTC_DS3234_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_DS3234_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (1 < mode ) return RTC_U_ILLEGAL_PARAM;
  if (num==0) {
    //_ds3234.controlAlarmInterrupt(-1,-1);
    uint8_t reg=_ds3234.readFromRegister(DS3234_REGISTER_CONTROL);
    if (1==mode) { // ONにする場合
      reg = reg & 0b11111011;
    } else { // OFFにする場合
      reg = reg | 0b00000100;
    }
    _ds3234.writeToRegister(DS3234_REGISTER_CONTROL,reg);
    return RTC_U_SUCCESS;
  } else {
    uint8_t reg=_ds3234.readFromRegister(DS3234_REGISTER_STATUS);
    if (0==mode) { // OFFにする場合
      reg = reg & 0b11110111;
    } else { // ONにする場合
      reg = reg | 0b00001000;
    }
    _ds3234.writeToRegister(DS3234_REGISTER_STATUS,reg);
    return RTC_U_SUCCESS;
  }
}

int RTC_DS3234_U::checkLowPower(void) {
  uint8_t reg;
  reg=_ds3234.readFromRegister(DS3234_REGISTER_STATUS);
  uint8_t mask=0b10000000;
  reg=reg & mask;
  if (reg>0) return 1;
  return 0;
}

int RTC_DS3234_U::clearPowerFlag(void) {
  uint8_t reg;
  reg=_ds3234.readFromRegister(DS3234_REGISTER_STATUS);
  uint8_t mask=0b01111111;
  reg=reg & mask;
  _ds3234.writeToRegister(DS3234_REGISTER_STATUS, reg);
  return RTC_U_SUCCESS;
}

/* ================  温度関係 ============== */
/*
 * 温度レジスタMSB 0x11 (0-6bit) 正負フラグ 7bit目
 * 温度レジスタMSB 0x12 上位2bit (6,7bit)
 * x 0.25で摂氏の温度が取得
 */
float RTC_DS3234_U::getTemperature(uint8_t kind) {
  uint8_t msb = _ds3234.readFromRegister(DS3234_REGISTER_TEMPM);
  uint8_t lsb = _ds3234.readFromRegister(DS3234_REGISTER_TEMPL);
  uint8_t flag = msb & 0b10000000;
  lsb = (lsb >> 6) & 0b00000011;
  int val = ( msb & 0b01111111) << 2;
  val = val | lsb;
  if (0!=flag) val = val*(-1);
  float celcius = val * 0.25;
  float kelvin = celcius + 273.15;
  float fahrenheit = celcius*1.8 + 32.0;
  switch (kind) {
    case RTC_U_TEMPERATURE_KELVIN:return kelvin;
    case RTC_U_TEMPERATURE_CELCIUS:return celcius;
    case RTC_U_TEMPERATURE_FAHRENHEIT: return fahrenheit;
  }
  return RTC_U_TEMPERATURE_FAILURE;
}

/*
 * 温度制御レジスタ(0x13) BB_TD(最下位bit) : 0 温度制御なし , 1 温度制御あり
 *    バッテリバックアップ中の温度制御を行うか否かの設定
 * ステータスレジスタ(0x0F)のCRATE1,CRATE0 (4,5bit) : 0から3 (温度変換サンプル周期指定)
 * 
 * modeの最下位bit : BB_TDに対応
 * modeの1,2bit   : CRATE1, CRATE0に対応
 */
int RTC_DS3234_U::setTemperatureFunction(uint8_t mode) {
  uint8_t BB_TD = mode & 0b1;
  uint8_t CRATE = mode & 0b110;
  CRATE = CRATE<<3;
  if (mode >= 8) return RTC_U_ILLEGAL_PARAM;
  if (0==BB_TD) {
    _ds3234.writeToRegister(DS3234_REGISTER_TEMPEN, 0);
  } else {
    _ds3234.writeToRegister(DS3234_REGISTER_TEMPEN, 1);
  }
  uint8_t status_reg = _ds3234.readFromRegister(DS3234_REGISTER_STATUS);
  status_reg = status_reg & 0b11001111;
  status_reg = status_reg | CRATE;
  _ds3234.writeToRegister(DS3234_REGISTER_STATUS, status_reg);
  return RTC_U_SUCCESS;
}

/*
 * ステータスレジスタ(0x0F)のBSY(2bit目)が1の場合は制御不可 (0か1かを返す)
 */
int RTC_DS3234_U::getTemperatureFunctionState(void) {
  uint8_t status_reg = _ds3234.readFromRegister(DS3234_REGISTER_STATUS);
  status_reg = (status_reg >> 2) & 0b1;
  return (int) status_reg;
}

/*
 * ステータスレジスタ(0x0F)のBSY(2bit目)が1の場合は制御不可
 * 制御レジスタ(0x0E)のCONV(5bit目)に1を書き込むと手動で変換され，終了すると自動で0に戻る
 */
int RTC_DS3234_U::controlTemperatureFunction(uint8_t action) {
  if (action !=1) return RTC_U_ILLEGAL_PARAM;
  uint8_t status_reg = _ds3234.readFromRegister(DS3234_REGISTER_STATUS);
  status_reg = status_reg & 0b100;
  if (1==status_reg) return RTC_U_FAILURE;
  uint8_t control_reg = _ds3234.readFromRegister(DS3234_REGISTER_CONTROL);
  control_reg = control_reg | 0b00100000;
  _ds3234.writeToRegister(DS3234_REGISTER_CONTROL, control_reg);
  return RTC_U_SUCCESS;
}

/* ======================= oscillator control =========================== */
int RTC_DS3234_U::setOscillator(uint8_t mode) {
  _ds3234.writeToRegister(DS3234_REGISTER_XTAL, mode);
  return RTC_U_SUCCESS;
}

int RTC_DS3234_U::getOscillator(void) {
  return _ds3234.readFromRegister(DS3234_REGISTER_XTAL);
}

/* ================================================================ */
#ifdef DEBUG
void RTC_DS3234_U::dumpReg(void) {
  String regName[]={
    "sec            ",
    "min            ",
    "hour           ",
    "week           ",
    "day            ",
    "month          ",
    "year           ",
    "alarm_1 sec    ",
    "alarm_1 min    ",
    "alarm_1 hour   ",
    "alarm_1 w/m day",
    "alarm_2 min    ",
    "alarm_2 hour   ",
    "alarm_2 w/m day",
    "control        ",
    "status         ",
    "crystal offset ",
    "temperature MSB",
    "temperature LSB",
    "temperature ctl"
  };
  Serial.println("register name   | addr | value");
  Serial.println("----------------+------+------------------");
  for (int i=0; i< RTC_DS3234_REG_NUM; i++) {
    if (i > 0x0F) {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("  | ");Serial.println(regValue[i],BIN);

    } else {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
    }
  }
}

bool RTC_DS3234_U::backupRegValues(void){
  for (int i=0; i<RTC_DS3234_REG_NUM;i++){
    regValue[i]=_ds3234.readFromRegister((DS3234_registers) i);
  }
  return true;
}

bool RTC_DS3234_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num >= RTC_DS3234_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
