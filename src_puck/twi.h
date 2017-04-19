#ifndef GPA_TWI_H
#define GPA_TWI_H
#include <avr/io.h>

void twi_init();

uint8_t twi_start();

uint8_t twi_send_addr(uint8_t addr);

uint8_t twi_send_data(uint8_t data);

void twi_stop();

uint8_t twi_read_nack();

void twi_write_register(uint8_t addr, uint8_t reg, uint8_t dat);

uint8_t twi_read_register(uint8_t addr, uint8_t reg);

void twi_read_burst(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t n);

#endif

