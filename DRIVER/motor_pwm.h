#ifndef _MOTOR_PWM_H
#define _MOTOR_PWM_H

void motor_pwm_init(void);
void motor_pwm_set(int pwm_lf, int pwm_lb, int pwm_rf, int pwm_rb);

#endif
