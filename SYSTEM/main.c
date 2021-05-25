#include "main.h"
#include "main_task.h"
#include "qspi.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

//启动任务句柄
xTaskHandle startTask;

void SystemClock_Config(void);

/**********************************************************************************************************
*函 数 名: vStartTask
*功能说明: 系统启动任务，调用各类初始化函数，并创建消息队列和要运行的用户任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(vStartTask, pvParameters)
{
	main_task_create();
	
	vTaskDelete(NULL);
}

/**********************************************************************************************************
*函 数 名: main
*功能说明: 系统程序入口
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
int main(void)
{
    //使能I-Cache
    SCB_EnableICache();
    //使能D-Cache
    SCB_EnableDCache();
	//初始化HAL库
	HAL_Init();
	//时钟系统配置72M
	SystemClock_Config();
	
    //映射QSPI到内存地址上
    qspi_init();
    
	//创建启动任务
	xTaskCreate(vStartTask, "startTask", 128, NULL, 0, &startTask);
	//OS调度器启动
    vTaskStartScheduler();
	while (1);
}

/**********************************************************************************************************
*函 数 名: HAL_TIM_PeriodElapsedCallback
*功能说明: 定时器溢出更新中断回调函数
*形    参: 定时器句柄
*返 回 值: 无
**********************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM7) {
		HAL_IncTick();
	}
}

/**********************************************************************************************************
*函 数 名: SystemClock_Config
*功能说明: 时钟系统配置
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};


	//配置主内部调节器输出电压
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    //根据RCC_OscInitTypeDef结构中指定的参数初始化RCC振荡器。
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 9;
    while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);
    //激活Over-Drive模式
    HAL_PWREx_EnableOverDrive();
    
    //初始化CPU，AHB和APB时钟
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
