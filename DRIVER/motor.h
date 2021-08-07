#ifndef __MOTOR_H
#define __MOTOR_H
#include "tim_control.h"
#include <stdbool.h>
#include "pid.h"
short read_encoder(int motor_id);
void set_motor(int motor_id, short control_val);
void pid_param_init(pid_controler_t *controler, float params[]); //此函数后期移动回去pid.c中
extern pid_controler_t motor_controler[5];
#endif
