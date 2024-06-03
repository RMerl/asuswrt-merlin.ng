/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008-2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_KEYMILE_H
#define __CONFIG_KEYMILE_H

#undef	CONFIG_WATCHDOG		/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_MAXARGS		32 /* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_HUSH_INIT_VAR

#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_LOADS_ECHO
#define CONFIG_SYS_LOADS_BAUD_CHANGE


/* Support the IVM EEprom */
#define	CONFIG_SYS_IVM_EEPROM_ADR	0x50
#define CONFIG_SYS_IVM_EEPROM_MAX_LEN	0x400
#define CONFIG_SYS_IVM_EEPROM_PAGE_LEN	0x100

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/* UBI Support for all Keymile boards */
#define CONFIG_MTD_CONCAT

#ifndef CONFIG_KM_DEF_ENV_BOOTPARAMS
#define CONFIG_KM_DEF_ENV_BOOTPARAMS \
	"actual_bank=0\0"
#endif

#ifndef CONFIG_KM_DEF_NETDEV
#define CONFIG_KM_DEF_NETDEV	\
	"netdev=eth0\0"
#endif

#ifndef CONFIG_KM_UBI_PARTITION_NAME_BOOT
#define CONFIG_KM_UBI_PARTITION_NAME_BOOT	"ubi0"
#endif /* CONFIG_KM_UBI_PARTITION_NAME_BOOT */

#ifndef CONFIG_KM_UBI_PART_BOOT_OPTS
#define CONFIG_KM_UBI_PART_BOOT_OPTS		""
#endif /* CONFIG_KM_UBI_PART_BOOT_OPTS */

#ifndef CONFIG_KM_UBI_PARTITION_NAME_APP
/* one flash chip only called boot */
/* boot: CONFIG_KM_UBI_PARTITION_NAME_BOOT */
# define CONFIG_KM_UBI_LINUX_MTD					\
	"ubi.mtd=" CONFIG_KM_UBI_PARTITION_NAME_BOOT			\
	CONFIG_KM_UBI_PART_BOOT_OPTS
# define CONFIG_KM_DEV_ENV_FLASH_BOOT_UBI				\
	"ubiattach=ubi part " CONFIG_KM_UBI_PARTITION_NAME_BOOT "\0"
#else /* CONFIG_KM_UBI_PARTITION_NAME_APP */
/* two flash chips called boot and app */
/* boot: CONFIG_KM_UBI_PARTITION_NAME_BOOT */
/* app:  CONFIG_KM_UBI_PARTITION_NAME_APP */
# define CONFIG_KM_UBI_LINUX_MTD					\
	"ubi.mtd=" CONFIG_KM_UBI_PARTITION_NAME_BOOT			\
	CONFIG_KM_UBI_PART_BOOT_OPTS " "				\
	"ubi.mtd=" CONFIG_KM_UBI_PARTITION_NAME_APP
# define CONFIG_KM_DEV_ENV_FLASH_BOOT_UBI				\
	"ubiattach=if test ${boot_bank} -eq 0; then; "			\
	"ubi part " CONFIG_KM_UBI_PARTITION_NAME_BOOT "; else; "	\
	"ubi part " CONFIG_KM_UBI_PARTITION_NAME_APP "; fi\0"
#endif /* CONFIG_KM_UBI_PARTITION_NAME_APP */

#ifdef CONFIG_NAND_ECC_BCH
#define CONFIG_KM_UIMAGE_NAME "ecc_bch_uImage\0"
#define CONFIG_KM_ECC_MODE    " eccmode=bch"
#else
#define CONFIG_KM_UIMAGE_NAME "uImage\0"
#define CONFIG_KM_ECC_MODE
#endif

/*
 * boottargets
 * - set 'subbootcmds'
 * - set 'bootcmd' and 'altbootcmd'
 * available targets:
 * - 'release': for a standalone system		kernel/rootfs from flash
 */
#define CONFIG_KM_DEF_ENV_BOOTTARGETS					\
	"subbootcmds=ubiattach ubicopy checkfdt cramfsloadfdt "		\
		"set_fdthigh cramfsloadkernel flashargs add_default "	\
		"addpanic boot\0"					\
	"develop="							\
		"tftp 200000 scripts/develop-${arch}.txt && "		\
		"env import -t 200000 ${filesize} && "			\
		"run setup_debug_env\0"					\
	"ramfs="							\
		"tftp 200000 scripts/ramfs-${arch}.txt && "		\
		"env import -t 200000 ${filesize} && "			\
		"run setup_debug_env\0"					\
	""

/*
 * bootargs
 * - modify 'bootargs'
 *
 * - 'add_default': default bootargs common for all arm/ppc boards
 * - 'addpanic': add kernel panic options
 * - 'flashargs': defaults arguments for flash base boot
 *
 */
#define CONFIG_KM_DEF_ENV_BOOTARGS					\
	"add_default="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off:"				\
		" console=" CONFIG_KM_CONSOLE_TTY ",${baudrate}"	\
		" mem=${kernelmem} init=${init}"			\
		CONFIG_KM_ECC_MODE					\
		" phram.phram=phvar,${varaddr}," __stringify(CONFIG_KM_PHRAM)\
		" " CONFIG_KM_UBI_LINUX_MTD " "				\
		CONFIG_KM_DEF_BOOT_ARGS_CPU				\
		"\0"							\
	"addpanic="							\
		"setenv bootargs ${bootargs} panic=1 panic_on_oops=1\0"	\
	"flashargs="							\
		"setenv bootargs "					\
		"root=mtdblock:rootfs${boot_bank} "			\
		"rootfstype=squashfs ro\0"				\
	""

/*
 * flash_boot
 * - commands for booting from flash
 *
 * - 'cramfsloadkernel': copy kernel from a cramfs to ram
 * - 'ubiattach': attach ubi partition
 * - 'ubicopy': copy ubi volume to ram
 *              - volume names: bootfs0, bootfs1, bootfs2, ...
 *
 * processor specific settings
 * - 'cramfsloadfdt': copy fdt from a cramfs to ram
 */
#define CONFIG_KM_DEF_ENV_FLASH_BOOT					\
	"cramfsaddr=" __stringify(CONFIG_KM_CRAMFS_ADDR) "\0"		\
	"cramfsloadkernel=cramfsload ${load_addr_r} ${uimage}\0"	\
	"ubicopy=ubi read "__stringify(CONFIG_KM_CRAMFS_ADDR)		\
			" bootfs${boot_bank}\0"				\
	"uimage=" CONFIG_KM_UIMAGE_NAME					\
	CONFIG_KM_DEV_ENV_FLASH_BOOT_UBI

/*
 * constants
 * - KM specific constants and commands
 *
 * - 'default': setup default environment
 */
#define CONFIG_KM_DEF_ENV_CONSTANTS					\
	"backup_bank=0\0"						\
	"release=run newenv; reset\0"					\
	"pnvramsize=" __stringify(CONFIG_KM_PNVRAM) "\0"		\
	"testbootcmd=setenv boot_bank ${test_bank}; "			\
		"run ${subbootcmds}; reset\0"				\
	""

#ifndef CONFIG_KM_DEF_ENV
#define CONFIG_KM_DEF_ENV	\
	CONFIG_KM_DEF_ENV_BOOTPARAMS					\
	CONFIG_KM_DEF_NETDEV						\
	CONFIG_KM_DEF_ENV_CPU						\
	CONFIG_KM_DEF_ENV_BOOTTARGETS					\
	CONFIG_KM_DEF_ENV_BOOTARGS					\
	CONFIG_KM_DEF_ENV_FLASH_BOOT					\
	CONFIG_KM_DEF_ENV_CONSTANTS					\
	"altbootcmd=run bootcmd\0"					\
	"boot=bootm ${load_addr_r} - ${fdt_addr_r}\0"			\
	"bootcmd=km_checkbidhwk &&  "					\
		"setenv bootcmd \'if km_checktestboot; then; "          \
				"setenv boot_bank ${test_bank}; else; " \
				"setenv boot_bank ${actual_bank}; fi;"  \
			"run ${subbootcmds}; reset\' && "		\
		"setenv altbootcmd \'setenv boot_bank ${backup_bank}; "	\
			"run ${subbootcmds}; reset\' && "		\
		"saveenv && saveenv && boot\0"				\
	"cramfsloadfdt="						\
		"cramfsload ${fdt_addr_r} "				\
		"fdt_0x${IVM_BoardId}_0x${IVM_HWKey}.dtb\0"		\
	"fdt_addr_r="__stringify(CONFIG_KM_FDT_ADDR) "\0"		\
	"init=/sbin/init-overlay.sh\0"					\
	"load_addr_r="__stringify(CONFIG_KM_KERNEL_ADDR) "\0"		\
	"load=tftpboot ${load_addr_r} ${u-boot}\0"			\
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0"					\
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0"				\
	""
#endif /* CONFIG_KM_DEF_ENV */

#endif /* __CONFIG_KEYMILE_H */
