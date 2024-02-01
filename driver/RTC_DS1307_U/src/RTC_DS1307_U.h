#ifndef __RTC_DS1307_U_H__
#define __RTC_DS1307_U_H__

#include <Wire.h>
#include "arduino.h"
#include "RTC_U.h"

//#define USE_SQW_PIN
#define DEBUG


#define RTC_DS1307_DEFAULT_I2C_ADDR 0x68  // ＲＴＣのI2Cスレーブアドレス


#define RTC_DS1307_INIT_DELAY               1000      // 電源投入時のRTCの回路が安定するために必要な時間
#define RTC_DS1307_DEFAULT_YEAR             2001      // 電源投入時に設定する時間 (年)
#define RTC_DS1307_DEFAULT_MONTH            1         //                          (月)
#define RTC_DS1307_DEFAULT_DAY              1         //                          (日)
#define RTC_DS1307_DEFAULT_DAY_OF_WEEK      6         //                          (曜日) 土曜日
#define RTC_DS1307_DEFAULT_HOUR             0         //                          (時)
#define RTC_DS1307_DEFAULT_MIN              0         //                          (分)
#define RTC_DS1307_DEFAULT_SECOND           0         //                          (秒)

#define RTC_DS1307_NUM_OF_INTERUPT_PIN      0         // 割り込み信号ピン数   : 0種類
#define RTC_DS1307_NUM_OF_ALARM             0         // アラームの種類       : 1種類
#define RTC_DS1307_NUM_OF_TIMER             0         // タイマの種類         : 1種類
#define RTC_DS1307_NUM_OF_CLOCKOUT          1         // クロック出力端子の数 : 1つ
#define RTC_DS1307_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_DS1307_HAVE_CENTURY_BIT         false     // 年が桁あふれした場合の検出用bitの有無
#define RTC_DS1307_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_DS1307_DETECT_LOW_BATTERY       false     // 電圧低下を検出できるか?
#define RTC_DS1307_OSCILLATOR_CONTROL       false     // 時計の計時の進み方を調整できるか


#define RTC_DS1307_REG_SECOND               0x00      // 秒のデータ   (0～59)
#define RTC_DS1307_REG_MINUTE               0x01      // 分のデータ   (0～59)
#define RTC_DS1307_REG_HOUR                 0x02      // 時のデータ   (0～24) もしくは (0～12 + AM/PM)
#define RTC_DS1307_REG_WDAY                 0x03      // 曜日のデータ (0～7)
#define RTC_DS1307_REG_MDAY                 0x04      // 日のデータ   (0～31)
#define RTC_DS1307_REG_MONTH                0x05      // 月のデータ   (0～12)
#define RTC_DS1307_REG_YEAR                 0x06      // 年のデータ   (0～99)
#define RTC_DS1307_REG_CLOCK_CONTROL        0x07      // クロック出力制御用
#define RTC_DS1307_REG_NVRAM                0x08      // NVRAMの先頭アドレス

#define RTC_DS1307_REG_NUM 8

#define RTC_DS1307_OFF       0x00
#define RTC_DS1307_ON        0x80
#define RTC_DS1307_SQW_1HZ   0x10
#define RTC_DS1307_SQW_4kHZ  0x11
#define RTC_DS1307_SQW_8kHZ  0x12
#define RTC_DS1307_SQW_32kHZ 0x13

#define RTC_DS1307_SRAM_BASE_ADDR 0x08
#define RTC_DS1307_SRAM_SIZE 56

class RTC_DS1307_U : public RTC_Unified {
public:
  RTC_DS1307_U(TwoWire *theWire, int32_t rtcID = -1);
  bool  begin(bool init=true, uint32_t addr=RTC_DS1307_DEFAULT_I2C_ADDR);
  bool  setTime(date_t*);
  bool  getTime(date_t*);
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);
  void  getRtcInfo(rtc_u_info_t *info);
  int   clockHaltStatus(void);
  int   controlClockHalt(uint8_t mode);
  int   getSRAM(uint8_t addr, uint8_t *array, uint16_t len);
  int   setSRAM(uint8_t addr, uint8_t *array, uint16_t len);


#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif /* DEBUG */

private:
  TwoWire *_i2c_if;
  int     _i2c_addr;
  int32_t _rtcID;
  int   initRTC(void);
  bool  readReg(byte addr, byte *reg);
  bool  writeReg(byte addr, byte val);
#ifdef DEBUG
  uint8_t  regValue[RTC_DS1307_REG_NUM];
#endif
};

#endif /* __RTC_DS1307_U_H__ */
