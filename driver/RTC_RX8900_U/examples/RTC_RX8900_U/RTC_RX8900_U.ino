#include "RTC_RX8900_U.h"

#define CLOCK_OUT_PIN 3

/*
 * 本サンプルプログラムはRTCの動作テストを行うため，詳細な出力が必要な場合はRTC_RX8900_U.hの以下の行を有効にしてください．
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
#define TEST_FREQ_COLS 1
#define TEST_TIMER_COLS 10
#define TEST_ALARM_COLS 6
#define TEST_TEMPERATURE_COLS 2
#define TEST_CLOCK_COLS 1
uint8_t testResults[TEST_LINE][TEST_COL];

enum{
TEST_FREQ,
TEST_TIMER,
TEST_ALARM,
TEST_TEMPERATURE,
TEST_CLOCK
};
#endif /* DEBUG */

RTC_RX8900_U rtc = RTC_RX8900_U(&Wire);
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
  if (rtcInfo.haveMilliSec) { // RTCがミリ秒単位の出力を保つ場合(普通はないけど，将来のため?)
    Serial.print(" ");Serial.print(date->millisecond);
  }
  Serial.println("");
}

/*
 * 割り込み端子の情報の出力
 */
void printInterrupt(uint16_t val) {
  if (val <0) {
    Serial.println(" Error : illegal interrupt flag value. ");
    return;
  }
  uint8_t reg=(uint8_t) val;
  uint8_t UTIMER_F, TIMER_F, ALARM_F;
  UTIMER_F=reg&0b100;
  if (UTIMER_F>0) UTIMER_F=1;
  TIMER_F=reg&0b10;
  if (TIMER_F>0) TIMER_F=1;
  ALARM_F=reg&0b1;
  if (ALARM_F>0) ALARM_F=1;
  Serial.print("UTIMER = ");Serial.print(UTIMER_F);Serial.print(", TIMER = ");Serial.print(TIMER_F);Serial.print(" , ALARM = ");Serial.println(ALARM_F);
}

/*
 * 周波数出力機能のテスト
 */
void testFreqOut(void) {
  Serial.println("1. freq signal output test.");
#ifdef DEBUG
  uint8_t mask, value;
#endif /* DEBUG */
  Serial.println("1.1. 36.768kHz output test.");
  //
  // 周波数信号を出力する設定
  int rst=rtc.setClockOut(0, 0b11, CLOCK_OUT_PIN); // 3番ピンを使う
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
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" extention  | xxxx11xx ");
    //
    mask = 0b00001100;
    value = 0b1100;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("1.1 Error: Extention reg (0x0D) is unmatch.");
      testResults[TEST_FREQ][0]=TEST_FAIL ;
    } else {
      Serial.println("1.1 Success");
      testResults[TEST_FREQ][0]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  //
  // 周波数信号出力を止める設定
  rst = rtc.controlClockOut(0,0);
  if (rst<0) {
    Serial.print("Error : stop clock output failure, error=");Serial.println(rst);
  } else {
    Serial.println("Success : stop clock output.");
  }
  Serial.println("all freq signal test done.");
}

/*
 * タイマのテスト
 */
void testTimer(void) {
  Serial.println("2. timer test.");
#ifdef DEBUG
  uint8_t mask, value;
#endif /* DEBUG */
  Serial.println("2.1.  9 sec timer setting.");
  // multi : timerカウンタ0とtimerカウンタ1の値に割当て
  // mode - repeat         : 無視
  //      - useInteruptPin : コントールレジスタ 4bit目(TIE bit)に割当て (0 ピン出力なし, 1 ピン出力あり)
  //      - interval       : extentionレジスタの0,1bit目(TSEL0,1)に割当て
  rtc_timer_mode_t mode;
  // 9秒で発火するタイマ設定
  uint16_t multi=0b101000000; //   256 (counter1 - 0bit目) , 64 (counter0 - 6bit目)
  mode.interval = 0b01;     // TSEL1 = 0 , TSEC0 = 1 (64Hz)
  mode.useInteruptPin = 1;  // TIE bit = 1 ピン出力あり
  //
  //
  int flag=rtc.setTimer((uint8_t)0,&mode , multi);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : setup timer.");
  }
  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }
  uint16_t intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" counter0   | 01000000 ");
    Serial.println(" counter1   | 00000001 ");
    Serial.println(" extention  | xxx1xx01 ");
    Serial.println(" control    | xxx1xxxx ");
    //
    mask = 0b11111111;
    value = 0b01000000;
    if (!rtc.checkRegValues(0x0B, mask, value)) {
      Serial.println("Error: Counter0 reg (0x0B) is unmatch.");
      flag=false;
    }
    mask = 0b11111111;
    value = 0b00000001;
    if (!rtc.checkRegValues(0x0C, mask, value)) {
      Serial.println("Error: Counter1 reg (0x0C) is unmatch.");
      flag=false;
    }
    mask = 0b00010011;
    value = 0b00010001;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      flag=false;
    }
    mask = 0b00010000;
    value = 0b00010000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("2.1 Success");
      testResults[TEST_TIMER][0]=TEST_SUCCESS;
    } else {
      Serial.println("2.1 Fail");
      testResults[TEST_TIMER][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for timer.");
  delay(16000);
  Serial.println("2.2. check 9 sec timer result.");
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" flag       | xxx1xxxx ");
    //
    mask = 0b00010000;
    value = 0b00010000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0E) is unmatch.");
      Serial.println("2.2 Fail");
      testResults[TEST_TIMER][1]=TEST_FAIL;
    } else {
      Serial.println("2.2 Success");
      testResults[TEST_TIMER][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマ停止
  //
  Serial.println("2.3. stop timer. ");
  flag=rtc.controlTimer(0, 0 );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop timer.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" extention  | xxx0xxxx ");
    //
    mask = 0b00010000;
    value = 0b0;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      Serial.println("2.3 Fail");
      testResults[TEST_TIMER][2]=TEST_FAIL;
    } else {
      Serial.println("2.3 Success");
      testResults[TEST_TIMER][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }

  // 5分20秒で発火するタイマ設定
  Serial.println("2.4. minutes timer setting. ");
  mode.interval = 0b10;     // TSEL1 = 1 , TSEC0 = 0 (1Hz)
  mode.useInteruptPin = 0;  // TIE bit = 0 ピン出力なし
  //
  //
  flag=rtc.setTimerMode(0, &mode);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set timer mode, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set timer mode.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println("*counter0   | 01000000 ");
    Serial.println("*counter1   | 00000001 ");
    Serial.println(" extention  | xxx0xx10 ");
    Serial.println(" control    | xxx0xxxx ");
    //
    mask  = 0b00010011;
    value = 0b00000010;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      flag=false;
    }
    mask  = 0b00010000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("2.4 Success");
      testResults[TEST_TIMER][3]=TEST_SUCCESS;
    } else {
      Serial.println("2.4 Fail");
      testResults[TEST_TIMER][3]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマ再開
  Serial.println("2.5. re-start timer. ");
  //
  //
  flag=rtc.controlTimer(0, 1 );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to start timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : start timer.");
  }
  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" extention  | xxx1xxxx ");
    //
    mask = 0b00010000;
    value = 0b00010000;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      Serial.println("2.5 Fail");
      testResults[TEST_TIMER][4]=TEST_FAIL;
    } else {
      Serial.println("2.5 Success");
      testResults[TEST_TIMER][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for timer.");
  delay(360000);
  Serial.println("2.6. check minute timer result.");
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" flag       | xxx1xxxx ");
    //
    mask = 0b00010000;
    value = 0b00010000;
    if (!rtc.checkRegValues(0x0E, mask, value)) {
      Serial.println("Error: flag reg (0x0E) is unmatch.");
      Serial.println("2.6 Fail");
      testResults[TEST_TIMER][5]=TEST_FAIL;
    } else {
      Serial.println("2.6 Success");
      testResults[TEST_TIMER][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマ停止
  Serial.println("2.7. stop timer. ");
  //
  //
  flag=rtc.controlTimer(0, 0 );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop timer.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" extention  | xxx1xxxx ");
    //
    mask = 0b00010000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      Serial.println("2.7 Fail");
      testResults[TEST_TIMER][6]=TEST_FAIL;
    } else {
      Serial.println("2.7 Success");
      testResults[TEST_TIMER][6]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }

  //
  Serial.println("2.8. update timer setting. ");
  // multiは無視
  // mode - repeat         : 無視
  //      - useInteruptPin : 無視
  //      - interval       : extentionレジスタの5bit目(USEL)に1を代入 (分単位で発火)
  mode.interval = 0b01;     // USEL = 1 (分)
  //
  flag=rtc.setTimer((uint8_t)1,&mode , 0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : setup timer.");
  }
  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" extention  | xx1xxxxx ");
    //
    mask = 0b00100000;
    value = 0b00100000;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      Serial.println("2.8 Fail");
      testResults[TEST_TIMER][7]=TEST_FAIL;
    } else {
      Serial.println("2.8 Success");
      testResults[TEST_TIMER][7]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for timer.");
  delay(70000);
  Serial.println("2.9. check minute timer result.");
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" flag       | xx1xxxxx ");
    //
    mask = 0b00100000;
    value = 0b00100000;
    if (!rtc.checkRegValues(0x0E, mask, value)) {
      Serial.println("Error: control reg (0x0E) is unmatch.");
      Serial.println("2.9 Fail");
      testResults[TEST_TIMER][8]=TEST_FAIL;
    } else {
      Serial.println("2.9 Success");
      testResults[TEST_TIMER][8]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // タイマ停止
  Serial.println("2.10. stop timer. ");
  //
  //
  flag=rtc.controlTimer(1, 0 );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop timer.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" control    | xx1xxxxx ");
    //
    mask = 0b00100000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      Serial.println("2.10 Fail");
      testResults[TEST_TIMER][9]=TEST_FAIL;
    } else {
      Serial.println("2.10 Success");
      testResults[TEST_TIMER][9]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }

}

/*
 * アラームのテスト
 */
void testAlarm(void) {
  Serial.println("3. alarm test.");
#ifdef DEBUG
  uint8_t mask, min, hour, dweek, value;
#endif /* DEBUG */
  date_t date;
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  //日付で判定するモードで具体的な時刻指定
  date_t timing;
  //
  Serial.println("3.1. day alarm setting");
  //
  // 現在時刻の1分後
  timing.minute=date.minute+1;
  timing.hour=date.hour;
  timing.mday=date.mday;
  alarm_mode_t mode;
  mode.type=1; // 日付で判定
  Serial.println(" alarm day test.");
  int flag=rtc.setAlarm(0,&mode, &timing);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set alarm day , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set alarm day.");
  }
#ifdef DEBUG
  min  = rtc.intToBCD(timing.minute);
  hour = rtc.intToBCD(timing.hour);
  dweek= rtc.intToBCD(timing.mday);
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" num | register   |  val(bin) ");
    Serial.println(" ----+------------+--------- ");
    Serial.print(  "  08 | min  alarm | ");Serial.println(min,BIN);
    Serial.print(  "  09 | hour alarm | ");Serial.println(hour,BIN);
    Serial.print(  "  0A | day  alarm | ");Serial.println(dweek,BIN);
    Serial.println("  0D | extention  | x1xxxxxx");
    Serial.println("  0F | contorl    | xxxx1xxx");
    //
    mask=0b11111111;
    if (!rtc.checkRegValues(0x08, mask, min)) {
      Serial.println("Error: min alarm reg (0x08) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(0x09, mask, hour)) {
      Serial.println("Error: hour alarm reg (0x09) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(0x0A, mask, dweek)) {
      Serial.println("Error: week/day alarm reg (0x0A) is unmatch.");
      flag=false;
    }
    //
    mask = 0b01000000;
    value = 0b01000000;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      flag=false;
    }
    mask = 0b00001000;
    value = 0b00001000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.1 Success");
      testResults[TEST_ALARM][0]=TEST_SUCCESS;
    } else {
      Serial.println("3.1 Fail");
      testResults[TEST_ALARM][0]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for alarm.");
  delay(70000);
  Serial.println("3.2. day alarm result.");
  uint16_t intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" flag       | xxxx1xxx ");
    //
    mask = 0b00001000;
    value = 0b00001000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0E) is unmatch.");
      Serial.println("3.2 Fail");
      testResults[TEST_ALARM][1]=TEST_FAIL;
    } else {
      Serial.println("3.2 Success");
      testResults[TEST_ALARM][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  intr=rtc.checkInterupt();
  printInterrupt(intr);
  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */
  Serial.println(" alarm day test done.");

  // alarmをOFF
  Serial.println("3.3. stop day alarm.");
  //
  flag=rtc.controlAlarm(0,0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop alarm day , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop alarm day.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" control    | xxxx0xxx ");
    //
    mask = 0b00001000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      Serial.println("3.3 Fail");
      testResults[TEST_ALARM][2]=TEST_FAIL;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_ALARM][2]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  //
  Serial.println("3.4. set week alarm.");
  //
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);
  //日付で判定するモードで具体的な時刻指定
  // 現在時刻の1分後
  timing.minute=date.minute+1;
  timing.hour=date.hour;
  timing.wday=0xFF; // 曜日未指定(all)を選択
  mode.type=0; // 週で判定
  Serial.println(" alarm week test.");
  flag=rtc.setAlarm(0,&mode, &timing);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set alarm week , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set alarm week.");
  }
#ifdef DEBUG
  min  = rtc.intToBCD(timing.minute);
  hour = rtc.intToBCD(timing.hour);
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" num | register   |  val(bin) ");
    Serial.println(" ----+------------+--------- ");
    Serial.print(  "  08 | min  alarm | ");Serial.println(min,BIN);
    Serial.print(  "  09 | hour alarm | ");Serial.println(hour,BIN);
    Serial.println("  0A | week alarm | 10000000");
    Serial.println("  0D | extention  | x0xxxxxx");
    Serial.println("  0F | contorl    | xxxx1xxx");
    //
    mask=0b11111111;
    if (!rtc.checkRegValues(0x08, mask, min)) {
      Serial.println("Error: min alarm reg (0x08) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(0x09, mask, hour)) {
      Serial.println("Error: hour alarm reg (0x09) is unmatch.");
      flag=false;
    }
    dweek=0b10000000;
    if (!rtc.checkRegValues(0x0A, mask, dweek)) {
      Serial.println("Error: week/day alarm reg (0x0A) is unmatch.");
      flag=false;
    }
    //
    mask = 0b01000000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0D, mask, value)) {
      Serial.println("Error: extension reg (0x0D) is unmatch.");
      flag=false;
    }
    mask = 0b00001000;
    value = 0b00001000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.4 Success");
      testResults[TEST_ALARM][3]=TEST_SUCCESS;
    } else {
      Serial.println("3.4 Fail");
      testResults[TEST_ALARM][3]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for alarm.");
  delay(70000);
  Serial.println("3.5. day alarm result.");
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" flag       | xxxx1xxx ");
    //
    mask = 0b00001000;
    value = 0b00001000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0E) is unmatch.");
      Serial.println("3.5 Fail");
      testResults[TEST_ALARM][4]=TEST_FAIL;
    } else {
      Serial.println("3.5 Success");
      testResults[TEST_ALARM][4]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  intr=rtc.checkInterupt();
  printInterrupt(intr);
  if (RTC_U_SUCCESS==rtc.clearInterupt(7)) {
    Serial.println("clear interrupt success.");
  } else {
    Serial.println("clear interrupt fail.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */
  Serial.println(" alarm week test done.");

  // alarmをOFF
  Serial.println("3.6. day alarm result.");
  // alarmをOFF
  flag=rtc.controlAlarm(0,0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop alarm week , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop alarm week.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    bool flag=true;
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" control    | xxxx0xxx ");
    //
    mask = 0b00001000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      Serial.println("3.6 Fail");
      testResults[TEST_ALARM][5]=TEST_FAIL;
    } else {
      Serial.println("3.6 Success");
      testResults[TEST_ALARM][5]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 温度による発振周波数の制御機能のテスト
 */
void testTemperature(void) {
#ifdef DEBUG
  uint8_t mask, value;
#endif /* DEBUG */
  Serial.println("4. temperature based oscillator control function test.");
  // CSEL ビットに値を設定
  Serial.println("4.1. set csel bits of control register."); //デフォルト値は0b01
  int flag = rtc.setTemperatureFunction(0b10);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set csel bits, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set csel bits.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- temperature function -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" control    | 10xxxxxx ");
    //
    mask = 0b11000000;
    value = 0b10000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      Serial.println("4.1 Fail");
      testResults[TEST_TEMPERATURE][0]=TEST_FAIL;
    } else {
      Serial.println("4.1 Success");
      testResults[TEST_TEMPERATURE][0]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  // CSEL ビットに値を設定
  Serial.println("4.2. set csel bits of control register."); //デフォルト値は0b01
  flag = rtc.setTemperatureFunction(0b0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set csel bits, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set csel bits.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- temperature function -- ");
    Serial.println(" register   |  val(bin) ");
    Serial.println(" -----------+--------- ");
    Serial.println(" control    | 00xxxxxx ");
    //
    mask = 0b11000000;
    value = 0b00000000;
    if (!rtc.checkRegValues(0x0F, mask, value)) {
      Serial.println("Error: control reg (0x0F) is unmatch.");
      Serial.println("4.2 Fail");
      testResults[TEST_TEMPERATURE][1]=TEST_FAIL;
    } else {
      Serial.println("4.2 Success");
      testResults[TEST_TEMPERATURE][1]=TEST_SUCCESS;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  float temperature = rtc.getTemperature(RTC_U_TEMPERATURE_CELCIUS);
  Serial.print(" rtc temperature = ");Serial.print(temperature);Serial.println("C");
}

void testClock(void) {
  Serial.println("5. reset clock.");
  // CSEL ビットに値を設定
  Serial.println("5.1 set reset flag to control register (0x0F).");
  int flag = rtc.controlClock();
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set reset bit of control register, retval = ");Serial.println(flag);
#ifdef DEBUG
    Serial.println("5.1 Fail");
    testResults[TEST_CLOCK][0]=TEST_FAIL;
#endif /* DEBUG */
  } else {
    Serial.println("Success : set reset bit of control register.");
#ifdef DEBUG
    Serial.println("5.1 Success");
    testResults[TEST_CLOCK][0]=TEST_SUCCESS;
#endif /* DEBUG */
  }
}


#ifdef DEBUG
void calcResult(void){
  //
  Serial.println("========== check all test result =========");
  //
  int success=0, fail=0, not_run=0;
  for (int i=0; i< TEST_FREQ_COLS ;i++) {
    if (testResults[TEST_FREQ][i]==TEST_NOT_RUN) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_FREQ][i]==TEST_FAIL) {
      Serial.print("test 1.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TIMER_COLS ;i++) {
    if (testResults[TEST_TIMER][i]==TEST_NOT_RUN) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TIMER][i]==TEST_FAIL) {
      Serial.print("test 2.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_ALARM_COLS ;i++) {
    if (testResults[TEST_ALARM][i]==TEST_NOT_RUN) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_ALARM][i]==TEST_FAIL) {
      Serial.print("test 3.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_TEMPERATURE_COLS ;i++) {
    if (testResults[TEST_TEMPERATURE][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_TEMPERATURE][i]==TEST_FAIL) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_CLOCK_COLS ;i++) {
    if (testResults[TEST_CLOCK][i]==TEST_NOT_RUN) {
      Serial.print("test 5.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_CLOCK][i]==TEST_FAIL) {
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

void setup(void) {
  Serial.begin(9600) ;         // シリアル通信の初期化
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

  int power_flag=rtc.checkLowPower();
  if (0 > power_flag) {
    Serial.print("Error: fail to get power flag , retval = ");Serial.println(power_flag);
  } else {
    Serial.print("Success: success to get power flag , retval = ");Serial.println(power_flag, BIN);
    if (power_flag>0) Serial.println("Caution: low power event detected.");
  }
#if defined(DEBUG) && defined(DUMP_REGISTER)
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG && DUMP_REGISTER */
  int reset_rst =rtc.clearPowerFlag();
  if (RTC_U_SUCCESS!=reset_rst) {
    Serial.print("Error: fail to clear power flag , retval = ");Serial.println(reset_rst);
  } else {
    Serial.println("Success : clear power flag.");
  }
#if defined(DEBUG) && defined(DUMP_REGISTER)
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG && DUMP_REGISTER */
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

  Serial.println("setting time.");
  printTime(&date);

  /* RTCに現在時刻を設定 */
  if (!rtc.setTime(&date)) {
    Serial.print("set time to RTC fail.") ;     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
#if defined(DEBUG) && defined(DUMP_REGISTER)
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG && DUMP_REGISTER */

  Serial.println("current time.");
  if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  else printTime(&date);

  Serial.println("==================  function test start. ==================");
  Serial.println("++++++++++++++++++  freq out ++++++++++++++++++");
  testFreqOut();
  Serial.println("++++++++++++++++++  timer ++++++++++++++++++");
  testTimer();
  Serial.println("++++++++++++++++++  alarm ++++++++++++++++++");
  testAlarm();
  Serial.println("++++++++++++++++++  temperature ++++++++++++++++++");
  testTemperature();
  Serial.println("++++++++++++++++++  clock reset ++++++++++++++++++");
  testClock();
#ifdef DEBUG
  calcResult();
#endif /* DEBUG */
  Serial.println("==================  all function test done. ==================");
  delay(10000);
  Serial.println("==================  long run test  ==================");
}

void loop(void) {
  date_t date;
  //if (!rtc.getTime(&date)) Serial.println("Error : can not get time data.");
  //else printTime(&date);
  delay(1000) ;            // １秒後に繰り返す
}
