#include "main.h"
#include "steering_task.h"

#include "FreeRTOS.h"
#include "task.h"

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
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	steering_task_create();
	
	//使能GPIOC时钟
	__HAL_RCC_GPIOC_CLK_ENABLE();
	//初始化IO引脚
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
*函 数 名: main
*功能说明: 系统程序入口
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
int main(void)
{
	//初始化HAL库
	HAL_Init();
	//时钟系统配置72M
	SystemClock_Config();
	
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
	if (htim->Instance == TIM9) {
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

	//配置主内部调节器输出电压
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
	
	//根据RCC_OscInitTypeDef结构中指定的参数初始化RCC振荡器。
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK);
	//初始化CPU，AHB和APB时钟
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	while (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK);
}
