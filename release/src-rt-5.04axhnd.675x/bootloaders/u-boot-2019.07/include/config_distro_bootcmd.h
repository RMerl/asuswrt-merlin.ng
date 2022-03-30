/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * Copyright 2014 Red Hat, Inc.
 */

#ifndef _CONFIG_CMD_DISTRO_BOOTCMD_H
#define _CONFIG_CMD_DISTRO_BOOTCMD_H

/*
 * A note on error handling: It is possible for BOOT_TARGET_DEVICES to
 * reference a device that is not enabled in the U-Boot configuration, e.g.
 * it may include MMC in the list without CONFIG_CMD_MMC being enabled. Given
 * that BOOT_TARGET_DEVICES is a macro that's expanded by the C pre-processor
 * at compile time, it's not  possible to detect and report such problems via
 * a simple #ifdef/#error combination. Still, the code needs to report errors.
 * The best way I've found to do this is to make BOOT_TARGET_DEVICES expand to
 * reference a non-existent symbol, and have the name of that symbol encode
 * the error message. Consequently, this file contains references to e.g.
 * BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC. Given the
 * prevalence of capitals here, this looks like a pre-processor macro and
 * hence seems like it should be all capitals, but it's really an error
 * message that includes some other pre-processor symbols in the text.
 */

#define BOOTENV_SHARED_BLKDEV_BODY(devtypel) \
		"if " #devtypel " dev ${devnum}; then " \
			"devtype=" #devtypel "; " \
			"run scan_dev_for_boot_part; " \
		"fi\0"

#define BOOTENV_SHARED_BLKDEV(devtypel) \
	#devtypel "_boot=" \
	BOOTENV_SHARED_BLKDEV_BODY(devtypel)

#define BOOTENV_DEV_BLKDEV(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
		"devnum=" #instance "; " \
		"run " #devtypel "_boot\0"

#define BOOTENV_DEV_NAME_BLKDEV(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#ifdef CONFIG_SANDBOX
#define BOOTENV_SHARED_HOST	BOOTENV_SHARED_BLKDEV(host)
#define BOOTENV_DEV_HOST	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_HOST	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_HOST
#define BOOTENV_DEV_HOST \
	BOOT_TARGET_DEVICES_references_HOST_without_CONFIG_SANDBOX
#define BOOTENV_DEV_NAME_HOST \
	BOOT_TARGET_DEVICES_references_HOST_without_CONFIG_SANDBOX
#endif

#ifdef CONFIG_CMD_MMC
#define BOOTENV_SHARED_MMC	BOOTENV_SHARED_BLKDEV(mmc)
#define BOOTENV_DEV_MMC		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_MMC	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_MMC
#define BOOTENV_DEV_MMC \
	BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC
#define BOOTENV_DEV_NAME_MMC \
	BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOTENV_SHARED_UBIFS \
	"ubifs_boot=" \
		"env exists bootubipart || " \
			"env set bootubipart UBI; " \
		"env exists bootubivol || " \
			"env set bootubivol boot; " \
		"if ubi part ${bootubipart} && " \
			"ubifsmount ubi${devnum}:${bootubivol}; " \
		"then " \
			"devtype=ubi; " \
			"run scan_dev_for_boot; " \
		"fi\0"
#define BOOTENV_DEV_UBIFS	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_UBIFS	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_UBIFS
#define BOOTENV_DEV_UBIFS \
	BOOT_TARGET_DEVICES_references_UBIFS_without_CONFIG_CMD_UBIFS
#define BOOTENV_DEV_NAME_UBIFS \
	BOOT_TARGET_DEVICES_references_UBIFS_without_CONFIG_CMD_UBIFS
#endif

#ifdef CONFIG_EFI_LOADER
#if defined(CONFIG_ARM64)
#define BOOTEFI_NAME "bootaa64.efi"
#elif defined(CONFIG_ARM)
#define BOOTEFI_NAME "bootarm.efi"
#elif defined(CONFIG_X86_RUN_32BIT)
#define BOOTEFI_NAME "bootia32.efi"
#elif defined(CONFIG_X86_RUN_64BIT)
#define BOOTEFI_NAME "bootx64.efi"
#elif defined(CONFIG_ARCH_RV32I)
#define BOOTEFI_NAME "bootriscv32.efi"
#elif defined(CONFIG_ARCH_RV64I)
#define BOOTEFI_NAME "bootriscv64.efi"
#endif
#endif

#ifdef BOOTEFI_NAME
#if defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
/*
 * On 32bit ARM systems there is a reasonable number of systems that follow
 * the $soc-$board$boardver.dtb name scheme for their device trees. Use that
 * scheme if we don't have an explicit fdtfile variable.
 */
#define BOOTENV_EFI_SET_FDTFILE_FALLBACK                                  \
	"if test -z \"${fdtfile}\" -a -n \"${soc}\"; then "               \
	  "setenv efi_fdtfile ${soc}-${board}${boardver}.dtb; "           \
	"fi; "
#else
#define BOOTENV_EFI_SET_FDTFILE_FALLBACK
#endif


#define BOOTENV_SHARED_EFI                                                \
	"boot_efi_binary="                                                \
		"if fdt addr ${fdt_addr_r}; then "                        \
			"bootefi bootmgr ${fdt_addr_r};"                  \
		"else "                                                   \
			"bootefi bootmgr ${fdtcontroladdr};"              \
		"fi;"                                                     \
		"load ${devtype} ${devnum}:${distro_bootpart} "           \
			"${kernel_addr_r} efi/boot/"BOOTEFI_NAME"; "      \
		"if fdt addr ${fdt_addr_r}; then "                        \
			"bootefi ${kernel_addr_r} ${fdt_addr_r};"         \
		"else "                                                   \
			"bootefi ${kernel_addr_r} ${fdtcontroladdr};"     \
		"fi\0"                                                    \
	\
	"load_efi_dtb="                                                   \
		"load ${devtype} ${devnum}:${distro_bootpart} "           \
			"${fdt_addr_r} ${prefix}${efi_fdtfile}\0"         \
	\
	"efi_dtb_prefixes=/ /dtb/ /dtb/current/\0"                        \
	"scan_dev_for_efi="                                               \
		"setenv efi_fdtfile ${fdtfile}; "                         \
		BOOTENV_EFI_SET_FDTFILE_FALLBACK                          \
		"for prefix in ${efi_dtb_prefixes}; do "                  \
			"if test -e ${devtype} "                          \
					"${devnum}:${distro_bootpart} "   \
					"${prefix}${efi_fdtfile}; then "  \
				"run load_efi_dtb; "                      \
			"fi;"                                             \
		"done;"                                                   \
		"if test -e ${devtype} ${devnum}:${distro_bootpart} "     \
					"efi/boot/"BOOTEFI_NAME"; then "  \
				"echo Found EFI removable media binary "  \
					"efi/boot/"BOOTEFI_NAME"; "       \
				"run boot_efi_binary; "                   \
				"echo EFI LOAD FAILED: continuing...; "   \
		"fi; "                                                    \
		"setenv efi_fdtfile\0"
#define SCAN_DEV_FOR_EFI "run scan_dev_for_efi;"
#else
#define BOOTENV_SHARED_EFI
#define SCAN_DEV_FOR_EFI
#endif

#ifdef CONFIG_SATA
#define BOOTENV_SHARED_SATA	BOOTENV_SHARED_BLKDEV(sata)
#define BOOTENV_DEV_SATA	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_SATA	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_SATA
#define BOOTENV_DEV_SATA \
	BOOT_TARGET_DEVICES_references_SATA_without_CONFIG_SATA
#define BOOTENV_DEV_NAME_SATA \
	BOOT_TARGET_DEVICES_references_SATA_without_CONFIG_SATA
#endif

#ifdef CONFIG_NVME
#define BOOTENV_RUN_NVME_INIT "run nvme_init; "
#define BOOTENV_SET_NVME_NEED_INIT "setenv nvme_need_init; "
#define BOOTENV_SHARED_NVME \
	"nvme_init=" \
		"if ${nvme_need_init}; then " \
			"setenv nvme_need_init false; " \
			"nvme scan; " \
		"fi\0" \
	\
	"nvme_boot=" \
		BOOTENV_RUN_NVME_INIT \
		BOOTENV_SHARED_BLKDEV_BODY(nvme)
#define BOOTENV_DEV_NVME	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_NVME	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_NVME_INIT
#define BOOTENV_SET_NVME_NEED_INIT
#define BOOTENV_SHARED_NVME
#define BOOTENV_DEV_NVME \
	BOOT_TARGET_DEVICES_references_NVME_without_CONFIG_NVME
#define BOOTENV_DEV_NAME_NVME \
	BOOT_TARGET_DEVICES_references_NVME_without_CONFIG_NVME
#endif

#ifdef CONFIG_SCSI
#define BOOTENV_RUN_SCSI_INIT "run scsi_init; "
#define BOOTENV_SET_SCSI_NEED_INIT "scsi_need_init=; "
#define BOOTENV_SHARED_SCSI \
	"scsi_init=" \
		"if ${scsi_need_init}; then " \
			"scsi_need_init=false; " \
			"scsi scan; " \
		"fi\0" \
	\
	"scsi_boot=" \
		BOOTENV_RUN_SCSI_INIT \
		BOOTENV_SHARED_BLKDEV_BODY(scsi)
#define BOOTENV_DEV_SCSI	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_SCSI	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_SCSI_INIT
#define BOOTENV_SET_SCSI_NEED_INIT
#define BOOTENV_SHARED_SCSI
#define BOOTENV_DEV_SCSI \
	BOOT_TARGET_DEVICES_references_SCSI_without_CONFIG_SCSI
#define BOOTENV_DEV_NAME_SCSI \
	BOOT_TARGET_DEVICES_references_SCSI_without_CONFIG_SCSI
#endif

#ifdef CONFIG_IDE
#define BOOTENV_RUN_IDE_INIT "run ide_init; "
#define BOOTENV_SET_IDE_NEED_INIT "setenv ide_need_init; "
#define BOOTENV_SHARED_IDE \
	"ide_init=" \
		"if ${ide_need_init}; then " \
			"setenv ide_need_init false; " \
			"ide reset; " \
		"fi\0" \
	\
	"ide_boot=" \
		BOOTENV_RUN_IDE_INIT \
		BOOTENV_SHARED_BLKDEV_BODY(ide)
#define BOOTENV_DEV_IDE		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_IDE	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_IDE_INIT
#define BOOTENV_SET_IDE_NEED_INIT
#define BOOTENV_SHARED_IDE
#define BOOTENV_DEV_IDE \
	BOOT_TARGET_DEVICES_references_IDE_without_CONFIG_IDE
#define BOOTENV_DEV_NAME_IDE \
	BOOT_TARGET_DEVICES_references_IDE_without_CONFIG_IDE
#endif

#if defined(CONFIG_DM_PCI)
#define BOOTENV_RUN_NET_PCI_ENUM "run boot_net_pci_enum; "
#define BOOTENV_SHARED_PCI \
	"boot_net_pci_enum=pci enum\0"
#else
#define BOOTENV_RUN_NET_PCI_ENUM
#define BOOTENV_SHARED_PCI
#endif

#ifdef CONFIG_CMD_USB
#define BOOTENV_RUN_NET_USB_START "run boot_net_usb_start; "
#define BOOTENV_SHARED_USB \
	"boot_net_usb_start=usb start\0" \
	"usb_boot=" \
		"usb start; " \
		BOOTENV_SHARED_BLKDEV_BODY(usb)
#define BOOTENV_DEV_USB		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_USB	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_NET_USB_START
#define BOOTENV_SHARED_USB
#define BOOTENV_DEV_USB \
	BOOT_TARGET_DEVICES_references_USB_without_CONFIG_CMD_USB
#define BOOTENV_DEV_NAME_USB \
	BOOT_TARGET_DEVICES_references_USB_without_CONFIG_CMD_USB
#endif

#ifdef CONFIG_CMD_VIRTIO
#define BOOTENV_SHARED_VIRTIO	BOOTENV_SHARED_BLKDEV(virtio)
#define BOOTENV_DEV_VIRTIO	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_VIRTIO	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_VIRTIO
#define BOOTENV_DEV_VIRTIO \
	BOOT_TARGET_DEVICES_references_VIRTIO_without_CONFIG_CMD_VIRTIO
#define BOOTENV_DEV_NAME_VIRTIO \
	BOOT_TARGET_DEVICES_references_VIRTIO_without_CONFIG_CMD_VIRTIO
#endif

#if defined(CONFIG_CMD_DHCP)
#if defined(CONFIG_EFI_LOADER)
/* http://www.iana.org/assignments/dhcpv6-parameters/dhcpv6-parameters.xml */
#if defined(CONFIG_ARM64) || defined(__aarch64__)
#define BOOTENV_EFI_PXE_ARCH "0xb"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00011:UNDI:003000"
#elif defined(CONFIG_ARM) || defined(__arm__)
#define BOOTENV_EFI_PXE_ARCH "0xa"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00010:UNDI:003000"
#elif defined(CONFIG_X86) || defined(__x86_64__)
#define BOOTENV_EFI_PXE_ARCH "0x7"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00007:UNDI:003000"
#elif defined(__i386__)
#define BOOTENV_EFI_PXE_ARCH "0x6"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00006:UNDI:003000"
#elif defined(CONFIG_ARCH_RV32I) || ((defined(__riscv) && __riscv_xlen == 32))
#define BOOTENV_EFI_PXE_ARCH "0x19"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00025:UNDI:003000"
#elif defined(CONFIG_ARCH_RV64I) || ((defined(__riscv) && __riscv_xlen == 64))
#define BOOTENV_EFI_PXE_ARCH "0x1b"
#define BOOTENV_EFI_PXE_VCI "PXEClient:Arch:00027:UNDI:003000"
#elif defined(CONFIG_SANDBOX)
# error "sandbox EFI support is only supported on ARM and x86"
#else
#error Please specify an EFI client identifier
#endif

/*
 * Ask the dhcp server for an EFI binary. If we get one, check for a
 * device tree in the same folder. Then boot everything. If the file was
 * not an EFI binary, we just return from the bootefi command and continue.
 */
#define BOOTENV_EFI_RUN_DHCP \
	"setenv efi_fdtfile ${fdtfile}; "                                 \
	BOOTENV_EFI_SET_FDTFILE_FALLBACK                                  \
	"setenv efi_old_vci ${bootp_vci};"                                \
	"setenv efi_old_arch ${bootp_arch};"                              \
	"setenv bootp_vci " BOOTENV_EFI_PXE_VCI ";"                       \
	"setenv bootp_arch " BOOTENV_EFI_PXE_ARCH ";"                     \
	"if dhcp ${kernel_addr_r}; then "                                 \
		"tftpboot ${fdt_addr_r} dtb/${efi_fdtfile};"              \
		"if fdt addr ${fdt_addr_r}; then "                        \
			"bootefi ${kernel_addr_r} ${fdt_addr_r}; "        \
		"else "                                                   \
			"bootefi ${kernel_addr_r} ${fdtcontroladdr};"     \
		"fi;"                                                     \
	"fi;"                                                             \
	"setenv bootp_vci ${efi_old_vci};"                                \
	"setenv bootp_arch ${efi_old_arch};"                              \
	"setenv efi_fdtfile;"                                             \
	"setenv efi_old_arch;"                                            \
	"setenv efi_old_vci;"
#else
#define BOOTENV_EFI_RUN_DHCP
#endif
#define BOOTENV_DEV_DHCP(devtypeu, devtypel, instance) \
	"bootcmd_dhcp=" \
		BOOTENV_RUN_NET_USB_START \
		BOOTENV_RUN_NET_PCI_ENUM \
		"if dhcp ${scriptaddr} ${boot_script_dhcp}; then " \
			"source ${scriptaddr}; " \
		"fi;" \
		BOOTENV_EFI_RUN_DHCP \
		"\0"
#define BOOTENV_DEV_NAME_DHCP(devtypeu, devtypel, instance) \
	"dhcp "
#else
#define BOOTENV_DEV_DHCP \
	BOOT_TARGET_DEVICES_references_DHCP_without_CONFIG_CMD_DHCP
#define BOOTENV_DEV_NAME_DHCP \
	BOOT_TARGET_DEVICES_references_DHCP_without_CONFIG_CMD_DHCP
#endif

#if defined(CONFIG_CMD_DHCP) && defined(CONFIG_CMD_PXE)
#define BOOTENV_DEV_PXE(devtypeu, devtypel, instance) \
	"bootcmd_pxe=" \
		BOOTENV_RUN_NET_USB_START \
		BOOTENV_RUN_NET_PCI_ENUM \
		"dhcp; " \
		"if pxe get; then " \
			"pxe boot; " \
		"fi\0"
#define BOOTENV_DEV_NAME_PXE(devtypeu, devtypel, instance) \
	"pxe "
#else
#define BOOTENV_DEV_PXE \
	BOOT_TARGET_DEVICES_references_PXE_without_CONFIG_CMD_DHCP_or_PXE
#define BOOTENV_DEV_NAME_PXE \
	BOOT_TARGET_DEVICES_references_PXE_without_CONFIG_CMD_DHCP_or_PXE
#endif

#define BOOTENV_DEV_NAME(devtypeu, devtypel, instance) \
	BOOTENV_DEV_NAME_##devtypeu(devtypeu, devtypel, instance)
#define BOOTENV_BOOT_TARGETS \
	"boot_targets=" BOOT_TARGET_DEVICES(BOOTENV_DEV_NAME) "\0"

#define BOOTENV_DEV(devtypeu, devtypel, instance) \
	BOOTENV_DEV_##devtypeu(devtypeu, devtypel, instance)
#define BOOTENV \
	BOOTENV_SHARED_HOST \
	BOOTENV_SHARED_MMC \
	BOOTENV_SHARED_PCI \
	BOOTENV_SHARED_USB \
	BOOTENV_SHARED_SATA \
	BOOTENV_SHARED_SCSI \
	BOOTENV_SHARED_NVME \
	BOOTENV_SHARED_IDE \
	BOOTENV_SHARED_UBIFS \
	BOOTENV_SHARED_EFI \
	BOOTENV_SHARED_VIRTIO \
	"boot_prefixes=/ /boot/\0" \
	"boot_scripts=boot.scr.uimg boot.scr\0" \
	"boot_script_dhcp=boot.scr.uimg\0" \
	BOOTENV_BOOT_TARGETS \
	\
	"boot_syslinux_conf=extlinux/extlinux.conf\0" \
	"boot_extlinux="                                                  \
		"sysboot ${devtype} ${devnum}:${distro_bootpart} any "    \
			"${scriptaddr} ${prefix}${boot_syslinux_conf}\0"  \
	\
	"scan_dev_for_extlinux="                                          \
		"if test -e ${devtype} "                                  \
				"${devnum}:${distro_bootpart} "           \
				"${prefix}${boot_syslinux_conf}; then "   \
			"echo Found ${prefix}${boot_syslinux_conf}; "     \
			"run boot_extlinux; "                             \
			"echo SCRIPT FAILED: continuing...; "             \
		"fi\0"                                                    \
	\
	"boot_a_script="                                                  \
		"load ${devtype} ${devnum}:${distro_bootpart} "           \
			"${scriptaddr} ${prefix}${script}; "              \
		"source ${scriptaddr}\0"                                  \
	\
	"scan_dev_for_scripts="                                           \
		"for script in ${boot_scripts}; do "                      \
			"if test -e ${devtype} "                          \
					"${devnum}:${distro_bootpart} "   \
					"${prefix}${script}; then "       \
				"echo Found U-Boot script "               \
					"${prefix}${script}; "            \
				"run boot_a_script; "                     \
				"echo SCRIPT FAILED: continuing...; "     \
			"fi; "                                            \
		"done\0"                                                  \
	\
	"scan_dev_for_boot="                                              \
		"echo Scanning ${devtype} "                               \
				"${devnum}:${distro_bootpart}...; "       \
		"for prefix in ${boot_prefixes}; do "                     \
			"run scan_dev_for_extlinux; "                     \
			"run scan_dev_for_scripts; "                      \
		"done;"                                                   \
		SCAN_DEV_FOR_EFI                                          \
		"\0"                                                      \
	\
	"scan_dev_for_boot_part="                                         \
		"part list ${devtype} ${devnum} -bootable devplist; "     \
		"env exists devplist || setenv devplist 1; "              \
		"for distro_bootpart in ${devplist}; do "                 \
			"if fstype ${devtype} "                           \
					"${devnum}:${distro_bootpart} "   \
					"bootfstype; then "               \
				"run scan_dev_for_boot; "                 \
			"fi; "                                            \
		"done; "                                                  \
		"setenv devplist\0"					  \
	\
	BOOT_TARGET_DEVICES(BOOTENV_DEV)                                  \
	\
	"distro_bootcmd=" BOOTENV_SET_SCSI_NEED_INIT                      \
		BOOTENV_SET_NVME_NEED_INIT                                \
		BOOTENV_SET_IDE_NEED_INIT                                 \
		"for target in ${boot_targets}; do "                      \
			"run bootcmd_${target}; "                         \
		"done\0"

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "run distro_bootcmd"
#endif

#endif  /* _CONFIG_CMD_DISTRO_BOOTCMD_H */
