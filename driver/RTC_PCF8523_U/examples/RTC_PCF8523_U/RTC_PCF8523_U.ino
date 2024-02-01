#include "RTC_PCF8523_U.h"

/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_8564NB_U.hの以下の行を有効にしてください．
 *   #define DEBUG
 */

//#undef DEBUG  // 詳細なメッセージ出力が必要な場合はここをコメントアウト
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)
//#define POWER_ON    // 電源OFFからONにして試験を行う場合に有効にする

#ifdef DEBUG
#define TEST_LINE 9
#define TEST_COL 8
#define TEST_NOT_RUN 0
#define TEST_SUCCESS 1
#define TEST_FAIL 2

#define TEST_POWER_COLS 4
#define TEST_SETTIME_COLS 2
#define TEST_CLOCK_CONTROL_COLS 8
#define TEST_FREQ_COLS 4
#define TEST_ALARM_COLS 7
#define TEST_TIMER_B_COLS 6
#define TEST_TIMER_A_COLS 6
#define TEST_TIMER_W_COLS 5
#define TEST_TIMER_S_COLS 6

uint8_t testResults[TEST_LINE][TEST_COL];

enum {
  TEST_POWER,
  TEST_SETTIME,
  TEST_CLOCK_CONTROL,
  TEST_FREQ,
  TEST_ALARM,
  TEST_TIMER_B,
  TEST_TIMER_A,
  TEST_TIMER_W,
  TEST_TIMER_S
};
#endif /* DEBUG */

RTC_PCF8523_U rtc = RTC_PCF8523_U(&Wire);
rtc_u_info_t rtcInfo;


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
 * 割り込みフラグの解析と出力
 */
void printInterupt(int val) {
  if ((31 < val) || (0 > val) ) {
    Serial.println("Error : Illegal interupt flag val.");
    return;
  }
  uint8_t wtaf, ctaf, ctbf, sf, af;
  wtaf = (val & 0b10000) >> 4;
  ctaf = (val & 0b01000) >> 3;
  ctbf = (val & 0b00100) >> 2;
  sf   = (val & 0b00010) >> 1;
  af   = val & 0b00001;
  Serial.print("WTAF = ");Serial.print(wtaf);Serial.print(" , ");
  Serial.print("CTAF = ");Serial.print(ctaf);Serial.print(" , ");
  Serial.print("CTBF = ");Serial.print(ctbf);Serial.print(" , ");
  Serial.print("SF   = ");Serial.print(sf);Serial.print(" , ");
  Serial.print("AF   = ");Serial.println(af);
}

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
 * 電源電圧関連のフラグテスト
 */
void testPower(bool pon) {
  Serial.println("1. power/battery related function test.");
  //
  Serial.println("1.1 get power related flag.");
  //
  int power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
    uint8_t val = 0b10000000 & power_flag;
    if (0!=val) Serial.println("Caution: low power event detected.");
  }
#ifdef DEBUG
  if (pon) {
    if (0b10000000 == power_flag) {
      Serial.println("1.1 Success");
      testResults[TEST_POWER][0]=TEST_SUCCESS;
    } else {
      Serial.println("1.1 Fail");
      testResults[TEST_POWER][0]=TEST_FAIL;
    }
  } else {
    if (0b0 == power_flag) {
      Serial.println("1.1 Success");
      testResults[TEST_POWER][0]=TEST_SUCCESS;
    } else {
      Serial.println("1.1 Fail");
      testResults[TEST_POWER][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.backupRegValues();
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("1.2 clear power related flag value.");
  //
  int result = rtc.clearPowerFlag();
  if (0 > result) {
    Serial.print("Error: fail to clear power flag , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: success to clear power flag.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    if (pon) {
      //
      Serial.println(" -- power related regs -- ");
      Serial.println(" register              | val(bin) ");
      Serial.println(" -----------------+----+--------- ");
      Serial.println(" control3         | 02 | xxxx00xx");
      Serial.println(" second           | 03 | 0xxxxxxx");
      //
      bool flag=true;
      if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_3, 0b00001100, 0b00000000)) {
        Serial.println("1.2 Error: control3 reg (0x02) is unmatch.");
        flag=false;
      }
      if (!rtc.checkRegValues(RTC_PCF8523_REG_SECOND, 0b10000000, 0b00000000)) {
        Serial.println("1.2 Error: second reg (0x03) is unmatch.");
        flag=false;
      }
      if (flag) {
        Serial.println("1.2 Success");
        testResults[TEST_POWER][1]=TEST_SUCCESS;
      } else {
        Serial.println("1.2 Fail");
        testResults[TEST_POWER][1]=TEST_FAIL;
      }
    } else {
      //
      Serial.println(" -- power related regs -- ");
      Serial.println(" register              | val(bin) ");
      Serial.println(" -----------------+----+--------- ");
      Serial.println(" control3         | 02 | xxxx0xxx");
      Serial.println(" second           | 03 | 0xxxxxxx");
      //
      bool flag=true;
      if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_3, 0b00001000, 0b00000000)) {
        Serial.println("1.2 Error: control3 reg (0x02) is unmatch.");
        flag=false;
      }
      if (!rtc.checkRegValues(RTC_PCF8523_REG_SECOND, 0b10000000, 0b00000000)) {
        Serial.println("1.2 Error: second reg (0x03) is unmatch.");
        flag=false;
      }
      if (flag) {
        Serial.println("1.2 Success");
        testResults[TEST_POWER][1]=TEST_SUCCESS;
      } else {
        Serial.println("1.2 Fail");
        testResults[TEST_POWER][1]=TEST_FAIL;
      }
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("1.3 set power related function parameter.");
  //
  result = rtc.setLowPower(0b10100011);
  if (0 > result) {
    Serial.print("Error: fail to set power function parameter , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: set power function parameter.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- power related regs -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" control3         | 02 | 101xxx11");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_3, 0b11100011, 0b10100011)) {
      Serial.println("1.3 Error: control3 reg (0x02) is unmatch.");
      testResults[TEST_POWER][2]=TEST_FAIL;
    } else {
      Serial.println("1.3 Success");
      testResults[TEST_POWER][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("1.4 set power related function parameter again.");
  //
  result = rtc.setLowPower(0b11100000);
  if (0 > result) {
    Serial.print("Error: fail to set power function parameter , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: set power function parameter.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- power related regs -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" control3         | 02 | 111xxx00");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_3, 0b11100011, 0b11100000)) {
      Serial.println("1.4 Error: control3 reg (0x02) is unmatch.");
      testResults[TEST_POWER][3]=TEST_FAIL;
    } else {
      Serial.println("1.4 Success");
      testResults[TEST_POWER][3]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 時刻の設定
 */
void testSetDate(void) {
  Serial.println("2. date on RTC");
  Serial.println("2.1 set date on RTC");
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
    Serial.println(" -- setup time -- ");
    Serial.println(" register         | val(bin) ");
    Serial.println(" -----------------+---+--------- ");
    Serial.print  (" Second (no check)| 3 | ");Serial.println(sec,BIN);
    Serial.print  (" Minute           | 4 | ");Serial.println(min,BIN);
    Serial.print  (" Hour             | 5 | ");Serial.println(hour,BIN);
    Serial.print  (" Day              | 6 | ");Serial.println(mday,BIN);
    Serial.print  (" Weekday          | 7 | ");Serial.println(wday,BIN);
    Serial.print  (" Month            | 8 | ");Serial.println(month,BIN);
    Serial.print  (" Year             | 9 | ");Serial.println(year,BIN);
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_MINUTE, 0b01111111, min)) {
      Serial.println("2.1 Error: Minute reg (0x04) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_HOUR, 0b00111111, hour)) {
      Serial.println("2.1 Error: Hour reg (0x05) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_DATE, 0b00111111, mday)) {
      Serial.println("2.1 Error: Day reg (0x06) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_WEEKDAY, 0b00000111, wday)) {
      Serial.println("2.1 Error: Weekday reg (0x07) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_MONTH, 0b00011111, month)) {
      Serial.println("2.1 Error: Month reg (0x08) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_YEAR, 0b11111111, year)) {
      Serial.println("2.1 Error: Year reg (0x09) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("2.1 Success");
      testResults[TEST_SETTIME][0]=TEST_SUCCESS;
    } else {
      Serial.println("2.1 Fail");
      testResults[TEST_SETTIME][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("2.2 compare date");
  /* 時刻データをRTCに登録するための変数定義 */
  date_t date2;


  /* RTCに現在時刻を設定 */
  if (!rtc.getTime(&date2)) {
    Serial.print("get time to RTC fail.") ;     // 初期化失敗
    while(1);
  } else {
    Serial.println("get time to RTC success."); // 初期化成功
    printTime(&date2);
  }

  bool flag=true;
  if (date.year!=date2.year) flag=false;
  if (date.month!=date2.month) flag=false;
  if (date.wday!=date2.wday) flag=false;
  if (date.mday!=date2.mday) flag=false;
  if (date.hour!=date2.hour) flag=false;
  if (date.minute!=date2.minute) flag=false;
  if (flag) {
    Serial.println("2.2 Success");
#ifdef DEBUG
    testResults[TEST_SETTIME][1]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println("2.2 Fail");
#ifdef DEBUG
    testResults[TEST_SETTIME][1]=TEST_FAIL;
#endif /* DEBUG */
  }

}

/*
 * 時計の進み方などの調整機能
 */
void testControlClock(void) {
  Serial.println("3. clock control function test");
  //
  Serial.println("3.1 control clock frequency.");
  int result = rtc.setOscillator(0b10101010);
  if (0 > result) {
    Serial.print("Error: fail to control frequency offset , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: control clock frequency.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- control clock -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" Offset           | 0E | 10101010");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_OFFSET, 0b11111111, 0b10101010)) {
      Serial.println("3.1 Error: Offset reg (0x0E) is unmatch.");
      testResults[TEST_CLOCK_CONTROL][0]=TEST_FAIL;
    } else {
      Serial.println("3.1 Success");
      testResults[TEST_CLOCK_CONTROL][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("3.2 control clock frequency.");
  result = rtc.getOscillator();
  if (0 > result) {
    Serial.print("Error: fail to get control frequency offset , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: get control clock frequency.");
  }
#ifdef DEBUG
  if (0b10101010==result) {
    Serial.println("3.2 Success");
    testResults[TEST_CLOCK_CONTROL][1]=TEST_SUCCESS;
  } else {
    Serial.println("3.2 Fail");
    testResults[TEST_CLOCK_CONTROL][1]=TEST_FAIL;
  }
#endif /* DEBUG */

  //
  Serial.println("3.3 stop clock.");
  result = rtc.controlClockHalt(0);
  if (0 > result) {
    Serial.print("Error: fail to stop clock , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: stop clock.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- control clock -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" Control 1        | 00 | xx1xxxxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00100000, 0b00100000)) {
      Serial.println("3.3 Error: Control 1 reg (0x00) is unmatch.");
      testResults[TEST_CLOCK_CONTROL][2]=TEST_FAIL;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_CLOCK_CONTROL][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("3.4 get clock status.");
  result = rtc.clockHaltStatus();
  if (0 > result) {
    Serial.print("Error: fail to get clock status , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: get clock status.");
  }
  if (1==result) {
    Serial.println("3.4 Success");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println("3.4 Fail");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  /* 現在時刻の進み方を確認 */
  //
  Serial.println("3.5 check clock stop function.");
  date_t date, date2;
  if (!rtc.getTime(&date)) {
    Serial.print("get time to RTC fail.") ;     // 初期化失敗
  } else {
    Serial.println("get time to RTC success."); // 初期化成功
    printTime(&date);
  }
  delay(10000); // 10秒待機
  if (!rtc.getTime(&date2)) {
    Serial.print("get time to RTC fail.") ;     // 初期化失敗
  } else {
    Serial.println("get time to RTC success."); // 初期化成功
    printTime(&date2);
  }
  bool flag=true;
  if (date.year!=date2.year) flag=false;
  if (date.month!=date2.month) flag=false;
  if (date.wday!=date2.wday) flag=false;
  if (date.mday!=date2.mday) flag=false;
  if (date.hour!=date2.hour) flag=false;
  if (date.minute!=date2.minute) flag=false;
  if (date.second!=date2.second) flag=false;
  if (flag) {
    Serial.println("3.5 Success");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][4]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println("3.5 Fail");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][4]=TEST_FAIL;
#endif /* DEBUG */
  }

  //
  Serial.println("3.6 restart clock.");
  result = rtc.controlClockHalt(1);
  if (0 > result) {
    Serial.print("Error: fail to restart clock , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: restart clock.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- control clock -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" Control 1        | 00 | xx0xxxxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00100000, 0b00000000)) {
      Serial.println("3.6 Error: Control 1 reg (0x00) is unmatch.");
      testResults[TEST_CLOCK_CONTROL][5]=TEST_FAIL;
    } else {
      Serial.println("3.6 Success");
      testResults[TEST_CLOCK_CONTROL][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("3.7 get clock status.");
  result = rtc.clockHaltStatus();
  if (0 > result) {
    Serial.print("Error: fail to get clock status , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: get clock status.");
  }
  if (0==result) {
    Serial.println("3.7 Success");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][6]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println("3.7 Fail");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][6]=TEST_FAIL;
#endif /* DEBUG */
  }

  /* 現在時刻の進み方を確認 */
  //
  Serial.println("3.8 check clock stop function.");
  if (!rtc.getTime(&date)) {
    Serial.print("get time to RTC fail.") ;     // 初期化失敗
  } else {
    Serial.println("get time to RTC success."); // 初期化成功
    printTime(&date);
  }
  delay(10000); // 10秒待機
  if (!rtc.getTime(&date2)) {
    Serial.print("get time to RTC fail.") ;     // 初期化失敗
  } else {
    Serial.println("get time to RTC success."); // 初期化成功
    printTime(&date2);
  }
  flag=true;
  if (date.year!=date2.year) flag=false;
  if (date.month!=date2.month) flag=false;
  if (date.wday!=date2.wday) flag=false;
  if (date.mday!=date2.mday) flag=false;
  if (date.hour!=date2.hour) flag=false;
  if (date.minute!=date2.minute) flag=false;
  if (date.second!=date2.second) flag=false;
  if (!flag) {
    Serial.println("3.8 Success");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][7]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println("3.8 Fail");
#ifdef DEBUG
    testResults[TEST_CLOCK_CONTROL][7]=TEST_FAIL;
#endif /* DEBUG */
  }
}
/*
 * 周波数信号出力
 */
void testFreq(void) {
  Serial.println("4. clock freq signal output");
  //
  Serial.println("4.1 select frequency.");
  // freq=101でclockoutを設定
  // CLKOUT control register (0x0F)の3から5bitが101
  int result = rtc.setClockOut(0, 0b101);
  if (0 > result) {
    Serial.print("Error: fail to set frequency flag , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: set output frequency.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" CLKOUT control   | 0F | xx101xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00111000, 0b00101000)) {
      Serial.println("4.1 Error: CLKOUT control reg (0x0F) is unmatch.");
      testResults[TEST_FREQ][0]=TEST_FAIL;
    } else {
      Serial.println("4.1 Success");
      testResults[TEST_FREQ][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  Serial.println("4.2 stop signal output.");
  // clockoutを停止
  // CLKOUT control register (0x0F)の3から5bitが111
  result = rtc.controlClockOut(0, 0);
  if (0 > result) {
    Serial.print("Error: fail to stop frequency output , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: stop frequency output.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" CLKOUT control   | 0F | xx111xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00111000, 0b00111000)) {
      Serial.println("4.2 Error: CLKOUT control reg (0x0F) is unmatch.");
      testResults[TEST_FREQ][1]=TEST_FAIL;
    } else {
      Serial.println("4.2 Success");
      testResults[TEST_FREQ][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  
  Serial.println("4.3 re-start signal output.");
  // clockoutを再開
  // CLKOUT control register (0x0F)の3から5bitが101
    result = rtc.controlClockOut(0, 1);
  if (0 > result) {
    Serial.print("Error: fail to stop frequency output , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: stop frequency output.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" CLKOUT control   | 0F | xx101xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00111000, 0b00101000)) {
      Serial.println("4.3 Error: CLKOUT control reg (0x0F) is unmatch.");
      testResults[TEST_FREQ][2]=TEST_FAIL;
    } else {
      Serial.println("4.3 Success");
      testResults[TEST_FREQ][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */



  Serial.println("4.4 stop signal output again by setting change.");
  // freq=111(停止)でclockoutを設定
  // CLKOUT control register (0x0F)の3から5bitが111
  result = rtc.setClockOutMode(0, 0b111);
  if (0 > result) {
    Serial.print("Error: fail to set frequency flag , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: set output frequency.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock signal output -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" CLKOUT control   | 0F | xx111xxx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00111000, 0b00111000)) {
      Serial.println("4.4 Error: CLKOUT control reg (0x0F) is unmatch.");
      testResults[TEST_FREQ][3]=TEST_FAIL;
    } else {
      Serial.println("4.4 Success");
      testResults[TEST_FREQ][3]=TEST_SUCCESS;
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
  Serial.println("5 alarm test");
  // アラーム設定
  Serial.println(F("5.1 setup alarm 1."));
  date_t timing;
  rtc.getTime(&timing);
  printTime(&timing);
  alarm_mode_t mode;
  mode.useInteruptPin=1;
  timing.minute+=2;
  timing.hour=0xff;
  timing.mday=0xff;
  timing.wday=0xff;
  int rst=rtc.setAlarm(0, &mode, &timing);
  if (rst<0) {
    Serial.print(F("Error : set alarm failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set alarm."));
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
    Serial.print  (F(" alarm min        | A | "));Serial.println(min,BIN);
    Serial.println(F(" alarm hour       | B | 1xxxxxxx"));
    Serial.println(F(" alarm date       | C | 1xxxxxxx"));
    Serial.println(F(" alarm week day   | D | 1xxxxxxx"));
    Serial.println(F(" control 1        | 0 | xxxxxx1x"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_MINUTE, 0b11111111, min)) {
      Serial.println(F("5.1 Error: Alarm Minute reg (0x0A) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("5.1 Error: Alarm Hour reg (0x0B) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_DATE, 0b10000000, 0b10000000)) {
      Serial.println(F("5.1 Error: Alarm date reg (0x0C) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_WEEKDAY, 0b10000000, 0b10000000)) {
      Serial.println(F("5.1 Error: Alarm week day reg (0x0D) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00000010, 0b00000010)) {
      Serial.println(F("5.1 Error: Control 1 reg (0x00) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("5.1 Success"));
      testResults[TEST_ALARM][0]=TEST_SUCCESS;
    } else {
      Serial.println(F("5.1 Fail"));
      testResults[TEST_ALARM][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラーム発生フラグのクリア
  Serial.println(F("5.2 clear alarm flags"));
  rst = rtc.clearInterupt(0b11111);
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
    Serial.println(F(" control 2        | 1 | xxxx0xxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00001000, 0b00000000)) {
      Serial.println(F("5.2 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_ALARM][1]=TEST_FAIL;
    } else {
      Serial.println(F("5.2 Success"));
      testResults[TEST_ALARM][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("5.3 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b1;
  if (rst == 0) {
    Serial.println(F("5.3 Success"));
#ifdef DEBUG
    testResults[TEST_ALARM][2]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("5.3 Fail"));
#ifdef DEBUG
    testResults[TEST_ALARM][2]=TEST_FAIL;
#endif /* DEBUG */
  }

  // 2分半待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 150sec."));
  delay(150000);

  // 割り込みのフラグがたっていることを確認
  Serial.println(F("5.4 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b1;
  if (rst == 1) {
    Serial.println(F("5.4 Success"));
#ifdef DEBUG
    testResults[TEST_ALARM][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("5.4 Fail"));
#ifdef DEBUG
    testResults[TEST_ALARM][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  // アラーム発生フラグ確認
  Serial.println(F("5.5 check alarm flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | xxxx1xxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00001000, 0b00001000)) {
      Serial.println(F("5.5 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_ALARM][4]=TEST_FAIL;
    } else {
      Serial.println(F("5.5 Success"));
      testResults[TEST_ALARM][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラームを止めて，レジスタの内容を確認
  Serial.println(F("5.6 stop alarm."));
  rst=rtc.controlAlarm(0, 0);
  if (rst<0) {
    Serial.print(F("Error : stop alarm failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop alarm."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" alarm min        | A | 1xxxxxxx"));
    Serial.println(F(" alarm hour       | B | 1xxxxxxx"));
    Serial.println(F(" alarm date       | C | 1xxxxxxx"));
    Serial.println(F(" alarm week day   | D | 1xxxxxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_MINUTE, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm Minute reg (0x0A) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm Hour reg (0x0B) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_DATE, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm date reg (0x0C) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_ALARM_WEEKDAY, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm week day reg (0x0D) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("5.6 Success"));
      testResults[TEST_ALARM][5]=TEST_SUCCESS;
    } else {
      Serial.println(F("5.6 Fail"));
      testResults[TEST_ALARM][5]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // アラームの外部割り込み信号出力設定を解除
  Serial.println(F("5.7 unset interupt signal output"));
  mode. useInteruptPin=0;
  rst=rtc.setAlarmMode(0, & mode);
  if (rst<0) {
    Serial.print(F("Error : unset interupt signal output failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : unset interupt signal output."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values"));
  } else {
    //
    //
    Serial.println(F(" -- alarm setting -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 1        | 0 | xxxxxx1x"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00000010, 0b00000000)) {
      Serial.println(F("5.7 Error: control 1 reg (0x00) is unmatch."));
      testResults[TEST_ALARM][6]=TEST_FAIL;
    } else {
      Serial.println(F("5.7 Success"));
      testResults[TEST_ALARM][6]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * タイマB
 */
void testTimerB(void) {
  //
  Serial.println("6. Timer B.");
  // アラーム発生フラグのクリア
  Serial.println(F("6.1 clear alarm flags"));
  int rst = rtc.clearInterupt(0b11111);
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
    Serial.println(F(" --------- timer B ------------ "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | xx0xxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00100000, 0b00000000)) {
      Serial.println(F("6.1 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_B][0]=TEST_FAIL;
    } else {
      Serial.println(F("6.1 Success"));
      testResults[TEST_TIMER_B][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("6.2 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b100;
  if (rst == 0) {
    Serial.println(F("6.2 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_B][1]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("6.2 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_B][1]=TEST_FAIL;
#endif /* DEBUG */
  }

  // タイマBの設定を実行してレジスタを確認
  Serial.println(F("6.3 set timer B"));
  rtc_timer_mode_t mode;
  mode.pulse  = 0b111;
  mode.repeat = 1;
  mode.useInteruptPin = 1;
  mode.interval = 0b001;
  rst = rtc.setTimer(0, &mode, 0b11111111);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" ---------- timer B------------ "));
    Serial.println(F(" register              | val(bin) "));
    Serial.println(F(" -----------------+----+--------- "));
    Serial.println(F(" control 2        | 01 | xxxxxxx1"));
    Serial.println(F(" CLKOUT           | 0F | x1xxxxx1"));
    Serial.println(F(" Timer B freq     | 12 | x111x001"));
    Serial.println(F(" Timer B          | 13 | 111111(11)"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00000001, 0b00000001)) {
      Serial.println(F("6.3 Error: control 2 reg (0x01) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b01000001, 0b01000001)) {
      Serial.println(F("6.3 Error: CLKOUT reg (0x0F) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_B_FREQ_CONTROL, 0b01110111, 0b01110001)) {
      Serial.println(F("6.3 Error: Timer B freq reg (0x12) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_B, 0b11111100, 0b11111100)) {
      Serial.println(F("6.3 Error: Timer B reg (0x13) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("6.3 Success"));
      testResults[TEST_TIMER_B][2]=TEST_SUCCESS;
    } else {
      Serial.println(F("6.3 Fail"));
      testResults[TEST_TIMER_B][2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */




  // 10秒待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 10sec."));
  delay(10000);

  // 割り込みのフラグがたっていることを確認
  Serial.println(F("6.4 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b00100;
  if (rst != 0) {
    Serial.println(F("6.4 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_B][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("6.4 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_B][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  // アラーム発生フラグ確認
  Serial.println(F("6.5 check alarm flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- timer B -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | xx1xxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00100000, 0b00100000)) {
      Serial.println(F("6.5 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_B][4]=TEST_FAIL;
    } else {
      Serial.println(F("6.5 Success"));
      testResults[TEST_TIMER_B][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマBを止める
  Serial.println(F("6.6 stop timer B"));
  rst = rtc.controlTimer(0,0);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- timer B -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" CLKOUT           | F | xxxxxxx0"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00000001, 0b00000000)) {
      Serial.println(F("6.6 Error: CLKOUT reg (0x0F) is unmatch."));
      testResults[TEST_TIMER_B][5]=TEST_FAIL;
    } else {
      Serial.println(F("6.6 Success"));
      testResults[TEST_TIMER_B][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * タイマAテスト
 */
void testTimerA(void) {
  //
  Serial.println("7. Timer A.");
  // アラーム発生フラグのクリア
  Serial.println(F("7.1 clear alarm flags"));
  int rst = rtc.clearInterupt(0b11111);
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
    Serial.println(F(" ---------- timer A ------------ "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | x0xxxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b01000000, 0b00000000)) {
      Serial.println(F("7.1 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_A][0]=TEST_FAIL;
    } else {
      Serial.println(F("7.1 Success"));
      testResults[TEST_TIMER_A][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("7.2 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b1000;
  if (rst == 0) {
    Serial.println(F("7.2 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_A][1]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("7.2 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_A][1]=TEST_FAIL;
#endif /* DEBUG */
  }

  // タイマBの設定を実行してレジスタを確認
  Serial.println(F("7.3 set timer A"));
  rtc_timer_mode_t mode;
  mode.repeat = 1;
  mode.useInteruptPin = 1;
  mode.interval = 0b001;
  rst = rtc.setTimer (1, &mode, 0b11111111);
  if (rst<0) {
    Serial.print(F("Error : set timer A failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set timer A ."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" ---------- timer A ----------- "));
    Serial.println(F(" register              | val(bin) "));
    Serial.println(F(" -----------------+----+--------- "));
    Serial.println(F(" control 2        | 01 | xxxxxx1x"));
    Serial.println(F(" CLKOUT           | 0F | 1xxxx01x"));
    Serial.println(F(" Timer A freq     | 10 | xxxxx001"));
    Serial.println(F(" Timer A          | 11 | 111111(11)"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00000010, 0b00000010)) {
      Serial.println(F("7.3 Error: control 2 reg (0x01) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b10000110, 0b10000010)) {
      Serial.println(F("7.3 Error: CLKOUT reg (0x0F) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_A_FREQ_CONTROL, 0b00000111, 0b00000001)) {
      Serial.println(F("7.3 Error: Timer A freq reg (0x10) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_A, 0b11111100, 0b11111100)) {
      Serial.println(F("7.3 Error: Timer A reg (0x11) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("7.3 Success"));
      testResults[TEST_TIMER_A][2]=TEST_SUCCESS;
    } else {
      Serial.println(F("7.3 Fail"));
      testResults[TEST_TIMER_A][2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // 10秒待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 10sec."));
  delay(10000);

  // 割り込みのフラグがたっていることを確認
  Serial.println(F("7.4 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b01000;
  if (rst != 0) {
    Serial.println(F("7.4 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_A][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("7.4 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_A][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  // アラーム発生フラグ確認
  Serial.println(F("7.5 check alarm flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" --  timer A  -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | x1xxxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b01000000, 0b01000000)) {
      Serial.println(F("7.5 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_A][4]=TEST_FAIL;
    } else {
      Serial.println(F("7.5 Success"));
      testResults[TEST_TIMER_A][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマAを止める
  Serial.println(F("7.6 stop timer A"));
  rst = rtc.controlTimer(1,0);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" --  timer A  -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" CLKOUT           | F | xxxxx00x"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00000110, 0b00000000)) {
      Serial.println(F("7.6 Error: CLKOUT reg (0x0F) is unmatch."));
      testResults[TEST_TIMER_A][5]=TEST_FAIL;
    } else {
      Serial.println(F("7.6 Success"));
      testResults[TEST_TIMER_A][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * ワッチドッグタイマ
 */
void testWatchDog(void) {
  //
  Serial.println("8. Watchdog Timer.");
  // アラーム発生フラグのクリア
  Serial.println(F("8.1 clear alarm flags"));
  int rst = rtc.clearInterupt(0b11111);
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
    Serial.println(F(" -------- watch dog timer ------- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | 0xxxxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b10000000, 0b00000000)) {
      Serial.println(F("8.1 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_W][0]=TEST_FAIL;
    } else {
      Serial.println(F("8.1 Success"));
      testResults[TEST_TIMER_W][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("8.2 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b10000;
  if (rst == 0) {
    Serial.println(F("8.2 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_W][1]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("8.2 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_W][1]=TEST_FAIL;
#endif /* DEBUG */
  }

  // ワッチドッグタイマの設定を実行してレジスタを確認
  Serial.println(F("8.3 set watchdog timer"));
  rtc_timer_mode_t mode;
  mode.repeat = 1;
  mode.useInteruptPin = 1;
  mode.interval = 0b001;
  rst = rtc.setTimer (2, &mode, 0b11111111);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -------- watch dog timer ----- "));
    Serial.println(F(" register              | val(bin) "));
    Serial.println(F(" -----------------+----+--------- "));
    Serial.println(F(" control 2        | 01 | xxxxx1xx"));
    Serial.println(F(" CLKOUT           | 0F | 1xxxx10x"));
    Serial.println(F(" Timer A freq     | 10 | xxxxx001"));
    Serial.println(F(" Timer A          | 11 | 111111(11)"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00000100, 0b00000100)) {
      Serial.println(F("8.3 Error: control 2 reg (0x01) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b10000110, 0b10000100)) {
      Serial.println(F("8.3 Error: CLKOUT reg (0x0F) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_A_FREQ_CONTROL, 0b00000111, 0b00000001)) {
      Serial.println(F("8.3 Error: Timer A freq reg (0x10) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_A, 0b11111100, 0b11111100)) {
      Serial.println(F("8.3 Error: Timer A reg (0x11) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("8.3 Success"));
      testResults[TEST_TIMER_W][2]=TEST_SUCCESS;
    } else {
      Serial.println(F("8.3 Fail"));
      testResults[TEST_TIMER_W][2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // 10秒待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 10sec."));
  delay(10000);

  // アラーム発生フラグ確認
  Serial.println(F("8.4 check alarm flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- watch dog timer -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | 1xxxxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b10000000, 0b10000000)) {
      Serial.println(F("8.4 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_W][3]=TEST_FAIL;
    } else {
      Serial.println(F("8.4 Success"));
      testResults[TEST_TIMER_W][3]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // watchdogタイマを止める
  Serial.println(F("8.5 stop watchdog timer"));
  rst = rtc.controlTimer(2,0);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" --  watch dog timer  -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" CLKOUT           | F | xxxxx00x"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b00000110, 0b00000000)) {
      Serial.println(F("8.5 Error: CLKOUT reg (0x0F) is unmatch."));
      testResults[TEST_TIMER_W][4]=TEST_FAIL;
    } else {
      Serial.println(F("8.5 Success"));
      testResults[TEST_TIMER_W][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
}

/*
 * Second Timer
 */
void testTimerSec(void) {
  //
  Serial.println("9. Second Timer.");
  // アラーム発生フラグのクリア
  Serial.println(F("9.1 clear alarm flags"));
  int rst = rtc.clearInterupt(0b11111);
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
    Serial.println(F(" -------- second timer ------- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | xxx0xxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00010000, 0b00000000)) {
      Serial.println(F("9.1 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_S][0]=TEST_FAIL;
    } else {
      Serial.println(F("9.1 Success"));
      testResults[TEST_TIMER_S][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("9.2 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : check interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : check interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b10;
  if (rst == 0) {
    Serial.println(F("9.2 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_S][1]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("9.2 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_S][1]=TEST_FAIL;
#endif /* DEBUG */
  }

  // 秒タイマの設定を実行してレジスタを確認
  Serial.println(F("9.3 set watchdog timer"));
  rtc_timer_mode_t mode;
  mode.repeat = 1;
  rst = rtc.setTimer (3, &mode, 0b11111111);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" ------- second timer ----- "));
    Serial.println(F(" register              | val(bin) "));
    Serial.println(F(" -----------------+----+--------- "));
    Serial.println(F(" control 1        | 00 | xxxxx1xx"));
    Serial.println(F(" CLKOUT           | 0F | 1xxxxxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00000100, 0b00000100)) {
      Serial.println(F("9.3 Error: control 2 reg (0x01) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_PCF8523_REG_TIMER_CLOCKOUT_CONTROL, 0b10000000, 0b10000000)) {
      Serial.println(F("9.3 Error: CLKOUT reg (0x0F) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("9.3 Success"));
      testResults[TEST_TIMER_S][2]=TEST_SUCCESS;
    } else {
      Serial.println(F("9.3 Fail"));
      testResults[TEST_TIMER_S][2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 2秒待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 2sec."));
  delay(2000);

  // 割り込みのフラグがたっていることを確認
  Serial.println(F("9.4 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b10;
  if (rst != 0) {
    Serial.println(F("9.4 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_S][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("9.4 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_S][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  // アラーム発生フラグ確認
  Serial.println(F("9.5 check alarm flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- second timer -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control 2        | 1 | xxx1xxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_2, 0b00010000, 0b00010000)) {
      Serial.println(F("9.5 Error: control 2 reg (0x01) is unmatch."));
      testResults[TEST_TIMER_S][4]=TEST_FAIL;
    } else {
      Serial.println(F("9.5 Success"));
      testResults[TEST_TIMER_S][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 秒タイマを止める
  Serial.println(F("9.6 stop watchdog timer"));
  rst = rtc.controlTimer(3,0);
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" --  second timer  -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" Control 1        | 0 | xxxxx0xx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_PCF8523_REG_CONTROL_1, 0b00000100, 0b00000000)) {
      Serial.println(F("9.6 Error: control 1 reg (0x0F) is unmatch."));
      testResults[TEST_TIMER_S][5]=TEST_FAIL;
    } else {
      Serial.println(F("9.6 Success"));
      testResults[TEST_TIMER_S][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
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

  for (int i=0; i< TEST_POWER_COLS ;i++) {
    if (testResults[TEST_POWER][i]==TEST_NOT_RUN) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_POWER][i]==TEST_FAIL) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_SETTIME_COLS ;i++) {
    if (testResults[TEST_SETTIME][i]==TEST_NOT_RUN) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_SETTIME][i]==TEST_FAIL) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_CLOCK_CONTROL_COLS ;i++) {
    if (testResults[TEST_CLOCK_CONTROL][i]==TEST_NOT_RUN) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_CLOCK_CONTROL][i]==TEST_FAIL) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_FREQ_COLS ;i++) {
    if (testResults[TEST_FREQ][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_FREQ][i]==TEST_FAIL) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_ALARM_COLS ;i++) {
    if (testResults[TEST_ALARM][i]==TEST_NOT_RUN) {
      Serial.print("test 5.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_ALARM][i]==TEST_FAIL) {
      Serial.print("test 5.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_B_COLS ;i++) {
    if (testResults[TEST_TIMER_B][i]==TEST_NOT_RUN) {
      Serial.print("test 6.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_B][i]==TEST_FAIL) {
      Serial.print("test 6.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_A_COLS ;i++) {
    if (testResults[TEST_TIMER_A][i]==TEST_NOT_RUN) {
      Serial.print("test 7.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_A][i]==TEST_FAIL) {
      Serial.print("test 7.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_W_COLS ;i++) {
    if (testResults[TEST_TIMER_W][i]==TEST_NOT_RUN) {
      Serial.print("test 8.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_W][i]==TEST_FAIL) {
      Serial.print("test 8.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_S_COLS ;i++) {
    if (testResults[TEST_TIMER_S][i]==TEST_NOT_RUN) {
      Serial.print("test 9.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_S][i]==TEST_FAIL) {
      Serial.print("test 9.");Serial.print(i+1);Serial.println(" is failed.");
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
 * setup
 */
void setup(void) {
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
  //if (rtc.begin(true)) {
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

  Serial.println("==================  test power flag ==================");
#ifdef POWER_ON
  testPower(true);
#else
  testPower(false);
#endif
  Serial.println("==================  set time to rtc ==================");
  testSetDate();
  Serial.println("================== clock control functions ==================");
  testControlClock();
  Serial.println("==================  clock freq signal output ==================");
  testFreq();
  Serial.println("==================  Alarm ==================");
  testAlarm();
  Serial.println("==================  Timer B ==================");
  testTimerB();
  Serial.println("==================  Timer A ==================");
  testTimerA();
  Serial.println("==================  WatchDog ==================");
  testWatchDog();
  Serial.println("==================  Sec Timer ==================");
  testTimerSec();

 
#ifdef DEBUG
  calcResult();
#endif /* DEBUG */
  Serial.println("==================  all function test done. ==================");
  delay(10000);
  Serial.println("==================  long run test  ==================");

}




/*
 * メインループ
 */
void loop() {

  date_t date;
  rtc.getTime(&date);
//#ifndef DEBUG
//  printTime(&date);
//#endif /* not DEBUG */

  delay(1000) ;            // １秒後に繰り返す
}
