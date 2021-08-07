#include "tim_control.h"
double encoder_val[5];//默认为0
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
*@function:处理捕获的数据，编码器
*@param:定时器结构体
*@return:无
****************************/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    if(htim->Instance == TIM3)//判断定时器
    {
        if(htim->Channel == TIM_CHANNEL_1)//判断触发的通道
        {
            if(!status_flag[1])//初始捕获状态标志位判断,此时捕获到上升沿
            {
                status_flag[1] = 1;//标志已经捕获上升沿
                rising_val[1] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1); //读取此时的CNT值
                update_count[1] = 0; //开始记录更新事件的发生次数
                /*
                 *此处用于判断电机的转向
                 *判断AB 相位的相对位置关系来得出其转向
                */
                if(HAL_GPIO_ReadPin(MOTOR1_ENCODER_GPIO_Port, MOTOR1_ENCODER_Pin) == GPIO_PIN_RESET)
                {
                    direct_[1] = FORWARD;//前进，正转
                }
                else
                {
                    direct_[1] = BACKWARD;//后退，反转
                }
                //把捕获极性设置为下降沿捕获。等待下降沿的到来
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
            }
            else//捕获到下降沿
            {
                status_flag[1] = 0;//清空标志位
                falling_val[1] = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1); //读取此时的CNT值
                encoder_val[1] = (SPEED_PARAM / (falling_val[1] - rising_val[1] + update_count[1] * TIM_COUNT_VAL)) * direct_[1]; //把脉宽值反比例转化到转速，加上方向
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING); //设置上升沿捕获，回到第一步
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


    for(uint8_t i = 1; i <= 4; ++i) temp_sum += fabs(encoder_val[i]);//求转速均值，用于move by encoder时的数据，开始计数时会置0
    encoder_sum  += temp_sum / (4.0);
    temp_sum = 0;
}
/*******************
*@name:HAL_TIM_PeriodElapsedCallback
*@function:利用定时器来刷新任务,计算时长
*@param:定时器结构体
*@return:无
**********************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        update_count[1]++;//
        update_count[2]++;//用于计算脉宽，处理捕获中途发生定时器更新事件的情况
    }
    else if(htim->Instance == TIM5)
    {
        update_count[3]++;
        update_count[4]++;
    }

}

