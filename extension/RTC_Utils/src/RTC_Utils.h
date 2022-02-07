#ifndef _RTC_UTILS_H_
#define _RTC_UTILS_H_

#ifdef USE_RTC8564NB
#include "RTC_8564NB_U.h"
#endif /* USE_RTC8564NB */

#ifdef USE_DS1307
#include "RTC_DS1307_U.h"
#endif /* USE_DS1307 */

#ifdef USE_DS3234
#include "RTC_DS3234_U.h"
#endif /* USE_DS3234 */

#ifdef __RTC_EPSON_8564NB_U_H__
#define RTC_CLASS RTC_8564NB_U
#endif /* __RTC_EPSON_8564NB_U_H__ */

#ifdef __RTC_DS1307_U_H__
#define RTC_CLASS RTC_DS1307_U
#endif /* __RTC_DS1307_U_H__ */

#ifdef __RTC_DS3234_U_H__
#define RTC_CLASS RTC_DS3234_U
#endif /* __RTC_DS3234_U_H__ */

#endif /* _RTC_UTILS_H_ */

