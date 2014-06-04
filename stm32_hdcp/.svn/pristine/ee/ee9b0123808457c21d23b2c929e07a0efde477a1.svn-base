#ifndef _RTC_H_EB88C9D9_230B_4E92_9128_FDAA067F0D84
#define _RTC_H_EB88C9D9_230B_4E92_9128_FDAA067F0D84
//时间结构体
typedef struct
{
	u8 hour;
	u8 min;
	u8 sec;                 
	//公历日月年周
	u8  w_month;
	u8  w_date;
	u8  week;
	u16 w_year;
	u16 daycnt;//优化，减少计算量
	uint32 secondsCount;
}tm;
extern tm timer;
extern tm timer2;
extern void RTC_Config(void);
uint32 RTC_DateToSeconds(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
uint32 RTC_Add(u32 seccount,u8 hour,u8 min,u8 sec);
u8 RTC_SecondsToDate(tm *timer,u32 timecount);
extern u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
extern u8 RTC_Get(tm *timer);


#endif
