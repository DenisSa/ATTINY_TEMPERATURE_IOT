# Makefile for programming the ATtiny85
# modified the one generated by CrossPack

DEVICE      = attiny85
CLOCK      = 8000000
PROGRAMMER = -c dragon_isp 
OBJECTS    = main.o gpio.o am2302Sensor.o ds18b20.o BasicSerial3.o eeprom_defaults.o
# for ATTiny85
# see http://www.engbedded.com/fusecalc/
FUSES       = -U lfuse:w:0xa2:m -U hfuse:w:0xd5:m -U efuse:w:0xff:m 

# Tune the lines below only if you know what you are doing:
#FILES = main.c softuart.h softuart.c gpio.h gpio.c ds18b20.h ds18b20.c am2302Sensor.h am2302Sensor.c
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $(FILES)

# symbolic targets:
all:	main.eeprom main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

dump: 
	$(AVRDUDE) -U eeprom:r:eedump.hex:i
	
flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

eeprom: all
	$(AVRDUDE) -U eeprom:w:main.eeprom:i
	
fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
	
main.eeprom: main.elf
	rm -f main.eeprom
	avr-objcopy --change-section-lma .eeprom=0 -j .eeprom -O ihex main.elf main.eeprom
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
