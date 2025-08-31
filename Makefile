#
# Simple GBDK-2020 Makefile: compiles all .c and .s from src/, assets/, and project root
#

# Point this to your GBDK root (defaults to 3 levels up)
ifndef GBDK_HOME
  GBDK_HOME := ../../../
endif

LCC := "$(GBDK_HOME)bin/lcc"

# Output ROM name (Example.gb)
PROJECT := BunkerBoy_jam
BINS    := $(PROJECT).gb

# Where to look for sources
SRC_DIRS     := src assets .
C_SOURCES    := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.c))
ASM_SOURCES  := $(foreach d,$(SRC_DIRS),$(wildcard $(d)/*.s))

# Tell the compiler where headers live
INCLUDES := -I"$(GBDK_HOME)include" -Iinclude -Iassets -IhUGEDriver/include

# Link the prebuilt driver library by adding it as an input
HUGE_LIB := hUGEDriver/gbdk/hUGEDriver.lib

# Optional: target & linker flags (DMG)
LCCFLAGS ?= -msm83:gb -Wm-yo4 -Wm-yt0x19 -Wm-ys

# Debugging (enable with: make GBDK_DEBUG=1)
ifdef GBDK_DEBUG
  LCCFLAGS += -debug -v
endif

all: $(BINS)

# Single-call compile & link
$(BINS): $(C_SOURCES) $(ASM_SOURCES)
	$(LCC) $(LCCFLAGS) $(INCLUDES) -o $@ $^ $(HUGE_LIB)

clean:
	- rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm *.noi *.rst
