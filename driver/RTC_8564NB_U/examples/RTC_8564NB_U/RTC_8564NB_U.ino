#include "RTC_8564NB_U.h"

#define CLOCK_OUT_PIN 3 // clock周波数信号出力をコントロールする端子(D3)

/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_8564NB_U.hの以下の行を有効にしてください．
 *   #define DEBUG
 */

//#undef DEBUG  // 詳細なメッセージ出力が必要な場合はここをコメントアウト
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)



#ifdef DEBUG
#define TEST_LINE 4
#define TEST_COL 7
#define TEST_NOT_RUN 0
#define TEST_SUCCESS 1
#define TEST_FAIL 2
#define TEST_SETTIME_COLS 1
#define TEST_FREQ_COLS 3
#define TEST_TIMER_COLS 7
#define TEST_ALARM_COLS 5
uint8_t testResults[TEST_LINE][TEST_COL];

enum {
  TEST_SETTIME,
  TEST_FREQ,
  TEST_TIMER,
  TEST_ALARM
};

enum{
  TEST_FREQ_OUTPUT_SETTING,
  TEST_FREQ_CHANGE_SETTING,
  TEST_FREQ_STOP_OUTPUT
};


uint8_t maskFreq[]={
  0b10000011,
  0b10000011,
  0b10000000
};

uint8_t valueFreq[]={
  0b10000010,
  0b10000011,
  0b00000000
};

enum{
  TEST_TIMER_SETUP,
  TEST_TIMER_CHECK_FLAG_1,
  TEST_TIMER_CLEAR_FLAG_1,
  TEST_TIMER_CHANGE_MODE,
  TEST_TIMER_CHECK_FLAG_2,
  TEST_TIMER_CLEAR_FLAG_2,
  TEST_TIMER_STOP
};

enum{
  TEST_ALARM_SETUP,
  TEST_ALARM_CHECK_FLAG,
  TEST_ALARM_CLEAR_FLAG,
  TEST_ALARM_CHANGE_MODE,
  TEST_ALARM_STOP
};

#endif /* DEBUG */


RTC_8564NB_U rtc = RTC_8564NB_U(&Wire);



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
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.print  (" Second (no check)| 2 | ");Serial.println(sec,BIN);
    Serial.print  (" Minute           | 3 | ");Serial.println(min,BIN);
    Serial.print  (" Hour             | 4 | ");Serial.println(hour,BIN);
    Serial.print  (" Day              | 5 | ");Serial.println(mday,BIN);
    Serial.print  (" Weekday          | 6 | ");Serial.println(wday,BIN);
    Serial.print  (" Month            | 7 | ");Serial.println(month,BIN);
    Serial.print  (" Year             | 8 | ");Serial.println(year,BIN);
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_MINUTES, 0b01111111, min)) {
      Serial.println("1 Error: Minute reg (0x03) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_HOURS, 0b00111111, hour)) {
      Serial.println("1 Error: Hour reg (0x04) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_DAYS, 0b00111111, mday)) {
      Serial.println("1 Error: Day reg (0x05) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_WEEKDAYS, 0b00000111, wday)) {
      Serial.println("1 Error: Weekday reg (0x06) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_MONTHS, 0b00011111, month)) {
      Serial.println("1 Error: Month reg (0x07) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_YEARS, 0b11111111, year)) {
      Serial.println("1 Error: Year reg (0x08) is unmatch.");
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
  int rst=rtc.setClockOut(0, 0b10, CLOCK_OUT_PIN); // 3番ピンを使う
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
    Serial.println(" -- clock signal setting. -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" Clkout freq | D | 1xxxxx10 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT, maskFreq[TEST_FREQ_OUTPUT_SETTING], valueFreq[TEST_FREQ_OUTPUT_SETTING])) {
      Serial.println("2.1 Error: Clkout freq reg (0xD) is unmatch.");
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
  rst=rtc.setClockOutMode(0, 0b11);
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
    Serial.println(" -- clock signal setting. -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" Clkout freq | D | 1xxxxx11 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT, maskFreq[TEST_FREQ_CHANGE_SETTING], valueFreq[TEST_FREQ_CHANGE_SETTING])) {
      Serial.println("2.2 Error: Clkout freq reg (0xD) is unmatch.");
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
    Serial.println(" -- clock signal setting. -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ------------+---+--------- ");
    Serial.println(" Clkout freq | D | 0xxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_CLOCKOUT, maskFreq[TEST_FREQ_STOP_OUTPUT], valueFreq[TEST_FREQ_STOP_OUTPUT])) {
      Serial.println("2.3 Error: Clkout freq reg (0xD) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT]=TEST_FAIL ;
    } else {
      Serial.println("2.3 Success");
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
}

/*
 * 割り込み情報のプリントアウト
 */
void printInterupt(int flag) {
  uint8_t alarm = (uint8_t) ((0b1000 & flag)>>3);
  uint8_t timer = (uint8_t) ((0b100&flag)>>2);
  Serial.print("TIMER = ");Serial.print(timer);Serial.print(", ALARM = ");Serial.println(alarm);
}
/*
 * タイマテスト
 */
void testTimer(void) {
  Serial.println("3. timer test.");
  // タイマ設定
  Serial.println("3.1 setup timer.");
  // 設定内容
  rtc_timer_mode_t mode;
  mode.interval       = 0b01;
  mode.repeat         = 0;
  mode.useInteruptPin = 1;
  uint8_t multi = 0b00101010;
  // TD1 , TD0 : 0 , 1 (64Hz) mode.interval = 0b01
  // TI/TP bit : 0 ( once )     mode.repeat = 0
  // TIE bit : 1  mode.useInteruptPin = 1
  // multi : 42 0b00101010
  //
  int rst=rtc.setTimer(0, &mode, multi);
  if (rst<0) {
    Serial.print("Error : timer setup failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : timer setting.");
  }
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | 0x00x0x1");
    Serial.println(" Timer control    | E | 1xxxxx01");
    Serial.println(" Timer (no check) | F | 00101010");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b10110101, 0b00000001)) {
      Serial.println("3.1 Error: Control 2 reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL, 0b10000011, 0b10000001)) {
      Serial.println("3.1 Error: Timer control reg (0x0E) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.1 Success");
      testResults[TEST_TIMER][TEST_TIMER_SETUP]=TEST_SUCCESS;
    } else {
      Serial.println("3.1 Fail");
      testResults[TEST_TIMER][TEST_TIMER_SETUP]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // 時間待ち
  delay(4000);
  // 割り込みフラグの確認
  Serial.println("3.2 check interupt flag.");
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print("Error : check interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : check interupt.");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxxx1xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00000100, 0b00000100)) {
      Serial.println("3.2 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_CHECK_FLAG_1]=TEST_FAIL;
    } else {
      Serial.println("3.2 Success");
      testResults[TEST_TIMER][TEST_TIMER_CHECK_FLAG_1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みフラグのクリア
  Serial.println("3.3 clear interupt flag.");
  rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print("Error : clear interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : clear interupt.");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxxx0xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00000100, 0b00000000)) {
      Serial.println("3.3 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_CLEAR_FLAG_1]=TEST_FAIL;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_TIMER][TEST_TIMER_CLEAR_FLAG_1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマ設定変更
  Serial.println("3.4 timer mode change.");
  // 設定内容
  mode.interval       = 0b10;
  mode.repeat         = 0;
  mode.useInteruptPin = 1;
  // TD1 , TD0 : 1 , 0 (1Hz) mode.interval = 0b10
  // TI/TP bit : 0 ( once )     mode.repeat = 0
  // TIE bit : 1  mode.useInteruptPin = 1
  rst=rtc.setTimerMode(0, &mode);
  if (rst<0) {
    Serial.print("Error : change timer mode failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : change timer mode.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxx0xxx1");
    Serial.println(" Timer control    | E | 1xxxxx10");
    Serial.println(" Timer (no check) | F | 00101010");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00010001, 0b00000001)) {
      Serial.println("3.4 Error: Control 2 reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL, 0b10000011, 0b10000010)) {
      Serial.println("3.4 Error: Timer control reg (0x0E) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.4 Success");
      testResults[TEST_TIMER][TEST_TIMER_CHANGE_MODE]=TEST_SUCCESS;
    } else {
      Serial.println("3.4 Fail");
      testResults[TEST_TIMER][TEST_TIMER_CHANGE_MODE]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 時間待ち
  delay(60000);

  // 割り込みフラグ確認
  Serial.println("3.5 check interupt flag.");
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print("Error : check interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : check interupt .");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxxx1xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00000100, 0b00000100)) {
      Serial.println("3.5 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_CHECK_FLAG_2]=TEST_FAIL;
    } else {
      Serial.println("3.5 Success");
      testResults[TEST_TIMER][TEST_TIMER_CHECK_FLAG_2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みフラグクリア
  Serial.println("3.6 clear interupt flag.");
  rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print("Error : clear interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : clear interupt .");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxxx0xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00000100, 0b00000000)) {
      Serial.println("3.6 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_CLEAR_FLAG_2]=TEST_FAIL;
    } else {
      Serial.println("3.6 Success");
      testResults[TEST_TIMER][TEST_TIMER_CLEAR_FLAG_2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // タイマ停止
  Serial.println("3.7 stop timer.");
  rst = rtc.controlTimer(0,0);
  if (rst<0) {
    Serial.print("Error : stop timer failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : stop timer .");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting.  -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Timer control    | E | 0xxxxxxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_TIMER_CONTROL, 0b10000000, 0b00000000)) {
      Serial.println("3.7 Error: Timer control reg (0x0E) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_STOP]=TEST_FAIL;
    } else {
      Serial.println("3.7 Success");
      testResults[TEST_TIMER][TEST_TIMER_STOP]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * アラームのテスト
 */
void testAlarm(void) {
  Serial.println("4. Alarm test.");
  // アラーム設定
  Serial.println("4.1 setup alarm.");
  alarm_mode_t mode;
  mode.useInteruptPin=1;
  date_t timing;
  rtc.getTime(&timing);
  printTime(&timing);
  // 設定内容 (2分後に設定:分以外は無視)
  timing.minute+=2;
  timing.hour=0xff;
  timing.mday=0xff;
  timing.wday=0xff;
  int rst=rtc.setAlarm(0, &mode, &timing);
  if (rst<0) {
    Serial.print("Error : set alarm failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : set alarm.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    uint8_t min = rtc.intToBCD(timing.minute);
    //
    Serial.println(" -- alarm setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxx0x1x");
    Serial.print  (" Minute Alarm     | 9 | ");Serial.println(min,BIN);
    Serial.println(" Hour Alarm       | A | 1xxxxxxx");
    Serial.println(" Day Alarm        | B | 1xxxxxxx");
    Serial.println(" WeekDay Alarm    | C | 1xxxxxxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00010010, 0b00000010)) {
      Serial.println("4.1 Error: Control 2 reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_MINUITE, 0b11111111, min)) {
      Serial.println("4.1 Error: Minute Alarm reg (0x09) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_HOUR, 0b11111111, 0b10000000)) {
      Serial.println("4.1 Error: Hour Alarm reg (0x0A) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_DAY, 0b11111111, 0b10000000)) {
      Serial.println("4.1 Error: Day Alarm reg (0x0B) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY, 0b11111111, 0b10000000)) {
      Serial.println("4.1 Error: WeekDay Alarm reg (0x0C) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("4.1 Success");
      testResults[TEST_ALARM][TEST_ALARM_SETUP]=TEST_SUCCESS;
    } else {
      Serial.println("4.1 Fail");
      testResults[TEST_ALARM][TEST_ALARM_SETUP]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 2分間待機
  delay(120000);
  // アラーム状態確認
  Serial.println("4.2 check interupt flag.");
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print("Error : check interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : check interupt .");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxx1xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00001000, 0b00001000)) {
      Serial.println("4.2 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG]=TEST_FAIL;
    } else {
      Serial.println("4.2 Success");
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みフラグクリア
  Serial.println("4.3 clear interupt flag.");
  rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print("Error : clear interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : clear interupt .");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxx0xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00001000, 0b00000000)) {
      Serial.println("4.3 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG]=TEST_FAIL;
    } else {
      Serial.println("4.3 Success");
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム設定変更(割り込みピンを使わない)
  Serial.println("4.4 clear interupt flag.");
  mode.useInteruptPin=0;

  rst=rtc.setAlarmMode(0, &mode);
  if (rst<0) {
    Serial.print("Error : clear interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : clear interupt .");
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxxxx0x");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00000010, 0b00000000)) {
      Serial.println("4.4 Error: Control 2 reg (0x01) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_CHANGE_MODE]=TEST_FAIL;
    } else {
      Serial.println("4.4 Success");
      testResults[TEST_ALARM][TEST_ALARM_CHANGE_MODE]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム停止
  Serial.println("4.5 stop alarm.");
  rst=rtc.controlAlarm(0, 0);
  if (rst<0) {
    Serial.print("Error : stop alarm failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : stop alarm.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting. -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.println(" Control 2        | 1 | xxxx0x0x");
    Serial.println(" Minute Alarm     | 9 | 1xxxxxxx");
    Serial.println(" Hour Alarm       | A | 1xxxxxxx");
    Serial.println(" Day Alarm        | B | 1xxxxxxx");
    Serial.println(" WeekDay Alarm    | C | 1xxxxxxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_CONTROL2, 0b00010010, 0b00000000)) {
      Serial.println("4.5 Error: Control 2 reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_MINUITE, 0b11111111, 0b10000000)) {
      Serial.println("4.5 Error: Minute Alarm reg (0x09) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_HOUR, 0b11111111, 0b10000000)) {
      Serial.println("4.5 Error: Hour Alarm reg (0x0A) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_DAY, 0b11111111, 0b10000000)) {
      Serial.println("4.5 Error: Day Alarm reg (0x0B) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_8564NB_REG_ALARM_WEEKDAY, 0b11111111, 0b10000000)) {
      Serial.println("4.5 Error: WeekDay Alarm reg (0x0C) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("4.5 Success");
      testResults[TEST_ALARM][TEST_ALARM_STOP]=TEST_SUCCESS;
    } else {
      Serial.println("4.5 Fail");
      testResults[TEST_ALARM][TEST_ALARM_STOP]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
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
    if (testResults[0][i]==TEST_NOT_RUN) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[0][i]==TEST_FAIL) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_FREQ_COLS ;i++) {
    if (testResults[1][i]==TEST_NOT_RUN) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[1][i]==TEST_FAIL) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_COLS ;i++) {
    if (testResults[2][i]==TEST_NOT_RUN) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[2][i]==TEST_FAIL) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_ALARM_COLS ;i++) {
    if (testResults[3][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[3][i]==TEST_FAIL) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is failed.");
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
  //if (rtc.begin()) {
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

  int power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
    if (0b010!=power_flag) Serial.println("Caution: low power event detected.");
  }
  int reset_rst =rtc.clearPowerFlag();
  if (RTC_U_SUCCESS!=reset_rst) {
    Serial.print("Error: fail to clear power flag , retval = ");Serial.println(reset_rst);
  } else {
    Serial.println("Success : clear power flag.");
  }

#ifdef DEBUG
  rtc.backupRegValues();
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */

  int rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print("Error : clear interupt failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : clear interupt.");
  }

  Serial.println("==================  set time to rtc ==================");
  testSetDate();
  Serial.println("==================  clock signal output ==================");
  testFreq();
  Serial.println("==================  timer ==================");
  testTimer();
  Serial.println("==================  alarm ==================");
  testAlarm();

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
