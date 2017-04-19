#ifndef GPA_MPU6050_H_
#define GPA_MPU6050_H_
#include "twi.h"


#define MPU6050_ADDRESS 0xD0

#define MPU6050_FS_SEL_0 (0x00 << 3)
#define MPU6050_FS_SEL_1 (0x01 << 3)
#define MPU6050_FS_SEL_2 (0x02 << 3)
#define MPU6050_FS_SEL_3 (0x03 << 3)

#define MPU6050_SAMPLRT_DIV 0x19
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_ACCEL_CONFIG2 0x1D


#define MPU6050_ACCEL_XOUT_H 0x3B
#define MPU6050_ACCEL_XOUT_L 0x3C
#define MPU6050_ACCEL_YOUT_H 0x3D
#define MPU6050_ACCEL_YOUT_L 0x3E
#define MPU6050_ACCEL_ZOUT_H 0x3F
#define MPU6050_ACCEL_ZOUT_L 0x40

#define MPU6050_TEMP_OUT_H 0x41
#define MPU6050_TEMP_OUT_L 0x42

#define MPU6050_GYRO_XOUT_H 0x43
#define MPU6050_GYRO_XOUT_L 0x44
#define MPU6050_GYRO_YOUT_H 0x45
#define MPU6050_GYRO_YOUT_L 0x46
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_GYRO_ZOUT_L 0x48

typedef enum
{
	MPU6050_GYRO_SCALE_250DPS  = 0,
	MPU6050_GYRO_SCALE_500DPS  = 1,
	MPU6050_GYRO_SCALE_1000DPS = 2,
	MPU6050_GYRO_SCALE_2000DPS = 3
} mpu6050_gyro_scale_t;

typedef enum
{
	MPU6050_ACCEL_SCALE_2G  = 0,
	MPU6050_ACCEL_SCALE_4G  = 1,
	MPU6050_ACCEL_SCALE_8G  = 2,
	MPU6050_ACCEL_SCALE_16G = 3
} mpu6050_accel_scale_t;

typedef struct __attribute__((__packed__)) {
	int16_t accel_xyz[3];
	int16_t temp;
	int16_t gyro_xyz[3];
} mpu6050_burst_data;

#define MPU6050_DLPF_CFG_MASK(value) (0x07 & (value)) //Set DLPF_CFG by writing (MPU6050_DLPF_CFG_MASK & value)

void mpu6050_init();

void mpu6050_calibrate_still();

void mpu6050_set_accel_scale(mpu6050_accel_scale_t scale);

int16_t mpu6050_accel_x();

int16_t mpu6050_accel_y();

int16_t mpu6050_accel_z();

void mpu6050_set_gyro_scale(mpu6050_gyro_scale_t scale);

int16_t mpu6050_gyro_x();

int16_t mpu6050_gyro_y();

int16_t mpu6050_gyro_z();

int16_t mpu6050_temp();

void mpu6050_burst(mpu6050_burst_data* burst_data);


#endif



