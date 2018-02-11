#include <string.h>
#include <stdnoreturn.h>
#include <util/delay.h>
#include "uart.h"
#include "avrshock2.h"




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

