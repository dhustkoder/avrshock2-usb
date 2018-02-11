#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include "uart.h"
#include "avrshock2.h"




noreturn void main(void)
{
	static uint8_t data[sizeof(avrshock2_buttons) + sizeof(avrshock2_analogs)];
	avrshock2_init();
	uart_init();

	avrshock2_set_mode(AVRSHOCK2_MODE_ANALOG, true);

	for (;;) {
		avrshock2_poll();

		memcpy(data, avrshock2_buttons, sizeof(avrshock2_buttons));

		memcpy(data + sizeof(avrshock2_buttons), avrshock2_analogs,
		       sizeof(avrshock2_analogs));

		uart_send(data, sizeof(data));

		_delay_ms(15);
	}
}

