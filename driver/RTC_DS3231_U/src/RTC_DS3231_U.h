
#ifndef __RTC_DS3231_U_H__
#define __RTC_DS3231_U_H__

#include "RTC_U.h"
#include "Wire.h"

#define DEBUG

#define RTC_DS3231_DEFAULT_YEAR           1         // 電源投入時に設定する時間 (年)
#define RTC_DS3231_DEFAULT_MONTH          1         //                          (月)
#define RTC_DS3231_DEFAULT_DAY            1         //                          (日)
#define RTC_DS3231_DEFAULT_DAY_OF_WEEK    6         //                          (曜日) 土曜日
#define RTC_DS3231_DEFAULT_HOUR           0         //                          (時)
#define RTC_DS3231_DEFAULT_MIN            0         //                          (分)
#define RTC_DS3231_DEFAULT_SECOND         0         //                          (秒)

#define RTC_DS3231_NUM_OF_INTERUPT_PIN    1         // 割り込み信号ピン数   : 1種類
#define RTC_DS3231_NUM_OF_ALARM           2         // アラームの種類       : 1種類
#define RTC_DS3231_NUM_OF_TIMER           0         // タイマの種類         : 0種類
#define RTC_DS3231_NUM_OF_CLOCKOUT        2         // クロック出力端子の数 : 2つ (制御できるのは1つ)
#define RTC_DS3231_NUM_OF_YEAR_DIGITS     2         // RTCの年の桁数        : 2桁
#define RTC_DS3231_HAVE_CENTURY_BIT       true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_DS3231_HAVE_MILLISEC          false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_DS3231_INDEPENDENT_SQW        false     // SQWピンが割り込みピンから独立しているか?
#define RTC_DS3231_DETECT_LOW_BATTERY     true      // 電圧低下を検出できるか?
#define RTC_DS3231_OSCILLATOR_CONTROL     true     // 時計の計時の進み方を調整できるか

#define RTC_DS3231_INIT_DELAY             2000

#define RTC_DS3231_REG_NUM                0x13
#define RTC_DS3231_DEFAULT_ADRS           0x68

#define RTC_DS3231_REG_SECOND             0x00
#define RTC_DS3231_REG_MINUTE             0x01
#define RTC_DS3231_REG_HOUR               0x02
#define RTC_DS3231_REG_WEEKDAY            0x03
#define RTC_DS3231_REG_DATE               0x04
#define RTC_DS3231_REG_MONTH              0x05
#define RTC_DS3231_REG_YEAR               0x06
#define RTC_DS3231_REG_ALARM1_SECCOND     0x07
#define RTC_DS3231_REG_ALARM1_MINUTE      0x08
#define RTC_DS3231_REG_ALARM1_HOUR        0x09
#define RTC_DS3231_REG_ALARM1_DAY         0x0A
#define RTC_DS3231_REG_ALARM2_MINUTE      0x0B
#define RTC_DS3231_REG_ALARM2_HOUR        0x0C
#define RTC_DS3231_REG_ALARM2_DAY         0x0D
#define RTC_DS3231_REG_CONTROL            0x0E
#define RTC_DS3231_REG_STATUS             0x0F
#define RTC_DS3231_REG_AGING_OFFSET       0x10
#define RTC_DS3231_REG_TEMPERATURE_MSB    0x11
#define RTC_DS3231_REG_TEMPERATURE_LSB    0x12

/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class RTC_DS3231_U : public RTC_Unified {
public:
  RTC_DS3231_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(bool init=true, uint32_t addr=RTC_DS3231_DEFAULT_ADRS);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  int   setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing);
  int   setAlarmMode(uint8_t num, alarm_mode_t * mode);
  int   controlAlarm(uint8_t num, uint8_t action);
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);
  int   checkInterupt(void);
  int   clearInterupt(uint16_t type);
  void  getRtcInfo(rtc_u_info_t *info);
  int   checkLowPower(void);
  int   clearPowerFlag(void);
  float getTemperature(uint8_t mode);
  int   getTemperatureFunctionState(void);
  int   controlTemperatureFunction(uint8_t action);
  int   setOscillator(uint8_t mode);
  int   getOscillator(void);
#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif

private:
  TwoWire *i2c_if;
  uint8_t     i2c_addr;
  int32_t rtcID;
  bool alarm1_wday;
  bool alarm2_wday;
#ifdef DEBUG
  uint8_t  regValue[RTC_DS3231_REG_NUM];
#endif
  int readRegs(uint8_t addr, uint8_t *val, uint8_t len);
  int writeRegs(uint8_t addr, uint8_t *val, uint8_t len);
};

#endif /* __RTC_DS3231_U_H__ */
