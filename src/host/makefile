SOURCE = $(wildcard *.c)
HEADERS = $(wildcard *.h)
INCLUDE_DIRS = -I$(PROJECT_ROOT)/src -I$(PROJECT_ROOT)/external/avrshock2/src

all: $(SOURCE) $(HEADERS)
	$(CC) -std=gnu11 -Wall -Wextra -O3 -DAVRSHOCK2_USB_BAUD=$(BAUD_RATE) $(INCLUDE_DIRS) $(SOURCE) -o ../../avrshock2_usb_host

clean:

