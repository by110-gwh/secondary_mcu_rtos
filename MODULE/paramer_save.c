#include "paramer_save.h"
#include <string.h>
#include "stm32f7xx_hal.h"
#include "w25q.h"

//���������ַ
#define PARAMETER_SAVE_ADDR 0x7F8000

//����Ĳ����ṹ��
volatile paramer_save_t paramer_save_data;

/**********************************************************************************************************
*�� �� ��: save_paramer_init
*����˵��: ��ʼ������
*��    ��: ��
*�� �� ֵ: ��
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
*�� �� ��: write_save_paramer
*����˵��: �������
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void write_save_paramer()
{
    w25q_erase(PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
    w25q_write((uint8_t *)&paramer_save_data, PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
}

/**********************************************************************************************************
*�� �� ��: read_save_paramer
*����˵��: ��ȡ����Ĳ���
*��    ��: ��
*�� �� ֵ: ��
**********************************************************************************************************/
void read_save_paramer()
{
    w25q_read((uint8_t *)&paramer_save_data, PARAMETER_SAVE_ADDR, sizeof(paramer_save_t));
	if (paramer_save_data.inited != 0xAE) {
		save_paramer_init();
		write_save_paramer();
	}
}
