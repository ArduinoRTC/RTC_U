
#ifndef __RTC_U_H__
#define __RTC_U_H__

#define RTC_U_UNSUPPORTED -1
#define RTC_U_SUCCESS      0
#define RTC_U_FAILURE      1

#include <Arduino.h>
#include <dateUtils.h>


/*****************************************************
*  型の定義                                          *
*****************************************************/

typedef struct {
    int8_t  repeat;
    int8_t  useInteruptPin;
    int8_t  interval;
} timer_mode_t;

typedef struct {
    int8_t  useInteruptPin;
} alarm_mode_t;

enum {
    EPSON8564NB  = 1,
    DS1307,
    DS3234
};

typedef struct {
    uint32_t    type;
    uint8_t     numOfInteruptPin;
    uint8_t     numOfAlarm;
    uint8_t     numOfTimer;
    uint8_t     numOfClockOut;
    uint8_t     numOfYearDigits;
    bool        haveYearOverflowBit;
    bool        haveMilliSec;
    bool        independentSQW;
} rtc_info_t;

class RTC_Unified {
public:
    RTC_Unified() {}
    virtual             ~RTC_Unified() {}
    virtual bool        setTime(date_t*)=0;
    virtual bool        getTime(date_t*)=0;
    virtual int         setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing)=0;
    virtual int         setAlarmMode(uint8_t num, alarm_mode_t * mode)=0;
    virtual int         controlAlarm(uint8_t num, uint8_t action)=0;
    virtual int         setTimer(uint8_t num, timer_mode_t * mode, uint8_t multi)=0;
    virtual int         setTimerMode(uint8_t num, timer_mode_t * mode)=0;
    virtual int         controlTimer(uint8_t num, uint8_t action)=0;
    virtual int         setClockOut(uint8_t num, uint8_t freq, int8_t pin)=0;
    virtual int         setClockOutMode(uint8_t num, uint8_t freq)=0;
    virtual int         controlClockOut(uint8_t num, uint8_t mode)=0;
    virtual uint16_t    checkInterupt(void)=0;
    virtual bool        clearInterupt(uint16_t type)=0;
    virtual void        getRtcInfo(rtc_info_t *info)=0;
    void                convertEpochTime(date_t * dateTime , unsigned long epochTime);
    unsigned long       convertDateToEpoch(date_t dateTime);
    String              getWday(uint8_t day);
private:
    unsigned int        getDays(uint16_t year);
};


#endif /* __RTC_U_H__ */

