# arm builds
ifeq ($(BUILD),arm)
CROSS := arm-linux-
TARGET := trace
AMS_TRACE_CFLAGS += -DTARGET_BUILD=1
endif

ifeq ($(BUILD),arm64)
CROSS := aarch64-linux-
TARGET := trace
AMS_TRACE_CFLAGS += -DTARGET_BUILD=1
endif

ifeq ($(BUILD),mips)
CROSS := mipsel-linux-
TARGET := trace
CFLAGS += -DTARGET_BUILD=1
endif

ifeq ($(BUILD),)
TARGET := trace_host
endif

CC := $(CROSS)gcc
CXX := $(CROSS)g++
STRIP := $(CROSS)strip
export CC CXX STRIP BUILD

AMS_TRACE_CFLAGS += -Wall -Werror

C_FILES := trace.c trace_decode.c trace_decode_clk.c

override CFLAGS += $(AMS_TRACE_CFLAGS)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(TARGET)
distclean:
	rm -f trace trace_host


$(TARGET): $(C_FILES) Makefile
	$(CC) $(CFLAGS) $(C_FILES) -o $@
