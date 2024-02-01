#include "RTC_4543_U.h"

// AVR  - Arduino Mega
// SAM  - Arduino Due
// SAMD - Arduino MKR WiFi 1010
#define DATA_PIN 5
#define CLK_PIN 4
#define WR_PIN 12
#define CE_PIN 11
#define FSEL_PIN 13

RTC_4543_U rtc = RTC_4543_U(DATA_PIN, CLK_PIN, WR_PIN, CE_PIN, FSEL_PIN);
rtc_u_info_t rtcInfo;

/*
 * RTCの機種情報をプリントアウト
 */
void printRtcInfo(rtc_u_info_t * rtcInfo) {
  Serial.print("RTC type number            : ");Serial.println(rtcInfo->type);
  Serial.print("number of interupt pin     : ");Serial.println(rtcInfo->numOfInteruptPin);
  Serial.print("number of Alarm            : ");Serial.println(rtcInfo->numOfAlarm);
  Serial.print("number of Timer            : ");Serial.println(rtcInfo->numOfTimer);
  Serial.print("number of clock output pin : ");Serial.println(rtcInfo->numOfClockOut);
  Serial.print("number of year digit       : ");Serial.println(rtcInfo->numOfYearDigits);
  Serial.print("RTC have year overflow bit : ");
  if (rtcInfo->haveYearOverflowBit) {
    Serial.println("true");
  } else {
    Serial.println("false");
  }
  Serial.print("RTC have millisec function : ");
  if (rtcInfo->haveMilliSec) {
    Serial.println("true");
  } else {
    Serial.println("false");
  }
  Serial.print("SQW pin and interupt pin are independent : ");
  if (rtcInfo->independentSQW) {
    Serial.println("true");
  } else {
    Serial.println("false");
  }
}

/*
 * 時刻情報の出力
 */
void printTime(date_t * date){
  Serial.print(date->year);Serial.print("/");Serial.print(date->month);Serial.print("/");Serial.print(date->mday);Serial.print(" ");
  switch(date->wday) {
    case SUN : Serial.print("SUN");break;
    case MON : Serial.print("MON");break;
    case TUE : Serial.print("TUE");break;
    case WED : Serial.print("WED");break;
    case THU : Serial.print("THU");break;
    case FRI : Serial.print("FRI");break;
    case SAT : Serial.print("SAT");break;
  }
  Serial.print(" ");
  Serial.print(date->hour);Serial.print(":");Serial.print(date->minute);Serial.print(":");Serial.print(date->second);
  if (rtcInfo.haveMilliSec) { // RTCがミリ秒単位の出力を保つ場合(普通はないけど，将来のため?)
    Serial.print(" ");Serial.print(date->millisecond);
  }
  Serial.println("");
}

void setup() {
  Serial.begin(9600) ;         // シリアル通信の初期化
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }
  delay(10000);

  rtc.getRtcInfo(&rtcInfo);
  printRtcInfo(&rtcInfo);

  if (rtc.begin(false)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while(1) ;                                              // 処理中断
  }

  int power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
    if (power_flag>0) Serial.println("Caution: low power event detected.");
  }

  int reset_rst =rtc.clearPowerFlag();
  if (RTC_U_SUCCESS!=reset_rst) {
    Serial.print("Error: fail to clear power flag , retval = ");Serial.println(reset_rst);
  } else {
    Serial.println("Success : clear power flag.");
  }

  date_t date;
  date.year=2019;
  date.month=5;
  date.mday=22;
  date.wday=3;
  date.hour=15;
  date.minute=30;
  date.second=0;
  date.millisecond=0;

  Serial.println("setting time.");
  printTime(&date);

  /* RTCに現在時刻を設定 */
  if (!rtc.setTime(&date)) {
    Serial.print("set time to RTC fail.") ;     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }

  Serial.println("current time.");
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);

}

void loop() {
  date_t date;
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  delay(1000) ;            // １秒後に繰り返す
}
