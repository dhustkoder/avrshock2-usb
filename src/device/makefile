TARGET = avrshock2-usb
SOURCES = ../../external/avrshock2/src/avrshock2.c $(wildcard *.c)
HEADERS = ../../external/avrshock2/src/avrshock2.h $(wildcard *.h)
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
         -DNDEBUG -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL

# AVRDUDE
AVRDUDE = avrdude
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_PROGRAMMER = arduino

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET).elf
	$(OBJCOPY) -j .text  -j .data -O ihex $(TARGET).elf $(TARGET).hex
	$(SIZE) $(TARGET).elf


program: $(TARGET)
	$(AVRDUDE) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200 -U flash:w:$(TARGET).hex:i


clean:
	rm -rf *.elf *.hex
