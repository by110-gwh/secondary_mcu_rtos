#ifndef _STEERING_ACTION_TASK_H
#define _STEERING_ACTION_TASK_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

extern xTaskHandle steering_action_task_handle;
extern volatile uint8_t steering_action_task_exit;

void steering_action_task_create(uint8_t action_group);

#endif
