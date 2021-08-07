#include "usbd_custom_hid_if.h"
#include "usb_device.h"
#include "steering_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//HID报告描述符
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
{
	0x05,
	0x8c,
	0x09,
	0x01,
	0xa1,
	0x01,
	0x09,
	0x03,
	0x15,
	0x00,
	0x26,
	0x00,
	0xff,
	0x75,
	0x08,
	0x95,
	0x40,
	0x81,
	0x02,
	0x09,
	0x04,
	0x15,
	0x00,
	0x26,
	0x00,
	0xff,
	0x75,
	0x08,
	0x95,
	0x40,
	0x91,
	0x02,
	0xC0
};

/**********************************************************************************************************
*函 数 名: CUSTOM_HID_Init_FS
*功能说明: HID初始化回调函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
static int8_t CUSTOM_HID_Init_FS(void)
{
	return (USBD_OK);
}

/**********************************************************************************************************
*函 数 名: CUSTOM_HID_DeInit_FS
*功能说明: HID复位回调函数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
static int8_t CUSTOM_HID_DeInit_FS(void)
{
	return (USBD_OK);
}

/**********************************************************************************************************
*函 数 名: USBD_CUSTOM_HID_SendReport_FS
*功能说明: HID发送数据
*形    参: 数据指针 数据大小
*返 回 值: 状态
**********************************************************************************************************/
int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
	return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, 64);
}

/**********************************************************************************************************
*函 数 名: CUSTOM_HID_OutEvent_FS
*功能说明: HID接收回调函数
*形    参: 事件 状态
*返 回 值: 状态
**********************************************************************************************************/
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{	
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(rec_data_queue, ((uint8_t *)hUsbDeviceFS.pClassData), &xHigherPriorityTaskWoken);
    USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);
	if(xHigherPriorityTaskWoken) {
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	
	return (USBD_OK);
}

//HID操作函数集合
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS = {
	CUSTOM_HID_ReportDesc_FS,
	CUSTOM_HID_Init_FS,
	CUSTOM_HID_DeInit_FS,
	CUSTOM_HID_OutEvent_FS
};
