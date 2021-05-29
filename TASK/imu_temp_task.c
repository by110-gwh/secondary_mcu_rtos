#include "imu_temp_task.h"
#include "imu.h"
#include "stm32f7xx_hal.h"
#include "pid.h"

#include "FreeRTOS.h"
#include "task.h"

//�����ջ��С
#define IMU_TEMP_TASK_STACK 512
//�������ȼ�
#define IMU_TEMP_TASK_PRIORITY 12

//����������
xTaskHandle imu_temp_task_handle;
//�����˳���־
volatile uint8_t imu_temp_task_exit;
//�¶ȿ���PID
pid_controler_t imu_temp_pid;

/**********************************************************************************************************
*�� �� ��: imu_temp_pid_init
*����˵��: ���������¿���PID��ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void imu_temp_pid_init(void)
{
    imu_temp_pid.last_expect = 0;
    imu_temp_pid.expect = 0;
    imu_temp_pid.feedback = 0;

    imu_temp_pid.err = 0;
    imu_temp_pid.last_err = 0;
    imu_temp_pid.err_max = 0;

    imu_temp_pid.integrate_separation_err = 2;
    imu_temp_pid.integrate = 0;
    imu_temp_pid.integrate_max = 30;

    imu_temp_pid.dis_err = 0;

    imu_temp_pid.kp = 30;
    imu_temp_pid.ki = 0.5;
    imu_temp_pid.kd = 0;
    
    imu_temp_pid.feedforward_kp = 0;
    imu_temp_pid.feedforward_kd = 0;

    imu_temp_pid.control_output = 0;
    imu_temp_pid.control_output_limit = 100;

    imu_temp_pid.short_circuit_flag = 0;
    imu_temp_pid.err_callback = NULL;
    imu_temp_pid.pri_data = NULL;
}

/**********************************************************************************************************
*�� �� ��: imu_temp_gpio_init
*����˵��: ���������¿���IO��ʼ�����û�����д�˺���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void imu_temp_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	//ʹ��GPIOʱ��
	__HAL_RCC_GPIOC_CLK_ENABLE();
    
	//��ʼ��IO����
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/**********************************************************************************************************
*�� �� ��: imu_temp_task
*����˵��: ���������¿�������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(imu_temp_task, pvParameters)
{
    uint8_t count;
    
    imu_temp_gpio_init();
    imu_temp_pid_init();
    count = 0;
    while (!imu_temp_task_exit)
    {
        
        //�¶�PID����
        if (count == 100) {
            imu_temp_pid.feedback = tempDataFilter;
            imu_temp_pid.expect = 50;
            pid_control(&imu_temp_pid);
        }
        
        //ģ��PWM���
        if (count == 100)
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
        else if (count >= imu_temp_pid.control_output)
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
        
        if (count == 100)
            count = 0;
        else
            count++;
        
        //˯��5ms
        vTaskDelay(1);
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: imu_temp_task_create
*����˵��: ���������¿���������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void imu_temp_task_create(void)
{
	imu_temp_task_exit = 0;
    xTaskCreate(imu_temp_task, "imu_temp_task", IMU_TEMP_TASK_STACK, NULL, IMU_TEMP_TASK_PRIORITY, &imu_temp_task_handle);
}
