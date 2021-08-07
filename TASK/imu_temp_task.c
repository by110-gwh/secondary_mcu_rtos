#include "imu_temp_task.h"
#include "imu.h"
#include "stm32f7xx_hal.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define IMU_TEMP_TASK_STACK 512
//任务优先级
#define IMU_TEMP_TASK_PRIORITY 12

//声明任务句柄
xTaskHandle imu_temp_task_handle;
//任务退出标志
volatile uint8_t imu_temp_task_exit;
//温度控制PID
pid_paramer_t imu_temp_pid_data_para = {
    .integrate_max = 30,
    .kp = 30,
    .ki = 0.5,
    .kd = 0,
    .control_output_limit = 100
};

pid_data_t imu_temp_pid_data;

/**********************************************************************************************************
*函 数 名: imu_temp_pid_data_init
*功能说明: 传感器恒温控制PID初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
static void imu_temp_pid_data_init(void)
{
    imu_temp_pid_data.expect = 0;
    imu_temp_pid_data.feedback = 0;

    imu_temp_pid_data.err = 0;
    imu_temp_pid_data.last_err = 0;
    imu_temp_pid_data.integrate = 0;
    imu_temp_pid_data.dis_err = 0;

    imu_temp_pid_data.control_output = 0;

    imu_temp_pid_data.short_circuit_flag = 0;
    imu_temp_pid_data.err_callback = NULL;
    imu_temp_pid_data.pri_data = NULL;
}

/**********************************************************************************************************
*函 数 名: imu_temp_gpio_init
*功能说明: 传感器恒温控制IO初始化，用户需重写此函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
static void imu_temp_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//使能GPIO时钟
	__HAL_RCC_GPIOC_CLK_ENABLE();
    
	//初始化IO引脚
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**********************************************************************************************************
*函 数 名: imu_temp_task
*功能说明: 传感器恒温控制任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(imu_temp_task, pvParameters)
{
    uint8_t count;
    
    imu_temp_gpio_init();
    imu_temp_pid_data_init();
    count = 0;
    while (!imu_temp_task_exit) {
        
        //温度PID控制
        if (count == 100) {
            imu_temp_pid_data.feedback = tempDataFilter;
            imu_temp_pid_data.expect = 50;
            pid_control(&imu_temp_pid_data, &imu_temp_pid_data_para);
        }
        
        //模拟PWM输出
        if (count == 100)
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
        else if (count >= imu_temp_pid_data.control_output)
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
        
        if (count == 100)
            count = 0;
        else
            count++;
        
        //睡眠1ms
        vTaskDelay(1);
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: imu_temp_task_create
*功能说明: 传感器恒温控制相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void imu_temp_task_create(void)
{
	imu_temp_task_exit = 0;
    xTaskCreate(imu_temp_task, "imu_temp_task", IMU_TEMP_TASK_STACK, NULL, IMU_TEMP_TASK_PRIORITY, &imu_temp_task_handle);
}
