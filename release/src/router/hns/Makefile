
include ../common.mak

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/json-c -I$(TOP)/sqlite

LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(LIBDIR) -lm -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(RTCONFIG_NOTIFICATION_CENTER),y)
CFLAGS  += -I$(TOP)/nt_center/lib
endif

ifeq ($(HND_ROUTER_BE_4916),y)
CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include
endif

OBJS    +=  hns_db.o hns_wrs_wbl.o hns_infected_cc.o

ifeq ($(RTCONFIG_SOC_MT7988D),y)
export FILE_NAME := "tm_hns_asus_zenwifi-bt6_standalone_2_1_0_2025_03_21"
endif

ifeq ($(BRCM_CHIP),6764)
export FILE_NAME := "tm_hns_asus_rt-be58_go_standalone_2_1_0_2025_07_10"
endif


.PHONY: prebuild

prebuild:
	@[ -e prebuild ] || mkdir prebuild
ifneq ($(FILE_NAME),)
	@tar -zxf package/$(FILE_NAME).tgz -C prebuild
	@cp prebuild/$(FILE_NAME)/* prebuild -rf
	@rm prebuild/$(FILE_NAME) -rf
endif
	@cp package/model.enc prebuild -rf

ifeq ($(wildcard $(SRCBASE)/router/hns/*.c),)
all: prebuild
	@cp -f prebuild/hwinfo hwinfo
	@cp -f prebuild/libhns.so ./
else
all: hwinfo prebuild libhns.so
endif

hwinfo: hwinfo.o
	@$(CC) -o $@ $^ $(LDFLAGS)

libhns.so: $(OBJS)
	@echo " [HNS] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^

install: all
	@echo "[HNS] installing utilies"
	@install -d $(INSTALLDIR)/hns/usr/sbin
	@install -d $(INSTALLDIR)/hns/usr/lib
	@install -m 755 hwinfo $(INSTALLDIR)/hns/usr/sbin/hwinfo
	@$(STRIP) $(INSTALLDIR)/hns/usr/sbin/hwinfo
	@install -m 755 libhns.so $(INSTALLDIR)/hns/usr/lib/libhns.so
	@$(STRIP) $(INSTALLDIR)/hns/usr/lib/libhns.so
	@echo "[HNS] prebuild installing $(INSTALLDIR)/usr/hns"
	@install -d $(INSTALLDIR)/usr/hns
	@cp -f prebuild/hns_cli $(INSTALLDIR)/hns/usr/sbin/
	@cp -f prebuild/shn_ctrl $(INSTALLDIR)/hns/usr/sbin/
	@cp -f prebuild/ufwd $(INSTALLDIR)/hns/usr/sbin/
	@cp -f prebuild/libcrypto.so.1.0.0 $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libDpiWrapper.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libjsoncpp.so.0.10.5 $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libptn_handler.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libshn_dc.so.1.0 $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libshn_pctrl.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libshn_utils.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libtdts_ctrl.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libtdts.so.2.0.1 $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libtmfbeng.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libudb.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libUtilities.so $(INSTALLDIR)/hns/usr/lib/
	@cp -f prebuild/libzip.so.4.0.0 $(INSTALLDIR)/hns/usr/lib/
	@cd $(INSTALLDIR)/hns/usr/lib/ && \
		ln -sf libjsoncpp.so.0.10.5 libjsoncpp.so && \
		ln -sf libjsoncpp.so.0.10.5 libjsoncpp.so.0 && \
		ln -sf libshn_dc.so.1.0 libshn_dc.so && \
		ln -sf libshn_dc.so.1.0 libshn_dc.so.1 && \
		ln -sf libtdts.so.2.0.1 libtdts.so && \
		ln -sf libzip.so.4.0.0 libzip.so && \
		ln -sf libzip.so.4.0.0 libzip.so.4
	@echo "[HNS] prebuild installing $(TARGETDIR)/usr/hns/"
	@install -d $(TARGETDIR)/usr/hns/
	@cp -f prebuild/*.enc $(TARGETDIR)/usr/hns/
	@cp -f prebuild/*.dat $(TARGETDIR)/usr/hns/
	@cp -f prebuild/*.trf $(TARGETDIR)/usr/hns/
	@cp -f prebuild/*.pem $(TARGETDIR)/usr/hns/
	@cp -f prebuild/*.cert $(TARGETDIR)/usr/hns/
	@cp -f prebuild/udata.json $(TARGETDIR)/usr/hns/
	@cp -f prebuild/wrs.parental.conf $(TARGETDIR)/usr/hns/
	@cp -f prebuild/wrs.security.conf $(TARGETDIR)/usr/hns/
	@cp -f prebuild/big_prof.conf $(TARGETDIR)/usr/hns/

%.o: %.c
	@echo " [hns] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o hwinfo libhns.so
