#include <RTC_U.h>

/*
 * http://mrkk.ciao.jp/memorandom/unixtime/unixtime.html をArduinoでも動くように修正したもの．
 *   元のプログラムの「西暦元年3/1から西暦1970/1/1までの日数」は特にあやしいので，日数計算サイトで再計算したものをベースに調整．
 *    暦が中世で変更になった影響も受けるので，注意が必要．
 */

#define ARRAYSIZE(_arr) (sizeof(_arr) / sizeof(_arr[0]))
#define YEAR_ONE            365
#define YEAR_FOUR           1461  // (YEAR_ONE * 4 + 1)  //  it is YEAR_ONE*4+1 so the maximum reminder of day / YEAR_FOUR is YEAR_ONE * 4 and it occurs only on 2/29
#define YEAR_100            36524  // (YEAR_FOUR * 25 - 1)
#define YEAR_400            146097  // (YEAR_100*4 + 1)    //  it is YEAR_100*4+1 so the maximum reminder of day / YEAR_400 is YEAR_100 * 4 and it occurs only on 2/29
void RTC_Unified::convertEpochTime(date_t * date, unsigned long unixtime) {
    uint32_t    unixday;
    uint16_t    year = 0;
    uint8_t     leap = 0;
    uint32_t    n;
    uint8_t     month, day, weekday;
    uint8_t     hour, minute, second;
    static const uint16_t monthday[] = { 0,31,61,92,122,153,184,214,245,275,306,337 };

    second = unixtime % 60;
    minute = (unixtime / 60) % 60;
    hour = (unixtime / 3600) % 24;

    unixday = (uint32_t)(unixtime / SECONDS_IN_A_DAY);
    weekday = (uint8_t)(((unixday % 7) + 4 ) % 7); //  because the unix epoch day is thursday
    unixday += UNIX_EPOCH_DAY;  //  days from 0000/03/01 to 1970/01/01

    year += 400 * (uint16_t) (unixday / YEAR_400);
    unixday %= YEAR_400;

    n = unixday / YEAR_100;
    year += n * 100;
    unixday %= YEAR_100;

    if (n == 4){
        leap = 1;
    } else {
        year += 4 * (uint16_t) (unixday / YEAR_FOUR);
        unixday %= YEAR_FOUR;
        
        n = unixday / YEAR_ONE;
        year += n;
        unixday %= YEAR_ONE;
        if (n == 4) {
            leap = 1;
        }
    }
    if (leap != 0) {
        month = 2;
        day = 29;
    }
    else {
        month = (unixday * 5 + 2) / 153;
        day = unixday - monthday[month] + 1;    //  
        month += 3;
        if (month > 12) {
            ++year;
            month -= 12;
        }
    }
    date->second = second;
    date->minute = minute;
    date->hour = hour;
    date->mday = day;
    date->month = month;
    date->year = year;
    date->wday = weekday;
};


/*
 * 参考
 * https://dixq.net/forum/viewtopic.php?t=10264
 */
String RTC_Unified::getWday(uint8_t day){
    switch(day){
        case SUN: return "Sun";
        case MON: return "Mon";
        case TUE: return "Tue";
        case WED: return "Wed";
        case THU: return "Thu";
        case FRI: return "Fri";
        case SAT: return "Sat";
        default:  return "";
    }
};

/* 西暦元年から引数の年までの経過日数を求める */
unsigned int RTC_Unified::getDays(uint16_t year) {
	unsigned int result;
	if(year<0)return -getDays(-year)-366+365;
	else if(year==0)return 0;
	result=(unsigned int) year*365;
	result+=(unsigned int) year/4;   /* 通常の閏年 */
	result-=(unsigned int) year/100; /* 100年に一度は閏年ではない */
	result+=(unsigned int) year/400; /* 400年に一度は100の倍数だが，閏年 */
	return result;
}

unsigned long RTC_Unified::convertDateToEpoch(date_t dateTime){
	const unsigned int monthDays[13]={
			0,31,59,90,120,151,181,212,243,273,304,334,365
	};/*その月までの日数の和（累積和）*/

    uint16_t    year = dateTime.year;
    uint8_t     month = dateTime.month;
    uint8_t     day = dateTime.mday;
    uint8_t     hour = dateTime.hour;
    uint8_t     minute = dateTime.minute;
    uint8_t     second = dateTime.second;

	unsigned long result;
    if (year < 1970) return 0;
	/* year年1月1日 00:00:00までの日数を求める*/
	result=getDays(year-1)-getDays(1969);
	/*秒に変換*/
	result*=SECONDS_IN_A_DAY;
	/*月の日数を秒に変換して足す*/
	result+=monthDays[month-1]*SECONDS_IN_A_DAY;
	/*閏年かつ3月以降なら1日足す*/
	if(year%400==0 || (year%4==0 && year%100!=0)) {
		if(month>=3)result+=SECONDS_IN_A_DAY;
	}
	/*その月の日数を秒に変換して足す*/
	result+=(day-1)*SECONDS_IN_A_DAY;
	result+=(unsigned long) hour*60*60;
	result+=(unsigned long) minute*60;
	result+=(unsigned long) second;
	return result;
}

uint8_t RTC_Unified::intToBCD(uint8_t data){
    uint8_t lowBits,highBits;
    lowBits = data % 10;
    highBits = ((uint8_t) (data /10)) % 10;
    return (highBits << 4) | lowBits;
}

uint8_t RTC_Unified::bcdToInt(uint8_t bcdData){
    uint8_t lowBits, highBits;
    lowBits = bcdData & 0x0F;
    highBits = (bcdData >> 4) * 10;
    return lowBits + highBits;
}
