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

//����ͨ���ߵ�ƽʱ�䣬��λus,��Χ0 - 4500
volatile float steering_pulse_ch[32];
//����ͨ��΢��ƫ��������λus,��Χ-100 - 100
volatile int8_t steering_pulse_offset[32];
//��ʱ��2���
static TIM_HandleTypeDef htim8;

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
	__HAL_RCC_TIM8_CLK_ENABLE();
	
	//��ʱ��2��������ʼ��
	htim8.Instance = TIM8;
	htim8.Init.Prescaler = 216 - 1;
	htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim8.Init.Period = 5000 - 1;
	htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	HAL_TIM_Base_Init(&htim8);

	//��ʱ��2ѡ���ڲ�ʱ��
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig);
	
	//��ʱ��2ѡ�����ͨ����ʼ��
	HAL_TIM_OC_Init(&htim8);
	
	//�رն�ʱ��2�����������
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);
	
	//��ʼ������Ƚ�ͨ��
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
	
	//ʹ�ܶ�ʱ��2�ж�
	HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
	HAL_NVIC_SetPriority(TIM8_CC_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);
	
	//������ʹ��
	HAL_TIM_Base_Start_IT(&htim8);
	//����Ƚ�ͨ��ʹ��
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_2);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_3);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_4);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_5);
	HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_6);
}

static uint8_t next_ch;
/**********************************************************************************************************
*�� �� ��: TIM8_UP_TIM13_IRQHandler
*����˵��: ��ʱ��8�����жϺ���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void TIM8_UP_TIM13_IRQHandler(void)
{
	//��ʱ������ж�
	if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_UPDATE) != RESET) {
        //�ж�����ļ���ͨ��
		if (next_ch == 0) {
			//����ͨ������ߵ�ƽ
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
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[6] - steering_pulse_offset[6]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[7] - steering_pulse_offset[7]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[8] - steering_pulse_offset[8]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[9] - steering_pulse_offset[9]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[10] - steering_pulse_offset[10]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[11] - steering_pulse_offset[11]);
			next_ch = 6;
		} else if (next_ch == 6) {
			//����ͨ������ߵ�ƽ
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
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[12] - steering_pulse_offset[12]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[13] - steering_pulse_offset[13]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[14] - steering_pulse_offset[14]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[15] - steering_pulse_offset[15]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[16] - steering_pulse_offset[16]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[17] - steering_pulse_offset[17]);
			next_ch = 12;
		} else if (next_ch == 12) {
			//����ͨ������ߵ�ƽ
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
			//���ñȽ���
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, (uint16_t)steering_pulse_ch[18] - steering_pulse_offset[18]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, (uint16_t)steering_pulse_ch[19] - steering_pulse_offset[19]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, (uint16_t)steering_pulse_ch[20] - steering_pulse_offset[20]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, (uint16_t)steering_pulse_ch[21] - steering_pulse_offset[21]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_5, (uint16_t)steering_pulse_ch[22] - steering_pulse_offset[22]);
			__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_6, (uint16_t)steering_pulse_ch[23] - steering_pulse_offset[23]);
			next_ch = 18;
		} else if (next_ch == 18) {
			//����ͨ������ߵ�ƽ
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
			//���ñȽ���
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
*�� �� ��: TIM8_CC_IRQHandler
*����˵��: ��ʱ��8�����жϺ���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void TIM8_CC_IRQHandler(void)
{	
	//��ʱ�����ͨ��1�Ƚ��ж�
	if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC1) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH1_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH7_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH13_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH19_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC1);
	//��ʱ�����ͨ��2�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC2) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH2_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH8_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH14_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH20_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC2);
	//��ʱ�����ͨ��3�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC3) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH3_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH9_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH15_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH21_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC3);
	//��ʱ�����ͨ��4�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC4) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH4_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH10_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH16_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH22_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC4);
	//��ʱ�����ͨ��5�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC5) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH5_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH11_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH17_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH23_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC5);
	//��ʱ�����ͨ��4�Ƚ��ж�
	} else if (__HAL_TIM_GET_FLAG(&htim8, TIM_FLAG_CC6) != RESET) {
		if (next_ch == 0) {
			//��Ӧͨ������͵�ƽ
			CH6_L;
		} else if (next_ch == 6) {
			//��Ӧͨ������͵�ƽ
			CH12_L;
		} else if (next_ch == 12) {
			//��Ӧͨ������͵�ƽ
			CH18_L;
		} else if (next_ch == 18) {
			//��Ӧͨ������͵�ƽ
			CH24_L;
		}
		__HAL_TIM_CLEAR_IT(&htim8, TIM_FLAG_CC6);
	}
}
