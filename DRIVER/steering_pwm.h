#ifndef _STEERING_PWM_H
#define _STEERING_PWM_H

#include "stdint.h"

extern volatile float steering_pulse_ch[32];
extern volatile int8_t steering_pulse_offset[32];
void steering_pwm_init(void);

#endif
