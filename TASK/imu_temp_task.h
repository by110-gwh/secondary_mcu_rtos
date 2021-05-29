#ifndef __IMU_TEMP_TASK_H
#define __IMU_TEMP_TASK_H

#include <stdint.h>

extern volatile uint8_t imu_temp_task_exit;

void imu_temp_task_create(void);

#endif
