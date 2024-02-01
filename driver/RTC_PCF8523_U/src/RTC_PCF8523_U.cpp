#include "RTC_PCF8523_U.h"

RTC_PCF8523_U::RTC_PCF8523_U(TwoWire * theWire, int32_t id) {
  i2c_if=theWire;
  rtcID=id;
}

bool RTC_PCF8523_U::begin(bool init, uint8_t addr) {
  clockOutMode=0b111;
  i2c_addr=addr;
  i2c_if->begin();
  delay(RTC_PCF8523_INIT_DELAY);
  uint8_t control_1;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0>result) return false;
  if (!init) {
    control_1 = control_1 & 0b11010111;
    result = writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
    if (0>result) return false;
    return true;
  }
  control_1 = 0x58;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0>result) return false;
  uint8_t control_3 = 0b0;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  if (0>result) return false;
  date_t date;
  date.year = RTC_PCF8523_DEFAULT_YEAR+2000;
  date.month = RTC_PCF8523_DEFAULT_MONTH;
  date.mday = RTC_PCF8523_DEFAULT_DAY;
  date.wday = RTC_PCF8523_DEFAULT_DAY_OF_WEEK;
  date.hour = RTC_PCF8523_DEFAULT_HOUR;
  date.minute = RTC_PCF8523_DEFAULT_MIN;
  date.second = RTC_PCF8523_DEFAULT_SECOND;
  if (0 > setTime(&date)) return false;
  return true;
}

void RTC_PCF8523_U::getRtcInfo(rtc_u_info_t *info){
  info->type=PCF8523;
  info->numOfInteruptPin=RTC_PCF8523_NUM_OF_INTERUPT_PIN;
  info->numOfAlarm=RTC_PCF8523_NUM_OF_ALARM;
  info->numOfTimer=RTC_PCF8523_NUM_OF_TIMER;
  info->numOfClockOut=RTC_PCF8523_NUM_OF_CLOCKOUT;
  info->numOfYearDigits=RTC_PCF8523_NUM_OF_YEAR_DIGITS;
  info->haveYearOverflowBit=RTC_PCF8523_HAVE_CENTURY_BIT;
  info->haveMilliSec=RTC_PCF8523_HAVE_MILLISEC;
  info->independentSQW=RTC_PCF8523_INDEPENDENT_SQW;
}

/* ==================== 時刻関係 ==================== */
/*
 * 
 */
bool RTC_PCF8523_U::getTime(date_t* date){
  uint8_t regs[7];
  int result = readRegs(RTC_PCF8523_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS ) return false;
  date->millisecond=0;
  date->second = bcdToInt((regs[0] & 0b01111111));
  date->minute = bcdToInt(regs[1]);
  date->hour   = bcdToInt(regs[2]);
  date->mday = bcdToInt(regs[3]);
  date->wday = regs[4];
  date->month = bcdToInt(regs[5]);
  date->year = bcdToInt(regs[6]) + 2000;
  return true;
}

bool RTC_PCF8523_U::setTime(date_t* date){
  if (date->year >=3000) return false;
  uint8_t year=date->year-2000;
  if (date->wday > 6) return false;
  uint8_t regs[7];
  regs[0]=intToBCD(date->second);
  regs[1]=intToBCD(date->minute);
  regs[2]=intToBCD(date->hour);
  regs[3]=intToBCD(date->mday);
  regs[4]=date->wday;
  regs[5]=intToBCD(date->month);
  regs[6]=intToBCD((uint8_t) (year));
  int result= writeRegs(RTC_PCF8523_REG_SECOND, regs, 7);
  if (result != RTC_U_SUCCESS) return false;
  return true;
}
/* ==================== 電圧低下関係 ==================== */
/*
 * secondレジスタの最上位bit(7bit)と
 * control3レジスタの下4bit BSF, BLF, BSIE, BLIEを返す
 */
int RTC_PCF8523_U::checkLowPower(void) {
  uint8_t control_3, second;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  if (0 > result ) return RTC_U_FAILURE;
  control_3 = control_3 & 0b1100;

  result = readRegs(RTC_PCF8523_REG_SECOND, &second, 1);
  if (0 > result ) return RTC_U_FAILURE;
  second = second & 0b10000000;
  if (second != 0) {
    control_3 = control_3 | 0b10000000;
  }
  return control_3;
}

/*
 * secondレジスタの最上位bit(7bit)と
 * control3レジスタの下4bit BSF, BLF, BSIE, BLIEを0クリア
 */
int RTC_PCF8523_U::clearPowerFlag(void){
  uint8_t control_3;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  if (0 > result ) return result;
  control_3 = control_3 & 0b11110011;
  result= writeRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  if (0 > result ) return result;
  uint8_t second;
  result = readRegs(RTC_PCF8523_REG_SECOND, &second, 1);
  if (0 > result ) return result;
  second = second & 0b01111111;
  result = writeRegs(RTC_PCF8523_REG_SECOND, &second, 1);
  return result;
}

/*
 * control3レジスタの上位3bit(bit5から7)を設定
 */
int RTC_PCF8523_U::setLowPower(uint8_t mode) {
  uint8_t control_3;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  if (0 > result ) return RTC_U_FAILURE;
  mode = mode & 0b11100011;
  control_3 = control_3 & 0b00011100;
  control_3 = control_3 | mode;
  result= writeRegs(RTC_PCF8523_REG_CONTROL_3, &control_3, 1);
  return result;
}
/* ==================== 割り込み関係 ==================== */
/*
 * 
 */
int RTC_PCF8523_U::checkInterupt(void) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0 > result ) return RTC_U_FAILURE;
  return (control_2>>3);
}

int RTC_PCF8523_U::clearInterupt(uint16_t type) {
  uint8_t val = (uint8_t) type;
  val = (~val) <<3;
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0 > result ) return RTC_U_FAILURE;
  control_2 = control_2 & val;
  result= writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  return result;
}

/* ==================== 時刻調整関係 ==================== */
/*
 * 
 */
int RTC_PCF8523_U::controlClockHalt(uint8_t mode) {
  uint8_t control_1;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0 > result ) return RTC_U_FAILURE;
  uint8_t mask;
  if (mode==1) {
    control_1 = control_1 & 0b11011111;
  } else if (mode==0) {
    control_1 = control_1 & 0b11011111;
    control_1 = control_1 | 0b00100000;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  result= writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  return result;
}

int RTC_PCF8523_U::clockHaltStatus(void) {
  uint8_t control_1;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0 > result ) return RTC_U_FAILURE;
  control_1 = control_1 & 0b00100000;
  if (control_1==0) return 0;
  return 1;
}

int RTC_PCF8523_U::controlClock(void) {
  uint8_t control_1 = 0b01011000;
  return writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
}

int RTC_PCF8523_U::setOscillator(uint8_t mode) {
  return writeRegs(RTC_PCF8523_REG_OFFSET, &mode, 1);
}

int RTC_PCF8523_U::getOscillator(void) {
  uint8_t offset;
  int result = readRegs(RTC_PCF8523_REG_OFFSET, &offset, 1);
  if (0 > result ) return RTC_U_FAILURE;
  return offset;
}

/* ==============================clock信号出力関係============================== */
/*
 * setClockOutMode(num, freq)を実行
 * controlClockOut(num,1)を実行
 */
int RTC_PCF8523_U::setClockOut(uint8_t num, uint8_t freq, int8_t pin) {
  int result = setClockOutMode(num, freq);
  if (0 > result ) return result;
  return controlClockOut(num, 1);
}

/*
 * num : 0限定
 * freq : 0から7
 * freqの値を3bit左シフトして，clockOutModeに保存
 * CLKOUT control register (0x0F)の3から5bitにclockOutModeの値を代入
 */
int RTC_PCF8523_U::setClockOutMode(uint8_t num, uint8_t freq) {
  if (num >= RTC_PCF8523_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  if (freq > 7) return RTC_U_ILLEGAL_PARAM;
  clockOutMode = freq << 3;
  uint8_t tmr_clkout_ctrl;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &tmr_clkout_ctrl, 1);
  if (0 > result) return result;
  tmr_clkout_ctrl = tmr_clkout_ctrl & 0b11000111;
  tmr_clkout_ctrl = tmr_clkout_ctrl | clockOutMode;
  return writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &tmr_clkout_ctrl, 1);
}

/*
 * OFFにする : CLKOUT control register (0x0F)の3から5bitを0b111にする
 * ONにする : clockOutModeの値をCLKOUT control register (0x0F)の3から5bitに書き戻す
 * num : 0限定
 * mode : 1 ON, 0 OFF
 */
int RTC_PCF8523_U::controlClockOut(uint8_t num, uint8_t mode) {
  if (num >= RTC_PCF8523_NUM_OF_CLOCKOUT) return RTC_U_ILLEGAL_PARAM;
  uint8_t tmr_clkout_ctrl;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &tmr_clkout_ctrl, 1);
  if (0 > result ) return RTC_U_FAILURE;
  if (mode==0) {
    tmr_clkout_ctrl = tmr_clkout_ctrl | 0b00111000;
  } else if (mode==1) {
    tmr_clkout_ctrl = tmr_clkout_ctrl & 0b11000111;
    tmr_clkout_ctrl = tmr_clkout_ctrl | clockOutMode;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  result = writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &tmr_clkout_ctrl, 1);
  return result;
}

/* ========================アラーム関係================================= */
int RTC_PCF8523_U::setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {
  if (num >= RTC_PCF8523_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t min_alarm_reg, hour_alarm_reg, day_alarm_reg, weekday_alarm_reg;
  if (60 > timing->minute) {
    min_alarm_reg = intToBCD(timing->minute);
  } else {
    min_alarm_reg = 0b10000000;
  }
  if (60 > timing->hour) {
    hour_alarm_reg = intToBCD(timing->hour);
  } else {
    hour_alarm_reg = 0b10000000;
  }
  if (31 >= timing->mday) {
    day_alarm_reg = intToBCD(timing->mday);
  } else {
    day_alarm_reg = 0b10000000;
  }
  if (7 > timing->wday) {
    weekday_alarm_reg = timing->wday;
  } else {
    weekday_alarm_reg = 0b10000000;
  }
  min_alarm_backup = min_alarm_reg;
  hour_alarm_backup = hour_alarm_reg;
  day_alarm_backup = day_alarm_reg;
  weekday_alarm_backup = weekday_alarm_reg;
  int result = writeRegs(RTC_PCF8523_REG_ALARM_MINUTE, &min_alarm_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_PCF8523_REG_ALARM_HOUR, &hour_alarm_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_PCF8523_REG_ALARM_DATE, &day_alarm_reg, 1);
  if (0 > result) return result;
  result = writeRegs(RTC_PCF8523_REG_ALARM_WEEKDAY, &weekday_alarm_reg, 1);
  if (0 > result) return result;
  result = setAlarmMode(num, mode);
  if (0 > result) return result;
  result = controlAlarm(num, 1);
  return result;
}

int RTC_PCF8523_U::setAlarmMode(uint8_t num, alarm_mode_t * mode) {
  if (num >= RTC_PCF8523_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  uint8_t control_1;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0 > result ) return RTC_U_FAILURE;
  if (0==mode->useInteruptPin) {
    control_1 = control_1 & 0b11111101;
  } else if (1==mode->useInteruptPin) {
    control_1 = control_1 | 0b00000010;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  result = writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  return result;
}

int RTC_PCF8523_U::controlAlarm(uint8_t num, uint8_t action) {
  if (num >= RTC_PCF8523_NUM_OF_ALARM) return RTC_U_ILLEGAL_PARAM;
  if (action==0) {
    int result = readRegs(RTC_PCF8523_REG_ALARM_MINUTE, &min_alarm_backup, 1);
    if (0>result) return result;
    result = readRegs(RTC_PCF8523_REG_ALARM_HOUR, &hour_alarm_backup, 1);
    if (0>result) return result;
    result = readRegs(RTC_PCF8523_REG_ALARM_DATE, &day_alarm_backup, 1);
    if (0>result) return result;
    result = readRegs(RTC_PCF8523_REG_ALARM_WEEKDAY, &weekday_alarm_backup, 1);
    if (0>result) return result;
    uint8_t AEN = 0b10000000;
    result = writeRegs(RTC_PCF8523_REG_ALARM_MINUTE, &AEN, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_HOUR, &AEN, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_DATE, &AEN, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_WEEKDAY, &AEN, 1);
  } else if (action==1) {
    int result = writeRegs(RTC_PCF8523_REG_ALARM_MINUTE, &min_alarm_backup, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_HOUR, &hour_alarm_backup, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_DATE, &day_alarm_backup, 1);
    if (0>result) return result;
    result = writeRegs(RTC_PCF8523_REG_ALARM_WEEKDAY, &weekday_alarm_backup, 1);
    if (0>result) return result;
    min_alarm_backup = 0b10000000;
    hour_alarm_backup = 0b10000000;
    day_alarm_backup = 0b10000000;
    weekday_alarm_backup = 0b10000000;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return RTC_U_SUCCESS;
}

/* ========================タイマ関係================================= */

int RTC_PCF8523_U::setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi) {
  switch(num) {
    case 0: return setBTimer(mode, multi);
    case 1: return setATimer(mode, multi);
    case 2: return setWTimer(mode, multi);
    case 3: return setSecTimer(mode);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_PCF8523_U::setTimerMode(uint8_t num, rtc_timer_mode_t * mode) {
  switch(num) {
    case 0: return setBTimerMode(mode);
    case 1: return setATimerMode(mode);
    case 2: return setWTimerMode(mode);
    case 3: return setSecTimerMode(mode);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_PCF8523_U::controlTimer(uint8_t num, uint8_t action) {
  switch(num) {
    case 0: return controlBTimer(action);
    case 1: return controlATimer(action);
    case 2: return controlWTimer(action);
    case 3: return controlSecTimer(action);
  }
  return RTC_U_ILLEGAL_PARAM;
}

int RTC_PCF8523_U::setBTimer(rtc_timer_mode_t * mode, uint16_t multi) {
  uint8_t timer_b_reg = (uint8_t) multi;
  int result = writeRegs(RTC_PCF8523_REG_TIMER_B, &timer_b_reg, 1);
  if (0>result) return result;
  result = setBTimerMode(mode);
  if (0>result) return result;
  return controlBTimer(1);
}

int RTC_PCF8523_U::setBTimerMode(rtc_timer_mode_t * mode) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b11011110;
  switch(mode->useInteruptPin) {
    case 0 : break;
    case 1 : control_2 = control_2 | 0b00000001;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t clk_out;
  result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  clk_out = clk_out & 0b10111111;
  switch(mode->repeat) {
    case 0 : break;
    case 1 : clk_out = clk_out | 0b01000000;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t tmr_b_freq = (0b00000111 & mode->pulse) << 4 ;
  tmr_b_freq = tmr_b_freq | (mode->interval & 0b00000111) ;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_B_FREQ_CONTROL, &tmr_b_freq, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  return result;
}

int RTC_PCF8523_U::controlBTimer(uint8_t action) {
  uint8_t clk_out;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  if (action==1) {
    clk_out = clk_out | 0b00000001;
  } else if (action==0) {
    clk_out = clk_out & 0b11111110;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
}

int RTC_PCF8523_U::setATimer(rtc_timer_mode_t * mode, uint16_t multi) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b10111111;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  uint8_t tmr_a = (uint8_t) multi;
  result = writeRegs(RTC_PCF8523_REG_TIMER_A, &tmr_a, 1);
  if (0>result) return result;
  result = setATimerMode(mode);
  if (0>result) return result;
  return controlATimer(1);
}

int RTC_PCF8523_U::setATimerMode(rtc_timer_mode_t * mode) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b10111101;
  switch(mode->useInteruptPin) {
    case 0 : break;
    case 1 : control_2 = control_2 | 0b00000010;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t clk_out;
  result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  clk_out = clk_out & 0b01111111;
  switch(mode->repeat) {
    case 0 : break;
    case 1 : clk_out = clk_out | 0b10000000;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t tmr_a_freq = 0b00000111 & mode->interval ; 
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_A_FREQ_CONTROL, &tmr_a_freq, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  return result;
}

int RTC_PCF8523_U::controlATimer(uint8_t action) {
  uint8_t clk_out;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  if (action==1) {
    clk_out = clk_out & 0b11111001;
    clk_out = clk_out | 0b00000010;
  } else if (action==0) {
    clk_out = clk_out & 0b11111001;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
}

int RTC_PCF8523_U::setWTimer(rtc_timer_mode_t * mode, uint16_t multi) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b01111111;
  uint8_t tmr_a = (uint8_t) multi;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_A, &tmr_a, 1);
  if (0>result) return result;
  result = setWTimerMode(mode);
  if (0>result) return result;
  return controlWTimer(1);
}

int RTC_PCF8523_U::setWTimerMode(rtc_timer_mode_t * mode) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b01111011;
  switch(mode->useInteruptPin) {
    case 0 : break;
    case 1 : control_2 = control_2 | 0b00000100;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t clk_out;
  result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  clk_out = clk_out & 0b01111111;
  switch(mode->repeat) {
    case 0 : break;
    case 1 : clk_out = clk_out | 0b10000000;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t tmr_a_freq = 0b00000111 & mode->interval ; 
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_A_FREQ_CONTROL, &tmr_a_freq, 1);
  if (0>result) return result;
  result = writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  return result;
}

int RTC_PCF8523_U::controlWTimer(uint8_t action) {
  uint8_t clk_out;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  if (action==1) {
    clk_out = clk_out & 0b11111001;
    clk_out = clk_out | 0b00000100;
  } else if (action==0) {
    clk_out = clk_out & 0b11111001;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
}

int RTC_PCF8523_U::setSecTimer(rtc_timer_mode_t * mode) {
  uint8_t control_2;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  control_2 = control_2 & 0b11101111;
  result = writeRegs(RTC_PCF8523_REG_CONTROL_2, &control_2, 1);
  if (0>result) return result;
  result = setSecTimerMode(mode);
  if (0>result) return result;
  return controlSecTimer(1);
}

int RTC_PCF8523_U::setSecTimerMode(rtc_timer_mode_t * mode) {
  uint8_t clk_out;
  int result = readRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  if (0>result) return result;
  clk_out = clk_out & 0b01111111;
  switch(mode->repeat) {
    case 0 : break;
    case 1 : clk_out = clk_out | 0b10000000;break;
    default : return RTC_U_ILLEGAL_PARAM;
  }
  result = writeRegs(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, &clk_out, 1);
  return result;
}

int RTC_PCF8523_U::controlSecTimer(uint8_t action) {
  uint8_t control_1;
  int result = readRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
  if (0>result) return result;
  if (action==1) {
    control_1 = control_1 & 0b11111011;
    control_1 = control_1 | 0b00000100;
  } else if (action==0) {
    control_1 = control_1 & 0b11111011;
  } else {
    return RTC_U_ILLEGAL_PARAM;
  }
  return writeRegs(RTC_PCF8523_REG_CONTROL_1, &control_1, 1);
}

/* ========================レジスタ読み書き================================= */
int RTC_PCF8523_U::readRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if (addr >= RTC_PCF8523_REG_NUM) return RTC_U_ILLEGAL_PARAM;
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

int RTC_PCF8523_U::writeRegs(uint8_t addr, uint8_t *val, uint8_t len) {
  if (val==NULL) return RTC_U_ILLEGAL_PARAM;
  if (addr >= RTC_PCF8523_REG_NUM) return RTC_U_ILLEGAL_PARAM;
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
void RTC_PCF8523_U::dumpReg(void) {
  String regName[]={
    "Control_1              ",
    "Control_2              ",
    "Control_3              ",
    "Second                 ",
    "Minute                 ",
    "Hour                   ",
    "Date                   ",
    "Weekday                ",
    "Month                  ",
    "Year                   ",
    "Alarm Minute           ",
    "Alarm Hour             ",
    "Alarm Date             ",
    "Alarm Weekday          ",
    "Offset                 ",
    "Timer Clockout Control ",
    "Timer A Freq Control   ",
    "Timer A                ",
    "Timer B Freq Control   ",
    "Timer B                "
  };
  Serial.println("register name           | addr | value");
  Serial.println("------------------------+------+------------------");
  for (int i=0; i< RTC_PCF8523_REG_NUM; i++) {
    if (i > 0x0F) {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("  | ");Serial.println(regValue[i],BIN);
    } else {
      Serial.print(regName[i]);Serial.print(" |  ");Serial.print(i,HEX);Serial.print("   | ");Serial.println(regValue[i],BIN);
    }
  }
}

bool RTC_PCF8523_U::backupRegValues(void){
  if (RTC_U_SUCCESS != readRegs(0, regValue, RTC_PCF8523_REG_NUM)) return false;
  return true;
}

bool RTC_PCF8523_U::checkRegValues(uint8_t num, uint8_t mask, uint8_t value){
  if (num >= RTC_PCF8523_REG_NUM)  return false;
  //
  uint8_t reg=regValue[num] & mask;
  if (reg==value) {
    return true;
  }
  return false;
}
#endif /* DEBUG */
/* ================================================================ */
