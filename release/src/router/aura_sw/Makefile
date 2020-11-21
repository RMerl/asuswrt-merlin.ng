include ../.config

ifeq ($(HND_ROUTER),y)
CFLAGS += -Werror=return-type -Werror=uninitialized -Wno-date-time 
endif
CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
CFLAGS  += -s -O2 -DNO_PARALLEL

ifeq ($(HND_ROUTER),y)
LDFLAGS += -L$(TOP)/wlcsm$(EXHND) -lwlcsm
LDFLAGS += -lm -L$(TOP)/shared -lshared -L$(TOP_PLATFORM)/nvram${BCMEX}${EX7} -lnvram 
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm$(EXHND) -lwlcsm
else
LDFLAGS += -lm -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX}${EX7} -lnvram
endif

include $(SRCBASE)/router/.config


ifeq ($(wildcard $(SRCBASE)/router/aura_sw/*.c),)
all: 
	-cp -f prebuild/$(BUILD_NAME)/sb_flash_update sb_flash_update
else
all: sb_flash_update
endif

sb_flash_update: sb_flash_update.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

install:
	install -D sb_flash_update $(INSTALLDIR)/usr/sbin/sb_flash_update
	$(STRIP) $(INSTALLDIR)/usr/sbin/sb_flash_update
	install -d $(INSTALLDIR)/usr/aura_sw
	cp -f LED.bin $(INSTALLDIR)/usr/aura_sw/LED.bin

clean:
	rm -rf *.o *~ sb_flash_update
