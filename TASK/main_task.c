#include "main_task.h"
#include "steering_task.h"
#include "usmart_task.h"
#include "time_cnt.h"
#include "i2c.h"
#include "paramer_save.h"
#include "ahrs_task.h"
#include "imu_temp_task.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define MAIN_TASK_STACK            512
//任务优先级
#define MAIN_TASK_PRIORITY         5

//声明任务句柄
xTaskHandle main_task_handle;
//任务退出标志
volatile uint8_t main_task_exit;

/**********************************************************************************************************
*函 数 名: main_task
*功能说明: 主任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(main_task, parameters)
{
    Get_Time_Init();
    steering_task_create();
    usmart_task_create();
    i2c_init();
    read_save_paramer();
    ahrs_task_create();
    imu_temp_task_create();
    while (!main_task_exit) {
		vTaskDelay(1000);
    }
	vTaskDelete(NULL);
}

/**********************************************************************************************************
*函 数 名: main_task_create
*功能说明: 主函数相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void main_task_create(void)
{
	main_task_exit = 0;
    xTaskCreate(main_task, "main_task", MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle);
}
