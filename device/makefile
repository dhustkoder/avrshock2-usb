TARGET = avrshock2-usb
SOURCES = avrshock2/src/avrshock2.c src/avrshock2-usb.c
HEADERS = avrshock2/src/avrshock2.h
INCLUDE_DIRS = -Iavrshock2/src
MCU = atmega328p
F_CPU = 16000000

# C
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -std=c11 -pedantic-errors -Wall -Wextra -Wno-main -DNDEBUG -DAVRSHOCK2_UART_DEBUG  \
	 -Os -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums         \
         -ffast-math -fstrict-aliasing -ffunction-sections -fdata-sections                  \
	 -fwhole-program -ffreestanding -fno-tree-scev-cprop                                \
	 -Wl,--relax -fno-unwind-tables  -fno-asynchronous-unwind-tables                    \
         -DNDEBUG -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL $(INCLUDE_DIRS)

# AVRDUDE
AVRDUDE = avrdude
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_PROGRAMMER = arduino

all: $(TARGET)

$(TARGET): $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET).elf
	$(OBJCOPY) -j .text  -j .data -O ihex $(TARGET).elf $(TARGET).hex
	$(SIZE) $(TARGET).elf


program: $(TARGET)
	$(AVRDUDE) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200 -U flash:w:$(TARGET).hex:i


clean:
	rm -rf *.elf *.hex
