#include "steering_task.h"
#include "steering_updata_task.h"
#include "steering_action_task.h"
#include "usbd_custom_hid_if.h"
#include "steering_task.h"
#include "steering_pwm.h"
#include "usb_device.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//任务堆栈大小
#define STEERING_TASK_STACK 256
//任务优先级
#define STEERING_TASK_PRIORITY 11

typedef struct {
	uint8_t header;
	uint8_t cmd;
	uint8_t chanel;
	uint8_t data_l;
	uint8_t data_h;
} rec_cmd_t;

//声明任务句柄
xTaskHandle steering_task_handle;
//任务退出标志
volatile uint8_t steering_task_exit;

//接收数据队列
QueueHandle_t rec_data_queue;
//动作组语句保存数组
action_group_t action_group_save[8192] __attribute__((at(0x08020000)));
//各个通道舵机的速度，每20ms增加或减少2us
volatile uint8_t steering_speed[16];
//各个通道高电平时间，单位us,范围0 - 4500
volatile uint16_t steering_position[16];

/**********************************************************************************************************
*函 数 名: action_erase
*功能说明: 擦除动作保存区域
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void action_erase(void)
{
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t PageError;
	__disable_irq();
	//解锁flash
	HAL_FLASH_Unlock();
	
	//擦除动作保存区域，在第二扇区
	pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_4;
	pEraseInit.Sector = 5;
	pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	pEraseInit.NbSectors = 1;
	pEraseInit.Banks = FLASH_BANK_1;
	HAL_FLASHEx_Erase(&pEraseInit, &PageError);
	//flash上锁
	HAL_FLASH_Lock();
	__enable_irq();
}

/**********************************************************************************************************
*函 数 名: action_download
*功能说明: 动作组下载
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void action_download(void)
{
	rec_cmd_t rec_cmd;
	action_group_t tmp;
	uint32_t count;
	
	//擦除动作保存区域
	action_erase();
	//语句计数清零
	count = 0;
	
	//解锁flash
	HAL_FLASH_Unlock();
	while (1) {
		//接收USB数据
		if (xQueueReceive(rec_data_queue, &rec_cmd, portMAX_DELAY)) {
			//要保存的语句
			if (rec_cmd.cmd == 9 || rec_cmd.cmd == 1 || rec_cmd.cmd == 2 || rec_cmd.cmd == 6) {
				tmp.cmd = rec_cmd.cmd;
				tmp.chanel = rec_cmd.chanel;
				tmp.data = rec_cmd.data_h << 8 | rec_cmd.data_l;
				//写入flash
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08020000 + count * sizeof(action_group_t), *((uint32_t *)&tmp));
				count++;
			} else if (rec_cmd.cmd == 0x0D){
			} else {
				break;
			}
			//请求USB接收数据
			USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
		}
	}
	//flash上锁
	HAL_FLASH_Lock();
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
		//接收USB数据
		if (xQueueReceive(rec_data_queue, &rec_cmd, portMAX_DELAY)) {
			//前缀0xFF
			if (rec_cmd.header == 0xFF) {
				//心跳包
				if (rec_cmd.cmd == 0 && rec_cmd.chanel == 0x12) {
					uint8_t send_buf[3] = {0xFF, 0xF0, 0x12};
					USBD_CUSTOM_HID_SendReport_FS(send_buf, 3);
				}
				//请求设置速度
				if (rec_cmd.cmd == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskResume(steering_updata_task_handle);
					steering_speed[rec_cmd.chanel] = (rec_cmd.data_h << 8 | rec_cmd.data_l) * 2;
				}
				//请求设置位置
				if (rec_cmd.cmd == 2) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskResume(steering_updata_task_handle);
					steering_position[rec_cmd.chanel] = rec_cmd.data_h << 8 | rec_cmd.data_l;
				}
				//动作组下载
				if (rec_cmd.cmd == 10 && rec_cmd.data_l == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskResume(steering_updata_task_handle);
					USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
					action_download();
				}
				//动作组擦除
				if (rec_cmd.cmd == 0xFA && rec_cmd.data_l == 1) {
					steering_action_task_exit = 1;
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskResume(steering_updata_task_handle);
					action_erase();
				}
				//动作组执行
				if (rec_cmd.cmd == 9) {
					if (rec_cmd.data_l < 16)
						steering_action_task_create(rec_cmd.data_l);
				}
				//暂停运行
				if (rec_cmd.cmd == 11 && rec_cmd.data_l == 1) {
					if (steering_action_task_handle)
						vTaskSuspend(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskSuspend(steering_updata_task_handle);
				}
				//恢复运行
				if (rec_cmd.cmd == 11 && rec_cmd.data_l == 0) {
					if (steering_action_task_handle)
						vTaskResume(steering_action_task_handle);
					if (steering_updata_task_handle)
						vTaskResume(steering_updata_task_handle);
				}
			}
			//请求USB接收数据
			USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
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
