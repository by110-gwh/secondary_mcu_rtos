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

//任务堆栈大小
#define STEERING_TASK_STACK 512
//任务优先级
#define STEERING_TASK_PRIORITY 11

//声明任务句柄
xTaskHandle steering_task_handle;
//任务退出标志
volatile uint8_t steering_task_exit;

//接收数据队列
QueueHandle_t rec_data_queue;
//各个通道舵机的速度，每20ms增加或减少2us
volatile float steering_speed[32];
//各个通道高电平时间，单位us,范围0 - 4500
volatile uint16_t steering_position[32];
//舵控数据缓冲
uint8_t usb_hid_buffer[128];

/**********************************************************************************************************
*函 数 名: action_group_delete_all
*功能说明: 全部动作组删除
*形    参: 无
*返 回 值: 无
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
*函 数 名: action_group_delete
*功能说明: 动作组删除
*形    参: 无
*返 回 值: 无
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
*函 数 名: action_download
*功能说明: 动作组下载
*形    参: 动作组编号 动作组动作数量
*返 回 值: 无
**********************************************************************************************************/
static void action_download(uint8_t action_group_num, uint8_t action_cnt)
{
    uint32_t count;
    uint8_t send_buf[6];
    send_buf[0] = 0x55;
    send_buf[1] = 0x55;
    
    //保存动作组
    paramer_save_data.action_num[action_group_num] = action_cnt;
    write_save_paramer();
    
    //擦除flash，每个动作组占13扇区
    w25q_erase(ACTION_GROUP_SAVE_ADDR + action_group_num * 13 * 4096, action_cnt * sizeof(action_t));
    
    //上位机协议
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
        //写入flash
        w25q_write((uint8_t *)action, ACTION_GROUP_SAVE_ADDR + action_group_num * 13 * 4096 + count * sizeof(action_t), sizeof(action_t));
        
        //接收成功应答
        send_buf[2] = 4;
        send_buf[3] = 5;
        send_buf[4] = 4;
        send_buf[5] = 0;
        USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
        count++;
    }
    
    //上位机协议
    if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY) == pdFALSE)
        return;
    send_buf[2] = 4;
    send_buf[3] = 5;
    send_buf[4] = 5;
    send_buf[5] = 0;
    USBD_CUSTOM_HID_SendReport_FS(send_buf, 6);
}

/**********************************************************************************************************
*函 数 名: send_battery_voltage
*功能说明: 发送电池电压
*形    参: 无
*返 回 值: 无
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
*函 数 名: send_steering_err
*功能说明: 发送舵机误差
*形    参: 无
*返 回 值: 无
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
*函 数 名: download_steering_err
*功能说明: 下载舵机误差
*形    参: 无
*返 回 值: 无
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
*函 数 名: steering_task
*功能说明: 舵机控制任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(steering_task, pvParameters)
{
	uint8_t i;
	
    //初始化舵机位置和速度
	for (i = 0; i < 32; i++) {
		steering_position[i] = 0;
		steering_speed[i] = 0;
        steering_pulse_offset[i] = paramer_save_data.steering_offset[i];
	}
    //初始化舵控数据队列
	rec_data_queue = xQueueCreate(2, sizeof(usb_hid_buffer));
    //舵机PWM初始化
	steering_pwm_init();
    //USB接口初始化
	USB_DEVICE_Init();
    //舵机位置更新任务初始化
	steering_updata_task_create();
	
    while (!steering_task_exit)
    {
		//接收USB数据
		if (xQueueReceive(rec_data_queue, usb_hid_buffer, portMAX_DELAY)) {
            //接收到正确的数据包
            if (usb_hid_buffer[0] == 0x55 && usb_hid_buffer[1] == 0x55) {
                //设置舵机位置
                if (usb_hid_buffer[3] == 2) {
                    steering_speed[usb_hid_buffer[4]] = 0;
                    steering_position[usb_hid_buffer[4]] = usb_hid_buffer[6] << 8 | usb_hid_buffer[5];
                //设置舵机位置带延时
                } else if (usb_hid_buffer[3] == 3) {
                    action_t *action = (action_t *)(usb_hid_buffer + 4);
                    uint8_t index = 0;
                    //32路舵机一共有两个数据包
                    if (action->steering_num > 16)
                        xQueueReceive(rec_data_queue, usb_hid_buffer + 64, portMAX_DELAY);
                    //设置舵机位置和速度
                    while (action->steering_num--) {
                        vTaskSuspendAll();
                        steering_position[action->steering[index].steering_num] = action->steering[index].pos;
                        steering_speed[action->steering[index].steering_num] = (steering_position[action->steering[index].steering_num] - steering_pulse_ch[action->steering[index].steering_num])  * 20.0f / action->time;
                        xTaskResumeAll();
                        index++;
                    }
                //动作组下载
                } else if (usb_hid_buffer[3] == 5) {
					action_download(usb_hid_buffer[5], usb_hid_buffer[6]);
                //动作组擦除
                } else if (usb_hid_buffer[3] == 8) {
                    //全部动作组擦除
                    if (usb_hid_buffer[4] == 0xFF)
                        action_group_delete_all();
                    //单个动作组擦除
                    else
                        action_group_delete(usb_hid_buffer[4]);
                //动作组执行
                } else if (usb_hid_buffer[3] == 6) {
                    steering_action_task_create(usb_hid_buffer[4], usb_hid_buffer[5] | usb_hid_buffer[6] << 8);
                //动作组停止
                } else if (usb_hid_buffer[3] == 7) {
                    steering_action_task_exit = 1;
                //获取电池电压
                } else if (usb_hid_buffer[3] == 15) {
                    send_battery_voltage();
                //误差在线微调
                } else if (usb_hid_buffer[3] == 14) {
                    steering_pulse_offset[usb_hid_buffer[4]] = (usb_hid_buffer[5] | usb_hid_buffer[6] << 8) - 1500;
                //误差读取
                } else if (usb_hid_buffer[3] == 13) {
                    send_steering_err();
                //误差写入
                } else if (usb_hid_buffer[3] == 12) {
                    download_steering_err((int8_t *)usb_hid_buffer + 4);
				}
            }
		}
    }
	vTaskDelete(NULL);
}


/**********************************************************************************************************
*函 数 名: steering_task_create
*功能说明: 舵机控制相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void steering_task_create(void)
{
	steering_task_exit = 0;
    xTaskCreate(steering_task, "steering_task", STEERING_TASK_STACK, NULL, STEERING_TASK_PRIORITY, &steering_task_handle);
}
