#ifndef _TIME_CNT_H
#define _TIME_CNT_H

#include "time_cnt.h"
#include <stdint.h>

typedef struct
{
	uint8_t inited;
	//µ¥Î»us
	uint32_t Last_Time;
	uint32_t Now_Time;
	uint32_t Time_Delta;
} Testime;

typedef struct
{
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	uint16_t microsecond;
} Time_t;

typedef enum
{
    up_count = 1,
    dowm_count
} count_mode;
typedef struct
{
    count_mode mode;
    uint32_t * sys_time_ptr;
    uint32_t period_val;
} time_t;

void sys_time_init(uint32_t * ptr, count_mode mode, uint32_t period);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
extern Time_t Time_Sys;

void Get_Time_Init(void);
void Get_Time_Period(Testime *Time_Lab);
#endif

