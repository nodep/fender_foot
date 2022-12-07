TARGET  = fender_foot
DEVICE  = avr128da48

# get us up to 24MHz; warp speed!
F_CPU   = 24000000

# Get avrdude for Windows here: https://github.com/mariusgreuel/avrdude
AVRDUDE = avrdude -c pkobn_updi -p $(DEVICE)

CFLAGS  = -std=c++20 -Wall -Os -DF_CPU=$(F_CPU)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -D_DEBUG

# these are needed for GCC 12 to avoid some warnings
CFLAGS += --param=min-pagesize=0 -Wno-volatile

LFLAGS  = -Wl,--relax,--gc-sections
#LFLAGS += -Wl,-u,vfprintf -lprintf_min
#LFLAGS += -Wl,-u,vfprintf -lprintf_flt -lm

# I used avr-gcc 12.1 which I got from here:
# https://blog.zakkemble.net/avr-gcc-builds/
# You also need the device specs for new AVRs. Those are in
# the latest AVR-Dx pack which you can get here:
# https://packs.download.microchip.com/
# The file has an extension atpack, but it is actually a zip file.
# Rename it and unzip.
# You need to copy the include files, device specs and .o files
# from the unzipped atpack to avr-gcc respective directories.
# You can use the script in tools to help with that.
COMPILE = avr-g++ -mmcu=$(DEVICE) $(CFLAGS)

OBJPATH = obj
OBJECTS = $(addprefix $(OBJPATH)/, main.o avrdbg.o pedals.o display.o graph.o)
TGTNAME = $(OBJPATH)/$(TARGET)

hex: $(TGTNAME).hex

$(TGTNAME).hex: $(OBJPATH) $(OBJECTS)
	@echo linking...
	@$(COMPILE) -o $(TGTNAME).elf $(OBJECTS) $(LFLAGS)
	@rm -f $(TGTNAME).hex $(TGTNAME).eep.hex
	@avr-objcopy -j .text -j .data -O ihex $(TGTNAME).elf $(TGTNAME).hex
	@avr-objdump -h -S $(TGTNAME).elf > $(TGTNAME).lst
	@echo size:
	@avr-size $(TGTNAME).elf

flash: all hex
	@echo flashing...
	@$(AVRDUDE) -U flash:w:$(TGTNAME).hex:i

clean:
	@echo cleaning...
	@rm -f $(TGTNAME).hex $(TGTNAME).elf $(TGTNAME).lst $(OBJECTS)

reset:
	$(AVRDUDE)

read_fuse:
	$(AVRDUDE) -U hfuse:r:-:h -U lfuse:r:-:h -U efuse:r:-:h

$(OBJPATH):
	@mkdir $(OBJPATH)

$(OBJPATH)/%.o: %.cpp makefile $(OBJPATH)
	@echo compiling $<
	@$(COMPILE) -c $< -o $@

# the following saves the assembly listings and preprocessed sources
#	$(COMPILE) -g -save-temps -c $< -o $@

all: clean hex
