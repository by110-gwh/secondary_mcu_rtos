#include "chassis.h"

#define TIME_PARAM 10
#define CHASSIS_RADIUS 18.0
#define MAX_SPEED 180.0
CHASSIS chassis;
float motor_target[5];
short time_count;

/**********************************************************************
  * @Name    set_speed
  * @brief     直接修改器底盘的速度值
  * @param   x: [输入/出]  x方向速度
**			 y: [输入/出]  y方向速度
**			 w: [输入/出]  w方向速度
  * @retval  void
  * @author  peach99CPP
  * @Data    2021-08-06
***********************************************************************/
void set_speed(float x, float y, float w)
{
    if (chassis._switch)//只有当底盘的使能开关被打开时才允许进行操作
    {
        chassis.x_speed = x;
        chassis.y_speed = y;
        chassis.w_speed = w;
    }
}

/************************************************************
*@name:change_switch_status
*@function:底盘使能开关
*@param:状态，bool值
*@return:无
**************************************************************/
void change_switch_status(bool status)
{
    chassis._switch = status;
}

/**********************************************************************
  * @Name    speed_variation
  * @brief  对外提供速度修改的接口
  * @param   x_var: [输入/出] x方向速度的改变量
**			 y_var: [输入/出] y方向速度的改变量
**			 w_var: [输入/出] w方向速度的改变量
  * @retval  void
  * @author  peach99CPP
  * @Data    2021-08-06
***********************************************************************/

void speed_variation(float x_var, float y_var, float w_var)
{
    if(chassis._switch)
    {
        chassis.x_speed += x_var;
        chassis.y_speed += y_var;
        chassis.w_speed += w_var;
    }
}

/************************************************************
*@name:chassis_synthetic_control
*@function:底盘的综合控制函数，包含多种控制
*@param:无
*@return:无
**************************************************************/
void chassis_synthetic_control(void)
{
    int i;
    float x, y, w, factor;
    double max_val;
    if (chassis._switch == false) return;//如果底盘不被使能，则没有后续操作

    if (++time_count == TIME_PARAM)
    {
        time_count = 0;
        //在这里刷一些陀螺仪和寻迹的PID更新值.把修改值叠加在chassis的x y速度上
    }
    max_val = 0;//对最大值数据进行初始化
    factor = 1;//倍率因子初始化

    x = chassis.x_speed;
    y = chassis.y_speed;
    w = chassis.w_speed;
    /***************************************
    * motor1  左上角
    * motor2  左下角
    * motor3  右下角
    * motor4  右上角
    * 从左上角逆时针旋转一圈，就是4个电机 1 2 3 4
    ****************************************/

    motor_target[1] = -0.707 * x - 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[2] = 0.707 * x - 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[3] = 0.707 * x + 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[4] = -0.707 * x + 0.707 * y + CHASSIS_RADIUS * w;

    //再来一个限幅操作，避免单边速度过高导致控制效果不理想
    //
    for (i = 1; i <= 4; ++i) //找出最大值
    {
        if (motor_target[i] > max_val)
            max_val = motor_target[i];
    }
    factor = (max_val > MAX_SPEED) ? MAX_SPEED / max_val : 1;
    if (max_val > MAX_SPEED)//最大值是否超限制，进行操作，确保最大值仍在范围内且转速比例 不变
    {
        factor = MAX_SPEED / max_val;
        for (i = 1; i < 4; ++i)
        {
            motor_target[i] *= factor;
        }

    }
    for (i = 1; i <= 4; ++i)
    {
        /*
        *对电机进行遍历
        *首先获取转速期待值
        *读取编码器参数
        *传入PID计算函数得到计算值,以返回值形式传参
        *由计算值控制电机
        */
        motor_controler[i].expect = motor_target[i];
        motor_controler[i].feedback = read_encoder(i);
        set_motor(i, pid_control(&motor_controler[i]));
    }


}
