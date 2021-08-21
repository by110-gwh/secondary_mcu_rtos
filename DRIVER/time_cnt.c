#include "time_cnt.h"
#include "stm32f7xx_hal.h"

TIM_HandleTypeDef htim6;
volatile uint32_t TIME_ISR_CNT;
//ϵͳʱ��
Time_t Time_Sys;

void HAL_TIM6_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/**********************************************************************************************************
*�� �� ��: Get_Time_Init
*����˵��: ʱ�����ڼ���ģ���ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void Get_Time_Init(void)
{
	//ʹ�ܶ�ʱ��ʱ��
	__HAL_RCC_TIM6_CLK_ENABLE();
	
	//��ʱ����ʼ��
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 108 - 1;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 10000 - 1;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim6);
    
    //��ʱ���жϻص�����ע��
    htim6.PeriodElapsedCallback = HAL_TIM6_PeriodElapsedCallback;
	
	//ʹ�ܶ�ʱ���ж�
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	
	//������ʱ��
	HAL_TIM_Base_Start_IT(&htim6);
}

/**********************************************************************************************************
*�� �� ��: HAL_TIM6_PeriodElapsedCallback
*����˵��: ��ʱ��6�����жϻص�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_TIM6_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t Microsecond_Cnt = 0;
    //ÿ10ms�Լ�
    TIME_ISR_CNT++;
    Microsecond_Cnt += 10;
    //1��
    if (Microsecond_Cnt >= 1000) {
        Microsecond_Cnt = 0;
        Time_Sys.second++;
        //1����
        if (Time_Sys.second >= 60) {
            Time_Sys.second = 0;
            Time_Sys.minute++;
            //1Сʱ
            if (Time_Sys.minute >= 60) {
                Time_Sys.minute = 0;
                Time_Sys.hour++;
            }
        }
    }
}

/**********************************************************************************************************
*�� �� ��: Get_Period
*����˵��: ��ȡʱ������
*��    ��: ʱ�����ڽṹ��
*�� �� ֵ: ��
**********************************************************************************************************/
void Get_Time_Period(Testime *Time_Lab)
{
    uint32_t time_cnt;
    //�ر��ж�
    __disable_irq();
    time_cnt = TIM6->CNT;
	//�����δ��ʼ��
	if (Time_Lab->inited == 0) {
		Time_Lab->inited = 1;
        //����ڴ��ڼ�����жϲ��������
        if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) && time_cnt < 10000 / 2) {
            Time_Lab->Now_Time = 10000 * (TIME_ISR_CNT + 1) + time_cnt;
        } else {
            Time_Lab->Now_Time = 10000 * TIME_ISR_CNT + time_cnt;
        }
		Time_Lab->Last_Time = Time_Lab->Now_Time;
		Time_Lab->Time_Delta = 0;
	} else {
        Time_Lab->Last_Time = Time_Lab->Now_Time;
        //��λus
        //����ڴ��ڼ�����жϲ��������
        if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) && time_cnt < 10000 / 2) {
            Time_Lab->Now_Time = 10000 * (TIME_ISR_CNT + 1) + time_cnt;
        } else {
            Time_Lab->Now_Time = 10000 * TIME_ISR_CNT + time_cnt;
        }
        Time_Lab->Time_Delta = Time_Lab->Now_Time - Time_Lab->Last_Time;
    }
    //�����ж�
    __enable_irq();
}

/**********************************************************************************************************
*�� �� ��: delay_us
*����˵��: us����ʱ
*��    ��: ��ʱʱ�䣨��λus��
*�� �� ֵ: ��
**********************************************************************************************************/
void delay_us(uint32_t us)
{
    uint32_t pre_cnt;
    uint32_t cur_cnt;
    int cnt = us;

    cur_cnt = TIM6->CNT;
    while (cnt > 0) {
        pre_cnt = cur_cnt;
        cur_cnt = TIM6->CNT;
        if (cur_cnt >= pre_cnt) {
            cnt -= cur_cnt - pre_cnt;
        } else {
            cnt -= 10000 + cur_cnt - pre_cnt;
        }
    }
}
