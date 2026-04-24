# RTC_U
This library is a combination of API definitions—created with the thought that “programming might be easier if the RTC API could be standardized,” inspired by the [AdafruitUSD] project, which unifies interfaces for various sensors—and an Arduino library (device driver) built using an RTC I happened to have on hand.

## 1. About Licensing
Please check the contents of each directory for the license of each the RTC libraries.
Since each driver modifies the interface portion of code created by others, it inherits the license terms of the original source code.

## 2.Supported RTCs
|Vender| Model Number |
| :--- | :--- |
|Epson|[RTC 8564NB][RTC8564NB]|
|Epson|[RTC4543][RTC4543]|
|Epson|[RX8025][RX8025]|
|Epson|[RX8900][RX8900]|
|MAXIM|[DS1307][DS1307]|
|MAXIM|[DS3234][DS3234]|
|MAXIM|[DS3231][DS3231]|
|Micro Crystal|[RV8803][RV8803]|
|NXP|[PCF8523][PCF8523]|

## 3. Usage Notes
Since I created this as a personal weekend hobby project, I haven’t been able to perform comprehensive testing on each driver.
Also, since I don’t own a logic analyzer or similar equipment, I’m unable to debug any issues with SPI or I2C-level behavior, so those issues remain unresolved.

## 4. How to Use
First, place this directory in the Arduino development environment’s library directory.
Next, move (or copy) the directory for the RTC you wish to use from the `driver` directory to the library directory as well.

Please install the [dateUtils library](https://github.com/houtbrion/dateUtils).

You are now ready to proceed.

# API Manual
Since the specific details of operation vary depending on the individual RTC, please refer to the README.md file or the RTC datasheet for each RTC driver (library) located in the driver directory.

## 1. Structure Definitions, etc.
### 1.1. Structures Storing RTC Functions
```
typedef struct {
    uint32_t    type;                   // Chip type
    uint8_t     numOfInterruptPin;       // Number of interrupt pins
    uint8_t     numOfAlarm;             // Number of configurable alarms
    uint8_t     numOfTimer;             // Number of configurable timers
    uint8_t     numOfClockOut;          // Number of pins outputting clock frequency
    uint8_t     numOfYearDigits;        // Number of digits in the year register
    bool        haveYearOverflowBit;    // Whether a flag is present to indicate overflow when the year register is 2 digits
    bool        haveMilliSec;           // Whether millisecond functionality is available
    bool        independentSQW;         // Whether the interrupt pin and frequency output pin are independent
    bool        detectLowBattery;       // Whether functionality is available to detect power loss, low battery voltage, or a switch to battery power
    bool        controlOscillator;      // Whether the clock speed adjustment feature is available
} rtc_u_info_t;
```
### 1.2. Timer Operating Modes
```
typedef struct {
    uint8_t  pulse;
    uint8_t  repeat;
    uint8_t  useInterruptPin;
    uint8_t  interval;
} rtc_timer_mode_t;
```

### 1.3. Alarm Operation Modes
```
typedef struct {
    uint8_t  useInterruptPin;
    uint8_t  type;
} alarm_mode_t;
```

### 1.4. RTC Types
```
enum {
    EPSON8564NB  = 1,
    EPSON_RX8025,
    EPSON_RX8900,
    EPSON_4543SA,
    DS1307,
    DS3231,
    DS3234,
    PCF8523,
    RV8803
};
```

### 1.5. Features for RTCs that Perform Special Operations in Response to External Signal Inputs
```
typedef struct {
    uint8_t  useInterruptPin;
    bool capture;
    uint8_t level;
    uint8_t filter;
    bool reset;
} event_mode_t;
```
This data type is necessary to support RTCs etc... that have the capability to capture a snapshot of the current time by applying a voltage to a specific pin or similar means.

### 1.6 Reference: Data Types in dateUtils.h
The data types used in “dateUtils.h” by this library are as follows.

```
enum {
  SUN = 0,
  MON = 1,
  TUE = 2,
  WED = 3,
  THU = 4,
  FRI = 5,
  SAT = 6
};
```

```
typedef struct  {
  uint16_t  year;
  uint8_t   month;
  uint8_t   mday;
  uint8_t   wday;
  uint8_t   hour;
  uint8_t   minute;
  uint8_t   second;
  int16_t   millisecond;
} date_t;
```

## 2. Member Functions

Among the member functions described below, some functions—other than those for setting and retrieving the time—may not be available depending on the type of RTC. Therefore, if a function is called on an RTC that does not support it, the function will return `RTC_U_UNSUPPORTED` without performing any action, regardless of the arguments. Please refer to the README.md file for each RTC driver to determine which functions are not supported by which RTCs.

### 2.1. General

A member function that retrieves information about the RTC chip type and features.
```
void getRtcInfo(rtc_u_info_t *info)
```

This is the RTC initialization function. Since the arguments vary depending on the RTC type, refer to the respective drivers for specific details.
```
bool begin(bool init=true)
```
If `true` is specified as the argument (default), the time (initial value) is set. If you encounter issues where previously set times are lost when the Arduino restarts, specify `false` as the argument.

| Return Value | Meaning |
|---|---|
|true|Initialization successful|
|false|Initialization failed|

### 2.2. Setting the Time
Sets the time specified in the argument to the RTC. Note that this library (and the drivers for each RTC) does not support the 12-hour clock (AM/PM).
```
bool setTime(date_t* time)
```
| Return Value | Meaning |
|---|---|
|true|Set successfully|
|false|Set failed|

### 2.3. Getting the Time
Writes the time information retrieved from the RTC to the structure provided as an argument.
```
bool getTime(date_t* time)
```
| Return Value | Meaning |
|---|---|
|true|Retrieved successfully|
|false|Retrieval failed|

### 2.4. Alarm-Related Functions
Sets the timing and mode for the alarm specified by the number `num`. Since the values and meanings of `mode` vary depending on the RTC, refer to the respective drivers for details.
```
int setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS |Set successfully|
|RTC_U_FAILURE |Set failed|
|RTC_U_ILLEGAL_PARAM |An unsupported alarm number or other invalid parameter was specified|

Sets the operation mode (mode) for the alarm with the number specified by `num`. Since the value and meaning of `mode` vary depending on the RTC, refer to the respective drivers for details.
```
int setAlarmMode(uint8_t num, alarm_mode_t * mode)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS |Set successfully|
|RTC_U_FAILURE |Set failed|
|RTC_U_ILLEGAL_PARAM |An unsupported alarm number or other parameter was specified|

A function to start, stop, or pause the alarm with the number specified by `num`. The meaning of `action` and the specific behavior vary depending on the RTC type; refer to the respective drivers.
```
int controlAlarm(uint8_t num, uint8_t action)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |
|RTC_U_ILLEGAL_PARAM | An unsupported alarm number or other parameter was specified |

### 2.5. Timers
This function configures the operation of the timer specified by `num` using `mode` and `multi`. Generally, this means the timer will trigger at the frequency specified by `mode` and at the interval specified by `multi`; however, since this varies depending on the RTC specification, please refer to the respective drivers for details.
```
int setTimer(uint8_t num, rtc_timer_mode_t * mode, uint16_t multi)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS |Set successfully|
|RTC_U_FAILURE |Set failed|
|RTC_U_ILLEGAL_PARAM |Specifying an unsupported alarm number, etc.|

Sets the operating mode of the timer with the number specified by `num`. Since the details of the mode vary depending on the RTC type, please refer to the respective drivers.
```
int setTimerMode(uint8_t num, rtc_timer_mode_t * mode)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS |Configuration successful|
|RTC_U_FAILURE |Configuration failed|
|RTC_U_ILLEGAL_PARAM |Specifying an unsupported alarm number, etc.|

A function to start, stop, or pause the timer with the number specified by `num`. The meaning of `action` and the specific behavior vary depending on the RTC type, so please refer to the respective drivers.
```
int controlTimer(uint8_t num, uint8_t action)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |
|RTC_U_ILLEGAL_PARAM | Specified an unsupported alarm number, etc. |

### 2.6. Clock Output
Outputs a signal at the frequency specified by the second argument `freq` to the pin specified by the third argument. The first argument `num` is
```
int setClockOut(uint8_t num, uint8_t freq, int8_t pin)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS |Configuration successful|
|RTC_U_FAILURE |Configuration failed|
|RTC_U_ILLEGAL_PARAM |Specified an unsupported clock, etc.|

A function to change the frequency of the clock output configuration specified by the first argument.
```
int setClockOutMode(uint8_t num, uint8_t freq)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |
|RTC_U_ILLEGAL_PARAM | Specified an unsupported clock, etc. |

A function that controls (starts/stops) the clock output specified by the first argument.
```
int controlClockOut(uint8_t num, uint8_t mode)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |
|RTC_U_ILLEGAL_PARAM | Specified an unsupported clock, etc. |

### 2.7. Interrupts
This function checks whether an interrupt has occurred and returns the interrupt number. Since the return value varies depending on the RTC type, refer to the respective drivers.
```
int checkInterrupt(void)
```

Clears the interrupt flag for the alarm or timer specified by `type`. Since the value of `type` varies depending on the RTC type, refer to the respective drivers.
```
int clearInterrupt(uint16_t type)
```

| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |
|RTC_U_ILLEGAL_PARAM | Specified an unsupported clock, etc. |


### 2.8 Power Supply
It is not uncommon for systems to be configured with a battery to keep the time updated even if the computer’s main power is disconnected.
However, due to factors such as battery life, the RTC often records instances where the battery voltage drops, allowing the system to verify whether a power drop occurred the next time the computer is powered on.

For RTCs equipped with this feature, the following two functions are provided to retrieve the internal flag value indicating a voltage drop and to clear that flag:

```
int checkLowPower(void)
```
This function retrieves the flag value indicating whether a voltage drop has occurred. If the RTC does not have this feature, it returns ``RTC_U_UNSUPPORTED``; if the retrieval fails, it returns ``RTC_U_FAILURE``. Since both of these values are negative, a value of 0 or greater indicates the flag value. The specific meaning of this flag value varies depending on the RTC type, so please refer to the README.md file for each RTC driver.


```
int clearPowerFlag(void)
```
This function is used to clear the flag value that monitors voltage drops.
If the RTC does not have a voltage drop monitoring function, it returns ``RTC_U_UNSUPPORTED``; if writing the flag value fails, it returns ``RTC_U_FAILURE``; and if writing succeeds, it returns ``RTC_U_SUCCESS``.

```
int setLowPower(uint8_t mode)
```
On some RTCs, you can configure parameters related to low-power detection. If the configuration is successful, it returns ``RTC_U_SUCCESS``; if it fails, it returns ``RTC_U_FAILURE``. If the RTC does not support this feature, it returns ``RTC_U_UNSUPPORTED``.

### 2.9 Clock Halt Functions
Some RTCs include a feature to stop the clock update (clock) in order to reduce power consumption. The following member functions have been provided to support this feature.

```
int controlClockHalt(uint8_t mode)
```
Controls the stopping and resuming of the clock on RTCs that support this feature.

|mode value|Meaning|
|-- -|---|
|0| Stop the clock|
|1| Resume the clock|

| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful|
|RTC_U_FAILURE | Configuration failed|
|RTC_U_ILLEGAL_PARAM | Invalid `mode` value|

```
int clockHaltStatus(void)
```
This function indicates whether the clock is stopped or not.
| Return Value | Meaning |
|---|---|
|1| Clock is stopped |
|0| Clock is not stopped |
|RTC_U_FAILURE | Failed to retrieve information |

Translated with DeepL.com (free version)

### 2.10 Time Adjustment
Some RTCs offer adjustment functions other than stopping the clock. For example, the EPSON RX8900 allows you to clear (reset) counters for values smaller than a second. These functions are summarized below. Since the availability and behavior of these functions vary by RTC, please refer to the README.md file for each specific RTC.
```
int controlClock(void)
```
| Return Value | Meaning |
|---|---|
|RTC_U_SUCCESS | Configuration successful |
|RTC_U_FAILURE | Configuration failed |

### 2.11
Some RTCs provide an area where a portion of the registers can be freely used as SRAM. Functions are provided to read from and write to this area.
However, the usable area is limited to locations where the entire 8-bit block corresponding to a specific address is available.
Depending on the RTC type, there may be cases where only a specific 1-bit of a register is available; such locations are excluded from the usable area.
```
int getSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
The first argument, ``addr``, is not the address of the corresponding register, but rather the index of the area starting from 0. The second argument is a pointer to the area (array) where the read data will be stored, and the third argument is the size of the area to be read (1 or greater).

```
int setSRAM(uint8_t addr, uint8_t *array, uint16_t len)
```
The meaning of the arguments is the same as in ``getSRAM()``.

### 2.12 Utility Functions
A function that takes a UNIX timestamp (the number of seconds elapsed since January 1, 1970) as its second argument and returns the year, month, day, and time into the structure specified by the first argument.
```
void                convertEpochTime(date_t * dateTime , unsigned long epochTime)
```

If a timestamp is specified as an argument, this function returns the corresponding UNIX time. If the argument specifies a time prior to January 1, 1970, 0 is returned.
```
unsigned long       convertDateToEpoch(date_t dateTime)
```

If an integer corresponding to a day of the week is specified as an argument, this function returns a string representing the day of the week, such as “Sun” or “Mon”. If the argument is 0, the return value is ‘Sun’; if it is 1, the return value is “Mon”.
```
String              getWday(uint8_t day)
```

## 3. Sample Programs

We have provided sample programs that demonstrate the RTC's main functions.
Please edit the `config.h` file included with each sample program (sketch) to define the RTC pins and specify which RTC to use, and then install the program.

|Function|Sample Program Name|
|---|---|
|Alarm|SimpleAlarm|
|Countdown Timer|SimpleCountdownTimer|
|Interval Timer|SimpleIntervalTimer|
|Watchdog Timer|None|
|Offset Adjustment|SimpleOffset|
|RTC Temperature|SimpleTemperature|
|TCXO|Same as above|
|SQW Output Adjustment|SimpleSQW|
|SRAM|accessSRAM|
|External Event|None|

The following table summarizes the features supported by each RTC.

|RTC|Alarm|Countdown Timer|Interval Timer|Watchdog Timer|Offset Adjustment|RTC Temperature|TCXO|SQW Output Adjustment|SRAM|External Event|
|---|---|---|---|---|---|---|---|---|---|---|
|4543|||||||||||
|8564|○|○||||||○|||
|DS1307||||||||○|○||
|DS3231|○||||○|○||○|||
|DS3234|○||||○|○|○|○|○||
|PCF8523|○|○||○|○||||||
|RV8803|○|○|○||○|||○|○|○|
|RX8025|○||○||○||||||
|RX8900|○|○|○|||○|○|○|○||


[RTC4543]:https://www5.epsondevice.com/ja/products/rtc/rtc4543sb.html
[RTC8564NB]:https://www5.epsondevice.com/ja/products/rtc/rtc8564nb.html
[RX8025]:https://www5.epsondevice.com/ja/products/rtc/rx8025sa.html
[RX8900]:https://www5.epsondevice.com/ja/products/rtc/rx8900sa.html
[DS1307]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS1307.html
[DS3231]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3231.html
[DS3234]:https://www.maximintegrated.com/jp/products/analog/real-time-clocks/DS3234.html
[RV8803]:https://www.microcrystal.com/jp/%E8%A3%BD%E5%93%81/%E3%83%AA%E3%82%A2%E3%83%AB%E3%82%BF%E3%82%A4%E3%83%A0%E3%82%AF%E3%83%AD%E3%83%83%E3%82%AF%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB/rv-8803-c7/
[PCF8523]:https://www.nxp.com/products/peripherals-and-logic/signal-chain/real-time-clocks/rtcs-with-ic-bus/100-na-real-time-clock-calendar-with-battery-backup:PCF8523

<!--- コメント

## 動作検証

|CPU| 機種 |ベンダ| 結果 | 備考 |
| :--- | :--- | :--- | :---: | :--- |
|AVR| [Uno R3][Uno]  |[Arduino][Arduino]|  ○    |      |
|       | [Mega2560 R3][Mega] |[Arduino][Arduino] |  ○    |      |
|       | [Leonardo Ethernet][LeonardoEth] |[Arduino][Arduino] | ○     |      |
|       | [Uno WiFi][UnoWiFi] |[Arduino][Arduino] | ○     | |
|       | [Pro mini 3.3V][ProMini] | [Sparkfun][Sparkfun] |   ×   |      |
| ARM/M0+ | [M0 Pro][M0Pro] |[Arduino][Arduino] |○||
|ESP8266|[ESPr developer][ESPrDev]| [スイッチサイエンス][SwitchScience] |||
|ESP32 | [ESPr one 32][ESPrOne32] | [スイッチサイエンス][SwitchScience] |×|　|




[Adafruit Unified Sensor Driver][AdafruitUSD]
[Groveシールド][shield]
[Arduino M0 Pro][M0Pro]
[Arduino Due][Due]
[Arduino Uno R3][Uno]
[Arduino Mega2560 R3][Mega]
[Arduino Leonardo Ethernet][LeonardoEth]
[Arduino Pro mini 328 - 3.3V/8MHz][ProMini]
[ESpr one][ESPrOne]
[ESPr one 32][ESPrOne32]
[Grove][Grove]
[Seed Studio][SeedStudio]
[Arduino][Arduino]
[Sparkfun][Sparkfun]
[スイッチサイエンス][SwitchScience]
--->
