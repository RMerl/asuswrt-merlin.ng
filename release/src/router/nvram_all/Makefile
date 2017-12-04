# Under Solaris, you need to 
#    CFLAGS += -xO2 -Xc
#    LDLIBS += -lnsl -lsocket
# Some versions of Linux may need
#    CFLAGS += -D_GNU_SOURCE
# To cross-compile
#    CC = arm-linux-gcc

include ../.config

CFLAGS += -Wall -O

CFLAGS += -DASUS
CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include
LDFLAGS = -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX} -lnvram
ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/common/include
LDFLAGS += -lgcc_s
endif

ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif


INSTALL = install

all: nvram_all

nvram_all: nvram_all.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install: nvram_all
	$(STRIP) nvram_all
	$(INSTALL) -d $(INSTALLDIR)/usr/sbin 
	$(INSTALL) nvram_all $(INSTALLDIR)/usr/sbin

clean:
	rm -f nvram_all *.o
