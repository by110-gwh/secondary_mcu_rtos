#include <rl_usb.h>
#include "stm32f4xx.h"


#define __NO_USB_LIB_C
#include "usb_config.c"

#define USB_DEVICE     ((USB_OTG_DeviceTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_DEVICE_BASE))
#define USB_INEP(i)    ((USB_OTG_INEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_IN_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USB_OUTEP(i)   ((USB_OTG_OUTEndpointTypeDef *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_OUT_ENDPOINT_BASE + ((i) * USB_OTG_EP_REG_SIZE)))
#define USB_DFIFO(i)   *(__IO uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE + ((i) * USB_OTG_FIFO_SIZE))

PCD_HandleTypeDef hpcd_USB_OTG_FS;
static volatile uint8_t issetup;
static uint8_t usb_rec_buf[5][64];
static volatile uint8_t SetupStage_flag;
static volatile uint8_t DataInStage_flag;
static volatile uint8_t DataOutStage_flag;
static volatile uint8_t Reset_flag;
static volatile uint8_t SOF_flag;
static volatile uint8_t Suspend_flag;
static volatile uint8_t Resume_flag;
static uint8_t Epnum_I;
static uint8_t Epnum_O;

void USBD_IntrEna(void)
{
	HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

void USBD_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
	hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
	hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
	hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
	HAL_PCD_Init(&hpcd_USB_OTG_FS);
	HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x200);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x80);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x80);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 3, 0x80);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 4, 0x80);
	HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 5, 0x80);
    USBD_IntrEna();
}

void USBD_Connect(BOOL con)
{
	if (con)
		HAL_PCD_Start(&hpcd_USB_OTG_FS);
	else
		HAL_PCD_Stop(&hpcd_USB_OTG_FS);
}

void USBD_Reset(void)
{
}

void USBD_Suspend(void)
{
}

void USBD_Resume(void)
{
}

void USBD_WakeUp(void)
{
}

void USBD_WakeUpCfg(BOOL cfg)
{
}

void USBD_SetAddress(U32 adr, U32 setup)
{
	if (!setup) {
        return;
    }
	HAL_PCD_SetAddress(&hpcd_USB_OTG_FS, adr);
}

void USBD_Configure(BOOL cfg)
{
}

void USBD_ConfigEP(USB_ENDPOINT_DESCRIPTOR *pEPD)
{
	uint8_t ep_addr;
	uint16_t ep_mps;
	uint8_t ep_type;
    ep_addr = pEPD->bEndpointAddress;
    ep_mps = pEPD->wMaxPacketSize;
	switch (pEPD->bmAttributes & USB_ENDPOINT_TYPE_MASK) {
        case USB_ENDPOINT_TYPE_CONTROL:
            ep_type = EP_TYPE_CTRL;
            break;

        case USB_ENDPOINT_TYPE_ISOCHRONOUS:
            ep_type = EP_TYPE_ISOC;
            break;

        case USB_ENDPOINT_TYPE_BULK:
            ep_type = EP_TYPE_BULK;
            break;

        case USB_ENDPOINT_TYPE_INTERRUPT:
            ep_type = EP_TYPE_INTR;
            break;
    }
	HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, ep_addr, ep_mps, ep_type);
	HAL_PCD_EP_Receive(&hpcd_USB_OTG_FS,ep_addr, usb_rec_buf[ep_addr&0x0F], 64);
}

void USBD_DirCtrlEP(U32 dir)
{
}

void USBD_EnableEP(U32 EPNum)
{
}

void USBD_DisableEP(U32 EPNum)
{
}

void USBD_ResetEP(U32 EPNum)
{
}

void USBD_SetStallEP(U32 EPNum)
{
	HAL_PCD_EP_SetStall(&hpcd_USB_OTG_FS, EPNum);
}

void USBD_ClrStallEP(U32 EPNum)
{
	HAL_PCD_EP_ClrStall(&hpcd_USB_OTG_FS, EPNum);
}

void USBD_ClearEPBuf(U32 EPNum)
{
	HAL_PCD_EP_Flush(&hpcd_USB_OTG_FS, EPNum);
}

U32 USBD_ReadEP(U32 EPNum, U8 *pData, U32 bufsz)
{
	EPNum &= 0x0F;
	uint32_t i;
	if (bufsz > hpcd_USB_OTG_FS.OUT_ep[EPNum].xfer_count)
		bufsz = hpcd_USB_OTG_FS.OUT_ep[EPNum].xfer_count;
	if (issetup) {
		for (i = 0; i < bufsz; i++)
			pData[i] = ((uint8_t *)hpcd_USB_OTG_FS.Setup)[i];
	} else {
		for (i = 0; i < bufsz; i++)
			pData[i] = ((uint8_t *)usb_rec_buf[EPNum])[i];
	}
	HAL_PCD_EP_Receive(&hpcd_USB_OTG_FS, EPNum, usb_rec_buf[EPNum], 64);
	return bufsz;
}

U32 USBD_WriteEP(U32 EPNum, U8 *pData, U32 cnt)
{
	HAL_PCD_EP_Flush(&hpcd_USB_OTG_FS, EPNum);
	HAL_PCD_EP_Transmit(&hpcd_USB_OTG_FS, EPNum, pData, cnt);
	return cnt;
}

U32 USBD_GetFrame(void)
{
	return 0;
}


#ifdef __RTX
U32 LastError;                          /* Last Error                         */

U32 USBD_GetError(void)
{
    return (LastError);
}
#endif

void USBD_Handler(void)
{
	if (DataInStage_flag) {
		DataInStage_flag = 0;
		if (USBD_P_EP[Epnum_I&0x0F]) {
			USBD_P_EP[Epnum_I&0x0F](USBD_EVT_IN);
		}
	}
	
	if (DataOutStage_flag) {
		issetup = 0;
		DataOutStage_flag = 0;
		if (HAL_PCD_EP_GetRxCount(&hpcd_USB_OTG_FS, Epnum_O&0x0F) != 0) {
			if (USBD_P_EP[Epnum_O&0x0F]) {
				USBD_P_EP[Epnum_O&0x0F](USBD_EVT_OUT);
			}
		}
	}
	if (SetupStage_flag) {
		issetup = 1;
		SetupStage_flag = 0;
			if (USBD_P_EP[0]) {
				USBD_P_EP[0](USBD_EVT_SETUP);
			}
	}
	
	if (Reset_flag) {
		Reset_flag = 0;
		HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, 0x00U, 64U, 0x00U);
		HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, 0x80U, 64U, 0x00U);
		usbd_reset_core();
		if (USBD_P_Reset_Event) {
			USBD_P_Reset_Event();
		}
	}
	
	if (SOF_flag) {
		SOF_flag = 0;
		if (USBD_P_SOF_Event) {
			USBD_P_SOF_Event();
		}
	}
	
	if (Suspend_flag) {
		Suspend_flag = 0;
		if (USBD_P_Suspend_Event) {
			USBD_P_Suspend_Event();
		}
	}
	if (Resume_flag) {
		Resume_flag = 0;
		if (USBD_P_Resume_Event) {
			USBD_P_Resume_Event();
		}
	}
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
	SetupStage_flag = 1;
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
	DataInStage_flag = 1;
	Epnum_I = epnum;
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{	
	DataOutStage_flag = 1;
	Epnum_O = epnum;
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
	Reset_flag = 1;
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
	SOF_flag = 1;
}


void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
	Suspend_flag = 1;
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
	Resume_flag = 1;
}

void OTG_FS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
	USBD_SignalHandler();
}

void USBD_SignalHandler() {}
#include <string.h>
const char *info_get_unique_id_string_descriptor()
{
	static char string_unique_id[] = "2021-2-12";
	static char usb_desc_unique_id[2 + sizeof(string_unique_id) * 2];
	uint8_t i = 0, idx = 0, len = 0;
    len = strlen((const char *)string_unique_id);
    // bLength
    usb_desc_unique_id[idx++] = len * 2 + 2;
    // bDescriptorType
    usb_desc_unique_id[idx++] = 3;

    // bString
    for (i = 0; i < len; i++) {
        usb_desc_unique_id[idx++] = string_unique_id[i];
        usb_desc_unique_id[idx++] = 0;
    }
    return usb_desc_unique_id;
}
