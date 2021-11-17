include ../common.mak

#OOKLA_VER = 36521# updated in 2019.12.31
#OOKLA_VER = 37112# updated in 2021.08.26
OOKLA_VER = 371237# updated in 2021.10.19
OOKLA_PATH = OOKLA-$(OOKLA_VER)

prebuild:
	-@mkdir prebuild
# QCA
ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074),$(RTCONFIG_SOC_IPQ40XX)),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ807-401X  prebuild/ookla # IPQ80XX / IPQ40XX
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
ifeq ($(RTCONFIG_QCA_PLC2),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ60XX      prebuild/ookla # IPQ60XX
endif
else ifeq ($(RTCONFIG_QCN550X),y)
	-cp -f $(OOKLA_PATH)/ookla-QCN5502      prebuild/ookla # QCN550X (3.7.12.35)
else
	# nothing
endif

# BRCM
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_HND_ROUTER),y)
	-cp -f $(OOKLA_PATH)/ookla-AXHND-A7 prebuild/ookla     # AXHND-A7
else
	-cp -f $(OOKLA_PATH)/ookla-BCM7.x-6.x prebuild/ookla   # BCM6 / BCM7
endif # RTCONFIG_HND_ROUTER
endif # RTCONFIG_BCMARM

all: prebuild

install: all
	-@install -d $(INSTALLDIR)/usr/sbin
	-@install -D $(TOP)/ookla_speedtest/prebuild/ookla $(INSTALLDIR)/usr/sbin/ookla

clean:
	-rm -rf prebuild
