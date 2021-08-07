#ifndef _STEERING_TASK_H
#define _STEERING_TASK_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

//动作组存放起始地址，笔芯4K字节对齐
#define ACTION_GROUP_SAVE_ADDR 0

typedef __packed struct pos {
    uint8_t steering_num;
    uint16_t pos;
} pos_t;

typedef __packed struct action {
    uint8_t steering_num;
    uint16_t time;
    pos_t steering[32];
} action_t;

extern QueueHandle_t rec_data_queue;
extern volatile float steering_speed[32];
extern volatile uint16_t steering_position[32];
extern volatile uint8_t steering_task_exit;

void steering_task_create(void);

#endif
