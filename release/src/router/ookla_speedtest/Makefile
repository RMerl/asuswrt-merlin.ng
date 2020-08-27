include ../common.mak

OOKLA_VER = 36521# updated in 2019.12.31
OOKLA_PATH = OOKLA-$(OOKLA_VER)

prebuild:
	-@mkdir prebuild
ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074),$(RTCONFIG_SOC_IPQ40XX)),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ807-401X  prebuild/ookla # IPQ80XX / IPQ40XX
else ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
	-cp -f $(OOKLA_PATH)/ookla-AXHND prebuild/ookla        # AXHND
else
	-cp -f $(OOKLA_PATH)/ookla-HND prebuild/ookla          # HND
endif # HND_ROUTER_AX
else
	-cp -f $(OOKLA_PATH)/ookla-BCM7.x-6.x prebuild/ookla   # BCM6 / BCM7
endif # HND_ROUTER
endif # ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074),$(RTCONFIG_SOC_IPQ40XX)),y)

all: prebuild

install: all
	-@install -d $(INSTALLDIR)/usr/sbin
	-@install -D $(TOP)/ookla_speedtest/prebuild/ookla $(INSTALLDIR)/usr/sbin/ookla

clean:
	-rm -rf prebuild
