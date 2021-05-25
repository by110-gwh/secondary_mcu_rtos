#include "steering_pwm.h"
#include "stm32f7xx_hal.h"
#include "steering_task.h"

//ͨ��IO����ߵ�ƽ���û�����д
//��ĳͨ�����ã��򽫺�����գ���#define CH16_H
#define CH1_H GPIOE->BSRR = 1U << 1
#define CH2_H GPIOE->BSRR = 1U << 3
#define CH3_H GPIOE->BSRR = 1U << 4
#define CH4_H GPIOE->BSRR = 1U << 5
#define CH5_H GPIOE->BSRR = 1U << 6
#define CH6_H GPIOC->BSRR = 1U << 13
#define CH7_H GPIOC->BSRR = 1U << 14
#define CH8_H GPIOC->BSRR = 1U << 15
#define CH9_H GPIOC->BSRR = 1U << 0
#define CH10_H GPIOC->BSRR = 1U << 1
#define CH11_H GPIOC->BSRR = 1U << 2
#define CH12_H GPIOC->BSRR = 1U << 3
#define CH13_H GPIOA->BSRR = 1U << 0
#define CH14_H GPIOA->BSRR = 1U << 1
#define CH15_H GPIOA->BSRR = 1U << 2
#define CH16_H GPIOA->BSRR = 1U << 3

//ͨ��IO����͵�ƽ���û�����д
//��ĳͨ�����ã��򽫺�����գ���#define CH16_L
#define CH1_L GPIOE->BSRR = 1U << (1 + 16)
#define CH2_L GPIOE->BSRR = 1U << (3 + 16)
#define CH3_L GPIOE->BSRR = 1U << (4 + 16)
#define CH4_L GPIOE->BSRR = 1U << (5 + 16)
#define CH5_L GPIOE->BSRR = 1U << (6 + 16)
#define CH6_L GPIOC->BSRR = 1U << (13 + 16)
#define CH7_L GPIOC->BSRR = 1U << (14 + 16)
#define CH8_L GPIOC->BSRR = 1U << (15 + 16)
#define CH9_L GPIOC->BSRR = 1U << (0 + 16)
#define CH10_L GPIOC->BSRR = 1U << (1 + 16)
#define CH11_L GPIOC->BSRR = 1U << (2 + 16)
#define CH12_L GPIOC->BSRR = 1U << (3 + 16)
#define CH13_L GPIOA->BSRR = 1U << (0 + 16)
#define CH14_L GPIOA->BSRR = 1U << (1 + 16)
#define CH15_L GPIOA->BSRR = 1U << (2 + 16)
#define CH16_L GPIOA->BSRR = 1U << (3 + 16)

//����ͨ���ߵ�ƽʱ�䣬��λus,��Χ0 - 4500
volatile uint16_t steering_pulse_ch[16];
//��ʱ��2���
static TIM_HandleTypeDef htim2;

/**********************************************************************************************************
*�� �� ��: steering_gpio_init
*����˵��: �������IO��ʼ�����û�����д�˺���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void steering_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//ʹ��GPIOʱ��
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
    
	//��ʼ��IO����
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOC, 0xE00F, GPIO_PIN_RESET);
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
*�� �� ��: steering_init
*����˵��: ���������ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void steering_pwm_init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	steering_gpio_init();
	
	//ʹ�ܶ�ʱ��ʱ��
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	//��ʱ��2��������ʼ��
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 84 - 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 5000 - 1;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim2);

	//��ʱ��2ѡ���ڲ�ʱ��
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
	
	//��ʱ��2ѡ�����ͨ����ʼ��
	HAL_TIM_OC_Init(&htim2);
	
	//�رն�ʱ��2�����������
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
	
	//��ʼ������Ƚ�ͨ��
	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
	HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3);
	HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4);
	
	//ʹ�ܶ�ʱ��2�ж�
	HAL_NVIC_SetPriority(TIM2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
	
	//������ʹ��
	HAL_TIM_Base_Start_IT(&htim2);
	//����Ƚ�ͨ��ʹ��
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_3);
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_4);
}

/**********************************************************************************************************
*�� �� ��: TIM2_IRQHandler
*����˵��: ��ʱ��2�жϺ���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void TIM2_IRQHandler(void)
{
	static uint8_t next_ch;
	//��ʱ������ж�
	if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
		//�ж�����ļ���ͨ��
		if (next_ch == 0) {
			//�ĸ�ͨ������ߵ�ƽ
			if (steering_pulse_ch[4])
				CH5_H;
			if (steering_pulse_ch[5])
				CH6_H;
			if (steering_pulse_ch[6])
				CH7_H;
			if (steering_pulse_ch[7])
				CH8_H;
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, steering_pulse_ch[4]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, steering_pulse_ch[5]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, steering_pulse_ch[6]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, steering_pulse_ch[7]);
			next_ch = 4;
		} else if (next_ch == 4) {
			//�ĸ�ͨ������ߵ�ƽ
			if (steering_pulse_ch[8])
				CH9_H;
			if (steering_pulse_ch[9])
				CH10_H;
			if (steering_pulse_ch[10])
				CH11_H;
			if (steering_pulse_ch[11])
				CH12_H;
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, steering_pulse_ch[8]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, steering_pulse_ch[9]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, steering_pulse_ch[10]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, steering_pulse_ch[11]);
			next_ch = 8;
		} else if (next_ch == 8) {
			//�ĸ�ͨ������ߵ�ƽ
			if (steering_pulse_ch[12])
				CH13_H;
			if (steering_pulse_ch[13])
				CH14_H;
			if (steering_pulse_ch[14])
				CH15_H;
			if (steering_pulse_ch[15])
				CH16_H;
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, steering_pulse_ch[12]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, steering_pulse_ch[13]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, steering_pulse_ch[14]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, steering_pulse_ch[15]);
			next_ch = 12;
		} else if (next_ch == 12) {
			//�ĸ�ͨ������ߵ�ƽ
			if (steering_pulse_ch[0])
				CH1_H;
			if (steering_pulse_ch[1])
				CH2_H;
			if (steering_pulse_ch[2])
				CH3_H;
			if (steering_pulse_ch[3])
				CH4_H;
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, steering_pulse_ch[0]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, steering_pulse_ch[1]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, steering_pulse_ch[2]);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, steering_pulse_ch[3]);
			next_ch = 0;
		}
			
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC1);
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC2);
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC3);
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC4);
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_UPDATE);
	//��ʱ�����ͨ��1�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH1_L;
		} else if (next_ch == 4) {
			//��Ӧͨ������͵�ƽ
			CH5_L;
		} else if (next_ch == 8) {
			//��Ӧͨ������͵�ƽ
			CH9_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH13_L;
		}
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC1);
	//��ʱ�����ͨ��2�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC2) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH2_L;
		} else if (next_ch == 4) {
			//��Ӧͨ������͵�ƽ
			CH6_L;
		} else if (next_ch == 8) {
			//��Ӧͨ������͵�ƽ
			CH10_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH14_L;
		}
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC2);
	//��ʱ�����ͨ��3�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC3) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH3_L;
		} else if (next_ch == 4) {
			//��Ӧͨ������͵�ƽ
			CH7_L;
		} else if (next_ch == 8) {
			//��Ӧͨ������͵�ƽ
			CH11_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH15_L;
		}
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC3);
	//��ʱ�����ͨ��4�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC4) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH4_L;
		} else if (next_ch == 4) {
			//��Ӧͨ������͵�ƽ
			CH8_L;
		} else if (next_ch == 8) {
			//��Ӧͨ������͵�ƽ
			CH12_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH16_L;
		}
		__HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC4);
	}
}
