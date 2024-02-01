
#ifndef __RTC_DS3234_U_H__
#define __RTC_DS3234_U_H__


#include "SparkFunDS3234RTC.h"
#include "RTC_U.h"

#define DEBUG

#define RTC_DS3234_DEFAULT_YEAR             2001      // 電源投入時に設定する時間 (年)
#define RTC_DS3234_DEFAULT_MONTH            1         //                          (月)
#define RTC_DS3234_DEFAULT_DAY              1         //                          (日)
#define RTC_DS3234_DEFAULT_DAY_OF_WEEK      6         //                          (曜日) 土曜日
#define RTC_DS3234_DEFAULT_HOUR             0         //                          (時)
#define RTC_DS3234_DEFAULT_MIN              0         //                          (分)
#define RTC_DS3234_DEFAULT_SECOND           0         //                          (秒)

#define RTC_DS3234_NUM_OF_INTERUPT_PIN      1         // 割り込み信号ピン数   : 1種類
#define RTC_DS3234_NUM_OF_ALARM             2         // アラームの種類       : 1種類
#define RTC_DS3234_NUM_OF_TIMER             0         // タイマの種類         : 0種類
#define RTC_DS3234_NUM_OF_CLOCKOUT          2         // クロック出力端子の数 : 2つ (制御できるのは1つ)
#define RTC_DS3234_NUM_OF_YEAR_DIGITS       2         // RTCの年の桁数        : 2桁
#define RTC_DS3234_HAVE_CENTURY_BIT         true      // 年が桁あふれした場合の検出用bitの有無
#define RTC_DS3234_HAVE_MILLISEC            false     // RTCがミリ秒を取り扱うことができるか?
#define RTC_DS3234_INDEPENDENT_SQW          false     // SQWピンが割り込みピンから独立しているか?
#define RTC_DS3234_DETECT_LOW_BATTERY       true      // 電圧低下を検出できるか?
#define RTC_DS3234_OSCILLATOR_CONTROL       true      // 時計の計時の進み方を調整できるか

#define RTC_DS3234_SRAM_SIZE 256

#ifdef DEBUG
#define RTC_DS3234_REG_NUM 0x14
#endif


/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class RTC_DS3234_U : public RTC_Unified {
public:
  RTC_DS3234_U(uint8_t cs, SPIClass *spi, int32_t rtcID=-1);
  RTC_DS3234_U(uint8_t cs, uint8_t miso, uint8_t mosi, uint8_t sck, int32_t rtcID=-1);
  bool  begin(bool init=true);
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
  int   setTemperatureFunction(uint8_t mode);
  int   getTemperatureFunctionState(void);
  int   controlTemperatureFunction(uint8_t action);
  int   setOscillator(uint8_t mode);
  int   getOscillator(void);
  int   getSRAM(uint8_t addr, uint8_t *array, uint16_t len);
  int   setSRAM(uint8_t addr, uint8_t *array, uint16_t len);

#ifdef DEBUG
  void  dumpReg(void);
  bool  backupRegValues(void);
  bool  checkRegValues(uint8_t num, uint8_t mask, uint8_t value);
#endif

private:
  uint8_t  _cs,  _mosi,  _miso,  _sck;
  SPIClass *_spi;
  int32_t _rtcID;
  SparkFun_DS3234 _ds3234;
  bool _alarm1_wday;
  bool _alarm2_wday;
#ifdef DEBUG
  uint8_t  regValue[RTC_DS3234_REG_NUM];
#endif
};

#endif /* __RTC_DS3234_U_H__ */
