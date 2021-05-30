#include "steering_action_task.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "w25q.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define STEERING_ACTION_TASK_STACK 256
//任务优先级
#define STEERING_ACTION_TASK_PRIORITY 12

//声明任务句柄
xTaskHandle steering_action_task_handle;
//任务退出标志
volatile uint8_t steering_action_task_exit;

//动作组语句保存数组
action_group_t read_action_group;

/**********************************************************************************************************
*函 数 名: steering_action_task
*功能说明: 舵机动作组执行任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(steering_action_task, pvParameters)
{
	uint32_t index;
	
	//寻找动作组开始位置
	for (index = 0; index < 8192; index++) {
		//读出动作组语句
		w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
		if (read_action_group.cmd == 9 && read_action_group.data == *((uint16_t *)pvParameters)) {
			break;
		}
	}
	index++;
	//读出动作组语句
	w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
	//执行语句
	while (read_action_group.cmd != 9 && index < 8192 && !steering_action_task_exit) {
		//请求设置速度
		if (read_action_group.cmd == 1) {
			steering_speed[read_action_group.chanel] = read_action_group.data * 2;
		}
		//请求设置位置
		if (read_action_group.cmd == 2) {
			steering_position[read_action_group.chanel] = read_action_group.data;
		}
		//延时语句
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
		//读出动作组语句
		w25q_read((uint8_t *)&read_action_group, ACTION_GROUP_SAVE_ADDR + index * sizeof(action_group_t), sizeof(action_group_t));
	}
TASK_EXIT:
	steering_action_task_handle = 0;
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: steering_action_task_create
*功能说明: 舵机动作组执行相关任务创建
*形    参: 动作组编号
*返 回 值: 无
**********************************************************************************************************/
void steering_action_task_create(uint8_t action_group)
{
	steering_action_task_exit = 0;
    xTaskCreate(steering_action_task, "steering_action_task", STEERING_ACTION_TASK_STACK, &action_group, STEERING_ACTION_TASK_PRIORITY, &steering_action_task_handle);
}
