
nothing: default

include $(BUILD_DIR)/make.common



unexport \
BRCMAPPS                   \
BRCM_KERNEL_AUXFS_JFFS2    \
BRCM_PSI_VERSION           \
BRCM_PTHREADS              \
BRCM_RAMDISK_BOOT_EN       \
BRCM_RAMDISK_SIZE          \
BRCM_NFS_MOUNT_EN          \
BRCM_SNMP                  \
BUILD_VLANCTL              \
BUILD_DDNSD                \
BUILD_DPROXY               \
BUILD_EBTABLES             \
BUILD_EPITTCP              \
BUILD_FTPD                 \
BUILD_FTPD_STORAGE         \
BUILD_IPPD                 \
BUILD_IPSEC_TOOLS          \
BUILD_MKSQUASHFS           \
BUILD_PPPD                 \
BUILD_SNMP                 \
BUILD_SSHD                 \
BUILD_SSHD_MIPS_GENKEY     \
BUILD_BRCM_CMS             \
BUILD_TR69C                \
BUILD_TR69_QUEUED_TRANSFERS \
BUILD_TR69C_SSL            \
BUILD_TR69_XBRCM           \
BUILD_TR69_UPLOAD          \
BUILD_TR69C_VENDOR_RPC     \
BUILD_OMCI                 \
BUILD_UDHCP                \
BUILD_UDHCP_RELAY          \
BUILD_ZEBRA                \
BUILD_LIBUSB               \
WEB_POPUP                  \
BUILD_TR69C_BCM_SSL        \
BUILD_BOARD_LOG_SECTION    \
BRCM_LOG_SECTION_SIZE      \
BRCM_FLASHBLK_SIZE         \
BRCM_AUXFS_PERCENT         \
BRCM_BACKUP_PSI            \
BUILD_GPONCTL              \
BUILD_SPUCTL               \
FLASH_NAND_BLOCK_16KB      \
FLASH_NAND_BLOCK_128KB     \
FLASH_NAND_BLOCK_256KB     \
FLASH_NAND_BLOCK_512KB     \
FLASH_NAND_BLOCK_1024KB     \
BUILD_IQCTL                 \
BUILD_EPONCTL               \
BRCM_PARTITION_CFG_FILE     

export BRCM_KERNEL_DEBUG           \

export INC_BCMDRIVER_PATH 

export BRCM_RDP_PARAM1_SIZE BRCM_RDP_PARAM2_SIZE BRCM_DHD_PARAM1_SIZE BRCM_DHD_PARAM2_SIZE BRCM_DHD_PARAM3_SIZE OOPSLOG_PARTITION_NAME
export BRCM_DRIVER_PKTFLOW_DEBUG BRCM_DRIVER_PKTFLOW_IPV6 BRCM_DRIVER_PKTFLOW_MCAST
export INC_RDPA_MW_PATH INC_SPI_PATH INC_FLASH_PATH INC_ENET_DMA_FLAGS PROFILE_KERNEL_VER INC_ADSLDRV_PATH 
export INC_BCMLIBS_PATH INC_UTILS_PATH RDPSDK_DIR BCMDRIVERS_DIR
export KERNEL_DEBUG
export BUILD_PHY_ADSL   ### FIXME -- These should be using Kconfig
export ORIG_PROFILE_ARCH
export BUILD_BCM_WLAN_NO_MFGBIN
export BUILD_BCM_WLAN_DGASP
export RDP_PROJECT2

ifeq ($(RDK_BUILD),y)
#The kernel build will use the value of 'CROSS_COMPILE'. Make.common will calculate the kernel toolchain and set it in KCROSS_COMPILE
	CROSS_COMPILE:=$(KCROSS_COMPILE)
endif

ifeq ($(strip $(DESKTOP_LINUX)),)
default version_info headers_install bcmkernel_headers_install olddefconfig modules modules_install clean mrproper tools/perf tools/perf_clean dtbs prepare_bcm_driver:
	$(MAKE) -C $(KERNEL_DIR) $(MAKEOVERRIDES) $(MAKECMDGOALS) $(KERN_TARGET)
else
default version_info headers_install bcmkernel_headers_install olddefconfig modules modules_install tools/perf tools/perf_clean dtbs prepare_bcm_driver:
	@echo "******************** SKIP kernel build for DESKTOP_LINUX ********************";
	touch $(KERNEL_DIR)/vmlinux
mrproper clean:
	rm -f $(KERNEL_DIR)/vmlinux $(KERNEL_DIR)/.config
endif


bcm_headers_install:
	$(Q)echo "======================================================"
	$(Q)$(MAKE) -C $(BRCMDRIVERS_DIR) bcm_headers_install

.PHONY: nothing bcm_headers_install version_info headers_install default olddefconfig modules modules_install clean mrproper tools/perf tools/perf_clean dtbs
