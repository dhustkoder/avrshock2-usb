#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>
#include "avrshock2_usb_types.h"

#ifndef AVRSHOCK2_USB_BAUD
#error Need AVRSHOCK2_USB_BAUD definition
#endif
/* concat B + baud rate, example: B9600 */
#define BAUD_PASTER_AUX(x, y) x ## y
#define BAUD_EVALUATOR_AUX(x, y) BAUD_PASTER_AUX(x, y)
#define BAUD BAUD_EVALUATOR_AUX(B, AVRSHOCK2_USB_BAUD)

#define NBUTTONS (16)
#define NAXIS    (4)

static const int buttons[NBUTTONS] = { 
	BTN_SELECT, BTN_THUMBL, BTN_THUMBR, BTN_START, BTN_TL2, 
	BTN_TR2, BTN_TL, BTN_TR, BTN_Y, BTN_A, BTN_B, BTN_X,
	BTN_DPAD_UP, BTN_DPAD_DOWN, BTN_DPAD_LEFT, BTN_DPAD_RIGHT
};

static const avrshock2_button_t buttons_avrshock2_masks[NBUTTONS] = {
	AVRSHOCK2_BUTTON_SELECT, AVRSHOCK2_BUTTON_L3, AVRSHOCK2_BUTTON_R3,
	AVRSHOCK2_BUTTON_START, AVRSHOCK2_BUTTON_L2, AVRSHOCK2_BUTTON_R2,
	AVRSHOCK2_BUTTON_L1, AVRSHOCK2_BUTTON_R1, AVRSHOCK2_BUTTON_SQUARE,
	AVRSHOCK2_BUTTON_CIRCLE, AVRSHOCK2_BUTTON_CROSS, AVRSHOCK2_BUTTON_TRIANGLE,
	AVRSHOCK2_BUTTON_UP, AVRSHOCK2_BUTTON_DOWN, AVRSHOCK2_BUTTON_LEFT,
	AVRSHOCK2_BUTTON_RIGHT
};

static const int axis[NAXIS] = {
	ABS_X, ABS_Y,
	ABS_RX, ABS_RY
};

static const avrshock2_axis_t axis_avrshock2_idxs[NAXIS] = {
	AVRSHOCK2_AXIS_LX, AVRSHOCK2_AXIS_LY,
	AVRSHOCK2_AXIS_RX, AVRSHOCK2_AXIS_RY

};


static int uinput_fd;
static int avrshock2_fd;
static bool terminate_signal = false;


static void term_system(void);

static void signal_handler(const int signum)
{
	if (terminate_signal) {
		printf("\nSignals sequentially received, aborting program...\n");
		term_system();
		exit(signum);
	}
	printf("\nReceived signal: %d\n", signum);
	terminate_signal = true;
}

static void input_event(const int type, const int code, const int val)
{
	static struct input_event ie = { 0 };
	ie.type = type;
	ie.code = code;
	ie.value = val;
	if ((unsigned)write(uinput_fd, &ie, sizeof(ie)) < sizeof(ie))
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
	uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (uinput_fd == -1) {
		perror("Couldn't open uinput_fd");
		return false;
	}

	avrshock2_fd = open(device_path, O_RDWR | O_NOCTTY | O_SYNC);
	if (avrshock2_fd == -1) {
		perror("Coudn't open avrshock2_fd");
		goto Lclose_uinput;
	}

	/* setup serial communication to raw mode */
	struct termios serial_setup = { 0 };
	if (tcgetattr(avrshock2_fd, &serial_setup) != 0) {
	        perror("tcgetattr error");
	        goto Lclose_avrshock2;
	}
	cfsetospeed(&serial_setup, BAUD);
	cfsetispeed(&serial_setup, BAUD);

	/* control flags */
	serial_setup.c_cflag  = (serial_setup.c_cflag & ~CSIZE) | CS8;
	serial_setup.c_cflag |= (CLOCAL | CREAD);
	serial_setup.c_cflag &= ~(PARENB | PARODD);
	serial_setup.c_cflag &= ~CSTOPB;
	serial_setup.c_cflag &= ~CRTSCTS;

	/* input flags */
	serial_setup.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR);
	serial_setup.c_iflag &= ~(IXON | IXOFF | IXANY);

	/* output flags */
	serial_setup.c_oflag &= ~OPOST;
	/* local flags */
	serial_setup.c_lflag = 0;
	/* control characters */
	serial_setup.c_cc[VMIN]  = 0;
	serial_setup.c_cc[VTIME] = 0;
	if (tcsetattr(avrshock2_fd, TCSANOW, &serial_setup) != 0) {
	        perror("tcsetattr error");
	        goto Lclose_avrshock2;
	}


	/* enables btns */
	ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
	for (int i = 0; i < NBUTTONS; ++i)
		ioctl(uinput_fd, UI_SET_KEYBIT, buttons[i]);

	/* enabel and configure axis and hats */
	ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);
	struct uinput_abs_setup abs_setup = { 0 };
	abs_setup.absinfo.value = 0x80;
	abs_setup.absinfo.maximum = 0xFF;
	for (int i = 0; i < NAXIS; ++i) {
		abs_setup.code = axis[i];
		ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
	}

	/* create device */
	struct uinput_setup usetup = { 0 };
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0xDEAD;
	usetup.id.product = 0xC0DE;
	strcpy(usetup.name, "avrshock2 joystick");
	ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
	ioctl(uinput_fd, UI_DEV_CREATE);

	/* install signal handler */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);

	/* handshaking avrshock2 device */
	puts("Trying to handshake avrshock2 device.");
	const uint8_t send_code[] = { 0xDE, 0xAD };
	const uint8_t recv_code_match[] = { 0xC0, 0xDE };
	uint8_t recv_code[] = { 0x00, 0x00 };
	do {
		serial_send(send_code, sizeof(send_code));
		serial_recv(recv_code, sizeof(recv_code));
	} while (memcmp(recv_code, recv_code_match, sizeof(recv_code)) != 0);
	puts("Handshake successful!\n"
	     "virtual device \'avrshock2 joystick\' created.");

	return true;

Lclose_avrshock2:
	close(avrshock2_fd);
Lclose_uinput:
	close(uinput_fd);
	return false;
}

static void term_system(void)
{
	ioctl(uinput_fd, UI_DEV_DESTROY);
	close(avrshock2_fd);
	close(uinput_fd);
}


int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s [device]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!init_system(argv[1]))
		return EXIT_FAILURE;

	struct avrshock2_usb_data data = { 0 };
	struct avrshock2_usb_data data_old = { 0 };
	while (!terminate_signal) {
		serial_recv(&data, sizeof(data));
		if (memcmp(&data_old, &data, sizeof(data)) != 0) {
			memcpy(&data_old, &data, sizeof(data));
			for (int i = 0; i < NBUTTONS; ++i) {
				input_event(
				  EV_KEY, 
				  buttons[i], 
				  (buttons_avrshock2_masks[i]&data.buttons) ? 1 : 0
				);
			}

			for (int i = 0; i < NAXIS; ++i)
				input_event(EV_ABS, axis[i], data.axis[axis_avrshock2_idxs[i]]);

			input_event(EV_SYN, SYN_REPORT, 0);
		}

		usleep(((((1.0 / AVRSHOCK2_USB_BAUD) * 8) * sizeof(data)) * 1000000) / 2);
	}

	term_system();
	return EXIT_SUCCESS;
}
