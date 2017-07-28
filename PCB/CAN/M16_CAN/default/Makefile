###############################################################################
# Makefile for the project CAN_Test
###############################################################################

## General Flags
PROJECT = CAN_Test
MCU = atmega16
TARGET = CAN_Test.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -O0 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = CAN_Test.o CAN_BUS.o mcp2515.o SPI_Ctrl.o LCD_CTRL.o USART.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) CAN_Test.hex CAN_Test.eep size

## Compile
CAN_Test.o: ../CAN_Test.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

CAN_BUS.o: ../CAN_BUS.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

mcp2515.o: ../mcp2515.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

SPI_Ctrl.o: ../SPI_Ctrl.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

LCD_CTRL.o: ../LCD_CTRL.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

USART.o: ../USART.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@sh avr-mem.sh ${TARGET} ${MCU}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) CAN_Test.elf dep/* CAN_Test.hex CAN_Test.eep

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

