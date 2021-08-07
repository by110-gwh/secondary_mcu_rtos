#include "usbd_custom_hid_if.h"
#include "usb_device.h"
#include "steering_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//HID����������
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
*�� �� ��: CUSTOM_HID_Init_FS
*����˵��: HID��ʼ���ص�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static int8_t CUSTOM_HID_Init_FS(void)
{
	return (USBD_OK);
}

/**********************************************************************************************************
*�� �� ��: CUSTOM_HID_DeInit_FS
*����˵��: HID��λ�ص�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static int8_t CUSTOM_HID_DeInit_FS(void)
{
	return (USBD_OK);
}

/**********************************************************************************************************
*�� �� ��: USBD_CUSTOM_HID_SendReport_FS
*����˵��: HID��������
*��    ��: ����ָ�� ���ݴ�С
*�� �� ֵ: ״̬
**********************************************************************************************************/
int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
	return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, 64);
}

/**********************************************************************************************************
*�� �� ��: CUSTOM_HID_OutEvent_FS
*����˵��: HID���ջص�����
*��    ��: �¼� ״̬
*�� �� ֵ: ״̬
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

//HID������������
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS = {
	CUSTOM_HID_ReportDesc_FS,
	CUSTOM_HID_Init_FS,
	CUSTOM_HID_DeInit_FS,
	CUSTOM_HID_OutEvent_FS
};
