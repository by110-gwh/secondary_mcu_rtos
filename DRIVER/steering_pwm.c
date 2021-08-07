#include "steering_pwm.h"
#include "stm32f7xx_hal.h"
#include "steering_task.h"

//通道IO输出高电平，用户需重写
//如某通道不用，则将后面清空，如#define CH16_H
#define CH1_H GPIOE->BSRR = 1U << 1
#define CH2_H GPIOE->BSRR = 1U << 3
#define CH3_H GPIOE->BSRR = 1U << 4
#define CH4_H GPIOE->BSRR = 1U << 5
#define CH5_H GPIOE->BSRR = 1U << 6
#define CH6_H GPIOC->BSRR = 1U << 13
#define CH7_H GPIOC->BSRR = 1U << 14
#define CH8_H GPIOC->BSRR = 1U << 15
#define CH9_H GPIOC->BSRR = 1U << 4
#define CH10_H GPIOC->BSRR = 1U << 1
#define CH11_H GPIOC->BSRR = 1U << 2
#define CH12_H GPIOC->BSRR = 1U << 3
#define CH13_H GPIOA->BSRR = 1U << 0
#define CH14_H GPIOA->BSRR = 1U << 1
#define CH15_H GPIOA->BSRR = 1U << 2
#define CH16_H GPIOA->BSRR = 1U << 3
#define CH17_H
#define CH18_H
#define CH19_H
#define CH20_H
#define CH21_H
#define CH22_H
#define CH23_H
#define CH24_H

//通道IO输出低电平，用户需重写
//如某通道不用，则将后面清空，如#define CH16_L
#define CH1_L GPIOE->BSRR = 1U << (1 + 16)
#define CH2_L GPIOE->BSRR = 1U << (3 + 16)
#define CH3_L GPIOE->BSRR = 1U << (4 + 16)
#define CH4_L GPIOE->BSRR = 1U << (5 + 16)
#define CH5_L GPIOE->BSRR = 1U << (6 + 16)
#define CH6_L GPIOC->BSRR = 1U << (13 + 16)
#define CH7_L GPIOC->BSRR = 1U << (14 + 16)
#define CH8_L GPIOC->BSRR = 1U << (15 + 16)
#define CH9_L GPIOC->BSRR = 1U << (4 + 16)
#define CH10_L GPIOC->BSRR = 1U << (1 + 16)
#define CH11_L GPIOC->BSRR = 1U << (2 + 16)
#define CH12_L GPIOC->BSRR = 1U << (3 + 16)
#define CH13_L GPIOA->BSRR = 1U << (0 + 16)
#define CH14_L GPIOA->BSRR = 1U << (1 + 16)
#define CH15_L GPIOA->BSRR = 1U << (2 + 16)
#define CH16_L GPIOA->BSRR = 1U << (3 + 16)
#define CH17_L
#define CH18_L
#define CH19_L
#define CH20_L
#define CH21_L
#define CH22_L
#define CH23_L
#define CH24_L

//各个通道高电平时间，单位us,范围0 - 4500
volatile float steering_pulse_ch[32];
//各个通道微调偏移量，单位us,范围-100 - 100
volatile int8_t steering_pulse_offset[32];
//定时器2句柄
static TIM_HandleTypeDef htim8;

/**********************************************************************************************************
*函 数 名: steering_gpio_init
*功能说明: 舵机驱动IO初始化，用户需重写此函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
static void steering_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//使能GPIO时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
    
	//初始化IO引脚
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOC, 0xE01E, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = 0xE00F;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
	HAL_GPIO_WritePin(GPIOE, 0x7A, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = 0x7A;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**********************************************************************************************************
*函 数 名: steering_init
*功能说明: 舵机驱动初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void steering_pwm_init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	steering_gpio_init();
	
	//使能定时器时钟
	__HAL_RCC_TIM8_CLK_ENABLE();
	
	//定时器2计数器初始化
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 216 - 1;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 5000 - 1;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim8);

	//定时器2选择内部时钟
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig);
	
	//定时器2选择输出通道初始化
	HAL_TIM_OC_Init(&htim8);
	
	//关闭定时器2级联功能输出
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);
	
	//初始化输出比较通道
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2);
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4);
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_5);
	HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_6);
	
	//使能定时器2中断
	HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
	HAL_NVIC_SetPriority(TIM8_CC_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
	
	//计数器使能
	HAL_TIM_Base_Start_IT(&htim8);
	//输出比较通道使能
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_3);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_4);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_5);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_6);
}

static uint8_t next_ch;
/**********************************************************************************************************
*函 数 名: TIM8_UP_TIM13_IRQHandler
*功能说明: 定时器8更新中断函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void TIM8_UP_TIM13_IRQHandler(void)
{
	//定时器溢出中断
	if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_UPDATE) != RESET) {
        //判断输出哪几号通道
		if (next_ch == 0) {
			//六个通道输出高电平
			if ((uint16_t)steering_pulse_ch[6])
				CH7_H;
			if ((uint16_t)steering_pulse_ch[7])
				CH8_H;
			if ((uint16_t)steering_pulse_ch[8])
				CH9_H;
			if ((uint16_t)steering_pulse_ch[9])
				CH10_H;
			if ((uint16_t)steering_pulse_ch[10])
				CH11_H;
			if ((uint16_t)steering_pulse_ch[11])
				CH12_H;
			//设置比较器
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[6] - steering_pulse_offset[6]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[7] - steering_pulse_offset[7]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[8] - steering_pulse_offset[8]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[9] - steering_pulse_offset[9]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[10] - steering_pulse_offset[10]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[11] - steering_pulse_offset[11]);
			next_ch = 6;
		} else if (next_ch == 6) {
			//六个通道输出高电平
			if ((uint16_t)steering_pulse_ch[12])
				CH13_H;
			if ((uint16_t)steering_pulse_ch[13])
				CH14_H;
			if ((uint16_t)steering_pulse_ch[14])
				CH15_H;
			if ((uint16_t)steering_pulse_ch[15])
				CH16_H;
			if ((uint16_t)steering_pulse_ch[16])
				CH17_H;
			if ((uint16_t)steering_pulse_ch[17])
				CH18_H;
			//设置比较器
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[12] - steering_pulse_offset[12]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[13] - steering_pulse_offset[13]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[14] - steering_pulse_offset[14]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[15] - steering_pulse_offset[15]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[16] - steering_pulse_offset[16]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[17] - steering_pulse_offset[17]);
			next_ch = 12;
		} else if (next_ch == 12) {
			//六个通道输出高电平
			if ((uint16_t)steering_pulse_ch[18])
				CH19_H;
			if ((uint16_t)steering_pulse_ch[19])
				CH20_H;
			if ((uint16_t)steering_pulse_ch[20])
				CH21_H;
			if ((uint16_t)steering_pulse_ch[21])
				CH22_H;
			if ((uint16_t)steering_pulse_ch[22])
				CH23_H;
			if ((uint16_t)steering_pulse_ch[23])
				CH24_H;
			//设置比较器
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[18] - steering_pulse_offset[18]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[19] - steering_pulse_offset[19]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[20] - steering_pulse_offset[20]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[21] - steering_pulse_offset[21]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[22] - steering_pulse_offset[22]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[23] - steering_pulse_offset[23]);
			next_ch = 18;
		} else if (next_ch == 18) {
			//六个通道输出高电平
			if ((uint16_t)steering_pulse_ch[0])
				CH1_H;
			if ((uint16_t)steering_pulse_ch[1])
				CH2_H;
			if ((uint16_t)steering_pulse_ch[2])
				CH3_H;
			if ((uint16_t)steering_pulse_ch[3])
				CH4_H;
			if ((uint16_t)steering_pulse_ch[4])
				CH5_H;
			if ((uint16_t)steering_pulse_ch[5])
				CH6_H;
			//设置比较器
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[0] - steering_pulse_offset[0]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[1] - steering_pulse_offset[1]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[2] - steering_pulse_offset[2]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[3] - steering_pulse_offset[3]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[4] - steering_pulse_offset[4]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[5] - steering_pulse_offset[5]);
			next_ch = 0;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC1);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC2);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC3);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC4);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC5);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC6);
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_UPDATE);
    }
}



/**********************************************************************************************************
*函 数 名: TIM8_CC_IRQHandler
*功能说明: 定时器8捕获中断函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void TIM8_CC_IRQHandler(void)
{	
	//定时器输出通道1比较中断
	if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC1) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH1_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH7_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH13_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH19_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC1);
	//定时器输出通道2比较中断
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC2) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH2_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH8_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH14_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH20_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC2);
	//定时器输出通道3比较中断
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC3) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH3_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH9_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH15_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH21_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC3);
	//定时器输出通道4比较中断
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC4) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH4_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH10_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH16_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH22_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC4);
	//定时器输出通道5比较中断
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC5) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH5_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH11_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH17_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH23_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC5);
	//定时器输出通道4比较中断
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC6) != RESET) {
		if (next_ch == 0) {
			//相应通道输出低电平
			CH6_L;
		} else if (next_ch == 6) {
			//相应通道输出低电平
			CH12_L;
		} else if (next_ch == 12) {
			//相应通道输出低电平
			CH18_L;
		} else if (next_ch == 18) {
			//相应通道输出低电平
			CH24_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC6);
	}
}
