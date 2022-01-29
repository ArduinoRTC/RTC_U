#include "RTC_DS3234_U.h"

#define INT_PIN_NO 3 // 割り込み信号用端子 (D3)
//#define INT_PIN
//#define USE_SQW

RTC_DS3234_U rtc = RTC_DS3234_U(10,&SPI);

#ifdef INT_PIN
bool intFlag=false;

void callBack(void){
  intFlag=true;
}


uint8_t intrupt_pin = INT_PIN_NO; 
#endif /* INT_PIN */
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
  date_t date;
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
  alarm_mode_t alarm_mode;       // アラーム動作モード定義用変数
  date_t alarm_timing;       // アラーム発生時刻用変数
  
  /* アラーム1の設定内容 */
#ifdef INT_PIN
  alarm_mode.useInteruptPin=1;   // 割り込みピンを使う
#else /* INT_PIN */
  alarm_mode.useInteruptPin=0;   // 割り込みピンは使わない
#endif /* INT_PIN */
  alarm_timing.hour=0xff;
  alarm_timing.minute=0xff;
  alarm_timing.second=30;
  alarm_timing.mday=0xff;
  alarm_timing.wday=0xff;
  /* アラーム1に書き込み */
  rtc.setAlarm(0, &alarm_mode, &alarm_timing);
  Serial.println("set Alarm1.. done.");

  /* アラーム2の設定内容 */
#ifdef INT_PIN
  alarm_mode.useInteruptPin=1;   // 割り込みピンを使う
#else /* INT_PIN */
  alarm_mode.useInteruptPin=0;   // 割り込みピンは使わない
#endif /* INT_PIN */    
  alarm_timing.hour=0xff;
  alarm_timing.minute=31;
  alarm_timing.second=0xff;
  alarm_timing.mday=0xff;
  alarm_timing.wday=0xff;
  /* アラーム2に書き込み */
  rtc.setAlarm(1, &alarm_mode, &alarm_timing);
  Serial.println("set Alarm2.. done.");

  if (rtcInfo.numOfTimer > 0) {
    /* タイマの設定 */
    timer_mode_t timer_mode;
    timer_mode.repeat=1;                // 繰り返し動作
    timer_mode.useInteruptPin=1;        // 割り込みピンを使う
    timer_mode.interval=2;              // 単位は秒
    rtc.setTimer(0, &timer_mode, 25);   // 0番のタイマ割り込みを25(秒)単位で発生させる
  }
  rtc.clearInterupt(0);
#if defined(USE_SQW) && !defined(INT_PIN)
  /* パルス出力機能の設定 */
  if ( 0 < rtcInfo.numOfClockOut ) {
    if (0==rtc.setClockOut(0, 3)) {       // 1Hzで出力 & 出力抑制用のピンは使わない
      Serial.println("Setting freq signal output function : 1Hz (without control pin)");
    } else {
      Serial.println("fail to control freq signal output function");
    }
  }
#endif /* USE_SQW && ! INT_PIN */
#ifdef INT_PIN
  if (rtcInfo.numOfInteruptPin > 0) {
    pinMode(intrupt_pin,INPUT_PULLUP) ;
    attachInterrupt(digitalPinToInterrupt(intrupt_pin),callBack,FALLING );
    Serial.println("setup interupt pin");
  }
#endif /* INT_PIN */
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

/*
 * メインループ
 */
void loop()
{
  date_t date;
  rtc.getTime(&date);
  printTime(&date);
  if (0 < rtc.checkInterupt()) {

#ifdef INT_PIN
    if (intFlag==true) {
      intFlag=false;
      Serial.println("interupt pin is used");
    }
#endif /* INT_PIN */
    Serial.println("interupt is triggered");
    printInteruptFlag(rtc.checkInterupt());
    rtc.clearInterupt(0);
    printInteruptFlag(rtc.checkInterupt());
  }

  if ((date.minute==0x32)&&(date.second==0x00)&&(rtcInfo.numOfInteruptPin > 0)) {  // ３２分になったら機能終了
    printInteruptFlag(rtc.checkInterupt());
    rtc.clearInterupt(0);
    printInteruptFlag(rtc.checkInterupt());
    rtc.controlTimer(0,0); // 0番のタイマをOFF(0)に変更
    printInteruptFlag(rtc.checkInterupt());
    rtc.controlTimer(0,0); // 0番のタイマをOFF(0)に変更
    printInteruptFlag(rtc.checkInterupt());
  }
  delay(1000) ;            // １秒後に繰り返す
}
