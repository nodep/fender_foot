TARGET  = fender_foot
DEVICE  = avr128da48

# get us up to 24MHz; warp speed!
F_CPU   = 24000000

# Get avrdude here: https://github.com/mariusgreuel/avrdude
AVRDUDE = avrdude -c pkobn_updi -p $(DEVICE)

CFLAGS  = -Wall -Os -DF_CPU=$(F_CPU)
CFLAGS += -D_DEBUG

LFLAGS  = -Wl,--relax
#LFLAGS += -u vfprintf

# I used AVR Toolchain 3.7 and the latest AVR-Dx pack:
# https://packs.download.microchip.com/
# You need to copy the include files, device specs and .o files
# from the pack to the avr-gcc respective directories
COMPILE = avr-g++ -mmcu=$(DEVICE) $(CFLAGS)

OBJECTS = main.o avrdbg.o pedals.o

hex: $(TARGET).hex

$(TARGET).hex: $(OBJECTS)
	$(COMPILE) -o $(TARGET).elf $(OBJECTS) $(LFLAGS)
	rm -f $(TARGET).hex $(TARGET).eep.hex
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	avr-objdump -h -S $(TARGET).elf > $(TARGET).lst
	avr-size $(TARGET).elf

flash: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$(TARGET).hex:i

clean:
	rm -f $(TARGET).hex $(TARGET).elf $(TARGET).lst $(OBJECTS)

reset:
	$(AVRDUDE)

read_fuse:
	$(AVRDUDE) -U hfuse:r:-:h -U lfuse:r:-:h -U efuse:r:-:h

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.cpp.o:
	$(COMPILE) -c $< -o $@

# the following saves the assembly listings and preprocessed sources
#	$(COMPILE) -g -save-temps -c $< -o $@

all: clean hex
