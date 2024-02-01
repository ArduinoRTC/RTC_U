#include "RTC_RV8803_U.h"



RTC_RV8803_U::RTC_RV8803_U(TwoWire * theWire, int32_t id) {
  i2c_if=theWire;
  rtcID=id;
}

bool RTC_RV8803_U::begin(bool init, uint8_t addr) {
  i2c_addr=addr;
  i2c_if->begin();
  delay(RTC_RV8803_INIT_DELAY);
  if (!init) return true;
  date_t date;
  date.year = RTC_RV8803_DEFAULT_YEAR+2000;
  date.month = RTC_RV8803_DEFAULT_MONTH;
  date.mday = RTC_RV8803_DEFAULT_DAY;
  date.wday = RTC_RV8803_DEFAULT_DAY_OF_WEEK;
  date.hour = RTC_RV8803_DEFAULT_HOUR;
  date.minute = RTC_RV8803_DEFAULT_MIN;
  date.second = RTC_RV8803_DEFAULT_SECOND;
  if (!setTime(&date)) return false;
  return true;
}

void RTC_RV8803_U::getRtcInfo(rtc_u_info_t *info){
  info->type=RV8803;
  info->numOfInteruptPin=RTC_RV8803_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_RV8803_NUM_OF_ALARM;
  info->numOfTimer=RTC_RV8803_NUM_OF_TIMER;
  info->numOfClockOut=RTC_RV8803_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_RV8803_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_RV8803_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_RV8803_HAVE_MILLISEC;
  info->independentSQW=RTC_RV8803_INDEPENDENT_SQW;
}

/* ==================== 時刻関係 ==================== */
/*
 * 曜日とbit列の変換
 */
uint8_t RTC_RV8803_U::intToWday(uint8_t i_wday) {
  switch(i_wday) {
    case 0: return 0;
    case 1: return 0b10;
    case 2: return 0b100;
    case 3: return 0b1000;
    case 4: return 0b10000;
    case 5: return 0b100000;
    case 6: return 0b1000000;
  }
  return 0;
}

uint8_t RTC_RV8803_U::wdayToInt(uint8_t wday) {
  switch(wday) {
    case 0b10: return 1;
    case 0b100: return 2;
    case 0b1000: return 3;
    case 0b10000: return 4;
    case 0b100000: return 5;
    case 0b1000000: return 6;
  }
  return 0;
}
/*
 * 
 */
bool RTC_RV8803_U::getTime(date_t* date){
  uint8_t regs[7];
  int result = readRegs(RTC_RV8803_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS ) return false;
  date->millisecond=0;
  date->second = bcdToInt((regs[0] & 0b01111111));
  date->minute = bcdToInt(regs[1]);
  date->hour   = bcdToInt(regs[2]);
  date->wday = wdayToInt(regs[3]);
  date->mday = bcdToInt(regs[4]);
  date->month = bcdToInt(regs[5]);
  date->year = bcdToInt(regs[6]) + 2000;
  return true;
}

bool RTC_RV8803_U::setTime(date_t* date){
  if (date->year >=3000) return false;
  if (date->wday > 6) return false;
  uint8_t regs[7];
  regs[0]=intToBCD(date->second);
  regs[1]=intToBCD(date->minute);
  regs[2]=intToBCD(date->hour);
  regs[3]=intToWday(date->wday);
  regs[4]=intToBCD(date->mday);
  regs[5]=intToBCD(date->month);
  uint8_t year = date->year -2000;
  regs[6]=intToBCD(year);
  int result= writeRegs(RTC_RV8803_REG_SECOND, regs, 7);
  if (RTC_U_SUCCESS != result) return false;
  return true;
}

/* ===================== 電源電圧関係 ========================= */
/*
 * 
 */
int RTC_RV8803_U::checkLowPower(void) {
  uint8_t flag_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (result != RTC_U_SUCCESS ) return result;
  flag_reg = flag_reg & 0b11; // 最下位2bit分だけにする
  return (int) flag_reg;
}

int RTC_RV8803_U::clearPowerFlag(void) {
  uint8_t flag_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (result != RTC_U_SUCCESS ) return result;
  flag_reg = flag_reg & 0b11111100;
  return writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
}

/* ===================== 周波数信号出力 ========================= */
/*
 * 
 */
int RTC_RV8803_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  if (num >= RTC_RV8803_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  clkoe=pin;
  pinMode(clkoe,OUTPUT);
  int result = setClockOutMode(num, freq);
  if (0 > result) return result;
  return controlClockOut(num, 1);
}

int RTC_RV8803_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_RV8803_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (3<freq) return RTC_U_ILLEGAL_PARAM;
  uint8_t ext_reg;
  int rst = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (rst < 0) return rst;
  ext_reg = ext_reg & 0b11110011;
  ext_reg = ext_reg | (freq<<2);
  return writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
}

int RTC_RV8803_U::controlClockOut(uint8_t num, uint8_t action) {
  if (num >= RTC_RV8803_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (action==0) {
    digitalWrite(clkoe,LOW);
  } else if (action==1) {
    digitalWrite(clkoe,HIGH);
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/* ===================== 時計の進み方の制御 ========================= */
/*
 * 
 */
int RTC_RV8803_U::setOscillator(uint8_t mode) {
  if (63 < mode) return RTC_U_ILLEGAL_PARAM;
  //return writeRegs(RTC_RV8803_REG_OFFSET, &mode, 1);
  uint8_t reg_val=mode;
  return writeRegs(RTC_RV8803_REG_OFFSET, &reg_val, 1);
}

int RTC_RV8803_U::getOscillator(void) {
  uint8_t offset;
  int result = readRegs(RTC_RV8803_REG_OFFSET, &offset, 1);
  if (0>result) return result;
  return offset;
}

int RTC_RV8803_U::controlClock(void) {
  uint8_t control_reg;
  int result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0>result) return result;
  control_reg = control_reg | 0b00000001;
  return writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1); 
}

/* ===================== 割り込み関係 ========================= */
/*
 * 
 */
int RTC_RV8803_U::checkInterupt(void) {
  uint8_t flag_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0>result) return result;
  flag_reg = flag_reg & 0b00111100;
  flag_reg = flag_reg >> 2;
  return flag_reg;
}

int RTC_RV8803_U::clearInterupt(uint16_t type) {
  //if (type > 15) return RTC_U_ILLEGAL_PARAM;
  uint8_t mask=(uint8_t) type;
  mask = mask & 0b1111;
  mask = (~mask) << 2;
  mask = mask | 0b11;
  uint8_t flag_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0>result) return result;
  //flag_reg = flag_reg & 0b11000011;
  flag_reg = flag_reg & mask;
  return writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
}

/* ===================== アラーム ========================= */
/*
 * 
 */
int RTC_RV8803_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_RV8803_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t flag_reg, control_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0>result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0>result) return result;
  flag_reg = flag_reg & 0b11110111;
  control_reg = control_reg & 0b11110111;
  result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0>result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0>result) return result;
  if (timing->minute >= 60) {
    alarm_min = 0b10000000;
  } else {
    alarm_min = intToBCD(timing->minute);
  }
  if (timing->hour >= 60) {
    alarm_hour = 0b10000000;
  } else {
    alarm_hour = intToBCD(timing->hour);
  }
  uint8_t ext_reg;
  result = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0>result) return result;
  if (mode->type==0) {
    ext_reg = ext_reg & 0b10111111;
    if (timing->wday > 6) {
      alarm_day = 0b10000000;
    } else {
      alarm_day = intToWday(timing->wday);
    }
  } else if (mode->type==1) {
    ext_reg = ext_reg | 0b01000000;
    if (timing->mday > 31) {
      alarm_day = 0b10000000;
    } else {
      alarm_day = intToBCD(timing->wday);
    }
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  result = writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (result < 0) return result;
  result = setAlarmMode(num, mode);
  if (result < 0) return result;
  return controlAlarm(num, 1);
}

int RTC_RV8803_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_RV8803_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t control_reg;
  int result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0>result) return result;
  control_reg = control_reg & 0b11110110;
  if (mode->useInteruptPin==0) {
    control_reg = control_reg & 0b11110110;
  } else if (mode->useInteruptPin==1) {
    control_reg = control_reg & 0b11110110;
    control_reg = control_reg | 0b00001000;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
}

int RTC_RV8803_U::controlAlarm(uint8_t num, uint8_t action) {
  if (num >= RTC_RV8803_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  int result;
  if (action==0) {
    uint8_t AE = 0b10000000;
    result = readRegs(RTC_RV8803_REG_ALARM_MINUTE, &alarm_min, 1);
    if (0 > result) return result;
    result = readRegs(RTC_RV8803_REG_ALARM_HOUR, &alarm_hour, 1);
    if (0 > result) return result;
    result = readRegs(RTC_RV8803_REG_ALARM_DAY, &alarm_day, 1);
    if (0 > result) return result;
    result = writeRegs(RTC_RV8803_REG_ALARM_MINUTE, &AE, 1);
    if (0 > result) return result;
    result = writeRegs(RTC_RV8803_REG_ALARM_HOUR, &AE, 1);
    if (0 > result) return result;
    result = writeRegs(RTC_RV8803_REG_ALARM_DAY, &AE, 1);
    if (0 > result) return result;
  } else if (action==1) {
    result = writeRegs(RTC_RV8803_REG_ALARM_MINUTE, &alarm_min, 1);
    if (0 > result) return result;
    result = writeRegs(RTC_RV8803_REG_ALARM_HOUR, &alarm_hour, 1);
    if (0 > result) return result;
    result = writeRegs(RTC_RV8803_REG_ALARM_DAY, &alarm_day, 1);
    if (0 > result) return result;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/* ========================= タイマ ========================= */
/*
 * 
 */
int RTC_RV8803_U::setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi) {
  switch(num) {
    case 0: return setIntervalTimer(mode);
    case 1: return setCountdownTimer(mode, multi);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_RV8803_U::setTimerMode(uint8_t num, rtc_timer_mode_t * mode) {
  switch(num) {
    case 0: return setIntervalTimerMode(mode);
    case 1: return setCountdownTimerMode(mode);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_RV8803_U::controlTimer(uint8_t num, uint8_t action) {
  switch(num) {
    case 0: return controlIntervalTimer(action);
    case 1: return controlCountdownTimer(action);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_RV8803_U::setIntervalTimer(rtc_timer_mode_t * mode) {
  uint8_t flag_reg, control_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;
  flag_reg = flag_reg & 0b11011111;
  control_reg = control_reg & 0b11011111;
  result = writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;
  result = setIntervalTimerMode(mode);
  if (0 > result) return result;
  return controlIntervalTimer(1);
}

int RTC_RV8803_U::setIntervalTimerMode(rtc_timer_mode_t * mode) {
  if (mode->interval > 1) return RTC_U_ILLEGAL_PARAM;
  uint8_t ext_reg;
  int result = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;
  ext_reg = ext_reg & 0b11011111;
  if (mode->interval == 1) ext_reg = ext_reg | 0b00100000;
  return writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
}

int RTC_RV8803_U::controlIntervalTimer(uint8_t action) {
  uint8_t control_reg;
  int result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;
  switch(action) {
    case 0: control_reg = control_reg & 0b11011111;break;
    case 1: control_reg = control_reg | 0b00100000;break;
    default: return RTC_U_ILLEGAL_PARAM; 
  }
  return writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
}

int RTC_RV8803_U::setCountdownTimer(rtc_timer_mode_t * mode, uint16_t multi) {
  uint8_t ext_reg, flag_reg, control_reg;
  int result = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;
  ext_reg = ext_reg & 0b11101111;
  control_reg = control_reg & 0b11101111;
  flag_reg = flag_reg & 0b11101111;
  result = writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;

  uint8_t timer_counter_0, timer_counter_1;
  timer_counter_0 = (uint8_t) (multi & 0x00FF);
  timer_counter_1 = (uint8_t) ((multi>>8) & 0x0F);

  result = writeRegs(RTC_RV8803_REG_TIMER_COUNTER_0, &timer_counter_0, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_RV8803_REG_TIMER_COUNTER_1, &timer_counter_1, 1);
  if (0 > result) return result;

  result = setCountdownTimerMode(mode);
  if (0 > result) return result;

  return controlCountdownTimer(1);
}

int RTC_RV8803_U::setCountdownTimerMode(rtc_timer_mode_t * mode) {
  if (mode->useInteruptPin>1) return RTC_U_ILLEGAL_PARAM;
  if (mode->interval>3) return RTC_U_ILLEGAL_PARAM;
  uint8_t ext_reg, control_reg;
  int result = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;

  control_reg = control_reg & 0b11101111;
  if (mode->useInteruptPin==1) control_reg = control_reg | 0b00010000;

  ext_reg = ext_reg & 0b11111100;
  ext_reg = ext_reg | mode->interval;

  result = writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;

  return writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
}


int RTC_RV8803_U::controlCountdownTimer(uint8_t action) {
  if (action > 1) return RTC_U_ILLEGAL_PARAM;
  uint8_t ext_reg;
  int result = readRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
  if (0 > result) return result;
  ext_reg = ext_reg & 0b11101111;
  if (action==1) ext_reg = ext_reg | 0b00010000;
  return writeRegs(RTC_RV8803_REG_EXTENSION, &ext_reg, 1);
}

/* ======================= SRAM アクセス ============================== */
int RTC_RV8803_U::getSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_RV8803_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  addr = addr + RTC_RV8803_SRAM_BASE_ADDR;
  return readRegs(addr, data, len);
}

int RTC_RV8803_U::setSRAM(uint8_t addr, uint8_t *data, uint16_t len) {
  if (len==0) return RTC_U_ILLEGAL_PARAM;
  if ((addr+len)>RTC_RV8803_SRAM_SIZE) return RTC_U_ILLEGAL_PARAM;
  if (data==NULL) return RTC_U_ILLEGAL_PARAM;
  addr = addr + RTC_RV8803_SRAM_BASE_ADDR;
  return writeRegs(addr, data, len);
}

/* ========================= 外部イベント ========================= */
/*
 * 
 */
int RTC_RV8803_U::setEvent(event_mode_t *mode) {
  if (mode->useInteruptPin>1) return RTC_U_ILLEGAL_PARAM;
  if (mode->level>1) return RTC_U_ILLEGAL_PARAM;
  if (mode->filter>3) return RTC_U_ILLEGAL_PARAM;
  uint8_t flag_reg, control_reg, event_reg=0;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;

  control_reg = control_reg & 0b11111011;
  flag_reg = flag_reg & 0b11111011;

  if (mode->useInteruptPin==1) control_reg = control_reg | 0b00000100;
  if (mode->capture) event_reg = 0b10000000;
  if (mode->level==1) event_reg = event_reg | 0b01000000;
  uint8_t ET = (mode->filter) << 4;
  event_reg = event_reg | ET;
  if (mode->reset) event_reg = event_reg | 1;

  result = writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;

  result = writeRegs(RTC_RV8803_REG_CONTROL, &control_reg, 1);
  if (0 > result) return result;

  return writeRegs(RTC_RV8803_REG_EVENT, &event_reg, 1);
}

int RTC_RV8803_U::getEvent(void) {
  uint8_t flag_reg;
  int result = readRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;

  uint8_t evf = flag_reg & 0b00000100;
  if (0!=evf) return RTC_U_NO_EXTERNAL_EVENT;

  flag_reg = flag_reg & 0b11111011;
  result = writeRegs(RTC_RV8803_REG_FLAG, &flag_reg, 1);
  if (0 > result) return result;

  uint8_t ten_ms_reg, sec_reg;
  result = readRegs(RTC_RV8803_REG_10MS_2, &ten_ms_reg, 1);
  if (0 > result) return result;
  result = readRegs(RTC_RV8803_REG_SECOND_3, &sec_reg, 1);
  if (0 > result) return result;

  int val = (sec_reg << 8) | ten_ms_reg;
  return val;
}

/* ================================================================ */
/*
 * レジスタ番号がジャンプしているところがあるが，そこはケアしていないので
 * 使う側で注意が必要
 */
int RTC_RV8803_U::readRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if ((addr >= RTC_RV8803_REG_NUM)&&(addr!=RTC_RV8803_REG_OFFSET)&&(addr!=RTC_RV8803_REG_EVENT)) return RTC_U_ILLEGAL_PARAM;
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

int RTC_RV8803_U::writeRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if ((addr >= RTC_RV8803_REG_NUM)&&(addr!=RTC_RV8803_REG_OFFSET)&&(addr!=RTC_RV8803_REG_EVENT)) return RTC_U_ILLEGAL_PARAM;
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
void RTC_RV8803_U::dumpReg(void) {
  String regName[]={
    "Second          ",
    "Minute          ",
    "Hour            ",
    "WeekDay         ",
    "Date            ",
    "Month           ",
    "Year            ",
    "RAM             ",
    "Alarm Minute    ",
    "Alarm Hour      ",
    "Alarm W/mDay    ",
    "Timer Counter 0 ",
    "Timer Counter 1 ",
    "Extension       ",
    "Flag            ",
    "Control         ",
    "10ms            ",
    "Second          ",
    "Minute          ",
    "Hour            ",
    "WeekDay         ",
    "Date            ",
    "Month           ",
    "Year            ",
    "Alarm Minute    ",
    "Alarm Hour      ",
    "Alarm W/mDay    ",
    "Timer Counter 0 ",
    "Timer Counter 1 ",
    "Extension       ",
    "Flag            ",
    "Control         ",
    "10ms            ",
    "Second          ",
    "Offset          ",
    "Event           "
  };
  Serial.println("register name    | addr | value");
  Serial.println("-----------------+------+------------------");
  for (int i=0; i< RTC_RV8803_REG_NUM; i++) {
    if (i > 0x0F) {
      if (i==0x22) {
        Serial.print(regName[i]);Serial.print(" |  ");Serial.print("2C");Serial.print("  | ");Serial.println(regValue[i],BIN);
      } else if (i==0x23) {
        Serial.print(regName[i]);Serial.print(" |  ");Serial.print("2F");Serial.print("  | ");Serial.println(regValue[i],BIN);
      } else {
        Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("  | ");Serial.println(regValue[i],BIN);
      }
    } else {
      Serial.print(regName[i]);Serial.print(" |  0");Serial.print(i,HEX);Serial.print("  | ");Serial.println(regValue[i],BIN);
    }
  }
}

bool RTC_RV8803_U::backupRegValues(void){
  uint8_t *regBasic=&regValue[RTC_RV8803_REG_SECOND];
  uint8_t *regExt1 =&regValue[RTC_RV8803_REG_10MS];
  if (RTC_U_SUCCESS != readRegs(RTC_RV8803_REG_SECOND, regBasic, RTC_RV8803_REG_NUM_BASIC)) return false;
  if (RTC_U_SUCCESS != readRegs(RTC_RV8803_REG_10MS, regExt1, RTC_RV8803_REG_NUM_EXTENTION_1)) return false;
  uint8_t *regExt2 =&regValue[RTC_RV8803_REG_10MS_2];
  if (RTC_U_SUCCESS != readRegs(RTC_RV8803_REG_10MS_2, regExt2, RTC_RV8803_REG_NUM_EXTENTION_2)) return false;
  uint8_t reg;
  if (RTC_U_SUCCESS != readRegs(RTC_RV8803_REG_OFFSET, &reg, 1)) return false;
  regValue[0x22]=reg;
  if (RTC_U_SUCCESS != readRegs(RTC_RV8803_REG_EVENT, &reg, 1)) return false;
  regValue[0x23]=reg;
  return true;
}

bool RTC_RV8803_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num == RTC_RV8803_REG_OFFSET) num=0x22;
  if (num == RTC_RV8803_REG_EVENT) num=0x23;
  if (num >= RTC_RV8803_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
