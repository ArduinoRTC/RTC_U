
#define USE_NTP
#define USE_WIFI
#define USE_WIFI_NINA


#define WIFI_SSID "foo"
#define WIFI_PASS "bar"


#include "RTC_8564NB_U.h"

#ifdef USE_NTP
#ifdef USE_WIFI
#ifdef USE_WIFI_NINA
#include <WiFiNINA.h>
#else /* USE_WIFI_NINA */
#include <WiFi.h>
#endif /* USE_WIFI_NINA */
#include <WiFiUdp.h>
#else /* USE_WIFI */
#include <Ethernet.h>
#include <EthernetUdp.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0x0D };
#endif /* USE_WIFI */

#include <NTPClient.h>

/*
   ネットワーク関係の定数
*/
// DHCPでのアドレス取得失敗時の対策や，長時間経過後のアドレス再割当て等は対応していない
boolean useDhcp = true;                             // 固定IPで運用する場合は false に変更

/* 以下は固定IP運用の場合の変数なので適宜変更して使用すること */
IPAddress ip(192, 168, 0, 201);
IPAddress dnsServer(192, 168, 0, 1);
IPAddress gatewayAddress(192, 168, 0, 1);
IPAddress netMask(255, 255, 255, 0);

#ifdef USE_WIFI
int status = WL_IDLE_STATUS;
#endif /* USE_WIFI */

#ifdef USE_WIFI
WiFiUDP ntpUDP;
#else /* USE_WIFI */
EthernetUDP ntpUDP;
#endif /* USE_WIFI */

NTPClient timeClient(ntpUDP, 9 * 60 * 60); // JST
#endif /* USE_NTP */


RTC_8564NB_U rtc = RTC_8564NB_U(&Wire);


rtc_info_t rtcInfo;

/*
   RTCの機種情報をプリントアウト
*/
void printRtcInfo(rtc_info_t * rtcInfo) {
  Serial.print("RTC type number            : "); Serial.println(rtcInfo->type);
  Serial.print("number of interupt pin     : "); Serial.println(rtcInfo->numOfInteruptPin);
  Serial.print("number of Alarm            : "); Serial.println(rtcInfo->numOfAlarm);
  Serial.print("number of Timer            : "); Serial.println(rtcInfo->numOfTimer);
  Serial.print("number of clock output pin : "); Serial.println(rtcInfo->numOfClockOut);
  Serial.print("number of year digit       : "); Serial.println(rtcInfo->numOfYearDigits);
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
   電源起動時とリセットの時だけのみ処理される関数(初期化と設定処理)
*/
void setup()
{
  Serial.begin(9600) ;                                      // シリアル通信の初期化
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }

  rtc.getRtcInfo(&rtcInfo);
  printRtcInfo(&rtcInfo);

  if (rtc.begin()) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while (1) ;                                             // 処理中断
  }
  /* 時刻データをRTCに登録するための変数定義 */
  date_t dateTime;
#ifdef USE_NTP
#ifdef USE_WIFI
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);

    // wait 10 seconds for connection:
    delay(10000);
  }
#else /* USE_WIFI */
  Ethernet.begin(mac);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
#endif /* USE_WIFI */
  Serial.println(F("network setup done"));
#endif /* USE_NTP */

#ifdef USE_NTP
  unsigned long currentTime;
  timeClient.begin();
  timeClient.update();
  currentTime = timeClient.getEpochTime();
  rtc.convertEpochTime(&dateTime, currentTime);
#else /* USE_NTP */
  dateTime.year = 2019;
  dateTime.month = 5;
  dateTime.mday = 22;
  dateTime.wday = 3;
  dateTime.hour = 15;
  dateTime.minute = 30;
  dateTime.second = 0;
  dateTime.millisecond = 0;
#endif /* USE_NTP */

  /* RTCに現在時刻を設定 */
  if (!rtc.setTime(&dateTime)) {
    Serial.print("set time to RTC fail.") ;     // 初期化失敗
    while (1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
}


/*
   RTCの時刻情報の表示
*/
void printTime(date_t * date) {
  Serial.print(date->year); Serial.print("/"); Serial.print(date->month); Serial.print("/"); Serial.print(date->mday); Serial.print(" ");
  switch (date->wday) {
    case SUN : Serial.print("SUN"); break;
    case MON : Serial.print("MON"); break;
    case TUE : Serial.print("TUE"); break;
    case WED : Serial.print("WED"); break;
    case THU : Serial.print("THU"); break;
    case FRI : Serial.print("FRI"); break;
    case SAT : Serial.print("SAT"); break;
  }
  Serial.print(" ");
  Serial.print(date->hour); Serial.print(":"); Serial.print(date->minute); Serial.print(":"); Serial.print(date->second);
  if (rtcInfo.haveMilliSec) { // RTCがミリ秒単位の出力を保つ場合(普通はないけど，将来のため?)
    Serial.print(" "); Serial.print(date->millisecond);
  }
  Serial.println("");
}

/*
   メインループ
*/
void loop()
{
  date_t dateTime;
  unsigned long currentTime;

#ifdef USE_NTP
  timeClient.update();
  currentTime = timeClient.getEpochTime();
  rtc.convertEpochTime(&dateTime, currentTime);
  Serial.println("--- NTP ---");
  printTime(&dateTime);
#endif /* USE_NTP */

  rtc.getTime(&dateTime);
  Serial.println("--- RTC ---");
  printTime(&dateTime);

  delay(5000) ;            // １秒後に繰り返す
}
