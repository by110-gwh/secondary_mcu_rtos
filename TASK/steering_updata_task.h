#ifndef _STEERING_UPDATA_TASK_H
#define _STEERING_UPDATA_TASK_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

extern xTaskHandle steering_updata_task_handle;
extern volatile uint8_t steering_updata_task_exit;
extern volatile uint8_t steering_updata_task_hang;

void steering_updata_task_create(void);

#endif
