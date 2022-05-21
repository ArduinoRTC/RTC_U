
/*
 * 1ラウンドの測定期間
 */

/*
 * 使用するRTCのI/Fに関する定義
 */

// SPI : DS3234
#define CS_PIN 5
#define SPI_IF SPI
// I2C : 各種
#define I2C_IF Wire

/*
 * 使用するRTCの選択
 */
//#define USE_DS3231
//#define USE_DS3234
//#define USE_PCF8523
//#define USE_RV8803
#define USE_RX8025

#ifdef USE_DS3231
#include "RTC_DS3231_U.h"
RTC_DS3231_U rtc = RTC_DS3231_U(&I2C_IF);
#define WAIT_TIME 7200000UL // 2時間
#define DURATION 7200
#endif /* USE_DS3231 */

#ifdef USE_DS3234
#include "RTC_DS3234_U.h"
RTC_DS3234_U rtc = RTC_DS3234_U(CS_PIN,&SPI_IF);
#define WAIT_TIME 7200000UL // 2時間
#define DURATION 7200
#endif /* USE_DS3234 */

#ifdef USE_PCF8523
#include "RTC_PCF8523_U.h"
RTC_PCF8523_U rtc = RTC_PCF8523_U(&I2C_IF);
#define WAIT_TIME 7200000UL // 2時間
#define DURATION 7200
#endif /* USE_PCF8523 */

#ifdef USE_RV8803
#include "RTC_RV8803_U.h"
RTC_RV8803_U rtc = RTC_RV8803_U(&I2C_IF);
#define WAIT_TIME 28800000UL // 8時間
#define DURATION 28800
#endif /* USE_RV8803 */

#ifdef USE_RX8025
#include "RTC_RX8025_U.h"
RTC_RX8025_U rtc = RTC_RX8025_U(&I2C_IF);
#define WAIT_TIME 7200000UL // 2時間
#define DURATION 7200
#endif /* USE_RX8025 */
