#
# Linux wlceventd Makefile
#
# $Copyright ASUSTeK Corporation$
#
# $Id: Makefile 401201 2013-05-09 03:06:42Z kenlo $
#

include $(SRCBASE)/router/.config

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
ifeq ($(HND_ROUTER_AX), y)
CFLAGS  += -I$(SRCBASE)/../components/math/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include
endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2 $(EXTRACFLAGS)
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += $(EXTRALDFLAGS)

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client -I$(TOP)/httpd -I./sysdeps/qtn
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

#ifdef BCMINTERNAL
ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
#endif

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/json-c
CFLAGS  += -I$(TOP)/sqlite
CFLAGS  += -I$(TOP)/nt_center/lib
CFLAGS  += -I$(TOP)/nt_center/actMail
CFLAGS  += -I$(TOP)/wlc_nt
CFLAGS  += -DASUSWRT_SDK
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -lpthread -L$(TOP)/nt_center/lib -lnt -lm
LDFLAGS += -L$(TOP)/wlc_nt -lwlc_nt_client
else
ifeq ($(RTCONFIG_AMAS),y)
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
endif
endif

ifeq ($(RTCONFIG_CFGSYNC),y)
CFLAGS  += -I$(TOP)/cfg_mnt
LDFLAGS += -L$(TOP)/cfg_mnt -lcfgmnt
endif

ifeq ($(RTCONFIG_SW_HW_AUTH), y)
CFLAGS += -I$(TOP)/sw-hw-auth
endif

ifeq ($(RTCONFIG_LIBASUSLOG),y)
CFLAGS += -I$(TOP)/libasuslog
LDFLAGS += -L$(TOP)/libasuslog -lasuslog
endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

ifeq ($(wildcard $(SRCBASE)/router/wlceventd/*.c),)
all:
	-cp -f prebuilt/$(BUILD_NAME)/wlceventd wlceventd
else
all: wlceventd
endif

OBJS = wlceventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install wlceventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/wlceventd

clean:
	rm -f *.o *.a *.so wlceventd

${OBJS}:

wlceventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

