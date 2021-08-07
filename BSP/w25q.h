#ifndef _W25Q_H
#define _W25Q_H

#include <stdint.h>

uint8_t w25q_read_sr(uint8_t regno);
void w25q_write_enable(void);
void w25q_read(uint8_t* pBuffer,uint32_t addr, uint16_t cnt);
void w25q_write(uint8_t* pBuffer,uint32_t addr,uint16_t cnt);
void w25q_erase_sector(uint32_t addr);
void w25q_erase(uint32_t addr, uint32_t cnt);
void w25q_wait_busy(void);

#endif

