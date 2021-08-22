#include "motion_control_task.h"
#include "motor_pwm.h"
#include "motor_encode.h"
#include "pid.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//任务堆栈大小
#define MOTION_CONTROL_TASK_STACK 256
//任务优先级
#define MOTION_CONTROL_TASK_PRIORITY 12

//声明任务句柄
static xTaskHandle motion_control_task_handle;
//任务退出标志
volatile uint8_t motion_control_task_exit = 1;

//电机pid参数
static pid_paramer_t motor_pid_data_para = {
    .integrate_max = 0,
    .kp = 3,
    .ki = 25,
    .kd = 0,
    .control_output_limit = 0
};
//电机pid数据
pid_data_t motor_pid_data_lf;
pid_data_t motor_pid_data_lb;
pid_data_t motor_pid_data_rf;
pid_data_t motor_pid_data_rb;

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
        //获取电机编码器值
        motor_encode_get(&motor_encode_lf_fb, &motor_encode_lb_fb, &motor_encode_rf_fb, &motor_encode_rb_fb);
        //更新pid反馈值
        motor_pid_data_lf.feedback = motor_encode_lf_fb;
        motor_pid_data_lb.feedback = motor_encode_lb_fb;
        motor_pid_data_rf.feedback = motor_encode_rf_fb;
        motor_pid_data_rb.feedback = motor_encode_rb_fb;
        //pid运算
        pid_control(&motor_pid_data_lf, &motor_pid_data_para);
        pid_control(&motor_pid_data_lb, &motor_pid_data_para);
        pid_control(&motor_pid_data_rf, &motor_pid_data_para);
        pid_control(&motor_pid_data_rb, &motor_pid_data_para);
        //电机输出
        motor_pwm_set(motor_pid_data_lf.control_output, motor_pid_data_lb.control_output, 
                motor_pid_data_rf.control_output, motor_pid_data_rb.control_output);
//        printf("%0.1f,%0.1f,%0.1f\r\n", motor_pid_data_lf.expect, motor_pid_data_lf.feedback, motor_pid_data_lf.control_output);
        //睡眠5ms
        vTaskDelayUntil(&xLastWakeTime, (5 / portTICK_RATE_MS));
    }
    vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: motion_control_task_create
*功能说明: 运动控制器相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void motion_control_task_create(void)
{
	motion_control_task_exit = 0;
    xTaskCreate(motion_control_task, "motion_control_task", MOTION_CONTROL_TASK_STACK, NULL, MOTION_CONTROL_TASK_PRIORITY, &motion_control_task_handle);
}
