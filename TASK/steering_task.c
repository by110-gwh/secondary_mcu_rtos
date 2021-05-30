#include "steering_task.h"
#include "steering_updata_task.h"
#include "steering_action_task.h"
#include "usbd_custom_hid_if.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "w25q.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//�����ջ��С
#define STEERING_TASK_STACK 512
//�������ȼ�
#define STEERING_TASK_PRIORITY 11

typedef struct {
	uint8_t header;
	uint8_t cmd;
	uint8_t chanel;
	uint8_t data_l;
	uint8_t data_h;
} rec_cmd_t;

//����������
xTaskHandle steering_task_handle;
//�����˳���־
volatile uint8_t steering_task_exit;

//�������ݶ���
QueueHandle_t rec_data_queue;
//����ͨ��������ٶȣ�ÿ20ms���ӻ����2us
volatile uint8_t steering_speed[16];
//����ͨ���ߵ�ƽʱ�䣬��λus,��Χ0 - 4500
volatile uint16_t steering_position[16];

/**********************************************************************************************************
*�� �� ��: action_erase
*����˵��: ����������������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void action_erase(void)
{
	w25q_erase(ACTION_GROUP_SAVE_ADDR, 8192 * sizeof(action_group_t));
}

/**********************************************************************************************************
*�� �� ��: action_download
*����˵��: ����������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void action_download(void)
{
	rec_cmd_t rec_cmd;
	action_group_t tmp;
	uint32_t count;
	
	//����������������
	action_erase();
	//����������
	count = 0;
	
	//����flash
	HAL_FLASH_Unlock();
	while (1) {
		//����USB����
		if (xQueueReceive(rec_data_queue, &rec_cmd, portMAX_DELAY)) {
			//Ҫ��������
			if (rec_cmd.cmd == 9 || rec_cmd.cmd == 1 || rec_cmd.cmd == 2 || rec_cmd.cmd == 6) {
				tmp.cmd = rec_cmd.cmd;
				tmp.chanel = rec_cmd.chanel;
				tmp.data = rec_cmd.data_h << 8 | rec_cmd.data_l;
				//д��flash
				w25q_write((uint8_t *)&tmp, ACTION_GROUP_SAVE_ADDR + count * sizeof(action_group_t), sizeof(action_group_t));
				count++;
			} else if (rec_cmd.cmd == 0x0D){
			} else {
				break;
			}
			//����USB��������
			USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
		}
	}
	//flash����
	HAL_FLASH_Lock();
}

/**********************************************************************************************************
*�� �� ��: steering_task
*����˵��: �����������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
portTASK_FUNCTION(steering_task, pvParameters)
{
	uint8_t i;
	rec_cmd_t rec_cmd;
	
	for (i = 0; i < 16; i++) {
		steering_position[i] = 0;
		steering_speed[i] = 2 * 20;
	}
	rec_data_queue = xQueueCreate(2, sizeof(rec_cmd_t));
	steering_pwm_init();
	USB_DEVICE_Init();
	steering_updata_task_create();
	
    while (!steering_task_exit)
    {
		//����USB����
		if (xQueueReceive(rec_data_queue, &rec_cmd, portMAX_DELAY)) {
			//ǰ׺0xFF
			if (rec_cmd.header == 0xFF) {
				//������
				if (rec_cmd.cmd == 0 && rec_cmd.chanel == 0x12) {
					uint8_t send_buf[3] = {0xFF, 0xF0, 0x12};
					USBD_CUSTOM_HID_SendReport_FS(send_buf, 3);
				}
				//���������ٶ�
				if (rec_cmd.cmd == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					steering_updata_task_hang = 0;
					steering_speed[rec_cmd.chanel] = (rec_cmd.data_h << 8 | rec_cmd.data_l) * 2;
				}
				//��������λ��
				if (rec_cmd.cmd == 2) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					steering_updata_task_hang = 0;
					steering_position[rec_cmd.chanel] = rec_cmd.data_h << 8 | rec_cmd.data_l;
				}
				//����������
				if (rec_cmd.cmd == 10 && rec_cmd.data_l == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					steering_updata_task_hang = 0;
					USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
					action_download();
				}
				//���������
				if (rec_cmd.cmd == 0xFA && rec_cmd.data_l == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					steering_updata_task_hang = 0;
					action_erase();
				}
				//������ִ��
				if (rec_cmd.cmd == 9) {
					if (rec_cmd.data_l < 16)
						steering_action_task_create(rec_cmd.data_l);
				}
				//��ͣ����
				if (rec_cmd.cmd == 11 && rec_cmd.data_l == 1) {
					if (steering_action_task_handle)
						vTaskSuspend(steering_action_task_handle);
					steering_updata_task_hang = 1;
				}
				//�ָ�����
				if (rec_cmd.cmd == 11 && rec_cmd.data_l == 0) {
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					steering_updata_task_hang = 0;
				}
			}
			//����USB��������
			USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
		}
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*�� �� ��: steering_task_create
*����˵��: �������������񴴽�
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void steering_task_create(void)
{
	steering_task_exit = 0;
    xTaskCreate(steering_task, "steering_task", STEERING_TASK_STACK, NULL, STEERING_TASK_PRIORITY, &steering_task_handle);
}
