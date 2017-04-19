#include "modbus.h"
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

void boot_program_page (uint16_t page, uint8_t *buf)
{
    uint16_t i;
    uint8_t sreg;
    // Disable interrupts.
    sreg = SREG;
    cli();
    eeprom_busy_wait ();
    boot_page_erase (page);
    boot_spm_busy_wait ();      // Wait until the memory is erased.
    for (i=0; i<SPM_PAGESIZE; i+=2)
    {
        // Set up little-endian word.
        uint16_t w = *buf++;
        w += (*buf++) << 8;
    
        boot_page_fill (page + i, w);
    }
    boot_page_write (page);     // Store buffer in flash page.
    boot_spm_busy_wait();       // Wait until the memory is written.
    // Reenable RWW-section again. We need this if we want to jump back
    // to the application after bootloading.
    boot_rww_enable ();
    // Re-enable interrupts (if they were ever enabled).
    SREG = sreg;
}

uint16_t tx_crc = 0xFFFF;

void modbus_wait(){
	_delay_us(MODBUS_MARK_TIME_US);
}

uint16_t modbus_crc(uint8_t byte, uint16_t crc){
	crc ^= byte;          // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      	if ((crc & 0x0001) != 0) {      // If the LSB is set
        	crc >>= 1;                    // Shift right and XOR 0xA001
        	crc ^= 0xA001;
      	}
      	else {                            // Else LSB is not set
        	crc >>= 1;                    // Just shift right
    	}
    }
	return crc;
}

void modbus_put(uint8_t byte){
	tx_crc = modbus_crc(byte, tx_crc);
	modbus_tx(byte);
}

void modbus_putregister(uint16_t reg){
	modbus_put((reg >> 8) & 0xFF);//Modbus says MSB!
	modbus_put((reg >> 0) & 0xFF);
	
	
}


void modbus_putcrc(uint16_t crc){
	modbus_put((crc >> 0) & 0xFF);
	modbus_put((crc >> 8) & 0xFF);//Modbus says MSB!
	
}

void modbus_putarray(uint8_t* array, uint8_t len){
	for (int i = 0; i < len; ++i)
	{
		modbus_put(array[i]);
	}
}

/*Resets the digested crc*/
void modbus_reset(){
	tx_crc = 0xFFFF;
}

void modbus_start(uint8_t id, uint8_t function){
	modbus_reset();
	modbus_wait();
	modbus_put(id);
	modbus_put(function);
}

void modbus_stop(){
	modbus_putcrc(tx_crc);
	modbus_wait();
}

void modbus_throw(uint8_t id, uint8_t function, uint8_t exc_code){
	modbus_start(id, function + 128);
	modbus_put(exc_code);
	modbus_stop();
}

/*Checks length and if crc is correct*/
uint16_t modbus_is_frame_valid(uint8_t* frame, uint8_t len){
	if (len < 4)
	{
		return 0;
	}
	uint16_t tcrc = 0xFFFF;
	uint8_t i = 0;
	for (; i < len - 2; ++i)
	{
		tcrc = modbus_crc(frame[i], tcrc);//Calculate crc of recieved frame
	}
	return (((tcrc >> 0) & 0xFF) == frame[len-2]) && (((tcrc >> 8) & 0xFF) == frame[len-1]);//Compare the calculated and recieved crc...
}

/*Returns recipient id of the frame*/
uint8_t modbus_get_id(uint8_t* frame){
	return frame[0];
}

/*Returns function code of the frame*/
uint8_t modbus_get_function(uint8_t* frame){
	return frame[1];
}

/*Returns a pointer to the data field of the frame*/
uint8_t* modbus_get_data(uint8_t* frame){
	return (frame + 2);
}

uint16_t modbus_uint16(uint8_t* frame, uint8_t offset){
	return (frame[offset+2] << 8)|(frame[offset+3] << 0);
}


void modbus_execute(uint8_t* frame){
	uint8_t i = modbus_get_id(frame);
	uint8_t f = modbus_get_function(frame);

	if (i == 0)//ID 0 is reserved for broadcasted messages
	{
		modbus_on_broadcast(frame);//I don't know how to deal with that shit. Better leave it to the application...
		return;
	}
	modbus_type t = MODBUS_NONE;
	switch(f){
		case MODBUS_READ_INPUT_REGISTERS:
			t = MODBUS_INPUT_REGISTER;
		case MODBUS_READ_HOLDING_REGISTERS: {
			if (t == MODBUS_NONE)
			{
				t = MODBUS_HOLDING_REGISTER;
			}
			uint16_t a = modbus_uint16(frame, 0);
			uint16_t n = modbus_uint16(frame, 2);
			if (n > 127)
			{
				modbus_throw(i,f,3);
			}else{
				modbus_start(i, f);
				modbus_put(n*2);
				for (int i = 0; i < n; ++i)
				{
					modbus_putregister(modbus_get_register(t, a+i));
				}
				modbus_stop();
			}
		}	
		break;

		case MODBUS_WRITE_HOLDING_REGISTER: {
			t = MODBUS_HOLDING_REGISTER;
			uint16_t a = modbus_uint16(frame, 0);
			uint16_t n = modbus_uint16(frame, 2);
			modbus_set_register(t, a, n);

			modbus_start(i,f);
			modbus_putregister(a);
			modbus_putregister(modbus_get_register(t, a));
			modbus_stop();
		}
		break;

		case MODBUS_WRITE_HOLDING_REGISTERS: {
			t = MODBUS_HOLDING_REGISTER;
			uint16_t a = modbus_uint16(frame, 0);
			uint16_t n = modbus_uint16(frame, 2);
			uint8_t nb = frame[4 + 2];
			if (n*2 != nb)
			{
				modbus_throw(i,f,3);
			}else{
				for (int i = 0; i < n; ++i)
				{
					modbus_set_register(t, a+i, modbus_uint16(frame, 5 + 2*i));
				}
				modbus_start(i,f);
				modbus_putregister(a);
				modbus_putregister(n);
				modbus_stop();
			}
		}
		break;

		case 0xCC: {
			uint8_t cmd = frame[2];
			
			modbus_start(i,f);
			if (cmd==0x00)
			{
					modbus_putregister(SPM_PAGESIZE);
					modbus_putregister(boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS));
					modbus_putregister(boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS));
					modbus_putregister(boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS));
					modbus_putregister(boot_lock_fuse_bits_get(GET_LOCK_BITS));
					
					

			}else if (cmd==0x01)
			{
				uint16_t a = modbus_uint16(frame, 1);
				boot_program_page(a, frame+5);
				modbus_putregister(a);
				modbus_putregister(SPM_PAGESIZE);
			}else if (cmd==0x02)
			{
				
			}
			modbus_stop();
			
		}
		break;

		default:
			modbus_throw(i, f, 1);
		break;
	}
}