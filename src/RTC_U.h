
#ifndef __RTC_U_H__
#define __RTC_U_H__

#define RTC_U_UNSUPPORTED   -1
#define RTC_U_SUCCESS        0
#define RTC_U_FAILURE       -2
#define RTC_U_ILLEGAL_PARAM -3
#define RTC_U_NO_EXTERNAL_EVENT -4

#define RTC_U_TEMPERATURE_KELVIN 0
#define RTC_U_TEMPERATURE_CELCIUS 1
#define RTC_U_TEMPERATURE_FAHRENHEIT 2
#define RTC_U_TEMPERATURE_FAILURE -500.0

#include <Arduino.h>
#include <dateUtils.h>


/*****************************************************
*  型の定義                                          *
*****************************************************/

typedef struct {
    uint8_t  pulse;
    uint8_t  repeat;
    uint8_t  useInteruptPin;
    uint8_t  interval;
} timer_mode_t;

typedef struct {
    uint8_t  useInteruptPin;
    uint8_t  type;
} alarm_mode_t;

typedef struct {
    uint8_t  useInteruptPin;
    bool capture;
    uint8_t level;
    uint8_t filter;
    bool reset;
} event_mode_t;

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
    bool        detectLowBattery;
    bool        controlOscillator;
} rtc_info_t;

class RTC_Unified {
public:
    RTC_Unified() {}
    virtual             ~RTC_Unified() {}
    virtual bool        setTime(date_t*) {return false;};
    virtual bool        getTime(date_t*) {return false;};
    virtual int         setAlarm(uint8_t num, alarm_mode_t * mode, date_t* timing) {return RTC_U_UNSUPPORTED;};
    virtual int         setAlarmMode(uint8_t num, alarm_mode_t * mode) {return RTC_U_UNSUPPORTED;};
    virtual int         controlAlarm(uint8_t num, uint8_t action) {return RTC_U_UNSUPPORTED;};
    virtual int         setTimer(uint8_t num, timer_mode_t * mode, uint16_t multi) {return RTC_U_UNSUPPORTED;};
    virtual int         setTimerMode(uint8_t num, timer_mode_t * mode) {return RTC_U_UNSUPPORTED;};
    virtual int         controlTimer(uint8_t num, uint8_t action) {return RTC_U_UNSUPPORTED;};
    virtual int         setClockOut(uint8_t num, uint8_t freq, int8_t pin) {return RTC_U_UNSUPPORTED;};
    virtual int         setClockOutMode(uint8_t num, uint8_t freq) {return RTC_U_UNSUPPORTED;};
    virtual int         controlClockOut(uint8_t num, uint8_t mode) {return RTC_U_UNSUPPORTED;};
    virtual int         checkInterupt(void) {return 0;};
    virtual int         clearInterupt(uint16_t type) {return false;};
    virtual void        getRtcInfo(rtc_info_t *info) {};
    virtual int         setOscillator(uint8_t mode) {return RTC_U_UNSUPPORTED;};
    virtual int         getOscillator(void) {return RTC_U_UNSUPPORTED;};
    virtual int         controlTemperatureFunction(uint8_t action) {return RTC_U_UNSUPPORTED;};
    virtual int         getTemperatureFunctionState(void) {return RTC_U_UNSUPPORTED;};
    virtual int         setTemperatureFunction(uint8_t mode) {return RTC_U_UNSUPPORTED;};
    virtual float       getTemperature(uint8_t mode) {return -500.0;};
    virtual int         clockHaltStatus(void) {return RTC_U_UNSUPPORTED;};
    virtual int         controlClockHalt(uint8_t mode) {return RTC_U_UNSUPPORTED;};
    virtual int         checkLowPower(void) {return RTC_U_UNSUPPORTED;};
    virtual int         clearPowerFlag(void) {return RTC_U_UNSUPPORTED;};
    virtual int         setLowPower(uint8_t mode) {return RTC_U_UNSUPPORTED;};
    virtual int         controlClock(void) {return RTC_U_UNSUPPORTED;};
    virtual int         setEvent(event_mode_t mode) {return RTC_U_UNSUPPORTED;};
    virtual int         getEvent(void) {return RTC_U_UNSUPPORTED;};


    void                convertEpochTime(date_t * dateTime , unsigned long epochTime);
    unsigned long       convertDateToEpoch(date_t dateTime);
    String              getWday(uint8_t day);
    uint8_t             intToBCD(uint8_t data);
    uint8_t             bcdToInt(uint8_t data);

private:
    unsigned int        getDays(uint16_t year);
};


#endif /* __RTC_U_H__ */

