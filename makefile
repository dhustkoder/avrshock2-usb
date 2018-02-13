PROJECT_ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
UPLOAD_DEVICE ?= /dev/ttyUSB0
BAUD_RATE ?= 57600
SET_MAKE_VARS = PROJECT_ROOT=$(PROJECT_ROOT) UPLOAD_DEVICE=$(UPLOAD_DEVICE) BAUD_RATE=$(BAUD_RATE)
all: host


host:
	make -C src/host $(SET_MAKE_VARS)

device:
	make -C src/device $(SET_MAKE_VARS)

upload:
	make -C src/device $(SET_MAKE_VARS) program

clean:
	make -C src/device $(SET_MAKE_VARS) clean
	make -C src/host $(SET_MAKE_VARS) clean

