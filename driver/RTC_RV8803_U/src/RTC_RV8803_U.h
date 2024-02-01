#ifndef __RTC_RV8803_U_H__
#define __RTC_RV8803_U_H__

#include "RTC_U.h"
#include "Wire.h"

#define DEBUG
#define DISABLE_EXT2_REGS

#define RTC_RV8803_DEFAULT_YEAR           1         // 電源投入時に設定する時間 (年)
#define RTC_RV8803_DEFAULT_MONTH          1         //                          (月)
#define RTC_RV8803_DEFAULT_DAY            1         //                          (日)
#define RTC_RV8803_DEFAULT_DAY_OF_WEEK    6         //                          (曜日) 土曜日
#define RTC_RV8803_DEFAULT_HOUR           0         //                          (時)
#define RTC_RV8803_DEFAULT_MIN            0         //                          (分)
#define RTC_RV8803_DEFAULT_SECOND         0         //                          (秒)

#define RTC_RV8803_NUM_OF_INTERUPT_PIN    1         // 割り込み信号ピン数   : 1種類
#define RTC_RV8803_NUM_OF_ALARM           1         // アラームの種類       : 1種類
#define RTC_RV8803_NUM_OF_TIMER           2         // タイマの種類         : 0種類
#define RTC_RV8803_NUM_OF_CLOCKOUT        1         // クロック出力端子の数 : 2つ (制御できるのは1つ)
#define RTC_RV8803_NUM_OF_YEAR_DIGITS     2         // RTCの年の桁数        : 2桁
#define RTC_RV8803_HAVE_CENTURY_BIT       true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_RV8803_HAVE_MILLISEC          false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_RV8803_INDEPENDENT_SQW        false     // SQWピンが割り込みピンから独立しているか?
#define RTC_RV8803_DETECT_LOW_BATTERY     true      // 電圧低下を検出できるか?
#define RTC_RV8803_OSCILLATOR_CONTROL     true      // 時計の計時の進み方を調整できるか


#define RTC_RV8803_REG_NUM                36

#define RTC_RV8803_REG_NUM_BASIC          16
#define RTC_RV8803_REG_NUM_EXTENTION_1    16

#define RTC_RV8803_REG_NUM_EXTENTION_2    2
#define RTC_RV8803_REG_NUM_EXTENTION_3    1
#define RTC_RV8803_REG_NUM_EXTENTION_4    1


#define RTC_RV8803_DEFAULT_ADRS           0x32

#define RTC_RV8803_REG_SECOND             0x00
#define RTC_RV8803_REG_MINUTE             0x01
#define RTC_RV8803_REG_HOUR               0x02
#define RTC_RV8803_REG_WEEKDAY            0x03
#define RTC_RV8803_REG_DATE               0x04
#define RTC_RV8803_REG_MONTH              0x05
#define RTC_RV8803_REG_YEAR               0x06
#define RTC_RV8803_REG_RAM                0x07
#define RTC_RV8803_REG_ALARM_MINUTE       0x08
#define RTC_RV8803_REG_ALARM_HOUR         0x09
#define RTC_RV8803_REG_ALARM_DAY          0x0A
#define RTC_RV8803_REG_TIMER_COUNTER_0    0x0B
#define RTC_RV8803_REG_TIMER_COUNTER_1    0x0C
#define RTC_RV8803_REG_EXTENSION          0x0D
#define RTC_RV8803_REG_FLAG               0x0E
#define RTC_RV8803_REG_CONTROL            0x0F
#define RTC_RV8803_REG_10MS               0x10
#define RTC_RV8803_REG_SECOND_2           0x11
#define RTC_RV8803_REG_MINUTE_2           0x12
#define RTC_RV8803_REG_HOUR_2             0x13
#define RTC_RV8803_REG_WEEKDAY_2          0x14
#define RTC_RV8803_REG_DATE_2             0x15
#define RTC_RV8803_REG_MONTH_2            0x16
#define RTC_RV8803_REG_YEAR_2             0x17
#define RTC_RV8803_REG_ALARM_MINUTE_2     0x18
#define RTC_RV8803_REG_ALARM_HOUR_2       0x19
#define RTC_RV8803_REG_ALARM_DAY_2        0x1A
#define RTC_RV8803_REG_TIMER_COUNTER_0_2  0x1B
#define RTC_RV8803_REG_TIMER_COUNTER_1_2  0x1C
#define RTC_RV8803_REG_EXTENSION_2        0x1D
#define RTC_RV8803_REG_FLAG_2             0x1E
#define RTC_RV8803_REG_CONTROL_2          0x1F
#define RTC_RV8803_REG_10MS_2             0x20
#define RTC_RV8803_REG_SECOND_3           0x21
#define RTC_RV8803_REG_OFFSET             0x2C
#define RTC_RV8803_REG_EVENT              0x2F

#define RTC_RV8803_SRAM_BASE_ADDR         0x07
#define RTC_RV8803_SRAM_SIZE              1

#define RTC_RV8803_INIT_DELAY             2000

class RTC_RV8803_U : public RTC_Unified {
public:
  RTC_RV8803_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool init=true, uint8_t addr=RTC_RV8803_DEFAULT_ADRS);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  void  getRtcInfo(rtc_u_info_t *info);
// 電源関係
  int   checkLowPower(void);
  int   clearPowerFlag(void);
// クロック信号出力
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);
// 時計の進み方の制御
  int   setOscillator(uint8_t mode);
  int   getOscillator(void);
  int   controlClock(void);
// 割り込み
  int   checkInterupt(void);
  int   clearInterupt(uint16_t type);
// アラーム
  int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing);
  int   setAlarmMode(uint8_t num, alarm_mode_t * mode);
  int   controlAlarm(uint8_t num, uint8_t action);
// タイマ
  int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi);
  int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode);
  int   controlTimer(uint8_t num, uint8_t action);
// 外部イベント
  int   setEvent(event_mode_t *mode);
  int   getEvent(void);
// SRAMアクセス
  int   getSRAM(uint8_t addr, uint8_t *array, uint16_t len);
  int   setSRAM(uint8_t addr, uint8_t *array, uint16_t len);

#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif /* DEBUG */
#ifdef DEBUG
  uint8_t intToWday(uint8_t i_wday);
  uint8_t wdayToInt(uint8_t wday);
#endif /* DEBUG */

private:
  uint8_t clkoe;
  TwoWire *i2c_if;
  uint8_t i2c_addr;
  int32_t rtcID;
  uint8_t alarm_min, alarm_hour, alarm_day;
#ifdef DEBUG
  uint8_t regValue[RTC_RV8803_REG_NUM];
#endif
#ifndef DEBUG
  uint8_t intToWday(uint8_t i_wday);
  uint8_t wdayToInt(uint8_t wday);
#endif /* DEBUG */
  int readRegs(uint8_t addr, uint8_t *val, uint8_t len);
  int writeRegs(uint8_t addr, uint8_t *val, uint8_t len);

  int   setIntervalTimer(rtc_timer_mode_t * mode);
  int   setCountdownTimer(rtc_timer_mode_t * mode, uint16_t multi);
  int   setIntervalTimerMode(rtc_timer_mode_t * mode);
  int   setCountdownTimerMode(rtc_timer_mode_t * mode);
  int   controlIntervalTimer(uint8_t action);
  int   controlCountdownTimer(uint8_t action);

};


#endif /* __RTC_RV8803_U_H__ */
