#ifndef _MOTION_CONTROL_TASK_H
#define _MOTION_CONTROL_TASK_H

#include "pid.h"
#include <stdint.h>

extern pid_data_t motor_pid_data_lf;
extern pid_data_t motor_pid_data_lb;
extern pid_data_t motor_pid_data_rf;
extern pid_data_t motor_pid_data_rb;

void motion_control_task_create(void);

#endif
