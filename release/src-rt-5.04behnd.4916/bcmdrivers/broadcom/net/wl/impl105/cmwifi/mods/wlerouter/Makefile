WLEROUTER_LKM_NAME      :=wlerouter
obj-m	+=$(WLEROUTER_LKM_NAME).o

#when build from source, it will got to wlerouter directory
#else it will build directly from root directory

#for header file to be included

SRCBASE = $(WLSRCBASE)
# CMWIFI start
#include ${SRCBASE}/makefiles/cm_linux_modules.mk
#include $(WLCFGDIR)/$(DEFAULT_CONFIG)
WLCFGDIR=${SRCBASE}/wl/config
LINUXDIR_INC_BASE = $(LINUXDIR)/arch/$(ARCH)
include $(WLCFGDIR)/wlconfig_lx_cmwifi_apsta
EXTRA_CFLAGS	+= -I../../../main/components/proto/include

# CMWIFI end
include $(WLCFGDIR)/wl.mk
WLEROUTER_LKM_BASE  :=.

ifneq ($(CMWIFI),)
KBUILD_CFLAGS += -DCMWIFI=1
KBUILD_CFLAGS += -I$(WLSRCBASE)/include
KBUILD_CFLAGS += -I$(WLSRCBASE)/shared/bcmwifi/include
KBUILD_CFLAGS += $(WLAN_ComponentIncPath)
KBUILD_CFLAGS += $(WLAN_StdIncPathA)
KBUILD_CFLAGS += -I$(WLSRCBASE)/common/include
KBUILD_CFLAGS += -DBCMDRIVER -Dlinux
endif

EXTRA_CFLAGS    += -I$(WL_CMWIFI_DIR)/mods/wlerouter/include
EXTRA_CFLAGS    += -I$(WL_CMWIFI_DIR)/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../components/proto/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/include
EXTRA_CFLAGS    += -I$(WLSRCBASE)/../components/math/include/

ifneq ($(CMWIFI),)
  IFLAGS += -I$(LINUXDIR)/include/linux

ifeq ($(call KERNEL_GE,3.8.0),TRUE)
  IFLAGS += -I$(LINUXDIR)/include/uapi
  IFLAGS += -I$(LINUXDIR)/include/generated/uapi
  IFLAGS += -I$(LINUXDIR_INC_BASE)/include/generated/
endif

ifeq ($(strip $(BRCM_CHIP)),3384)
  IFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-bcm3384
else
  IFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-brcmstb
endif
  IFLAGS += -I$(LINUXDIR_INC_BASE)/include/asm/mach-generic
ifneq ($(CMWIFI_EROUTER),)
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/include
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqm
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/dqnet
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/fpm
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/mdqm
ifneq ($(CMWIFI_EROUTER_RFAP),)
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/rdpa
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/rdpa_gpl/include
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/framework
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/bdmf/system/linux
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem
  IFLAGS += -I$(EXTMODDIR)/runner/drivers/rdp_subsystem/BCM3390
  IFLAGS += -I$(EXTMODDIR)/runner/projects/CM3390/drivers/rdd
else ifneq ($(CMWIFI_EROUTER_GFAP),)
  IFLAGS += -I$(LINUXDIR)/drivers/bcm_media_gw/gfap
  IFLAGS += -I$(LINUXDIR)/include/config
endif
ifneq ($(CMWIFI_33940),)
ifneq ($(CMWIFI_RDKB),)
    include $(LINUXBUILDDIR)/.config
else
    include $(LINUXDIR)/.config
endif #CMWIFI_RDKB
ifneq ($(strip $(CONFIG_BCM_DHD_RUNNER)),)
  DFLAGS += -DBCM_DHD_RUNNER
# BCM_DHD_TX_ON_RUNNER and BCM_DHD_RX_ON_RUNNER are also defined in dhd_router.mk
  DFLAGS += -DBCM_DHD_TX_ON_RUNNER
  DFLAGS += -DBCM_DHD_RX_ON_RUNNER
  DFLAGS += -I$(EXTMODDIR)/include/339x/
  DFLAGS += -I$(EXTMODDIR)/api/339x/
endif #CONFIG_BCM_DHD_RUNNER
  DFLAGS += -DCMWIFI_33940 -DWLEROUTER_AS_MODULE
ifeq ($(CMWIFI_RDKB),)
  IFLAGS += -I$(LINUXDIR)/../../../../repos/rbb_gfap_common/include
  IFLAGS += -I$(LINUXDIR)/../../../../repos/rbb_gfap_common/include/339x
  IFLAGS += -I$(LINUXDIR)/../../../../repos/rbb_gfap_common/api/339x
else
  IFLAGS += -I$(EXTMODDIR)/include
  IFLAGS += -I$(EXTMODDIR)/include/339x
  IFLAGS += -I$(EXTMODDIR)/api/339x
endif #CMWIFI_RDKB

endif  # CMWIFI_33940
endif  # WLEROUTER
endif

ifeq ($(CMWIFI_EROUTER),1)
  ifeq ($(BRCM_CHIP),3390)
    DFLAGS += -DCM3390
  endif
endif

EXTRA_CFLAGS += $(WLFLAGS) $(DFLAGS) $(IFLAGS) $(USR_CFLAGS)

ifeq ($(CMWIFI_EROUTER_GFAP),1)
WLEROUTER_FILES :=wl_erouter_gfap.c
endif
ifeq ($(CMWIFI_EROUTER_RFAP),1)
ifeq ($(CM_BUILDROOT),)
WLEROUTER_FILES :=wl_erouter_rfap_legacy.c
else
WLEROUTER_FILES :=wl_erouter_rfap.c
endif
endif
ifeq ($(CMWIFI_33940),1)
WLEROUTER_FILES :=wl_erouter_33940.c
endif

# include wlerouter common utils
WLEROUTER_FILES += wler_utils.c

ifeq ($(CMWIFI_33940),)
# 33940/3392 support pktc but don't support local hot-cache chaining
# code shared so add compile time protection against any unintention use
WLEROUTER_FILES +=wl_erouter_pktc.c
WLEROUTER_FILES += wlan_bcg_cm_linux.c
endif

WLEROUTER_OBJ_FILES := $(WLEROUTER_FILES:.c=.o)
WLEROUTER_BUILD_OBJS := $(addprefix $(WLEROUTER_LKM_BASE)/src/,$(WLEROUTER_OBJ_FILES))

obj-m	+=$(WLEROUTER_LKM_NAME).o
$(WLEROUTER_LKM_NAME)-objs+=$(WLEROUTER_BUILD_OBJS)
