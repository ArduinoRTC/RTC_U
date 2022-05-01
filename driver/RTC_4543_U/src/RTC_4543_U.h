#ifndef __RTC_4543_U_H__
#define __RTC_4543_U_H__


#define DEBUG


#include "arduino.h"
#include "RTC_U.h"

#define RTC_EPSON_4543_INIT_DELAY               1000      // 電源投入時のRTCの回路が安定するために必要な時間
#define RTC_EPSON_4543_NUM_OF_INTERUPT_PIN      0         // 割り込み信号ピン数   : 1種類
#define RTC_EPSON_4543_NUM_OF_ALARM             0         // アラームの種類       : 1種類
#define RTC_EPSON_4543_NUM_OF_TIMER             0         // タイマの種類         : 1種類
#define RTC_EPSON_4543_NUM_OF_CLOCKOUT          1         // クロック出力端子の数 : 1つ
#define RTC_EPSON_4543_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_EPSON_4543_HAVE_CENTURY_BIT         false      // 年が桁あふれした場合の検出用bitの有無
#define RTC_EPSON_4543_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_EPSON_4543_INDEPENDENT_SQW          false      // SQWピンが割り込みピンから独立しているか?
#define RTC_EPSON_4543_DETECT_LOW_BATTERY       true      // 電圧低下を検出できるか?
#define RTC_EPSON_4543_OSCILLATOR_CONTROL       false     // 時計の計時の進み方を調整できるか

#define RTC_EPSON_4543_DEFAULT_YEAR             1         // 電源投入時に設定する時間     (年-2000)
#define RTC_EPSON_4543_DEFAULT_MONTH            1         //                          (月)
#define RTC_EPSON_4543_DEFAULT_DAY              1         //                          (日)
#define RTC_EPSON_4543_DEFAULT_DAY_OF_WEEK      1         //                          (曜日) 月曜日
#define RTC_EPSON_4543_DEFAULT_HOUR             0         //                          (時)
#define RTC_EPSON_4543_DEFAULT_MIN              0         //                          (分)
#define RTC_EPSON_4543_DEFAULT_SECOND           0         //                          (秒)

// Arduino Mega
//#define RTC_4543_DELAY  __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t")
#define RTC_4543_DELAY  delayMicroseconds(2)


/*
#if defined(RTC_4543_DELAY) && defined(F_CPU) && (F_CPU <= 1000000L)
  #undef RTC_4543_DELAY
  #define RTC_4543_DELAY __asm__("nop\n\t")
#endif
*/

class RTC_4543_U : public RTC_Unified {
public:
  RTC_4543_U(uint8_t _dataPin, uint8_t _clkPin, uint8_t _wrPin, uint8_t _cePin, uint8_t _fsel, int32_t _rtcID=-1);
  bool  begin(bool init=true, uint32_t addr=0);
  bool  setTime(date_t*);
  bool  getTime(date_t*);

  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);

  void  getRtcInfo(rtc_info_t *info);
  int   checkLowPower(void);
  int   clearPowerFlag(void);


private:
  uint8_t dataPin, clkPin, wrPin, cePin, foe, fsel;
  bool fdt;
  int32_t rtcID;
  uint8_t sec, minute, hour, wday, mday, month, year;
  void getAllData(void);
  void setAllData(void);
  void beginTransmission(bool mode_write);
  void endTransmission(void);
  uint8_t readData(bool half=false);
  void writeData(uint8_t data, bool half=false);
};

#endif /* __RTC_4543_U_H__ */
