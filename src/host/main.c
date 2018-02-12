#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>
#include "../device/avrshock2_usb_data.h"


#define NBUTTONS (12)
#define NAXIS    (4)
#define NHATS    (2)

static const int buttons[NBUTTONS] = { 
	BTN_SELECT, BTN_THUMBL, BTN_THUMBR, BTN_START, BTN_TL2, 
	BTN_TR2, BTN_TL, BTN_TR, BTN_Y, BTN_A, BTN_B, BTN_X
};

static const avrshock2_button_t buttons_avrshock2_masks[NBUTTONS] = {
	AVRSHOCK2_BUTTON_SELECT, AVRSHOCK2_BUTTON_L3, AVRSHOCK2_BUTTON_R3,
	AVRSHOCK2_BUTTON_START, AVRSHOCK2_BUTTON_L2, AVRSHOCK2_BUTTON_R2,
	AVRSHOCK2_BUTTON_L1, AVRSHOCK2_BUTTON_R1, AVRSHOCK2_BUTTON_SQUARE,
	AVRSHOCK2_BUTTON_CIRCLE, AVRSHOCK2_BUTTON_CROSS, AVRSHOCK2_BUTTON_TRIANGLE
};

static const int axis[NAXIS] = {
	ABS_X, ABS_Y,
	ABS_RX, ABS_RY
};

static const avrshock2_axis_t axis_avrshock2_idxs[NAXIS] = {
	AVRSHOCK2_AXIS_LX, AVRSHOCK2_AXIS_LY,
	AVRSHOCK2_AXIS_RX, AVRSHOCK2_AXIS_RY

};

static const int hats[NHATS] = {
	ABS_HAT0X, ABS_HAT0Y
};

static int uintput_fd;
static int avrshock2_fd;


static void input_event(const int type, const int code, const int val)
{
	static struct input_event ie = { 0 };
	ie.type = type;
	ie.code = code;
	ie.value = val;
	if ((unsigned)write(uintput_fd, &ie, sizeof(ie)) < sizeof(ie))
		perror("input_event write failed");
}

static void serial_send(const void* const data, const short size)
{
	if (write(avrshock2_fd, data, size) < size)
		perror("serial_send write failed");
}

static void serial_recv(void* const data, const short size)
{
	if (read(avrshock2_fd, data, size) == -1)
		perror("serial_recv read failed");
}

static bool init_system(const char* const device_path)
{

	uintput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (uintput_fd == -1) {
		perror("Couldn't open uinput_fd");
		return false;
	}

	avrshock2_fd = open(device_path, O_RDWR | O_NOCTTY | O_SYNC);
	if (avrshock2_fd == -1) {
		perror("Coudn't open avrshock2_fd");
		close(uintput_fd);
		return false;
	}

	struct termios tty = { 0 };
	if (tcgetattr(avrshock2_fd, &tty) != 0) {
	        perror("tcgetattr error");
	        close(avrshock2_fd);
	        close(uintput_fd);
	        return false;
	}

	cfsetospeed(&tty, B38400);
	cfsetispeed(&tty, B38400);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	                                // no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls,
	                                    // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);  // shut off parity
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	if (tcsetattr(avrshock2_fd, TCSANOW, &tty) != 0) {
	        perror("tcsetattr error");
	        close(avrshock2_fd);
	        close(uintput_fd);
	        return false;
	}


	/* enables btns */
	ioctl(uintput_fd, UI_SET_EVBIT, EV_KEY);
	for (int i = 0; i < NBUTTONS; ++i)
		ioctl(uintput_fd, UI_SET_KEYBIT, buttons[i]);

	/* enabel and configure axis and hats */
	ioctl(uintput_fd, UI_SET_EVBIT, EV_ABS);
	struct uinput_abs_setup abs_setup = { 0 };
	abs_setup.absinfo.maximum = 0xFF;
	for (int i = 0; i < NAXIS; ++i) {
		abs_setup.code = axis[i];
		ioctl(uintput_fd, UI_ABS_SETUP, &abs_setup);
	}
	for (int i = 0; i < NHATS; ++i) {
		abs_setup.code = hats[i];
		ioctl(uintput_fd, UI_ABS_SETUP, &abs_setup);
	}

	/* create device */
	struct uinput_setup usetup = { 0 };
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0xDEAD;
	usetup.id.product = 0xC0DE;
	strcpy(usetup.name, "avrshock2 joystick");
	ioctl(uintput_fd, UI_DEV_SETUP, &usetup);
	ioctl(uintput_fd, UI_DEV_CREATE);


	/* handshaking avrshock2 device */
	puts("Trying to handshake avrshock2 device.\n"
	     "If this is taking too long, try disconecting and connecting your device back.");
	const uint8_t send_code[] = { 0xDE, 0xAD };
	const uint8_t recv_code_match[] = { 0xC0, 0xDE };
	uint8_t recv_code[] = { 0x00, 0x00 };
	do {
		serial_send(send_code, sizeof(send_code));
		serial_recv(recv_code, sizeof(recv_code));
	} while (memcmp(recv_code, recv_code_match, sizeof(recv_code)) != 0);
	puts("Handshake successful!\n"
	     "virtual device \'avrshock2 joystick\' created");

	return true;
}

static void term_system(void)
{
	ioctl(uintput_fd, UI_DEV_DESTROY);
	close(avrshock2_fd);
	close(uintput_fd);
}


int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s [device]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!init_system(argv[1]))
		return EXIT_FAILURE;

	struct avrshock2_usb_data data;

	for (;;) {
		serial_recv(&data, sizeof(data));
		
		for (int i = 0; i < NBUTTONS; ++i) {
			input_event(
			  EV_KEY, 
			  buttons[i], 
			  (buttons_avrshock2_masks[i]&data.buttons) ? 1 : 0
			);
		}

		for (int i = 0; i < NAXIS; ++i)
			input_event(EV_ABS, axis[i], data.axis[axis_avrshock2_idxs[i]]);

		if (data.buttons&AVRSHOCK2_BUTTON_LEFT)
			input_event(EV_ABS, ABS_HAT0X, 0);
		else if (data.buttons&AVRSHOCK2_BUTTON_RIGHT)
			input_event(EV_ABS, ABS_HAT0X, 255);
		else
			input_event(EV_ABS, ABS_HAT0X, 127);

		if (data.buttons&AVRSHOCK2_BUTTON_UP)
			input_event(EV_ABS, ABS_HAT0Y, 0);
		else if (data.buttons&AVRSHOCK2_BUTTON_DOWN)
			input_event(EV_ABS, ABS_HAT0Y, 255);
		else
			input_event(EV_ABS, ABS_HAT0Y, 127);
				
		input_event(EV_SYN, SYN_REPORT, 0);
		usleep(2000);
	}

	term_system();
	return EXIT_SUCCESS;
}
