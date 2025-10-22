
include ../common.mak
include $(SRCBASE)/.config
include ../.config

CFLAGS = $(if $(RTCONFIG_DEBUG),-g,-Os) -Wall
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include

CFLAGS += -DHND_ROUTER -DLINUX26 -DCONFIG_BCMWL5
CFLAGS += -I$(HND_SRC)/bcmdrivers/opensource/include/bcm963xx
CFLAGS += -I$(SRCBASE)/shared -I$(HND_SRC)/kernel/linux-4.19/drivers/char/pse
CFLAGS += -I$(TOP)/pse_lib -L$(TOP)/pse_lib -lPsE
CFLAGS += -DUSER_API

CFLAGS += -L$(TOP_PLATFORM)/i2c-tools/i2c-tools-4.0/lib -li2c

OBJS = pse_test.o

EXE = pse_test

%.o: %.c
	$(CC) $(CFLAGS) -c $<

ifeq ($(wildcard $(SRCBASE)/router/pse_test/*.c),)
all:
	-cp -f prebuilt/pse_test pse_test
else
all: $(EXE)
endif

$(EXE): $(OBJS)
	$(CC) -o $@ $^  $(CFLAGS)

vpath %.c

install:
	install -D $(EXE) $(INSTALLDIR)/usr/sbin/$(EXE)

clean:
	rm -rf *.o $(EXE) *~
