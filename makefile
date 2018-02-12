

all:
	make -C src/device
	make -C src/host

upload:
	make -C src/device program


clean:
	make -C src/device clean
	make -C src/host clean

