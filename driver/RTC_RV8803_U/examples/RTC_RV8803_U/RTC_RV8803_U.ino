#include "RTC_RV8803_U.h"


/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_8564NB_U.hの以下の行を有効にしてください．
 *   #define DEBUG
 */

//#undef DEBUG  // 詳細なメッセージ出力が必要な場合はここをコメントアウト
#define DUMP_REGISTER  // レジスタの値を書き換えた後に，レジスタ値のdumpを見たい場合はこれを有効にする(DEBUGも有効にする)
#define POWER_ON      // 電源OFF状態から給電を開始する試験手順の場合に有効化する
#define CLKOE_PIN 3


#ifdef DEBUG

#define TEST_NOT_RUN 0
#define TEST_SUCCESS 1
#define TEST_FAIL 2

#define TEST_LINE 8
#define TEST_COL  7

#define TEST_POWER_COLS 4
#define TEST_SETTIME_COLS 2
#define TEST_FREQ_COLS 3
#define TEST_CLOCK_CONTROL_COLS 4
#define TEST_ALARM_COLS 7
#define TEST_TIMER_INTERVAL_COLS 5
#define TEST_TIMER_COUNTDOWN_COLS 6
#define TEST_EXT_EVENT_COLS 1

uint8_t testResults[TEST_LINE][TEST_COL];

enum {
  TEST_POWER,
  TEST_SETTIME,
  TEST_FREQ,
  TEST_CLOCK_CONTROL,
  TEST_ALARM,
  TEST_TIMER_INTERVAL,
  TEST_TIMER_COUNTDOWN,
  TEST_EXT_EVENT
};
#endif /* DEBUG */

RTC_RV8803_U rtc = RTC_RV8803_U(&Wire);
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
  if ((15 < val) || (0 > val) ) {
    Serial.println("Error : Illegal interupt flag val.");
    return;
  }
  uint8_t uf, tf, af, evf;
  uf  = (val & 0b1000) >> 3;
  tf  = (val & 0b0100) >> 2;
  af  = (val & 0b0010) >> 1;
  evf = val & 0b0001;
  Serial.print("UF  = ");Serial.print(uf);Serial.print(" , ");
  Serial.print("TF  = ");Serial.print(tf);Serial.print(" , ");
  Serial.print("AF  = ");Serial.print(af);Serial.print(" , ");
  Serial.print("EVF = ");Serial.println(evf);
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
    uint8_t val = 0b11 & power_flag;
    if (0!=val) Serial.println("Caution: low power event detected.");
  }
#ifdef DEBUG
  if (pon) {
    if (0b11 == power_flag) {
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


  // レジスタの点検
  Serial.println("1.2 check power related register.");
  //
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    if (pon) {
      //
      Serial.println(" -- power related register -- ");
      Serial.println(" register              | val(bin) ");
      Serial.println(" -----------------+----+--------- ");
      Serial.println(" flag             | 0E | xxxxxx11");
      //
      bool flag=true;
      if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00000011, 0b00000011)) {
        Serial.println("1.2 Error: flag reg (0x0E) is unmatch.");
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
      Serial.println(" -- power related register -- ");
      Serial.println(" register              | val(bin) ");
      Serial.println(" -----------------+----+--------- ");
      Serial.println(" flag             | 0E | xxxxxx00");
      //
      bool flag=true;
      if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00000011, 0b00000000)) {
        Serial.println("1.2 Error: flag reg (0x0E) is unmatch.");
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
  Serial.println("1.3 clear power related flag value.");
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
    //
    Serial.println(" -- power related register -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" flag             | 0E | xxxxxx00");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00000011, 0b00000000)) {
      Serial.println("1.3 Error: flag reg (0x0E) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("1.3 Success");
      testResults[TEST_POWER][2]=TEST_SUCCESS;
    } else {
      Serial.println("1.3 Fail");
      testResults[TEST_POWER][2]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("1.4 get power related flag.");
  //
  power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
  }
#ifdef DEBUG
  if (0b00 == power_flag) {
    Serial.println("1.4 Success");
    testResults[TEST_POWER][3]=TEST_SUCCESS;
  } else {
    Serial.println("1.4 Fail");
    testResults[TEST_POWER][3]=TEST_FAIL;
  }
#endif /* DEBUG */

}
/*
 * 時刻の設定
 */
void testSetDate(void) {
  Serial.println("2. date related function");
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
  rtc.getTime(&date);
  printTime(&date);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    uint8_t sec, min, hour, mday, wday, month, year;
    //
    sec   = rtc.intToBCD(date.second);
    min   = rtc.intToBCD(date.minute);
    hour  = rtc.intToBCD(date.hour);
    wday  = rtc.intToWday(date.wday);
    mday  = rtc.intToBCD(date.mday);
    month = rtc.intToBCD(date.month);
    year  = rtc.intToBCD(date.year-2000);
    //
    Serial.println(" -- setup time. -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.print  (" Second (no check)| 00 | ");Serial.println(sec,BIN);
    Serial.print  (" Minute           | 01 | ");Serial.println(min,BIN);
    Serial.print  (" Hour             | 02 | ");Serial.println(hour,BIN);
    Serial.print  (" Day              | 03 | ");Serial.println(mday,BIN);
    Serial.print  (" Weekday          | 04 | ");Serial.println(wday,BIN);
    Serial.print  (" Month            | 05 | ");Serial.println(month,BIN);
    Serial.print  (" Year             | 06 | ");Serial.println(year,BIN);
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_MINUTE, 0b01111111, min)) {
      Serial.println("2.1 Error: Minute reg (0x01) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_HOUR, 0b00111111, hour)) {
      Serial.println("2.1 Error: Hour reg (0x02) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_WEEKDAY, 0b01111111, wday)) {
      Serial.println("2.1 Error: Weekday reg (0x03) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_DATE, 0b00111111, mday)) {
      Serial.println("2.1 Error: Day reg (0x04) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_MONTH, 0b00011111, month)) {
      Serial.println("2.1 Error: Month reg (0x05) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_YEAR, 0b11111111, year)) {
      Serial.println("2.1 Error: Year reg (0x06) is unmatch.");
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
 * 周波数信号出力関係
 */
void testFreq(void){
  Serial.println("3. clock freq signal output");
  //
  Serial.println("3.1 select frequency.");
  // freq=101でclockoutを設定
  // CLKOUT control register (0x0F)の3から5bitが101
  int result = rtc.setClockOut(0, 0b10, CLKOE_PIN);
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
    Serial.println(" extension        | 0D | xxxx10xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_EXTENSION, 0b00001100, 0b00001000)) {
      Serial.println("3.1 Error: extemsion reg (0x0D) is unmatch.");
      testResults[TEST_FREQ][0]=TEST_FAIL;
    } else {
      Serial.println("3.1 Success");
      testResults[TEST_FREQ][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("3.2 stop sqw output.");
  result = rtc.controlClockOut(0, 0);
  if (0 > result) {
    Serial.print("Error: fail to stop sqw output , retval = ");Serial.println(result);
#ifdef DEBUG
      Serial.println("3.2 Fail");
      testResults[TEST_FREQ][1]=TEST_FAIL;
#endif /* DEBUG */
  } else {
    Serial.println("Success: stop sqw output.");
#ifdef DEBUG
      Serial.println("3.2 Success");
      testResults[TEST_FREQ][1]=TEST_SUCCESS;
#endif /* DEBUG */
  }

  //
  Serial.println("3.3 select frequency.");
  //
  result = rtc.setClockOutMode(0, 0);
  if (0 > result) {
    Serial.print("Error: fail to set frequency , retval = ");Serial.println(result);
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
    Serial.println(" extension        | 0D | xxxx00xx");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_EXTENSION, 0b00001100, 0b00000000)) {
      Serial.println("3.3 Error: extemsion reg (0x0D) is unmatch.");
      testResults[TEST_FREQ][2]=TEST_FAIL;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_FREQ][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


}
/*
 * 
 */
void testControlClock(void) {
  Serial.println("4. clock control");
  //
  Serial.println("4.1 setup frequency control.");
  int result = rtc.setOscillator(0b101010);
  if (0 > result) {
    Serial.print("Error: fail to setup frequency control , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: setup frequency control.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock control reg -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" offset           | 2C | xx101010");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_OFFSET, 0b00111111, 0b00101010)) {
      Serial.println("4.1 Error: offset reg (0x2C) is unmatch.");
      testResults[TEST_CLOCK_CONTROL][0]=TEST_FAIL;
    } else {
      Serial.println("4.1 Success");
      testResults[TEST_CLOCK_CONTROL][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("4.2 get power related flag.");
  //
  int offset_reg=rtc.getOscillator();
  if (0 > offset_reg) {
    Serial.print("Error: fail to get offset reg , retval = ");Serial.println(offset_reg);
  } else {
    Serial.print("Success: success to get offset reg, retval = ");Serial.println(offset_reg, BIN);
  }
#ifdef DEBUG
  if (0b101010 == offset_reg) {
    Serial.println("4.2 Success");
    testResults[TEST_CLOCK_CONTROL][1]=TEST_SUCCESS;
  } else {
    Serial.println("4.2 Fail");
    testResults[TEST_CLOCK_CONTROL][1]=TEST_FAIL;
  }
#endif /* DEBUG */

  //
  Serial.println("4.3 setup frequency control.");
  result = rtc.setOscillator(0);
  if (0 > result) {
    Serial.print("Error: fail to setup frequency control , retval = ");Serial.println(result);
  } else {
    Serial.println("Success: setup frequency control.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- clock control reg -- ");
    Serial.println(" register              | val(bin) ");
    Serial.println(" -----------------+----+--------- ");
    Serial.println(" offset           | 2C | xx000000");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_OFFSET, 0b00111111, 0b0)) {
      Serial.println("4.3 Error: offset reg (0x2C) is unmatch.");
      testResults[TEST_CLOCK_CONTROL][2]=TEST_FAIL;
    } else {
      Serial.println("4.3 Success");
      testResults[TEST_CLOCK_CONTROL][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("4.4 get power related flag.");
  //
  offset_reg=rtc.getOscillator();
  if (0 > offset_reg) {
    Serial.print("Error: fail to get offset reg , retval = ");Serial.println(offset_reg);
  } else {
    Serial.print("Success: success to get offset reg, retval = ");Serial.println(offset_reg, BIN);
  }
#ifdef DEBUG
  if (0 == offset_reg) {
    Serial.println("4.4 Success");
    testResults[TEST_CLOCK_CONTROL][3]=TEST_SUCCESS;
  } else {
    Serial.println("4.4 Fail");
    testResults[TEST_CLOCK_CONTROL][3]=TEST_FAIL;
  }
#endif /* DEBUG */

}

/*
 * 
 */
void testAlarm(void) {
  Serial.println("5 alarm test");
  // アラーム設定
  Serial.println(F("5.1 setup alarm."));
  date_t timing;
  rtc.getTime(&timing);
  printTime(&timing);
  alarm_mode_t mode;
  mode.useInteruptPin=1;
  mode.type=1;
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
    Serial.println(F(" -- alarm Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.print  (F(" alarm min        | 8 | "));Serial.println(min,BIN);
    Serial.println(F(" alarm hour       | 9 | 1xxxxxxx"));
    Serial.println(F(" alarm day        | A | 1xxxxxxx"));
    Serial.println(F(" control          | F | xxxx1xxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_MINUTE, 0b11111111, min)) {
      Serial.println(F("5.1 Error: Alarm Minute reg (0x08) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("5.1 Error: Alarm Hour reg (0x09) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_DAY, 0b10000000, 0b10000000)) {
      Serial.println(F("5.1 Error: Alarm date reg (0x0A) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_CONTROL, 0b00001000, 0b00001000)) {
      Serial.println(F("5.1 Error: Control reg (0x0F) is unmatch."));
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
  rst = rtc.clearInterupt(0b1111);
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
    Serial.println(F(" -- alarm Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xxxx0xxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00001000, 0b00000000)) {
      Serial.println(F("5.2 Error: flag reg (0x0E) is unmatch."));
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
  rst = rst & 0b0010;
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
  Serial.println(F("5.4 check alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b0010;
  if (rst != 0) {
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
    Serial.println(F(" -- alarm Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xxxx1xxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00001000, 0b00001000)) {
      Serial.println(F("5.5 Error: flag reg (0x0E) is unmatch."));
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
    Serial.println(F(" -- alarm Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" alarm min        | 8 | 1xxxxxxx"));
    Serial.println(F(" alarm hour       | 9 | 1xxxxxxx"));
    Serial.println(F(" alarm date       | A | 1xxxxxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_MINUTE, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm Minute reg (0x08) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_HOUR, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm Hour reg (0x09) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_ALARM_DAY, 0b10000000, 0b10000000)) {
      Serial.println(F("5.6 Error: Alarm date reg (0x0A) is unmatch."));
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
    Serial.println(F(" -- alarm Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" control          | F | xxxx0xxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_CONTROL, 0b00001000, 0b00000000)) {
      Serial.println(F("5.7 Error: control reg (0x0F) is unmatch."));
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
 * 
 */
void testIntervalTimer(void) {
  Serial.println("6 interval timer");
  // タイマ設定
  Serial.println(F("6.1 setup interval timer."));
  rtc_timer_mode_t mode;
  mode.useInteruptPin=1;
  mode.interval=1;
  int rst = rtc.setTimer(0,&mode,0);
  if (rst<0) {
    Serial.print(F("Error : set interval timer, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set interval timer."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- interval timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" extension        | D | xx1xxxxx"));
    Serial.println(F(" control          | F | xx1xxxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_EXTENSION, 0b00100000, 0b00100000)) {
      Serial.println(F("6.1 Error: extension reg (0x0D) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_CONTROL, 0b00100000, 0b00100000)) {
      Serial.println(F("6.1 Error: Control reg (0x0F) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("6.1 Success"));
      testResults[TEST_TIMER_INTERVAL][0]=TEST_SUCCESS;
    } else {
      Serial.println(F("6.1 Fail"));
      testResults[TEST_TIMER_INTERVAL][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // 定周期タイマ発火フラグのクリア
  Serial.println(F("6.2 clear alarm flags"));
  rst = rtc.clearInterupt(0b1111);
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
    Serial.println(F(" -- interval timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xx0xxxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00100000, 0b00000000)) {
      Serial.println(F("6.2 Error: flag reg (0x0E) is unmatch."));
      testResults[TEST_TIMER_INTERVAL][1]=TEST_FAIL;
    } else {
      Serial.println(F("6.2 Success"));
      testResults[TEST_TIMER_INTERVAL][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("6.3 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b1000;
  if (rst == 0) {
    Serial.println(F("6.3 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_INTERVAL][2]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("6.3 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_INTERVAL][2]=TEST_FAIL;
#endif /* DEBUG */
  }

  // 90秒待って，アラームの割り込みフラグが立っていることを確認
  Serial.println(F("wait 90sec."));
  delay(90000);

  // 割り込みのフラグがたっていることを確認
  Serial.println(F("6.4 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b1000;
  if (rst != 0) {
    Serial.println(F("6.4 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_INTERVAL][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("6.4 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_INTERVAL][3]=TEST_FAIL;
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
    Serial.println(F(" -- interval timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xx1xxxxx"));
    //
    //
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00100000, 0b00100000)) {
      Serial.println(F("6.5 Error: flag reg (0x0E) is unmatch."));
      testResults[TEST_TIMER_INTERVAL][4]=TEST_FAIL;
    } else {
      Serial.println(F("6.5 Success"));
      testResults[TEST_TIMER_INTERVAL][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}


/*
 * 
 */
void testCountdownTimer(void) {
  Serial.println("7 countdown timer");
  // タイマ設定
  Serial.println(F("7.1 setup countdown timer."));
  rtc_timer_mode_t mode;
  mode.useInteruptPin=1; // INTピン利用
  mode.interval=0b01;    // 64Hz
  uint16_t multi= (0b00000010 << 8) | 0b01000000; // 512 + 64
  int rst = rtc.setTimer(1,&mode,multi);
  if (rst<0) {
    Serial.print(F("Error : set countdown timer, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : set countdown timer."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- countdown timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" timer counter 0  | B | 01000000"));
    Serial.println(F(" timer counter 1  | C | xxxx0010"));
    Serial.println(F(" extension        | D | xxx1xx01"));
    Serial.println(F(" control          | F | xxx1xxxx"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_TIMER_COUNTER_0, 0b11111111, 0b01000000)) {
      Serial.println(F("7.1 Error: timer counter 0 reg (0x0B) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_TIMER_COUNTER_1, 0b00001111, 0b00000010)) {
      Serial.println(F("7.1 Error: timer counter 1 reg (0x0C) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_EXTENSION, 0b00010011, 0b00010001)) {
      Serial.println(F("7.1 Error: extension reg (0x0D) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_CONTROL, 0b00010000, 0b00010000)) {
      Serial.println(F("7.1 Error: Control reg (0x0F) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("7.1 Success"));
      testResults[TEST_TIMER_COUNTDOWN][0]=TEST_SUCCESS;
    } else {
      Serial.println(F("7.1 Fail"));
      testResults[TEST_TIMER_COUNTDOWN][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  // 定周期タイマ発火フラグのクリア
  Serial.println(F("7.2 clear alarm flags"));
  rst = rtc.clearInterupt(0b1111);
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
    Serial.println(F(" -- countdown timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xxx0xxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00010000, 0b00000000)) {
      Serial.println(F("7.2 Error: flag reg (0x0E) is unmatch."));
      testResults[TEST_TIMER_COUNTDOWN][1]=TEST_FAIL;
    } else {
      Serial.println(F("7.2 Success"));
      testResults[TEST_TIMER_COUNTDOWN][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 割り込みのフラグが消えていることを確認
  Serial.println(F("7.3 get alarm flags"));
  rst = rtc.checkInterupt();
  if (rst<0) {
    Serial.print(F("Error : clear interupt failure, error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : clear interupt ."));
  }
  printInterupt(rst);
  rst = rst & 0b0100;
  if (rst == 0) {
    Serial.println(F("7.3 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_COUNTDOWN][2]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("7.3 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_COUNTDOWN][2]=TEST_FAIL;
#endif /* DEBUG */
  }

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
  rst = rst & 0b0100;
  if (rst != 0) {
    Serial.println(F("7.4 Success"));
#ifdef DEBUG
    testResults[TEST_TIMER_COUNTDOWN][3]=TEST_SUCCESS;
#endif /* DEBUG */
  } else {
    Serial.println(F("7.4 Fail"));
#ifdef DEBUG
    testResults[TEST_TIMER_COUNTDOWN][3]=TEST_FAIL;
#endif /* DEBUG */
  }

  // アラーム発生フラグ確認
  Serial.println(F("7.5 check countdown timer flag bit on reg"));
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- countdown timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" flag             | E | xxx1xxxx"));
    //
    //
    if (!rtc.checkRegValues(RTC_RV8803_REG_FLAG, 0b00010000, 0b00010000)) {
      Serial.println(F("7.5 Error: flag reg (0x0E) is unmatch."));
      testResults[TEST_TIMER_COUNTDOWN][4]=TEST_FAIL;
    } else {
      Serial.println(F("7.5 Success"));
      testResults[TEST_TIMER_COUNTDOWN][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマを止める
  Serial.println(F("7.6 stop countdown timer."));
  rst = rtc.controlTimer(1,0);
  if (rst<0) {
    Serial.print(F("Error : fail to stop cowntdown timer , error="));Serial.println(rst);
  } else {
    Serial.println(F("Success : stop cowntdown timer."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    //
    Serial.println(F(" -- countdown timer Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" -----------------+---+--------- "));
    Serial.println(F(" extension        | D | xxx0xxxx"));
    //
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_EXTENSION, 0b00010000, 0b00000000)) {
      Serial.println(F("7.6 Error: flag reg (0x0E) is unmatch."));
      testResults[TEST_TIMER_COUNTDOWN][5]=TEST_FAIL;
    } else {
      Serial.println(F("7.6 Success"));
      testResults[TEST_TIMER_COUNTDOWN][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println(F("*************************************************************"));
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}


/*
 * 
 */
void testExtEvent(void) {
  Serial.println("8 external event");
  // タイマ設定
  Serial.println(F("8.1 setup external event."));
  event_mode_t mode;
  mode.useInteruptPin=1;
  mode.capture=true; // 外部イベント有効化
  mode.level=1;
  mode.filter=0b11;
  mode.reset= true;
  int result = rtc.setEvent(&mode);
  if (result<0) {
    Serial.print(F("Error : set external event, error="));Serial.println(result);
  } else {
    Serial.println(F("Success : set external event."));
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println(F("Error: fail to get register values."));
  } else {
    //
    Serial.println(F(" -- external event Setting. -- "));
    Serial.println(F(" register             | val(bin) "));
    Serial.println(F(" ----------------+----+--------- "));
    Serial.println(F(" control         | 0F | xxxxx1xx"));
    Serial.println(F(" event           | 2F | 1111xxx1"));
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_RV8803_REG_CONTROL, 0b00000100, 0b00000100)) {
      Serial.println(F("8.1 Error: control reg (0x0F) is unmatch."));
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_RV8803_REG_EVENT, 0b11110001, 0b11110001)) {
      Serial.println(F("8.1 Error: event reg (0x2F) is unmatch."));
      flag=false;
    }
    if (flag) {
      Serial.println(F("8.1 Success"));
      testResults[TEST_EXT_EVENT][0]=TEST_SUCCESS;
    } else {
      Serial.println(F("8.1 Fail"));
      testResults[TEST_EXT_EVENT][0]=TEST_FAIL;
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

  for (int i=0; i< TEST_FREQ_COLS ;i++) {
    if (testResults[TEST_FREQ][i]==TEST_NOT_RUN) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_FREQ][i]==TEST_FAIL) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_CLOCK_CONTROL_COLS ;i++) {
    if (testResults[TEST_CLOCK_CONTROL][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_CLOCK_CONTROL][i]==TEST_FAIL) {
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

  for (int i=0; i< TEST_TIMER_INTERVAL_COLS ;i++) {
    if (testResults[TEST_TIMER_INTERVAL][i]==TEST_NOT_RUN) {
      Serial.print("test 6.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_INTERVAL][i]==TEST_FAIL) {
      Serial.print("test 6.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_COUNTDOWN_COLS ;i++) {
    if (testResults[TEST_TIMER_COUNTDOWN][i]==TEST_NOT_RUN) {
      Serial.print("test 7.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER_COUNTDOWN][i]==TEST_FAIL) {
      Serial.print("test 7.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_EXT_EVENT_COLS ;i++) {
    if (testResults[TEST_EXT_EVENT][i]==TEST_NOT_RUN) {
      Serial.print("test 8.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_EXT_EVENT][i]==TEST_FAIL) {
      Serial.print("test 8.");Serial.print(i+1);Serial.println(" is failed.");
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
 * 
 */
void setup() {
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
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  }
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
  Serial.println("==================  clock freq signal output ==================");
  testFreq();
  Serial.println("================== clock control functions ==================");
  testControlClock();
  Serial.println("==================  Alarm ==================");
  testAlarm();
  Serial.println("==================  Interval Timer ==================");
  testIntervalTimer();
  Serial.println("==================  countdown Timer ==================");
  testCountdownTimer();
  Serial.println("==================  countdown Timer ==================");
  testExtEvent();

#ifdef DEBUG
  calcResult();
#endif /* DEBUG */
  Serial.println("==================  all function test done. ==================");
  delay(10000);
  Serial.println("==================  long run test  ==================");

}

void loop() {


}
