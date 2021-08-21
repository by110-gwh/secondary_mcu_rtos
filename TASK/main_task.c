#include "main_task.h"
#include "steering_task.h"
#include "usmart_task.h"
#include "time_cnt.h"
#include "i2c1.h"
#include "paramer_save.h"
#include "ahrs_task.h"
#include "imu_temp_task.h"
#include "qspi.h"
#include "motion_control_task.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define MAIN_TASK_STACK            512
//�������ȼ�
#define MAIN_TASK_PRIORITY         5

//����������
xTaskHandle main_task_handle;
//�����˳���־
volatile uint8_t main_task_exit;

/**********************************************************************************************************
*�� �� ��: main_task
*����˵��: ������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(main_task, parameters)
{
    qspi_init();
    Get_Time_Init();
    i2c1_init();
    read_save_paramer();
    steering_task_create();
    usmart_task_create();
    ahrs_task_create();
    imu_temp_task_create();
    motion_control_task_create();
    while (!main_task_exit) {
		vTaskDelay(1000);
    }
	vTaskDelete(NULL);
}

/**********************************************************************************************************
*�� �� ��: main_task_create
*����˵��: ������������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void main_task_create(void)
{
	main_task_exit = 0;
    xTaskCreate(main_task, "main_task", MAIN_TASK_STACK, NULL, MAIN_TASK_PRIORITY, &main_task_handle);
}
