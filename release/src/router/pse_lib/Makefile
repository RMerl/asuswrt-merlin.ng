
include ../common.mak
include $(SRCBASE)/.config
include ../.config

CFLAGS = $(if $(RTCONFIG_DEBUG),-g,-Os) -Wall -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include

CFLAGS += -DHND_ROUTER -DLINUX26 -DCONFIG_BCMWL5
CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx
CFLAGS += -I$(SRCBASE)/shared -I$(HND_SRC)/kernel/linux-4.19/drivers/char/pse 
CFLAGS += -DUSER_API

#CFLAGS += -I$(HND_SRC)/kernel/linux-4.19/include/uapi/linux #-I$(HND_SRC)/kernel/linux-4.19/include/
CFLAGS += -L$(TOP_PLATFORM)/i2c-tools/i2c-tools-4.0/lib -li2c

OBJS = PsElib.o i2cbusses.o util.o

LIBS = libPsE.so

%.o: %.c
	$(CC) $(CFLAGS) -c $<

ifeq ($(wildcard $(SRCBASE)/router/pse_lib/*.c),)
all:
	-cp -f prebuilt/libPsE.so libPsE.so
else
all: $(LIBS)
endif

$(LIBS): $(OBJS)
	$(CC) -shared -o $@ $^  $(CFLAGS)

vpath %.c $(TOP_PLATFORM)/i2c-tools/i2c-tools-4.0/tools

install:
	install -D $(LIBS) $(INSTALLDIR)/usr/lib/$(LIBS)

clean:
	rm -rf *.o *.so $(LIBS) *~
