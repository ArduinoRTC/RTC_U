#ifndef __RTC_PCF8523_U_H__
#define __RTC_PCF8523_U_H__

#include "RTC_U.h"
#include "Wire.h"

#define DEBUG

#define RTC_PCF8523_DEFAULT_YEAR           1         // 電源投入時に設定する時間 (年)
#define RTC_PCF8523_DEFAULT_MONTH          1         //                          (月)
#define RTC_PCF8523_DEFAULT_DAY            1         //                          (日)
#define RTC_PCF8523_DEFAULT_DAY_OF_WEEK    6         //                          (曜日) 土曜日
#define RTC_PCF8523_DEFAULT_HOUR           0         //                          (時)
#define RTC_PCF8523_DEFAULT_MIN            0         //                          (分)
#define RTC_PCF8523_DEFAULT_SECOND         0         //                          (秒)

#define RTC_PCF8523_NUM_OF_INTERUPT_PIN    1         // 割り込み信号ピン数   : 1種類
#define RTC_PCF8523_NUM_OF_ALARM           1         // アラームの種類       : 1種類
#define RTC_PCF8523_NUM_OF_TIMER           4         // タイマの種類         : 4種類
#define RTC_PCF8523_NUM_OF_CLOCKOUT        1         // クロック出力端子の数 : 2つ (制御できるのは1つ)
#define RTC_PCF8523_NUM_OF_YEAR_DIGITS     2         // RTCの年の桁数        : 2桁
#define RTC_PCF8523_HAVE_CENTURY_BIT       true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_PCF8523_HAVE_MILLISEC          false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_PCF8523_INDEPENDENT_SQW        false     // SQWピンが割り込みピンから独立しているか?
#define RTC_PCF8523_DETECT_LOW_BATTERY     true      // 電圧低下を検出できるか?
#define RTC_PCF8523_OSCILLATOR_CONTROL     true     // 時計の計時の進み方を調整できるか

#define RTC_PCF8523_REG_NUM                0x14
#define RTC_PCF8523_DEFAULT_ADRS           0x68

#define RTC_PCF8523_REG_CONTROL_1               0x00
#define RTC_PCF8523_REG_CONTROL_2               0x01
#define RTC_PCF8523_REG_CONTROL_3               0x02
#define RTC_PCF8523_REG_SECOND                  0x03
#define RTC_PCF8523_REG_MINUTE                  0x04
#define RTC_PCF8523_REG_HOUR                    0x05
#define RTC_PCF8523_REG_DATE                    0x06
#define RTC_PCF8523_REG_WEEKDAY                 0x07
#define RTC_PCF8523_REG_MONTH                   0x08
#define RTC_PCF8523_REG_YEAR                    0x09
#define RTC_PCF8523_REG_ALARM_MINUTE            0x0A
#define RTC_PCF8523_REG_ALARM_HOUR              0x0B
#define RTC_PCF8523_REG_ALARM_DATE              0x0C
#define RTC_PCF8523_REG_ALARM_WEEKDAY           0x0D
#define RTC_PCF8523_REG_OFFSET                  0x0E
#define RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL  0x0F
#define RTC_PCF8523_REG_TIMER_A_FREQ_CONTROL    0x10
#define RTC_PCF8523_REG_TIMER_A                 0x11
#define RTC_PCF8523_REG_TIMER_B_FREQ_CONTROL    0x12
#define RTC_PCF8523_REG_TIMER_B                 0x13

#define RTC_PCF8523_INIT_DELAY             2000

class RTC_PCF8523_U : public RTC_Unified {
public:
  RTC_PCF8523_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool init=true, uint8_t addr=RTC_PCF8523_DEFAULT_ADRS);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  void  getRtcInfo(rtc_u_info_t *info);
  int   checkLowPower(void);
  int   clearPowerFlag(void);
  int   setLowPower(uint8_t mode);
  int   checkInterupt(void);
  int   clearInterupt(uint16_t type);

  int   clockHaltStatus(void);
  int   controlClockHalt(uint8_t mode);
  int   controlClock(void);
  int   setOscillator(uint8_t mode);
  int   getOscillator(void);

  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);

  int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing);
  int   setAlarmMode(uint8_t num, alarm_mode_t * mode);
  int   controlAlarm(uint8_t num, uint8_t action);

  int setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi);
  int setTimerMode(uint8_t num, rtc_timer_mode_t * mode);
  int controlTimer(uint8_t num, uint8_t action);


#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif /* DEBUG */

private:
  int   setBTimer(rtc_timer_mode_t * mode, uint16_t multi);
  int   setBTimerMode(rtc_timer_mode_t * mode);
  int   controlBTimer(uint8_t action);
  int   setATimer(rtc_timer_mode_t * mode, uint16_t multi);
  int   setATimerMode(rtc_timer_mode_t * mode);
  int   controlATimer(uint8_t action);
  int   setWTimer(rtc_timer_mode_t * mode, uint16_t multi);
  int   setWTimerMode(rtc_timer_mode_t * mode);
  int   controlWTimer(uint8_t action);
  int   setSecTimer(rtc_timer_mode_t * mode);
  int   setSecTimerMode(rtc_timer_mode_t * mode);
  int   controlSecTimer(uint8_t action);
  TwoWire *i2c_if;
  uint8_t i2c_addr;
  int32_t rtcID;
  uint8_t clockOutMode;
  uint8_t min_alarm_backup, hour_alarm_backup, day_alarm_backup, weekday_alarm_backup;
#ifdef DEBUG
  uint8_t regValue[RTC_PCF8523_REG_NUM];
#endif
  int readRegs(uint8_t addr, uint8_t *val, uint8_t len);
  int writeRegs(uint8_t addr, uint8_t *val, uint8_t len);
};

#endif /* __RTC_PCF8523_U_H__ */
