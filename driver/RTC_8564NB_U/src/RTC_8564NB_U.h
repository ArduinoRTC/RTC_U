/***************************************************************************
*  8564NB.h - RTC-8564NB(リアルタイムクロック)関数のインクルードファイル   *
*                                                                          *
* ======================================================================== *
*  VERSION DATE        BY                    CHANGE/COMMENT                *
* ------------------------------------------------------------------------ *
*  1.00    2012-01-15  きむ茶工房(きむしげ)  Create                        *
*  1.10    2012-09-13  きむ茶工房            ArduinoIDE1.0.1に対応して変更 *
***************************************************************************/
/*
 *  RTC用ライブラリも含めて，大幅変更したため，原型をあまり
 *  とどめていないが，きむ茶工房さんのものを引き継いでます．
 */

#ifndef __RTC_EPSON_8564NB_U_H__
#define __RTC_EPSON_8564NB_U_H__

#include <Wire.h>
#include "arduino.h"
#include "RTC_U.h"

//#define DEBUG

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




/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class RTC_8564NB_U : public RTC_Unified {
public:
  RTC_8564NB_U(TwoWire * theWire, int32_t rtcID=-1);
  bool  begin(uint32_t addr=RTC_EPSON_8564NB_DEFAULT_ADRS);
  bool  setTime(rtc_date_t*);
  bool  getTime(rtc_date_t*);
  int   setAlarm(uint8_t num, alarm_mode_t * mode, rtc_date_t* timing);
  int   setAlarmMode(uint8_t num, alarm_mode_t * mode);
  int   controlAlarm(uint8_t num, uint8_t action);
  int   setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi);
  int   setTimerMode(uint8_t num, timer_mode_t * mode);
  int   controlTimer(uint8_t num, uint8_t action);
  int   setClockOut(uint8_t num, uint8_t freq, int8_t pin=-1);
  int   setClockOutMode(uint8_t num, uint8_t freq);
  int   controlClockOut(uint8_t num, uint8_t mode);
  uint16_t   checkInterupt(void);
  bool  clearInterupt(uint16_t type);
  void  getRtcInfo(rtc_info_t *info);

private:
  int   initRTC(void);
  bool  readReg(byte addr, byte *reg);
  bool  writeReg(byte addr, byte val);
  int   SetTimer(byte sclk,byte count);
  int   StopTimer();
  int   SetAlarm(byte Hour,byte Min,byte mDay,byte wDay) ;
  int   StopAlarm();
  int   offAlarm();
  int   sTime(byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec) ;
  int   rTime(byte *tm) ;
  unsigned int bcd2bin(byte dt) ;
  unsigned int bin2bcd(unsigned int num) ;

  TwoWire *_i2c_if;
  int     _i2c_addr;
  int32_t _rtcID;
  int8_t  _clkoe_pin;
};

#endif /* __RTC_EPSON_8564NB_U_H__ */
