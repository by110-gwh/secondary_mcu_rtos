#ifndef __AHRS_TASK_H
#define __AHRS_TASK_H

#include <stdint.h>

extern volatile uint8_t ahrs_task_exit;

void ahrs_task_create(void);

#endif
