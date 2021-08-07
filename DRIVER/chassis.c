#include "chassis.h"

#define TIME_PARAM 10
#define CHASSIS_RADIUS 18.0
#define MAX_SPEED 180.0
CHASSIS chassis;
float motor_target[5];
short time_count;

/**********************************************************************
  * @Name    set_speed
  * @brief     ֱ���޸������̵��ٶ�ֵ
  * @param   x: [����/��]  x�����ٶ�
**			 y: [����/��]  y�����ٶ�
**			 w: [����/��]  w�����ٶ�
  * @retval  void
  * @author  peach99CPP
  * @Data    2021-08-06
***********************************************************************/
void set_speed(float x, float y, float w)
{
    if (chassis._switch)//ֻ�е����̵�ʹ�ܿ��ر���ʱ��������в���
    {
        chassis.x_speed = x;
        chassis.y_speed = y;
        chassis.w_speed = w;
    }
}

/************************************************************
*@name:change_switch_status
*@function:����ʹ�ܿ���
*@param:״̬��boolֵ
*@return:��
**************************************************************/
void change_switch_status(bool status)
{
    chassis._switch = status;
}

/**********************************************************************
  * @Name    speed_variation
  * @brief  �����ṩ�ٶ��޸ĵĽӿ�
  * @param   x_var: [����/��] x�����ٶȵĸı���
**			 y_var: [����/��] y�����ٶȵĸı���
**			 w_var: [����/��] w�����ٶȵĸı���
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
*@function:���̵��ۺϿ��ƺ������������ֿ���
*@param:��
*@return:��
**************************************************************/
void chassis_synthetic_control(void)
{
    int i;
    float x, y, w, factor;
    double max_val;
    if (chassis._switch == false) return;//������̲���ʹ�ܣ���û�к�������

    if (++time_count == TIME_PARAM)
    {
        time_count = 0;
        //������ˢһЩ�����Ǻ�Ѱ����PID����ֵ.���޸�ֵ������chassis��x y�ٶ���
    }
    max_val = 0;//�����ֵ���ݽ��г�ʼ��
    factor = 1;//�������ӳ�ʼ��

    x = chassis.x_speed;
    y = chassis.y_speed;
    w = chassis.w_speed;
    /***************************************
    * motor1  ���Ͻ�
    * motor2  ���½�
    * motor3  ���½�
    * motor4  ���Ͻ�
    * �����Ͻ���ʱ����תһȦ������4����� 1 2 3 4
    ****************************************/

    motor_target[1] = -0.707 * x - 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[2] = 0.707 * x - 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[3] = 0.707 * x + 0.707 * y + CHASSIS_RADIUS * w;
    motor_target[4] = -0.707 * x + 0.707 * y + CHASSIS_RADIUS * w;

    //����һ���޷����������ⵥ���ٶȹ��ߵ��¿���Ч��������
    //
    for (i = 1; i <= 4; ++i) //�ҳ����ֵ
    {
        if (motor_target[i] > max_val)
            max_val = motor_target[i];
    }
    factor = (max_val > MAX_SPEED) ? MAX_SPEED / max_val : 1;
    if (max_val > MAX_SPEED)//���ֵ�Ƿ����ƣ����в�����ȷ�����ֵ���ڷ�Χ����ת�ٱ��� ����
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
        *�Ե�����б���
        *���Ȼ�ȡת���ڴ�ֵ
        *��ȡ����������
        *����PID���㺯���õ�����ֵ,�Է���ֵ��ʽ����
        *�ɼ���ֵ���Ƶ��
        */
        motor_controler[i].expect = motor_target[i];
        motor_controler[i].feedback = read_encoder(i);
        set_motor(i, pid_control(&motor_controler[i]));
    }


}
