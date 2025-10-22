
include ../common.mak

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/json-c -I$(TOP)/sqlite

LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(LIBDIR) -lm -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(HND_ROUTER_BE_4916),y)
CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include
endif

OBJS    +=  ark_db.o ark_hook.o

# ARK engine files to move
# ark.ini and gloger.ini are installed separately below, not here to install
ARK_FILES := app.id app.xdb ark.ko ark.xdb ark.xr \
	gbusd.ini gbus.ko gdd gdr.ini glog.ini \
	gmq.ini gtools.ko kmod_logger.ko mal.xdb xni.ko xpi.ko

SBIN_FILES := arkctl arklog arklogd ark_service ark_tmctl.sh arktool gar.sh gbus \
	gbusd gbusd.sh gdr gdrctl ghs gini glog gloger gloger.sh gmq gns mdns \
	mqtt_pub mqtt_sub neigh.sh qos.sh xni

LIB_FILES := libgbus.so libgtools.so libpcrecpp.so.0.0.0 libpcreposix.so.0.0.1

##### TODO : add platform here #####
ifeq ($(BRCM_CHIP),6813)
export FILE_NAME := "gar_asus_be88u_aarch64_0.0.1.14"
endif

ifeq ($(BRCM_CHIP),6764)
export FILE_NAME := "SmartRouter.ASUS_RTBE58_GO.0.2.7.32"
endif

ifeq ($(RTCONFIG_SOC_MT7988D),y)
export FILE_NAME := "SmartRouter.ASUS_GS7.0.2.7.31"
endif

.PHONY: prebuild

prebuild:
	@[ -e prebuild ] || mkdir prebuild
	@echo "prebuild : $(FILE_NAME)"
ifneq ($(FILE_NAME),)
	@tar -xf package/$(FILE_NAME).tar -C prebuild
	@echo "[ARK] move ark files"
	@for f in $(ARK_FILES); do \
		if [ -e "prebuild/usr/ark/$$f" ]; then \
			cp -rf "prebuild/usr/ark/$$f" "prebuild"; \
		else \
			echo "prebuild/usr/ark/$$f NOT EXISTS" >&2; \
		fi; \
	done
	@for f in $(SBIN_FILES); do \
		if [ -e "prebuild/usr/sbin/$$f" ]; then \
			cp -rf "prebuild/usr/sbin/$$f" "prebuild"; \
		else \
			echo "prebuild/usr/sbin/$$f NOT EXISTS" >&2; \
		fi; \
	done
	@for f in $(LIB_FILES); do \
		if [ -e "prebuild/tmp/ark/$$f" ]; then \
			cp -rf "prebuild/tmp/ark/$$f" "prebuild"; \
		else \
			echo "prebuild/tmp/ark/$$f NOT EXISTS" >&2; \
		fi; \
	done
	@echo "[ARK] remove unused files"
	@rm -rf prebuild/usr
	@rm -rf prebuild/tmp
	@rm -rf prebuild/init.d
	@echo "[ARK] install ark.ini and gloger.ini"
	@cp package/ark.ini prebuild -rf
	@cp package/gloger.ini prebuild -rf
endif

ifeq ($(wildcard $(SRCBASE)/router/ark/*.c),)
all:
	@cp -f prebuild/libark_db.so ./
else
all: prebuild libark_db.so
endif

libark_db.so: $(OBJS)
	@echo "[ARK] LD $@"
	@$(LD) $(LDFLAGS) -shared -o $@ $^

install: all
	@echo "[ARK] installing ark utilies"
	@install -d $(INSTALLDIR)/ark/usr/sbin
	@install -d $(INSTALLDIR)/usr/ark
	@install -d $(INSTALLDIR)/ark/usr/lib
	@install -m 755 libark_db.so $(INSTALLDIR)/ark/usr/lib/libark_db.so
	@$(STRIP) $(INSTALLDIR)/ark/usr/lib/libark_db.so
	@echo "[ARK] prebuild installing $(INSTALLDIR)/ark/usr/sbin"
	@for f in $(SBIN_FILES); do \
		if [ -e "prebuild/$$f" ]; then \
			cp -rf "prebuild/$$f" "$(INSTALLDIR)/ark/usr/sbin/"; \
		fi; \
	done
	@echo "[ARK] prebuild installing $(INSTALLDIR)/ark/usr/lib"
	@for f in $(LIB_FILES); do \
		if [ -e "prebuild/$$f" ]; then \
			cp -rf "prebuild/$$f" "$(INSTALLDIR)/ark/usr/lib/"; \
		fi; \
	done
	@echo "[ARK] prebuild installing $(INSTALLDIR)/usr/ark/"
	@cp -f prebuild/*.ko $(INSTALLDIR)/usr/ark/
	@cp -f prebuild/app.id $(INSTALLDIR)/usr/ark/
	@cp -f prebuild/*.xdb $(INSTALLDIR)/usr/ark/
	@cp -f prebuild/*.xr $(INSTALLDIR)/usr/ark/
	@cp -f prebuild/*.ini $(INSTALLDIR)/usr/ark/

%.o: %.c
	@echo "[ARK] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o
