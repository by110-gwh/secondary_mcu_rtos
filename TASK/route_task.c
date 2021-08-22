#include "route_task.h"
#include "motion_control_task.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define ROUTE_TASK_TASK_STACK 256
//�������ȼ�
#define ROUTE_TASK_TASK_PRIORITY 12

//����������
static xTaskHandle route_task_handle;
//�����˳���־
volatile uint8_t route_task_exit = 1;

/**********************************************************************************************************
*�� �� ��: route_task
*����˵��: ·������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(route_task, pvParameters)
{
    while (!route_task_exit) {
        motor_pid_data_lf.expect = 200;
        motor_pid_data_lb.expect = 200;
        motor_pid_data_rf.expect = 200;
        motor_pid_data_rb.expect = 200;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = 0;
        motor_pid_data_lb.expect = 0;
        motor_pid_data_rf.expect = 0;
        motor_pid_data_rb.expect = 0;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = -200;
        motor_pid_data_lb.expect = -200;
        motor_pid_data_rf.expect = -200;
        motor_pid_data_rb.expect = -200;
        vTaskDelay(1000);
        motor_pid_data_lf.expect = 0;
        motor_pid_data_lb.expect = 0;
        motor_pid_data_rf.expect = 0;
        motor_pid_data_rb.expect = 0;
        vTaskDelay(1000);
    }
    vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: route_task_create
*����˵��: ·��������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void route_task_create(void)
{
	route_task_exit = 0;
    xTaskCreate(route_task, "motion_control_task", ROUTE_TASK_TASK_STACK, NULL, ROUTE_TASK_TASK_PRIORITY, &route_task_handle);
}
