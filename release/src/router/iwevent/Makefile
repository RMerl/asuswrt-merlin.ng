include ../common.mak

EXEC = wlceventd
SRC=iwevent.c
PROGS= wlceventd
STATIC=libiw.a

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/common/include
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth -Werror
CFLAGS += $(EXTRACFLAGS)
ifeq ($(RTCONFIG_RALINK),y)
CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/generated/autoconf.h
else
CONF_H  = $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h
endif

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

ifeq ($(MT798X),y)
CFLAGS	+= -DMT798X
endif

ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS  += -I$(TOP)/cfg_mnt
CFLAGS  += -I$(TOP)/networkmap/api
LDFLAGS += -L$(TOP)/cfg_mnt -lcfgmnt
LDFLAGS += -L$(TOP)/networkmap/api/ -lnmpapi
endif 

LDFLAGS +=  -L. -liw
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LIBS= -lm

ifneq ($(wildcard ./prebuild),)
all:
	-cp prebuild/$(EXEC) $(EXEC)
else
# Composition of the library :
OBJS = iwlib.o

IWLIB=$(STATIC)
IWLIB_INSTALL=install-static

all:  $(STATIC) $(PROGS)
endif

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



