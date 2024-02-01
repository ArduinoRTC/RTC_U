#ifndef __RTC_EPSON_RX8025_U_H__
#define __RTC_EPSON_RX8025_U_H__

#define DEBUG

#include <Wire.h>
#include "arduino.h"
#include "RTC_U.h"

#define RTC_EPSON_RX8025_DEFAULT_ADRS 0x32

#define RTC_EPSON_RX8025_INIT_DELAY               1000      // 電源投入時のRTCの回路が安定するために必要な時間
#define RTC_EPSON_RX8025_NUM_OF_INTERUPT_PIN      2         // 割り込み信号ピン数   : 1種類
#define RTC_EPSON_RX8025_NUM_OF_ALARM             2         // アラームの種類       : 1種類
#define RTC_EPSON_RX8025_NUM_OF_TIMER             1         // タイマの種類         : 1種類
#define RTC_EPSON_RX8025_NUM_OF_CLOCKOUT          1         // クロック出力端子の数 : 1つ
#define RTC_EPSON_RX8025_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_EPSON_RX8025_HAVE_CENTURY_BIT         true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_EPSON_RX8025_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_EPSON_RX8025_INDEPENDENT_SQW          true      // SQWピンが割り込みピンから独立しているか?
#define RTC_EPSON_RX8025_DETECT_LOW_BATTERY       true      // 電圧低下を検出できるか?
#define RTC_EPSON_RX8025_OSCILLATOR_CONTROL       true      // 計時用発振器の周波数を制御できるか
/*
 * 温度センサ : あり/なし
 * 温度制御一時停止 : 可/不可
 * 温度制御間隔設定 : 可/不可
 * 温度制御状態監視 : 可/不可
 */

#define RTC_EPSON_RX8025_CONTROL1_REG_CFG        0b00100000  //0-23hours. INTRA is always LOW 

#define RTC_EPSON_RX8025_DEFAULT_YEAR             1         // 電源投入時に設定する時間     (年-2000)
#define RTC_EPSON_RX8025_DEFAULT_MONTH            1         //                          (月)
#define RTC_EPSON_RX8025_DEFAULT_DAY              1         //                          (日)
#define RTC_EPSON_RX8025_DEFAULT_DAY_OF_WEEK      1         //                          (曜日) 月曜日
#define RTC_EPSON_RX8025_DEFAULT_HOUR             0         //                          (時)
#define RTC_EPSON_RX8025_DEFAULT_MIN              0         //                          (分)
#define RTC_EPSON_RX8025_DEFAULT_SECOND           0         //                          (秒)

#define RTC_EPSON_RX8025_I2C_ACCESS_INTERVAL      65         // 65micro sec (63マイクロ秒がアプリケーションマニュアルの推奨)
#define RTC_EPSON_RX8025_REG_NUM 16

#define RTC_EPSON_RX8025_REG_SECOND           0x0
#define RTC_EPSON_RX8025_REG_MINUTE           0x1
#define RTC_EPSON_RX8025_REG_HOUR             0x2
#define RTC_EPSON_RX8025_REG_WEEKDAY          0x3
#define RTC_EPSON_RX8025_REG_DAY              0x4
#define RTC_EPSON_RX8025_REG_MONTH            0x5
#define RTC_EPSON_RX8025_REG_YEAR             0x6
#define RTC_EPSON_RX8025_REG_DIGITAL_OFFSET   0x7
#define RTC_EPSON_RX8025_REG_ALARM_W_MINUTE   0x8
#define RTC_EPSON_RX8025_REG_ALARM_W_HOUR     0x9
#define RTC_EPSON_RX8025_REG_ALARM_W_WEEKDAY  0xA
#define RTC_EPSON_RX8025_REG_ALARM_D_MINUTE   0xB
#define RTC_EPSON_RX8025_REG_ALARM_D_HOUR     0xC
#define RTC_EPSON_RX8025_REG_RESERVED         0xD
#define RTC_EPSON_RX8025_REG_CONTROL1         0xE
#define RTC_EPSON_RX8025_REG_CONTROL2         0xF

class RTC_RX8025_U : public RTC_Unified {
public:
  RTC_RX8025_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool force=true, uint32_t addr=RTC_EPSON_RX8025_DEFAULT_ADRS);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing);
  int   controlAlarm(uint8_t num, uint8_t action);
  int   setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi);
  int   setTimerMode(uint8_t num, rtc_timer_mode_t * mode);
  int   controlTimer(uint8_t num, uint8_t action);
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   controlClockOut(uint8_t num, uint8_t mode);
  int   checkInterupt(void);
  int   clearInterupt(uint16_t type);
  int   setLowPower(uint8_t mode);
  void  getRtcInfo(rtc_u_info_t *info);
  int   checkLowPower(void);
  int   clearPowerFlag(void);
  int   setOscillator(uint8_t mode);
  int   getOscillator(void);

#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif /* DEBUG */

private:
  int getRegValue(uint8_t addr);
  bool setRegValue(uint8_t addr, uint8_t val);
  bool setTimeReg(uint8_t sec, uint8_t minute, uint8_t hour, uint8_t wday, uint8_t day, uint8_t mon, uint8_t year);
  bool checkYearOverflow(uint8_t m);
  TwoWire *_i2c_if;
  int     _i2c_addr;
  int32_t _rtcID;
  uint8_t foePin;
  uint8_t timerSetting;
#ifdef DEBUG
  uint8_t  regValue[RTC_EPSON_RX8025_REG_NUM];
#endif
};


#endif /* __RTC_EPSON_RX8025_U_H__ */
