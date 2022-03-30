/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_KEYMILE_POWERPC_H
#define __CONFIG_KEYMILE_POWERPC_H

/* Do boardspecific init for all boards */

#define CONFIG_JFFS2_CMDLINE

/* standard km ethernet_present for piggy */
#define CONFIG_KM_COMMON_ETH_INIT

/* EEprom support 24C08, 24C16, 24C64 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_ENABLE
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3  /* 8 Byte write page */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CONFIG_ENV_SIZE		0x04000		/* Size of Environment */

#define CONFIG_SYS_MEMTEST_START 0x00100000	/* memtest works on */

#define CONFIG_SYS_MEMTEST_END	0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */

/* Reserve 4 MB for malloc */
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)

/******************************************************************************
 * (PRAM usage)
 * ... -------------------------------------------------------
 * ... |ROOTFSSIZE | PNVRAM |PHRAM |RESERVED_PRAM | END_OF_RAM
 * ... |<------------------- pram -------------------------->|
 * ... -------------------------------------------------------
 * @END_OF_RAM:
 * @CONFIG_KM_RESERVED_PRAM: reserved pram for special purpose
 * @CONFIG_KM_PHRAM: address for /var
 * @CONFIG_KM_PNVRAM: address for PNVRAM (for the application)
 * @CONFIG_KM_ROOTFSSIZE: address for rootfilesystem in RAM
 */

/* size of rootfs in RAM */
#define CONFIG_KM_ROOTFSSIZE	0x0
/* pseudo-non volatile RAM [hex] */
#define CONFIG_KM_PNVRAM	0x80000
/* physical RAM MTD size [hex] */
#define CONFIG_KM_PHRAM		0x100000
/* resereved pram area at the end of memroy [hex] */
#define CONFIG_KM_RESERVED_PRAM	0x0
/* set the default PRAM value to at least PNVRAM + PHRAM when pram env variable
 * is not valid yet, which is the case for when u-boot copies itself to RAM */
#define CONFIG_PRAM		((CONFIG_KM_PNVRAM + CONFIG_KM_PHRAM)>>10)

#define CONFIG_KM_CRAMFS_ADDR	0xC00000
#define CONFIG_KM_KERNEL_ADDR	0x400000	/* 7680Kbytes */
#define CONFIG_KM_FDT_ADDR	0xB80000	/* 512Kbytes */

/* architecture specific default bootargs */
#define CONFIG_KM_DEF_BOOT_ARGS_CPU		""

#define CONFIG_KM_DEF_ENV_CPU						\
	"u-boot="CONFIG_HOSTNAME "/u-boot.bin\0"		\
	"update="							\
		"protect off " __stringify(BOOTFLASH_START) " +${filesize} && "\
		"erase " __stringify(BOOTFLASH_START) "  +${filesize} && "\
		"cp.b ${load_addr_r} " __stringify(BOOTFLASH_START)	\
		"  ${filesize} && "					\
		"protect on " __stringify(BOOTFLASH_START) "  +${filesize}\0"\
	"set_fdthigh=true\0"						\
	"checkfdt=true\0"						\
	""

#endif /* __CONFIG_KEYMILE_POWERPC_H */
