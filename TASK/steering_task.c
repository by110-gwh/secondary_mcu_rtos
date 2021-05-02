#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "usbd_custom_hid_if.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define STEERING_TASK_STACK 256
//任务优先级
#define STEERING_TASK_PRIORITY 13

//声明任务句柄
xTaskHandle steering_task_handle;
//任务退出标志
volatile uint8_t steering_task_exit;

void steering_control_data_in_callback(uint8_t *pdata, uint32_t len)
{
	//前缀0xFF
	if (pdata[0] == 0xFF) {
		//心跳包
		if (pdata[1] == 0 && pdata[2] == 0x12) {
			uint8_t send_buf[3] = {0xFF, 0xF0, 0x12};
			USBD_CUSTOM_HID_SendReport_FS(send_buf, 3);
		}
	}
}

/**********************************************************************************************************
*函 数 名: steering_task
*功能说明: 舵机控制任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(steering_task, pvParameters)
{
    portTickType xLastWakeTime;
	
	steering_pwm_init();
	USB_DEVICE_Init();
	
    xLastWakeTime = xTaskGetTickCount();
    while (!steering_task_exit)
    {
        
        //睡眠5ms
        vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: steering_task_create
*功能说明: 舵机控制相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void steering_task_create(void)
{
	steering_task_exit = 0;
    xTaskCreate(steering_task, "steering_task", STEERING_TASK_STACK, NULL, STEERING_TASK_PRIORITY, &steering_task_handle);
}
