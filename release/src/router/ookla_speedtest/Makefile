include ../common.mak

OOKLA_VER = 38000# updated in 2022.08.16
OOKLA_PATH = OOKLA-$(OOKLA_VER)
OOKLA_VER2 = 39027# updated in 2026.6.17
OOKLA_PATH2 = OOKLA-$(OOKLA_VER2)

.PHONY: prebuild

prebuild:
	@[ -e prebuild ] || mkdir prebuild
# QCA
ifeq ($(RTCONFIG_QCA),y)
ifeq ($(RTCONFIG_QCN550X),y)
	-cp -f $(OOKLA_PATH)/ookla-QCN5502      prebuild/ookla # QCN550X (3.7.12.35)
else ifeq ($(RTCONFIG_SOC_IPQ53XX),y)
ifeq ($(MUSL64),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ53XX      prebuild/ookla
else
	-cp -f $(OOKLA_PATH)/ookla-IPQ53XX-32   prebuild/ookla
endif # MSUL64
endif # RTCONFIG_QCN550X
endif # RTCONFIG_QCA

# BRCM
ifeq ($(RTCONFIG_BCMARM),y)
ifneq (,$(and $(RTCONFIG_HND_ROUTER_BE_4916),$(filter $(ASUSWRT_BRCM_SDK_VERSION),WIFI8_SDK_20260402 WIFI8_SDK_20260601)))
	-cp -f $(OOKLA_PATH2)/ookla-BCM4916-WIFI8 prebuild/ookla # BCM4916 WIFI8 SDK
else
	-cp -f $(OOKLA_PATH)/ookla-AXHND-A7 prebuild/ookla     # AXHND-A7
endif # WIFI8
endif # RTCONFIG_BCMARM

# MTK
ifeq ($(RTCONFIG_RALINK),y)
ifeq ($(or $(RTCONFIG_MT798X),$(RTCONFIG_MT799X)),y)
	-cp -f $(OOKLA_PATH)/ookla-MT7986A  prebuild/ookla     # MT7986A
else
	-cp -f $(OOKLA_PATH)/ookla-MTK7621A prebuild/ookla     # MTK7621A
endif # RTCONFIG_MT798X / RTCONFIG_MT799X
endif # RTCONFIG_RALINK

all: prebuild

install: all
	-@install -d $(INSTALLDIR)/usr/sbin
	-@install -D $(TOP)/ookla_speedtest/prebuild/ookla $(INSTALLDIR)/usr/sbin/ookla

clean:
	-rm -rf prebuild
