#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"

//USB����
USBD_HandleTypeDef hUsbDeviceFS;

/**********************************************************************************************************
*�� �� ��: USB_DEVICE_Init
*����˵��: USB�豸��ʼ��
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void USB_DEVICE_Init(void)
{
	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_CUSTOM_HID);
	USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &USBD_CustomHID_fops_FS);
	USBD_Start(&hUsbDeviceFS);
}
