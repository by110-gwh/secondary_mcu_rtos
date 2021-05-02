#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define DEVICE_ID1 (UID_BASE)
#define DEVICE_ID2 (UID_BASE + 0x4)
#define DEVICE_ID3 (UID_BASE + 0x8)

//���к��ַ�������
#define USB_SIZ_STRING_SERIAL 0x1A

#define USBD_VID 0x1920
#define USBD_PID 0x0100
#define USBD_LANGID_STRING 0x0409
#define USBD_MANUFACTURER_STRING "Wit-Device"
#define USBD_PRODUCT_STRING "Wit-Motion "
#define USBD_CONFIGURATION_STRING "Custom HID Config"
#define USBD_INTERFACE_STRING "Custom HID Interface"


//USB�豸������
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
	0x12,
	USB_DESC_TYPE_DEVICE,
	0x00,
	0x02,
	0x00,
	0x00,
	0x00,
	USB_MAX_EP0_SIZE,
	LOBYTE(USBD_VID),
	HIBYTE(USBD_VID),
	LOBYTE(USBD_PID),
	HIBYTE(USBD_PID),
	0x00,
	0x02,
	USBD_IDX_MFC_STR,
	USBD_IDX_PRODUCT_STR,
	USBD_IDX_SERIAL_STR,
	USBD_MAX_NUM_CONFIGURATION
};

//ID����������
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
     USB_LEN_LANGID_STR_DESC,
     USB_DESC_TYPE_STRING,
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING)
};

//��ʱ�����ַ���
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

//��ʱ�������к��ַ���
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
	USB_SIZ_STRING_SERIAL,
	USB_DESC_TYPE_STRING,
};

/**********************************************************************************************************
*�� �� ��: IntToUnicode
*����˵��: ������ת��Ϊunicode������ָ���ַ�����
*��    ��: Ҫת�������� ������ַ��� ���ַ�������
*�� �� ֵ: ��
**********************************************************************************************************/
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
	uint8_t idx = 0;

	for (idx = 0; idx < len; idx++)
	{
		if (((value >> 28)) < 0xA)
		{
			pbuf[2 * idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2 * idx] = (value >> 28) + 'A' - 10;
		}

		value = value << 4;

		pbuf[2 * idx + 1] = 0;
	}
}

/**********************************************************************************************************
*�� �� ��: Get_SerialNum
*����˵��: ��ȡ���кŲ����浽���к��ַ�����
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
static void Get_SerialNum(void)
{
	uint32_t deviceserial0, deviceserial1, deviceserial2;

	deviceserial0 = *(uint32_t *) DEVICE_ID1;
	deviceserial1 = *(uint32_t *) DEVICE_ID2;
	deviceserial2 = *(uint32_t *) DEVICE_ID3;

	deviceserial0 += deviceserial2;

	if (deviceserial0 != 0)
	{
		IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
		IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
	}
}


/**********************************************************************************************************
*�� �� ��: USBD_FS_DeviceDescriptor
*����˵��: ��ȡUSB�豸������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = sizeof(USBD_FS_DeviceDesc);
	return USBD_FS_DeviceDesc;
}

/**********************************************************************************************************
*�� �� ��: USBD_FS_LangIDStrDescriptor
*����˵��: ��ȡUSB����������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = sizeof(USBD_LangIDDesc);
	return USBD_LangIDDesc;
}

/**********************************************************************************************************
*�� �� ��: USBD_FS_ProductStrDescriptor
*����˵��: ��ȡUSB��Ʒ���ַ���������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((uint8_t *)USBD_PRODUCT_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

/**********************************************************************************************************
*�� �� ��: USBD_FS_ManufacturerStrDescriptor
*����˵��: ��ȡUSB�����ַ���������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}

/**********************************************************************************************************
*�� �� ��: USBD_FS_SerialStrDescriptor
*����˵��: ��ȡUSB���к��ַ���������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	*length = USB_SIZ_STRING_SERIAL;
	Get_SerialNum();
	return (uint8_t *) USBD_StringSerial;
}

/**********************************************************************************************************
*�� �� ��: USBD_FS_ConfigStrDescriptor
*����˵��: ��ȡUSB�����ַ���������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}


/**********************************************************************************************************
*�� �� ��: USBD_FS_InterfaceStrDescriptor
*����˵��: ��ȡUSB�ӿ��ַ���������
*��    ��: USB�ٶ� ��������������ָ��
*�� �� ֵ: �ַ���ָ��
**********************************************************************************************************/
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
	USBD_GetString((uint8_t *)USBD_INTERFACE_STRING, USBD_StrDesc, length);
	return USBD_StrDesc;
}



//USB�豸��������ȡ��������
USBD_DescriptorsTypeDef FS_Desc = {
	USBD_FS_DeviceDescriptor,
	USBD_FS_LangIDStrDescriptor,
	USBD_FS_ManufacturerStrDescriptor,
	USBD_FS_ProductStrDescriptor,
	USBD_FS_SerialStrDescriptor,
	USBD_FS_ConfigStrDescriptor,
	USBD_FS_InterfaceStrDescriptor
};
