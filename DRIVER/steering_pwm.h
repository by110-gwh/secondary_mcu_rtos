#ifndef _STEERING_PWM_H
#define _STEERING_PWM_H

#include "stdint.h"

//����ͨ���ߵ�ƽʱ�䣬��λus
extern uint16_t steering_pulse_ch[16];

void steering_pwm_init(void);

#endif
