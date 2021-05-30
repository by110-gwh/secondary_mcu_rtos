#ifndef __IMU_H
#define __IMU_H

#include <stdint.h>
#include "vector3.h"
#include "mpu6050.h"
//#include "ist8310.h"

//采样频率 200Hz
#define Sampling_Freq 200

typedef struct
{
	float x_sumplain ;
	float x_sumsq;
	float x_sumcube ;

	float y_sumplain ;
	float y_sumsq ;
	float y_sumcube ;

	float z_sumplain ;
	float z_sumsq ;
	float z_sumcube ;

	float xy_sum ;
	float xz_sum ;
	float yz_sum ;

	float x2y_sum ;
	float x2z_sum ;
	float y2x_sum ;
	float y2z_sum ;
	float z2x_sum ;
	float z2y_sum ;

	unsigned int size;
} Least_Squares_Intermediate_Variable;

//传感器滤波后数据
extern Vector3i_t accDataFilter;
extern Vector3i_t gyroDataFilter;
extern Vector3i_t acceCorrectFilter;
extern Vector3i_t gyroCorrectFilter;
extern float tempDataFilter;

void imu_init(void);
void get_imu_data(void);
void accel_calibration(uint8_t where);
void gyro_calibration(void);

#endif
