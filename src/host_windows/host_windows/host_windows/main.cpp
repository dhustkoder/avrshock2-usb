

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <iostream>
#include "SerialPort.h"
#define STATIC
#include "../../../../external/vJoy/inc/public.h"
#include "../../../../external/vJoy/inc/vjoyinterface.h"
#include "avrshock2_usb_types.h"


const char* const portName = "\\\\.\\COM3";

//Declare a global object
SerialPort *arduino;

static bool init()
{
	arduino = new SerialPort(portName);
	std::cout << "is connected: " << arduino->isConnected() << std::endl;
	if (!arduino->isConnected()) {
		return false;
	}

	/* handshaking avrshock2 device */
	puts("Trying to handshake avrshock2 device.");
	const uint8_t send_code[] = { 0xDE, 0xAD };
	const uint8_t recv_code_match[] = { 0xC0, 0xDE };
	uint8_t recv_code[] = { 0x00, 0x00 };
	do {
		arduino->writeSerialPort((char*)send_code, sizeof(send_code));
		arduino->readSerialPort((char*)recv_code, sizeof(recv_code));
	} while (memcmp(recv_code, recv_code_match, sizeof(recv_code)) != 0);
	puts("Handshake successful!");
	short ver = vJoyNS::GetvJoyVersion();
	std::cout << "VJOY VER: " << ver << '\n';
	std::cout << "VJOY ENABLED: " << vJoyNS::vJoyEnabled() << '\n';
	vJoyNS::AcquireVJD(1);
	return true;
}

static void term()
{
	delete arduino;
}


int main(int argc, char** argv)
{
	/*
	Stick 1 = left analog stick
	Stick 2 = right analog stick
	POV = dpad
	Button 1 = A
	Button 2 = B
	Button 3 = X
	Button 4 = Y
	Button 5 = LB
	Button 6 = RB
	Button 7 = back
	Button 8 = start
	Button 9 = left analog stick center pushed in
	Button 10 = right analog stick center pushed in
	Button 11 = left trigger
	Button 12 = right trigger
	Button 13 = X silver guide button
	*/

	static const unsigned buttons[] = {
		AVRSHOCK2_BUTTON_CROSS, AVRSHOCK2_BUTTON_CIRCLE,
		AVRSHOCK2_BUTTON_SQUARE, AVRSHOCK2_BUTTON_TRIANGLE,
		AVRSHOCK2_BUTTON_L1,  AVRSHOCK2_BUTTON_R1,
		AVRSHOCK2_BUTTON_SELECT, AVRSHOCK2_BUTTON_START,
		AVRSHOCK2_BUTTON_L3, AVRSHOCK2_BUTTON_R3,
		AVRSHOCK2_BUTTON_L2, AVRSHOCK2_BUTTON_R2,
	};

	if (!init())
		return -1;

	avrshock2_usb_data data, old_data;

	for (;;) {
		arduino->readSerialPort((char*)&data, sizeof(data));
		if (memcmp(&data, &old_data, sizeof(data)) != 0) {
			memcpy(&old_data, &data, sizeof(data));

			for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i) {
				vJoyNS::SetBtn((data.buttons&buttons[i]), 1, i + 1);
			}

			vJoyNS::SetAxis(data.axis[AVRSHOCK2_AXIS_LX] * 128, 1, HID_USAGE_X);
			vJoyNS::SetAxis(data.axis[AVRSHOCK2_AXIS_LY] * 128, 1, HID_USAGE_Y);
			vJoyNS::SetAxis(data.axis[AVRSHOCK2_AXIS_RX] * 128, 1, HID_USAGE_RX);
			vJoyNS::SetAxis(data.axis[AVRSHOCK2_AXIS_RY] * 128, 1, HID_USAGE_RY);
			int pov = -1;
			if (data.buttons&AVRSHOCK2_BUTTON_UP)
				pov = 0;
			else if (data.buttons&AVRSHOCK2_BUTTON_RIGHT)
				pov = 1;
			else if (data.buttons&AVRSHOCK2_BUTTON_DOWN)
				pov = 2;
			else if (data.buttons&AVRSHOCK2_BUTTON_LEFT)
				pov = 3;
			vJoyNS::SetContPov(pov, 1, 1);
		} else {
			Sleep(3);
		}
	}


	term();
	return 0;
}