#include "motion_control_task.h"
#include "motor_pwm.h"
#include "motor_encode.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//任务堆栈大小
#define MOTION_CONTROL_TASK_STACK 256
//任务优先级
#define MOTION_CONTROL_TASK_PRIORITY 12

//声明任务句柄
xTaskHandle motion_control_task_handle;
//任务退出标志
volatile uint8_t motion_control_task_exit = 1;

/**********************************************************************************************************
*函 数 名: motion_control_task
*功能说明: 运动控制器任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(motion_control_task, pvParameters)
{
    float motor_encode_lf_fb = 0;
    float motor_encode_lb_fb = 0;
    float motor_encode_rf_fb = 0;
    float motor_encode_rb_fb = 0;
    portTickType xLastWakeTime;
    
    motor_pwm_init();
    motor_encode_init();
    vTaskDelay(100);
    
    //获取系统当前节拍
    xLastWakeTime = xTaskGetTickCount();
    while (!motion_control_task_exit) {
        int motor_encode;
        int motor_encode_sum = 0;
        int motor_encode_cnt = 0;
        //右前方电机
        while (xQueueReceive(motor_encode_rf_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_rf_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_rf_fb = 1000000.0 / motor_encode_sum;
        }
        //右后方电机
        while (xQueueReceive(motor_encode_rb_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_rb_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_rb_fb = 1000000.0 / motor_encode_sum;
        }
        //左前方电机
        while (xQueueReceive(motor_encode_lf_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_lf_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_lf_fb = 1000000.0 / motor_encode_sum;
        }
        //左后方电机
        while (xQueueReceive(motor_encode_lb_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_lb_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_lb_fb = 1000000.0 / motor_encode_sum;
        }
        
        
        //睡眠5ms
        vTaskDelayUntil(&xLastWakeTime, (5 / portTICK_RATE_MS));
    }
    vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: motion_control_task_create
*功能说明: 运动控制器相关任务创建
*形    参: 动作组编号 重复运行次数
*返 回 值: 无
**********************************************************************************************************/
void motion_control_task_create(void)
{
	motion_control_task_exit = 0;
    xTaskCreate(motion_control_task, "motion_control_task", MOTION_CONTROL_TASK_STACK, NULL, MOTION_CONTROL_TASK_PRIORITY, &motion_control_task_handle);
}
