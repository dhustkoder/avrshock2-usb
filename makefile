

all: host


host:
	make -C src/host

device:
	make -C src/device

upload:
	make -C src/device program

clean:
	make -C src/device clean
	make -C src/host clean

