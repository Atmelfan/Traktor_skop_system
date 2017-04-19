#include "mpu6050.h"



void mpu6050_init(){
	while(twi_read_register(MPU6050_ADDRESS, 0x75) != 0x68);
	twi_write_register(MPU6050_ADDRESS, MPU6050_SAMPLRT_DIV, 0x07);
	twi_write_register(MPU6050_ADDRESS, MPU6050_CONFIG, 0x06);
	twi_write_register(MPU6050_ADDRESS, MPU6050_GYRO_CONFIG, 0x00);
	twi_write_register(MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG, 0x00);
	twi_write_register(MPU6050_ADDRESS, MPU6050_ACCEL_CONFIG2, 0x06);

	twi_write_register(MPU6050_ADDRESS, 0x6B, 0x01);

}

int16_t mpu6050_accel_x(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_XOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_XOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_accel_y(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_YOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_YOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_accel_z(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_ZOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_ACCEL_ZOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_gyro_x(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_XOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_XOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_gyro_y(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_YOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_YOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_gyro_z(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_ZOUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_GYRO_ZOUT_L);
	return (h << 8)|l;
}

int16_t mpu6050_temp(){
	uint8_t h = twi_read_register(MPU6050_ADDRESS, MPU6050_TEMP_OUT_H);
	uint8_t l = twi_read_register(MPU6050_ADDRESS, MPU6050_TEMP_OUT_L);
	return (h << 8)|l;
}

void mpu6050_burst(mpu6050_burst_data* burst_data){
	uint8_t temp[14];
	twi_read_burst(MPU6050_ADDRESS, MPU6050_ACCEL_XOUT_H, (uint8_t*)&temp, 12);
	for (int i = 0; i < 7; ++i)
	{
		int16_t t = (temp[2*i+0] << 8)|(temp[2*i+1]);//MPU6050 stores values high byte first(big endian) which is stupid...
		*(uint16_t*)burst_data = t;
		burst_data += 2;
	}
}




