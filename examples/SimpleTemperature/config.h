
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
#define USE_RX8900

/*
 * 温度の単位系の選択
 */
#define TEMP_UNIT RTC_U_TEMPERATURE_CELCIUS
//#define TEMP_UNIT RTC_U_TEMPERATURE_KELVIN
//#define TEMP_UNIT RTC_U_TEMPERATURE_FAHRENHEIT

#ifdef USE_DS3231
#include "RTC_DS3231_U.h"
RTC_DS3231_U rtc = RTC_DS3231_U(&I2C_IF);
#endif /* USE_DS3231 */

#ifdef USE_DS3234
#include "RTC_DS3234_U.h"
RTC_DS3234_U rtc = RTC_DS3234_U(CS_PIN,&SPI_IF);
#endif /* USE_DS3234 */

#ifdef USE_RX8900
#include "RTC_RX8900_U.h"
RTC_RX8900_U rtc = RTC_RX8900_U(&I2C_IF);
#endif /* USE_RX8900 */
