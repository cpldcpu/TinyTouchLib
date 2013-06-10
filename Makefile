
DEVICE  = attiny85
F_CPU   = 16000000	# in Hz
AVRDUDE = avrdude -c usbasp -p t85 -V -U flash:w:main.hex

CFLAGS  = -I. -DDEBUG_LEVEL=0
CFLAGS += -Wno-deprecated-declarations -D__PROG_TYPES_COMPAT__
CFLAGS += -Wl,--gc-sections
CFLAGS += -fdata-sections -ffunction-sections
OBJECTS = main.o tinytouchlib.o
COMPILE = avr-gcc -Wall -Os --std=gnu99 -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)


# symbolic targets:
help:
	@echo "make hex ....... to build main.hex"
	@echo "make flash ..... to flash the firmware using avrdude/usbasp"
	@echo "make clean ..... to delete objects and hex file"
	
hex: main.hex

# rule for uploading firmware:
flash: main.hex
	$(AVRDUDE) -U flash:w:main.hex:i

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf *.o 

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

main.elf: $(OBJECTS)	
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size -C --mcu=$(DEVICE) main.elf

# debugging targets:

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
