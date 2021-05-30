#ifndef __PARAMER_SAVE_H
#define __PARAMER_SAVE_H

#include <stdint.h>

typedef struct {
	float accel_x_offset;
	float accel_y_offset;
	float accel_z_offset;
	float accel_x_scale;
	float accel_y_scale;
	float accel_z_scale;
	short gyro_x_offset;
	short gyro_y_offset;
	short gyro_z_offset;
	uint8_t inited;
} paramer_save_t;

extern volatile paramer_save_t paramer_save_data;

void write_save_paramer(void);
void read_save_paramer(void);

#endif
