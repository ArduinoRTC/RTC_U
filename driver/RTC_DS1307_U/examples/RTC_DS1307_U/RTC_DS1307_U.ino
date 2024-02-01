#include "RTC_DS1307_U.h"

#define CLOCK_OUT_PIN 3

/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_RX8900_U.hの以下の行を有効にしてください．
 *   #define DEBUG
 */

//#undef DEBUG  // 詳細なメッセージ出力が必要な場合はここをコメントアウト
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)

#ifdef DEBUG
#define TEST_LINE 3
#define TEST_COL 10

#define TEST_NOT_RUN 0
#define TEST_SUCCESS 1
#define TEST_FAIL 2

#define TEST_SETTIME_COLS 1
#define TEST_FREQ_COLS 4
#define TEST_HALT_COLS 4

enum {
  TEST_SETTIME,
  TEST_FREQ,
  TEST_HALT
};

enum{
  TEST_FREQ_OUTPUT_SETTING,
  TEST_FREQ_CHANGE_SETTING,
  TEST_FREQ_STOP_OUTPUT_1,
  TEST_FREQ_STOP_OUTPUT_2
};

enum{
  TEST_HALT_SETUP_HALT,
  TEST_HALT_CLOCK_MATCH,
  TEST_HALT_RESTART,
  TEST_HALT_CLOCK_MATCH2
};

uint8_t testResults[TEST_LINE][TEST_COL];

#endif /* DEBUG */

RTC_DS1307_U rtc = RTC_DS1307_U(&Wire);
//RTC_DS1307_U rtc = RTC_DS1307_U(&Wire1);


rtc_u_info_t rtcInfo;

/*
 * RTCの機種情報をプリントアウト
 */
void printRtcInfo(rtc_u_info_t * rtcInfo) {
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
 * 時刻の設定
 */
void testSetDate(void) {
  Serial.println("1. set date on RTC");
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
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    uint8_t sec, min, hour, mday, wday, month, year;
    //
    sec   = rtc.intToBCD(date.second);
    min   = rtc.intToBCD(date.minute);
    hour  = rtc.intToBCD(date.hour);
    wday  = date.wday;
    mday  = rtc.intToBCD(date.mday);
    month = rtc.intToBCD(date.month);
    year  = rtc.intToBCD(date.year-2000);
    //
    Serial.println(" -- setup time. -- ");
    Serial.println(" register             | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.print  (" Second (no check)| 0 | ");Serial.println(sec,BIN);
    Serial.print  (" Minute           | 1 | ");Serial.println(min,BIN);
    Serial.print  (" Hour             | 2 | ");Serial.println(hour,BIN);
    Serial.print  (" Day              | 3 | ");Serial.println(mday,BIN);
    Serial.print  (" Weekday          | 4 | ");Serial.println(wday,BIN);
    Serial.print  (" Month            | 5 | ");Serial.println(month,BIN);
    Serial.print  (" Year             | 6 | ");Serial.println(year,BIN);
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS1307_REG_MINUTE, 0b01111111, min)) {
      Serial.println("1 Error: Minute reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS1307_REG_HOUR, 0b00111111, hour)) {
      Serial.println("1 Error: Hour reg (0x02) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS1307_REG_WDAY, 0b00000111, wday)) {
      Serial.println("1 Error: Day reg (0x03) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS1307_REG_MDAY, 0b00111111, mday)) {
      Serial.println("1 Error: Weekday reg (0x04) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS1307_REG_MONTH, 0b00011111, month)) {
      Serial.println("1 Error: Month reg (0x05) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS1307_REG_YEAR, 0b11111111, year)) {
      Serial.println("1 Error: Year reg (0x06) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("1 Success");
      testResults[TEST_SETTIME][0]=TEST_SUCCESS;
    } else {
      Serial.println("1 Fail");
      testResults[TEST_SETTIME][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}


/*
 * 周波数信号出力
 */
void testFreq(void) {
  Serial.println("2. clock signal output test.");
  // 周波数信号を出力する設定
  Serial.println("2.1 setup clock signal output.");
  //
  int rst=rtc.setClockOut(0, 0, CLOCK_OUT_PIN); // 3番ピンを使う
  if (rst<0) {
    Serial.print("Error : setup clock out failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : start clock output.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" control     | 7 | xxx1xx00 ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_CLOCK_CONTROL, 0b00010011 , 0b00010000 )) {
      Serial.println("2.1 Error: control reg (0x7) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_FAIL ;
    } else {
      Serial.println("2.1 Success");
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数信号出力設定を変更
  Serial.println("2.2 change clock signal output setting.");
  //
  rst=rtc.setClockOutMode(0, 3);
  if (rst<0) {
    Serial.print("Error : change clock out setting failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : change clock out setting.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" control     | 7 | xxx1xx11 ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_CLOCK_CONTROL, 0b00010011 , 0b00010011 )) {
      Serial.println("2.2 Error: control reg (0x7) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_CHANGE_SETTING]=TEST_FAIL ;
    } else {
      Serial.println("2.2 Success");
      testResults[TEST_FREQ][TEST_FREQ_CHANGE_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数信号出力を止める
  Serial.println("2.3 stop clock signal output.");
  //
  rst=rtc.controlClockOut(0, 0);
  if (rst<0) {
    Serial.print("Error : stop clock out failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : stop clock out.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" control     | 7 | 0xx0xxxx ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_CLOCK_CONTROL, 0b10010000 , 0b00000000 )) {
      Serial.println("2.3 Error: control reg (0x7) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_1]=TEST_FAIL ;
    } else {
      Serial.println("2.3 Success");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_1]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数信号出力端子をHIGH?にする
  Serial.println("2.4 stop clock signal output.");
  //
  rst=rtc.controlClockOut(0, 1);
  if (rst<0) {
    Serial.print("Error : stop clock out failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : stop clock out.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" control     | 7 | 1xx0xxxx ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_CLOCK_CONTROL, 0b10010000 , 0b10000000 )) {
      Serial.println("2.4 Error: control reg (0x7) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_2]=TEST_FAIL ;
    } else {
      Serial.println("2.4 Success");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_2]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


}


void testHalt(void){
  Serial.println("3. clock halt test.");
  delay(3000);
  // クロックを止める設定を実施
  Serial.println("3.1 setup clock halt.");
  //
  int rst=rtc.controlClockHalt(0); // haltする
  if (rst<0) {
    Serial.print("Error : setup clock halt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : halt clock.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock halt -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" second      | 0 | 1xxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_SECOND, 0b10000000 , 0b10000000 )) {
      Serial.println("3.1 Error: second reg (0x0) is unmatch.");
      testResults[TEST_HALT][TEST_HALT_SETUP_HALT]=TEST_FAIL ;
    } else {
      Serial.println("3.1 Success");
      testResults[TEST_HALT][TEST_HALT_SETUP_HALT]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // クロックが実際に止まっているかを確認
  Serial.println("3.2 check clock.");
  date_t date1,date2;
  rtc.getTime(&date1);
  printTime(&date1);
  delay(10000);
  rtc.getTime(&date2);
  printTime(&date2);

#ifdef DEBUG
  bool flag=true;
  if (date1.second != date2.second) {
    flag=false;
    Serial.println("second is not same");
  }
  if (date1.minute != date2.minute) {
    flag=false;
    Serial.println("minute is not same");
  }
  if (date1.hour != date2.hour) {
    flag=false;
    Serial.println("hour is not same");
  }
  if (date1.wday != date2.wday) {
    flag=false;
    Serial.println("week day is not same");
  }
  if (date1.mday != date2.mday) {
    flag=false;
    Serial.println("date is not same");
  }
  if (date1.month != date2.month) {
    flag=false;
    Serial.println("month is not same");
  }
  if (date1.year != date2.year) {
    flag=false;
    Serial.println("year is not same");
  }
  if (flag==false) {
    Serial.println("3.2 fail");
    testResults[TEST_HALT][TEST_HALT_CLOCK_MATCH]=TEST_FAIL ;
  } else {
    Serial.println("3.2 success");
    testResults[TEST_HALT][TEST_HALT_CLOCK_MATCH]=TEST_SUCCESS ;
  }
#endif /* DEBUG */


  // クロックを再開
  Serial.println("3.3 restart clock.");
  //
  rst=rtc.controlClockHalt(1); // clockを動作させる
  if (rst<0) {
    Serial.print("Error : restart clock failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : restart clock.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock halt -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" second      | 0 | 0xxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_DS1307_REG_SECOND, 0b10000000 , 0b00000000 )) {
      Serial.println("3.3 Error: second reg (0x0) is unmatch.");
      testResults[TEST_HALT][TEST_HALT_RESTART]=TEST_FAIL ;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_HALT][TEST_HALT_RESTART]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // クロックが実際に再開したかを確認
  Serial.println("3.4 check restarting.");
  rtc.getTime(&date1);
  delay(63000);
  rtc.getTime(&date2);
  printTime(&date2);

#ifdef DEBUG
  flag=true;
  if (date1.second != date2.second) {
    flag=false;
    Serial.println("second is not same");
  }
  if (date1.minute != date2.minute) {
    flag=false;
    Serial.println("minute is not same");
  }
  if (date1.hour != date2.hour) {
    flag=false;
    Serial.println("hour is not same");
  }
  if (date1.wday != date2.wday) {
    flag=false;
    Serial.println("week day is not same");
  }
  if (date1.mday != date2.mday) {
    flag=false;
    Serial.println("date is not same");
  }
  if (date1.month != date2.month) {
    flag=false;
    Serial.println("month is not same");
  }
  if (date1.year != date2.year) {
    flag=false;
    Serial.println("year is not same");
  }
  if (flag==true) {
    Serial.println("3.4 fail");
    testResults[TEST_HALT][TEST_HALT_CLOCK_MATCH2]=TEST_FAIL ;
  } else {
    Serial.println("3.4 Success");
    testResults[TEST_HALT][TEST_HALT_CLOCK_MATCH2]=TEST_SUCCESS ;
  }
#endif /* DEBUG */

}

#ifdef DEBUG
/*
 * テスト結果の集計
 */
void calcResult(void){
  //
  Serial.println("========== check all test result =========");
  //
  int success=0, fail=0, not_run=0;

  for (int i=0; i< TEST_SETTIME_COLS ;i++) {
    if (testResults[TEST_SETTIME][i]==TEST_NOT_RUN) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_SETTIME][i]==TEST_FAIL) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_FREQ_COLS ;i++) {
    if (testResults[TEST_FREQ][i]==TEST_NOT_RUN) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_FREQ][i]==TEST_FAIL) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }


  for (int i=0; i< TEST_HALT_COLS ;i++) {
    if (testResults[TEST_HALT][i]==TEST_NOT_RUN) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_HALT][i]==TEST_FAIL) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  //
  Serial.println("========== summary of test result =========");
  //
  int total=not_run+success+fail;
  Serial.print(" success     : ");Serial.print(success);Serial.print("/");Serial.println(total);
  Serial.print(" fail        : ");Serial.print(fail);Serial.print("/");Serial.println(total);
  Serial.print(" not execute : ");Serial.print(not_run);Serial.print("/");Serial.println(total);

}
#endif /* DEBUG */


/*
 * 電源起動時とリセットの時だけのみ処理される関数(初期化と設定処理)
 */
void setup()
{
  Serial.begin(9600) ;                                      // シリアル通信の初期化
  while (!Serial) {
    ; // シリアルポートが開くのを待つ
  }

  delay(10000);

#ifdef DEBUG
  for (int i=0; i<TEST_LINE ; i++) {
    for (int j=0; j<TEST_COL ; j++) {
      testResults[i][j]=TEST_NOT_RUN;
    }
  }
#endif /* DEBUG */

  rtc.getRtcInfo(&rtcInfo);
  printRtcInfo(&rtcInfo);



  if (rtc.begin(false)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while(1) ;                                              // 処理中断
  }
#ifdef DEBUG
  rtc.backupRegValues();
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */

  Serial.println("==================  set time to rtc ==================");
  testSetDate();
  Serial.println("==================  clock signal output ==================");
  testFreq();
  Serial.println("==================  clock halt ==================");
  testHalt();

#ifdef DEBUG
  calcResult();
#endif /* DEBUG */
  Serial.println("==================  all function test done. ==================");
  delay(10000);
  Serial.println("==================  long run test  ==================");

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
#ifndef DEBUG
  printTime(&date);
#endif /* not DEBUG */
  delay(1000) ;            // １秒後に繰り返す
}
