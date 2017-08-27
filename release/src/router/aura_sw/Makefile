include ../.config

CFLAGS  += -I. -I$(TOP)/shared -I$(TOP)/kernel_header/include -I$(SRCBASE)/include
CFLAGS  += -s -O2 -DNO_PARALLEL

LDFLAGS += -lm -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX} -lnvram
LDFLAGS += -L$(TOP)/shared -lshared -L$(TOP)/nvram${BCMEX}${EX7} -lnvram 

include $(SRCBASE)/.config

ifeq ($(wildcard $(SRCBASE)/router/aura_sw/*.c),)
all: 
	-cp -f prebuild/sb_flash_update sb_flash_update
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
