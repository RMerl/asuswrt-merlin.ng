ifneq ($(strip $(BUILD_SWMDK)),)
befall:all
else
befall:
	@echo "Skipping switch mdk "
endif

MDK_BALL = $(wildcard mdk-*.gz)
untar:
	if [ ! -e RELEASE ]; then \
	    echo untaring $(MDK_BALL); \
	  (tar --strip-components=1 -xkf $(MDK_BALL) 2> /dev/null || true); \
    else \
	    echo $(MDK_BALL) untared already; \
	fi

#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
MDK := $(shell pwd)
CDK = $(MDK)/cdk
BMD = $(MDK)/bmd
PHY = $(MDK)/phy
MDK_BLD_DIR = $(MDK)/examples/linux-user

.PHONY: all cdk bmd phy_bld clean cdk_clean bmd_clean phy_clean

all:untar
	cd $(MDK_BLD_DIR)
	$(MAKE) -j1 -C $(MDK_BLD_DIR)

install:
	@echo done

cdk:
	cd $(MDK_BLD_DIR)
	$(MAKE) -C $(MDK_BLD_DIR) cdk

bmd:
	cd $(MDK_BLD_DIR)
	$(MAKE) -C $(MDK_BLD_DIR) bmd

phy_bld:
	cd $(MDK_BLD_DIR)
	$(MAKE) -C $(MDK_BLD_DIR) phy


clean:
	-$(MAKE) -C $(MDK_BLD_DIR) clean

cdk_clean:
	-$(MAKE) -C $(MDK_BLD_DIR) cdk_clean

bmd_clean:
	-$(MAKE) -C $(MDK_BLD_DIR) bmd_clean

phy_clean:
	-$(MAKE) -C $(MDK_BLD_DIR) phy_clean


#
# Set our CommEngine directory (by splitting the pwd into two words
# at /userspace and taking the first word only).
# Then include the common defines under CommEngine.
#
CURR_DIR := $(shell pwd)
BUILD_DIR:=$(HND_SRC)

#
# Private apps and libs are allowed to include header files from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#
 
#
# Private apps and libs are allowed to link with libraries from the
# private and public directories.
#
# WARNING: Do not modify this section unless you understand the
# license implications of what you are doing.
#

include $(BUILD_DIR)/make.common

ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(KERNEL_LINKS_DIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ALLOWED_LIB_DIRS := /lib:/lib/private:/lib/public

ifndef CROSS_COMPILE
ifndef TOOLCHAIN_TOP
TOOLCHAIN_TOP = /opt/toolchains/uclibc-crosstools-gcc-4.2.3-3
endif
ifndef TOOLCHAIN
TOOLCHAIN = $(TOOLCHAIN_TOP)/usr
endif
CROSS_COMPILE = $(TOOLCHAIN)/bin/$(TOOLCHAIN_PREFIX)-
endif

LINUX_SHARED_LIBRARY=1

ifneq ($(strip $(BUILD_BRCM_CMS)),)
COMPILE_FLAGS = -DBRCM_CMS_BUILD
endif

ifneq ($(strip $(BCM_JUMBO_FRAME)),)
COMPILE_FLAGS += -DCONFIG_BCM_JUMBO_FRAME
endif

ifneq ($(strip $(BUILD_MCAST_SNOOP)),)
COMPILE_FLAGS += -DCONFIG_BCM_MCAST_SNOOP
endif

ifneq ($(strip $(BRCM_SWITCH_SCHED_SP)),)
COMPILE_FLAGS += -DBCM_SWITCH_SCHED_SP
endif
ifneq ($(strip $(BRCM_SWITCH_SCHED_WRR)),)
COMPILE_FLAGS += -DBCM_SWITCH_SCHED_WRR
endif

ifneq ($(strip $(BRCM_ENET_MULTI_IMP_SUPPORT)),)
COMPILE_FLAGS += -DCONFIG_BCM_ENET_MULTI_IMP_SUPPORT
endif

#This is required due to an error raised by the 4.6 compiler 
#in bmd_phy_probe_default.c
COMPILE_FLAGS += -Wno-address -Wno-sizeof-pointer-memaccess

ifneq ($(strip $(BRCM_DRIVER_MoCA)),)
ifeq ($(strip $(BRCM_DRIVER_GPON)),)
COMPILE_FLAGS += -DBCM_MOCA_NO_GPON
endif
endif

COMPILE_FLAGS += -DCONFIG_EPON_NUM_FE_PORTS=$(EPON_NUM_FE_PORTS)
COMPILE_FLAGS += -DCONFIG_EPON_PORT=$(EPON_PORT)

ifneq ($(strip $(BUILD_EPON_SDK)),)
COMPILE_FLAGS += -DEPON_SDK_BUILD
endif

ifneq ($(strip $(BRCM_PORTS_ON_INT_EXT_SW)),)
COMPILE_FLAGS += -DBRCM_PORTS_ON_INT_EXT_SW
endif

COMPILE_FLAGS +=  -DCHIP_$(BRCM_CHIP)

ifneq ($(strip $(EPON_ONU_TYPE)),)
COMPILE_FLAGS +=   -D$(EPON_ONU_TYPE)
endif

export CDK BMD PHY CROSS_COMPILE ALLOWED_INCLUDE_PATHS ALLOWED_LIB_DIRS CMS_LIB_PATH 
export BUILD_DIR CMS_LIB_RPATH CMS_LOG_FLAGS COMPILE_FLAGS LINUX_SHARED_LIBRARY
export BRCM_EXT_SWITCH_REV
