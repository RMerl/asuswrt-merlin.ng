#
# Generic portion of the Broadcom wl NIC driver makefile
#
# input: O_TARGET, CONFIG_WL_CONF and wl_suffix
# output: obj-m, obj-y
#
# Copyright (C) 2025, Broadcom. All Rights Reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#
# <<Broadcom-WL-IPTag/Dual:>>
#
# $Id$
#

# Example values of Makefile variables used in this Makefile:
# src:               ../../bcmdrivers/broadcom/net/wl/bcm963178/main/components/router/hnd_wl/
# SRCBASE_OFFSET:    ../../../../main/src
# ROUTERBASE_OFFSET: ../../router
# WLCONF_O:          wlconf.o
# obj                ../../bcmdrivers/broadcom/net/wl/bcm963178/main/components/router/hnd_wl/
# TARGET             wl

# The CWD is the linux kernel root dir, e.g. kernel/linux-4.19

wl_generic_mkfile_dir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

# abspath removes any '..' in the path as well
#WLSRC_BASE := $(abspath $(src)/$(SRCBASE_OFFSET))
#ROUTER_BASE := $(abspath $(src)/$(ROUTERBASE_OFFSET))
WLSRC_BASE := $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src
ROUTER_BASE := $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/router
CLM_FILE_SUFFIX := _nic
CLM_FILE_SUFFIX_NIC = _nic

ifeq ($(PREBUILT_EXTRAMOD),1)
REBUILD_WL_MODULE=0
else
REBUILD_WL_MODULE=$(shell if [ -f "$(WLSRC_BASE)/wl/sys/wlc.c" -a "$(REUSE_PREBUILT_WL)" != "1" ]; then echo 1; else echo 0; fi)
endif

$(info "Rebuild WL Module: $(REBUILD_WL_MODULE) ----")
# If source directory (src/wl/sys) exists and REUSE_PREBUILT_WL is undefined,
# then build inside $(SRCBASE_OFFSET)/wl/sys, otherwise use pre-builts
ifeq ($(REBUILD_WL_MODULE),1)

    # Get the source files and flags from the specified config file
    # (Remove config's string quotes before trying to use the file)
    ifeq ($(CONFIG_WL_CONF),)
        $(error var_vlist($(VLIST)) var_config_wl_use($(shell env|grep CONFIG_WL_USE)))
        $(error CONFIG_WL_CONF is undefined)
    endif

    WLCONFFILE := $(strip $(subst ",,$(CONFIG_WL_CONF)))
    WLCFGDIR   := $(WLSRC_BASE)/wl/config

    # include $(WLSRC_BASE)/makefiles/WLAN_Common.mk

    # include router config to source MFP, HSPOT & WNM settings
    include $(ROUTER_BASE)/.config

    KBUILD_CFLAGS += -I../../router-sysdep/bcmdrv/include
    KBUILD_CFLAGS += -DBCMDRIVER -Dlinux
    KBUILD_CFLAGS += -DBCA_HNDROUTER
    KBUILD_CFLAGS += -DBCM_ROUTER
ifneq (,$(filter $(MODEL),RTAX58U TUFAX3000 RTAX82U RTAX82_XD6 RTAX55 RTAX1800))
    KBUILD_CFLAGS += -g -DBCMDBG -DWLMSG_ASSOC
endif
ifeq ($(CMWIFI),)
    KBUILD_CFLAGS += -Wno-error=date-time
else ifneq ($(CMWIFI_RDKB),)
    # RDK toolchain supports date-time, but OpenBFC stbgcc-4.8-1.5 does not.
    KBUILD_CFLAGS += -Wno-error=date-time
endif

ifneq ($(CMWIFI),)
# BCMINTERNAL and BCMDBG are getting enabled by default. Disabling here till actual fix is found.
    NO_BCMINTERNAL := 1
    NO_BCMDBG := 1
endif

    # define OS flag to pick up wl osl file from wl.mk
    WLLX=1
    ifdef RTCONFIG_DPSTA
        DPSTA=1
    endif
    ifdef CONFIG_CR4_OFFLOAD
        WLOFFLD=1
    endif
    ifdef RTCONFIG_BCMARM
        MFP=1
    endif
    ifdef RTCONFIG_BRCM_HOSTAPD
        WL_AP_CFG80211=1
        WL_SAE=1
    endif
    include $(WLCFGDIR)/$(WLCONFFILE)

    # Disable ROUTER_COMA in ARM router for now.
    ROUTER_COMA=0

#ifdef BCMOTP
    # define the BCMOTP_SHARED_SYMBOLS_USE flag to NOT pick up the bcmotp.c file from wl.mk
    # for just adding BCMNVRAMR and BCMNVRAMW to the compile flag
    ifeq ($(BCMOTP),1)
	BCMOTP_SHARED_SYMBOLS_USE=1
    endif
#endif

    ifeq ($(WLAUTOD11SHM),1)
        # Include makefile to build d11 shm files
        D11SHM_SRCBASE := $(src)/$(SRCBASE_OFFSET)
        # match the optimized relative path in WL_OBJS for d11shm.c file.
        D11SHM_TEMPDIR := $(src)/../../../src/wl
        D11SHM_TEMPDIR2 := $(abspath $(HND_SRC))/bcmdrivers/broadcom/net/wl/bcm9$(BRCM_CHIP)/main/src
        EXTRA_CFLAGS += -I$(D11SHM_TEMPDIR2)
        IFLAGS += -I$(D11SHM_TEMPDIR)
        D11SHM_IFLAGS := $(IFLAGS)
        ifneq ($(CMWIFI),)
          D11SHM_IFLAGS += -I$(src)
        endif
        D11SHM_CFLAGS := $(DFLAGS) $(IFLAGS) $(WFLAGS)
        D11SHM_CFGFILE := $(D11SHM_SRCBASE)/wl/sys/wlc_cfg.h
        D11SHMCDIR := src/wl
        include $(D11SHM_SRCBASE)/makefiles/d11shm.mk
        D11SHM_TARGET := $(D11SHM_HEADER)
	D11SHM_CFLAGS += $(CMWIFI_RDK_CFLAGS)
    endif

    ifeq ($(WLAUTOD11REGS),1)
        # Include makefile to build d11 REGS files
        AUTOREGS_SRCBASE := $(abspath $(SRCBASE)/../../main/src)
        # match the optimized relative path in WL_OBJS for autoreg*.c files.
        AUTOREGS_TEMPDIR := $(src)/../../../src/wl
        EXTRA_CFLAGS += -I$(AUTOREGS_TEMPDIR)
        IFLAGS += -I$(AUTOREGS_TEMPDIR)
        AUTOREGS_IFLAGS := $(IFLAGS)
        ifneq ($(CMWIFI),)
          AUTOREGS_IFLAGS += -I$(src)
        endif
        AUTOREGS_CFLAGS := $(DFLAGS) $(IFLAGS) $(WFLAGS)
        D11REGS_CFGFILE := $(AUTOREGS_SRCBASE)/wl/sys/wlc_cfg.h
        AUTOREGSCDIR := src/wl
        include $(AUTOREGS_SRCBASE)/makefiles/autoregs.mk
	AUTOREGS_TARGET := $(AUTOREGS_TEMPDIR)/d11regsoffs.h
	AUTOREGS_CFLAGS += $(CMWIFI_RDK_CFLAGS)
    endif

    include $(WLCFGDIR)/wl.mk

    WLAN_ComponentsInUse := accel avs awd bcmwifi bcmcrypto ppr olpc keymgmt iocv dump hal phy msch chctx math
    WLAN_ComponentsInUse += phymods  # old name of phy, preserved for compatibility
    ifeq ($(WLCLMAPI),1)
        WLAN_ComponentsInUse += clm clm-api
    endif
    ifeq ($(WL_MBO),1)
      ifeq ($(MBO_AP),1)
        WLAN_ComponentsInUse += encode gas mbo_oce
      endif
    endif
    ifeq ($(WL_OCE),1)
      ifeq ($(OCE_AP),1)
        WLAN_ComponentsInUse += mbo_oce
      endif
    endif

    ifneq ($(CONFIG_AIRIQ),)
        WLAN_ComponentsInUse += airiq airiq_drv
    endif

    ifeq ($(FTM),1)
        WLAN_ComponentsInUse += ftm
        WLAN_ComponentsInUse += pasn
    endif
    # For trunk driver, we need the accel component. This will be a
    # no-op while compiling drivers from other older branches.
    # WLAN_ComponentsInUse += accel

    include $(src)/$(SRCBASE_OFFSET)/makefiles/WLAN_Common.mk

    EXTRA_CFLAGS += $(WLAN_ComponentIncPath)
    EXTRA_CFLAGS += $(WLAN_StdIncPathA)

    ifeq ($(WLFILES_SRC),)
      $(error WLFILES_SRC is undefined in $(WLCFGDIR)/$(WLCONFFILE))
    endif

    ifeq ($(WLCLMAPI),1)
      ifeq ($(WLCLMLOAD),1)
        CLM_TYPE ?= min
      else
        CLM_TYPE ?= router
      endif
    # wlc_clm_data_nic.o, wlc_clm_data_nic.c paths should match with the path in
    # WL_OBJS so optimizing the path for clm.
    # If the /bin/realpath utility is available, use it for simplicity.
    # If not, fall back to internal custom makefile code.
    optimized_clm_path := $(call .relpath,$(wl_generic_mkfile_dir), \
      $(wl_generic_mkfile_dir)$(SRCBASE_OFFSET)/../components/clm-api/src, \
      $(if $(wildcard /bin/realpath),1),-m)
    # $(call WLAN_GenClmCompilerRule,$(src)/$(optimized_clm_path),$(src)/$(SRCBASE_OFFSET))
    $(call WLAN_GenClmCompilerRule,$(abspath $(src))/main_components/clm-api/src/,$(abspath $(src)/main_src/))
    endif

    # need -I. to pick up wlconf.h in build directory

    EXTRA_CFLAGS    += -DWL_ALL_PASSIVE

    ifeq ($(CONFIG_CACHE_L310),y)
    EXTRA_CFLAGS    += -DWL_PL310_WAR
    endif

    EXTRA_CFLAGS += -DDMA $(WLFLAGS)
ifeq ($(CMWIFI),)
    EXTRA_CFLAGS += -Werror
else
    EXTRA_CFLAGS += -Wno-error
endif
    EXTRA_CFLAGS += -I$(src) -I$(src)/.. -I$(WLSRC_BASE)/wl/linux \
		    -I$(WLSRC_BASE)/wl/sys -I$(WLSRC_BASE)/wl/dot1as/include \
		    -I$(WLSRC_BASE)/wl/dot1as/src -I$(WLSRC_BASE)/wl/proxd/include

    EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
    EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)
    EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ifneq ($(strip $(BUILD_HND_NIC)),)
    # BCA_HNDNIC to be used to differentiate between FD and NIC mode operation
    # on BCA_HNDROUTER platforms
    EXTRA_CFLAGS += -DBCA_HNDNIC
endif
ifneq ($(strip $(BUILD_OPENWRT_NATIVE)),)
    # For native OpenWrt we need stricter cfg80211 API compliance
    EXTRA_CFLAGS += -DWL_CFG80211_STRICT
else ifeq ($(BUILD_PRPL_FEEDS),y)
    EXTRA_CFLAGS += -DWL_CFG80211_STRICT -DPRPL_FEEDS
else ifeq ($(RDKB_ONE_WIFI),y)
    EXTRA_CFLAGS += -DWL_CFG80211_STRICT
endif
ifeq ($(RDKB_ONE_WIFI),y)
    EXTRA_CFLAGS += -DRDKB_ONE_WIFI
endif

    ifneq ("$(CONFIG_CC_OPTIMIZE_FOR_SIZE)","y")
      EXTRA_CFLAGS += -finline-limit=2048
    endif

    ifdef RTCONFIG_DPSTA
ifneq ($(wildcard ../../router-sysdep/dpsta),)
    DPSTASRC := ../dpsta
else
    DPSTASRC := ../dpsta
endif
    # include path for dpsta.h
    EXTRA_CFLAGS += -I$(src)/$(DPSTASRC)
    endif

    # Build the phy source files iff -DPHY_HAL is present, handling either old (src) or new (components) phy location.
    ifneq ($(findstring PHY_HAL,$(WLFLAGS)),)
        PHYLOC := $(firstword $(wildcard $(src)/$(SRCBASE_OFFSET)/../components/phy/old $(src)/$(SRCBASE_OFFSET)/wl/phy))
        EXTRA_CFLAGS += -I$(PHYLOC)
    else
	WLFILES_SRC := $(filter-out src/wl/phy/%,$(WLFILES_SRC))
	WLFILES_SRC := $(filter-out components/phy/%,$(WLFILES_SRC))
    endif

#ifdef WLTEST
	ifeq ($(WLTEST),1)
		EXTRA_CFLAGS += -DBCMDBG_PHYDUMP
	endif
#endif

    # allow C99/C11 extensions such as variable declaration in for loop
    EXTRA_CFLAGS += -std=gnu99
    # relax the warning to allow the mixed mode declaration and code
    EXTRA_CFLAGS += -Wno-declaration-after-statement

    # wl shared directory
    ifeq ($(strip $(USE_WLAN_SHARED)), 1)
        ifneq ($(strip $(CONFIG_BCM_WIFI_FORWARDING_DRV)),)
            WLWFD := 1
        endif
        ifneq ($(strip $(CONFIG_BCM_HND_EAP)),)
            WLEAPBLD := 1
        endif
        ifneq ($(strip $(CONFIG_BCM_ARCHER_WLAN)),)
            # Archer WLAN (implements WiFi Forwarding Driver)
            WLWFD := 1
            EXTRA_CFLAGS += -DBCM_AWL
            WLFILES_SRC += ../../shared/impl1/wl_awl.c
            # WLFILES_SRC += ../bcmdrivers/broadcom/net/wl/shared/impl1/wl_awl.c
        endif

        # Settings that apply only to WFD
        ifeq ($(strip $(WLWFD)), 1)
		EXTRA_CFLAGS += -DBCM_WFD
		ifneq ($(strip $(CONFIG_BCM_CROSSBOW)),)
			EXTRA_CFLAGS += -DBCM_WLHDR
		endif

		# Enable Fcache based WFD for 47189
		ifeq ($(BRCM_CHIP),47189)
			ifneq ($(strip $(BCA_CPEROUTER)),)
				EXTRA_CFLAGS += -DCONFIG_BCM_FC_BASED_WFD
			endif
		endif

		WLFILES_SRC += ../../shared/impl1/wl_wfd.c
	endif

	# WFD or EAP, but one build currently build, enable and run
	# PKTFWD while not really using it. There is no way to detect that in a
	# Makefile. Will be fixed later when that one build enables WFD.
        ifneq ($(filter 1,$(WLWFD) $(WLEAPBLD)),)
		ifneq ($(strip $(CONFIG_BCM_PKTFWD)),)
			EXTRA_CFLAGS += -DBCM_PKTFWD
			ifneq ($(BUILD_HND_EAP_AP1),y)
				EXTRA_CFLAGS += -DWL_PKTQUEUE_RXCHAIN
			endif
			EXTRA_CFLAGS += -DWL_PKTFWD_INTRABSS

			# Enable credit based Host Flow Control
			ifneq ($(strip $(CONFIG_BCM_PKTFWD_FLCTL)),)
				EXTRA_CFLAGS += -DBCM_PKTFWD_FLCTL
			endif

			WLFILES_SRC += ../../shared/impl1/wl_pktfwd.c
		else
			WLFILES_SRC += ../../shared/impl1/wl_pktc.c
		endif

		ifneq ($(strip $(CONFIG_BCM_EAPFWD)),)
			EXTRA_CFLAGS += -DBCM_EAPFWD
		endif
		EXTRA_CFLAGS += -I$(WLSRC_BASE)/../../../shared/impl1
		ifneq ($(BUILD_HND_EAP_AP1),y)
			EXTRA_CFLAGS += -DPKTC -DPKTC_TBL
		endif

		 WLFILES_SRC += ../../shared/impl1/wl_thread.c
	endif

	ifneq ($(strip $(CONFIG_BLOG)),)
          EXTRA_CFLAGS += -DBCM_BLOG
          WLFILES_SRC += ../../shared/impl1/wl_blog.c
	endif

	ifneq ($(strip $(CONFIG_BCM_BPM_BULK_FREE)),)
	  EXTRA_CFLAGS += -DBPM_BULK_FREE
	endif
    endif

    #Enable to use PCIe MSI interrupts if enabled by BSP
    ifeq ($(CONFIG_PCI_MSI),y)
        EXTRA_CFLAGS += -DBCM_WLAN_PCIE_MSI
    endif

    ifneq ($(strip $(CONFIG_BCM_WLAN_64BITPHYSADDR)),)
        EXTRA_CFLAGS += -DBCMDMA64OSL
    endif

    ifneq ($(strip $(CONFIG_BCM_KF_NBUFF)),)
	EXTRA_CFLAGS += -DBCM_NBUFF -DBCM_NBUFF_PKT
	WLFILES_SRC += ../../shared/impl1/wl_nbuff.c
    endif

    # packet chaining 16-bit chain index support
    ifneq ($(strip $(CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT)),)
        EXTRA_CFLAGS += -DCONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
    endif

    ifneq ($(strip $(BCA_CPEROUTER)),)
    # speed service
    # Always compile in due to binary compatibility issue
        WLFILES_SRC += ../../shared/impl1/wl_spdsvc.c

        ifneq ($(strip $(CONFIG_BCM_WLAN_NIC_RX_RNR_ACCEL)),)
          EXTRA_CFLAGS += -DMC_RX_LOOPBACK_ENABLED
        endif

    endif

    ifeq ($(CMWIFI),)
        EXTRA_CFLAGS += -DBULK_PKTLIST
    endif

    # broadcom serial LED Controller, used when WLLED is defined
    ifneq ($(strip $(CONFIG_BCM_WLCLED)),)
        EXTRA_CFLAGS += -DWLLED_CLED
    endif

    ifeq ($(BAS_WLHAL),y)
      include $(abspath $(WLSRC_BASE)/../components/bas_hal/wlhal/src/bas_wlhal_wl_router.mk)
    endif

ifneq ($(BCM_SANITIZE),)
    # these flags apply only to wl.ko when we build with BCM_SANITIZE.
    EXTRA_CFLAGS += -fsanitize=address -Wframe-larger-than=8192 -Wno-builtin-declaration-mismatch
    EXTRA_CFLAGS += --param asan-instrumentation-with-call-threshold=0 --param asan-stack=0
    EXTRA_CFLAGS += --param asan-instrument-allocas=0 --param asan-globals=0
endif

    # Save the $(EXTRA_CFLAGS) into a file and use gcc's @file option to
    # include them for C compilation to avoid "execvp Argument too long".
    wlflags := \
      $(sort $(filter -D%,$(EXTRA_CFLAGS))) \
      $(filter-out @% -D% -I%,$(EXTRA_CFLAGS)) \
      $(filter @%,$(EXTRA_CFLAGS)) \
      $(addprefix -I,$(abspath $(patsubst -I%,%,$(filter -I%,$(EXTRA_CFLAGS)))))
    $(info ### EXTRA_CFLAGS is $(sort $(filter-out -I%,$(wlflags))))

    ifneq ($(filter-out 3.%,$(MAKE_VERSION)),)
      $(file > $(obj)/wlflags)
      $(foreach wlflag,$(wlflags),$(file >> $(obj)/wlflags,$(wlflag)))
    else
      $(shell echo $(wlflags) > $(obj)/wlflags)
    endif

    # Help the "cmd-check" macro of the kernel to detect command line changes by
    # changing the command line when the contents of wlflags file has changed.
    # We do this by hashing wlflags and creating a dummy definition with the
    # hash.
    EXTRA_CFLAGS := @$(obj)/wlflags -D__WLFLAGS_SHA1_$(firstword $(shell sha1sum $(obj)/wlflags))

    # The paths in WLFILES_SRC need a bit of adjustment.
    # WLFILES_SRC1 -> ../components/avs/src/avs.c ../../../shared/impl1/wl_spdsvc.c
    WLFILES_SRC := $(patsubst ../../shared/%,../bcmdrivers/broadcom/net/wl/shared/%,$(WLFILES_SRC))
    WLFILES_SRC := $(patsubst components/%,../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/components/%,$(WLFILES_SRC))
    WLFILES_SRC := $(patsubst src/%,../bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/%,$(WLFILES_SRC))
    WLFILES_SRC1 := $(addprefix ../,$(filter-out src/%,$(WLFILES_SRC)))

    # WLFILES_SRC2 ->  shared/bcm_app_utils.c wl/sys/wldev_common.c wl/keymgmt/src/km_hw_dump.c
    WLFILES_SRC2 := $(patsubst src/%,%,$(filter src/%,$(WLFILES_SRC)))

    # WL_OBJS_T ->
    # $(wl_generic_mkfile_dir)../../../../main/src/shared/bcm_app_utils.o \
    # $(wl_generic_mkfile_dir)../../../../main/src/wl/sys/wldev_common.o \
    # $(wl_generic_mkfile_dir)../../../../main/src/../components/avs/src/avs.o \
    # $(wl_generic_mkfile_dir)../../../../main/src/../../../shared/impl1/wl_spdsvc.o \
    # $(wl_generic_mkfile_dir)../../../../main/src/../components/phy/cmn/radar/phy_radar_utils.o
    # WL_OBJS_T := $(patsubst %.c,%.o,$(addprefix $(wl_generic_mkfile_dir)$(SRCBASE_OFFSET)/,$(WLFILES_SRC1) $(WLFILES_SRC2)))
    WL_OBJS_T := $(patsubst %.c,%.o,$(WLFILES_SRC1) $(WLFILES_SRC2))

    # WL_OBJS ->  ../../avs/src/avs.o ../../phy/cmn/radar/phy_radar_utils.o \
    # ../../../../../shared/impl1/wl_spdsvc.o ../../../src/shared/bcm_app_utils.o \
    # ../../../src/wl/keymgmt/src/km_hw_dump.o ../../../src/wl/sys/wldev_common.o
    # If the /bin/realpath utility is available, use it for simplicity.
    # If not, fall back to internal custom makefile code.
    # WL_OBJS := $(sort $(call .relpath,$(wl_generic_mkfile_dir),$(WL_OBJS_T), \
     #  $(if $(wildcard /bin/realpath),1),-m))
	# pwd is build/
    WL_OBJS := $(sort $(patsubst ../../../shared/%,../bcmdrivers/broadcom/net/wl/shared/%,$(WL_OBJS_T)))

    # wl-objs is for linking to wl.o
    $(TARGET)-objs := $(WLCONF_O) $(WL_OBJS)
    obj-$(CONFIG_BCM_WLAN) := $(TARGET).o

else # SRCBASE/wl/sys doesn't exist

    # Otherwise, assume prebuilt object module(s) in src/wl/linux directory
    ifneq ($(wl_suffix),)
	prebuilt := wl_$(wl_suffix).o
    else
	prebuilt := wl.o
    endif
    $(TARGET)-objs := prebuilt/$(prebuilt)
    obj-$(CONFIG_BCM_WLAN) := $(TARGET).o

endif

#WL_CONF_H: wlconf.h

UPDATESH   := $(WLCFGDIR)/diffupdate.sh

ifneq (,$(filter "y","$(CONFIG_BCM947622)" "$(CONFIG_BCM963178)"))
	ifeq ($(BUILD_HND_EAP),y)
		WLTUNEFILE := wltunable_lx_47622_eap.h
	else
		WLTUNEFILE := wltunable_lx_63178.h
	endif
else ifneq (,$(filter "y","$(CONFIG_BCM96756)" "$(CONFIG_BCM96855)"))
	WLTUNEFILE := wltunable_lx_6756.h
else ifneq (,$(filter "y","$(CONFIG_BCM947189)" "$(CONFIG_BCM953573)"))
	WLTUNEFILE := wltunable_lx_47189.h
else ifneq (,$(filter "y","$(CONFIG_BCM96878)"))
	WLTUNEFILE := wltunable_lx_6878.h
else ifneq (,$(filter "y","$(CONFIG_BCM96765)"))
	WLTUNEFILE := wltunable_lx_6765.h
else ifneq (,$(filter "y","$(CONFIG_BCM96766)"))
	WLTUNEFILE := wltunable_lx_6766.h
else ifneq (,$(filter "y","$(CONFIG_BCM96764)"))
	WLTUNEFILE := wltunable_lx_6764.h
else
	WLTUNEFILE := wltunable_lx_router.h
endif

$(obj)/$(WLCONF_O): $(obj)/$(WLCONF_H) $(D11SHM_TARGET) $(AUTOREGS_TARGET) FORCE

ifneq ($(D11SHM_TARGET),)
$(D11SHM_TARGET): $(obj)/$(WLCONF_H)
endif

ifneq ($(AUTOREGS_TARGET),)
$(AUTOREGS_TARGET): $(obj)/$(WLCONF_H)
endif

$(addprefix $(obj)/, $(WL_OBJS)): $(obj)/$(WLCONF_H) $(D11SHM_TARGET) $(AUTOREGS_TARGET)

ifneq ($(WL_PHYLIB),)
PHYLIB_OBJS = $(addprefix ../../../,$(sort $(patsubst %.c,%.o,$(PHY_SRC))))
$(addprefix $(obj)/,$(PHYLIB_OBJS)): $(obj)/$(WLCONF_H) $(D11SHM_TARGET) $(AUTOREGS_TARGET)
lib-y += $(PHYLIB_OBJS)
$(TARGET)-objs += lib.a
endif

$(WLCONF_H): $(obj)/$(WLCONF_H)
	cp -a $< $@

$(obj)/$(WLCONF_H): $(WLCFGDIR)/$(WLTUNEFILE) FORCE
	[ ! -f $@ ] || chmod +w $@
	@echo "check and update config file"
	@echo $(if $(VLIST),"VLIST          = $(VLIST)")
	@echo "CONFIG_WL_CONF = $(CONFIG_WL_CONF)"
	@echo "WLTUNEFILE     = $(WLTUNEFILE)"
	cp $< wltemp
	$(UPDATESH) wltemp $@

FORCE:

clean-files += $(SRCBASE_OFFSET)/wl/sys/*.o $(SRCBASE_OFFSET)/../components/phy/old/*.o $(SRCBASE_OFFSET)/wl/phy/*.o $(SRCBASE_OFFSET)/wl/ppr/src/*.o $(SRCBASE_OFFSET)/wl/sys/.*.*.cmd $(SRCBASE_OFFSET)/../components/phy/old/.*.*.cmd $(SRCBASE_OFFSET)/wl/phy/.*.*.cmd $(SRCBASE_OFFSET)/bcmcrypto/.*.*.cmd $(SRCBASE_OFFSET)/wl/clm/src/*.o $(SRCBASE_OFFSET)/wl/clm/src/.*.*.cmd $(SRCBASE_OFFSET)/shared/bcmwifi/src/*.o $(SRCBASE_OFFSET)/shared/bcmwifi/src/.*.*.cmd ./$(WLCONF_H) $(WLCONF_O) ./$(D11SHM_TEMPDIR)/d11regs* ./$(D11SHM_TEMPDIR)/d11shm* ./autoregs*

# This comment block must stay at the bottom of the file.
# Local Variables:
# mode: makefile-gmake
# fill-column: 80
# End:
#
# vim: filetype=make sw=2 tw=80 cc=+1 noet
