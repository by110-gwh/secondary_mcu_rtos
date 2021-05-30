#ifndef _QSPI_H
#define _QSPI_H

#include <stdint.h>

void qspi_init(void);
void qspi_deinit(void);
void spi_send_cmd(uint8_t cmd, uint32_t addr, uint8_t has_addr, uint8_t dummt_cycles, uint8_t count);
uint8_t qspi_receive(uint8_t* buf,uint32_t datalen);
uint8_t qspi_transmit(uint8_t* buf,uint32_t datalen);

#endif

