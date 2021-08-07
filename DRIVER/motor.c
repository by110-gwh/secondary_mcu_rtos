#include "motor.h"

#define MAX_VAL 7000

pid_controler_t motor_controler[5];


/************************************************************
*@name:read_encoder
*@function:�õ���������ֵ
*@param:��Ҫ��ȡ�ĵ�����
*@return:��������ֵ
**************************************************************/
short read_encoder(int motor_id)
{
    //�˴�������������ֵ����Ϊ��ֵ����ʱ���µģ������е��ӵ��������
    return (short)encoder_val[motor_id];

}
/************************************************************
*@name:set_motor
*@function�����Ƶ���Ķ�ʱ���ڣ��������ת��
*@motor_id:���Ƶ�Ŀ�������
*@control_val������Ŀ���ֵֵ
*@return: ��
**************************************************************/
void set_motor(int motor_id, short control_val)
{
    bool pos_flag;//��־λ����

    //�޷�
    if(control_val > MAX_VAL)
    {
        control_val = MAX_VAL;
    }
    else if(control_val < MAX_VAL)
    {
        control_val = -MAX_VAL;
    }
    //�����ж�
    if(control_val > 0)
    {
        pos_flag = 1;
    }
    else if(control_val < 0 || control_val == 0 )
    {
        pos_flag = 0;
        control_val *= (-1);//�ȼ��ڽ���ABS����
    }
    //��ͬ�����ͬ������
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
