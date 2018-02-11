#ifndef AVRSHOCK2_USB_UART_H_
#define AVRSHOCK2_USB_UART_H_
#include <stdint.h>

void uart_init(void);
void uart_send(const uint8_t* data, short size);


#endif
