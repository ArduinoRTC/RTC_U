/*
 * 起動時にRTCに設定されている時刻が正しいものか否かを確認して，必要なら時刻を設定の上で毎秒現在時刻をシリアルに出力する
 * 定周期タイマを実装しているRTC : RX8025, RX8900, RV8803
 *   PCF8523は定周期タイマを実装しているものの，1秒周期しかサポートしていないため，本プログラムでは対象にしていません．
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
 * タイマの設定
 */

int setupTimer_RX8025() {
  rtc_timer_mode_t mode;
  mode.useInteruptPin=0;  // 割り込みピン利用しない
  mode.interval=0b101;   // 1分周期(毎分0秒)
  return rtc.setTimer(0,&mode, 0xFF);
}

int setupTimer_RX8900() {
  rtc_timer_mode_t mode;
  mode.useInteruptPin=0;  // 割り込みピン利用しない
  mode.interval=1;        // 毎分0秒
  return rtc.setTimer(1,&mode, 0xFF);
}

int setupTimer_RV8803() {
  rtc_timer_mode_t mode;
  mode.interval=1;       // 毎分
  return rtc.setTimer(0,&mode, 0xFF);
}

int setupTimer() {
  rtc_u_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  switch(rtcInfo.type) {
    case EPSON_RX8025:return setupTimer_RX8025();
    case EPSON_RX8900:return setupTimer_RX8900();
    case RV8803:return setupTimer_RV8803();
  }
  return RTC_U_ILLEGAL_PARAM;
}

/*
 * アラーム由来の割り込みが発生したか否かを確認
 */
#define TIMER_INTERUPT_ON  1
#define TIMER_INTERUPT_OFF 0

int checkTimerInterupt_RX8025() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b100;
  if (0==flag) return TIMER_INTERUPT_OFF;
  return TIMER_INTERUPT_ON;
}

int checkTimerInterupt_RX8900() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b100;
  if (0==flag) return TIMER_INTERUPT_OFF;
  return TIMER_INTERUPT_ON;
}

int checkTimerInterupt_RV8803() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b01000;
  if (0==flag) return TIMER_INTERUPT_OFF;
  return TIMER_INTERUPT_ON;
}

int checkTimerInterupt(){
  rtc_u_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  switch(rtcInfo.type) {
    case EPSON_RX8025:return checkTimerInterupt_RX8025();
    case EPSON_RX8900:return checkTimerInterupt_RX8900();
    case RV8803:return checkTimerInterupt_RV8803();
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
  date.hour=15;
  date.minute=30;
  date.second=0;
  date.millisecond=0;
  Serial.println("setting time.");
  printTime(&date);
  if (!rtc.setTime(&date)) {
    Serial.println("set time to RTC fail.");     // 初期化失敗
    Serial.println("wait forever");
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
  if (!rtc.getTime(&date)) {
    Serial.println("Error : can not get time data.");
    Serial.println("wait forever");
    while(1);
  }
  printTime(&date);
  Serial.println("clear interupt flag.");
  int flag = rtc.clearInterupt(0xFF);
  if (0>flag) {
    Serial.println("Error : can not clear interupt flag.");
    Serial.println("wait forever");
    while(1);
  }
  Serial.println("setup timer.");
  if (RTC_U_SUCCESS!=setupTimer()) {
    Serial.println("fail to setup timer.");
    Serial.println("wait forever");
    while(1);
  }
}

void loop() {
  if (TIMER_INTERUPT_ON==checkTimerInterupt()) {
    Serial.println("Timer fire");
    int flag = rtc.clearInterupt(0xFF);
    if (0>flag) {
      Serial.println("Error : can not clear interupt flag.");
      Serial.println("wait forever");
      while(1);
    }
    date_t date;
    if (!rtc.getTime(&date)) {
      Serial.println("Error : can not get time data.");
      Serial.println("wait forever");
      while(1);
    }
    printTime(&date);
    if (RTC_U_SUCCESS!=setupTimer()) {
      Serial.println("fail to setup timer.");
      Serial.println("wait forever");
      while(1);
    }
  }
  delay(1000) ;            // １秒後に繰り返す
}
