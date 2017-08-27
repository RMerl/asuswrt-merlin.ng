include ../common.mak

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(SRCBASE)/include -I$(TOP)/sqlCipher -I$(TOP)/PMS_DBapis 
CFLAGS += -I$(TOP)/util-linux/include

LDFLAGS += -L $(TOP)/sqlCipher/.libs -lsqlcipher
LDFLAGS += -L$(SRCBASE)/router/openssl -lcrypto 

ifeq ($(RTCONFIG_QCA),y)
LDFLAGS1 += $(LDFLAGS) -ldl -lpthread
else
LDFLAGS1 += $(LDFLAGS) -lpthread
endif

# ASUSWRT
OBJS += main.o customizeAPIs.o PMS_DBAPIs.o
EXE := PMS_sqlite_test
LIB := libpms_sql.so

ifeq ($(wildcard *.c),)
all:
	-cp -f prebuild/libpms_sql.so libpms_sql.so
else
all: $(EXE) $(LIB)
endif

$(EXE): $(OBJS)
	@echo " [PMS_sqlite_test] CC $@"
	@$(CC) -g -o $@ $^ $(LDFLAGS1)
	@$(STRIP) PMS_sqlite_test 

$(LIB): PMS_DBAPIs.o customizeAPIs.o 
	@echo " [PMS_sqlite] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^
install: all
	@echo "[PMS_sqlite] Installing..."
ifneq ($(wildcard *.c),)
	@install -D PMS_sqlite_test $(INSTALLDIR)/PMS_DBapis/usr/sbin/PMS_sqlite_test
	@$(STRIP) $(INSTALLDIR)/PMS_DBapis/usr/sbin/PMS_sqlite_test
endif
	@install -d $(INSTALLDIR)/PMS_DBapis/usr/lib
	@install -m 755 libpms_sql.so $(INSTALLDIR)/PMS_DBapis/usr/lib
	@$(STRIP) $(INSTALLDIR)/PMS_DBapis/usr/lib/libpms_sql.so

%.o: %.c
	@echo " [PMS_sqlite] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *.so PMS_sqlite_test 
