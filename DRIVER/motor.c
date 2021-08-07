#include "motor.h"

#define MAX_VAL 7000

pid_controler_t motor_controler[5];


/************************************************************
*@name:read_encoder
*@function:得到编码器的值
*@param:想要读取的电机编号
*@return:编码器的值
**************************************************************/
short read_encoder(int motor_id)
{
    //此处不用清除数组的值，因为该值是随时更新的，不会有叠加的现象产生
    return (short)encoder_val[motor_id];

}
/************************************************************
*@name:set_motor
*@function：控制电机的定时器口，驱动电机转动
*@motor_id:控制的目标电机编号
*@control_val：输入的控制值值
*@return: 无
**************************************************************/
void set_motor(int motor_id, short control_val)
{
    bool pos_flag;//标志位变量

    //限幅
    if(control_val > MAX_VAL)
    {
        control_val = MAX_VAL;
    }
    else if(control_val < MAX_VAL)
    {
        control_val = -MAX_VAL;
    }
    //正负判断
    if(control_val > 0)
    {
        pos_flag = 1;
    }
    else if(control_val < 0 || control_val == 0 )
    {
        pos_flag = 0;
        control_val *= (-1);//等价于进行ABS操作
    }
    //不同电机不同处理方案
    switch (motor_id)
    {
    case 1:
        if(pos_flag)
        {
            TIM2->CCR3 = control_val;
            TIM2->CCR4 = 0;
        }
        else
        {
            TIM2->CCR3 = 0;
            TIM2->CCR4 = control_val;
        }
        break;
    case 2:
        if(pos_flag)
        {
            TIM2->CCR1 = control_val;
            TIM2->CCR2 = 0;
        }
        else
        {
            TIM2->CCR1 = 0;
            TIM2->CCR2 = control_val;
        }
        break;
    case 3:
        if(pos_flag)
        {
            TIM1->CCR1 = control_val;
            TIM1->CCR2 = 0;
        }
        else
        {
            TIM1->CCR1 = 0;
            TIM1->CCR2 = control_val;
        }
        break;
    case 4:
        if(pos_flag)
        {
            TIM1->CCR3 = control_val;
            TIM1->CCR4 = 0;
        }
        else
        {
            TIM1->CCR3 = 0;
            TIM1->CCR4 = control_val;
        }
    default:
        return;
    }
}
