#ifndef GPA_MODBUS_H_
#define GPA_MODBUS_H_

#include <stdlib.h>
#include <inttypes.h>
#include <util/delay.h>

/*Modbus function codes*/
#define MODBUS_READ_INPUTS 2
#define MODBUS_READ_COILS 1
#define MODBUS_WRITE_SINGLE_COIL 5
#define MODBUS_WRITE_MULTIPLE_COILS 15
#define MODBUS_READ_INPUT_REGISTERS 4
#define MODBUS_READ_HOLDING_REGISTERS 3
#define MODBUS_WRITE_HOLDING_REGISTER 6
#define MODBUS_WRITE_HOLDING_REGISTERS 16
#define MODBUS_READ_EXCEPTION_STATUS 7

typedef enum
{
	MODBUS_NONE,
	MODBUS_INPUT,
	MODBUS_COIL,
	MODBUS_INPUT_REGISTER,
	MODBUS_HOLDING_REGISTER
} modbus_type;

/*User functions to be implemented*/

#define MODBUS_MARK_TIME_US 2000 //Time of 3.5 characters @ 19200baud = 1/19200*11*3.5 = ~2ms 

/*------IMPLEMENT THESE!!!------*/
extern void modbus_tx(uint8_t byte);

extern uint16_t modbus_get_register(modbus_type t, uint16_t address);

extern void modbus_set_register(modbus_type t, uint16_t address, uint16_t value);

extern uint8_t modbus_get_coil(modbus_type t, uint16_t address);

extern void modbus_set_coil(modbus_type t, uint16_t address, uint8_t value);

extern void modbus_on_broadcast(uint8_t* frame);

void modbus_wait();

uint16_t modbus_crc(uint8_t byte, uint16_t crc);

void modbus_put(uint8_t byte);

void modbus_putregister(uint16_t reg);

void modbus_putarray(uint8_t* array, uint8_t len);
/*Resets the digested crc*/
void modbus_reset();

void modbus_start(uint8_t id, uint8_t function);

void modbus_stop();

void modbus_throw(uint8_t id, uint8_t function, uint8_t exc_code);

/*Checks length and if crc is correct*/
uint16_t modbus_is_frame_valid(uint8_t* frame, uint8_t len);

/*Returns recipient id of the frame*/
uint8_t modbus_get_id(uint8_t* frame);

/*Returns function code of the frame*/
uint8_t modbus_get_function(uint8_t* frame);

/*Returns a pointer to the data field of the frame*/
uint8_t* modbus_get_data(uint8_t* frame);

void modbus_execute(uint8_t* frame);

#endif










