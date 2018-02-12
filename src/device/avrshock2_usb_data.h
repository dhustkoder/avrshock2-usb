#ifndef AVRSHOCK2_USB_DATA_H_
#define AVRSHOCK2_USB_DATA_H_

#define AVRSHOCK2_H_TYPES_ONLY 
#include "../../external/avrshock2/src/avrshock2.h"
#undef AVRSHOCK2_H_TYPES_ONLY

struct avrshock2_usb_data {
	avrshock2_button_t buttons;
	avrshock2_axis_t axis[AVRSHOCK2_AXIS_SIZE];
};


#endif
