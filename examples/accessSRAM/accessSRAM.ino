/*
 * SRAM領域へのアクセス
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
 * SRAMへのアクセステスト
 */
#define DATA_MISMATCH -5
int checkSramAccess(void) {
  Serial.println("1. single access to SRAM");
  Serial.print("test addr :");
  for (int i=0; i< SRAM_SIZE; i++) {
    if (i!=0) Serial.print(" , ");
    Serial.print(i);
    uint8_t writeData, readData;
    if (0==(i%2)) writeData = 0b10101010;
    else writeData = 0b01010101;
    int result = rtc.setSRAM((uint8_t)i, &writeData, (uint8_t)1);
    if (0 > result) return result;
    result = rtc.getSRAM((uint8_t)i, &readData, (uint8_t)1);
    if (0 > result) return result;
    if (readData != writeData) return DATA_MISMATCH;
  }
  Serial.println("");
  Serial.println("2. bulk access to SRAM");
  uint8_t writeData[SRAM_SIZE], readData[SRAM_SIZE];
  for (int i=0; i< SRAM_SIZE; i++) {
    writeData[i]=~i;
  }
  int result = rtc.setSRAM((uint8_t)0, writeData, SRAM_SIZE);
  if (0 > result) return result;
  result = rtc.getSRAM((uint8_t)0, readData, SRAM_SIZE);
  if (0 > result) return result;
  Serial.print("test addr :");
  for (int i=0; i< SRAM_SIZE; i++) {
    if (i!=0) Serial.print(" , ");
    Serial.print(i);
    if (writeData[i] != readData[i]) return RTC_U_FAILURE;
  }
  Serial.println("");
  return RTC_U_SUCCESS;
}


void setup() {
  Serial.begin(9600) ;   // シリアル通信の初期化
  while (!Serial) {
    ;                    // シリアルポートが開くのを待つ
  }
  delay(10000);

  if (rtc.begin(false)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");       // 初期化失敗
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
    Serial.println("set time to RTC success.");        // 初期化成功
  }
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);

  Serial.println("==== sram access test : start =====");
  int flag=checkSramAccess();
  Serial.println("");
  if (0>flag) {
    Serial.print("Error : check SRAM access, error number - ");Serial.println(flag);
  } else {
    Serial.println("All SRAM access test success.");
  }
  Serial.println("==== sram access test : done =====");
}

void loop() {
  delay(10000) ;            // なにもしない
}
