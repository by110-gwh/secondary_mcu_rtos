#ifndef _STEERING_ACTION_TASK_H
#define _STEERING_ACTION_TASK_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

extern volatile uint8_t steering_action_task_exit;

void steering_action_task_create(uint8_t action_group, uint16_t run_cnt);

#endif
