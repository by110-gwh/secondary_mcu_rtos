#include "ahrs_task.h"
#include "ahrs.h"
#include "imu.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define AHRS_TASK_STACK 512
//�������ȼ�
#define AHRS_TASK_PRIORITY 13

//����������
xTaskHandle ahrs_task_handle;
//�����˳���־
volatile uint8_t ahrs_task_exit;

/**********************************************************************************************************
*�� �� ��: ahrs_task
*����˵��: ���ж�ʱ����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(ahrs_task, pvParameters)
{
    portTickType xLastWakeTime;

    //���������
    //vTaskSuspendAll();
	
	imu_init();
	ahrs_init();
    gyro_calibration();
    //���ѵ�����
    //xTaskResumeAll();


    xLastWakeTime = xTaskGetTickCount();
    while (!ahrs_task_exit)
    {
        
		//��ȡimu����
		get_imu_data();
		ahrs_update();
        //˯��5ms
        vTaskDelayUntil(&xLastWakeTime, (5 / portTICK_RATE_MS));
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: ahrs_task_create
*����˵��: ���������������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void ahrs_task_create(void)
{
	ahrs_task_exit = 0;
    xTaskCreate(ahrs_task, "ahrs_task", AHRS_TASK_STACK, NULL, AHRS_TASK_PRIORITY, &ahrs_task_handle);
}
