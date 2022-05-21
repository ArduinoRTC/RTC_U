/*
 * 起動時にRTCに設定されている時刻が正しいものか否かを確認して，必要なら時刻を設定の上で毎秒現在時刻をシリアルに出力する
 * アラーム未サポート : EPSON 4543, DS1307
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
 * アラームの設定 : 現在時刻の2分後に設定
 */

bool setupAlarm() {
  date_t timing;
  if (!rtc.getTime(&timing)) {
    Serial.println("Error : can not get time data.");
    return false;
  }
  alarm_mode_t mode;
  mode.useInteruptPin=0;
  mode.type=0;
  timing.minute+=2;
  if (RTC_U_SUCCESS!=rtc.setAlarm(0,&mode,&timing)) {
    Serial.println("Error : can not set alarm.");
    return false;
  }
  return true;
}


/*
 * アラーム由来の割り込みが発生したか否かを確認
 */
#define ALARM_INTERUPT_ON  1
#define ALARM_INTERUPT_OFF 0
int checkAlarmInterupt_8564() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b10;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_RX8025() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b10;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_RX8900() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b001;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_DS3231() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b01;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_DS3234() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b01;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_PCF8523() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b00001;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt_RV8803() {
  int flag=rtc.checkInterupt();
  if (flag < 0) return flag;
  flag=flag&0b0010;
  if (0==flag) return ALARM_INTERUPT_OFF;
  return ALARM_INTERUPT_ON;
}

int checkAlarmInterupt(){
  rtc_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  switch(rtcInfo.type) {
    case EPSON8564NB:return checkAlarmInterupt_8564();
    case EPSON_RX8025:return checkAlarmInterupt_RX8025();
    case EPSON_RX8900:return checkAlarmInterupt_RX8900();
    case DS3231:return checkAlarmInterupt_DS3231();
    case DS3234:return checkAlarmInterupt_DS3234();
    case PCF8523:return checkAlarmInterupt_PCF8523();
    case RV8803:return checkAlarmInterupt_RV8803();
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
  int flag = rtc.clearInterupt(0xFF);
  if (0>flag) {
    Serial.print("Error : can not clear interupt flag, error No.=");Serial.println(flag);
    Serial.println("wait forever");
    while(1);
  }
  if (!setupAlarm()) {
    Serial.println("wait forever");
    while(1);
  }
}

void loop() {
  if (ALARM_INTERUPT_ON==checkAlarmInterupt()) {
    Serial.println("Alarm fire");
    date_t date;
    if (!rtc.getTime(&date)) {
      Serial.println("Error : can not get time data.");
      Serial.println("wait forever");
      while(1);
    }
    printTime(&date);
    
    int flag = rtc.clearInterupt(0xFF);
    if (0>flag) {
      Serial.print("Error : can not clear interupt flag, error No.=");Serial.println(flag);
      Serial.println("wait forever");
      while(1);
    }

    if (!setupAlarm()) {
      Serial.println("wait forever");
      while(1);
    }
  }
  delay(1000) ;            // １秒後に繰り返す
}
