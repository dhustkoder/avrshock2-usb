#include <avr/io.h>


#define BAUD (38400)


#include <util/setbaud.h>

#include "uart.h"


void uart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	UCSR0C = (0x01<<UCSZ01)|(0x01<<UCSZ00); /* 8-bit data */ 
	UCSR0B = (0x01<<RXEN0)|(0x01<<TXEN0);   /* Enable RX and TX */
}

void uart_send(const uint8_t* const data, const short size)
{
	for (int i = 0; i < size; ++i) {
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = data[i];
	}
}

