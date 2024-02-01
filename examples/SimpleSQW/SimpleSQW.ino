/*
 * SQW出力のサンプル : SQW出力の選択が可能なすべてのRTCでサポートしている周波数は1Hzのため，それを出力します．
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
 * 周波数とパラメータの変換
 */
enum {
  SQW_8564=0,
  SQW_DS1307,
  SQW_DS3231,
  SQW_DS3234,
  SQW_RV8803,
  SQW_RX8900
};

enum {
  FREQ_32KHZ=0,
  FREQ_8KHZ,
  FREQ_4KHZ,
  FREQ_1KHZ,
  FREQ_32HZ,
  FREQ_1HZ
};

#define UNSUPPORT_FREQ 0xFF
uint8_t freqParam[][FREQ_1HZ+1]={
  {0,UNSUPPORT_FREQ,UNSUPPORT_FREQ,1,2,3},
  {3,2,1,UNSUPPORT_FREQ,UNSUPPORT_FREQ,0},
  {UNSUPPORT_FREQ,3,2,1,UNSUPPORT_FREQ,0},
  {UNSUPPORT_FREQ,3,2,1,UNSUPPORT_FREQ,0},
  {0,UNSUPPORT_FREQ,UNSUPPORT_FREQ,1,UNSUPPORT_FREQ,2},
  {0,UNSUPPORT_FREQ,UNSUPPORT_FREQ,1,UNSUPPORT_FREQ,2}
};

/*
 * rtcの温度による時刻の進み具体の調整(TCXO)のパラメータ設定
 */
int setupSQW(uint8_t freq) {
  rtc_u_info_t rtcInfo;
  rtc.getRtcInfo(&rtcInfo);
  uint8_t rtcType;
  switch(rtcInfo.type) {
    case EPSON8564NB:rtcType=SQW_8564;break;
    case DS1307:rtcType=SQW_DS1307;break;
    case DS3231:rtcType=SQW_DS3231;break;
    case DS3234:rtcType=SQW_DS3234;break;
    case RV8803:rtcType=SQW_RV8803;break;
    case EPSON_RX8900:rtcType=SQW_RX8900;break;
    default:return RTC_U_ILLEGAL_PARAM;
  }
  if (freqParam[rtcType][freq]==UNSUPPORT_FREQ) return RTC_U_ILLEGAL_PARAM;
  return rtc.setClockOut(0, freqParam[rtcType][freq], FOE_PIN);
}

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
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  int flag=setupSQW(FREQ_1HZ);
  if (0>flag) {
    Serial.print("Error : setup SQW, error = ");Serial.println(flag);     // 初期化失敗
    while(1);
  }
  Serial.println("setup done.");
}

void loop() {
  delay(10000) ;            // １0秒後に繰り返す
}
