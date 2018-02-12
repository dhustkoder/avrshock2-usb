#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/io.h>
#define BAUD (38400)
#include <util/setbaud.h>
#include "../../external/avrshock2/src/avrshock2.h"
#include "avrshock2_usb_data.h"



static void serial_send(const void* const data, const short size)
{
	for (int i = 0; i < size; ++i) {
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = ((uint8_t*)data)[i];
	}
}

static void serial_recv(void* const data, const short size)
{
	for (int i = 0; i < size; ++i) {
		loop_until_bit_is_set(UCSR0A, RXC0);
		((uint8_t*)data)[i] = UDR0;
	}
}

static void serial_init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = (0x01<<UCSZ01)|(0x01<<UCSZ00); /* 8-bit data */ 
	UCSR0B = (0x01<<RXEN0)|(0x01<<TXEN0);   /* Enable RX and TX */
	/* handshake */
	const uint8_t recv_code_match[] = { 0xDE, 0xAD };
	const uint8_t send_code[] = { 0xC0, 0xDE };
	uint8_t recv_code[] = { 0x00, 0x00 };
	do {
		serial_recv(recv_code, sizeof(recv_code));
		serial_send(send_code, sizeof(send_code));
	} while (memcmp(recv_code, recv_code_match, sizeof(recv_code)) != 0);
}


noreturn void main(void)
{
	struct avrshock2_usb_data data;

	serial_init();
	avrshock2_init();
	avrshock2_set_mode(AVRSHOCK2_MODE_ANALOG, true);

	for (;;) {
		if (avrshock2_poll(&data.buttons, data.axis))
			serial_send(&data, sizeof(data));
		_delay_us(2000);
	}
	
}

