ifeq ($(SRCBASE),)
	# ..../src/router/
	# (directory of the last (this) makefile)
	export TOP := $(shell cd $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))) && pwd)

	# ..../src/
	export SRCBASE := $(shell (cd $(TOP)/.. && pwd))
	export SRCBASEDIR := $(shell (cd $(TOP)/.. && pwd | sed 's/.*release\///g'))
else
	export TOP := $(SRCBASE)/router
endif

ifneq ($(wildcard $(TOP)-sysdep/),)
export TOP_PLATFORM := $(TOP)-sysdep
else
export TOP_PLATFORM := $(TOP)
endif

-include $(TOP)/.config
-include $(SRCBASE)/.config
-include $(SRCBASE)/profile.mak
include $(SRCBASE)/target.mak
include $(SRCBASE)/platform.mak

ifeq ($(PLATFORM_ARCH),)
export PLATFORM_ARCH := $(PLATFORM)
endif

ifeq ($(or $(PLATFORM_ARCH),$(CROSS_COMPILE),$(CONFIGURE),$(ARCH),$(HOST)),)
$(error Define Platform-specific definitions in platform.mak)
endif

export HOSTCC := gcc

export PLT := $(ARCH)
ifeq ($(RSDK_TOOLCHAIN),)
export TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/.. && pwd -P)
else
export TOOLCHAIN := $(RSDK_TOOLCHAIN)/
endif

export CC := $(CROSS_COMPILE)gcc
export GCC := $(CROSS_COMPILE)gcc
export CXX := $(CROSS_COMPILE)g++
export AR := $(CROSS_COMPILE)ar
export AS := $(CROSS_COMPILE)as
ifeq ($(HND_ROUTER),y)
export LD := $(CC)
else
export LD := $(CROSS_COMPILE)ld
endif
export NM := $(CROSS_COMPILE)nm
export OBJCOPY := $(CROSS_COMPILE)objcopy
export OBJDUMP := $(CROSS_COMPILE)objdump
export RANLIB := $(CROSS_COMPILE)ranlib
export READELF ?= $(CROSS_COMPILE)readelf
export STRIPX := $(CROSS_COMPILE)strip -x
export KSTRIPX := $(patsubst %-gcc,%-,$(KERNELCC))strip -x
ifeq ($(RTCONFIG_BCMARM),y)
export STRIP := $(CROSS_COMPILE)strip
else
export STRIP := $(CROSS_COMPILE)strip -R .note -R .comment
endif
export SIZE := $(CROSS_COMPILE)size

# Determine kernel version
SCMD=sed -e 's,[^=]*=[        ]*\([^  ]*\).*,\1,'
KVERSION:=	$(shell grep '^VERSION[ 	]*=' $(LINUXDIR)/Makefile|$(SCMD))
KPATCHLEVEL:=	$(shell grep '^PATCHLEVEL[ 	]*=' $(LINUXDIR)/Makefile|$(SCMD))
KSUBLEVEL:=	$(shell grep '^SUBLEVEL[ 	]*=' $(LINUXDIR)/Makefile|$(SCMD))
KEXTRAVERSION:=	$(shell grep '^EXTRAVERSION[ 	]*=' $(LINUXDIR)/Makefile|$(SCMD))
LINUX_KERNEL=$(KVERSION).$(KPATCHLEVEL).$(KSUBLEVEL)$(KEXTRAVERSION)
LINUX_KERNEL_VERSION=$(shell expr $(KVERSION) \* 65536 + $(KPATCHLEVEL) \* 256 + $(KSUBLEVEL))
ifeq ($(LINUX_KERNEL),)
$(error Empty LINUX_KERNEL variable)
endif
export LINUX_KERNEL
export LINUX_KERNEL_VERSION


include $(SRCBASE)/target.mak

export LIBDIR := $(TOOLCHAIN)/lib
export USRLIBDIR := $(TOOLCHAIN)/usr/lib

export PLATFORMDIR := $(TOP)/$(PLATFORM_ARCH)
ifeq ($(HND_ROUTER),y)
export INSTALLDIR := $(INSTALL_DIR)
export TARGETDIR := $(INSTALL_DIR)
else
export INSTALLDIR := $(PLATFORMDIR)/install
export TARGETDIR := $(PLATFORMDIR)/target
endif
export STAGEDIR := $(PLATFORMDIR)/stage
export PKG_CONFIG_SYSROOT_DIR := $(STAGEDIR)
export PKG_CONFIG_PATH := $(STAGEDIR)/usr/lib/pkgconfig:$(STAGEDIR)/etc/lib/pkgconfig
export PKG_CONFIG_LIBDIR := $(STAGEDIR)/usr/lib/pkgconfig:$(STAGEDIR)/usr/local/lib/pkgconfig:$(STAGEDIR)/etc/lib/pkgconfig

export EXTRACFLAGS += -DLINUX_KERNEL_VERSION=$(LINUX_KERNEL_VERSION) $(if $(STAGING_DIR),--sysroot=$(STAGING_DIR))
export EXTRALDFLAGS += $(if $(STAGING_DIR),--sysroot=$(STAGING_DIR))

#ifeq ($(RTCONFIG_BCMARM),y)
#ifneq ($(RTCONFIG_DHDAP),y)
#export EXTRACFLAGS += -I$(SRCBASE)/shared/bcmwifi/include -DTYPEDEF_FLOAT_T
#endif
#endif
ifeq ($(HND_ROUTER),y)
ifeq ($(HND_ROUTER_AX),y)
HND_TC := /opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr
else
HND_TC := /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr
endif

HND_RT_PLATFORM_EXT_CFLAGS := -DCHIP_$(BRCM_CHIP) -DCONFIG_BCM9$(BRCM_CHIP)
ifeq ($(BRCM_CHIP),$(filter $(BRCM_CHIP),6855))
HND_RT_PLATFORM_EXT_CFLAGS += -D_BCM96855_
endif

export EXTRACFLAGS += -DDEBUG_NOISY -DDEBUG_RCTEST
HND_CMN_CFLAGS := -DLINUX -Os -Wno-date-time -Wall -Darm -g -fPIC -pthread $(HND_RT_PLATFORM_EXT_CFLAGS) -I$(HND_TC)/include -L$(HND_TC)/lib
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_6855_),)
ifeq ($(BUILD_GPON),y)
export BCM_PON=y
endif
ifeq ($(BUILD_EPON_SDK),y)
export BCM_PON=y
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_6846_6856_6878_6855_),)
export BCM_PON_XRDP=y
endif
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_63158_63146_4912_6813_),)
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
BCM_DSL_RDP=y
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63158_63146_4912_6813_),)
export BCM_DSL_XRDP=y
endif
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_6858_63158_6846_6856_6878_63146_4912_6813_6855_),)
export BCM_XRDP=y
endif
ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_63138_63148_4908_),)
export BCM_RDP=y
endif
###### Need to export kernel config to userspace as well
ifeq ($(BCM_PON),y)
HND_CMN_CFLAGS += -DBCM_PON
endif
ifeq ($(BCM_PON_XRDP),y)
HND_CMN_CFLAGS += -DBCM_PON_XRDP
endif
ifeq ($(BCM_DSL_XRDP),y)
HND_CMN_CFLAGS += -DBCM_DSL_XRDP
endif
ifeq ($(BCM_DSL_RDP),y)
HND_CMN_CFLAGS += -DBCM_DSL_RDP
endif
ifeq ($(BCM_XRDP),y)
HND_CMN_CFLAGS += -DBCM_XRDP
endif
ifeq ($(BCM_RDP),y)
HND_CMN_CFLAGS += -DBCM_RDP
endif
ifneq ($(strip $(BRCM_DRIVER_GPON)_$(BUILD_GPON)),_)
HND_CMN_CFLAGS += -DCONFIG_BCM_MAX_GEM_PORTS=$(BCM_MAX_GEM_PORTS)
else
HND_CMN_CFLAGS += -DCONFIG_BCM_MAX_GEM_PORTS=1
endif
export CMS_DMP_FLAGS
export CMS_COMPILE_FLAGS
export CMN_WLAN_FLAGS
HND_CMN_CFLAGS += $(CMS_DMP_FLAGS)
HND_CMN_CFLAGS += $(CMS_COMPILE_FLAGS)
HND_CMN_CFLAGS += $(CMN_WLAN_FLAGS)
export HND_CMN_CFLAGS
endif

CPTMP = @[ -d $(TOP)/dbgshare ] && cp $@ $(TOP)/dbgshare/ || true


ifeq ($(CONFIG_RALINK),y)
# Move to platform.mak
else ifeq ($(CONFIG_QCA),y)
# Move to platform.mak
else ifeq ($(CONFIG_ALPINE),y)
# Move to platform.mak
else ifeq ($(CONFIG_LANTIQ),y)
# Move to platform.mak
else # CONFIG_RALINK != y && CONFIG_QCA != y
# Broadcom SoC
ifeq ($(CONFIG_LINUX26),y)
ifeq ($(RTCONFIG_BCMWL6),y)
ifneq ($(RTCONFIG_BCMARM),y)
# e.g. RT-AC66U
export KERNELCC := /opt/brcm/K26/hndtools-mipsel-linux-uclibc-4.2.3/bin/mipsel-linux-uclibc-gcc
else # RTCONFIG_BCMARM = y
export KERNELCC := $(CC)
endif
else # RTCONFIG_BCMWL6 != y
export KERNELCC := $(CC)
endif
else # CONFIG_LINUX26 != y
export KERNELCC := $(CC)-3.4.6
endif

ifeq ($(RTCONFIG_BCMWL6),y)
ifneq ($(RTCONFIG_BCMARM),y)
# e.g. RT-AC66U
export KERNELLD := /opt/brcm/K26/hndtools-mipsel-linux-uclibc-4.2.3/bin/mipsel-linux-uclibc-ld
else # RTCONFIG_BCMARM = y
export KERNELLD := $(LD)
endif
else # RTCONFIG_BCMWL6 != y
export KERNELLD := $(LD)
endif
endif

ifeq ($(KERNELCC),)
export KERNELCC := $(CC)
endif
ifeq ($(KERNELLD),)
export KERNELLD := $(LD)
endif

#	ifneq ($(STATIC),1)
#	SIZECHECK = @$(SRCBASE)/btools/sizehistory.pl $@ $(TOMATO_PROFILE_L)_$(notdir $@)
#	else
SIZECHECK = @$(SIZE) $@
#	endif
