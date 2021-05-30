#include "steering_action_task.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "w25q.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define STEERING_ACTION_TASK_STACK 256
//�������ȼ�
#define STEERING_ACTION_TASK_PRIORITY 12

//����������
xTaskHandle steering_action_task_handle;
//�����˳���־
volatile uint8_t steering_action_task_exit;

//��������䱣������
action_group_t read_action_group;

/**********************************************************************************************************
*�� �� ��: steering_action_task
*����˵��: ���������ִ������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(steering_action_task, pvParameters)
{
	uint32_t index;
	
	//Ѱ�Ҷ����鿪ʼλ��
	for (index = 0; index < 8192; index++) {
		//�������������
		w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
		if (read_action_group.cmd == 9 && read_action_group.data == *((uint16_t *)pvParameters)) {
			break;
		}
	}
	index++;
	//�������������
	w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
	//ִ�����
	while (read_action_group.cmd != 9 && index < 8192 && !steering_action_task_exit) {
		//���������ٶ�
		if (read_action_group.cmd == 1) {
			steering_speed[read_action_group.chanel] = read_action_group.data * 2;
		}
		//��������λ��
		if (read_action_group.cmd == 2) {
			steering_position[read_action_group.chanel] = read_action_group.data;
		}
		//��ʱ���
		if (read_action_group.cmd == 6) {
			uint32_t delay_cnt;
			for (delay_cnt = 0; delay_cnt < read_action_group.data; delay_cnt++) {
				vTaskDelay(1);
				if (steering_action_task_exit) {
					goto TASK_EXIT;
				}
			}
		}
		index++;
		//�������������
		w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
	}
TASK_EXIT:
	steering_action_task_handle = 0;
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: steering_action_task_create
*����˵��: ���������ִ��������񴴽�
*��    ��: ��������
*�� �� ֵ: ��
**********************************************************************************************************/
void steering_action_task_create(uint8_t action_group)
{
	steering_action_task_exit = 0;
    xTaskCreate(steering_action_task, "steering_action_task", STEERING_ACTION_TASK_STACK, &action_group, STEERING_ACTION_TASK_PRIORITY, &steering_action_task_handle);
}
