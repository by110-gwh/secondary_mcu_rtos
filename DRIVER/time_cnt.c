#include "time_cnt.h"
#include "stm32f7xx_hal.h"

TIM_HandleTypeDef htim6;
volatile uint32_t TIME_ISR_CNT;
//系统时间
Time_t Time_Sys;

void HAL_TIM6_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/**********************************************************************************************************
*函 数 名: Get_Time_Init
*功能说明: 时间周期计数模块初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void Get_Time_Init(void)
{
	//使能定时器时钟
	__HAL_RCC_TIM6_CLK_ENABLE();
	
	//定时器初始化
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 108 - 1;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 10000 - 1;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim6);
    
    //定时器中断回调函数注册
    htim6.PeriodElapsedCallback = HAL_TIM6_PeriodElapsedCallback;
	
	//使能定时器中断
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
	
	//启动定时器
	HAL_TIM_Base_Start_IT(&htim6);
}

/**********************************************************************************************************
*函 数 名: HAL_TIM6_PeriodElapsedCallback
*功能说明: 定时器6更新中断回调函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void HAL_TIM6_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static uint16_t Microsecond_Cnt = 0;
    //每10ms自加
    TIME_ISR_CNT++;
    Microsecond_Cnt += 10;
    //1秒
    if (Microsecond_Cnt >= 1000) {
        Microsecond_Cnt = 0;
        Time_Sys.second++;
        //1分钟
        if (Time_Sys.second >= 60) {
            Time_Sys.second = 0;
            Time_Sys.minute++;
            //1小时
            if (Time_Sys.minute >= 60) {
                Time_Sys.minute = 0;
                Time_Sys.hour++;
            }
        }
    }
}

/**********************************************************************************************************
*函 数 名: Get_Period
*功能说明: 获取时间周期
*形    参: 时间周期结构体
*返 回 值: 无
**********************************************************************************************************/
void Get_Time_Period(Testime *Time_Lab)
{
    uint32_t time_cnt;
    //关闭中断
    __disable_irq();
    time_cnt = TIM6->CNT;
	//如果还未初始化
	if (Time_Lab->inited == 0) {
		Time_Lab->inited = 1;
        //如果在此期间产生中断并产生溢出
        if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) && time_cnt < 10000 / 2) {
            Time_Lab->Now_Time = 10000 * (TIME_ISR_CNT + 1) + time_cnt;
        } else {
            Time_Lab->Now_Time = 10000 * TIME_ISR_CNT + time_cnt;
        }
		Time_Lab->Last_Time = Time_Lab->Now_Time;
		Time_Lab->Time_Delta = 0;
	} else {
        Time_Lab->Last_Time = Time_Lab->Now_Time;
        //单位us
        //如果在此期间产生中断并产生溢出
        if (__HAL_TIM_GET_FLAG(&htim6, TIM_FLAG_UPDATE) && time_cnt < 10000 / 2) {
            Time_Lab->Now_Time = 10000 * (TIME_ISR_CNT + 1) + time_cnt;
        } else {
            Time_Lab->Now_Time = 10000 * TIME_ISR_CNT + time_cnt;
        }
        Time_Lab->Time_Delta = Time_Lab->Now_Time - Time_Lab->Last_Time;
    }
    //开启中断
    __enable_irq();
}

/**********************************************************************************************************
*函 数 名: delay_us
*功能说明: us级延时
*形    参: 延时时间（单位us）
*返 回 值: 无
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
