#ifndef __RTC_RX8900_U_H__
#define __RTC_RX8900_U_H__

#define DEBUG

#include <Wire.h>
#include "arduino.h"
#include "RTC_U.h"

#define RTC_EPSON_RX8900_DEFAULT_ADRS 0x32

#define RTC_EPSON_RX8900_INIT_DELAY               1000      // 電源投入時のRTCの回路が安定するために必要な時間
#define RTC_EPSON_RX8900_NUM_OF_INTERUPT_PIN      1         // 割り込み信号ピン数   : 1種類
#define RTC_EPSON_RX8900_NUM_OF_ALARM             1         // アラームの種類       : 1種類
#define RTC_EPSON_RX8900_NUM_OF_TIMER             2         // タイマの種類         : 2種類
#define RTC_EPSON_RX8900_NUM_OF_CLOCKOUT          1         // クロック出力端子の数 : 1つ
#define RTC_EPSON_RX8900_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_EPSON_RX8900_HAVE_CENTURY_BIT         true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_EPSON_RX8900_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_EPSON_RX8900_INDEPENDENT_SQW          true      // SQWピンが割り込みピンから独立しているか?
#define RTC_EPSON_RX8900_DETECT_LOW_BATTERY       true      // 電圧低下を検出できるか?
#define RTC_EPSON_RX8900_OSCILLATOR_CONTROL       false     // 時計の計時の進み方を調整できるか

//#define RTC_EPSON_RX8900_CONTROL1_REG_CFG        0b00100000  //0-23hours. INTRA is always LOW 

#define RTC_EPSON_RX8900_DEFAULT_YEAR             1         // 電源投入時に設定する時間     (年-2000)
#define RTC_EPSON_RX8900_DEFAULT_MONTH            1         //                          (月)
#define RTC_EPSON_RX8900_DEFAULT_DAY              1         //                          (日)
#define RTC_EPSON_RX8900_DEFAULT_DAY_OF_WEEK      1         //                          (曜日) 月曜日
#define RTC_EPSON_RX8900_DEFAULT_HOUR             0         //                          (時)
#define RTC_EPSON_RX8900_DEFAULT_MIN              0         //                          (分)
#define RTC_EPSON_RX8900_DEFAULT_SECOND           0         //                          (秒)

#define RTC_EPSON_RX8900_I2C_ACCESS_INTERVAL      1         // 1ms

#define RTC_EPSON_RX8900_SRAM_BASE_ADDR         0x07
#define RTC_EPSON_RX8900_SRAM_SIZE              1

#ifdef DEBUG
#define RTC_EPSON_RX8900_REG_NUM 18
#endif

class RTC_RX8900_U : public RTC_Unified {
public:
  RTC_RX8900_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool init=true, uint32_t addr=RTC_EPSON_RX8900_DEFAULT_ADRS);
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
  float getTemperature(uint8_t mode);
  int   setTemperatureFunction(uint8_t mode);
  int   controlClock(void);
  int   getSRAM(uint8_t addr, uint8_t *array, uint16_t len);
  int   setSRAM(uint8_t addr, uint8_t *array, uint16_t len);
#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif

private:
  int getRegValue(uint8_t addr);
  bool setRegValue(uint8_t addr, uint8_t val);
  bool setTimeReg(uint8_t sec, uint8_t minute, uint8_t hour, uint8_t wday, uint8_t day, uint8_t mon, uint8_t year);
  int setTTimerFlag(void);
  int setTTimer(rtc_timer_mode_t * mode, uint16_t multi);
  int setTTimerMode(rtc_timer_mode_t * mode);
  int controlTTimer(uint8_t action);
  int setUTimerFlag(void);
  int setUTimer(rtc_timer_mode_t * mode, uint16_t multi);
  int setUTimerMode(rtc_timer_mode_t * mode);
  int controlUTimer(uint8_t action);
  int setAlarmFlag(void);
  void clearRegisters(void);
  TwoWire *_i2c_if;
  int     _i2c_addr;
  int32_t _rtcID;
  int8_t foePin;
#ifdef DEBUG
  uint8_t  regValue[RTC_EPSON_RX8900_REG_NUM];
#endif
};


#endif /* __RTC_RX8900_U_H__ */
