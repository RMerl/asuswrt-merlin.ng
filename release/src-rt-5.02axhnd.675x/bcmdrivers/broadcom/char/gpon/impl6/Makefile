#***********************************************************************
#
#  Copyright (c) 2015  Broadcom Corporation
#  All Rights Reserved
#
# <:label-BRCM:2015:proprietary:standard
# 
#  This program is the proprietary software of Broadcom and/or its
#  licensors, and may only be used, duplicated, modified or distributed pursuant
#  to the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied), right
#  to use, or waiver of any kind with respect to the Software, and Broadcom
#  expressly reserves all rights in and to the Software and all intellectual
#  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
#  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
#  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1. This program, including its structure, sequence and organization,
#     constitutes the valuable trade secrets of Broadcom, and you shall use
#     all reasonable efforts to protect the confidentiality thereof, and to
#     use this information only in connection with your use of Broadcom
#     integrated circuit products.
# 
#  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
#     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
#     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
#     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
#     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
#     PERFORMANCE OF THE SOFTWARE.
# 
#  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
#     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
#     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
#     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
#     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
#     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
#     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
#     LIMITED REMEDY.
# :>
#
#***********************************************************************/

#4.16L.03_NPI PLOAM API version is 7.0
BCM_PLOAM_VERSION_API_MAJOR=7
BCM_PLOAM_VERSION_API_MINOR=0

#4.16L.03_NPI PLOAM driver version is 2.0.0
BCM_PLOAM_VERSION_DRIVER_MAJOR=2
BCM_PLOAM_VERSION_DRIVER_MINOR=0
BCM_PLOAM_VERSION_DRIVER_FIX=0

#4.16L03 OMCI API version is 1.4
BCM_OMCI_VERSION_API_MAJOR=1
BCM_OMCI_VERSION_API_MINOR=4

#4.16L03 OMCI Driver version is 1.4.0
BCM_OMCI_VERSION_DRIVER_MAJOR=1
BCM_OMCI_VERSION_DRIVER_MINOR=4
BCM_OMCI_VERSION_DRIVER_FIX=0

ifeq ("$(CONFIG_BCM96838)" ,"y")
	CONFIG_PLATFORM=oren
endif

obj-$(CONFIG_BCM_GPON) += bcmgpon.o
obj-$(CONFIG_BCM_GPON) += gponstack.o


#
# Temporary until 63158A0 retired
#
ifeq ("$(BRCM_CHIP)","63158")
ifneq ("$(CONFIG_BRCM_CHIP_REV)","0x63158A0")
obj-$(CONFIG_BCM_NGPON) += ngponstack.o
endif
else 
obj-$(CONFIG_BCM_NGPON) += ngponstack.o
endif


BCM_SRC_FILES:=$(shell cd $(src) && find gpon_driver  -type f -name '*.c')
COMMON_SRC_FILES:=$(shell cd $(src) && find stack/common  -type f -name '*.c')

ifneq (,$(filter $(BRCM_CHIP),63158 6846 6856 6878))
    GPON_HAL_IMPL=impl3
else ifeq ("$(BRCM_CHIP)", "6858")
    GPON_HAL_IMPL=impl2
else
    GPON_HAL_IMPL=impl1
endif

ifneq (,$(filter $(BRCM_CHIP),63158 6846 6856 6878))
    GPON_SRC_FILES:=$(shell cd $(src) && find stack/gpon -type f -name '*.c' -not -path "stack/gpon/driver/impl1/*" -not -path "stack/gpon/driver/impl2/*")
else ifeq ("$(BRCM_CHIP)", "6858")
    GPON_SRC_FILES:=$(shell cd $(src) && find stack/gpon -type f -name '*.c' -not -path "stack/gpon/driver/impl1/*" -not -path "stack/gpon/driver/impl3/*" )
else 
    GPON_SRC_FILES:=$(shell cd $(src) && find stack/gpon -type f -name '*.c' -not -path "stack/gpon/driver/impl3/*" -not -path "stack/gpon/driver/impl2/*")
endif

ifneq ($(strip $(CONFIG_BCM_NGPON)),)
    NGPON_SRC_FILES:=$(shell cd $(src) && find stack/ngpon -type f -name '*.c')
endif

bcmgpon-objs += rdpa/rdpa_gpon.o
bcmgpon-objs += $(patsubst %.c, %.o, $(BCM_SRC_FILES)) 

gponstack-objs += $(patsubst %.c, %.o, $(COMMON_SRC_FILES))
gponstack-objs += $(patsubst %.c, %.o, $(GPON_SRC_FILES)) 

ifeq ("$(CONFIG_BCM_NGPON)" ,"m")
    ngponstack-objs += $(patsubst %.c, %.o, $(COMMON_SRC_FILES))
    ngponstack-objs += $(patsubst %.c, %.o, $(NGPON_SRC_FILES)) 
endif

ifeq ($(CONFIG_BCM_BUILD_NGPON_AMD2),y)
    EXTRA_CFLAGS += -DG989_3_AMD2
endif

ifeq ("$(CONFIG_64BIT)" ,"y")
	EXTRA_CFLAGS += -DPHYS_ADDR_64BIT
endif

EXTRA_CFLAGS += -DUSE_BDMF_SHELL
EXTRA_CFLAGS += -Wno-error=pointer-to-int-cast -Wno-error=int-to-pointer-cast
EXTRA_CFLAGS += -I$(src)/stack/include
EXTRA_CFLAGS += -I$(src)/stack/gpon/pwm
EXTRA_CFLAGS += -I$(src)/stack/common/driver -I$(src)/stack/gpon/driver

EXTRA_CFLAGS += -I$(src)/stack/ngpon/driver -I$(src)/stack/common/crypto -I$(src)/ngpon/state_machine/stack_shell
EXTRA_CFLAGS += -I$(src)/stack/ngpon/driver/impl2/ -I$(src)/stack/ngpon/driver/impl2/ru_gen -I$(src)/stack/ngpon/driver/impl2/hal

EXTRA_CFLAGS += -I$(src)/stack/gpon/driver/$(GPON_HAL_IMPL)/ -I$(src)/stack/gpon/driver/$(GPON_HAL_IMPL)/ru_gen -I$(src)/stack/gpon/driver/$(GPON_HAL_IMPL)/hal
EXTRA_CFLAGS += -I$(src)/stack/gpon/state_machine -I$(src)/gpon/state_machine/stack_shell
EXTRA_CFLAGS += -I$(src)/stack/common/logger -I$(src)/stack/common/logger_drv/include
EXTRA_CFLAGS += -I$(src)/stack/common/rogue_onu -I$(src)/stack/common/tod -I$(src)/stack/common/bw_mon
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/rdpa_gpl_ext/impl1/include/gpon_stack
EXTRA_CFLAGS += -I$(src)/rdpa
EXTRA_CFLAGS += -I$(src)/rdpa
EXTRA_CFLAGS += $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -DBCM_PLOAM_VERSION_API_MAJOR=$(BCM_PLOAM_VERSION_API_MAJOR) 
EXTRA_CFLAGS += -DBCM_PLOAM_VERSION_API_MINOR=$(BCM_PLOAM_VERSION_API_MINOR)
EXTRA_CFLAGS += -DBCM_PLOAM_VERSION_DRIVER_MAJOR=$(BCM_PLOAM_VERSION_DRIVER_MAJOR)
EXTRA_CFLAGS += -DBCM_PLOAM_VERSION_DRIVER_MINOR=$(BCM_PLOAM_VERSION_DRIVER_MINOR)
EXTRA_CFLAGS += -DBCM_PLOAM_VERSION_DRIVER_FIX=$(BCM_PLOAM_VERSION_DRIVER_FIX)
EXTRA_CFLAGS += -DBCM_OMCI_VERSION_API_MAJOR=$(BCM_OMCI_VERSION_API_MAJOR) 
EXTRA_CFLAGS += -DBCM_OMCI_VERSION_API_MINOR=$(BCM_OMCI_VERSION_API_MINOR)
EXTRA_CFLAGS += -DBCM_OMCI_VERSION_DRIVER_MAJOR=$(BCM_OMCI_VERSION_DRIVER_MAJOR)
EXTRA_CFLAGS += -DBCM_OMCI_VERSION_DRIVER_MINOR=$(BCM_OMCI_VERSION_DRIVER_MINOR)
EXTRA_CFLAGS += -DBCM_OMCI_VERSION_DRIVER_FIX=$(BCM_OMCI_VERSION_DRIVER_FIX)
EXTRA_CFLAGS += -DBCM_INCLUDE_SWITCH_DEPS
EXTRA_CFLAGS += -DBCM_GPON_USE_NVRAM
EXTRA_CFLAGS += -DINCLUDE_LOGS -DUSE_LOGGER -DRU_INCLUDE_FIELD_DB
EXTRA_CFLAGS += -DPLL_PPM_ADJ
# EXTRA_CFLAGS += -DSUPPORT_TO6 

# Uncomment in order to enable dba statistic
#EXTRA_CFLAGS += -DDBA_DEBUG_STATISTICS 

EXTRA_CFLAGS += -DBDMF_DRIVER_GPL_LAYER

# Uncomment to support OMCI extended features 
# EXTRA_CFLAGS += -DBCM_OMCI_TEST
# EXTRA_CFLAGS += -DGPON_OMCI_DUMP_DATA
# EXTRA_CFLAGS += -DGPON_OMCI_DUMP_TRACE

-include $(TOPDIR)/Rules.make

clean:
	$(Q)find . -type f -regextype posix-egrep -regex ".*\.(o|a|s|cmd|ko|.builtin|mod\.c|order)$$" -exec rm -f {} \;

# Makefile trick: breaking the following into two rules allows
# the "%.o : %.c" rule (defined earlier by kbuild), to take precidence
%.o : %.o_tmp
	$(Q)mv $< $@

%.o_tmp : %.$(BCMARCH).o_saved
	@echo "Using $<"
	$(Q)cp $< $@
