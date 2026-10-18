
include ../common.mak

CFLAGS += -Os -Wall $(EXTRACFLAGS) -fPIC
CFLAGS += -I. -I$(TOP)/shared -I$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -I$(SRCBASE)/include
CFLAGS += -I$(TOP)/json-c -I$(TOP)/sqlite

LDFLAGS += -L$(TOP_PLATFORM)/nvram$(BCMEX)$(EX7) -lnvram -L$(TOP)/shared -lshared
LDFLAGS += -L$(LIBDIR) -lm -L$(TOP)/sqlite/.libs -lsqlite3
LDFLAGS += -L$(TOP)/json-c/.libs -ljson-c

ifeq ($(RTCONFIG_BCMWL6),y)
CFLAGS  += -I$(SRCBASE)/shared/bcmwifi/include
endif

OBJS    +=  ark_db.o ark_hook.o ark_migrate.o

# ARK engine files to move
# copy gdm.ini from package/*.ini directly
ARK_FILES := app.id app.xdb aqua.ko ark.ko ark.xdb ark.xr gar.env \
	gbusd.ini gbus.ko glog.ini \
	gmq.conf gst.ini gtools.ko init.d/gar init.d/gbs kmod_logger.ko mal.xdb xni.ko xpi.ko \
	ark.ini gloger.ini \
	idp.id idp.xr \
	gbs.env

# gwd (gar watchdog) files live under usr/ark/gwd/. Keep the subdir layout when moving
# them into prebuild, since gwd/services/*.ini share basenames with top-level *.ini
# files (e.g. gloger.ini) that mean something different and must not be overwritten.
GWD_FILES := gwd/gwd.ini gwd/services/gbus.ini gwd/services/gloger.ini gwd/services/gmq.ini

# gbs.d/*.sh are the gbs boot-sequencing hooks (gbs.sh cds into usr/ark/gbs.d and sources
# them by START/STOP order). Keep the subdir layout too: flattening them collides with
# the same-named, differently-behaved usr/sbin/*.sh control scripts in SBIN_FILES.
GBS_D_FILES := gbs.d/aqua.sh gbs.d/gbus.sh gbs.d/gloger.sh gbs.d/gmq.sh gbs.d/gtools.sh gbs.d/gwd.sh

SBIN_FILES := aqua arkctl arklog arklogd ark_service ark_tmctl.sh arktool garp gar.sh gbus \
	gbusd gbusd.sh gdm gdm.sh ghs gini gizmo glog gloger gloger.sh gmdns gmq gmq.sh gns gping gst gst.sh gua gupnp mdns \
	mosquitto mosquitto_pub mosquitto_sub neigh.sh qos.sh xni \
	gbus.sh gbusk.sh gtools.sh gbs.sh gwd gwd.sh

LIB_FILES := libgbus.so libgtools.so libpcrecpp.so.0.0.0 libpcreposix.so.0.0.1


#
# ARK_INI_MOVE is for copying the ark.ini, in the future, we should use ark.ini from tar ball, no need to copy another one
#
export ARK_INI_MOVE :=

##### TODO : add platform here #####
ifeq ($(RTCONFIG_HND_ROUTER_BE_4916),y)
VMLINUX_VERSION := $(shell \
    grep -E '^(VERSION|PATCHLEVEL|SUBLEVEL) =' $(LINUXDIR)/Makefile | \
    awk '{printf "%s.", $$3}' | sed 's/\.$$//' \
)

# $(filter PATTERN...,TEXT)
# check if TEXT matches any PATTERN.
# returns matched word, otherwise empty
ifeq ($(BRCM_CHIP),6813)
ifneq (,$(filter 5.15.176 5.15.182,$(VMLINUX_VERSION)))
export FILE_NAME := "gar_asus_bcm4916_bn98_aarch64_0.1.0.0"
else
export FILE_NAME := "gar_asus_be88u_aarch64_0.0.1.31"
endif
endif

ifeq ($(BRCM_CHIP),6765)
export FILE_NAME := "gar_asus_bcm6765_be92u_aarch64_0.0.1.31"
endif
ifeq ($(BRCM_CHIP),6766)
export FILE_NAME := "gar_asus_bcm6766_gs7_arm_0.0.1.31"
endif
ifeq ($(BRCM_CHIP),6764)
export FILE_NAME := "gar_asus_bcm6764_be58u_v2_aarch64_0.1.0.14"
endif

# BCM6764L / BCM6765 share modules
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME), RT-BE58U TUF-BE3600))
export FILE_NAME := "gar_asus_bcm6765_be92u_aarch64_0.0.1.31"
endif
endif

# RTCONFIG_HND_ROUTER_AX_675X - TUF-AX5400
# RTCONFIG_HND_ROUTER_AX_6756 - TUF-AX5400_v2
ifeq ($(RTCONFIG_HND_ROUTER_AX_675X),y)
export FILE_NAME := "gar_asus_bcm6750_tuf-ax5400_v0.1.2.3"
endif
ifeq ($(RTCONFIG_HND_ROUTER_AX_6756),y)
export FILE_NAME := ""
endif

ifeq ($(RTCONFIG_SOC_MT7988D),y)
export FILE_NAME := "gar_asus_mt7988_bt8_aarch64_cortex-a53_0.0.1.31"
endif
ifeq ($(RTCONFIG_SOC_MT7987),y)
export FILE_NAME := "gar_asus_mt7987_be59_aarch64_0.0.1.31"
endif

# IPQ5322 kernel version or 32/64 bits
ifeq ($(RTCONFIG_SOC_IPQ53XX),y)
ifeq ($(MUSL64),y)
export FILE_NAME := "gar_asus_ipq5322_tuf_aarch64_cortex-a53_neon-vfpv4_0.0.1.31"
else
export FILE_NAME := "gar_asus_ipq5322_bd4d5_arm_cortex-a7_neon-vfpv4_0.0.1.31"
endif
endif

FILE_NAME_RAW := $(subst ",,$(FILE_NAME))
FILE_VERSION := $(lastword $(subst _, ,$(FILE_NAME_RAW)))

ifeq ($(shell v="$(FILE_VERSION)"; \
	if [ -n "$$v" ] && [ "$$(printf '%s\n%s\n' "0.0.1.31" "$$v" | sort -V | head -n1)" = "0.0.1.31" ]; then \
		echo y; \
	fi),y)
export ARK_SIG_MOVE :=
else
export ARK_SIG_MOVE := y
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
	@for f in $(ARK_FILES); do \
		if [ -e "prebuild/usr/ark/$$f" ]; then \
			cp -rf "prebuild/usr/ark/$$f" "prebuild"; \
		else \
			echo "prebuild/usr/ark/$$f NOT EXISTS" >&2; \
		fi; \
	done
	@echo "[ARK] move gwd files (keep subdir layout under gwd.d, since usr/sbin/gwd already takes the 'gwd' name in prebuild)"
	@for f in $(GWD_FILES); do \
		rel="$${f#gwd/}"; \
		if [ -e "prebuild/usr/ark/$$f" ]; then \
			mkdir -p "prebuild/gwd.d/$$(dirname $$rel)"; \
			cp -f "prebuild/usr/ark/$$f" "prebuild/gwd.d/$$rel"; \
		else \
			echo "prebuild/usr/ark/$$f NOT EXISTS" >&2; \
		fi; \
	done
	@echo "[ARK] move gbs.d files (keep subdir layout)"
	@for f in $(GBS_D_FILES); do \
		if [ -e "prebuild/usr/ark/$$f" ]; then \
			mkdir -p "prebuild/$$(dirname $$f)"; \
			cp -f "prebuild/usr/ark/$$f" "prebuild/$$f"; \
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
	#@cp package/gloger.ini prebuild -rf
	@cp package/gdm.ini prebuild -rf
ifeq ($(ARK_SIG_MOVE), y)
	@echo "[ARK] install sig from ark/sig"
	@cp sig/* prebuild -rf
endif
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
	-@cp -f prebuild/*.id $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.xdb $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.xr $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.ini $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.conf $(INSTALLDIR)/usr/ark/
	-@cp -f prebuild/*.env $(INSTALLDIR)/usr/ark/
	-@[ -d prebuild/gwd.d ] && mkdir -p $(INSTALLDIR)/usr/ark/gwd && cp -rf prebuild/gwd.d/* $(INSTALLDIR)/usr/ark/gwd/
	-@[ -d prebuild/gbs.d ] && mkdir -p $(INSTALLDIR)/usr/ark/gbs.d && cp -rf prebuild/gbs.d/* $(INSTALLDIR)/usr/ark/gbs.d/
	@echo "[ARK] prebuild installing $(TARGETDIR)/usr/ark/"
	@install -d $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.ko $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.id $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.xdb $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.xr $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.ini $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.conf $(TARGETDIR)/usr/ark/
	-@cp -f prebuild/*.env $(TARGETDIR)/usr/ark/
	-@[ -d prebuild/gwd.d ] && mkdir -p $(TARGETDIR)/usr/ark/gwd && cp -rf prebuild/gwd.d/* $(TARGETDIR)/usr/ark/gwd/
	-@[ -d prebuild/gbs.d ] && mkdir -p $(TARGETDIR)/usr/ark/gbs.d && cp -rf prebuild/gbs.d/* $(TARGETDIR)/usr/ark/gbs.d/

%.o: %.c
	@echo "[ARK] CC $@"
	@$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.so *.o
