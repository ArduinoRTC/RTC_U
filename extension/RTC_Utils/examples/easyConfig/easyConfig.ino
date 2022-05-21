/*
 * I2CのRTCを簡単に使う例
 */
//#define USE_RTC_4543
//#define USE_RTC8564NB
#define USE_DS1307
//#define USE_DS3231
//#define USE_DS3234
//#define USE_PCF8523
//#define USE_RV8803
//#define USE_RX8025
//#define USE_RX8900

#include "RTC_Utils.h"
RTC_CLASS rtc = RTC_CLASS(&Wire);

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
  Serial.println("");
}


void setup() {
  Serial.begin(9600) ;         // シリアル通信の初期化
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }
  delay(10000);

  if (rtc.begin(false)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while(1) ;                                              // 処理中断
  }

  Serial.println("Time of RTC setting is invalid.");
  date_t date;
  date.year=2019;
  date.month=5;
  date.mday=22;
  date.wday=3;
  date.hour=23;
  date.minute=59;
  date.second=20;
  date.millisecond=0;
  Serial.println("setting time.");
  printTime(&date);
  if (!rtc.setTime(&date)) {
    Serial.print("set time to RTC fail.");     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
}

void loop() {
  date_t date;
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  delay(1000) ;            // １秒後に繰り返す
}
