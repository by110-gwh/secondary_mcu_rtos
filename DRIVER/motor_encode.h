#ifndef _MOTOR_ENCODE_H
#define _MOTOR_ENCODE_H

#include "FreeRTOS.h"
#include "queue.h"

void motor_encode_init(void);
void motor_encode_get(float *encode_lf, float *encode_lb, float *encode_rf, float *encode_rb);

#endif
