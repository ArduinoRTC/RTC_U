/*
 * TCXO(温度に応じた時刻の進み方の調整)機能のパラメータを設定とRTCの温度の取得
 * DS3231はTCXO機能がないため，温度の取得のみ
 */
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

/*
 * rtcの温度による時刻の進み具体の調整(TCXO)のパラメータ設定
 */

int setupTCXO(void) {
  rtc_u_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  switch(rtcInfo.type) {
    case EPSON_RX8900:return rtc.setTemperatureFunction(3); // 30秒間隔
    case DS3231:return RTC_U_SUCCESS;  // DS3231は温度は取れるが，TCXO機能がないため無視
    case DS3234:return rtc.setTemperatureFunction(0); // 64秒
  }
  return RTC_U_ILLEGAL_PARAM;
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
  printTime(&date);
  if (!rtc.setTime(&date)) {
    Serial.print("Error : set time to RTC fail.");     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  float temperature = rtc.getTemperature(TEMP_UNIT);
  Serial.print("temperature = ");Serial.println(temperature);
  int flag=setupTCXO();
  if (0>flag) {
    Serial.print("Error : setup TCXO");     // 初期化失敗
    while(1);
  }
  Serial.println("setup done.");
}

void loop() {
  float temperature = rtc.getTemperature(TEMP_UNIT);
  Serial.print("temperature = ");Serial.println(temperature);
  date_t date;
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  delay(10000) ;            // １0秒後に繰り返す
}
