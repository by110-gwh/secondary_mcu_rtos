#include "tim_control.h"
double encoder_val[5];//Ĭ��Ϊ0
short status_flag[5];//
double encoder_sum, temp_sum;
int rising_val[5], falling_val[5], direct_[5], update_count[5];
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;

#define  FORWARD    1
#define  BACKWARD   -1
#define SPEED_PARAM    10000
#define TIM_COUNT_VAL  0xFFFF
/**********************
*@name:HAL_TIM_IC_CaptureCallback
*@function:����������ݣ�������
*@param:��ʱ���ṹ��
*@return:��
****************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    if(htim->Instance == TIM3)//�ж϶�ʱ��
    {
        if(htim->Channel == TIM_CHANNEL_1)//�жϴ�����ͨ��
        {
            if(!status_flag[1])//��ʼ����״̬��־λ�ж�,��ʱ����������
            {
                status_flag[1] = 1;//��־�Ѿ�����������
                rising_val[1] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1); //��ȡ��ʱ��CNTֵ
                update_count[1] = 0; //��ʼ��¼�����¼��ķ�������
                /*
                 *�˴������жϵ����ת��
                 *�ж�AB ��λ�����λ�ù�ϵ���ó���ת��
                */
                if(HAL_GPIO_ReadPin(MOTOR1_ENCODER_GPIO_Port, MOTOR1_ENCODER_Pin) == GPIO_PIN_RESET)
                {
                    direct_[1] = FORWARD;//ǰ������ת
                }
                else
                {
                    direct_[1] = BACKWARD;//���ˣ���ת
                }
                //�Ѳ���������Ϊ�½��ز��񡣵ȴ��½��صĵ���
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
            }
            else//�����½���
            {
                status_flag[1] = 0;//��ձ�־λ
                falling_val[1] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1); //��ȡ��ʱ��CNTֵ
                encoder_val[1] = (SPEED_PARAM / (falling_val[1] - rising_val[1] + update_count[1] * TIM_COUNT_VAL)) * direct_[1]; //������ֵ������ת����ת�٣����Ϸ���
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING); //���������ز��񣬻ص���һ��
            }
        }
        else if(htim->Channel == TIM_CHANNEL_3)//MOTOR2
        {
            if(!status_flag[2])
            {
                status_flag[2] = 1;
                rising_val[2] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3);
                update_count[2] = 0;
                if(HAL_GPIO_ReadPin(MOTOR2_ENCODER_GPIO_Port, MOTOR2_ENCODER_Pin) == GPIO_PIN_RESET)
                {
                    direct_[2] = FORWARD;//
                }
                else
                {
                    direct_[2] = BACKWARD;
                }
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_ICPOLARITY_FALLING);
            }
            else
            {
                status_flag[2] = 0;
                falling_val[2] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3);
                encoder_val[2] = (SPEED_PARAM / (falling_val[2] - rising_val[2] + update_count[2] * TIM_COUNT_VAL)) * direct_[2];
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_ICPOLARITY_RISING);
            }
        }
    }
    else if( htim->Instance == TIM5)
    {
        if(htim->Channel == TIM_CHANNEL_1) //MOTOR3
        {
            if(!status_flag[3])
            {
                status_flag[3] = 1;
                rising_val[3] = HAL_TIM_ReadCapturedValue(&htim5, TIM_CHANNEL_1);
                update_count[3] = 0;
                if(HAL_GPIO_ReadPin(MOTOR3_ENCODER_GPIO_Port, MOTOR3_ENCODER_Pin) == GPIO_PIN_RESET)
                {
                    direct_[3] = FORWARD;
                }
                else
                {
                    direct_[3] = BACKWARD;
                }
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
            }
            else
            {
                status_flag[3] = 0;
                falling_val[3] = HAL_TIM_ReadCapturedValue(&htim5, TIM_CHANNEL_1);
                encoder_val[3] = (SPEED_PARAM / (falling_val[3] - rising_val[3] + update_count[3] * TIM_COUNT_VAL)) * direct_[3];
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);
            }
        }

        else  if(htim->Channel == TIM_CHANNEL_3)//MOTOR4
        {
            if(!status_flag[4])
            {
                status_flag[4] = 1;
                rising_val[4] = HAL_TIM_ReadCapturedValue(&htim5, TIM_CHANNEL_3);
                update_count[4] = 0;
                if(HAL_GPIO_ReadPin(MOTOR4_ENCODER_GPIO_Port, MOTOR4_ENCODER_Pin) == GPIO_PIN_RESET)
                {
                    direct_[4] = FORWARD;
                }
                else
                {
                    direct_[4] = BACKWARD;
                }
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_3, TIM_ICPOLARITY_FALLING);
            }
            else
            {
                status_flag[4] = 0;
                falling_val[4] = HAL_TIM_ReadCapturedValue(&htim5, TIM_CHANNEL_3);
                encoder_val[4] = (SPEED_PARAM / (falling_val[4] - rising_val[4] + update_count[4] * TIM_COUNT_VAL)) * direct_[4];
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim5, TIM_CHANNEL_3, TIM_ICPOLARITY_RISING);
            }
        }
    }


    for(uint8_t i = 1; i <= 4; ++i) temp_sum += fabs(encoder_val[i]);//��ת�پ�ֵ������move by encoderʱ�����ݣ���ʼ����ʱ����0
    encoder_sum  += temp_sum / (4.0);
    temp_sum = 0;
}
/*******************
*@name:HAL_TIM_PeriodElapsedCallback
*@function:���ö�ʱ����ˢ������,����ʱ��
*@param:��ʱ���ṹ��
*@return:��
**********************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        update_count[1]++;//
        update_count[2]++;//���ڼ���������������;������ʱ�������¼������
    }
    else if(htim->Instance == TIM5)
    {
        update_count[3]++;
        update_count[4]++;
    }

}

