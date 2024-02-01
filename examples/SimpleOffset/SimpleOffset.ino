
#include "config.h"

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



#define MAX_OFFSET true
#define MIN_OFFSET false

enum {
  OFFSET_DS3231,
  OFFSET_DS3234,
  OFFSET_PCF8523,
  OFFSET_RV8803,
  OFFSET_RX8025
};

#define UNSUPPORT_FREQ 0xFF
uint8_t offsetParam[][2]={
  {0xEF, 0x80},
  {0xEF, 0x80},
  {0b0111111, 0b1000000},
  {0b011111, 0b100000},
  {0x3F, 0x42}
};

int setupOffset(bool selection) {
  rtc_u_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  uint8_t offsetType;
  switch(rtcInfo.type) {
    case DS3231:offsetType=OFFSET_DS3231;break;
    case DS3234:offsetType=OFFSET_DS3234;break;
    case PCF8523:offsetType=OFFSET_PCF8523;break;
    case RV8803:offsetType=OFFSET_RV8803;break;
    case EPSON_RX8025:offsetType=OFFSET_RX8025;break;
    default:return RTC_U_ILLEGAL_PARAM;
  }
  uint8_t setting;
  if (selection==MAX_OFFSET) {
    Serial.println("max offset");
    setting=offsetParam[offsetType][0];
  } else {
    Serial.println("min offset");
    setting=offsetParam[offsetType][1];
  }
  return rtc.setOscillator(setting);
}

unsigned long lastTime;
bool currentSetting;
void setup() {
  Serial.begin(9600) ;         // シリアル通信の初期化
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }
  delay(10000);

  if (rtc.begin(true)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while(1) ;                                              // 処理中断
  }
  date_t date;
  date.year=2019;
  date.month=5;
  date.mday=22;
  date.wday=3;
  date.hour=23;
  //date.hour=12;
  date.minute=59;
  date.second=20;
  date.millisecond=0;
  Serial.println("setting time.");
  if (!rtc.setTime(&date)) {
    Serial.print("Error : set time to RTC fail.");     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  lastTime = rtc.convertDateToEpoch(date);
  Serial.print("epoch time = ");Serial.println(lastTime);
  currentSetting=MAX_OFFSET;
  int flag=setupOffset(currentSetting);
  if (0>flag) {
    Serial.print("Error : setup offset, error = ");Serial.println(flag);     // 初期化失敗
    while(1);
  }
  Serial.println("setup done.");
}


void loop() {
  delay(WAIT_TIME);
  date_t date;
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  unsigned long currentTime=rtc.convertDateToEpoch(date);
  Serial.print("epoch time = ");Serial.println(currentTime);
  long diff=currentTime-lastTime-DURATION;
  Serial.print("differential of epoch time in 100sec = ");Serial.println(diff);
  lastTime=currentTime;
  if (currentSetting==MAX_OFFSET) {
    currentSetting=MIN_OFFSET;
  } else {
    currentSetting=MAX_OFFSET;
  }
  int flag=setupOffset(currentSetting);
  if (0>flag) {
    Serial.print("Error : setup offset, error = ");Serial.println(flag);     // 初期化失敗
    while(1);
  }
}
