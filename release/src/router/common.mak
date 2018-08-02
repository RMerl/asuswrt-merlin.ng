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
export EXTRACFLAGS += -DDEBUG_NOISY -DDEBUG_RCTEST
export HND_CMN_CFLAGS := -DCMS_LOG3  -DLINUX -Os -march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux -marm -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ -Wno-date-time -Wall -Darm -g -fPIC -DMDM_SHARED_MEM -DCMS_MEM_DEBUG  -DSUPPORT_ETHWAN -DSUPPORT_RDPA -DSUPPORT_TMCTL -DDMP_X_BROADCOM_COM_L2TPAC_1 -DSUPPORT_GRE_TUNNEL -DSUPPORT_IPSEC -DDMP_X_BROADCOM_COM_IPSEC_1 -DSUPPORT_TR64C -DDMP_X_BROADCOM_COM_TR64_1 -DSUPPORT_IPV6 -DDMP_X_BROADCOM_COM_DEV2_IPV6_1 -DDMP_DEVICE2_DSLITE_1 -DDMP_DEVICE2_DSLITE_2 -DDMP_DEVICE2_IPV6RD_1 -DDMP_DEVICE2_IPV6INTERFACE_1 -DDMP_DEVICE2_IPV6ROUTING_1 -DDMP_DEVICE2_DHCPV6CLIENT_1 -DDMP_DEVICE2_DHCPV6CLIENTSERVERIDENTITY_1  -DDMP_DEVICE2_DHCPV6SERVER_1  -DDMP_DEVICE2_DHCPV6SERVERADV_1 -DDMP_DEVICE2_DHCPV6SERVERCLIENTINFO_1 -DDMP_DEVICE2_NEIGHBORDISCOVERY_1 -DDMP_DEVICE2_ROUTERADVERTISEMENT_1 -DSUPPORT_TR69C -DSUPPORT_CPU_MEMORY_WEB_PAGE -DSUPPORT_JQPLOT -DSUPPORT_WEB_SOCKETS -DSUPPORT_HTTPD -DSUPPORT_CLI_CMD -DCLI_CMD_EDIT -DSUPPORT_CONSOLED -DSUPPORT_TELNETD -DSUPPORT_SSHD -DSUPPORT_TOD -DDMP_X_BROADCOM_COM_ACCESSTIMERESTRICTION_1 -DSUPPORT_URLFILTER -DSUPPORT_POLICYROUTING -DSUPPORT_UPNP -DDMP_X_BROADCOM_COM_UPNP_1 -DDMP_X_BROADCOM_COM_DLNA_1 -DSUPPORT_FCCTL -DSUPPORT_SNTP -DDMP_X_BROADCOM_COM_ETHERNETOAM_1 -DSUPPORT_ETHSWCTL -DSUPPORT_PWRMNGT -DDMP_X_BROADCOM_COM_PWRMNGT_1 -DSUPPORT_HOSTMIPS_PWRSAVE -DSUPPORT_ETH_PWRSAVE -DSUPPORT_ENERGY_EFFICIENT_ETHERNET -DSUPPORT_ETH_DEEP_GREEN_MODE -DSUPPORT_STORAGESERVICE -DDMP_STORAGESERVICE_1 -DSUPPORT_NTFS_3G -DSUPPORT_SAMBA -DSUPPORT_PPTP -DSUPPORT_NF_MANGLE -DSUPPORT_INTF_GROUPING -DSUPPORT_VLANCTL -DSUPPORT_QOS -DSUPPORT_RATE_LIMIT -DSUPPORT_DEBUG_TOOLS -DSUPPORT_CERT -DDMP_X_BROADCOM_COM_DIGITALCERTIFICATES_1 -DCOMPRESSED_CONFIG_FILE -DCMS_CONFIG_COMPAT -DCHIP_4908    -DCONFIG_BCM94908 -DCONFIG_BCM_MAX_GEM_PORTS=1 -DSUPPORT_INCREMENTAL_FLASHING   -DBRCM_WLAN -DWIRELESS -I/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/include -L/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/lib -Wno-date-time
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
