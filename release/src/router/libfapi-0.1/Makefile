# include ../common.mak
# QUANTENNA
include ../common.mak

CFLAGS = -O2 -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I$(TOP)/shared -I$(SRCBASE)/include -I.
ifeq ($(STATIC),1)
CFLAGS += -static
endif

# Install directories
INSTALL_DIR=$(INSTALLDIR)/lib/
CFLAGS += -I.. -I../shared -I../push_log
CFLAGS += -I$(TOOLCHAIN_DIR)/include/ -I$(BUILD_DIR)/staging_dir/include/ -I$(BUILD_DIR)/../../include/
CFLAGS += -I$(TOP)/fapi_wlan_common-1.0.0.1/include -I$(TOP)/libhelper-1.4.0.2/include -DPACKAGE_ID=\"fapi_wlan_common\" -DLOGGING_ID="fapi_wlan_common"
# LDFLAGS += -L$(TOP)/fapi_wlan_common-1.0.0.1 -lfapiwlancommon -L$(TOP)/libhelper-1.4.0.2 -lhelper
LDLIBS+= $(SYSLIBS)

ifeq ($(wildcard $(SRCBASE)/router/libfapi-0.1/*.c),)
all: 
	-cp -f prebuilt/libfapi.so libfapi.so
else
all: libfapi.so
endif

libfapi.so: libfapi.o
	@echo " [libfapi.so] CC $@"
	@$(CC) -shared $(CFLAGS) -o $@ $^

install: all
	install -D libfapi.so $(INSTALLDIR)/usr/lib/libfapi.so
	$(STRIP) $(INSTALLDIR)/usr/lib/libfapi.so

clean:
	rm -f *.o *.a *.so

	
%.o: %.c .%.depend
	@echo " [libfapi.so] CC $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

.depend: $(OBJS:%.o=%.c)
	@$(CC) $(CFLAGS) -M $^ > .depend

.%.depend: %.c
	@$(CC) $(CFLAGS) -M $< > $@

-include $(OBJS:%.o=.%.depend)
