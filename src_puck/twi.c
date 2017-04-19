#include "twi.h"
#include <stdlib.h>

#define TW_START 0xA4 //send start condition (TWINT,TWSTA,TWEN)
#define TW_READY (TWCR & 0x80) // ready when TWINT returns to logic 1.
#define TW_STATUS (TWSR & 0xF8) // returns value of status register
#define TW_SEND 0x84
#define TW_STOP 0x94
#define TW_NACK 0x84
#define TW_ACK 0xC4
#define TW_READ 0x01

void twi_init(){
	TWSR = 0;
	TWBR = ((F_CPU/F_SCL)-16)/2;
}

uint8_t twi_start(){
	TWCR = TW_START;
	while(!TW_READY);
	return (TW_STATUS & 0x08);
}

uint8_t twi_send_addr(uint8_t addr){
	TWDR = addr;
	TWCR = TW_SEND;
	while(!TW_READY);
	return (TW_STATUS & 0x18);
}

uint8_t twi_send_data(uint8_t data){
	TWDR = data;
	TWCR = TW_SEND;
	while(!TW_READY);
	return (TW_STATUS & 0x28);
}

void twi_stop(){
	TWCR = TW_STOP;
}

uint8_t twi_read_ack(){
	TWCR = TW_ACK;
	while(!TW_READY);
	return TWDR;
}

uint8_t twi_read_nack(){
	TWCR = TW_NACK;
	while(!TW_READY);
	return TWDR;
}

void twi_write_register(uint8_t addr, uint8_t reg, uint8_t dat){
	twi_start();
	twi_send_addr(addr);
	twi_send_data(reg);
	twi_send_data(dat);
	twi_stop();
}

uint8_t twi_read_register(uint8_t addr, uint8_t reg){
	uint8_t dat = 0;
	twi_start();
	twi_send_addr(addr);
	twi_send_data(reg);

	twi_start();
	twi_send_addr(addr + TW_READ);
	dat = twi_read_nack();
	twi_stop();
	return dat;
}

void twi_read_burst(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t n){
	twi_start();
	twi_send_addr(addr);
	twi_send_data(reg);

	twi_start();
	twi_send_addr(addr + TW_READ);
	for (int i = 0; i < n - 1; ++i)
	{
		*buffer = twi_read_ack();//Read N-1 bytes while acknowleging
		buffer++;
	}
	*buffer = twi_read_nack();//Read the Nth byte and don't acknowledge to abort burst.
	twi_stop();
}