#include "steering_pwm.h"
#include "stm32f4xx_hal.h"
#include "bitband.h"
#include "steering_task.h"

//ͨ��IO����ߵ�ƽ���û�����д
//��ĳͨ�����ã��򽫺�����գ���#define CH16_H
#define CH1_H PAout(0) = 1
#define CH2_H PAout(1) = 1
#define CH3_H PAout(2) = 1
#define CH4_H PAout(3) = 1
#define CH5_H PAout(4) = 1
#define CH6_H PAout(5) = 1
#define CH7_H PAout(6) = 1
#define CH8_H PAout(7) = 1
#define CH9_H PBout(0) = 1
#define CH10_H PBout(1) = 1
#define CH11_H PBout(2) = 1
#define CH12_H PBout(3) = 1
#define CH13_H PBout(4) = 1
#define CH14_H PBout(5) = 1
#define CH15_H PBout(6) = 1
#define CH16_H PBout(7) = 1

//ͨ��IO����͵�ƽ���û�����д
//��ĳͨ�����ã��򽫺�����գ���#define CH16_L
#define CH1_L PAout(0) = 0
#define CH2_L PAout(1) = 0
#define CH3_L PAout(2) = 0
#define CH4_L PAout(3) = 0
#define CH5_L PAout(4) = 0
#define CH6_L PAout(5) = 0
#define CH7_L PAout(6) = 0
#define CH8_L PAout(7) = 0
#define CH9_L PBout(0) = 0
#define CH10_L PBout(1) = 0
#define CH11_L PBout(2) = 0
#define CH12_L PBout(3) = 0
#define CH13_L PBout(4) = 0
#define CH14_L PBout(5) = 0
#define CH15_L PBout(6) = 0
#define CH16_L PBout(7) = 0

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
	__HAL_RCC_GPIOB_CLK_ENABLE();
	//��ʼ��IO����
	HAL_GPIO_WritePin(GPIOA, 0xFF, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = 0xFF;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB, 0xFF, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = 0xFF;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
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
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
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
