#ifndef AVRSHOCK2_USB_TYPES_H_
#define AVRSHOCK2_USB_TYPES_H_

#define AVRSHOCK2_H_TYPES_ONLY 
#include "avrshock2.h"
#undef AVRSHOCK2_H_TYPES_ONLY

#ifndef WIN32
struct __attribute__((__packed__)) avrshock2_usb_data {
	avrshock2_mode_t mode;
	avrshock2_button_t buttons;
	avrshock2_axis_t axis[AVRSHOCK2_AXIS_NAXIS];
};
#else
#pragma pack(push,1)
struct avrshock2_usb_data {
	avrshock2_mode_t mode;
	avrshock2_button_t buttons;
	avrshock2_axis_t axis[AVRSHOCK2_AXIS_NAXIS];
};
#pragma pack(pop)
#endif

#endif
