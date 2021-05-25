#include "main.h"
#include "main_task.h"
#include "qspi.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

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
	main_task_create();
	
	vTaskDelete(NULL);
}

/**********************************************************************************************************
*�� �� ��: main
*����˵��: ϵͳ�������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
int main(void)
{
    //ʹ��I-Cache
    SCB_EnableICache();
    //ʹ��D-Cache
    SCB_EnableDCache();
	//��ʼ��HAL��
	HAL_Init();
	//ʱ��ϵͳ����72M
	SystemClock_Config();
	
    //ӳ��QSPI���ڴ��ַ��
    qspi_init();
    
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
	if (htim->Instance == TIM7) {
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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};


	//�������ڲ������������ѹ
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    //����RCC_OscInitTypeDef�ṹ��ָ���Ĳ�����ʼ��RCC������
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);
    //����Over-Driveģʽ
    HAL_PWREx_EnableOverDrive();
    
    //��ʼ��CPU��AHB��APBʱ��
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
    
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
    while (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK);
}
