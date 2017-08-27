include ../common.mak
include $(SRCBASE)/router/.config

CFLAGS += -Wall -Os $(EXTRACFLAGS)
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE) -I$(SRCBASE)/include -I./include -I$(TOP)/libevent-2.0.21 -I$(TOP)/libevent-2.0.21/include -I$(TOP)/networkmap
CFLAGS += -I./sysdeps
CFLAGS += -I$(TOP)/openssl/include
CFLAGS += -I$(TOP)/json-c

LDFLAGS += -L$(TOP)/nvram${BCMEX}${EX7} -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -lpthread
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c
LDFLAGS += -lm
LDFLAGS += -L$(TOP)/libevent-2.0.21/.libs -levent

ifeq ($(HND_ROUTER),y)
CFLAGS += -DPTHREAD_STACK_SIZE_2M
LDFLAGS += -L$(TOP)/wlcsm -lwlcsm
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
CFLAGS += -I$(SRCBASE)/common/include
else ifeq ($(RTCONFIG_RALINK),y)
CFLAGS += -I$(TOP)/shared/sysdeps/ralink -I$(TOP)/wireless_tools
endif 

ifeq ($(RTCONFIG_RALINK),y)
COMMON_OBJS += ralink.o
vpath %.c sysdeps/ralink
else ifeq ($(RTCONFIG_QCA),y)
CFLAGS += -I$(TOP)/shared/sysdeps/qca -I$(TOP)/wireless_tools
COMMON_OBJS += qca.o
vpath %.c sysdeps/qca
else ifeq ($(RTCONFIG_REALTEK),y)
COMMON_OBJS += realtek.o
vpath %.c sysdeps/realtek
else
COMMON_OBJS += broadcom.o
vpath %.c sysdeps/broadcom
endif 

COMMON_OBJS += encrypt.o cfg_crc.o cfg_udp.o cfg_dencrypt.o cfg_common.o cfg_ipc.o cfg_wevent.o cfg_upgrade.o cfg_clientlist.o
SERVER_OBJS += $(COMMON_OBJS) cfg_mnt.o
CLIENT_OBJS += $(COMMON_OBJS) cfg_client.o

ifeq ($(or $(MAPAC1300),$(MAPAC2200),$(VRZAC1300)),y)
CFLAGS += -DREPORT_PAP_INFO
#COMMON_OBJS += cfg_onboarding.o
#CFLAGS += -DONBOARDING
else
ifneq ($(RTCONFIG_REALTEK),y)
COMMON_OBJS += cfg_roaming.o
CFLAGS += -DLEGACY_ROAMING
COMMON_OBJS += cfg_onboarding.o
CFLAGS += -DONBOARDING
# for wireless channel sync
#CFLAGS += -DSYNC_WCHANNEL
# for roaming info for sta
#COMMON_OBJS += cfg_roaminginfo.o
#CFLAGS += -DROAMING_INFO
# for master detection
#CFLAGS += -DMASTER_DET
endif
endif

ifeq ($(RTCONFIG_MASTER_DET),y)
CFLAGS += -DMASTER_DET
endif

ifeq ($(wildcard $(SRCBASE)/router/cfg_mnt/*.c),)
all:
	-cp -f prebuild/cfg_server cfg_server
	-cp -f prebuild/cfg_client cfg_client
	-cp -f prebuild/libcfgmnt.so libcfgmnt.so
else
all: cfg_server cfg_client libcfgmnt.so
endif

cfg_server: $(SERVER_OBJS)
	echo " [cfg_mnt] CC $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

cfg_client: $(CLIENT_OBJS)
	echo " [cfg_mnt] CC $@"
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

libcfgmnt.so: cfg_lib.o
	@echo " [libcfgmnt] CC $@"
	@$(CC) -fPIC -shared $(CFLAGS) -o $@ $^

install: all
	@echo "[cfg_mnt] Installing..."
	@install -D cfg_server $(INSTALLDIR)/sbin/cfg_server
	@install -D cfg_client $(INSTALLDIR)/sbin/cfg_client
	@install -D libcfgmnt.so $(INSTALLDIR)/usr/lib/libcfgmnt.so
	@$(STRIP) $(INSTALLDIR)/sbin/cfg_server
	@$(STRIP) $(INSTALLDIR)/sbin/cfg_client
	@$(STRIP) $(INSTALLDIR)/usr/lib/libcfgmnt.so
	@chmod 0500 $(INSTALLDIR)/sbin/cfg_server
	@chmod 0500 $(INSTALLDIR)/sbin/cfg_client

clean:
	rm -f cfg_server cfg_client *.so *.o .*.depend
