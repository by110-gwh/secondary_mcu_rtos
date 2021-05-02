#ifndef _USBD_CUSTOM_HID_IF_H
#define _USBD_CUSTOM_HID_IF_H

#include "usbd_customhid.h"

extern USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS;
int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len);

#endif
