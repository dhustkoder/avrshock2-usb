PROJECT_ROOT = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
UPLOAD_DEVICE ?= /dev/ttyUSB0

all: host


host:
	make -C src/host PROJECT_ROOT=$(PROJECT_ROOT)

device:
	make -C src/device PROJECT_ROOT=$(PROJECT_ROOT)

upload:
	make -C src/device PROJECT_ROOT=$(PROJECT_ROOT) UPLOAD_DEVICE=$(UPLOAD_DEVICE) program

clean:
	make -C src/device PROJECT_ROOT=$(PROJECT_ROOT) clean
	make -C src/host PROJECT_ROOT=$(PROJECT_ROOT) clean

