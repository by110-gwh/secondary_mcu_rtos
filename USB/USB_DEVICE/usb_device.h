#ifndef _USB_DEVICE__H
#define _USB_DEVICE__H

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#include "usbd_def.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

void USB_DEVICE_Init(void);

#endif
