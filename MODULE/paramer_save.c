#include "paramer_save.h"
#include <string.h>
#include "stm32f7xx_hal.h"
#include "w25q.h"

//参数保存地址
#define PARAMETER_SAVE_ADDR 0x7F8000

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
	paramer_save_data.gyro_z_offset = 0;
}

/**********************************************************************************************************
*函 数 名: write_save_paramer
*功能说明: 保存参数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void write_save_paramer()
{
    w25q_erase(PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
    w25q_write((uint8_t *)&paramer_save_data, PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
}

/**********************************************************************************************************
*函 数 名: read_save_paramer
*功能说明: 读取保存的参数
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void read_save_paramer()
{
    w25q_read((uint8_t *)&paramer_save_data, PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
	if (paramer_save_data.inited != 0xAE) {
		save_paramer_init();
		write_save_paramer();
	}
}
