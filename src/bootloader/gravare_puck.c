#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "modbus.h"

#define BAUD 19200                                   // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)            // set baud rate value for UBRR

#define TYPE_PASSW 204


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
	if (tx_index < 256)
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
		
		if(modbus_get_id((uint8_t*)&rx_buffer) == 1)
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


void setup(){
	DDRD |= (1 << 5)|(1 << 4)|(1 << 1);
	//PORTD &= ~(1 << 4);
	uart_init();
	/*Configure modbus mark timeout timer*/
	OCR0A = 125;
	TIMSK0 |= (1 << OCIE0A);
	sei();
	
}




/*========================================MODBUS========================================*/


uint16_t modbus_get_register(modbus_type t, uint16_t address){
	if (t == MODBUS_INPUT_REGISTER)
	{
		switch(address){

			default:
				return 0x0000;
		}
	}else{
		switch(address){

			default:
				return 0x0000;
		}
	}
	
}

void modbus_set_register(modbus_type t, uint16_t address, uint16_t value){
	switch(address){

		default:
			break; //Nothing...
	}
}

uint8_t modbus_get_coil(modbus_type t, uint16_t address){
	return 0x01;
}

void modbus_set_coil(modbus_type t, uint16_t address, uint8_t value){

}

void modbus_on_broadcast(uint8_t* frame){
	//Do nothing...
}

int main(void){
	setup();
	while(1){

		
	}
	return 1;
}



