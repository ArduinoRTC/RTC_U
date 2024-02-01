
#ifndef __RTC_EPSON_8564NB_U_H__
#define __RTC_EPSON_8564NB_U_H__

#include <Wire.h>
#include "arduino.h"
#include "RTC_U.h"

#define DEBUG

#define RTC_EPSON_8564NB_DEFAULT_ADRS             0B1010001 // ＲＴＣのI2Cスレーブアドレス
#define RTC_EPSON_8564NB_INIT_DELAY               1000      // 電源投入時のRTCの回路が安定するために必要な時間
#define RTC_EPSON_8564NB_DEFAULT_YEAR             2001      // 電源投入時に設定する時間 (年)
#define RTC_EPSON_8564NB_DEFAULT_MONTH            1         //                          (月)
#define RTC_EPSON_8564NB_DEFAULT_DAY              1         //                          (日)
#define RTC_EPSON_8564NB_DEFAULT_DAY_OF_WEEK      6         //                          (曜日) 土曜日
#define RTC_EPSON_8564NB_DEFAULT_HOUR             0         //                          (時)
#define RTC_EPSON_8564NB_DEFAULT_MIN              0         //                          (分)
#define RTC_EPSON_8564NB_DEFAULT_SECOND           0         //                          (秒)

#define RTC_EPSON_8564NB_NUM_OF_INTERUPT_PIN      1         // 割り込み信号ピン数   : 1種類
#define RTC_EPSON_8564NB_NUM_OF_ALARM             1         // アラームの種類       : 1種類
#define RTC_EPSON_8564NB_NUM_OF_TIMER             1         // タイマの種類         : 1種類
#define RTC_EPSON_8564NB_NUM_OF_CLOCKOUT          1         // クロック出力端子の数 : 1つ
#define RTC_EPSON_8564NB_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_EPSON_8564NB_HAVE_CENTURY_BIT         true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_EPSON_8564NB_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_EPSON_8564NB_INDEPENDENT_SQW          false     // SQWピンが割り込みピンから独立しているか?
#define RTC_EPSON_8564NB_DETECT_LOW_BATTERY       true      // 電圧低下を検出できるか?
#define RTC_EPSON_8564NB_OSCILLATOR_CONTROL       false     // 時計の計時の進み方を調整できるか

#define RTC_EPSON_8564NB_REG_CONTROL1             0x00
#define RTC_EPSON_8564NB_REG_CONTROL2             0x01
#define RTC_EPSON_8564NB_REG_SECONDS              0x02
#define RTC_EPSON_8564NB_REG_MINUTES              0x03
#define RTC_EPSON_8564NB_REG_HOURS                0x04
#define RTC_EPSON_8564NB_REG_DAYS                 0x05
#define RTC_EPSON_8564NB_REG_WEEKDAYS             0x06
#define RTC_EPSON_8564NB_REG_MONTHS               0x07
#define RTC_EPSON_8564NB_REG_YEARS                0x08
#define RTC_EPSON_8564NB_REG_ALARM_MINUITE        0x09
#define RTC_EPSON_8564NB_REG_ALARM_HOUR           0x0a
#define RTC_EPSON_8564NB_REG_ALARM_DAY            0x0b
#define RTC_EPSON_8564NB_REG_ALARM_WEEKDAY        0x0c
#define RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT       0x0d
#define RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL  0x0e
#define RTC_EPSON_8564NB_REG_ALARM_TIMER          0x0f

#define RTC_EPSON_8564NB_REG_NUM 16


/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class RTC_8564NB_U : public RTC_Unified {
public:
  RTC_8564NB_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool force=true, uint32_t addr=RTC_EPSON_8564NB_DEFAULT_ADRS);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing);
  int   setAlarmMode(uint8_t num, alarm_mode_t * mode);
  int   controlAlarm(uint8_t num, uint8_t action);
  int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi);
  int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode);
  int   controlTimer(uint8_t num, uint8_t action);
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);
  int   checkInterupt(void);
  int   clearInterupt(uint16_t type);
  void  getRtcInfo(rtc_u_info_t *info);
  int   checkLowPower(void);
  int   clearPowerFlag(void);
#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif

private:
  int getRegValue(uint8_t addr);
  bool setRegValue(uint8_t addr, uint8_t val);

  TwoWire *_i2c_if;
  int     _i2c_addr;
  int32_t _rtcID;
  int8_t  _clkoe_pin;
  uint8_t timer_multi;
  uint8_t alarm_minute, alarm_hour, alarm_date, alarm_wday;
#ifdef DEBUG
  uint8_t  regValue[RTC_EPSON_8564NB_REG_NUM];
#endif
};

#endif /* __RTC_EPSON_8564NB_U_H__ */
