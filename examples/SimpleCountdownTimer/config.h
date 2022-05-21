
/*
 * 使用するRTCのI/Fに関する定義
 */

#define I2C_IF Wire

/*
 * 使用するRTCの選択
 */
//#define USE_RTC8564NB
//#define USE_PCF8523
//#define USE_RV8803
#define USE_RX8900

#ifdef USE_RTC8564NB
#include "RTC_8564NB_U.h"
RTC_8564NB_U rtc = RTC_8564NB_U(&I2C_IF);
#endif /* USE_RTC8564NB */

#ifdef USE_PCF8523
#include "RTC_PCF8523_U.h"
RTC_PCF8523_U rtc = RTC_PCF8523_U(&I2C_IF);
#endif /* USE_PCF8523 */

#ifdef USE_RV8803
#include "RTC_RV8803_U.h"
RTC_RV8803_U rtc = RTC_RV8803_U(&I2C_IF);
#endif /* USE_RV8803 */

#ifdef USE_RX8900
#include "RTC_RX8900_U.h"
RTC_RX8900_U rtc = RTC_RX8900_U(&I2C_IF);
#endif /* USE_RX8900 */
