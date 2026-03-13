include ../../common.mak
include $(TOP)/.config


EXEC = nmp-amas
NMP_AMAS_SO = libnmp-amas.so

SRC=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRC))

CFLAGS  = -I$(TOP)/json-c
LDFLAGS = -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(HND_ROUTER),y)
CFLAGS += -Werror=return-type -Werror=uninitialized -Wno-date-time
endif

CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
LDFLAGS += -L$(TOP)/shared -lshared

ifeq ($(RTCONFIG_REALTEK),y)
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX} -lnvram
else
LDFLAGS += -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram
endif


ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS  += -I$(TOP)/cfg_mnt
LDFLAGS += -L$(TOP)/cfg_mnt -lcfgmnt
endif


ifeq ($(HND_ROUTER),y)
CFLAGS += -DHND_ROUTER
endif

ifeq ($(RTCONFIG_AMAS),y)
CFLAGS  += -I$(TOP)/amas-utils -I$(TOP)/shared/sysdeps/amas
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# all: $(EXEC) libnmp-amas
all: libnmp-amas


$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

libnmp-amas : $(OBJS)
ifneq ($(wildcard *.c),)
	$(CC) -fPIC -shared $^ -o $(NMP_AMAS_SO) $(LDFLAGS)
else
	-cp -f ../../nmp-amas/networkmap/prebuild/$(NMP_AMAS_SO) .
endif


install:
ifneq ($(wildcard ../prebuild/$(NMP_AMAS_SO)),)
	cp ../prebuild/$(NMP_AMAS_SO) $(INSTALLDIR)/lib/$(NMP_AMAS_SO)
else
	$(STRIP) $(NMP_AMAS_SO)
	install -D $(NMP_AMAS_SO) $(INSTALLDIR)/lib/$(NMP_AMAS_SO)
endif

clean:
	rm -f $(OBJS) $(NMP_AMAS_SO) $(EXEC)