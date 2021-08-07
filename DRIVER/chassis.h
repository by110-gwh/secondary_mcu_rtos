#ifndef __CHASSIS_H_
#define __CHASSIS_H_


#endif




#ifndef __CHASSIS_H
#define __CHASSIS_H
#include "main.h"
#include <stdbool.h>
#include "motor.h"
#include "tim_control.h"
typedef struct chassis_structure
{
    bool _switch;//是否使能底盘
    double x_speed;//x方向底盘速度
    double  y_speed;//y方向底盘速度
    double  w_speed;//w方向底盘角速度
} CHASSIS;





extern CHASSIS chassis;
void speed_variation(float x_var, float y_var, float w_var);
void set_speed (float x, float y, float w);
void change_switch_status(bool status);
void chassis_synthetic_control(void);
#endif
