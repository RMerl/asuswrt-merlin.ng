include ../common.mak

#OOKLA_VER = 36521# updated in 2019.12.31
OOKLA_VER = 37112# updated in 2021.08.26
OOKLA_PATH = OOKLA-$(OOKLA_VER)

# BCM4912 : seperate bin / header / modules from bcm6756
ifeq ($(BUILD_NAME),$(filter $(BUILD_NAME),ET12 XT12 GT-AX6000 GT-AX11000_PRO GT-AXE16000))
export OOKLA_BCM4912 := y
endif

prebuild:
	-@mkdir prebuild
# QCA
ifeq ($(or $(RTCONFIG_SOC_IPQ8064),$(RTCONFIG_SOC_IPQ8074),$(RTCONFIG_SOC_IPQ40XX)),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ807-401X  prebuild/ookla # IPQ80XX / IPQ40XX
else ifeq ($(RTCONFIG_SOC_IPQ60XX),y)
ifeq ($(RTCONFIG_QCA_PLC2),y)
	-cp -f $(OOKLA_PATH)/ookla-IPQ60XX      prebuild/ookla # IPQ60XX
endif
else
	# nothing
endif

# BRCM
ifeq ($(RTCONFIG_BCMARM),y)
ifeq ($(RTCONFIG_HND_ROUTER),y)
ifeq ($(RTCONFIG_HND_ROUTER_AX),y)
ifeq ($(or $(RTCONFIG_HND_ROUTER_AX_675X),$(RTCONFIG_HND_ROUTER_AX_6710)),y)
	-cp -f $(OOKLA_PATH)/ookla-AXHND-A7 prebuild/ookla     # AXHND-A7 - 675X/6710
else ifeq ($(RTCONFIG_HND_ROUTER_AX_6756),y)
ifeq ($(OOKLA_BCM4912), y)
	-cp -f $(OOKLA_PATH)/ookla-BCM4912 prebuild/ookla      # BCM4912
else
	-cp -f $(OOKLA_PATH)/ookla-AXHND-A7 prebuild/ookla     # AXHND-A7 - 6756
endif # OOKLA_BCM4912
else
	-cp -f $(OOKLA_PATH)/ookla-AXHND-A53 prebuild/ookla    # AXHND-A53
endif # RTCONFIG_HND_ROUTER_AX_675X
else
	-cp -f $(OOKLA_PATH)/ookla-HND prebuild/ookla          # HND
endif # RTCONFIG_HND_ROUTER_AX
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
