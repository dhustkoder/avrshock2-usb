#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/io.h>
#define BAUD (38400)
#include <util/setbaud.h>
#include "avrshock2.h"


static void uart_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = (0x01<<UCSZ01)|(0x01<<UCSZ00); /* 8-bit data */ 
	UCSR0B = (0x01<<RXEN0)|(0x01<<TXEN0);   /* Enable RX and TX */
}

static void uart_send(const uint8_t* const data, const short size)
{
	for (int i = 0; i < size; ++i) {
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = data[i];
	}
}


noreturn void main(void)
{
	static uint8_t data[sizeof(avrshock2_buttons) + sizeof(avrshock2_analogs)];
	static uint8_t data_old[sizeof(data)];

	avrshock2_init();
	uart_init();

	avrshock2_set_mode(AVRSHOCK2_MODE_ANALOG, true);

	memset(data, 0, sizeof(data));
	memset(data_old, 0, sizeof(data));

	for (;;) {
		avrshock2_poll();

		memcpy(data, avrshock2_buttons, sizeof(avrshock2_buttons));

		memcpy(data + sizeof(avrshock2_buttons), avrshock2_analogs,
		       sizeof(avrshock2_analogs));

		if (memcmp(data, data_old, sizeof(data)) != 0) {
			uart_send(data, sizeof(data));
			memcpy(data_old, data, sizeof(data));
		}

		_delay_ms(16);
	}
}

