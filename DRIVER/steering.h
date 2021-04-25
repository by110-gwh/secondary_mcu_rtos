#ifndef _STEERING_H
#define _STEERING_H

#include "stdint.h"

//各个通道高电平时间，单位us
extern uint16_t steering_pulse_ch[16];

void steering_init(void);

#endif
