
/*
 * 使用するRTCのI/Fに関する定義
 */

#define I2C_IF Wire

/*
 * 使用するRTCの選択
 */
//#define USE_RX8025
#define USE_RX8900
//#define USE_RV8803

#ifdef USE_RX8025
#include "RTC_RX8025_U.h"
RTC_RX8025_U rtc = RTC_RX8025_U(&I2C_IF);
#endif /* USE_RX8025 */

#ifdef USE_RX8900
#include "RTC_RX8900_U.h"
RTC_RX8900_U rtc = RTC_RX8900_U(&I2C_IF);
#endif /* USE_RX8900 */

#ifdef USE_RV8803
#include "RTC_RV8803_U.h"
RTC_RV8803_U rtc = RTC_RV8803_U(&I2C_IF);
#endif /* USE_RV8803 */
