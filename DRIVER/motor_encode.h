#ifndef _MOTOR_ENCODE_H
#define _MOTOR_ENCODE_H

#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t motor_encode_lf_queue;
extern QueueHandle_t motor_encode_lb_queue;
extern QueueHandle_t motor_encode_rf_queue;
extern QueueHandle_t motor_encode_rb_queue;

void motor_encode_init(void);

#endif
