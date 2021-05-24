#include "paramer_save.h"
#include <string.h>
#include "stm32f7xx_hal.h"

//保存的参数结构体
volatile paramer_save_t paramer_save_data;

/**********************************************************************************************************
*函 数 名: save_paramer_init
*功能说明: 初始化参数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void save_paramer_init()
{
	paramer_save_data.inited = 0xAE;
	paramer_save_data.accel_x_offset = 0;
	paramer_save_data.accel_y_offset = 0;
	paramer_save_data.accel_z_offset = 0;
	paramer_save_data.accel_x_scale = 1;
	paramer_save_data.accel_y_scale = 1;
	paramer_save_data.accel_z_scale = 1;
	paramer_save_data.gyro_x_offset = 0;
	paramer_save_data.gyro_y_offset = 0;
	paramer_save_data.gyro_y_offset = 0;
}

/**********************************************************************************************************
*函 数 名: write_save_paramer
*功能说明: 保存参数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void write_save_paramer()
{
	uint32_t i;
	FLASH_EraseInitTypeDef pEraseInit;
	uint32_t PageError;
	
	//关总中断
	__disable_irq();
	HAL_FLASH_Unlock();
	
	pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	pEraseInit.Sector = 3;
	pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
	pEraseInit.NbSectors = 1;
	HAL_FLASHEx_Erase(&pEraseInit, &PageError);
	
	for (i = 0; i < (sizeof(paramer_save_t) + 3) / 4; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, PARAMETER_SAVE_ADDR + i * 4, ((uint32_t *)&paramer_save_data)[i]) != HAL_OK) {
            break;
        }
    }
	
	HAL_FLASH_Lock();
	//开总中断
	__enable_irq();
}

/**********************************************************************************************************
*函 数 名: read_save_paramer
*功能说明: 读取保存的参数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void read_save_paramer()
{
	uint32_t *ReadAddress = (uint32_t *)PARAMETER_SAVE_ADDR;
	memcpy((uint8_t *)&paramer_save_data, ReadAddress, sizeof(paramer_save_t));
	if (paramer_save_data.inited != 0xAE) {
		save_paramer_init();
		write_save_paramer();
	}
}
