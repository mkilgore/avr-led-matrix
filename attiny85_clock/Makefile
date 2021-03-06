
TARGET := ledclock

MCU := attiny85
F_CPU := 8000000UL
BAUD := 9600UL
AVRDUDE_MCU := t85

PROGRAMMER_TYPE := usbasp
PROGRAMMER_ARGS :=

PREFIX := avr-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
AVRSIZE := avr-size
AVRDUDE := avrdude

SRCS := $(wildcard ./src/*.c)
OBJS := $(SRCS:.c=.o)

CPPFLAGS := -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) -I./include

CFLAGS += -Os -g -std=gnu99 -Wall
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS := -wl,-Map,$(TARGET).map
LDFLAGS := -Wl,--gc-sections

TARGET_ARCH := -mmcu=$(MCU)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -o $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.eeprom: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -S $< > $@

.PHONY: all eeprom clean flash flash_eeprom fuses show_fuses

clean:
	rm -f $(OBJS)
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).eeprom

flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(AVRDUDE_MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

flash_eeprom: $(TARGET).eeprom
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(AVRDUDE_MCU) $(PROGRAMMER_ARGS) -U eeprom:w:$<

LFUSE := 0xE2
HFUSE := 0xDF
EFUSE := 0xFF

FUSE_STRING := -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(AVRDUDE_MCU) \
		$(PROGRAMMER_ARGS) $(FUSE_STRING)

show_fuses:
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(AVRDUDE_MCU) $(PROGRAMMER_ARGS) -nv


