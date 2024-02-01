#include "RTC_DS3231_U.h"

/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_8564NB_U.hの以下の行を有効にしてください．
 *   #define DEBUG
 */

//#undef DEBUG  // 詳細なメッセージ出力が必要な場合はここをコメントアウト
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)

#ifdef DEBUG
#define TEST_LINE 5
#define TEST_COL 10
#define TEST_NOT_RUN 0
#define TEST_SUCCESS 1
#define TEST_FAIL 2
#define TEST_SETTIME_COLS 1
#define TEST_FREQ_COLS 5
#define TEST_ALARM_COLS 10
#define TEST_TEMPERATURE_COLS 2
#define TEST_OSCILLATOR_COLS 2
uint8_t testResults[TEST_LINE][TEST_COL];

enum {
  TEST_SETTIME,
  TEST_FREQ,
  TEST_ALARM,
  TEST_TEMPERATURE,
  TEST_OSCILLATOR
};

enum{
  TEST_SETTIME_SETTING
};

enum{
  TEST_FREQ_OUTPUT_SETTING,
  TEST_FREQ_CHANGE_SETTING,
  TEST_FREQ_STOP_OUTPUT,
  TEST_FREQ_STOP_OUTPUT_32K,
  TEST_FREQ_START_OUTPUT_32K
};

enum{
  TEST_ALARM_CLEAR_FLAG,
  TEST_ALARM_SETUP,
  TEST_ALARM_CHECK_FLAG,
  TEST_ALARM_CLEAR_FLAG2,
  TEST_ALARM_CHANGE_MODE,
  TEST_ALARM_STOP,
  TEST_ALARM_CLEAR_FLAG3,
  TEST_ALARM_SETUP2,
  TEST_ALARM_CHECK_FLAG2,
  TEST_ALARM_STOP2
};

enum{
  TEST_TEMPERATURE_GET,
  TEST_TEMPERATURE_EXEC_TCXO
};

enum{
  TEST_OSCILLATOR_1,
  TEST_OSCILLATOR_2
};
#endif /* DEBUG */


RTC_DS3231_U rtc = RTC_DS3231_U(&Wire);

rtc_u_info_t rtcInfo;

/*
 * RTCの機種情報をプリントアウト
 */
void printRtcInfo(rtc_u_info_t * rtcInfo) {
  Serial.print(F("RTC type number            : "));Serial.println(rtcInfo->type);
  Serial.print(F("number of interupt pin     : "));Serial.println(rtcInfo->numOfInteruptPin);
  Serial.print(F("number of Alarm            : "));Serial.println(rtcInfo->numOfAlarm);
  Serial.print(F("number of Timer            : "));Serial.println(rtcInfo->numOfTimer);
  Serial.print(F("number of clock output pin : "));Serial.println(rtcInfo->numOfClockOut);
  Serial.print(F("number of year digit       : "));Serial.println(rtcInfo->numOfYearDigits);
  Serial.print(F("RTC have year overflow bit : "));
  if (rtcInfo->haveYearOverflowBit) {
    Serial.println(F("true"));
  } else {
    Serial.println(F("false"));
  }
  Serial.print(F("RTC have millisec function : "));
  if (rtcInfo->haveMilliSec) {
    Serial.println(F("true"));
  } else {
    Serial.println(F("false"));
  }
  Serial.print(F("SQW pin and interupt pin are independent : "));
  if (rtcInfo->independentSQW) {
    Serial.println(F("true"));
  } else {
    Serial.println(F("false"));
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
    Serial.print(F("set time to RTC fail.")) ;     // 初期化失敗
    while(1);
  } else {
    Serial.println(F("set time to RTC success.")); // 初期化成功
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
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
    Serial.println(F(" -- setup time. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.print  (F(" Second (no check)| 0 | "));Serial.println(sec,BIN);
    Serial.print  (F(" Minute           | 1 | "));Serial.println(min,BIN);
    Serial.print  (F(" Hour             | 2 | "));Serial.println(hour,BIN);
    Serial.print  (F(" Day              | 3 | "));Serial.println(mday,BIN);
    Serial.print  (F(" Weekday          | 4 | "));Serial.println(wday,BIN);
    Serial.print  (F(" Month            | 5 | "));Serial.println(month,BIN);
    Serial.print  (F(" Year             | 6 | "));Serial.println(year,BIN);
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_MINUTE, 0b01111111, min)) {
      Serial.println(F("1 Error: Minute reg (0x01) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_HOUR, 0b00111111, hour)) {
      Serial.println(F("1 Error: Hour reg (0x02) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_WEEKDAY, 0b00000111, wday)) {
      Serial.println(F("1 Error: Day reg (0x03) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_DATE, 0b00111111, mday)) {
      Serial.println(F("1 Error: Weekday reg (0x04) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_MONTH, 0b00011111, month)) {
      Serial.println(F("1 Error: Month reg (0x05) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_YEAR, 0b11111111, year)) {
      Serial.println(F("1 Error: Year reg (0x06) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("1 Success"));
      testResults[TEST_SETTIME][TEST_SETTIME_SETTING]=TEST_SUCCESS;
    } else {
      Serial.println(F("1 Fail"));
      testResults[TEST_SETTIME][TEST_SETTIME_SETTING]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 周波数信号出力
 */
void testFreq(void) {
  Serial.println(F("2. clock signal output test."));
  // 周波数信号を出力する設定
  Serial.println(F("2.1 setup clock signal output."));
  //
  int rst=rtc.setClockOut(0, 1);
  if (rst<0) {
    Serial.print(F("Error : setup clock out failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : start clock output."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- clock signal output -- "));
    Serial.println(F(" register        | val(bin) "));
    Serial.println(F(" ------------+---+--------- "));
    Serial.println(F(" control     | E | xxx010xx "));
    //
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00011100, 0b00001000)) {
      Serial.println(F("2.1 Error: control reg (0xE) is unmatch."));
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_FAIL ;
    } else {
      Serial.println(F("2.1 Success"));
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数信号出力設定を変更
  Serial.println(F("2.2 change clock signal output setting."));
  //
  rst=rtc.setClockOutMode(0, 3);
  if (rst<0) {
    Serial.print(F("Error : change clock out setting failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : change clock out setting."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- clock signal output -- "));
    Serial.println(F(" register        | val(bin) "));
    Serial.println(F(" ------------+---+--------- "));
    Serial.println(F(" control     | E | xxx110xx "));
    //
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL,  0b00011100, 0b00011000)) {
      Serial.println(F("2.2 Error: control reg (0xE) is unmatch."));
      testResults[TEST_FREQ][TEST_FREQ_CHANGE_SETTING]=TEST_FAIL ;
    } else {
      Serial.println(F("2.2 Success"));
      testResults[TEST_FREQ][TEST_FREQ_CHANGE_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数信号出力を止める
  Serial.println(F("2.3 stop clock signal output."));
  //
  rst=rtc.controlClockOut(0, 0);
  if (rst<0) {
    Serial.print(F("Error : stop clock out failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop clock out."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- clock signal output -- "));
    Serial.println(F(" register        | val(bin) "));
    Serial.println(F(" ------------+---+--------- "));
    Serial.println(F(" control     | E | xxxxx1xx "));
    //
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000100, 0b00000100)) {
      Serial.println(F("2.3 Error: Clkout freq reg (0xE) is unmatch."));
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT]=TEST_FAIL ;
    } else {
      Serial.println(F("2.3 Success"));
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 32kHz端子への周波数信号出力を止める
  Serial.println(F("2.4 stop clock signal output on 32kHz pin."));
  //
  rst=rtc.controlClockOut(1, 0);
  if (rst<0) {
    Serial.print(F("Error : stop clock out failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop clock out."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- clock signal output -- "));
    Serial.println(F(" register        | val(bin) "));
    Serial.println(F(" ------------+---+--------- "));
    Serial.println(F(" Status      | F | xxxxx0xx "));
    //
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000100, 0b00000000)) {
      Serial.println(F("2.4 Error: Status reg (0xF) is unmatch."));
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_32K]=TEST_FAIL ;
    } else {
      Serial.println(F("2.4 Success"));
      testResults[TEST_FREQ][TEST_FREQ_STOP_OUTPUT_32K]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 32kHz端子への周波数信号出力を再開
  Serial.println(F("2.5 re-start clock signal output on 32kHz pin."));
  //
  rst=rtc.controlClockOut(1, 1);
  if (rst<0) {
    Serial.print(F("Error : stop clock out failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop clock out."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- clock signal output -- "));
    Serial.println(F(" register        | val(bin) "));
    Serial.println(F(" ------------+---+--------- "));
    Serial.println(F(" Status      | F | xxxx1xxx "));
    //
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00001000, 0b00001000)) {
      Serial.println(F("2.5 Error: Status reg (0xF) is unmatch."));
      testResults[TEST_FREQ][TEST_FREQ_START_OUTPUT_32K]=TEST_FAIL ;
    } else {
      Serial.println(F("2.5 Success"));
      testResults[TEST_FREQ][TEST_FREQ_START_OUTPUT_32K]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 割り込み情報のプリントアウト
 */
void printInterupt(int flag) {
  int alarm1, alarm2;
  switch(flag) {
    case 0 : alarm1=0;alarm2=0;break;
    case 1 : alarm1=1;alarm2=0;break;
    case 2 : alarm1=0;alarm2=1;break;
    case 3 : alarm1=1;alarm2=1;break;
    default: Serial.print(F("illegal val = "));Serial.println(flag,BIN);return;
  }
  Serial.print(F("ALARM2 = "));Serial.print(alarm2);Serial.print(F(", ALARM1 = "));Serial.println(alarm1);
}

/*
 * アラームのテスト(2種類アラームがあるため，2通り実施
 */
void testAlarm(void) {
  Serial.println(F("3. alarm test."));

  // アラーム発生フラグのクリア
  Serial.println(F("3.1 clear alarm flags"));
  int rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" status           | F | xxxxxx00"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000011, 0b00000000)) {
      Serial.println(F("3.1 Error: status reg (0x0F) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG]=TEST_FAIL;
    } else {
      Serial.println(F("3.1 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム設定
  Serial.println(F("3.2 setup alarm 1."));
  date_t timing;
  rtc.getTime(&timing);
  printTime(&timing);
  // 設定内容 - アラーム1 (num=0) 1分後に設定(秒と分以外は無視)
  // レジスタ(0x07～0x0A)に発火時刻情報を登録
  // controlレジスタ(0x0E)のINTCN(3bit目)を1に，A1IE(最下位bit)を1に設定
  alarm_mode_t mode;
  mode.useInteruptPin=1;
  timing.minute+=1;
  timing.hour=0xff;
  timing.mday=0xff;
  timing.wday=0xff;
  rst=rtc.setAlarm(0, &mode, &timing);
  if (rst<0) {
    Serial.print(F("Error : set alarm 1 failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set alarm 1."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    uint8_t min = rtc.intToBCD(timing.minute);
    uint8_t sec = rtc.intToBCD(timing.second);
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.print  (F(" alarm_1 sec      | 7 | "));Serial.println(sec,BIN);
    Serial.print  (F(" alarm_1 min      | 8 | "));Serial.println(min,BIN);
    Serial.println(F(" alarm_1 hour     | 9 | 1xxxxxxx"));
    Serial.println(F(" alarm_1 w/m day  | A | 1xxxxxxx"));
    Serial.println(F(" control          | E | xxxxx1x1"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM1_SECCOND, 0b11111111, sec)) {
      Serial.println(F("3.2 Error: Alarm 1 Second reg (0x07) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM1_MINUTE, 0b11111111, min)) {
      Serial.println(F("3.2 Error: Alarm 1 Minute reg (0x08) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM1_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("3.2 Error: Alarm 1 Hour reg (0x09) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM1_DAY, 0b10000000, 0b10000000)) {
      Serial.println(F("3.2 Error: Alarm 1 week/day reg (0x0A) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000101, 0b00000101)) {
      Serial.println(F("3.2 Error: Control reg (0x0E) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("3.2 Success"));
      testResults[TEST_ALARM][TEST_ALARM_SETUP]=TEST_SUCCESS;
    } else {
      Serial.println(F("3.2 Fail"));
      testResults[TEST_ALARM][TEST_ALARM_SETUP]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 2分間待機
  Serial.println(F("wait 2 min."));
  delay(120000);
  // アラーム状態確認
  Serial.println(F("3.3 check interupt flag."));
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" status           | F | xxxxxxx1"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000001, 0b00000001)) {
      Serial.println(F("3.3 Error: Status reg (0x0F) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG]=TEST_FAIL;
    } else {
      Serial.println(F("3.3 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム発生フラグのクリア
  Serial.println(F("3.4 clear alarm flags"));
  rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" status           | F | xxxxxx00"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000011, 0b00000000)) {
      Serial.println(F("3.4 Error: status reg (0x0F) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG2]=TEST_FAIL;
    } else {
      Serial.println(F("3.4 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // アラーム設定変更(割り込みピンを使わない)
  Serial.println(F("3.5 change alarm mode."));
  mode.useInteruptPin=0;

  rst=rtc.setAlarmMode(0, &mode);
  if (rst<0) {
    Serial.print(F("Error : change alarm mode failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : change alarm mode."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control          | E | xxxxxxx0"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000001, 0b00000000)) {
      Serial.println(F("3.5 Error: Control reg (0x0E) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CHANGE_MODE]=TEST_FAIL;
    } else {
      Serial.println(F("3.5 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CHANGE_MODE]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム停止
  Serial.println(F("3.6 stop alarm 1."));
  rst=rtc.controlAlarm(0, 0);
  if (rst<0) {
    Serial.print(F("Error : stop alarm 1 failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop alarm 1."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control          | E | xxxxxxx0"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000001, 0b00000000)) {
      Serial.println(F("3.6 Error: Control reg (0x0E) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_STOP]=TEST_FAIL;
    } else {
      Serial.println(F("3.6 Success"));
      testResults[TEST_ALARM][TEST_ALARM_STOP]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム発生フラグのクリア
  Serial.println(F("3.7 clear alarm flags"));
  rst = rtc.clearInterupt(0b11);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" status           | F | xxxxxx00"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000011, 0b00000000)) {
      Serial.println(F("3.7 Error: status reg (0x0F) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG3]=TEST_FAIL;
    } else {
      Serial.println(F("3.7 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CLEAR_FLAG3]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // アラーム2設定
  Serial.println(F("3.8 setup alarm 2."));
  rtc.getTime(&timing);
  printTime(&timing);
  // 設定内容 - アラーム2 (num=1) 2分後に設定(分以外は無視)
  // レジスタ(0x0B～0x0D)に発火時刻情報を登録
  // controlレジスタ(0x0E)のINTCN(3bit目)を1に，A2IE(1bit目)を1に設定
  mode.useInteruptPin=1;
  timing.second=0xff;
  timing.minute+=2;
  timing.hour=0xff;
  timing.mday=0xff;
  timing.wday=0xff;
  rst=rtc.setAlarm(1, &mode, &timing);
  if (rst<0) {
    Serial.print(F("Error : set alarm 2 failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set alarm 2."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    uint8_t min = rtc.intToBCD(timing.minute);
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.print  (F(" alarm_2 min      | B | "));Serial.println(min,BIN);
    Serial.println(F(" alarm_2 hour     | C | 1xxxxxxx"));
    Serial.println(F(" alarm_2 w/m day  | D | 1xxxxxxx"));
    Serial.println(F(" control          | E | xxxxx11x"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM2_MINUTE, 0b11111111, min)) {
      Serial.println(F("3.8 Error: Alarm 2 Minute reg (0x0B) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM2_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("3.8 Error: Alarm 2 Hour reg (0x0C) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_ALARM2_DAY, 0b10000000, 0b10000000)) {
      Serial.println(F("3.8 Error: Alarm 2 week/day reg (0x0D) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000110, 0b00000110)) {
      Serial.println(F("3.8 Error: Control reg (0x0E) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("3.8 Success"));
      testResults[TEST_ALARM][TEST_ALARM_SETUP2]=TEST_SUCCESS;
    } else {
      Serial.println(F("3.8 Fail"));
      testResults[TEST_ALARM][TEST_ALARM_SETUP2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 2分間待機
  Serial.println(F("wait 2.5 min."));
  delay(150000);
  // アラーム状態確認
  Serial.println(F("3.9 check interupt flag."));
  // 割り込みフラグ確認
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" status           | F | xxxxxx10"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_STATUS, 0b00000011, 0b00000010)) {
      Serial.println(F("3.9 Error: Status reg (0x0F) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG2]=TEST_FAIL;
    } else {
      Serial.println(F("3.9 Success"));
      testResults[TEST_ALARM][TEST_ALARM_CHECK_FLAG2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム停止
  Serial.println(F("3.10 stop alarm 2."));
  rst=rtc.controlAlarm(1, 0);
  if (rst<0) {
    Serial.print(F("Error : stop alarm 2 failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop alarm 2."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control          | E | xxxxxx0x"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00000010, 0b00000000)) {
      Serial.println(F("3.10 Error: Control reg (0x0E) is unmatch."));
      testResults[TEST_ALARM][TEST_ALARM_STOP2]=TEST_FAIL;
    } else {
      Serial.println(F("3.10 Success"));
      testResults[TEST_ALARM][TEST_ALARM_STOP2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * RTC内部温度に関連した機能のテスト
 */
void testTemperature(void){
  Serial.println(F("4. temperature related function test."));

  // RTC内部温度の取得
  Serial.println(F("4.1 get RTC temperature."));
  float temperature =rtc.getTemperature(RTC_U_TEMPERATURE_CELCIUS);
  if (RTC_U_TEMPERATURE_FAILURE==temperature) {
    Serial.print(F("Error : can not get RTC temperature, val="));Serial.println(temperature);
#ifdef DEBUG
    testResults[TEST_TEMPERATURE][TEST_TEMPERATURE_GET]=TEST_FAIL;
#endif /* DEBUG */
  } else {
    Serial.print(F("Success : get RTC temperature = "));Serial.println(temperature);
#ifdef DEBUG
    testResults[TEST_TEMPERATURE][TEST_TEMPERATURE_GET]=TEST_SUCCESS;
#endif /* DEBUG */
  }





  // 手動でTCXOを実行
  Serial.println(F("4.2 do TCXO function manually."));
  int rst = rtc.controlTemperatureFunction(1);
  if (rst<0) {
    Serial.print(F("Error : execute TCXO manually, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : execute TCXO manually."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- temperature function -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control          | E | xx1xxxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_CONTROL, 0b00100000, 0b00100000)) {
      Serial.println(F("4.2 Error: Control reg (0x0E) is unmatch."));
      testResults[TEST_TEMPERATURE][TEST_TEMPERATURE_EXEC_TCXO]=TEST_FAIL;
    } else {
      Serial.println(F("4.2 Success"));
      testResults[TEST_TEMPERATURE][TEST_TEMPERATURE_EXEC_TCXO]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 内部発振器の制御
 */
void testOscillatorControl(void) {
  Serial.println(F("5. oscillator control function test."));

  // 周波数制御パラメータ設定 1回目
  Serial.println(F("5.1 write and read setting."));
  uint8_t test_data = 0b01010101;
  rtc.setOscillator(test_data);
  uint8_t get_data = (uint8_t) rtc.getOscillator();
  if (get_data == test_data) {
    Serial.println(F("5.1 Success : set and get RTC oscillator setting."));
  } else {
    Serial.println(F("5.1 Error : set and get RTC oscillator setting."));
  }

#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- oscillator setting -- "));
    Serial.println(F(" register              | val(bin) "));
    Serial.println(F(" -----------------+----+--------- "));
    Serial.println(F(" crystal offset   | 10 | 01010101"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_AGING_OFFSET, 0b11111111, 0b01010101)) {
      Serial.println(F("5.1 Error: Crystal offset reg (0x10) is unmatch."));
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_1]=TEST_FAIL;
    } else {
      Serial.println(F("5.1 Success"));
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 周波数制御パラメータ設定 2回目
  Serial.println("5.2 write and read setting.");
  test_data = ~(test_data);
  rtc.setOscillator(test_data);
  get_data = (uint8_t) rtc.getOscillator();
  if (get_data == test_data) {
    Serial.println("5.2 Success : set and get RTC oscillator setting.");
  } else {
    Serial.println("5.2 Error : set and get RTC oscillator setting.");
  }

#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- oscillator setting -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" crystal offset   | 10 | 10101010");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_DS3231_REG_AGING_OFFSET, 0b11111111, 0b10101010)) {
      Serial.println("5.2 Error: Crystal offset reg (0x10) is unmatch.");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_2]=TEST_FAIL;
    } else {
      Serial.println("5.2 Success");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_2]=TEST_SUCCESS;
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

  for (int i=0; i< TEST_ALARM_COLS ;i++) {
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

  for (int i=0; i< TEST_TEMPERATURE_COLS ;i++) {
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

  for (int i=0; i< TEST_OSCILLATOR_COLS ;i++) {
    if (testResults[4][i]==TEST_NOT_RUN) {
      Serial.print("test 5.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[4][i]==TEST_FAIL) {
      Serial.print("test 5.");Serial.print(i+1);Serial.println(" is failed.");
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

  //if (rtc.begin(true)) {
  if (rtc.begin(false)) {
    Serial.println("Successful initialization of the RTC"); // 初期化成功
  } else {
    Serial.print("Failed initialization of the RTC");  // 初期化失敗
    while(1) ;                                              // 処理中断
  }

  int power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
    if (0!=power_flag) Serial.println("Caution: low power event detected.");
  }
#ifdef DEBUG
  rtc.backupRegValues();
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */

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

  Serial.println("==================  set time to rtc ==================");
  testSetDate();
  Serial.println("==================  clock signal output ==================");
  testFreq();
  Serial.println("==================  alarm ==================");
  testAlarm();
  Serial.println("==================  temperature ==================");
  testTemperature();
  Serial.println("==================  time oscillator control ==================");
  testOscillatorControl();

#ifdef DEBUG
  calcResult();
#endif /* DEBUG */
  Serial.println("==================  all function test done. ==================");
  delay(10000);
  Serial.println("==================  long run test  ==================");


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
#ifndef DEBUG
  printTime(&date);
#endif /* not DEBUG */

  delay(1000) ;            // １秒後に繰り返す

}
