
/*
 * SQW出力を制御するピンの定義
 */
#define FOE_PIN 5
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

//#define USE_RTC8564NB
//#define USE_DS1307
//#define USE_DS3231
//#define USE_DS3234
//#define USE_RV8803
#define USE_RX8900



#ifdef USE_RTC8564NB
#include "RTC_8564NB_U.h"
RTC_8564NB_U rtc = RTC_8564NB_U(&I2C_IF);
#endif /* USE_RTC8564NB */

#ifdef USE_DS1307
#include "RTC_DS1307_U.h"
RTC_DS1307_U rtc = RTC_DS1307_U(&I2C_IF);
#endif /* USE_DS1307 */

#ifdef USE_DS3231
#include "RTC_DS3231_U.h"
RTC_DS3231_U rtc = RTC_DS3231_U(&I2C_IF);
#endif /* USE_DS3231 */

#ifdef USE_DS3234
#include "RTC_DS3234_U.h"
RTC_DS3234_U rtc = RTC_DS3234_U(CS_PIN,&SPI_IF);
#endif /* USE_DS3234 */

#ifdef USE_RV8803
#include "RTC_RV8803_U.h"
RTC_RV8803_U rtc = RTC_RV8803_U(&I2C_IF);
#endif /* USE_RV8803 */

#ifdef USE_RX8900
#include "RTC_RX8900_U.h"
RTC_RX8900_U rtc = RTC_RX8900_U(&I2C_IF);
#endif /* USE_RX8900 */
