#include "route_task.h"
#include "motion_control_task.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define ROUTE_TASK_TASK_STACK 256
//任务优先级
#define ROUTE_TASK_TASK_PRIORITY 12

//声明任务句柄
static xTaskHandle route_task_handle;
//任务退出标志
volatile uint8_t route_task_exit = 1;

/**********************************************************************************************************
*函 数 名: route_task
*功能说明: 路径任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(route_task, pvParameters)
{
    while (!route_task_exit) {
        motor_pid_data_lf.expect = 200;
        motor_pid_data_lb.expect = 200;
        motor_pid_data_rf.expect = 200;
        motor_pid_data_rb.expect = 200;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = 0;
        motor_pid_data_lb.expect = 0;
        motor_pid_data_rf.expect = 0;
        motor_pid_data_rb.expect = 0;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = -200;
        motor_pid_data_lb.expect = -200;
        motor_pid_data_rf.expect = -200;
        motor_pid_data_rb.expect = -200;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = 0;
        motor_pid_data_lb.expect = 0;
        motor_pid_data_rf.expect = 0;
        motor_pid_data_rb.expect = 0;
        vTaskDelay(1000);
    }
    vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: route_task_create
*功能说明: 路径相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void route_task_create(void)
{
	route_task_exit = 0;
    xTaskCreate(route_task, "motion_control_task", ROUTE_TASK_TASK_STACK, NULL, ROUTE_TASK_TASK_PRIORITY, &route_task_handle);
}
