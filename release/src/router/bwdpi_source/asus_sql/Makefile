include ../../common.mak

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/sqlite -I$(TOP)/rc -I$(TOP)/httpd
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample
CFLAGS += -I$(TOP)/bwdpi_source/src/tdts_udb_sample/linux
CFLAGS += -I$(TOP)/bwdpi_source/include
CFLAGS += -I$(TOP)/bwdpi_source/asus_include

LDFLAGS += -L$(TOP)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(TOP)/bwdpi_source/asus -lbwdpi
LDFLAGS += -L$(TOP)/sqlite/.libs -lsqlite3

ifeq ($(RTCONFIG_QTN),y)
CFLAGS += -I$(TOP)/libqcsapi_client
LDFLAGS += -L$(TOP)/libqcsapi_client -lqcsapi_client
endif


ifeq ($(HND_ROUTER),y)
CFLAGS += -idirafter$(LINUXDIR)/include
CFLAGS += -I$(HND_SRC)/userspace/public/include -I$(HND_SRC)/userspace/public/include/linux -I$(HND_SRC)/userspace/private/include
LDFLAGS += -L$(TOP)/wlcsm -lwlcsm -L$(TOP)/bcm_flashutil -lbcm_flashutil -L$(TOP)/bcm_util -lbcm_crc -L$(TOP)/bcm_boardctl -lbcm_boardctl
endif

ifeq ($(RTCONFIG_PERMISSION_MANAGEMENT),y)
CFLAGS += -I$(TOP)/sqlCipher
CFLAGS += -I$(TOP)/PMS_DBapis
CFLAGS += -I$(TOP)/openssl/include/openssl
LDFLAGS += -L$(TOP)/openssl -lcrypto -lssl
LDFLAGS += -L $(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(TOP)/PMS_DBapis -lpms_sql
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
	-cp -f prebuild$(BCMEX)$(EX7)/bwdpi_sqlite bwdpi_sqlite
	-cp -f prebuild$(BCMEX)$(EX7)/libbwdpi_sql.so libbwdpi_sql.so
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
