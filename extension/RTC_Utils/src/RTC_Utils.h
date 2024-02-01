#ifndef _RTC_UTILS_H_
#define _RTC_UTILS_H_

#include "RTC_U.h"

#ifdef USE_RTC_4543
#include "RTC_4543_U.h"
#endif /* USE_RTC_4543 */

#ifdef USE_RTC8564NB
#include "RTC_8564NB_U.h"
#endif /* USE_RTC8564NB */

#ifdef USE_DS1307
#include "RTC_DS1307_U.h"
#endif /* USE_DS1307 */

#ifdef USE_DS3231
#include "RTC_DS3231_U.h"
#endif /* USE_DS3231 */

#ifdef USE_DS3234
#include "RTC_DS3234_U.h"
#endif /* USE_DS3234 */

#ifdef USE_PCF8523
#include "RTC_PCF8523_U.h"
#endif /* USE_PCF8523 */

#ifdef USE_RV8803
#include "RTC_RV8803_U.h"
#endif /* USE_RV8803 */

#ifdef USE_RX8025
#include "RTC_RX8025_U.h"
#endif /* USE_RX8025 */

#ifdef USE_RX8900
#include "RTC_RX8900_U.h"
#endif /* USE_RX8900 */

#ifdef __RTC_4543_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_4543_U
#endif /* RTC_CLASS */
#endif /* __RTC_4543_U_H__ */

#ifdef __RTC_EPSON_8564NB_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_8564NB_U
#endif /* RTC_CLASS */
#endif /* __RTC_EPSON_8564NB_U_H__ */

#ifdef __RTC_DS1307_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_DS1307_U
#endif /* RTC_CLASS */
#endif /* __RTC_DS1307_U_H__ */

#ifdef __RTC_DS3231_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_DS3231_U
#endif /* RTC_CLASS */
#endif /* __RTC_DS3231_U_H__ */

#ifdef __RTC_DS3234_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_DS3234_U
#endif /* RTC_CLASS */
#endif /* __RTC_DS3234_U_H__ */

#ifdef __RTC_PCF8523_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_PCF8523_U
#endif /* RTC_CLASS */
#endif /* __RTC_PCF8523_U_H__ */

#ifdef __RTC_RV8803_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_RV8803_U
#endif /* RTC_CLASS */
#endif /* __RTC_RV8803_U_H__ */

#ifndef __RTC_EPSON_RX8025_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_RX8025_U
#endif /* RTC_CLASS */
#endif /* __RTC_EPSON_RX8025_U_H__ */

#ifndef __RTC_RX8900_U_H__
#ifndef RTC_CLASS
#define RTC_CLASS RTC_RX8900_U
#endif /* RTC_CLASS */
#endif /* __RTC_RX8900_U_H__ */

#endif /* _RTC_UTILS_H_ */

