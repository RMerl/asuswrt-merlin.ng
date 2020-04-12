include ../../common.mak

ifeq ($(TOP_PLATFORM),)
TOP_PLATFORM=$(TOP)
endif

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -I$(SRCBASE)/include -I$(TOP)/httpd
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample/linux
CFLAGS += -I$(TOP)/bwdpi_source/include
CFLAGS += -I$(TOP)/bwdpi_source/asus_include

LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
ifeq ($(RTCONFIG_BCM_502L07P2),y)
LDFLAGS += -L$(TOP_PLATFORM)/ethctl_lib -lethctl
endif

ifeq ($(RTCONFIG_QTN),y)
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif

ifeq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
CFLAGS += -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux -I$(HND_SRC)/userspace/private/include
ifeq ($(or $(HND_ROUTER_AX_675X),$(HND_ROUTER_AX_6756),$(HND_ROUTER_AX_6710),$(BCM_502L07P2)),y)
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

# BUILD_NAME
ifneq ($(findstring 4G-,$(BUILD_NAME)),)
TARGET_MODEL = RT$(subst -,,$(BUILD_NAME))
else ifneq ($(findstring DSL,$(BUILD_NAME)),)
TARGET_MODEL = $(subst -,_,$(BUILD_NAME))
else
TARGET_MODEL = $(subst -,,$(subst +,P,$(BUILD_NAME)))
endif
ifeq ($(TARGET_MODEL),RTK3)
export RTK3=y
CFLAGS += -DRTK3
TARGET_MODEL:=RTAC3100
endif
CFLAGS  += -D$(TARGET_MODEL)

# for QCA toolchain compile issue, xxxx-ld has no pthead and dl in openwrt-toolchain
ifeq ($(RTCONFIG_QCA),y)
EXTRACC += -lpthread -ldl
EXTRALD =
else
EXTRACC = 
EXTRALD = 
endif

LD1 += $(LDFLAGS) $(EXTRACC)
LD2 += $(LDFLAGS) $(EXTRALD)

# ASUSWRT
OBJS += iqos.o wrs.o stat.o dpi.o wrs_app.o data_collect.o watchdog_check.o
OBJS += dpi_support.o  # dpi_support feature to enhance security
OBJS += wrs_wbl.o      # wrs - white and black list

# TrendMicro
OBJS += conf_app.o ioc_anomaly.o ioc_common.o ioc_qos.o ioc_vp.o ioc_wbl.o ioc_wrs.o ioc_mesh.o

ifeq ($(RT4GAC86U),y)
OBJS += $(LIBDIR)/libc_nonshared.a
endif

ifeq ($(RTCONFIG_BCMARM),y)
vpath %.c $(SRCBASE)/shared
endif

vpath %.c $(TOP)/bwdpi_source/src/tdts_udb_sample

ifeq ($(wildcard $(SRCBASE)/router/bwdpi_source/asus/*.c),)
all:
	-cp -f prebuild/$(BUILD_NAME)/libbwdpi.so libbwdpi.so
	-cp -f prebuild/$(BUILD_NAME)/hwinfo hwinfo
else
all: hwinfo libbwdpi.so dpi_support.prep
endif

libbwdpi.so: $(OBJS)
	@echo " [bwdpi] LD $@"
	@$(LD) $(LD2) -shared -o $@ $^

hwinfo: hwinfo.o
	@$(CC) -o $@ $^ $(LD1) -L. -lgcc_s

# this command is for debug only
dbg_dpi_support: dbg_dpi_support.o dpi_support.o
	@$(CC) -o $@ $^ $(LD1) -L. -lgcc_s

# this command is for debug only
bwdpi_cmd: main.o $(OBJS)
	@echo " [bwdpi] CC $@"
	@$(CC) -o $@ $^ $(LD1) -L. -lgcc_s
	@$(STRIP) bwdpi_cmd

install: all
	@echo "[bwdpi] Installing..."
	@install -d $(INSTALLDIR)/bwdpi_source/usr/lib
	@install -m 755 libbwdpi.so $(INSTALLDIR)/bwdpi_source/usr/lib
	@$(STRIP) $(INSTALLDIR)/bwdpi_source/usr/lib/libbwdpi.so
	@install -d $(INSTALLDIR)/bwdpi_source/usr/sbin
	@install -m 755 hwinfo $(INSTALLDIR)/bwdpi_source/usr/sbin/hwinfo
	@$(STRIP) $(INSTALLDIR)/bwdpi_source/usr/sbin/hwinfo

%.o: %.c
	@echo " [bwdpi] CC $@"
	@$(CC) $(CFLAGS) -c $<

dpi_support.prep: dpi_support.c
	@echo " [bwdpi] compile prep $@"
	@$(CC) $(CFLAGS) -E $< > $@

clean:
	rm -f *.so *.o *.prep hwinfo bwdpi_cmd dbg_dpi_support
