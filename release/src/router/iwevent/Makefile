include ../common.mak
include $(SRCBASE)/router/.config

EXEC = wlceventd
SRC=iwevent.c
PROGS= wlceventd
STATIC=libiw.a

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/common/include
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth -Werror
CFLAGS += $(EXTRACFLAGS)
CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/json-c
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -I$(TOP)/wlc_nt
CFLAGS  += -DASUSWRT_SDK
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -lpthread -L$(TOP)/nt_center/lib -lnt
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
endif

ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS  += -I$(TOP)/cfg_mnt
LDFLAGS += -L$(TOP)/cfg_mnt -lcfgmnt
endif 

LDFLAGS +=  -L. -liw
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LIBS= -lm
# Composition of the library :
OBJS = iwlib.o

IWLIB=$(STATIC)
IWLIB_INSTALL=install-static

all:  $(STATIC) $(PROGS)

#$(STATIC):
#	$(CC) -static $(OBJS) -liw

wlceventd: $(SRC) $(CONF_H)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $@  $(LIBS)

#iwevent: iwevent.o $(STATIC)

#iwevent: $(SRC) $(CONF_H) $(IWLIB)
#	$(CC) $(CFLAGS)  -o $@ $(SRC)

# Compilation of the static library
$(STATIC): $(OBJS)
	$(RM) $@
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@	

romfs:
	$(ROMFSINST) /bin/iwevent

install: $(STATIC) all
	install $(EXEC) -D $(INSTALLDIR)/usr/sbin/$(EXEC)
	install $(EXEC) $(INSTALLDIR)/usr/sbin/$(EXEC)
	$(STRIP) $(INSTALLDIR)/usr/sbin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o



