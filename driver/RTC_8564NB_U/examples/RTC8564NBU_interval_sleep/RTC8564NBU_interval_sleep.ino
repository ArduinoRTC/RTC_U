#include "RTC_8564NB_U.h"
#include <avr/sleep.h>

//#define INT_PIN
#define INT_PIN_NO 18 // 割り込み信号用端子 (D18)
//#define USE_SQW

//#define STANDBY_MODE SLEEP_MODE_IDLE
//#define STANDBY_MODE SLEEP_MODE_ADC
//#define STANDBY_MODE SLEEP_MODE_PWR_SAVE
//#define STANDBY_MODE SLEEP_MODE_STANDBY
#define STANDBY_MODE SLEEP_MODE_PWR_DOWN

#define TIMER_INTERVAL 10

RTC_8564NB_U rtc = RTC_8564NB_U(&Wire);


bool intFlag=false;

void callBack(void){
  intFlag=true;
}

uint8_t intrupt_pin = INT_PIN_NO;


rtc_info_t rtcInfo;

/*
 * RTCの機種情報をプリントアウト
 */
void printRtcInfo(rtc_info_t * rtcInfo) {
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
 * 電源起動時とリセットの時だけのみ処理される関数(初期化と設定処理)
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
    while(1) ;                                              // 処理中断
  }

  /* 時刻データをRTCに登録するための変数定義 */
  rtc_date_t date;
  date.year=2019;
  date.month=5;
  date.mday=22;
  date.wday=3;
  date.hour=15;
  date.minute=30;
  date.second=0;
  date.millisecond=0;

  /* RTCに現在時刻を設定 */
  if (!rtc.setTime(&date)) {
    Serial.print("set time to RTC fail.") ;     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }

  if (rtcInfo.numOfTimer > 0) {
    /* タイマの設定 */
    timer_mode_t timer_mode;
    timer_mode.repeat=1;                // 繰り返し動作
    timer_mode.useInteruptPin=1;        // 割り込みピンを使う
    timer_mode.interval=2;              // 単位は秒
    rtc.setTimer(0, &timer_mode, TIMER_INTERVAL);   // 0番のタイマ割り込みをTIMER_INTERVAL(秒)単位で発生させる
    Serial.println("set timer done.");
  }

  if (rtcInfo.numOfInteruptPin > 0) {
    pinMode(intrupt_pin,INPUT_PULLUP) ;
    attachInterrupt(digitalPinToInterrupt(intrupt_pin),callBack,FALLING );
    Serial.println("setup interupt pin");
  }
  rtc.clearInterupt(0);
}

/*
 * 発生した割り込みの種別をプリントアウト
 */
void printInteruptFlag (uint16_t flag) {
  Serial.print("interupt type = 0b");Serial.println(flag,BIN);
}

/*
 * RTCの時刻情報の表示
 */
void printTime(rtc_date_t * date){
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

/*
 * 端末のCPUを寝かせる(低電力モードに設定する)処理
 */
void goodNight(int i) {
  Serial.println("  Good Night");
  delay(100);
  noInterrupts();
  set_sleep_mode(i);
  sleep_enable();
  interrupts();
  sleep_cpu();
  sleep_disable();
}

/*
 * メインループ
 */
void loop()
{
  rtc_date_t date;
  rtc.getTime(&date);
  printTime(&date);

  if ((intFlag)&&(rtcInfo.numOfInteruptPin > 0)) {
    intFlag=false;
    Serial.println("interupt is triggered");
    printInteruptFlag(rtc.checkInterupt());
    rtc.clearInterupt(0);
    printInteruptFlag(rtc.checkInterupt());

  }
  goodNight(STANDBY_MODE);// 割込みが発生したか？
  //delay(1000) ;            // １秒後に繰り返す
}
