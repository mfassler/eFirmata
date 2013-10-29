
#####
#####   What are we building?
#####

# Target filename (without extension):
TARGET = eFirmata

# Objects that must be built in order to link
OBJECTS = startup.o
OBJECTS += CMSIS/system_LPC17xx.o
OBJECTS += peripheralClocks.o
OBJECTS += uart.o
OBJECTS += debug.o

OBJECTS += network/ethernet.o
OBJECTS += network/firmataProtocol.o
OBJECTS += network/arp.o
OBJECTS += network/ip.o
OBJECTS += network/icmp.o
OBJECTS += network/udp.o
OBJECTS += network/udpCat.o
OBJECTS += network/udpServices/oscope.o
OBJECTS += network/udpServices/firmataGPI.o
OBJECTS += network/udpServices/firmataPWM.o
OBJECTS += network/udpServices/firmataSPI.o
OBJECTS += ethernetPHY.o
OBJECTS += emac.o

OBJECTS += ssp.o
OBJECTS += adc.o
OBJECTS += dac.o
OBJECTS += pwm.o
#OBJECTS += gpioStuff.o
#OBJECTS += modules/quadrature.o
#OBJECTS += modules/stepperControl_positionEncoder.o
#OBJECTS += modules/stepperControl_dSpin.o
OBJECTS += timer.o
OBJECTS += main.o 


#####
#####   Binaries
#####

CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
AS = arm-none-eabi-as
#LD = arm-none-eabi-ld
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size



#####
#####   Includes, libraries
#####

INCDIRS = -I ./ -I CMSIS/



#####
#####   Flags
#####

CFLAGS = -W -Wall -O2 --std=gnu99 -fgnu89-inline -mcpu=cortex-m3 -mthumb
CFLAGS += -ffunction-sections -fdata-sections
#CFLAGS += -D'SELF_IP_ADDR={192, 168, 11, 177}'
CFLAGS += $(INCDIRS)

#LDFLAGS = -Map $(TARGET).map --gc-sections -T LPC1768-flash.ld

LDFLAGS = -Wl,-Map=$(TARGET).map,--gc-sections -T LPC1768-flash.ld

#####
#####   Targets and Rules
#####

# Default target:
all: $(TARGET).bin

%.bin: %.elf
	@echo
	@echo "  CONVERTING from .elf to .bin"
	$(OBJCOPY) -O binary $^ $@

%.hex: %.elf
	@echo
	@echo "  CONVERTING from .elf to .hex"
	$(OBJCOPY) -R .stack -O ihex $^ $@

%.disasm: %.elf
	$(OBJDUMP) -d $^ > $@

$(TARGET).elf: $(OBJECTS)
	@echo
	@echo "  LINKING to .elf:"
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

install: $(TARGET).bin
	lpc21isp -donotstart -bin $(TARGET).bin /dev/ttyUSB1 57600 14748

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET).hex
	rm -f $(TARGET).bin
	rm -f $(TARGET).elf
	rm -f $(TARGET).map
	rm -f $(TARGET).disasm


.PHONY: all clean size install

# Don't automatically delete intermdiate files (*.o and *.elf)
.SECONDARY:

