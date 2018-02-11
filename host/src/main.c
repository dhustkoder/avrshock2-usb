#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/uinput.h>


enum Button {
	BUTTON_SELECT   = 0x00,
	BUTTON_L3       = 0x01,
	BUTTON_R3       = 0x02,
	BUTTON_START    = 0x03,
	BUTTON_UP       = 0x04,
	BUTTON_RIGHT    = 0x05,
	BUTTON_DOWN     = 0x06,
	BUTTON_LEFT     = 0x07,

	BUTTON_L2       = 0x08,
	BUTTON_R2       = 0x09,
	BUTTON_L1       = 0x0A,
	BUTTON_R1       = 0x0B,
	BUTTON_TRIANGLE = 0x0C,
	BUTTON_CIRCLE   = 0x0D,
	BUTTON_CROSS    = 0x0E,
	BUTTON_SQUARE   = 0x0F,
	BUTTON_FIRST    = BUTTON_SELECT,
	BUTTON_LAST     = BUTTON_SQUARE
};

enum Analog {
	ANALOG_RX,
	ANALOG_RY,
	ANALOG_LX,
	ANALOG_LY,
	ANALOG_FIRST = ANALOG_RX,
	ANALOG_LAST  = ANALOG_LY
};


static const int virtual_btn_map[] = {
	[BUTTON_SELECT]   = BTN_SELECT,
	[BUTTON_L3]       = BTN_THUMBL,
	[BUTTON_R3]       = BTN_THUMBR,
	[BUTTON_START]    = BTN_START,
	[BUTTON_UP]       = KEY_UP,
	[BUTTON_RIGHT]    = KEY_RIGHT,
	[BUTTON_DOWN]     = KEY_DOWN,
	[BUTTON_LEFT]     = KEY_LEFT,

	[BUTTON_L2]       = BTN_TL2,
	[BUTTON_R2]       = BTN_TR2,
	[BUTTON_L1]       = BTN_TL,
	[BUTTON_R1]       = BTN_TR,
	[BUTTON_TRIANGLE] = BTN_Y,
	[BUTTON_CIRCLE]   = BTN_A,
	[BUTTON_CROSS]    = BTN_B,
	[BUTTON_SQUARE]   = BTN_X
};

static const int virtual_analog_map[] = {
	[ANALOG_RX] = ABS_RX,
	[ANALOG_RY] = ABS_RY,
	[ANALOG_LX] = ABS_X,
	[ANALOG_LY] = ABS_Y
};

static struct uinput_setup usetup;
static int uintput_fd;
static int avrshock2_fd;


static void emit(const int type, const int code, const int val)
{
	static struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;

	write(uintput_fd, &ie, sizeof(ie));
}


static bool init_system(void)
{

	uintput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	if (uintput_fd == -1) {
		perror("Couldn't open /ev/uintput");
		return false;
	}

	avrshock2_fd = open("/dev/ttyUSB0", O_NOCTTY | O_SYNC);
	if (avrshock2_fd == -1) {
		perror("Couldn't open /dev/ttyUSB0");
		close(uintput_fd);
		return false;
	}

	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (avrshock2_fd, &tty) != 0) {
	        perror("error %d from tcgetattr");
	        close(avrshock2_fd);
	        close(uintput_fd);
	        return false;
	}

	cfsetospeed (&tty, B38400);
	cfsetispeed (&tty, B38400);

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

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	                                // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	//tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (avrshock2_fd, TCSANOW, &tty) != 0) {
	        perror("error %d from tcsetattr");
	        close(avrshock2_fd);
	        close(uintput_fd);
	        return false;
	}


	/* setup buttons and analogs */
	ioctl(uintput_fd, UI_SET_EVBIT, EV_KEY);
	for (int i = BUTTON_FIRST; i <= BUTTON_LAST; ++i)
		ioctl(uintput_fd, UI_SET_KEYBIT, virtual_btn_map[i]);
	ioctl(uintput_fd, UI_SET_EVBIT, EV_ABS);
	for (int i = ANALOG_FIRST; i <= ANALOG_LAST; ++i)
		ioctl(uintput_fd, UI_SET_ABSBIT, virtual_analog_map[i]);
	
	/* create device */
	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_VIRTUAL;
	usetup.id.vendor = 0x1234; /* sample vendor */
	usetup.id.product = 0x5678; /* sample product */
	strcpy(usetup.name, "avrshock2 joystick");

	ioctl(uintput_fd, UI_DEV_SETUP, &usetup);

	struct uinput_abs_setup abs_setup;
	abs_setup.absinfo.value   = 0;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 255;
	abs_setup.absinfo.fuzz = 0;
	abs_setup.absinfo.flat = 0;
	abs_setup.absinfo.resolution = 0;
	for (int i = ANALOG_FIRST; i <= ANALOG_LAST; ++i) {
		abs_setup.code = virtual_analog_map[i];
		ioctl(uintput_fd, UI_ABS_SETUP, &abs_setup);
	}

	ioctl(uintput_fd, UI_DEV_CREATE);

	return true;
}

static void term_system(void)
{
	ioctl(uintput_fd, UI_DEV_DESTROY);
	close(avrshock2_fd);
	close(uintput_fd);
}

int main(void)
{
	uint8_t avrshock2_buff[20];

	if (!init_system())
		return EXIT_FAILURE;

	for (;;) {
		read(avrshock2_fd, avrshock2_buff, sizeof(avrshock2_buff));

		for (int i = BUTTON_FIRST; i <= BUTTON_LAST; ++i)
			emit(EV_KEY, virtual_btn_map[i], avrshock2_buff[i]);

		for (int i = ANALOG_FIRST; i <= ANALOG_LAST; ++i)
			emit(EV_ABS, virtual_analog_map[i], avrshock2_buff[BUTTON_LAST + 1 + i]);


		emit(EV_SYN, SYN_REPORT, 0);
		usleep(16000);
	}

	term_system();
	return EXIT_SUCCESS;
}
