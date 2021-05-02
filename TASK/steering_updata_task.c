#include "steering_updata_task.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define STEERING_UPDATA_TASK_STACK 256
//�������ȼ�
#define STEERING_UPDATA_TASK_PRIORITY 13

//����������
xTaskHandle steering_updata_task_handle;
//�����˳���־
volatile uint8_t steering_updata_task_exit;

/**********************************************************************************************************
*�� �� ��: steering_updata_task
*����˵��: �����������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(steering_updata_task, pvParameters)
{
    portTickType xLastWakeTime;
	
    xLastWakeTime = xTaskGetTickCount();
    while (!steering_updata_task_exit)
    {
		uint8_t i;
		//16ͨ���������һ��
		for (i = 0; i < 16; i++) {
			//��Ҫ���¶���Ƕ�
			if (steering_pulse_ch[i] != steering_position[i]) {
				//��ʼ��ֵ
				if (steering_pulse_ch[i] == 0)
					steering_pulse_ch[i] = steering_position[i];
				//�Ƕȼ�С����
				else if(steering_pulse_ch[i] > steering_position[i]) {
					//���ݲ�ֵѡ�񲽽�
					if (steering_pulse_ch[i] - steering_position[i] >= steering_speed[i])
						steering_pulse_ch[i] -=  steering_speed[i];
					else
						steering_pulse_ch[i] = steering_position[i];
				//�Ƕ�������
				} else {
					//���ݲ�ֵѡ�񲽽�
					if (steering_position[i] - steering_pulse_ch[i] >= steering_speed[i])
						steering_pulse_ch[i] +=  steering_speed[i];
					else
						steering_pulse_ch[i] = steering_position[i];
				}
			}
		}
        //˯��20ms
        vTaskDelayUntil(&xLastWakeTime, (20 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: steering_updata_task_create
*����˵��: �������������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void steering_updata_task_create(void)
{
	steering_updata_task_exit = 0;
    xTaskCreate(steering_updata_task, "steering_updata_task", STEERING_UPDATA_TASK_STACK, NULL, STEERING_UPDATA_TASK_PRIORITY, &steering_updata_task_handle);
}
