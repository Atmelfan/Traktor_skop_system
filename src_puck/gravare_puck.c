#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <math.h>
#include "mpu6050.h"
#include "modbus.h"

#define BAUD 19200                                   // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)            // set baud rate value for UBRR

#define TYPE_PASSW 204

/*EEPROM nonvolatile configuration values*/
uint8_t EEMEM eeprom_modbus_id = 247;
uint16_t EEMEM eeprom_gyro_conf = 0x0000;//
uint16_t EEMEM eeprom_accl_conf = 0x0000;//
uint16_t EEMEM eeprom_angl_conf = 0x0FFF;//

uint8_t modbus_id;
uint16_t gyro_conf;
uint16_t accl_conf;
uint16_t angl_conf;



volatile uint8_t rx_flag=0;


/*========================================RS485========================================*/
void uart_init (void)
{
    UBRR0H = (BAUDRATE>>8);
    UBRR0L = BAUDRATE;
    UCSR0B|= (1<<TXEN0)|(1<<RXEN0);             
    UCSR0C|= (1<<UCSZ00)|(1<<UCSZ01);			//8N1
    UCSR0B |= (1 << RXCIE0)|(1 << TXCIE0);

}

void uart_transmit (uint8_t data)
{
    while (!( UCSR0A & (1<<UDRE0)));                
    PORTD |= (1 << 4);
    UDR0 = data;                                   
}

void uart_put (char c)
{
    uart_transmit((uint8_t)c);
}

void modbus_tx(uint8_t byte){
	uart_transmit(byte);
}

void uart_puts (char* s)
{
    while(*s != '\0'){
    	uart_put(*s++);
    }
}

volatile uint8_t tx_index = 0, modbus_len = 0, rx_buffer[256];

ISR(USART_RX_vect)
{
	TCCR0B |= (1 << CS02);//Start (if not already running) and reset timeout timer
	TCNT0 = 0x00;
	if (tx_index < sizeof(rx_buffer)/sizeof(uint8_t))
		rx_buffer[tx_index++] = UDR0; 
}

ISR(USART_TX_vect)
{
	PORTD &= ~(1 << 4);
}

ISR(TIMER0_COMPA_vect)//The 3.5ch end condition has passed
{
	if (tx_index > 0)
	{
		
		if(modbus_get_id((uint8_t*)&rx_buffer) == modbus_id)
		{
			if (modbus_is_frame_valid((uint8_t*)&rx_buffer, tx_index))
			{
				modbus_execute((uint8_t*)&rx_buffer);
				PORTD ^= (1 << 5);
			}
		}
		
	}
	tx_index = 0;
	TCCR0B = 0x00;//Stop timer
}



void blink_error(){
	PORTD |= (1 << 5);
	_delay_ms(100);
	PORTD &= ~(1 << 5);
	_delay_ms(400);
}

typedef enum
{
	AXIS_X = 1,
	AXIS_Y = 2,
	AXIS_Z = 3
} mpu_6050_axis;

int16_t gyro_ofs[3] = {0};
int16_t raw_gyro[3] = {0};
int16_t raw_accel[3] = {0};
float angle_xyz[3] = {0};
float angle_last[3] = {0};


char buffer[33];

void setup(){
	DDRD |= (1 << 5)|(1 << 4)|(1 << 1);
	//PORTD &= ~(1 << 4);
	uart_init();
	twi_init();
	mpu6050_init();

	/*Configure modbus mark timeout timer*/
	OCR0A = 125;
	TIMSK0 |= (1 << OCIE0A);
	/**/
	TCNT1 = 0;
	TIMSK1 |= (1 << TOIE1);
	TCCR1B = (1 << CS11)|(1 << CS10);//DIV64

	modbus_id = eeprom_read_byte(&eeprom_modbus_id);
	if (modbus_id > 247)//The modbus standard & minimalmodbus doesn't like ids above 247
	{
		modbus_id = 247;
	}

	accl_conf = eeprom_read_word(&eeprom_accl_conf);
	gyro_conf = eeprom_read_word(&eeprom_gyro_conf);
	angl_conf = eeprom_read_word(&eeprom_angl_conf);
	sei();
	
}




/*========================================MODBUS========================================*/


uint16_t modbus_get_register(modbus_type t, uint16_t address){
	if (t == MODBUS_INPUT_REGISTER)
	{
		switch(address){
			case 0:
			case 1:
			case 2:
				return raw_gyro[address - 0];
			case 3:
			case 4:
			case 5:
				return raw_accel[address - 3];
			case 6:
			case 7:
			case 8:
				return (int16_t)(angle_xyz[address - 6]*100);
			case 9:
			case 10:
			case 11:
				return gyro_ofs[address-9];
			default:
				return 0x0000;
		}
	}else{
		switch(address){
			case 0:
				return (modbus_id << 8)|(TYPE_PASSW);
			case 1:
				return gyro_conf;
			case 2:
				return accl_conf;
			case 3:
				return angl_conf;
			default:
				return 0x0000;
		}
	}
	
}

void modbus_set_register(modbus_type t, uint16_t address, uint16_t value){
	switch(address){
		case 0:
			if ((value & 0x00FF) == TYPE_PASSW)//Verify so that the id doesn't get overwritten by accident.
			{
				uint8_t newid = (value >> 8) & 0xFF;
				if(newid != modbus_id && newid <= 247){
					eeprom_write_byte(&eeprom_modbus_id, newid);
					modbus_id = newid;
				}
			}
			break;
		case 1:
			gyro_conf = value;
			eeprom_write_word(&eeprom_gyro_conf, value);
			break;
		case 2:
			accl_conf = value;
			eeprom_write_word(&eeprom_accl_conf, value);
			break;
		case 3:
			angl_conf = value;
			eeprom_write_word(&eeprom_angl_conf, value);
			break;
		default:
			break; //Nothing...
	}
}

uint8_t modbus_get_coil(modbus_type t, uint16_t address){
	if (t == MODBUS_INPUT)
	{
		if (address < 6)
		{
			return PINB & (1 << address);
		}
	}
	
	return 0x01;
}

void modbus_set_coil(modbus_type t, uint16_t address, uint8_t value){

}

void modbus_on_broadcast(uint8_t* frame){
	//Do nothing...
}

volatile uint8_t n_ovf = 0;

ISR(TIMER1_OVF_vect)
{
	n_ovf++;
} 

int main(void){
	setup();
	PORTD ^= (1 << 5);
	for (int i = 0; i < 3; ++i)
	{
		int32_t sum = 0;
		for (int j = 0; j < 256; ++j)
		{
			switch(i){
				case 0:
					sum += mpu6050_gyro_x();
				break;
				case 1:
					sum += mpu6050_gyro_y();
				break;
				case 2:
					sum += mpu6050_gyro_z();
				break;
			}
			_delay_us(500); 
		}
		gyro_ofs[i] = sum/256;
		PORTD ^= (1 << 5);
	}

	const float k = 0.90f;
	while(1){
		uint16_t tc = TCNT1;
		TCNT1 = 0;
		float dt = (((float)tc)/65536.0f)*0.262144f + n_ovf*0.262144f;
		n_ovf = 0;

		raw_accel[0] = mpu6050_accel_x();
		raw_accel[1] = mpu6050_accel_y();
		raw_accel[2] = mpu6050_accel_z();
		raw_gyro[0] = mpu6050_gyro_x();
		raw_gyro[1] = mpu6050_gyro_y();
		raw_gyro[2] = mpu6050_gyro_z();

		if ((angl_conf >> 0) & 0x000F)//Z
		{
			angle_xyz[2] = k*(angle_xyz[2] + ((raw_gyro[2] - gyro_ofs[2])/131.0f)*dt) + (1.0f - k)*atan2(raw_accel[0],raw_accel[1])*180.0f/M_PI;
		}
		if ((angl_conf >> 4) & 0x000F)//Y
		{
			angle_xyz[1] = k*(angle_xyz[1] + ((raw_gyro[1] - gyro_ofs[1])/131.0f)*dt) + (1.0f - k)*atan2(raw_accel[0],raw_accel[2])*180.0f/M_PI;		
		}
		if ((angl_conf >> 8) & 0x000F)//X
		{
			angle_xyz[0] = k*(angle_xyz[0] + ((raw_gyro[0] - gyro_ofs[0])/131.0f)*dt) + (1.0f - k)*atan2(raw_accel[2],raw_accel[1])*180.0f/M_PI;
		}
		
	}
	return 1;
}



