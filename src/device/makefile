# Source
TARGET = avrshock2_usb_device
SOURCES = $(PROJECT_ROOT)/external/avrshock2/src/avrshock2.c $(wildcard *.c)
HEADERS = $(PROJECT_ROOT)/external/avrshock2/src/avrshock2.h $(wildcard *.h)
INCLUDE_DIRS = -I$(PROJECT_ROOT)/src -I$(PROJECT_ROOT)/external/avrshock2/src

# AVR config
MCU = atmega328p
F_CPU = 16000000

# AVRSHOCK2 config
F_AVRSHOCK2 = 250000
MODE        = SPI

# SPI SS
PORT_ATT  = PORTB
DDR_ATT   = DDRB
BIT_ATT   = PB2

# SPI MOSI
PORT_CMD  = PORTB
DDR_CMD   = DDRB
BIT_CMD   = PB3

# SPI MISO
PORT_DAT  = PORTB
DDR_DAT   = DDRB
PIN_DAT   = PINB
BIT_DAT   = PB4

# SPI SCK
PORT_CLK  = PORTB
DDR_CLK   = DDRB
BIT_CLK   = PB5

# build tools
CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -std=gnu11 -pedantic-errors -Wall -Wextra -Wno-main -DNDEBUG -DAVRSHOCK2_UART_DEBUG            \
	 -Os -flto -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums                     \
         -ffast-math -fstrict-aliasing -ffunction-sections -fdata-sections                              \
	 -fwhole-program -ffreestanding -fno-tree-scev-cprop                                            \
	 -Wl,--relax -fno-unwind-tables  -fno-asynchronous-unwind-tables                                \
         $(INCLUDE_DIRS) -DNDEBUG                                                                       \
         -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL                                                                \
         -DF_AVRSHOCK2=$(F_AVRSHOCK2) -DAVRSHOCK2_USB_BAUD=$(BAUD_RATE)UL  -DAVRSHOCK2_$(MODE)          \
         -DAVRSHOCK2_PORT_ATT=$(PORT_ATT) -DAVRSHOCK2_DDR_ATT=$(DDR_ATT) -DAVRSHOCK2_BIT_ATT=$(BIT_ATT) \
         -DAVRSHOCK2_PORT_CMD=$(PORT_CMD) -DAVRSHOCK2_DDR_CMD=$(DDR_CMD) -DAVRSHOCK2_BIT_CMD=$(BIT_CMD) \
         -DAVRSHOCK2_PORT_DAT=$(PORT_DAT) -DAVRSHOCK2_DDR_DAT=$(DDR_DAT) -DAVRSHOCK2_PIN_DAT=$(PIN_DAT) \
         -DAVRSHOCK2_BIT_DAT=$(BIT_DAT) \
         -DAVRSHOCK2_PORT_CLK=$(PORT_CLK) -DAVRSHOCK2_DDR_CLK=$(DDR_CLK) -DAVRSHOCK2_BIT_CLK=$(BIT_CLK)

# AVRDUDE
AVRDUDE = avrdude
AVRDUDE_PORT = $(UPLOAD_DEVICE)
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
