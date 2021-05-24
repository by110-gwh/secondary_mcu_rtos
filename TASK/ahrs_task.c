#include "ahrs_task.h"
#include "ahrs.h"
#include "imu.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define AHRS_TASK_STACK 512
//任务优先级
#define AHRS_TASK_PRIORITY 13

//声明任务句柄
xTaskHandle ahrs_task_handle;
//任务退出标志
volatile uint8_t ahrs_task_exit;

/**********************************************************************************************************
*函 数 名: ahrs_task
*功能说明: 飞行定时任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(ahrs_task, pvParameters)
{
    portTickType xLastWakeTime;

    //挂起调度器
    //vTaskSuspendAll();
	
	imu_init();
	ahrs_init();
    gyro_calibration();
    //唤醒调度器
    //xTaskResumeAll();


    xLastWakeTime = xTaskGetTickCount();
    while (!ahrs_task_exit)
    {
        
		//获取imu数据
		get_imu_data();
		ahrs_update();
        //睡眠5ms
        vTaskDelayUntil(&xLastWakeTime, (5 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: ahrs_task_create
*功能说明: 传感器组件相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void ahrs_task_create(void)
{
	ahrs_task_exit = 0;
    xTaskCreate(ahrs_task, "ahrs_task", AHRS_TASK_STACK, NULL, AHRS_TASK_PRIORITY, &ahrs_task_handle);
}
