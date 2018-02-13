#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include <avr/io.h>
#include "avrshock2.h"
#include "avrshock2_usb_types.h"


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
	#ifndef AVRSHOCK2_USB_BAUD
	#error Need AVRSHOCK2_USB_BAUD definition
	#endif
	#define BAUD AVRSHOCK2_USB_BAUD
	#include <util/setbaud.h>

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	#if USE_2X
	UCSR0A |= (0x01<<U2X0);
	#else
	UCSR0A &= ~(0x01<<U2X0);
	#endif
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
		_delay_us((((1.0 / AVRSHOCK2_USB_BAUD) * 8) * sizeof(data)) * 1000000);
	}
}

