#include "steering_updata_task.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define STEERING_UPDATA_TASK_STACK 256
//任务优先级
#define STEERING_UPDATA_TASK_PRIORITY 13

//声明任务句柄
xTaskHandle steering_updata_task_handle;
//任务退出标志
volatile uint8_t steering_updata_task_exit;

/**********************************************************************************************************
*函 数 名: steering_updata_task
*功能说明: 舵机控制任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(steering_updata_task, pvParameters)
{
    portTickType xLastWakeTime;
	
    xLastWakeTime = xTaskGetTickCount();
    while (!steering_updata_task_exit)
    {
		uint8_t i;
		//16通道舵机遍历一遍
		for (i = 0; i < 16; i++) {
			//需要更新舵机角度
			if (steering_pulse_ch[i] != steering_position[i]) {
				//初始赋值
				if (steering_pulse_ch[i] == 0)
					steering_pulse_ch[i] = steering_position[i];
				//角度减小方向
				else if(steering_pulse_ch[i] > steering_position[i]) {
					//根据差值选择步进
					if (steering_pulse_ch[i] - steering_position[i] >= steering_speed[i])
						steering_pulse_ch[i] -=  steering_speed[i];
					else
						steering_pulse_ch[i] = steering_position[i];
				//角度增大方向
				} else {
					//根据差值选择步进
					if (steering_position[i] - steering_pulse_ch[i] >= steering_speed[i])
						steering_pulse_ch[i] +=  steering_speed[i];
					else
						steering_pulse_ch[i] = steering_position[i];
				}
			}
		}
        //睡眠20ms
        vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: steering_updata_task_create
*功能说明: 舵机控制相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void steering_updata_task_create(void)
{
	steering_updata_task_exit = 0;
    xTaskCreate(steering_updata_task, "steering_updata_task", STEERING_UPDATA_TASK_STACK, NULL, STEERING_UPDATA_TASK_PRIORITY, &steering_updata_task_handle);
}
