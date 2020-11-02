-include $(SRCBASE)/router/.config

ifeq ($(HND_ROUTER),y)
export LINUXDIR := $(SRCBASE)/kernel/linux-4.1
else
export LINUXDIR := $(SRCBASE)/linux/linux-2.6
endif
export BUILD := $(shell (gcc -dumpmachine))

ifeq ($(RTCONFIG_BCMARM),y)

ifeq ($(HND_ROUTER),y)
export PRIVATE_EXTRACFLAGS := $(BRCM_COMMON_CFLAGS) -DHND_ROUTER -DLINUX26 -DLINUX_2_6_36 -DCONFIG_BCMWL5
export EXTRACFLAGS := -march=armv7-a -marm -DHND_ROUTER -DCONFIG_BCMWL5 -D__ARM_ARCH_7A__
else
 ifeq ($(EXTRACFLAGS),)
export EXTRACFLAGS := -DBCMWPA2 -DBCMARM -fno-delete-null-pointer-checks -marm 
 endif
endif

 ifeq ($(HND_ROUTER),y)
 ifeq ($(HND_ROUTER_AX),y)
export PLATFORM_ARCH := arm-glibc
export CROSS_COMPILE := /opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/bin/arm-buildroot-linux-gnueabi-
export CROSS_COMPILER := $(CROSS_COMPILE)
export CONFIGURE := ./configure LD=$(CROSS_COMPILE)ld --host=arm-buildroot-linux-gnueabi
ifeq ($(BRCM_CHIP),4908)
export CONFIGURE_64 := ./configure LD=$(CROSS_COMPILE_64)ld --host=aarch64-buildroot-linux-gnu
export HOSTCONFIG_64 := linux-aarch64 -DL_ENDIAN -march=armv8-a -fomit-frame-pointer -mabi=lp64 -ffixed-r8 -D__ARM_ARCH_8A__
export HOSTCONFIG := linux-armv4 -DL_ENDIAN -march=armv8-a -fomit-frame-pointer -mabi=aapcs-linux -marm -ffixed-r8 -msoft-float -D__ARM_ARCH_8A__
else
export HOSTCONFIG := linux-armv4 -DL_ENDIAN -march=armv7-a -fomit-frame-pointer -mabi=aapcs-linux -marm -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__
endif
export TOP_PLATFORM := $(SRCBASE)/router-sysdep
export BCMEX :=
export ARCH := arm
export HOST :=
export TOOLS := /opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1
export RTVER := 0.9.32.1
export BCMSUB := brcmarm
export KERNEL_BINARY=$(LINUXDIR)/vmlinux
export PRBM_EXT=_preb
 else
export PLATFORM_ARCH := arm-glibc
export CROSS_COMPILE := /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/bin/arm-buildroot-linux-gnueabi-
export CROSS_COMPILER := $(CROSS_COMPILE)
export CONFIGURE := ./configure LD=$(CROSS_COMPILE)ld --host=arm-buildroot-linux-gnueabi
export CONFIGURE_64 := ./configure LD=$(CROSS_COMPILE_64)ld --host=aarch64-buildroot-linux-gnu 
export HOSTCONFIG := linux-armv4 -DL_ENDIAN -march=armv8-a -fomit-frame-pointer -mabi=aapcs-linux -marm -ffixed-r8 -msoft-float -D__ARM_ARCH_8A__
export HOSTCONFIG_64 := linux-aarch64 -DL_ENDIAN -march=armv8-a -fomit-frame-pointer -mabi=lp64 -ffixed-r8 -D__ARM_ARCH_8A__
export TOP_PLATFORM := $(SRCBASE)/router
export BCMEX := _arm
export ARCH := arm
export HOST :=
export TOOLS := /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25
export RTVER := 0.9.32.1
export BCMSUB := brcmarm
export KERNEL_BINARY=$(LINUXDIR)/vmlinux
export PRBM_EXT=_preb
 endif
 else
export KERNEL_BINARY=$(LINUXDIR)/arch/arm/boot/zImage
export PLATFORM_ARCH := arm-uclibc
export CROSS_COMPILE := arm-brcm-linux-uclibcgnueabi-
export CROSS_COMPILER := $(CROSS_COMPILE)
export CONFIGURE := ./configure --host=arm-linux --build=$(BUILD)
export HOSTCONFIG := linux-armv4 -fomit-frame-pointer
export BCMEX := _arm
export EXTRA_FLAG := -lgcc_s
export ARCH := arm
export HOST := arm-linux
export TOOLS := $(SRCBASE)/toolchains/hndtools-arm-linux-2.6.36-uclibc-4.5.3
export RTVER := 0.9.32.1
export BCMSUB := brcmarm
 endif
else ifeq ($(RTCONFIG_BCM_ARM_GCLIBC),y)
export CROSS_COMPILE ?= arm-buildroot-linux-gnueabi-
export CONFIGURE := ./configure arm-linux --build=$(BUILD)
export TOOLCHAIN := $(shell cd $(dir $(shell which $(CROSS_COMPILE)gcc))/../.. && pwd -P)
export CFLAGS += -fno-strict-aliasing
SUBMAKE_SETTINGS += ARCH=$(ARCH)
else
 ifeq ($(EXTRACFLAGS),)
export EXTRACFLAGS := -DBCMWPA2 -fno-delete-null-pointer-checks -mips32 -mtune=mips32
 endif

export KERNEL_BINARY=$(LINUXDIR)/arch/mips/brcm-boards/bcm947xx/compressed/zImage
export PLATFORM_ARCH := mipsel-uclibc
export CROSS_COMPILE := mipsel-uclibc-
export CROSS_COMPILER := $(CROSS_COMPILE)
export READELF := mipsel-linux-readelf
export CONFIGURE := ./configure --host=mipsel-linux --build=$(BUILD)
export HOSTCONFIG := linux-mips32
export ARCH := mips
export HOST := mipsel-linux
export TOOLS := $(SRCBASE)/../../tools/brcm/hndtools-mipsel-linux
export RTVER := 0.9.30.1
export TEST := 3
endif

ifneq ($(HND_ROUTER),y)
ifeq ($(PLATFORM),)
export PLATFORM := $(PLATFORM_ARCH)
endif
endif

EXTRA_CFLAGS := -DLINUX26 -DCONFIG_BCMWL5 -DDEBUG_NOISY -DDEBUG_RCTEST -pipe -DTTEST 

export CONFIG_LINUX26=y
export CONFIG_BCMWL5=y


define platformRouterOptions
	@( \
	if [ "$(RTAC5300)" = "y" ] ; then \
		sed -i "/RTCONFIG_HAS_5G_2/d" $(1); \
		echo "RTCONFIG_HAS_5G_2=y" >>$(1); \
	fi; \
	if [ "$(GTAC5300)" = "y" ] ; then \
		sed -i "/RTCONFIG_HAS_5G_2/d" $(1); \
		echo "RTCONFIG_HAS_5G_2=y" >>$(1); \
	fi; \
	if [ "$(RTAC3200)" = "y" ] ; then \
		sed -i "/RTCONFIG_HAS_5G_2/d" $(1); \
		echo "RTCONFIG_HAS_5G_2=y" >>$(1); \
	fi; \
	if [ "$(RTAX92U)" = "y" -o "$(GTAX11000)" = "y" -o "$(RTAX95Q)" = "y" ]; then \
		sed -i "/RTCONFIG_HAS_5G_2/d" $(1); \
		echo "RTCONFIG_HAS_5G_2=y" >>$(1); \
	fi; \
	if [ "$(BCM_OAM)" = "y" ]; then \
		sed -i "/RTCONFIG_BCM_OAM/d" $(1); \
		echo "RTCONFIG_BCM_OAM=y" >>$(1); \
	fi; \
	)
endef

define platformBusyboxOptions
endef

define platformKernelConfig
# prepare config_base
# prepare prebuilt kernel binary
	@( \
	sed -i "/CONFIG_RGMII_BCM_FA/d" $(1); \
	if [ "$(RGMII_BCM_FA)" = "y" ]; then \
		echo "CONFIG_RGMII_BCM_FA=y" >> $(1); \
	else \
		echo "# CONFIG_RGMII_BCM_FA is not set" >> $(1); \
	fi; \
	if [ "$(LACP)" = "y" ]; then \
		if [ "$(HND_ROUTER)" != "y" ]; then \
			sed -i "/CONFIG_LACP/d" $(1); \
			echo "CONFIG_LACP=m" >>$(1); \
			sed -i "/CONFIG_BCM_AGG/d" $(1); \
			echo "CONFIG_BCM_AGG=y" >>$(1); \
		fi; \
	fi; \
	if [ "$(BCMNAND)" = "y" ]; then \
		sed -i "/CONFIG_MTD_NFLASH/d" $(1); \
		echo "CONFIG_MTD_NFLASH=y" >>$(1); \
		sed -i "/CONFIG_MTD_NAND/d" $(1); \
		echo "CONFIG_MTD_NAND=y" >>$(1); \
		echo "CONFIG_MTD_NAND_IDS=y" >>$(1); \
		echo "# CONFIG_MTD_NAND_VERIFY_WRITE is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_ECC_SMC is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_MUSEUM_IDS is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_DENALI is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_RICOH is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_DISKONCHIP is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_CAFE is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_NANDSIM is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_PLATFORM is not set" >>$(1); \
		echo "# CONFIG_MTD_NAND_ONENAND is not set" >>$(1); \
		sed -i "/CONFIG_MTD_BRCMNAND/d" $(1); \
		echo "CONFIG_MTD_BRCMNAND=y" >>$(1); \
	fi; \
	if [ "$(DSL_BCM)" = "y" ]; then \
		sed -i "/CONFIG_BCM_XTMCFG is not set/d" $(1); \
		echo "CONFIG_BCM_XTMCFG=m" >>$(1); \
		sed -i "/CONFIG_BCM_XTMRT is not set/d" $(1); \
		echo "CONFIG_BCM_XTMRT=m" >>$(1); \
		sed -i "/CONFIG_BCM_ADSL is not set/d" $(1); \
		echo "CONFIG_BCM_ADSL=m" >>$(1); \
	fi; \
	if [ "$(ARM)" = "y" ]; then \
		if [ "$(HND_ROUTER)" != "y" ]; then \
			mkdir -p $(SRCBASE)/router/ctf_arm/linux; \
		fi; \
		if [ "$(BCM7)" = "y" ]; then \
			if [ "$(ARMCPUSMP)" = "up" ]; then \
				cp -f $(SRCBASE)/router/ctf_arm/bcm7_up/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			else \
				cp -f $(SRCBASE)/router/ctf_arm/bcm7/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			fi; \
		elif [ "$(BCM_7114)" = "y" ]; then \
			if [ "$(GMAC3)" = "y" ]; then \
				cp -f $(SRCBASE)/router/ctf_arm/bcm_7114_gmac3/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			else \
				cp -f $(SRCBASE)/router/ctf_arm/bcm_7114/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			fi; \
		elif [ "$(BCM9)" = "y" ]; then \
                	cp -f $(SRCBASE)/router/ctf_arm/bcm9/ctf.* $(SRCBASE)/router/ctf_arm/linux/;\
		elif [ "$(HND_ROUTER)" = "y" ]; then \
			echo "do nothing"; \
		else \
			if [ "$(ARMCPUSMP)" = "up" ]; then \
				cp -f $(SRCBASE)/router/ctf_arm/bcm6_up/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			else \
				cp -f $(SRCBASE)/router/ctf_arm/bcm6/ctf.* $(SRCBASE)/router/ctf_arm/linux/; \
			fi; \
		fi; \
		if [ "$(DPSTA)" = "y" ]; then \
			mkdir -p $(SRCBASE)/router/dpsta/linux; \
			if [ "$(BCM7)" = "y" ]; then \
				cp -f $(SRCBASE)/router/dpsta/bcm7/dpsta.o $(SRCBASE)/router/dpsta/linux; \
				cp -f $(SRCBASE)/router/dpsta/bcm7/dpsta.h $(SRCBASE)/router/dpsta; \
				cp -f $(SRCBASE)/router/dpsta/bcm7/dpsta_linux.h $(SRCBASE)/router/dpsta; \
			elif [ "$(BCM_7114)" = "y" ]; then \
				cp -f $(SRCBASE)/router/dpsta/bcm7114/dpsta.o $(SRCBASE)/router/dpsta/linux; \
				cp -f $(SRCBASE)/router/dpsta/bcm7114/dpsta.h $(SRCBASE)/router/dpsta; \
				cp -f $(SRCBASE)/router/dpsta/bcm7114/dpsta_linux.h $(SRCBASE)/router/dpsta; \
			elif [ "$(BCM9)" = "y" ]; then \
				echo "do nothing"; \
			elif [ "$(HND_ROUTER)" = "y" ]; then \
				cp -f $(SRCBASE)/router/dpsta/hnd/dpsta.o $(SRCBASE)/router/dpsta/linux; \
				cp -f $(SRCBASE)/router/dpsta/hnd/dpsta.h $(SRCBASE)/router/dpsta; \
				cp -f $(SRCBASE)/router/dpsta/hnd/dpsta_linux.h $(SRCBASE)/router/dpsta; \
			else \
				if [ "$(ARMCPUSMP)" = "up" ]; then \
					cp -f $(SRCBASE)/router/dpsta/bcm6_up/dpsta.o $(SRCBASE)/router/dpsta/linux; \
				else \
					cp -f $(SRCBASE)/router/dpsta/bcm6/dpsta.o $(SRCBASE)/router/dpsta/linux; \
				fi; \
				cp -f $(SRCBASE)/router/dpsta/bcm6/dpsta.h $(SRCBASE)/router/dpsta; \
				cp -f $(SRCBASE)/router/dpsta/bcm6/dpsta_linux.h $(SRCBASE)/router/dpsta; \
			fi; \
		fi; \
	fi; \
	if [ "$(SFPRAM16M)" = "y" ]; then \
		sed -i "/CONFIG_WL_USE_AP/d" $(1); \
		echo "CONFIG_WL_USE_APSTA=y" >>$(1); \
		echo "# CONFIG_WL_USE_AP is not set" >>$(1); \
		echo "# CONFIG_WL_USE_AP_SDSTD is not set" >>$(1); \
		echo "# CONFIG_WL_USE_AP_ONCHIP_G is not set" >>$(1); \
		echo "# CONFIG_WL_USE_APSTA_ONCHIP_G is not set" >>$(1); \
		sed -i "/CONFIG_INET_GRO/d" $(1); \
		echo "# CONFIG_INET_GRO is not set" >> $(1); \
		sed -i "/CONFIG_INET_GSO/d" $(1); \
		echo "# CONFIG_INET_GSO is not set" >> $(1); \
		sed -i "/CONFIG_NET_SCH_HFSC/d" $(1); \
		echo "# CONFIG_NET_SCH_HFSC is not set" >> $(1); \
		sed -i "/CONFIG_NET_SCH_ESFQ/d" $(1); \
		echo "# CONFIG_NET_SCH_ESFQ is not set" >> $(1); \
		sed -i "/CONFIG_NET_SCH_TBF/d" $(1); \
		echo "# CONFIG_NET_SCH_TBF is not set" >> $(1); \
		sed -i "/CONFIG_NLS/d" $(1); \
		echo "# CONFIG_NLS is not set" >>$(1); \
		echo "# CONFIG_NLS_DEFAULT=\"iso8859-1\"">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_437 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_737 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_775 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_850 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_852 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_855 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_857 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_860 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_861 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_862 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_863 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_864 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_865 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_866 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_869 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_936 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_950 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_932 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_949 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_874 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_8 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_1250 is not set">>$(1); \
		echo "# CONFIG_NLS_CODEPAGE_1251 is not set">>$(1); \
		echo "# CONFIG_NLS_ASCII is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_1 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_2 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_3 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_4 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_5 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_6 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_7 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_9 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_13 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_14 is not set">>$(1); \
		echo "# CONFIG_NLS_ISO8859_15 is not set">>$(1); \
		echo "# CONFIG_NLS_KOI8_R is not set">>$(1); \
		echo "# CONFIG_NLS_KOI8_U is not set">>$(1); \
		echo "# CONFIG_NLS_UTF8 is not set">>$(1); \
		sed -i "/CONFIG_USB/d" $(1); \
		echo "# CONFIG_USB_SUPPORT is not set" >> $(1); \
		sed -i "/CONFIG_SCSI/d" $(1); \
		echo "# CONFIG_SCSI is not set" >> $(1); \
		sed -i "/CONFIG_LBD/d" $(1); \
		echo "# CONFIG_LBD is not set" >> $(1); \
		sed -i "/CONFIG_BLK_DEV_SD/d" $(1); \
		sed -i "/CONFIG_BLK_DEV_SR/d" $(1); \
		sed -i "/CONFIG_CHR_DEV_SG/d" $(1); \
		sed -i "/CONFIG_VIDEO/d" $(1); \
		echo "# CONFIG_VIDEO_DEV is not set" >> $(1); \
		sed -i "/CONFIG_V4L_USB_DRIVERS/d" $(1); \
		sed -i "/CONFIG_SOUND/d" $(1); \
		echo "# CONFIG_SOUND is not set" >> $(1); \
		sed -i "/CONFIG_SND/d" $(1); \
		sed -i "/CONFIG_HID/d" $(1); \
		echo "# CONFIG_HID is not set" >> $(1); \
		sed -i "/CONFIG_MMC/d" $(1); \
		echo "# CONFIG_MMC is not set" >> $(1); \
		sed -i "/CONFIG_PARTITION_ADVANCED/d" $(1); \
		echo "# CONFIG_PARTITION_ADVANCED is not set" >> $(1); \
		sed -i "/CONFIG_TRACE_IRQFLAGS_SUPPORT/d" $(1); \
		sed -i "/CONFIG_SYS_SUPPORTS_KGDB/d" $(1); \
		sed -i "/CONFIG_EXT2_FS/d" $(1); \
		echo "# CONFIG_EXT2_FS is not set" >> $(1); \
		sed -i "/CONFIG_EXT3_FS/d" $(1); \
		echo "# CONFIG_EXT3_FS is not set" >> $(1); \
		sed -i "/CONFIG_JBD/d" $(1); \
		echo "# CONFIG_JBD is not set" >> $(1); \
		sed -i "/CONFIG_REISERFS_FS/d" $(1); \
		echo "# CONFIG_REISERFS_FS is not set" >> $(1); \
		sed -i "/CONFIG_FAT_FS/d" $(1); \
		echo "# CONFIG_FAT_FS is not set" >> $(1); \
		sed -i "/CONFIG_VFAT_FS/d" $(1); \
		echo "# CONFIG_VFAT_FS is not set" >> $(1); \
		sed -i "/CONFIG_NFS_FS/d" $(1); \
		echo "# CONFIG_NFS_FS is not set" >> $(1); \
		sed -i "/CONFIG_NFSD/d" $(1); \
		echo "# CONFIG_NFSD is not set" >> $(1); \
		sed -i "/CONFIG_FUSE_FS/d" $(1); \
		echo "# CONFIG_FUSE_FS is not set" >> $(1); \
		sed -i "/CONFIG_CIFS/d" $(1); \
		echo "# CONFIG_CIFS is not set" >> $(1); \
		sed -i "/CONFIG_FAT/d" $(1); \
		sed -i "/CONFIG_INOTIFY/d" $(1); \
		echo "# CONFIG_INOTIFY is not set" >> $(1); \
		sed -i "/CONFIG_DNOTIFY/d" $(1); \
		echo "# CONFIG_DNOTIFY is not set" >> $(1); \
		sed -i "/CONFIG_CRYPTO_BLKCIPHER/d" $(1); \
		sed -i "/CONFIG_CRYPTO_HASH/d" $(1); \
		sed -i "/CONFIG_CRYPTO_MANAGER/d" $(1); \
		echo "# CONFIG_CRYPTO_MANAGER is not set" >> $(1); \
		sed -i "/CONFIG_CRYPTO_HMAC/d" $(1); \
		echo "# CONFIG_CRYPTO_HMAC is not set" >> $(1); \
		sed -i "/CONFIG_CRYPTO_ECB/d" $(1); \
		echo "# CONFIG_CRYPTO_ECB is not set" >> $(1); \
		sed -i "/CONFIG_CRYPTO_CBC/d" $(1); \
		echo "# CONFIG_CRYPTO_CBC is not set" >> $(1); \
	fi; \
	if [ "$(ARM)" = "y" ]; then \
		if [ "$(BCM7)" = "y" ]; then \
			if [ -d $(SRCBASE)/../../43602/src/wl/sysdeps/$(BUILD_NAME) ]; then \
				if [ -d $(SRCBASE)/../../43602/src/wl/sysdeps/$(BUILD_NAME)/clm ]; then \
					cp -f $(SRCBASE)/../../43602/src/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data.c $(SRCBASE)/../../43602/src/wl/clm/src/. ; \
					cp -f $(SRCBASE)/../../43602/src/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data_inc.c $(SRCBASE)/../../43602/src/wl/clm/src/. ; \
				fi; \
			else \
				if [ -d $(SRCBASE)/../../43602/src/wl/sysdeps/default/clm ]; then \
					cp -f $(SRCBASE)/../../43602/src/wl/sysdeps/default/clm/src/wlc_clm_data.c $(SRCBASE)/../../43602/src/wl/clm/src/. ; \
					cp -f $(SRCBASE)/../../43602/src/wl/sysdeps/default/clm/src/wlc_clm_data_inc.c $(SRCBASE)/../../43602/src/wl/clm/src/. ; \
				fi; \
			fi; \
			if [ -d $(SRCBASE)/router/wl_arm_7/prebuilt ]; then \
				mkdir -p $(SRCBASE)/../../dhd/src/dhd/linux ; \
				cp $(SRCBASE)/router/wl_arm_7/prebuilt/dhd.o $(SRCBASE)/../../dhd/src/dhd/linux ; \
			fi; \
			if [ -d $(SRCBASE)/router/et_arm_7/prebuilt ]; then \
				mkdir -p $(SRCBASE)/et/linux ; \
				cp $(SRCBASE)/router/et_arm_7/prebuilt/et.o $(SRCBASE)/et/linux ; \
			fi; \
		elif [ "$(BCM_7114)" = "y" ]; then \
			if [ -d $(SRCBASE)/router/wl_arm_7114/prebuilt ]; then \
				mkdir -p $(SRCBASE)/../dhd/src/dhd/linux ; \
				cp $(SRCBASE)/router/wl_arm_7114/prebuilt/$(BUILD_NAME)/dhd.o $(SRCBASE)/../dhd/src/dhd/linux ; \
                                mkdir -p $(SRCBASE)/../dhd24/src/dhd/linux ; \
                                cp $(SRCBASE)/router/wl_arm_7114/prebuilt/$(BUILD_NAME)/dhd24.o $(SRCBASE)/../dhd24/src/dhd/linux ; \
			fi; \
			if [ -d $(SRCBASE)/router/et_arm_7114/prebuilt ]; then \
				mkdir -p $(SRCBASE)/et/linux ; \
				cp $(SRCBASE)/router/et_arm_7114/prebuilt/et.o $(SRCBASE)/et/linux ; \
			fi; \
		elif [ "$(BCM_10)" = "y" ]; then \
			if [ -d $(SRCBASE)/router/wl_arm_10/prebuilt ]; then \
				mkdir $(SRCBASE)/wl/linux ; \
				cp $(SRCBASE)/router/wl_arm_10/prebuilt/wl*.o $(SRCBASE)/wl/linux ; \
			fi; \
			if [ -d $(SRCBASE)/router/et_arm_10/prebuilt ]; then \
				mkdir -p $(SRCBASE)/et/linux ; \
				cp $(SRCBASE)/router/et_arm_10/prebuilt/et.o $(SRCBASE)/et/linux ; \
			fi; \
		elif [ "$(BCM9)" = "y" ]; then \
			if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME) ]; then \
				if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/linux ]; then \
					cp -rf $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/linux $(SRCBASE)/wl/. ; \
				fi; \
				if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/clm ]; then \
					cp -f $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data.c $(SRCBASE)/wl/clm/src/. ; \
				fi; \
			elif [ -d $(SRCBASE)/wl/sysdeps/default ]; then \
				if [ -d $(SRCBASE)/wl/sysdeps/default/linux ]; then \
					cp -rf $(SRCBASE)/wl/sysdeps/default/linux $(SRCBASE)/wl/. ; \
				fi; \
				if [ -d $(SRCBASE)/wl/sysdeps/default/clm ]; then \
					cp -f $(SRCBASE)/wl/sysdeps/default/clm/src/wlc_clm_data.c $(SRCBASE)/wl/clm/src/. ; \
				fi; \
			fi; \
			if [ -d $(SRCBASE)/router/wl_arm_9/prebuilt ]; then \
				mkdir $(SRCBASE)/wl/linux ; \
				cp $(SRCBASE)/router/wl_arm_9/prebuilt/wl*.o $(SRCBASE)/wl/linux ; \
			fi; \
			if [ -d $(SRCBASE)/router/et_arm_9/prebuilt ]; then \
				mkdir -p $(SRCBASE)/et/linux ; \
				cp $(SRCBASE)/router/et_arm_9/prebuilt/et.o $(SRCBASE)/et/linux ; \
			fi; \
		elif [ "$(HND_ROUTER)" = "y" ]; then \
			echo "hnd_router platform work" ; \
			if [ -d $(TOP_PLATFORM)/hnd_extra/prebuilt ]; then \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/dhd/src/dhd/linux/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/dhd/src/shared/bcmwifi/include/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/linux/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/emf/linux/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/igs/linux/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/hnd/linux/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/exe/prebuilt/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/sys/ ; \
				mkdir -p $(HND_SRC)/bcmdrivers/opensource/char/map/impl1 ; \
				mkdir -p $(HND_SRC)/rdp/projects/WL4908/target/bdmf ; \
				mkdir -p $(HND_SRC)/rdp/projects/WL4908/target/rdpa ; \
				mkdir -p $(HND_SRC)/rdp/projects/WL4908/target/rdpa_gpl ; \
				(cd rdp/projects/WL4908/target/bdmf; rm -f Makefile; ln -sf ../../../../drivers/bdmf/Makefile Makefile); \
				(cd rdp/projects/WL4908/target/rdpa_gpl; rm -rf include; ln -sf ../../../../../rdp/drivers/rdpa_gpl/include include); \
				(cd rdp/projects/WL4908/target/bdmf; rm -rf framework; ln -sf ../../../../../rdp/drivers/bdmf/framework framework); \
				(cd rdp/projects/WL4908/target/bdmf; rm -rf system; ln -sf ../../../../../rdp/drivers/bdmf/system system); \
				if [ "$(HND_ROUTER_AX)" = "y" ]; then \
					cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_enet.o $(HND_SRC)/bcmdrivers/opensource/net/enet/impl7/bcm_enet$(PRBM_EXT).o ; \
				else \
					cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_enet.o $(HND_SRC)/bcmdrivers/opensource/net/enet/impl5/bcm_enet$(PRBM_EXT).o ; \
				fi; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wfd.o $(HND_SRC)/bcmdrivers/opensource/net/wfd/impl1/wfd$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcmpdc.o $(HND_SRC)/bcmdrivers/opensource/char/pdc/impl1/bcmpdc$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcmspu.o $(HND_SRC)/bcmdrivers/opensource/char/spudd/impl4/bcmspu$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_arm64_setup.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_arm_cpuidle.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_arm_irq.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_dt.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_extirq.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_i2c.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_legacy_io_map.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/blxargs.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/setup.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_usb.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/bcm_usb$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcm_thermal.o $(HND_SRC)/bcmdrivers/opensource/char/plat-bcm/impl1/bcm_thermal$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdp_fpm.o $(HND_SRC)/bcmdrivers/opensource/char/fpm/impl1/rdp_fpm$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdpa_cmd.o $(HND_SRC)/bcmdrivers/opensource/char/rdpa_drv/impl1/rdpa_cmd$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdpa_gpl_ext.o $(HND_SRC)/bcmdrivers/opensource/char/rdpa_gpl_ext/impl1/rdpa_gpl_ext$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdpa_mw.o $(HND_SRC)/bcmdrivers/opensource/char/rdpa_mw/impl1/rdpa_mw$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/ivi_map.h $(HND_SRC)/bcmdrivers/opensource/char/map/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/ivi_config.h $(HND_SRC)/bcmdrivers/opensource/char/map/impl1/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wlcsm.o $(HND_SRC)/bcmdrivers/broadcom/char/wlcsm_ext/impl1/wlcsm$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/pktrunner.o $(HND_SRC)/bcmdrivers/broadcom/char/pktrunner/impl2/pktrunner$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcmvlan.o $(HND_SRC)/bcmdrivers/broadcom/char/vlan/impl1/bcmvlan$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/chipinfo.o $(HND_SRC)/bcmdrivers/broadcom/char/chipinfo/impl1/chipinfo$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/cmdlist.o $(HND_SRC)/bcmdrivers/broadcom/char/cmdlist/impl1/cmdlist$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/nciTMSkmod.o $(HND_SRC)/bcmdrivers/broadcom/char/tms/impl1/nciTMSkmod$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/pktflow.o $(HND_SRC)/bcmdrivers/broadcom/char/pktflow/impl1/pktflow$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/pwrmngtd.o $(HND_SRC)/bcmdrivers/broadcom/char/pwrmngt/impl1/pwrmngtd$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bdmf.o $(HND_SRC)/rdp/projects/WL4908/target/bdmf/bdmf$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdpa.o $(HND_SRC)/rdp/projects/WL4908/target/rdpa/rdpa$(PRBM_EXT).o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/rdpa_gpl.o $(HND_SRC)/rdp/projects/WL4908/target/rdpa_gpl/rdpa_gpl$(PRBM_EXT).o ; \
				cp $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/linux/prebuilt/wl.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/linux/prebuilt/wl_apsta.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wl $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/exe/prebuilt/ ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/hnd.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/hnd/linux/prebuilt/hnd.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/dhd.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/dhd/src/dhd/linux/prebuilt/dhd.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/emf.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/emf/linux/prebuilt/emf.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/igs.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/igs/linux/prebuilt/igs.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wl.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/linux/prebuilt/wl_apsta.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wl.o $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/linux/prebuilt/wl.o ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/bcmwifi_rates.h $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/dhd/src/shared/bcmwifi/include/bcmwifi_rates.h ; \
				cp $(TOP_PLATFORM)/hnd_extra/prebuilt/wlc_types.h $(HND_SRC)/bcmdrivers/broadcom/net/wl/bcm9$(BCM_CHIP)/main/src/wl/sys/ ; \
			fi; \
		else \
			if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME) ]; then \
				if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/linux ]; then \
					cp -rf $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/linux $(SRCBASE)/wl/. ; \
				fi; \
				if [ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/clm ]; then \
					cp -f $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/clm/src/wlc_clm_data.c $(SRCBASE)/wl/clm/src/. ; \
				fi; \
			elif [ -d $(SRCBASE)/wl/sysdeps/default ]; then \
				if [ -d $(SRCBASE)/wl/sysdeps/default/linux ]; then \
					cp -rf $(SRCBASE)/wl/sysdeps/default/linux $(SRCBASE)/wl/. ; \
				fi; \
				if [ -d $(SRCBASE)/wl/sysdeps/default/clm ]; then \
					cp -f $(SRCBASE)/wl/sysdeps/default/clm/src/wlc_clm_data.c $(SRCBASE)/wl/clm/src/. ; \
				fi; \
			fi; \
			if [ -d $(SRCBASE)/router/wl_arm/prebuilt/$(BUILD_NAME) ]; then \
				mkdir $(SRCBASE)/wl/linux ; \
				cp $(SRCBASE)/router/wl_arm/prebuilt/$(BUILD_NAME)/wl*.o $(SRCBASE)/wl/linux ; \
			fi; \
			if [ -d $(SRCBASE)/router/et_arm/prebuilt ]; then \
				mkdir -p $(SRCBASE)/et/linux ; \
				cp $(SRCBASE)/router/et_arm/prebuilt/et.o $(SRCBASE)/et/linux ; \
			fi; \
		fi; \
	else \
		[ -d $(SRCBASE)/wl/sysdeps/default ] && \
			cp -rf $(SRCBASE)/wl/sysdeps/default/* $(SRCBASE)/wl/; \
		[ -d $(SRCBASE)/wl/sysdeps/$(BUILD_NAME) ] && \
			cp -rf $(SRCBASE)/wl/sysdeps/$(BUILD_NAME)/* $(SRCBASE)/wl/; \
	fi; \
	if [ "$(SNMPD)" = "y" ]; then \
		if [ -d $(SRCBASE)/router/net-snmp-5.7.2/asus_mibs/sysdeps/$(BUILD_NAME) ]; then \
			rm -f  $(SRCBASE)/router/net-snmp-5.7.2/mibs/RT-*.txt ; \
			rm -f  $(SRCBASE)/router/net-snmp-5.7.2/mibs/TM-*.txt ; \
			rm -rf $(SRCBASE)/router/net-snmp-5.7.2/agent/mibgroup/asus-mib ; \
			cp -rf $(SRCBASE)/router/net-snmp-5.7.2/asus_mibs/sysdeps/$(BUILD_NAME)/$(BUILD_NAME)-MIB.txt $(SRCBASE)/router/net-snmp-5.7.2/mibs ; \
			cp -rf $(SRCBASE)/router/net-snmp-5.7.2/asus_mibs/sysdeps/$(BUILD_NAME)/asus-mib $(SRCBASE)/router/net-snmp-5.7.2/agent/mibgroup ; \
		fi; \
	fi; \
	)
endef

#export PARALLEL_BUILD :=
export PARALLEL_BUILD := -j$(shell grep -c '^processor' /proc/cpuinfo)
