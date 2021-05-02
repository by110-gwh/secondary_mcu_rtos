#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "usbd_custom_hid_if.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define STEERING_TASK_STACK 256
//�������ȼ�
#define STEERING_TASK_PRIORITY 13

//����������
xTaskHandle steering_task_handle;
//�����˳���־
volatile uint8_t steering_task_exit;

void steering_control_data_in_callback(uint8_t *pdata, uint32_t len)
{
	//ǰ׺0xFF
	if (pdata[0] == 0xFF) {
		//������
		if (pdata[1] == 0 && pdata[2] == 0x12) {
			uint8_t send_buf[3] = {0xFF, 0xF0, 0x12};
			USBD_CUSTOM_HID_SendReport_FS(send_buf, 3);
		}
	}
}

/**********************************************************************************************************
*�� �� ��: steering_task
*����˵��: �����������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(steering_task, pvParameters)
{
    portTickType xLastWakeTime;
	
	steering_pwm_init();
	USB_DEVICE_Init();
	
    xLastWakeTime = xTaskGetTickCount();
    while (!steering_task_exit)
    {
        
        //˯��5ms
        vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: steering_task_create
*����˵��: �������������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void steering_task_create(void)
{
	steering_task_exit = 0;
    xTaskCreate(steering_task, "steering_task", STEERING_TASK_STACK, NULL, STEERING_TASK_PRIORITY, &steering_task_handle);
}
