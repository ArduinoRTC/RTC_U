

#include "RTC_DS3231_U.h"


// Constructors
RTC_DS3231_U::RTC_DS3231_U(TwoWire * theWire, int32_t id) {
  i2c_if=theWire;
  rtcID=id;
}

void RTC_DS3231_U::getRtcInfo(rtc_u_info_t *info){
  info->type=DS3231;
  info->numOfInteruptPin=RTC_DS3231_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_DS3231_NUM_OF_ALARM;
  info->numOfTimer=RTC_DS3231_NUM_OF_TIMER;
  info->numOfClockOut=RTC_DS3231_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_DS3231_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_DS3231_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_DS3231_HAVE_MILLISEC;
  info->independentSQW=RTC_DS3231_INDEPENDENT_SQW;
  info->detectLowBattery=RTC_DS3231_DETECT_LOW_BATTERY;
  info->controlOscillator=RTC_DS3231_OSCILLATOR_CONTROL;
}

bool RTC_DS3231_U::begin(bool init,uint32_t addr ){
  i2c_addr=addr;
  i2c_if->begin();
  delay(RTC_DS3231_INIT_DELAY);
  if (!init) return true;
  uint8_t regs[7];
  regs[0]=intToBCD(RTC_DS3231_DEFAULT_SECOND);
  regs[1]=intToBCD(RTC_DS3231_DEFAULT_MIN);
  regs[2]=intToBCD(RTC_DS3231_DEFAULT_HOUR);
  regs[3]=intToBCD(RTC_DS3231_DEFAULT_DAY_OF_WEEK);
  regs[4]=intToBCD(RTC_DS3231_DEFAULT_DAY);
  regs[5]=intToBCD(RTC_DS3231_DEFAULT_MONTH);
  regs[6]=intToBCD(RTC_DS3231_DEFAULT_YEAR);
  int result = writeRegs(RTC_DS3231_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS) return false;
  return true;
}


bool RTC_DS3231_U::getTime(date_t* date){
  uint8_t regs[7];
  int result = readRegs(RTC_DS3231_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS ) return false;
  date->millisecond=0;
  date->second = bcdToInt(regs[0]);
  date->minute = bcdToInt(regs[1]);
  date->hour   = bcdToInt(regs[2]);
  if (regs[3]==7) {
    date->wday = 0;
  } else {
    date->wday = regs[3];
  }
  date->mday = bcdToInt(regs[4]);
  date->month = bcdToInt(regs[5]);
  date->year = bcdToInt(regs[6]) + 2000;
  return true;
}

bool RTC_DS3231_U::setTime(date_t* date){
  if (date->year >=3000) return false;
  if (date->wday > 6) return false;
  uint8_t regs[7], tmp;
  tmp=(uint8_t) (date->year - 2000);
  regs[6]=intToBCD(tmp);
  tmp=date->wday;
  if (tmp==0) tmp=7;
  regs[3]=tmp;
  regs[0]=intToBCD(date->second);
  regs[1]=intToBCD(date->minute);
  regs[2]=intToBCD(date->hour);
  regs[4]=intToBCD(date->mday);
  regs[5]=intToBCD(date->month);
  int result= writeRegs(RTC_DS3231_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS) return false;
  return true;
}

/*
 * アラームの設定
 */
int  RTC_DS3231_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_DS3231_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t day, hour, min, sec, wday;
  uint8_t pinModeVal=0;
  if (mode->useInteruptPin == 1) pinModeVal=1;
  if (timing->wday==0) {
    wday=7;
  } else if (timing->wday > 6) {
    wday=0xFF;
  } else {
    wday=timing->wday;
  }
  if ((wday == 0xFF)&&(timing->mday==0xFF)) {
    day=0xFF;
  }
  if (0xFF != timing->mday) {
    day=intToBCD(timing->mday);
  } else if (wday<8) {
    day=intToBCD(wday);
    day = day | 0b01000000;
  } else {
    day = 0b10000000;
  }
  if (timing->second==0xFF) {
    sec=0b10000000;
  } else {
    sec=intToBCD(timing->second);
  }
  if (timing->minute==0xFF) {
    min=0b10000000;
  } else {
    min=intToBCD(timing->minute);
  }
  if (timing->hour==0xFF) {
    hour=0b10000000;
  } else {
    hour=intToBCD(timing->hour);
  }

  int result;
  if (num == 0) { // alarm 1の設定
    result = writeRegs(RTC_DS3231_REG_ALARM1_SECCOND, &sec, 1);
    if (result != RTC_U_SUCCESS) return result;
    result = writeRegs(RTC_DS3231_REG_ALARM1_MINUTE, &min, 1);
    if (result != RTC_U_SUCCESS) return result;
    result = writeRegs(RTC_DS3231_REG_ALARM1_HOUR, &hour, 1);
    if (result != RTC_U_SUCCESS) return result;
    result = writeRegs(RTC_DS3231_REG_ALARM1_DAY, &day, 1);
    if (result != RTC_U_SUCCESS) return result;
  } else {
    result = writeRegs(RTC_DS3231_REG_ALARM2_MINUTE, &min, 1);
    if (result != RTC_U_SUCCESS) return result;
    result = writeRegs(RTC_DS3231_REG_ALARM2_HOUR, &hour, 1);
    if (result != RTC_U_SUCCESS) return result;
    result = writeRegs(RTC_DS3231_REG_ALARM2_DAY, &day, 1);
    if (result != RTC_U_SUCCESS) return result;
  }
  result = setAlarmMode(num, mode);
  if (RTC_U_SUCCESS!=result) return result;
  return controlAlarm(num,1);
}


/*
 * アラームの動作モード設定
 *   コントロールレジスタ(0Eh/8Eh)のAxIEビットを変更
 *  引数modeの要素がNULL場合は変更しない
 */
int  RTC_DS3231_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_DS3231_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (mode==NULL) return RTC_U_ILLEGAL_PARAM;
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  uint8_t alarm_1  = reg & 0b00000001;
  uint8_t alarm_2  = reg & 0b00000010;
  uint8_t other    = reg & 0b11111100;
  uint8_t intrupts;
  
  if (num==0) { // alarm 1
    if (mode->useInteruptPin == 1) {
      intrupts=alarm_2 | 0b1;
    } else {
      intrupts=alarm_2;
    }
  } else { // alarm 2
    if (mode->useInteruptPin == 1) {
      intrupts=alarm_1 | 0b10;
    } else {
      intrupts=alarm_1;
    }
  }
  reg = other | intrupts;
  result = writeRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
  return result;
}

/*
 * アラームの動作変更(start/stop/resumeなど)
 * 0 : 成功(RTC_U_SUCCESS)
 * controlレジスタ(0x0E/8E)の下3bit(INTCN, A2IE, A1IE)を書き換え
 */
int  RTC_DS3231_U::controlAlarm(uint8_t num, uint8_t action) {
  if (action > 1 ) return RTC_U_ILLEGAL_PARAM;
  if (num >= RTC_DS3231_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  uint8_t alarm_1  = reg & 0b00000001;
  uint8_t alarm_2  = reg & 0b00000010;
  uint8_t other    = reg & 0b11111000;
  uint8_t intrupts;
  if (num == 0 ) {  // alarm1
    if (action == 0) {
      if (alarm_2 !=0) { // alarm2がONなので，INTCNを1にA1IEを0に設定
        intrupts = 0b00000110;
      } else { // alarm2がOFFなので，INTCNを0に，A1IEを0に設定
        intrupts = 0b00000000;
      }
    } else {
      if (alarm_2 !=0) { // alarm2がONなので，INTCNを1にA1IEを1に設定
        intrupts = 0b00000111;
      } else { // alarm2がOFFなので，INTCNを1に，A1IEを1に設定
        intrupts = 0b00000101;
      }
    }
  } else {
    if (action == 0) {
      if (alarm_1 !=0) { // alarm1がONなので，INTCNを1にA2IEを0に設定
        intrupts = 0b00000101;
      } else { // alarm1がOFFなので，INTCNを1に，A2IEを0に設定
        intrupts = 0b00000000;
      }
    } else {
      if (alarm_1 !=0) { // alarm1がONなので，INTCNを1にA2IEを0に設定
        intrupts = 0b00000111;
      } else { // alarm1がOFFなので，INTCNを1に，A2IEを0に設定
        intrupts = 0b00000110;
      }
    }
  }
  reg = other | intrupts;
  result = writeRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
  return result;
}


/*
 * 割り込み発生時のレジスタの内容をチェック
 * 返り値
 * 0 : 割り込みは発生していない
 * 1 : アラーム1番が発生した
 * 2 : アラーム2番が発生した
 * 3 : アラーム1,2の両方が割り込み発生した
 */
int RTC_DS3231_U::checkInterupt(void){
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  return (reg & 0b00000011);
}

/*
 * レジスタの割り込みフラグのクリア
 * ステータスレジスタ(0x0F|0x8F)のA2F(bit 1)とA1F(bit 0)を0に設定
 */
int RTC_DS3231_U::clearInterupt(uint16_t type){
  uint8_t mask;
  switch(type) {
    case 1: mask = 0b11111110;break;
    case 2: mask = 0b11111101;break;
    default: mask = 0b11111100;break;
  }
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  reg = reg & mask;
  result = writeRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  return result;
}

/*
 * クロック信号出力設定と出力開始
 * Controlレジスタ(0x0E)のINTCN bit(2bit目)をクリア
 *   RS2, RS1 (3,4bit目)に値を代入
 * num=0 : INT/SQW端子への出力
 * num=1 : 32KHz端子への出力
 */
int  RTC_DS3231_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_DS3231_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 3) return RTC_U_ILLEGAL_PARAM;
  return setClockOutMode(num, freq);
}

/*
 * クロック出力の設定
 * setClockOut()と同じ
 */
int RTC_DS3231_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_DS3231_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 3) return RTC_U_ILLEGAL_PARAM;
  if (num == 0) {
    uint8_t mask, SQW;
    mask=0b11100011;
    switch(freq) {
      case 0 : SQW = 0b00000000;break; // 1Hz
      case 1 : SQW = 0b00001000;break; // 1kHz
      case 2 : SQW = 0b00010000;break; // 4kHz
      case 3 : SQW = 0b00011000;break; // 8kHz
    }
    uint8_t reg;
    int result = readRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
    reg = (reg & mask) | SQW;
    result = writeRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
  } else {
    return RTC_U_UNSUPPORTED;
  }
  return RTC_U_SUCCESS;
}

/*
 * クロック出力の制御
 * num=0 : コントロールレジスタ(0x0E)のINTCNビット(下から3bit目)を書き換え
 * num=1 : ステータスレジスタ(0x0F)のEN32kHzビット(bit 3)を書き換え
 */
int RTC_DS3231_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_DS3231_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (1 < mode ) return RTC_U_ILLEGAL_PARAM;
  int result;
  uint8_t reg;
  if (num==0) {
    result = readRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
    if (1==mode) { // ONにする場合
      reg = reg & 0b11111011;
    } else { // OFFにする場合
      reg = reg | 0b00000100;
    }
    result = writeRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
  } else {
    result = readRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
    if (0==mode) { // OFFにする場合
      reg = reg & 0b11110111;
    } else { // ONにする場合
      reg = reg | 0b00001000;
    }
    result = writeRegs(RTC_DS3231_REG_CONTROL, &reg, 1);
    if (RTC_U_SUCCESS!=result) return result;
  }
  return result;
}

/* ================  電圧低下関係 ============== */
int RTC_DS3231_U::checkLowPower(void) {
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  reg=reg & 0b10000000;
  if (reg>0) return 1;
  return 0;
}

int RTC_DS3231_U::clearPowerFlag(void) {
  uint8_t reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  reg=reg & 0b01111111;
  result = writeRegs(RTC_DS3231_REG_STATUS, &reg, 1);
  return result;
}

/* ================  温度関係 ============== */
/*
 * 温度レジスタMSB 0x11 (0-6bit) 正負フラグ 7bit目
 * 温度レジスタMSB 0x12 上位2bit (6,7bit)
 * x 0.25で摂氏の温度が取得
 */
float RTC_DS3231_U::getTemperature(uint8_t kind) {
  uint8_t msb, lsb;
  int result = readRegs(RTC_DS3231_REG_TEMPERATURE_MSB, &msb, 1);
  if (RTC_U_SUCCESS!=result) return RTC_U_TEMPERATURE_FAILURE;
  result = readRegs(RTC_DS3231_REG_TEMPERATURE_LSB, &lsb, 1);
  if (RTC_U_SUCCESS!=result) return RTC_U_TEMPERATURE_FAILURE;
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
 * ステータスレジスタ(0x0F)のBSY(2bit目)が1の場合は制御不可 (0か1かを返す)
 */
int RTC_DS3231_U::getTemperatureFunctionState(void) {
  uint8_t status_reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &status_reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  status_reg = (status_reg >> 2) & 0b1;
  return (int) status_reg;
}

/*
 * ステータスレジスタ(0x0F)のBSY(2bit目)が1の場合は制御不可
 * 制御レジスタ(0x0E)のCONV(5bit目)に1を書き込むと手動で変換され，終了すると自動で0に戻る
 */
int RTC_DS3231_U::controlTemperatureFunction(uint8_t action) {
  if (action !=1) return RTC_U_ILLEGAL_PARAM;
  uint8_t status_reg;
  int result = readRegs(RTC_DS3231_REG_STATUS, &status_reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  status_reg = status_reg & 0b100;
  if (1==status_reg) return RTC_U_FAILURE;
  uint8_t control_reg;
  result = readRegs(RTC_DS3231_REG_CONTROL, &control_reg, 1);
  if (RTC_U_SUCCESS!=result) return result;
  control_reg = control_reg | 0b00100000;
  result = writeRegs(RTC_DS3231_REG_CONTROL, &control_reg, 1);
  return result;
}

/* ======================= oscillator control =========================== */
int RTC_DS3231_U::setOscillator(uint8_t mode) {
  int result = writeRegs(RTC_DS3231_REG_AGING_OFFSET, &mode, 1);
  if (result < 0) return result;
  return RTC_U_SUCCESS;
}

int RTC_DS3231_U::getOscillator(void) {
  uint8_t data;
  int result = readRegs(RTC_DS3231_REG_AGING_OFFSET, &data, 1);
  if (result < 0) return result;
  return data;
}
/* ================================================================ */
int RTC_DS3231_U::readRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if (addr >= RTC_DS3231_REG_NUM) return RTC_U_ILLEGAL_PARAM;
  i2c_if->beginTransmission(i2c_addr);
  i2c_if->write(addr);
  int result = i2c_if->endTransmission();
  if (result!=0) return RTC_U_FAILURE;
  result = i2c_if->requestFrom(i2c_addr, len);
  if (result != len) return RTC_U_FAILURE;
  for (int i=0; i< len; i++) {
    val[i]=i2c_if->read();
  }
  return RTC_U_SUCCESS;
}

int RTC_DS3231_U::writeRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if (addr >= RTC_DS3231_REG_NUM) return RTC_U_ILLEGAL_PARAM;
  i2c_if->beginTransmission(i2c_addr);
  i2c_if->write(addr);
  for (int i=0; i< len ; i++){
    i2c_if->write(val[i]);
  }
  int flag = i2c_if->endTransmission();
  if (flag == 0) return RTC_U_SUCCESS;
  return RTC_U_FAILURE;
}

/* ================================================================ */
#ifdef DEBUG
void RTC_DS3231_U::dumpReg(void) {
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
    "temperature LSB"
  };
  Serial.println("register name   | addr | value");
  Serial.println("----------------+------+------------------");
  for (int i=0; i< RTC_DS3231_REG_NUM; i++) {
    if (i > 0x0F) {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("  | ");Serial.println(regValue[i],BIN);
    } else {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
    }
  }
}

bool RTC_DS3231_U::backupRegValues(void){
  if (RTC_U_SUCCESS != readRegs(0, regValue, RTC_DS3231_REG_NUM)) return false;
  return true;
}

bool RTC_DS3231_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num >= RTC_DS3231_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
