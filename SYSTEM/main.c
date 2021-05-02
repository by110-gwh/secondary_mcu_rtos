#include "main.h"
#include "steering_task.h"

#include "FreeRTOS.h"
#include "task.h"

//����������
xTaskHandle startTask;

void SystemClock_Config(void);

/**********************************************************************************************************
*�� �� ��: vStartTask
*����˵��: ϵͳ�������񣬵��ø����ʼ����������������Ϣ���к�Ҫ���е��û�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(vStartTask, pvParameters)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	steering_task_create();
	
	//ʹ��GPIOCʱ��
	__HAL_RCC_GPIOC_CLK_ENABLE();
	//��ʼ��IO����
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	while (1) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		vTaskDelay(1000);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		vTaskDelay(1000);
	}
}

/**********************************************************************************************************
*�� �� ��: main
*����˵��: ϵͳ�������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
int main(void)
{
	//��ʼ��HAL��
	HAL_Init();
	//ʱ��ϵͳ����72M
	SystemClock_Config();
	
	//������������
	xTaskCreate(vStartTask, "startTask", 128, NULL, 0, &startTask);
	//OS����������
    vTaskStartScheduler();
	while (1);
}

/**********************************************************************************************************
*�� �� ��: HAL_TIM_PeriodElapsedCallback
*����˵��: ��ʱ����������жϻص�����
*��    ��: ��ʱ�����
*�� �� ֵ: ��
**********************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM9) {
		HAL_IncTick();
	}
}

/**********************************************************************************************************
*�� �� ��: SystemClock_Config
*����˵��: ʱ��ϵͳ����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	//�������ڲ������������ѹ
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
	
	//����RCC_OscInitTypeDef�ṹ��ָ���Ĳ�����ʼ��RCC������
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);
	//��ʼ��CPU��AHB��APBʱ��
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	while (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK);
}
