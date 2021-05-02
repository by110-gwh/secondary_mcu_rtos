#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_customhid.h"
#include "usbd_custom_hid_if.h"

//USB类句柄
USBD_HandleTypeDef hUsbDeviceFS;

/**********************************************************************************************************
*函 数 名: USB_DEVICE_Init
*功能说明: USB设备初始化
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void USB_DEVICE_Init(void)
{
	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_CUSTOM_HID);
	USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &USBD_CustomHID_fops_FS);
	USBD_Start(&hUsbDeviceFS);
}
