#include "steering_task.h"
#include "steering_updata_task.h"
#include "steering_action_task.h"
#include "usbd_custom_hid_if.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"
#include "w25q.h"
#include "paramer_save.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//�����ջ��С
#define STEERING_TASK_STACK 512
//�������ȼ�
#define STEERING_TASK_PRIORITY 11

//����������
xTaskHandle steering_task_handle;
//�����˳���־
volatile uint8_t steering_task_exit;

//�������ݶ���
QueueHandle_t rec_data_queue;
//����ͨ��������ٶȣ�ÿ20ms���ӻ����2us
volatile float steering_speed[32];
//����ͨ���ߵ�ƽʱ�䣬��λus,��Χ0 - 4500
volatile uint16_t steering_position[32];
//������ݻ���
uint8_t usb_hid_buffer[128];

/**********************************************************************************************************
*�� �� ��: action_group_delete_all
*����˵��: ȫ��������ɾ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void action_group_delete_all()
{
    uint8_t send_buf[4];
    uint8_t i;
    for (i = 0; i < 231; i++)
        paramer_save_data.action_num[i] = 0;
    write_save_paramer();
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    send_buf[2] = 2;
    send_buf[3] = 8;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 4);
}

/**********************************************************************************************************
*�� �� ��: action_group_delete
*����˵��: ������ɾ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void action_group_delete(uint8_t action_group_num)
{
    uint8_t send_buf[4];
    paramer_save_data.action_num[action_group_num] = 0;
    write_save_paramer();
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    send_buf[2] = 2;
    send_buf[3] = 8;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 4);
}

/**********************************************************************************************************
*�� �� ��: action_download
*����˵��: ����������
*��    ��: �������� �����鶯������
*�� �� ֵ: ��
**********************************************************************************************************/
static void action_download(uint8_t action_group_num, uint8_t action_cnt)
{
    uint32_t count;
    uint8_t send_buf[6];
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    
    //���涯����
    paramer_save_data.action_num[action_group_num] = action_cnt;
    write_save_paramer();
    
    //����flash��ÿ��������ռ13����
    w25q_erase(ACTION_GROUP_SAVE_ADDR + action_group_num * 13 * 4096, action_cnt * sizeof(action_t));
    
    //��λ��Э��
    send_buf[2] = 4;
    send_buf[3] = 5;
    send_buf[4] = 1;
    send_buf[5] = 0;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
    
    if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY) == pdFALSE)
        return;
    send_buf[2] = 4;
    send_buf[3] = 5;
    send_buf[4] = 2;
    send_buf[5] = 0;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
    
    if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY) == pdFALSE)
        return;
    send_buf[2] = 4;
    send_buf[3] = 5;
    send_buf[4] = 3;
    send_buf[5] = 0;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
    
    count = 0;
    while (action_cnt--) {
        if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY) == pdFALSE)
            return;
        if (xQueueReceive(rec_data_queue, usb_hid_buffer + 64, portMAX_DELAY) == pdFALSE)
            return;
        
        action_t *action = (action_t *)(usb_hid_buffer + 8);
        //д��flash
        w25q_write((uint8_t *)action, ACTION_GROUP_SAVE_ADDR + action_group_num * 13 * 4096 + count * sizeof(action_t), sizeof(action_t));
        
        //���ճɹ�Ӧ��
        send_buf[2] = 4;
        send_buf[3] = 5;
        send_buf[4] = 4;
        send_buf[5] = 0;
        USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
        count++;
    }
    
    //��λ��Э��
    if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY) == pdFALSE)
        return;
    send_buf[2] = 4;
    send_buf[3] = 5;
    send_buf[4] = 5;
    send_buf[5] = 0;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
}

/**********************************************************************************************************
*�� �� ��: send_battery_voltage
*����˵��: ���͵�ص�ѹ
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void send_battery_voltage()
{
    uint8_t send_buf[6];
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    send_buf[2] = 4;
    send_buf[3] = 15;
    send_buf[4] = 5000 & 0xFF;
    send_buf[5] = 5000 >> 8;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
}

/**********************************************************************************************************
*�� �� ��: send_steering_err
*����˵��: ���Ͷ�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void send_steering_err()
{
    typedef __packed struct err_data {
        uint16_t header;
        uint8_t len;
        uint8_t cmd;
        int8_t err[32];
    } send_data_t;
    
    uint8_t i;
    send_data_t send_data = {
        .header = 0x5555,
        .len = 34,
        .cmd = 13
    };
    
    for (i = 0; i < 32; i++)
        send_data.err[i] = paramer_save_data.steering_offset[i];
    USBD_CUSTOM_HID_SendReport_FS((uint8_t *)&send_data, 26);
}

/**********************************************************************************************************
*�� �� ��: download_steering_err
*����˵��: ���ض�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void download_steering_err(int8_t steering_err[32])
{
    uint8_t i;
    uint8_t send_buf[4];
    
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    send_buf[2] = 2;
    send_buf[3] = 12;
    
    for (i = 0; i < 32; i++) {
        paramer_save_data.steering_offset[i] = steering_err[i];
        steering_pulse_offset[i] = steering_err[i];
    }
    write_save_paramer();
    
    USBD_CUSTOM_HID_SendReport_FS((uint8_t *)&send_buf, 4);
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
	
    //��ʼ�����λ�ú��ٶ�
	for (i = 0; i < 32; i++) {
		steering_position[i] = 0;
		steering_speed[i] = 0;
        steering_pulse_offset[i] = paramer_save_data.steering_offset[i];
	}
    //��ʼ��������ݶ���
	rec_data_queue = xQueueCreate(2, sizeof(usb_hid_buffer));
    //���PWM��ʼ��
	steering_pwm_init();
    //USB�ӿڳ�ʼ��
	USB_DEVICE_Init();
    //���λ�ø��������ʼ��
	steering_updata_task_create();
	
    while (!steering_task_exit)
    {
		//����USB����
		if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY)) {
            //���յ���ȷ�����ݰ�
            if (usb_hid_buffer[0] == 0x55 && usb_hid_buffer[1] == 0x55) {
                //���ö��λ��
                if (usb_hid_buffer[3] == 2) {
                    steering_speed[usb_hid_buffer[4]] = 0;
                    steering_position[usb_hid_buffer[4]] = usb_hid_buffer[6] << 8 | usb_hid_buffer[5];
                //���ö��λ�ô���ʱ
                } else if (usb_hid_buffer[3] == 3) {
                    action_t *action = (action_t *)(usb_hid_buffer + 4);
                    uint8_t index = 0;
                    //32·���һ�����������ݰ�
                    if (action->steering_num > 16)
                        xQueueReceive(rec_data_queue, usb_hid_buffer + 64, portMAX_DELAY);
                    //���ö��λ�ú��ٶ�
                    while (action->steering_num--) {
                        vTaskSuspendAll();
                        steering_position[action->steering[index].steering_num] = action->steering[index].pos;
                        steering_speed[action->steering[index].steering_num] = (steering_position[action->steering[index].steering_num] - steering_pulse_ch[action->steering[index].steering_num])  * 20.0f / action->time;
                        xTaskResumeAll();
                        index++;
                    }
                //����������
                } else if (usb_hid_buffer[3] == 5) {
					action_download(usb_hid_buffer[5], usb_hid_buffer[6]);
                //���������
                } else if (usb_hid_buffer[3] == 8) {
                    //ȫ�����������
                    if (usb_hid_buffer[4] == 0xFF)
                        action_group_delete_all();
                    //�������������
                    else
                        action_group_delete(usb_hid_buffer[4]);
                //������ִ��
                } else if (usb_hid_buffer[3] == 6) {
                    steering_action_task_create(usb_hid_buffer[4], usb_hid_buffer[5] | usb_hid_buffer[6] << 8);
                //������ֹͣ
                } else if (usb_hid_buffer[3] == 7) {
                    steering_action_task_exit = 1;
                //��ȡ��ص�ѹ
                } else if (usb_hid_buffer[3] == 15) {
                    send_battery_voltage();
                //�������΢��
                } else if (usb_hid_buffer[3] == 14) {
                    steering_pulse_offset[usb_hid_buffer[4]] = (usb_hid_buffer[5] | usb_hid_buffer[6] << 8) - 1500;
                //����ȡ
                } else if (usb_hid_buffer[3] == 13) {
                    send_steering_err();
                //���д��
                } else if (usb_hid_buffer[3] == 12) {
                    download_steering_err((int8_t *)usb_hid_buffer + 4);
				}
            }
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
