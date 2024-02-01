
#include "RTC_RX8025_U.h"
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
#define TEST_TIMER_COLS 7
#define TEST_ALARM_COLS 8
#define TEST_LOW_POWER_COLS 2
#define TEST_OSCILLATOR_COLS 2

uint8_t maskFreq[]={
0b00010000
};

uint8_t valueFreq[]={
0b00000000
};

uint8_t maskTimer[]={
0b00000111,
0b00000100,
0b00000111,
0b00000111,
0b00000111,
0b00000100,
0b00000111
};

uint8_t maskAlarm[]={
0b10000000,
0b00000010,
0b00000111,
0b10000000,
0b01000000,
0b00000001,
0b00000111,
0b01000000
};

uint8_t maskAlarm2[]={
0b00000010,
0,
0,
0,
0b00000001,
0,
0,
0
};

uint8_t maskOscillator[]={
0b01111111,
0b01111111
};

uint8_t valueTimer[]={
0b00000100,
0b00000100,
0b00000000,
0b00000000,
0b00000101,
0b00000100,
0b00000000
};

uint8_t valueAlarm[]={
0b10000000,
0b00000010,
0b00000000,
0b00000000,
0b01000000,
0b00000001,
0b00000000,
0b00000000
};

uint8_t valueAlarm2[]={
0b00000000,
0,
0,
0,
0b00000000,
0,
0,
0
};

uint8_t valueOscillator[]={
  0b00010101,
  0b00000000
};

enum {
  TEST_FREQ,
  TEST_TIMER,
  TEST_ALARM,
  TEST_OSCILLATOR,
  TEST_LOW_POWER
};

enum{
TEST_FREQ_OUTPUT_SETTING
};

enum{
TEST_TIMER_1SEC_SETTING,
TEST_TIMER_1SEC_CHECK,
TEST_TIMER_1SEC_STOP,
TEST_TIMER_1MIN_SETTING,
TEST_TIMER_1MIN_START,
TEST_TIMER_1MIN_CHECK,
TEST_TIMER_1MIN_STOP,
};

enum{
TEST_ALARM_W_1MIN_SETTING,
TEST_ALARM_W_1MIN_CHECK,
TEST_ALARM_W_1MIN_CLEAR,
TEST_ALARM_W_1MIN_STOP,
TEST_ALARM_D_1MIN_SETTING,
TEST_ALARM_D_1MIN_CHECK,
TEST_ALARM_D_1MIN_CLEAR,
TEST_ALARM_D_1MIN_STOP
};

enum{
TEST_OSCILLATOR_SETUP,
TEST_OSCILLATOR_DISABLE
};

enum{
  TEST_LOW_POWER_SETUP1,
  TEST_LOW_POWER_SETUP2
};

uint8_t testResults[TEST_LINE][TEST_COL];

#endif /* DEBUG */

RTC_RX8025_U rtc = RTC_RX8025_U(&Wire);

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
 * 周波数出力機能のテスト
 */
void testFreqOut(void){
  Serial.println("1. freq signal output test.");
  //
  // 周波数信号を出力する設定
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
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxx0xxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskFreq[TEST_FREQ_OUTPUT_SETTING], valueFreq[TEST_FREQ_OUTPUT_SETTING])) {
      Serial.println("1.1 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_FAIL ;
    } else {
      Serial.println("1.1 Success");
      testResults[TEST_FREQ][TEST_FREQ_OUTPUT_SETTING]=TEST_SUCCESS ;
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
}

/*
 * タイマのテスト
 */
void testTimer(void) {
  Serial.println("2. timer test.");
  rtc_timer_mode_t timer;
  Serial.println("2.1.  1 sec timer setting.");
  timer.interval=0b100; // レベルモード 秒1回
  int flag=rtc.setTimer((uint8_t)0,&timer , (uint8_t)0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : setup timer.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxxxx100 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskTimer[TEST_TIMER_1SEC_SETTING], valueTimer[TEST_TIMER_1SEC_SETTING])) {
      Serial.println("2.1 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_SETTING]=TEST_FAIL ;
    } else {
      Serial.println("2.1 Success");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("wait for timer.");delay(2000);
  Serial.println("2.2. timer interupt check");
  int intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxx1xx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskTimer[TEST_TIMER_1SEC_CHECK], valueTimer[TEST_TIMER_1SEC_CHECK])) {
      Serial.println("2.2 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_CHECK]=TEST_FAIL ;
    } else {
      Serial.println("2.2 Success");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_CHECK]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

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
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxxxx000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskTimer[TEST_TIMER_1SEC_STOP], valueTimer[TEST_TIMER_1SEC_STOP])) {
      Serial.println("2.3 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_STOP]=TEST_FAIL ;
    } else {
      Serial.println("2.3 Success");
      testResults[TEST_TIMER][TEST_TIMER_1SEC_STOP]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("2.4. set timer mode to level mode 1 min. ");
  timer.interval=0b101; // レベルモード 1 min.
  flag=rtc.setTimerMode(0,&timer );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to change timer mode, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : change timer mode.");
  }
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxxxx000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskTimer[TEST_TIMER_1MIN_SETTING], valueTimer[TEST_TIMER_1MIN_SETTING])) {
      Serial.println("2.4 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_SETTING]=TEST_FAIL ;
    } else {
      Serial.println("2.4 Success");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_SETTING]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("2.5 start timer. ");
  flag=rtc.controlTimer(0, 1 );
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to start timer, retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : start timer.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxxxx101 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskTimer[TEST_TIMER_1MIN_START], valueTimer[TEST_TIMER_1MIN_START])) {
      Serial.println("2.5 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_START]=TEST_FAIL ;
    } else {
      Serial.println("2.5 Success");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_START]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */


  Serial.println("wait for timer.");delay(70000);
  Serial.println("2.6. timer interupt check");
  intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxx1xx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskTimer[TEST_TIMER_1MIN_CHECK], valueTimer[TEST_TIMER_1MIN_CHECK])) {
      Serial.println("2.6 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_CHECK]=TEST_FAIL ;
    } else {
      Serial.println("2.6 Success");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_CHECK]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("2.7 stop timer. ");
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
    //
    Serial.println(" -- timer setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | xxxxx000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskTimer[TEST_TIMER_1MIN_STOP], valueTimer[TEST_TIMER_1MIN_STOP])) {
      Serial.println("2.7 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_STOP]=TEST_FAIL ;
    } else {
      Serial.println("2.7 Success");
      testResults[TEST_TIMER][TEST_TIMER_1MIN_STOP]=TEST_SUCCESS ;
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
void testAlarm(void){
  Serial.println("3. alarm test.");
  date_t currentTime;
  rtc.getTime(&currentTime);

  Serial.println("current time.");
  printTime(&currentTime);

  date_t timing;
  timing.minute=currentTime.minute+1;
  timing.hour=currentTime.hour;
  timing.wday=0b01111111;

  alarm_mode_t alarm;
  int flag;
  Serial.println("3.1. alarm_W test.");
  // alarm_Wを設定
  flag=rtc.setAlarm(0,&alarm, &timing);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set alarm_W , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set alarm_W.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    uint8_t min, hour;
    //
    min  = rtc.intToBCD(timing.minute);
    hour = rtc.intToBCD(timing.hour);
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ----------------+---+--------- ");
    Serial.print  (" alarm_w min     | 8 | ");Serial.println(min,BIN);
    Serial.print  (" alarm_w hour    | 9 | ");Serial.println(hour,BIN);
    Serial.print  (" alarm_w weekday | A | ");Serial.println(timing.wday,BIN);
    Serial.println(" control1        | E | 1xxxxxxx ");
    Serial.println(" control2        | F | xxxxxx0x ");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_ALARM_W_MINUTE, 0b11111111, min)) {
      Serial.println("Error: alarm_w min reg (0x8) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_ALARM_W_HOUR, 0b11111111, hour)) {
      Serial.println("Error: alarm_w hour reg (0x9) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_ALARM_W_WEEKDAY, 0b11111111, timing.wday)) {
      Serial.println("Error: alarm_w weekday reg (0xA) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskAlarm[TEST_ALARM_W_1MIN_SETTING], valueAlarm[TEST_ALARM_W_1MIN_SETTING])) {
      Serial.println("Error: control-1 reg (0xE) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm2[TEST_ALARM_W_1MIN_SETTING], valueAlarm2[TEST_ALARM_W_1MIN_SETTING])) {
      Serial.println("Error: control-2 reg (0xF) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.1 Success");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_SETTING]=TEST_SUCCESS;
    } else {
      Serial.println("3.1 Fail");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_SETTING]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("wait for alarm.");
  delay(70000);
  Serial.println("3.2. alarm_w result.");
  int intr=rtc.checkInterupt();
  printInterrupt(intr);
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxxx1x ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm[TEST_ALARM_W_1MIN_CHECK], valueAlarm[TEST_ALARM_W_1MIN_CHECK])) {
      Serial.println("3.2 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_CHECK]=TEST_FAIL ;
    } else {
      Serial.println("3.2 Success");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_CHECK]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("3.3 clear interupt flag.");
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
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxx000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm[TEST_ALARM_W_1MIN_CLEAR], valueAlarm[TEST_ALARM_W_1MIN_CLEAR])) {
      Serial.println("3.3 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_CLEAR]=TEST_FAIL ;
    } else {
      Serial.println("3.3 Success");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_CLEAR]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  
  // alarm_WをOFF
  Serial.println("3.4 stop alarm_w.");
  flag=rtc.controlAlarm(0,0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop alarm_W , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop alarm_W.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | 0xxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskAlarm[TEST_ALARM_W_1MIN_STOP], valueAlarm[TEST_ALARM_W_1MIN_STOP])) {
      Serial.println("3.4 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_STOP]=TEST_FAIL ;
    } else {
      Serial.println("3.4 Success");
      testResults[TEST_ALARM][TEST_ALARM_W_1MIN_STOP]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println(" alarm_W test done.");

  rtc.getTime(&currentTime);
  timing.minute=currentTime.minute+1;
  timing.hour=currentTime.hour;

  // alarm_Dを設定
  Serial.println(" alarm_D test.");
  Serial.println("3.5 alarm_D test setting.");
  flag=rtc.setAlarm(1,&alarm, &timing);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to set alarm_D , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : set alarm_D.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    uint8_t min, hour;
    //
    min  = rtc.intToBCD(timing.minute);
    hour = rtc.intToBCD(timing.hour);
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register        | val(bin) ");
    Serial.println(" ----------------+---+--------- ");
    Serial.print  (" alarm_w min     | B | ");Serial.println(min,BIN);
    Serial.print  (" alarm_w hour    | C | ");Serial.println(hour,BIN);
    Serial.println(" control1        | E | x1xxxxxx ");
    Serial.println(" control2        | F | xxxxxxx0 ");
    //
    bool flag=true;
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_ALARM_D_MINUTE, 0b11111111, min)) {
      Serial.println("3.5 Error: alarm_w min reg (0xB) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_ALARM_D_HOUR, 0b11111111, hour)) {
      Serial.println("3.5 Error: alarm_w hour reg (0xC) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskAlarm[TEST_ALARM_D_1MIN_SETTING], valueAlarm[TEST_ALARM_D_1MIN_SETTING])) {
      Serial.println("3.5 Error: control-1 reg (0xE) is unmatch.");
      flag=false;
    }
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm2[TEST_ALARM_D_1MIN_SETTING], valueAlarm2[TEST_ALARM_D_1MIN_SETTING])) {
      Serial.println("3.5 Error: control-2 reg (0xF) is unmatch.");
      flag=false;
    }
    if (flag) {
      Serial.println("3.5 Success");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_SETTING]=TEST_SUCCESS;
    } else {
      Serial.println("3.5 Fail");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_SETTING]=TEST_FAIL;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */
  Serial.println("wait for alarm.");
  delay(70000);
  Serial.println("3.6 alarm_d result check.");
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxxxx1 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm[TEST_ALARM_D_1MIN_CHECK], valueAlarm[TEST_ALARM_D_1MIN_CHECK])) {
      Serial.println("3.6 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_CHECK]=TEST_FAIL ;
    } else {
      Serial.println("3.6 Success");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_CHECK]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("3.7 clear interupt flag.");
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
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | xxxxx000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, maskAlarm[TEST_ALARM_D_1MIN_CLEAR], valueAlarm[TEST_ALARM_D_1MIN_CLEAR])) {
      Serial.println("3.7 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_CLEAR]=TEST_FAIL ;
    } else {
      Serial.println("3.7 Success");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_CLEAR]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // alarm_DをOFF
  Serial.println("3.8 stop alarm_D.");
  flag=rtc.controlAlarm(1,0);
  if (RTC_U_SUCCESS!=flag) {
    Serial.print("Error: fail to stop alarm_D , retval = ");Serial.println(flag);
  } else {
    Serial.println("Success : stop alarm_D.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- alarm setting -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control1   | E | x0xxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL1, maskAlarm[TEST_ALARM_D_1MIN_STOP], valueAlarm[TEST_ALARM_D_1MIN_STOP])) {
      Serial.println("3.8 Error: control-1 reg (0xE) is unmatch.");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_STOP]=TEST_FAIL ;
    } else {
      Serial.println("3.8 Success");
      testResults[TEST_ALARM][TEST_ALARM_D_1MIN_STOP]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println(" alarm_D test done.");
}

/*
 * 時計の内部発振器の周波数設定
 */
void testOscillator(void) {
  uint8_t mode=0b10101;

  // 発振器の周波数を調整する機能をONにする
  Serial.println("4.1 setup oscillator.");
  int flag=rtc.setOscillator(mode);

#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- oscillator control -- ");
    Serial.println(" register           | val(bin) ");
    Serial.println(" ---------------+---+--------- ");
    Serial.println(" digital offset | 7 | 00010101 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_DIGITAL_OFFSET, maskOscillator[TEST_OSCILLATOR_SETUP], valueOscillator[TEST_OSCILLATOR_SETUP])) {
      Serial.println("4.1 Error: digital offset reg (0x7) is unmatch.");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_SETUP]=TEST_FAIL ;
    } else {
      Serial.println("4.1 Success");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_SETUP]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  // 発振器の周波数を調整する機能をONにする
  Serial.println("4.2 disable oscillator control.");
  mode=0;
  flag=rtc.setOscillator(mode);

#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- oscillator control -- ");
    Serial.println(" register           | val(bin) ");
    Serial.println(" ---------------+---+--------- ");
    Serial.println(" digital offset | 7 | 00000000 ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_DIGITAL_OFFSET, maskOscillator[TEST_OSCILLATOR_DISABLE], valueOscillator[TEST_OSCILLATOR_DISABLE])) {
      Serial.println("4.1 Error: digital offset reg (0x7) is unmatch.");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_DISABLE]=TEST_FAIL ;
    } else {
      Serial.println("4.1 Success");
      testResults[TEST_OSCILLATOR][TEST_OSCILLATOR_DISABLE]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

void testLowPower(void) {
  Serial.println("5. low power detection parameter test");
  Serial.println("5.1 set threshold value to 1.");
  int result=rtc.setLowPower(1);
  if (RTC_U_SUCCESS!=result) {
    Serial.print("Error: fail to set low power threshold , retval = ");Serial.println(result);
  } else {
    Serial.println("Success : set low power threshold.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- power related function -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | 1xxxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, 0b10000000 , 0b10000000)) {
      Serial.println("5.1 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_LOW_POWER][TEST_LOW_POWER_SETUP1]=TEST_FAIL ;
    } else {
      Serial.println("5.1 Success");
      testResults[TEST_LOW_POWER][TEST_LOW_POWER_SETUP1]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

  Serial.println("5.2 set threshold value to 0.");
  result=rtc.setLowPower(0);
  if (RTC_U_SUCCESS!=result) {
    Serial.print("Error: fail to set low power threshold , retval = ");Serial.println(result);
  } else {
    Serial.println("Success : set low power threshold.");
  }
#ifdef DEBUG
  if (!rtc.backupRegValues()) {
    Serial.println("Error: fail to get register values.");
  } else {
    //
    Serial.println(" -- power related function -- ");
    Serial.println(" register       | val(bin) ");
    Serial.println(" -----------+---+--------- ");
    Serial.println(" control2   | F | 0xxxxxxxx ");
    //
    if (!rtc.checkRegValues(RTC_EPSON_RX8025_REG_CONTROL2, 0b10000000 , 0b00000000)) {
      Serial.println("5.1 Error: control-2 reg (0xF) is unmatch.");
      testResults[TEST_LOW_POWER][TEST_LOW_POWER_SETUP2]=TEST_FAIL ;
    } else {
      Serial.println("5.1 Success");
      testResults[TEST_LOW_POWER][TEST_LOW_POWER_SETUP2]=TEST_SUCCESS ;
    }
  }
#ifdef DUMP_REGISTER
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DUMP_REGISTER */
#endif /* DEBUG */

}

/*
 * 割り込み端子の情報の出力
 */
void printInterrupt(int val) {
  if (val <0) {
    Serial.println(" Error : illegal interrupt flag value. ");
    return;
  }
  uint8_t reg=(uint8_t) val;
  uint8_t TIMER, ALARM_W, ALARM_D;
  TIMER=reg&0b100;
  if (TIMER>0) TIMER=1;
  ALARM_W=reg&0b10;
  if (ALARM_W>0) ALARM_W=1;
  ALARM_D=reg&0b1;
  if (ALARM_D>0) ALARM_D=1;
  Serial.print("TIMER = ");Serial.print(TIMER);Serial.print(", ALARM_W = ");Serial.print(ALARM_W);Serial.print(" , ALARM_D = ");Serial.println(ALARM_D);
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

  for (int i=0; i< TEST_OSCILLATOR_COLS ;i++) {
    if (testResults[TEST_OSCILLATOR][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_OSCILLATOR][i]==TEST_FAIL) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is failed.");
      fail++;
    } else {
      success++;
    }
  }

  for (int i=0; i< TEST_LOW_POWER_COLS ;i++) {
    if (testResults[TEST_LOW_POWER][i]==TEST_NOT_RUN) {
      Serial.print("test 4.");Serial.print(i+1);Serial.println(" is skipped.");
      not_run++;
    } else if (testResults[TEST_LOW_POWER][i]==TEST_FAIL) {
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
 * setup
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
    if (0b010!=power_flag) Serial.println("Caution: low power event detected.");
  }
  int reset_rst =rtc.clearPowerFlag();
  if (RTC_U_SUCCESS!=reset_rst) {
    Serial.print("Error: fail to clear power flag , retval = ");Serial.println(reset_rst);
  } else {
    Serial.println("Success : clear power flag.");
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

  Serial.println("setting time.");
  printTime(&date);

  /* RTCに現在時刻を設定 */
  if (!rtc.setTime(&date)) {
    Serial.print("set time to RTC fail.") ;     // 初期化失敗
    while(1);
  } else {
    Serial.println("set time to RTC success."); // 初期化成功
  }
#ifdef DEBUG
  rtc.backupRegValues();
  rtc.dumpReg();
  Serial.println("*************************************************************");
#endif /* DEBUG */

  Serial.println("current time.");
  rtc.getTime(&date);
  printTime(&date);

  Serial.println("==================  freq out ==================");
  testFreqOut();
  Serial.println("==================  timer ==================");
  testTimer();
  Serial.println("==================  alarm ==================");
  testAlarm();
  Serial.println("==================  oscillator ==================");
  testOscillator();
  Serial.println("==================  low power ==================");
  testLowPower();
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
void loop() {
  date_t date;
  rtc.getTime(&date);
  //printTime(&date);
  delay(1000) ;            // １秒後に繰り返す
}
