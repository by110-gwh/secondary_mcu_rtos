#include "steering_action_task.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "w25q.h"
#include "paramer_save.h"

#include "FreeRTOS.h"
#include "task.h"

//任务堆栈大小
#define STEERING_ACTION_TASK_STACK 256
//任务优先级
#define STEERING_ACTION_TASK_PRIORITY 12

//声明任务句柄
xTaskHandle steering_action_task_handle;
//任务退出标志
volatile uint8_t steering_action_task_exit = 1;
//重复运行次数
uint16_t repeat_cnt;

/**********************************************************************************************************
*函 数 名: steering_action_task
*功能说明: 舵机动作组执行任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(steering_action_task, pvParameters)
{
    uint8_t i;
    action_t action;
	uint32_t action_index;
    uint8_t action_group_num = *((uint16_t *)pvParameters);
    //重复运行
    while (repeat_cnt--) {
        //运行动作组
        for (action_index = 0; action_index < paramer_save_data.action_num[action_group_num]; action_index++) {
            //读出动作组语句
            w25q_read((uint8_t *)&action, ACTION_GROUP_SAVE_ADDR + action_group_num * 13 * 4096 + action_index * sizeof(action_t), sizeof(action_t));
            //执行动作组
            for (i = 0; i < action.steering_num; i++) {
                vTaskSuspendAll();
                steering_position[action.steering[i].steering_num] = action.steering[i].pos;
                steering_speed[action.steering[i].steering_num] = (steering_position[action.steering[i].steering_num] - steering_pulse_ch[action.steering[i].steering_num])  * 20.0f / action.time;
                xTaskResumeAll();
            }
            vTaskDelay(action.time);
            if (steering_action_task_exit)
                goto TASK_EXIT;
        }
    }
    steering_action_task_exit = 1;
TASK_EXIT:
	vTaskDelete(NULL);
    steering_action_task_handle = NULL;
}


/**********************************************************************************************************
*函 数 名: steering_action_task_create
*功能说明: 舵机动作组执行相关任务创建
*形    参: 动作组编号 重复运行次数
*返 回 值: 无
**********************************************************************************************************/
void steering_action_task_create(uint8_t action_group, uint16_t run_cnt)
{
    //上一次的任务还没结束
    if (steering_action_task_exit == 0) {
        return;
    }
    //判断此动作组是否已经编程
    if (paramer_save_data.action_num[action_group] == 0) {
        steering_action_task_exit = 1;
        return;
    }
	steering_action_task_exit = 0;
    if (run_cnt == 0)
        run_cnt = 1;
    repeat_cnt = run_cnt;
    xTaskCreate(steering_action_task, "steering_action_task", STEERING_ACTION_TASK_STACK, &action_group, STEERING_ACTION_TASK_PRIORITY, &steering_action_task_handle);
}
