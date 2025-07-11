include ./Makefile.config

ifeq ($(ASUSWRT_SDK),y)
-include $(SRCBASE)/router/.config
MAKE = make
CFLAGS = -Wall -O2 -g  $(EXTRACFLAGS)
CFLAGS += -I$(SRCBASE) -I$(SRCBASE)/include -I. -I$(TOP)/nt_center/lib/
CFLAGS += -I$(TOP)/sqlite 
CFLAGS += -I$(TOP)/shared
CFLAGS += -I$(TOP)/json-c
CFLAGS += -DASUSWRT_SDK

# NOTE : Kernel 3.x upper will show one pthread (even if you create more than one)
ifeq ($(CONFIG_KERNEL_PTHREAD_ONE),y)
CFLAGS += -DCONFIG_LINUX3X_OR_4X
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

LDFLAGS = -L$(TOP)/nt_center/lib/ -lnt
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3 -lm
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)${EX7} ${EXTRA_NV_LDFLAGS} -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

LIBS = -lpthread -lgcc_s

ifeq ($(RT4GAC86U),y)
OBJS += $(LIBDIR)/libc_nonshared.a
else
OBJS =
endif

ifeq ($(RTCONFIG_DMALLOC),y)
CFLAGS += -DDMALLOC -DDMALLOC_FUNC_CHECK -I$(STAGEDIR)/usr/include
LDFLAGS += -L$(STAGEDIR)/usr/lib -ldmalloc
endif

ifeq ($(RTCONFIG_TUNNEL),y)
CURL_DIR=$(TOP)/curl-7.21.7
OPENSSL_DIR=$(TOP)/openssl
XML2_DIR=$(TOP)/libxml2
WB_DIR=$(TOP)/wb

CFLAGS  += -DSUPPORT_PUSH_MSG #Support Push Notification Message to iOS/Android device.
CFLAGS  += -I$(CURL_DIR)/include -I$(OPENSSL_DIR)/include -I$(XML2_DIR)/include -I$(WB_DIR) -I$(WB_DIR)/ws_src
LDFLAGS += -L$(OPENSSL_DIR) -L$(CURL_DIR)/lib/.libs -L$(XML2_DIR)/.libs -L$(WB_DIR) -L$(WB_DIR)/output
LIBS    += -lcurl -lssl -lcrypto -lxml2 -lws

ifeq ($(RTCONFIG_TR069),y)
CFLAGS += -I$(TOP)/libssh2/include -I$(TOP)/zlib/include
LDFLAGS += -L$(TOP)/libssh2/src/.libs -lssh2 -L$(TOP)/zlib -lz
endif

endif

ifeq ($(RTCONFIG_CHINATEL_GUANGDONG),y)
CFLAGS  += -I$(TOP)/chinatel/guangdong/Plugin_SDK
LDFLAGS += -L$(TOP)/chinatel/guangdong/Plugin_SDK -lgdlib
endif

ifeq ($(RTCONFIG_CHINATEL_EOS),y)
CFLAGS  += -I$(TOP)/json-c -I$(TOP)/libubox -I$(TOP)/ubus
LDFLAGS += -L$(TOP)/libubox -lubox -lblobmsg_json -L$(TOP)/ubus -lubus
endif

ifeq ($(RTCONFIG_TRUSTZONE),y)
LDFLAGS += -L$(TOP)/ateed/ -latee
endif

ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),RT-BE82M GS-BE18000 GS-BE12000 GS7_PRO GT7))
LDFLAGS += -L$(TOP)/libhostapi -lhostapi
endif

MDIRS = lib

ifeq ($(wildcard $(SRCBASE)/router/nt_center/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/Notify_Event2NC Notify_Event2NC
	-cp -f prebuild/$(BUILD_NAME)/nt_center nt_center
	-cp -f prebuild/$(BUILD_NAME)/nt_monitor nt_monitor
	-cp -f lib/prebuild/$(BUILD_NAME)/libnt.so lib
	-cp -f lib/prebuild/$(BUILD_NAME)/nt_db lib
#	-cp -f lib/prebuild/$(BUILD_NAME)/*.h lib

else
all: ntdir nt_center Notify_Event2NC  nt_monitor
endif

nt_center: nt_center.o $(OBJS)
Notify_Event2NC: Notify_Event2NC.o $(OBJS)
nt_monitor: nt_monitor.o $(OBJS)

ntdir:
	set -e; for d in ${MDIRS}; do $(MAKE) -C $${d}; done
nt_center:
	@$(CC) ${CFLAGS} -o $@ $^ ${LDFLAGS} $(LIBS)
	$(if $(RTCONFIG_DEBUG),,@$(STRIP) $@)

Notify_Event2NC:
	@$(CC) ${CFLAGS} -o $@ $^ ${LDFLAGS} $(LIBS)
	$(if $(RTCONFIG_DEBUG),,@$(STRIP) $@)

nt_monitor:
	@$(CC) ${CFLAGS} -o $@ $^ ${LDFLAGS} $(LIBS)
	$(if $(RTCONFIG_DEBUG),,@$(STRIP) $@)

else # DSL_ASUSWRT_SDK 

CC = mips-linux-uclibc-gcc
MAKE = make 

CFLAGS = -Wall -O2 -g 
CFLAGS += $(TC_CFLAGS) -I.
CFLAGS += -I$(APP_NOTIFICATION_CENTER_DIR)/lib/
CFLAGS += -I$(APP_SQLITE_DIR)/

LDFLAGS = -lm
LDFLAGS += -L$(APP_NOTIFICATION_CENTER_DIR)/lib/ -lnt -lpthread
LDFLAGS += -L$(APP_SQLITE_DIR)/.libs -lsqlite3
LIBS =

MDIRS+= lib

all: ntdir nt_center Notify_Event2NC  nt_monitor

nt_center : nt_center.o 
Notify_Event2NC  : Notify_Event2NC.o
nt_monitor: nt_monitor.o

ntdir:
	set -e; for d in ${MDIRS}; do $(MAKE) -C $${d}; done

nt_center:
	@${CC} ${CFLAGS} $< -o $@ ${LDFLAGS}
Notify_Event2NC:
	@${CC} ${CFLAGS} $< -o $@ ${LDFLAGS}
nt_monitor:
	@$(CC) ${CFLAGS} $< -o $@ ${LDFLAGS} $(LIBS)

endif

.PHONY : all clean
clean:
	set -e; for d in ${MDIRS}; do $(MAKE) -C $${d} clean; done
	rm -rf *.o nt_center Notify_Event2NC nt_monitor

