#ifndef _STEERING_TASK_H
#define _STEERING_TASK_H

#include <stdint.h>

extern volatile uint8_t steering_task_exit;

void steering_control_data_in_callback(uint8_t *pdata, uint32_t len);
void steering_task_create(void);

#endif
