
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
# ark.ini and gloger.ini and gdm.ini are installed separately below, not here to install
ARK_FILES := app.id app.xdb aqua.ko ark.ko ark.xdb ark.xr gar.env \
	gbusd.ini gbus.ko glog.ini \
	gmq.conf gst.ini gtools.ko init.d/gar kmod_logger.ko mal.xdb xni.ko xpi.ko ark.ini

SBIN_FILES := aqua arkctl arklog arklogd ark_service ark_tmctl.sh arktool garp gar.sh gbus \
	gbusd gbusd.sh gdm gdm.sh ghs gini glog gloger gloger.sh gmdns gmq gns gping gst gst.sh gua gupnp mdns \
	mosquitto mosquitto_pub mosquitto_sub neigh.sh qos.sh xni

LIB_FILES := libgbus.so libgtools.so libpcrecpp.so.0.0.0 libpcreposix.so.0.0.1


#
# ARK_INI_MOVE is for copying the ark.ini, in the future, we should use ark.ini from tar ball, no need to copy another one
#
export ARK_INI_MOVE :=

##### TODO : add platform here #####
ifeq ($(BRCM_CHIP),6813)
export FILE_NAME := "gar_asus_be88u_aarch64_0.0.1.27"
endif

ifeq ($(RTCONFIG_SOC_MT7988D),y)
export FILE_NAME := "gar_asus_mt7988_bt8_aarch64_cortex-a53_0.0.1.27"
endif
ifeq ($(RTCONFIG_SOC_MT7987),y)
export FILE_NAME := "gar_asus_mt7987_be59_aarch64_0.0.1.27"
endif

# IPQ5322 kernel version or 32/64 bits
ifeq ($(RTCONFIG_SOC_IPQ53XX),y)
SUBLEVEL := $(shell grep '^SUBLEVEL =' $(LINUXDIR)/Makefile | awk '{print $$3}')
ifeq ($(MUSL64),y)
export FILE_NAME := "gar_asus_ipq5322_tuf_aarch64_cortex-a53_neon-vfpv4_0.0.1.27"
else
export FILE_NAME := "gar_asus_ipq5322_bd4d5_arm_cortex-a7_neon-vfpv4_0.0.1.27"
endif
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
		if [ -e "prebuild/tmp/gar/$$f" ]; then \
			cp -rf "prebuild/tmp/gar/$$f" "prebuild"; \
		else \
			echo "prebuild/tmp/gar/$$f NOT EXISTS" >&2; \
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
	@echo "[ARK] install ark.ini and gloger.ini and gdm.ini"
ifeq ($(ARK_INI_MOVE), y)
	@echo "[ARK] copy package/ark.ini"
	@cp package/ark.ini prebuild -rf
endif
	@cp package/gloger.ini prebuild -rf
	@cp package/gdm.ini prebuild -rf
	@echo "[ARK] install sig"
	@cp sig/* prebuild -rf
endif

ifeq ($(wildcard $(SRCBASE)/router/ark/*.c),)
all:
	@cp prebuild/$(BUILD_NAME)/* prebuild/
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
	-@cp -f prebuild/*.ko $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/app.id $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.xdb $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.xr $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.ini $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/gmq.conf $(INSTALLDIR)/usr/ark/
	@echo "[ARK] prebuild installing $(TARGETDIR)/usr/ark/"
	@install -d $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.ko $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/app.id $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.xdb $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.xr $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.ini $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/gmq.conf $(TARGETDIR)/usr/ark/

%.o: %.c
	@echo "[ARK] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o
