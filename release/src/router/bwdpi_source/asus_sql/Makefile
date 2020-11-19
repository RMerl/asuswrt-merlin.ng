include ../../common.mak

ifeq ($(TOP_PLATFORM),)
TOP_PLATFORM=$(TOP)
endif


CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/sqlite -I$(TOP)/rc -I$(TOP)/httpd
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample/linux
CFLAGS += -I$(TOP)/bwdpi_source/include
CFLAGS += -I$(TOP)/bwdpi_source/asus_include

LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(TOP)/bwdpi_source/asus -lbwdpi
LDFLAGS += -L$(LIBDIR) -lm -L$(TOP)/sqlite/.libs -lsqlite3


ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
CFLAGS += -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux -I$(HND_SRC)/userspace/private/include
ifeq ($(HND_ROUTER_AX_675X),y)
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm -L$(TOP_PLATFORM)/bcm_flashutil -lbcm_flashutil -L$(TOP_PLATFORM)/gen_util -lgen_util -L$(TOP_PLATFORM)/sys_util -lsys_util -L$(TOP_PLATFORM)/bcm_util -lbcm_util -L$(TOP_PLATFORM)/bcm_boardctl -lbcm_boardctl
else
LDFLAGS += -L$(TOP_PLATFORM)/wlcsm -lwlcsm -L$(TOP_PLATFORM)/bcm_flashutil -lbcm_flashutil -L$(TOP_PLATFORM)/bcm_util -lbcm_crc -L$(TOP_PLATFORM)/bcm_boardctl -lbcm_boardctl
endif
endif
ifeq ($(HND_ROUTER_AX),y)
CFLAGS += -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/math/include
CFLAGS += -I$(SRCBASE)/wl -I$(SRCBASE)/wl/sys -I$(SRCBASE)/../components/apps/wps/brcm_apps/include/
endif

ifeq ($(RTCONFIG_BCMARM),y)
CFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
CFLAGS += -I$(TOP)/openssl/include/openssl
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -L $(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
endif

# add rule for ProgControl3 new mechanism
ifeq ($(RTCONFIG_SOC_IPQ8064),y)
	# ipq806x
else ifeq ($(RTCONFIG_SOC_IPQ8074),y)
CFLAGS += -DPGCONTROL3
else ifeq ($(RTCONFIG_SOC_IPQ40XX),y)
	# ipq40xx
CFLAGS += -DPGCONTROL3
else ifeq ($(or $(RTCONFIG_QCA956X),$(RTCONFIG_QCN550X)),y)
	# qca956x / qcn550x
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
CFLAGS += -DPGCONTROL3
else ifeq ($(RTCONFIG_RALINK),y)
	# mtk
else ifeq ($(RTCONFIG_LANTIQ),y)
	# lantiq
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_BCM7),y)
	# bcm7
else ifeq ($(HND_ROUTER),y)
	# hnd / axhnd
CFLAGS += -DPGCONTROL3
else ifeq ($(RTCONFIG_BCM_7114),y)
	# bcm7114
CFLAGS += -DPGCONTROL3
else
ifeq ($(RTCONFIG_BCMSMP),y)
	# bcm6_smp
CFLAGS += -DPGCONTROL3
endif
endif
endif

# for QCA toolchain compile issue, xxxx-ld has no pthead and dl in openwrt-toolchain
ifeq ($(RTCONFIG_QCA),y)
EXTRACC = -lpthread -ldl
EXTRALD =
else
EXTRACC = -lpthread
EXTRALD = -lpthread
endif

LD1 += $(LDFLAGS) $(EXTRACC)
LD2 += $(LDFLAGS) $(EXTRALD)

# ASUSWRT
OBJS += bwdpi_sqlite.o traffic_analyzer.o sqlite_stat.o web_history.o AiProtectionMonitor.o

ifeq ($(RTCONFIG_BCMARM),y)
vpath %.c $(SRCBASE)/shared
endif

ifeq ($(wildcard $(SRCBASE)/router/bwdpi_source/asus_sql/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/bwdpi_sqlite bwdpi_sqlite
	-cp -f prebuild/$(BUILD_NAME)/libbwdpi_sql.so libbwdpi_sql.so
else
all: bwdpi_sqlite libbwdpi_sql.so
endif

bwdpi_sqlite: $(OBJS)
	@echo " [bwdpi_sqlite] CC $@"
	@$(CC) -o $@ $^ $(LD1)
	@$(STRIP) bwdpi_sqlite

libbwdpi_sql.so: sqlite_stat.o
	@echo " [bwdpi_sqlite] LD $@"
	@$(LD) $(LD2) -shared -o $@ $^

install: all
	@echo "[bwdpi_sqlite] Installing..."
	@install -D bwdpi_sqlite $(INSTALLDIR)/bwdpi_source/usr/sbin/bwdpi_sqlite
	@$(STRIP) $(INSTALLDIR)/bwdpi_source/usr/sbin/bwdpi_sqlite
	@chmod 0500 $(INSTALLDIR)/bwdpi_source/usr/sbin/bwdpi_sqlite
	@install -d $(INSTALLDIR)/bwdpi_source/usr/lib
	@install -m 755 libbwdpi_sql.so $(INSTALLDIR)/bwdpi_source/usr/lib
	@$(STRIP) $(INSTALLDIR)/bwdpi_source/usr/lib/libbwdpi_sql.so

	@cd $(INSTALLDIR)/bwdpi_source/usr/sbin && ln -sf bwdpi_sqlite TrafficAnalyzer
	@cd $(INSTALLDIR)/bwdpi_source/usr/sbin && ln -sf bwdpi_sqlite WebHistory
	@cd $(INSTALLDIR)/bwdpi_source/usr/sbin && ln -sf bwdpi_sqlite AiProtectionMonitor

%.o: %.c
	@echo " [bwdpi_sqlite] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o bwdpi_sqlite libbwdpi_sql.so
