#ifndef AVRSHOCK2_USB_TYPES_H_
#define AVRSHOCK2_USB_TYPES_H_

#define AVRSHOCK2_H_TYPES_ONLY 
#include "avrshock2.h"
#undef AVRSHOCK2_H_TYPES_ONLY

#define AVRSHOCK2_USB_DEVICE_BAUD (57600)
#define AVRSHOCK2_USB_HOST_BAUD    B57600

struct avrshock2_usb_data {
	avrshock2_button_t buttons;
	avrshock2_axis_t axis[AVRSHOCK2_AXIS_SIZE];
};


#endif
