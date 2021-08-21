#include "motion_control_task.h"
#include "motor_pwm.h"
#include "motor_encode.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//�����ջ��С
#define MOTION_CONTROL_TASK_STACK 256
//�������ȼ�
#define MOTION_CONTROL_TASK_PRIORITY 12

//����������
xTaskHandle motion_control_task_handle;
//�����˳���־
volatile uint8_t motion_control_task_exit = 1;

/**********************************************************************************************************
*�� �� ��: motion_control_task
*����˵��: �˶�����������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(motion_control_task, pvParameters)
{
    float motor_encode_lf_fb = 0;
    float motor_encode_lb_fb = 0;
    float motor_encode_rf_fb = 0;
    float motor_encode_rb_fb = 0;
    portTickType xLastWakeTime;
    
    motor_pwm_init();
    motor_encode_init();
    vTaskDelay(100);
    
    //��ȡϵͳ��ǰ����
    xLastWakeTime = xTaskGetTickCount();
    while (!motion_control_task_exit) {
        int motor_encode;
        int motor_encode_sum = 0;
        int motor_encode_cnt = 0;
        //��ǰ�����
        while (xQueueReceive(motor_encode_rf_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_rf_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_rf_fb = 1000000.0 / motor_encode_sum;
        }
        //�Һ󷽵��
        while (xQueueReceive(motor_encode_rb_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_rb_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_rb_fb = 1000000.0 / motor_encode_sum;
        }
        //��ǰ�����
        while (xQueueReceive(motor_encode_lf_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_lf_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_lf_fb = 1000000.0 / motor_encode_sum;
        }
        //��󷽵��
        while (xQueueReceive(motor_encode_lb_queue, &motor_encode, 0) == pdPASS) {
            if (motor_encode == 0xFFFFFFFF) {
                motor_encode_lb_fb = 0;
                break;
            }
            motor_encode_sum += motor_encode;
            motor_encode_cnt++;
        }
        if (motor_encode_cnt) {
            motor_encode_sum /= motor_encode_cnt;
            motor_encode_lb_fb = 1000000.0 / motor_encode_sum;
        }
        
        
        //˯��5ms
        vTaskDelayUntil(&xLastWakeTime, (5 / portTICK_RATE_MS));
    }
    vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: motion_control_task_create
*����˵��: �˶�������������񴴽�
*��    ��: �������� �ظ����д���
*�� �� ֵ: ��
**********************************************************************************************************/
void motion_control_task_create(void)
{
	motion_control_task_exit = 0;
    xTaskCreate(motion_control_task, "motion_control_task", MOTION_CONTROL_TASK_STACK, NULL, MOTION_CONTROL_TASK_PRIORITY, &motion_control_task_handle);
}
