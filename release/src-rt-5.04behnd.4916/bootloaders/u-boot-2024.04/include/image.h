/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "compiler.h"
#include <asm/byteorder.h>
#include <stdbool.h>

/* Define this to avoid #ifdefs later on */
struct lmb;
struct fdt_region;

#ifdef USE_HOSTCC
#include <sys/types.h>
#include <linux/kconfig.h>

#define IMAGE_INDENT_STRING	""

#else

#include <lmb.h>
#include <asm/u-boot.h>
#include <command.h>
#include <linker_lists.h>

#define IMAGE_INDENT_STRING	"   "

#endif /* USE_HOSTCC */

#include <hash.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <u-boot/hash-checksum.h>

extern ulong image_load_addr;		/* Default Load Address */
extern ulong image_save_addr;		/* Default Save Address */
extern ulong image_save_size;		/* Default Save Size */
extern ulong image_load_offset;	/* Default Load Address Offset */

/* An invalid size, meaning that the image size is not known */
#define IMAGE_SIZE_INVAL	(-1UL)

enum ih_category {
	IH_ARCH,
	IH_COMP,
	IH_OS,
	IH_TYPE,
	IH_PHASE,

	IH_COUNT,
};

/*
 * Operating System Codes
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
enum {
	IH_OS_INVALID		= 0,	/* Invalid OS	*/
	IH_OS_OPENBSD,			/* OpenBSD	*/
	IH_OS_NETBSD,			/* NetBSD	*/
	IH_OS_FREEBSD,			/* FreeBSD	*/
	IH_OS_4_4BSD,			/* 4.4BSD	*/
	IH_OS_LINUX,			/* Linux	*/
	IH_OS_SVR4,			/* SVR4		*/
	IH_OS_ESIX,			/* Esix		*/
	IH_OS_SOLARIS,			/* Solaris	*/
	IH_OS_IRIX,			/* Irix		*/
	IH_OS_SCO,			/* SCO		*/
	IH_OS_DELL,			/* Dell		*/
	IH_OS_NCR,			/* NCR		*/
	IH_OS_LYNXOS,			/* LynxOS	*/
	IH_OS_VXWORKS,			/* VxWorks	*/
	IH_OS_PSOS,			/* pSOS		*/
	IH_OS_QNX,			/* QNX		*/
	IH_OS_U_BOOT,			/* Firmware	*/
	IH_OS_RTEMS,			/* RTEMS	*/
	IH_OS_ARTOS,			/* ARTOS	*/
	IH_OS_UNITY,			/* Unity OS	*/
	IH_OS_INTEGRITY,		/* INTEGRITY	*/
	IH_OS_OSE,			/* OSE		*/
	IH_OS_PLAN9,			/* Plan 9	*/
	IH_OS_OPENRTOS,		/* OpenRTOS	*/
	IH_OS_ARM_TRUSTED_FIRMWARE,     /* ARM Trusted Firmware */
	IH_OS_TEE,			/* Trusted Execution Environment */
	IH_OS_OPENSBI,			/* RISC-V OpenSBI */
	IH_OS_EFI,			/* EFI Firmware (e.g. GRUB2) */

	IH_OS_COUNT,
};

/*
 * CPU Architecture Codes (supported by Linux)
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
enum {
	IH_ARCH_INVALID		= 0,	/* Invalid CPU	*/
	IH_ARCH_ALPHA,			/* Alpha	*/
	IH_ARCH_ARM,			/* ARM		*/
	IH_ARCH_I386,			/* Intel x86	*/
	IH_ARCH_IA64,			/* IA64		*/
	IH_ARCH_MIPS,			/* MIPS		*/
	IH_ARCH_MIPS64,			/* MIPS	 64 Bit */
	IH_ARCH_PPC,			/* PowerPC	*/
	IH_ARCH_S390,			/* IBM S390	*/
	IH_ARCH_SH,			/* SuperH	*/
	IH_ARCH_SPARC,			/* Sparc	*/
	IH_ARCH_SPARC64,		/* Sparc 64 Bit */
	IH_ARCH_M68K,			/* M68K		*/
	IH_ARCH_NIOS,			/* Nios-32	*/
	IH_ARCH_MICROBLAZE,		/* MicroBlaze   */
	IH_ARCH_NIOS2,			/* Nios-II	*/
	IH_ARCH_BLACKFIN,		/* Blackfin	*/
	IH_ARCH_AVR32,			/* AVR32	*/
	IH_ARCH_ST200,			/* STMicroelectronics ST200  */
	IH_ARCH_SANDBOX,		/* Sandbox architecture (test only) */
	IH_ARCH_NDS32,			/* ANDES Technology - NDS32  */
	IH_ARCH_OPENRISC,		/* OpenRISC 1000  */
	IH_ARCH_ARM64,			/* ARM64	*/
	IH_ARCH_ARC,			/* Synopsys DesignWare ARC */
	IH_ARCH_X86_64,			/* AMD x86_64, Intel and Via */
	IH_ARCH_XTENSA,			/* Xtensa	*/
	IH_ARCH_RISCV,			/* RISC-V */

	IH_ARCH_COUNT,
};

/*
 * Image Types
 *
 * "Standalone Programs" are directly runnable in the environment
 *	provided by U-Boot; it is expected that (if they behave
 *	well) you can continue to work in U-Boot after return from
 *	the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 *	will take over control completely. Usually these programs
 *	will install their own set of exception handlers, device
 *	drivers, set up the MMU, etc. - this means, that you cannot
 *	expect to re-enter U-Boot except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 *	parameters (address, size) are passed to an OS kernel that is
 *	being started.
 * "Multi-File Images" contain several images, typically an OS
 *	(Linux) kernel image and one or more data images like
 *	RAMDisks. This construct is useful for instance when you want
 *	to boot over the network using BOOTP etc., where the boot
 *	server provides just a single image file, but you want to get
 *	for instance an OS kernel and a RAMDisk image.
 *
 *	"Multi-File Images" start with a list of image sizes, each
 *	image size (in bytes) specified by an "uint32_t" in network
 *	byte order. This list is terminated by an "(uint32_t)0".
 *	Immediately after the terminating 0 follow the images, one by
 *	one, all aligned on "uint32_t" boundaries (size rounded up to
 *	a multiple of 4 bytes - except for the last file).
 *
 * "Firmware Images" are binary images containing firmware (like
 *	U-Boot or FPGA images) which usually will be programmed to
 *	flash memory.
 *
 * "Script files" are command sequences that will be executed by
 *	U-Boot's command interpreter; this feature is especially
 *	useful when you configure U-Boot to use a real shell (hush)
 *	as command interpreter (=> Shell Scripts).
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
enum image_type_t {
	IH_TYPE_INVALID		= 0,	/* Invalid Image		*/
	IH_TYPE_STANDALONE,		/* Standalone Program		*/
	IH_TYPE_KERNEL,			/* OS Kernel Image		*/
	IH_TYPE_RAMDISK,		/* RAMDisk Image		*/
	IH_TYPE_MULTI,			/* Multi-File Image		*/
	IH_TYPE_FIRMWARE,		/* Firmware Image		*/
	IH_TYPE_SCRIPT,			/* Script file			*/
	IH_TYPE_FILESYSTEM,		/* Filesystem Image (any type)	*/
	IH_TYPE_FLATDT,			/* Binary Flat Device Tree Blob	*/
	IH_TYPE_KWBIMAGE,		/* Kirkwood Boot Image		*/
	IH_TYPE_IMXIMAGE,		/* Freescale IMXBoot Image	*/
	IH_TYPE_UBLIMAGE,		/* Davinci UBL Image		*/
	IH_TYPE_OMAPIMAGE,		/* TI OMAP Config Header Image	*/
	IH_TYPE_AISIMAGE,		/* TI Davinci AIS Image		*/
	/* OS Kernel Image, can run from any load address */
	IH_TYPE_KERNEL_NOLOAD,
	IH_TYPE_PBLIMAGE,		/* Freescale PBL Boot Image	*/
	IH_TYPE_MXSIMAGE,		/* Freescale MXSBoot Image	*/
	IH_TYPE_GPIMAGE,		/* TI Keystone GPHeader Image	*/
	IH_TYPE_ATMELIMAGE,		/* ATMEL ROM bootable Image	*/
	IH_TYPE_SOCFPGAIMAGE,		/* Altera SOCFPGA CV/AV Preloader */
	IH_TYPE_X86_SETUP,		/* x86 setup.bin Image		*/
	IH_TYPE_LPC32XXIMAGE,		/* x86 setup.bin Image		*/
	IH_TYPE_LOADABLE,		/* A list of typeless images	*/
	IH_TYPE_RKIMAGE,		/* Rockchip Boot Image		*/
	IH_TYPE_RKSD,			/* Rockchip SD card		*/
	IH_TYPE_RKSPI,			/* Rockchip SPI image		*/
	IH_TYPE_ZYNQIMAGE,		/* Xilinx Zynq Boot Image */
	IH_TYPE_ZYNQMPIMAGE,		/* Xilinx ZynqMP Boot Image */
	IH_TYPE_ZYNQMPBIF,		/* Xilinx ZynqMP Boot Image (bif) */
	IH_TYPE_FPGA,			/* FPGA Image */
	IH_TYPE_VYBRIDIMAGE,	/* VYBRID .vyb Image */
	IH_TYPE_TEE,            /* Trusted Execution Environment OS Image */
	IH_TYPE_FIRMWARE_IVT,		/* Firmware Image with HABv4 IVT */
	IH_TYPE_PMMC,            /* TI Power Management Micro-Controller Firmware */
	IH_TYPE_STM32IMAGE,		/* STMicroelectronics STM32 Image */
	IH_TYPE_SOCFPGAIMAGE_V1,	/* Altera SOCFPGA A10 Preloader	*/
	IH_TYPE_MTKIMAGE,		/* MediaTek BootROM loadable Image */
	IH_TYPE_IMX8MIMAGE,		/* Freescale IMX8MBoot Image	*/
	IH_TYPE_IMX8IMAGE,		/* Freescale IMX8Boot Image	*/
	IH_TYPE_COPRO,			/* Coprocessor Image for remoteproc*/
	IH_TYPE_SUNXI_EGON,		/* Allwinner eGON Boot Image */
	IH_TYPE_SUNXI_TOC0,		/* Allwinner TOC0 Boot Image */
	IH_TYPE_FDT_LEGACY,		/* Binary Flat Device Tree Blob	in a Legacy Image */
	IH_TYPE_RENESAS_SPKG,		/* Renesas SPKG image */
	IH_TYPE_STARFIVE_SPL,		/* StarFive SPL image */

	IH_TYPE_COUNT,			/* Number of image types */
};

/*
 * Compression Types
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
enum {
	IH_COMP_NONE		= 0,	/*  No	 Compression Used	*/
	IH_COMP_GZIP,			/* gzip	 Compression Used	*/
	IH_COMP_BZIP2,			/* bzip2 Compression Used	*/
	IH_COMP_LZMA,			/* lzma  Compression Used	*/
	IH_COMP_LZO,			/* lzo   Compression Used	*/
	IH_COMP_LZ4,			/* lz4   Compression Used	*/
	IH_COMP_ZSTD,			/* zstd   Compression Used	*/

	IH_COMP_COUNT,
};

/**
 * Phases - images intended for particular U-Boot phases (SPL, etc.)
 *
 * @IH_PHASE_NONE: No phase information, can be loaded by any phase
 * @IH_PHASE_U_BOOT: Only for U-Boot proper
 * @IH_PHASE_SPL: Only for SPL
 */
enum image_phase_t {
	IH_PHASE_NONE		= 0,
	IH_PHASE_U_BOOT,
	IH_PHASE_SPL,

	IH_PHASE_COUNT,
};

#define IMAGE_PHASE_SHIFT	8
#define IMAGE_PHASE_MASK	(0xff << IMAGE_PHASE_SHIFT)
#define IMAGE_TYPE_MASK		0xff

/**
 * image_ph() - build a composite value combining and type
 *
 * @phase: Image phase value
 * @type: Image type value
 * Returns: Composite value containing both
 */
static inline int image_ph(enum image_phase_t phase, enum image_type_t type)
{
	return type | (phase << IMAGE_PHASE_SHIFT);
}

/**
 * image_ph_phase() - obtain the phase from a composite phase/type value
 *
 * @image_ph_type: Composite value to convert
 * Returns: Phase value taken from the composite value
 */
static inline int image_ph_phase(int image_ph_type)
{
	return (image_ph_type & IMAGE_PHASE_MASK) >> IMAGE_PHASE_SHIFT;
}

/**
 * image_ph_type() - obtain the type from a composite phase/type value
 *
 * @image_ph_type: Composite value to convert
 * Returns: Type value taken from the composite value
 */
static inline int image_ph_type(int image_ph_type)
{
	return image_ph_type & IMAGE_TYPE_MASK;
}

#define LZ4F_MAGIC	0x184D2204	/* LZ4 Magic Number		*/
#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/* Reused from common.h */
#define ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
struct legacy_img_hdr {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
};

struct image_info {
	ulong		start, end;		/* start/end of blob */
	ulong		image_start, image_len; /* start of image within blob, len of image */
	ulong		load;			/* load addr for the image */
	uint8_t		comp, type, os;		/* compression, type of image, os type */
	uint8_t		arch;			/* CPU architecture */
};

/*
 * Legacy and FIT format headers used by do_bootm() and do_bootm_<os>()
 * routines.
 */
struct bootm_headers {
	/*
	 * Legacy os image header, if it is a multi component image
	 * then boot_get_ramdisk() and get_fdt() will attempt to get
	 * data from second and third component accordingly.
	 */
	struct legacy_img_hdr	*legacy_hdr_os;		/* image header pointer */
	struct legacy_img_hdr	legacy_hdr_os_copy;	/* header copy */
	ulong		legacy_hdr_valid;

	/*
	 * The fit_ members are only used with FIT, but it involves a lot of
	 * #ifdefs to avoid compiling that code. Since FIT is the standard
	 * format, even for SPL, this extra data size seems worth it.
	 */
	const char	*fit_uname_cfg;	/* configuration node unit name */

	void		*fit_hdr_os;	/* os FIT image header */
	const char	*fit_uname_os;	/* os subimage node unit name */
	int		fit_noffset_os;	/* os subimage node offset */

	void		*fit_hdr_rd;	/* init ramdisk FIT image header */
	const char	*fit_uname_rd;	/* init ramdisk subimage node unit name */
	int		fit_noffset_rd;	/* init ramdisk subimage node offset */

	void		*fit_hdr_fdt;	/* FDT blob FIT image header */
	const char	*fit_uname_fdt;	/* FDT blob subimage node unit name */
	int		fit_noffset_fdt;/* FDT blob subimage node offset */

	void		*fit_hdr_setup;	/* x86 setup FIT image header */
	const char	*fit_uname_setup; /* x86 setup subimage node name */
	int		fit_noffset_setup;/* x86 setup subimage node offset */

#ifndef USE_HOSTCC
	struct image_info	os;		/* os image info */
	ulong		ep;		/* entry point of OS */

	ulong		rd_start, rd_end;/* ramdisk start/end */

	char		*ft_addr;	/* flat dev tree address */
	ulong		ft_len;		/* length of flat device tree */

	ulong		initrd_start;
	ulong		initrd_end;
	ulong		cmdline_start;
	ulong		cmdline_end;
	struct bd_info		*kbd;
#endif

	int		verify;		/* env_get("verify")[0] != 'n' */

#define BOOTM_STATE_START	0x00000001
#define BOOTM_STATE_FINDOS	0x00000002
#define BOOTM_STATE_FINDOTHER	0x00000004
#define BOOTM_STATE_LOADOS	0x00000008
#define BOOTM_STATE_RAMDISK	0x00000010
#define BOOTM_STATE_FDT		0x00000020
#define BOOTM_STATE_OS_CMDLINE	0x00000040
#define BOOTM_STATE_OS_BD_T	0x00000080
#define BOOTM_STATE_OS_PREP	0x00000100
#define BOOTM_STATE_OS_FAKE_GO	0x00000200	/* 'Almost' run the OS */
#define BOOTM_STATE_OS_GO	0x00000400
#define BOOTM_STATE_PRE_LOAD	0x00000800
#define BOOTM_STATE_MEASURE	0x00001000
	int		state;

#if defined(CONFIG_LMB) && !defined(USE_HOSTCC)
	struct lmb	lmb;		/* for memory mgmt */
#endif
};

#ifdef CONFIG_LMB
#define images_lmb(_images)	(&(_images)->lmb)
#else
#define images_lmb(_images)	NULL
#endif

extern struct bootm_headers images;

/*
 * Some systems (for example LWMON) have very short watchdog periods;
 * we must make sure to split long operations like memmove() or
 * checksum calculations into reasonable chunks.
 */
#ifndef CHUNKSZ
#define CHUNKSZ (64 * 1024)
#endif

#ifndef CHUNKSZ_CRC32
#define CHUNKSZ_CRC32 (64 * 1024)
#endif

#ifndef CHUNKSZ_MD5
#define CHUNKSZ_MD5 (64 * 1024)
#endif

#ifndef CHUNKSZ_SHA1
#define CHUNKSZ_SHA1 (64 * 1024)
#endif

#define uimage_to_cpu(x)		be32_to_cpu(x)
#define cpu_to_uimage(x)		cpu_to_be32(x)

/*
 * Translation table for entries of a specific type; used by
 * get_table_entry_id() and get_table_entry_name().
 */
typedef struct table_entry {
	int	id;
	char	*sname;		/* short (input) name to find table entry */
	char	*lname;		/* long (output) name to print for messages */
} table_entry_t;

/*
 * Compression type and magic number mapping table.
 */
struct comp_magic_map {
	int		comp_id;
	const char	*name;
	unsigned char	magic[2];
};

/*
 * get_table_entry_id() scans the translation table trying to find an
 * entry that matches the given short name. If a matching entry is
 * found, it's id is returned to the caller.
 */
int get_table_entry_id(const table_entry_t *table,
		const char *table_name, const char *name);
/*
 * get_table_entry_name() scans the translation table trying to find
 * an entry that matches the given id. If a matching entry is found,
 * its long name is returned to the caller.
 */
char *get_table_entry_name(const table_entry_t *table, char *msg, int id);

const char *genimg_get_os_name(uint8_t os);

/**
 * genimg_get_os_short_name() - get the short name for an OS
 *
 * @param os	OS (IH_OS_...)
 * Return: OS short name, or "unknown" if unknown
 */
const char *genimg_get_os_short_name(uint8_t comp);

const char *genimg_get_arch_name(uint8_t arch);

/**
 * genimg_get_phase_name() - Get the friendly name for a phase
 *
 * @phase: Phase value to look up
 * Returns: Friendly name for the phase (e.g. "U-Boot phase")
 */
const char *genimg_get_phase_name(enum image_phase_t phase);

/**
 * genimg_get_phase_id() - Convert a phase name to an ID
 *
 * @name: Name to convert (e.g. "u-boot")
 * Returns: ID for that phase (e.g. IH_PHASE_U_BOOT)
 */
int genimg_get_phase_id(const char *name);

/**
 * genimg_get_arch_short_name() - get the short name for an architecture
 *
 * @param arch	Architecture type (IH_ARCH_...)
 * Return: architecture short name, or "unknown" if unknown
 */
const char *genimg_get_arch_short_name(uint8_t arch);

const char *genimg_get_type_name(uint8_t type);

/**
 * genimg_get_type_short_name() - get the short name for an image type
 *
 * @param type	Image type (IH_TYPE_...)
 * Return: image short name, or "unknown" if unknown
 */
const char *genimg_get_type_short_name(uint8_t type);

const char *genimg_get_comp_name(uint8_t comp);

/**
 * genimg_get_comp_short_name() - get the short name for a compression method
 *
 * @param comp	compression method (IH_COMP_...)
 * Return: compression method short name, or "unknown" if unknown
 */
const char *genimg_get_comp_short_name(uint8_t comp);

/**
 * genimg_get_cat_name() - Get the name of an item in a category
 *
 * @category:	Category of item
 * @id:		Item ID
 * Return: name of item, or "Unknown ..." if unknown
 */
const char *genimg_get_cat_name(enum ih_category category, uint id);

/**
 * genimg_get_cat_short_name() - Get the short name of an item in a category
 *
 * @category:	Category of item
 * @id:		Item ID
 * Return: short name of item, or "Unknown ..." if unknown
 */
const char *genimg_get_cat_short_name(enum ih_category category, uint id);

/**
 * genimg_get_cat_count() - Get the number of items in a category
 *
 * @category:	Category to check
 * Return: the number of items in the category (IH_xxx_COUNT)
 */
int genimg_get_cat_count(enum ih_category category);

/**
 * genimg_get_cat_desc() - Get the description of a category
 *
 * @category:	Category to check
 * Return: the description of a category, e.g. "architecture". This
 * effectively converts the enum to a string.
 */
const char *genimg_get_cat_desc(enum ih_category category);

/**
 * genimg_cat_has_id() - Check whether a category has an item
 *
 * @category:	Category to check
 * @id:		Item ID
 * Return: true or false as to whether a category has an item
 */
bool genimg_cat_has_id(enum ih_category category, uint id);

int genimg_get_os_id(const char *name);
int genimg_get_arch_id(const char *name);
int genimg_get_type_id(const char *name);
int genimg_get_comp_id(const char *name);
void genimg_print_size(uint32_t size);

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || defined(USE_HOSTCC)
#define IMAGE_ENABLE_TIMESTAMP 1
#else
#define IMAGE_ENABLE_TIMESTAMP 0
#endif
void genimg_print_time(time_t timestamp);

/* What to do with a image load address ('load = <> 'in the FIT) */
enum fit_load_op {
	FIT_LOAD_IGNORED,	/* Ignore load address */
	FIT_LOAD_OPTIONAL,	/* Can be provided, but optional */
	FIT_LOAD_OPTIONAL_NON_ZERO,	/* Optional, a value of 0 is ignored */
	FIT_LOAD_REQUIRED,	/* Must be provided */
};

int boot_get_setup(struct bootm_headers *images, uint8_t arch, ulong *setup_start,
		   ulong *setup_len);

/* Image format types, returned by _get_format() routine */
#define IMAGE_FORMAT_INVALID	0x00
#define IMAGE_FORMAT_LEGACY	0x01	/* legacy image_header based format */
#define IMAGE_FORMAT_FIT	0x02	/* new, libfdt based format */
#define IMAGE_FORMAT_ANDROID	0x03	/* Android boot image */

/**
 * genimg_get_kernel_addr_fit() - Parse FIT specifier
 *
 * Get the real kernel start address from a string which is normally the first
 * argv of bootm/bootz
 *
 * These cases are dealt with, based on the value of @img_addr:
 *    NULL: Returns image_load_addr, does not set last two args
 *    "<addr>": Returns address
 *
 * For FIT:
 *    "[<addr>]#<conf>": Returns address (or image_load_addr),
 *	sets fit_uname_config to config name
 *    "[<addr>]:<subimage>": Returns address (or image_load_addr) and sets
 *	fit_uname_kernel to the subimage name
 *
 * @img_addr: a string might contain real image address (or NULL)
 * @fit_uname_config: Returns configuration unit name
 * @fit_uname_kernel: Returns subimage name
 *
 * Returns: kernel start address
 */
ulong genimg_get_kernel_addr_fit(const char *const img_addr,
				 const char **fit_uname_config,
				 const char **fit_uname_kernel);

ulong genimg_get_kernel_addr(char * const img_addr);
int genimg_get_format(const void *img_addr);
int genimg_has_config(struct bootm_headers *images);

/**
 * boot_get_fpga() - Locate the FPGA image
 *
 * @images: Information about images being loaded
 * Return 0 if OK, non-zero on failure
 */
int boot_get_fpga(struct bootm_headers *images);

/**
 * boot_get_ramdisk() - Locate the ramdisk
 *
 * @select: address or name of ramdisk to use, or NULL for default
 * @images: pointer to the bootm images structure
 * @arch: expected ramdisk architecture
 * @rd_start: pointer to a ulong variable, will hold ramdisk start address
 * @rd_end: pointer to a ulong variable, will hold ramdisk end
 *
 * boot_get_ramdisk() is responsible for finding a valid ramdisk image.
 * Currently supported are the following ramdisk sources:
 *      - multicomponent kernel/ramdisk image,
 *      - commandline provided address of decicated ramdisk image.
 *
 * returns:
 *     0, if ramdisk image was found and valid, or skiped
 *     rd_start and rd_end are set to ramdisk start/end addresses if
 *     ramdisk image is found and valid
 *
 *     1, if ramdisk image is found but corrupted, or invalid
 *     rd_start and rd_end are set to 0 if no ramdisk exists
 */
int boot_get_ramdisk(char const *select, struct bootm_headers *images,
		     uint arch, ulong *rd_start, ulong *rd_end);

/**
 * boot_get_loadable() - load a list of binaries to memory
 *
 * @images: pointer to the bootm images structure
 *
 * Takes the given FIT configuration, then looks for a field named
 * "loadables", a list of elements in the FIT given as strings, e.g.:
 *   loadables = "linux_kernel", "fdt-2";
 *
 * Each string is parsed, loading the corresponding element from the FIT into
 * memory.  Once placed, no additional actions are taken.
 *
 * Return:
 *     0, if only valid images or no images are found
 *     error code, if an error occurs during fit_image_load
 */
int boot_get_loadable(struct bootm_headers *images);

int boot_get_setup_fit(struct bootm_headers *images, uint8_t arch,
		       ulong *setup_start, ulong *setup_len);

/**
 * boot_get_fdt_fit() - load a DTB from a FIT file (applying overlays)
 *
 * This deals with all aspects of loading an DTB from a FIT.
 * The correct base image based on configuration will be selected, and
 * then any overlays specified will be applied (as present in fit_uname_configp).
 *
 * @param images	Boot images structure
 * @param addr		Address of FIT in memory
 * @param fit_unamep	On entry this is the requested image name
 *			(e.g. "kernel") or NULL to use the default. On exit
 *			points to the selected image name
 * @param fit_uname_configp	On entry this is the requested configuration
 *			name (e.g. "conf-1") or NULL to use the default. On
 *			exit points to the selected configuration name.
 * @param arch		Expected architecture (IH_ARCH_...)
 * @param datap		Returns address of loaded image
 * @param lenp		Returns length of loaded image
 *
 * Return: node offset of base image, or -ve error code on error
 */
int boot_get_fdt_fit(struct bootm_headers *images, ulong addr,
		     const char **fit_unamep, const char **fit_uname_configp,
		     int arch, ulong *datap, ulong *lenp);

/**
 * fit_image_load() - load an image from a FIT
 *
 * This deals with all aspects of loading an image from a FIT, including
 * selecting the right image based on configuration, verifying it, printing
 * out progress messages, checking the type/arch/os and optionally copying it
 * to the right load address.
 *
 * The property to look up is defined by image_type.
 *
 * @param images	Boot images structure
 * @param addr		Address of FIT in memory
 * @param fit_unamep	On entry this is the requested image name
 *			(e.g. "kernel") or NULL to use the default. On exit
 *			points to the selected image name
 * @param fit_uname_configp	On entry this is the requested configuration
 *			name (e.g. "conf-1") or NULL to use the default. On
 *			exit points to the selected configuration name.
 * @param arch		Expected architecture (IH_ARCH_...)
 * @param image_ph_type	Required image type (IH_TYPE_...). If this is
 *			IH_TYPE_KERNEL then we allow IH_TYPE_KERNEL_NOLOAD
 *			also. If a phase is required, this is included also,
 *			see image_phase_and_type()
 * @param bootstage_id	ID of starting bootstage to use for progress updates.
 *			This will be added to the BOOTSTAGE_SUB values when
 *			calling bootstage_mark()
 * @param load_op	Decribes what to do with the load address
 * @param datap		Returns address of loaded image
 * @param lenp		Returns length of loaded image
 * Return: node offset of image, or -ve error code on error:
 *   -ENOEXEC - unsupported architecture
 *   -ENOENT - could not find image / subimage
 *   -EACCES - hash, signature or decryptions failure
 *   -EBADF - invalid OS or image type, or cannot get image load-address
 *   -EXDEV - memory overwritten / overlap
 *   -NOEXEC - image decompression error, or invalid FDT
 */
int fit_image_load(struct bootm_headers *images, ulong addr,
		   const char **fit_unamep, const char **fit_uname_configp,
		   int arch, int image_ph_type, int bootstage_id,
		   enum fit_load_op load_op, ulong *datap, ulong *lenp);

/**
 * image_locate_script() - Locate the raw script in an image
 *
 * @buf: Address of image
 * @size: Size of image in bytes
 * @fit_uname: Node name of FIT image to read
 * @confname: Node name of FIT config to read
 * @datap: Returns pointer to raw script on success
 * @lenp: Returns size of raw script on success
 * @return 0 if OK, non-zero on error
 */
int image_locate_script(void *buf, int size, const char *fit_uname,
			const char *confname, char **datap, uint *lenp);

/**
 * fit_get_node_from_config() - Look up an image a FIT by type
 *
 * This looks in the selected conf- node (images->fit_uname_cfg) for a
 * particular image type (e.g. "kernel") and then finds the image that is
 * referred to.
 *
 * For example, for something like:
 *
 * images {
 *	kernel {
 *		...
 *	};
 * };
 * configurations {
 *	conf-1 {
 *		kernel = "kernel";
 *	};
 * };
 *
 * the function will return the node offset of the kernel@1 node, assuming
 * that conf-1 is the chosen configuration.
 *
 * @param images	Boot images structure
 * @param prop_name	Property name to look up (FIT_..._PROP)
 * @param addr		Address of FIT in memory
 */
int fit_get_node_from_config(struct bootm_headers *images,
			     const char *prop_name, ulong addr);

/**
 * boot_get_fdt() - locate FDT devicetree to use for booting
 *
 * @buf: Pointer to image
 * @select: FDT to select (this is normally argv[2] of the bootm command)
 * @arch: architecture (IH_ARCH_...)
 * @images: pointer to the bootm images structure
 * @of_flat_tree: pointer to a char* variable, will hold fdt start address
 * @of_size: pointer to a ulong variable, will hold fdt length
 *
 * boot_get_fdt() is responsible for finding a valid flat device tree image.
 * Currently supported are the following FDT sources:
 *      - multicomponent kernel/ramdisk/FDT image,
 *      - commandline provided address of decicated FDT image.
 *
 * Return:
 *     0, if fdt image was found and valid, or skipped
 *     of_flat_tree and of_size are set to fdt start address and length if
 *     fdt image is found and valid
 *
 *     1, if fdt image is found but corrupted
 *     of_flat_tree and of_size are set to 0 if no fdt exists
 */
int boot_get_fdt(void *buf, const char *select, uint arch,
		 struct bootm_headers *images, char **of_flat_tree,
		 ulong *of_size);

void boot_fdt_add_mem_rsv_regions(struct lmb *lmb, void *fdt_blob);
int boot_relocate_fdt(struct lmb *lmb, char **of_flat_tree, ulong *of_size);

int boot_ramdisk_high(struct lmb *lmb, ulong rd_data, ulong rd_len,
		  ulong *initrd_start, ulong *initrd_end);
int boot_get_cmdline(struct lmb *lmb, ulong *cmd_start, ulong *cmd_end);
int boot_get_kbd(struct lmb *lmb, struct bd_info **kbd);

/*******************************************************************/
/* Legacy format specific code (prefixed with image_) */
/*******************************************************************/
static inline uint32_t image_get_header_size(void)
{
	return sizeof(struct legacy_img_hdr);
}

#define image_get_hdr_l(f) \
	static inline uint32_t image_get_##f(const struct legacy_img_hdr *hdr) \
	{ \
		return uimage_to_cpu(hdr->ih_##f); \
	}
image_get_hdr_l(magic)		/* image_get_magic */
image_get_hdr_l(hcrc)		/* image_get_hcrc */
image_get_hdr_l(time)		/* image_get_time */
image_get_hdr_l(size)		/* image_get_size */
image_get_hdr_l(load)		/* image_get_load */
image_get_hdr_l(ep)		/* image_get_ep */
image_get_hdr_l(dcrc)		/* image_get_dcrc */

#define image_get_hdr_b(f) \
	static inline uint8_t image_get_##f(const struct legacy_img_hdr *hdr) \
	{ \
		return hdr->ih_##f; \
	}
image_get_hdr_b(os)		/* image_get_os */
image_get_hdr_b(arch)		/* image_get_arch */
image_get_hdr_b(type)		/* image_get_type */
image_get_hdr_b(comp)		/* image_get_comp */

static inline char *image_get_name(const struct legacy_img_hdr *hdr)
{
	return (char *)hdr->ih_name;
}

static inline uint32_t image_get_data_size(const struct legacy_img_hdr *hdr)
{
	return image_get_size(hdr);
}

/**
 * image_get_data - get image payload start address
 * @hdr: image header
 *
 * image_get_data() returns address of the image payload. For single
 * component images it is image data start. For multi component
 * images it points to the null terminated table of sub-images sizes.
 *
 * returns:
 *     image payload data start address
 */
static inline ulong image_get_data(const struct legacy_img_hdr *hdr)
{
	return ((ulong)hdr + image_get_header_size());
}

static inline uint32_t image_get_image_size(const struct legacy_img_hdr *hdr)
{
	return (image_get_size(hdr) + image_get_header_size());
}

static inline ulong image_get_image_end(const struct legacy_img_hdr *hdr)
{
	return ((ulong)hdr + image_get_image_size(hdr));
}

#define image_set_hdr_l(f) \
	static inline void image_set_##f(struct legacy_img_hdr *hdr, uint32_t val) \
	{ \
		hdr->ih_##f = cpu_to_uimage(val); \
	}
image_set_hdr_l(magic)		/* image_set_magic */
image_set_hdr_l(hcrc)		/* image_set_hcrc */
image_set_hdr_l(time)		/* image_set_time */
image_set_hdr_l(size)		/* image_set_size */
image_set_hdr_l(load)		/* image_set_load */
image_set_hdr_l(ep)		/* image_set_ep */
image_set_hdr_l(dcrc)		/* image_set_dcrc */

#define image_set_hdr_b(f) \
	static inline void image_set_##f(struct legacy_img_hdr *hdr, uint8_t val) \
	{ \
		hdr->ih_##f = val; \
	}
image_set_hdr_b(os)		/* image_set_os */
image_set_hdr_b(arch)		/* image_set_arch */
image_set_hdr_b(type)		/* image_set_type */
image_set_hdr_b(comp)		/* image_set_comp */

static inline void image_set_name(struct legacy_img_hdr *hdr, const char *name)
{
	/*
	 * This is equivalent to: strncpy(image_get_name(hdr), name, IH_NMLEN);
	 *
	 * Use the tortured code below to avoid a warning with gcc 12. We do not
	 * want to include a nul terminator if the name is of length IH_NMLEN
	 */
	memcpy(image_get_name(hdr), name, strnlen(name, IH_NMLEN));
}

int image_check_hcrc(const struct legacy_img_hdr *hdr);
int image_check_dcrc(const struct legacy_img_hdr *hdr);
#ifndef USE_HOSTCC
ulong env_get_bootm_low(void);
phys_size_t env_get_bootm_size(void);
phys_size_t env_get_bootm_mapsize(void);
#endif
void memmove_wd(void *to, void *from, size_t len, ulong chunksz);

static inline int image_check_magic(const struct legacy_img_hdr *hdr)
{
	return (image_get_magic(hdr) == IH_MAGIC);
}

static inline int image_check_type(const struct legacy_img_hdr *hdr, uint8_t type)
{
	return (image_get_type(hdr) == type);
}

static inline int image_check_arch(const struct legacy_img_hdr *hdr, uint8_t arch)
{
	/* Let's assume that sandbox can load any architecture */
	if (!tools_build() && IS_ENABLED(CONFIG_SANDBOX))
		return true;
	return (image_get_arch(hdr) == arch) ||
		(image_get_arch(hdr) == IH_ARCH_ARM && arch == IH_ARCH_ARM64);
}

static inline int image_check_os(const struct legacy_img_hdr *hdr, uint8_t os)
{
	return (image_get_os(hdr) == os);
}

ulong image_multi_count(const struct legacy_img_hdr *hdr);
void image_multi_getimg(const struct legacy_img_hdr *hdr, ulong idx,
			ulong *data, ulong *len);

void image_print_contents(const void *hdr);

#ifndef USE_HOSTCC
static inline int image_check_target_arch(const struct legacy_img_hdr *hdr)
{
#ifndef IH_ARCH_DEFAULT
# error "please define IH_ARCH_DEFAULT in your arch asm/u-boot.h"
#endif
	return image_check_arch(hdr, IH_ARCH_DEFAULT);
}
#endif /* USE_HOSTCC */

/**
 * image_decomp_type() - Find out compression type of an image
 *
 * @buf:	Address in U-Boot memory where image is loaded.
 * @len:	Length of the compressed image.
 * Return:	compression type or IH_COMP_NONE if not compressed.
 *
 * Note: Only following compression types are supported now.
 * lzo, lzma, gzip, bzip2
 */
int image_decomp_type(const unsigned char *buf, ulong len);

/**
 * image_decomp() - decompress an image
 *
 * @comp:	Compression algorithm that is used (IH_COMP_...)
 * @load:	Destination load address in U-Boot memory
 * @image_start Image start address (where we are decompressing from)
 * @type:	OS type (IH_OS_...)
 * @load_buf:	Place to decompress to
 * @image_buf:	Address to decompress from
 * @image_len:	Number of bytes in @image_buf to decompress
 * @unc_len:	Available space for decompression
 * Return: 0 if OK, -ve on error (BOOTM_ERR_...)
 */
int image_decomp(int comp, ulong load, ulong image_start, int type,
		 void *load_buf, void *image_buf, ulong image_len,
		 uint unc_len, ulong *load_end);

/**
 * Set up properties in the FDT
 *
 * This sets up properties in the FDT that is to be passed to linux.
 *
 * @images:	Images information
 * @blob:	FDT to update
 * @lmb:	Points to logical memory block structure
 * Return: 0 if ok, <0 on failure
 */
int image_setup_libfdt(struct bootm_headers *images, void *blob,
		       struct lmb *lmb);

/**
 * Set up the FDT to use for booting a kernel
 *
 * This performs ramdisk setup, sets up the FDT if required, and adds
 * paramters to the FDT if libfdt is available.
 *
 * @param images	Images information
 * Return: 0 if ok, <0 on failure
 */
int image_setup_linux(struct bootm_headers *images);

/**
 * bootz_setup() - Extract stat and size of a Linux xImage
 *
 * @image: Address of image
 * @start: Returns start address of image
 * @end : Returns end address of image
 * Return: 0 if OK, 1 if the image was not recognised
 */
int bootz_setup(ulong image, ulong *start, ulong *end);

/**
 * Return the correct start address and size of a Linux aarch64 Image.
 *
 * @image: Address of image
 * @start: Returns start address of image
 * @size : Returns size image
 * @force_reloc: Ignore image->ep field, always place image to RAM start
 * Return: 0 if OK, 1 if the image was not recognised
 */
int booti_setup(ulong image, ulong *relocated_addr, ulong *size,
		bool force_reloc);

/*******************************************************************/
/* New uImage format specific code (prefixed with fit_) */
/*******************************************************************/

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature/key node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"
#define FIT_KEY_REQUIRED	"required"
#define FIT_KEY_HINT		"key-name-hint"

/* cipher node */
#define FIT_CIPHER_NODENAME	"cipher"
#define FIT_ALGO_PROP		"algo"

/* image node */
#define FIT_DATA_PROP		"data"
#define FIT_DATA_POSITION_PROP	"data-position"
#define FIT_DATA_OFFSET_PROP	"data-offset"
#define FIT_DATA_SIZE_PROP	"data-size"
#define FIT_TIMESTAMP_PROP	"timestamp"
#define FIT_DESC_PROP		"description"
#define FIT_ARCH_PROP		"arch"
#define FIT_TYPE_PROP		"type"
#define FIT_OS_PROP		"os"
#define FIT_COMP_PROP		"compression"
#define FIT_ENTRY_PROP		"entry"
#define FIT_LOAD_PROP		"load"

/* configuration node */
#define FIT_KERNEL_PROP		"kernel"
#define FIT_RAMDISK_PROP	"ramdisk"
#define FIT_FDT_PROP		"fdt"
#define FIT_LOADABLE_PROP	"loadables"
#define FIT_DEFAULT_PROP	"default"
#define FIT_SETUP_PROP		"setup"
#define FIT_FPGA_PROP		"fpga"
#define FIT_FIRMWARE_PROP	"firmware"
#define FIT_STANDALONE_PROP	"standalone"
#define FIT_SCRIPT_PROP		"script"
#define FIT_PHASE_PROP		"phase"

#define FIT_MAX_HASH_LEN	HASH_MAX_DIGEST_SIZE

/* cmdline argument format parsing */
int fit_parse_conf(const char *spec, ulong addr_curr,
		ulong *addr, const char **conf_name);
int fit_parse_subimage(const char *spec, ulong addr_curr,
		ulong *addr, const char **image_name);

int fit_get_subimage_count(const void *fit, int images_noffset);
void fit_print_contents(const void *fit);
void fit_image_print(const void *fit, int noffset, const char *p);

/**
 * fit_get_end - get FIT image size
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     size of the FIT image (blob) in memory
 */
static inline ulong fit_get_size(const void *fit)
{
	return fdt_totalsize(fit);
}

/**
 * fit_get_end - get FIT image end
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     end address of the FIT image (blob) in memory
 */
ulong fit_get_end(const void *fit);

/**
 * fit_get_name - get FIT node name
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     NULL, on error
 *     pointer to node name, on success
 */
static inline const char *fit_get_name(const void *fit_hdr,
		int noffset, int *len)
{
	return fdt_get_name(fit_hdr, noffset, len);
}

int fit_get_desc(const void *fit, int noffset, char **desc);
int fit_get_timestamp(const void *fit, int noffset, time_t *timestamp);

int fit_image_get_node(const void *fit, const char *image_uname);
int fit_image_get_os(const void *fit, int noffset, uint8_t *os);
int fit_image_get_arch(const void *fit, int noffset, uint8_t *arch);
int fit_image_get_type(const void *fit, int noffset, uint8_t *type);
int fit_image_get_comp(const void *fit, int noffset, uint8_t *comp);
int fit_image_get_load(const void *fit, int noffset, ulong *load);
int fit_image_get_entry(const void *fit, int noffset, ulong *entry);
int fit_image_get_data(const void *fit, int noffset,
				const void **data, size_t *size);
int fit_image_get_data_offset(const void *fit, int noffset, int *data_offset);
int fit_image_get_data_position(const void *fit, int noffset,
				int *data_position);
int fit_image_get_data_size(const void *fit, int noffset, int *data_size);
int fit_image_get_data_size_unciphered(const void *fit, int noffset,
				       size_t *data_size);
int fit_image_get_data_and_size(const void *fit, int noffset,
				const void **data, size_t *size);

/**
 * fit_get_data_node() - Get verified image data for an image
 * @fit: Pointer to the FIT format image header
 * @image_uname: The name of the image node
 * @data: A pointer which will be filled with the location of the image data
 * @size: A pointer which will be filled with the size of the image data
 *
 * This function looks up the location and size of an image specified by its
 * name. For example, if you had a FIT like::
 *
 *     images {
 *         my-firmware {
 *             ...
 *	   };
 *      };
 *
 * Then you could look up the data location and size of the my-firmware image
 * by calling this function with @image_uname set to "my-firmware". This
 * function also verifies the image data (if enabled) before returning. The
 * image description is printed out on success. @data and @size will not be
 * modified on faulure.
 *
 * Return:
 * * 0 on success
 * * -EINVAL if the image could not be verified
 * * -ENOENT if there was a problem getting the data/size
 * * Another negative error if there was a problem looking up the image node.
 */
int fit_get_data_node(const void *fit, const char *image_uname,
		      const void **data, size_t *size);

/**
 * fit_get_data_conf_prop() - Get verified image data for a property in /conf
 * @fit: Pointer to the FIT format image header
 * @prop_name: The name of the property in /conf referencing the image
 * @data: A pointer which will be filled with the location of the image data
 * @size: A pointer which will be filled with the size of the image data
 *
 * This function looks up the location and size of an image specified by a
 * property in /conf. For example, if you had a FIT like::
 *
 *     images {
 *         my-firmware {
 *             ...
 *	   };
 *      };
 *
 *      configurations {
 *          default = "conf-1";
 *          conf-1 {
 *              some-firmware = "my-firmware";
 *          };
 *      };
 *
 * Then you could look up the data location and size of the my-firmware image
 * by calling this function with @prop_name set to "some-firmware". This
 * function also verifies the image data (if enabled) before returning. The
 * image description is printed out on success. @data and @size will not be
 * modified on faulure.
 *
 * Return:
 * * 0 on success
 * * -EINVAL if the image could not be verified
 * * -ENOENT if there was a problem getting the data/size
 * * Another negative error if there was a problem looking up the configuration
 *   or image node.
 */
int fit_get_data_conf_prop(const void *fit, const char *prop_name,
			   const void **data, size_t *size);

int fit_image_hash_get_algo(const void *fit, int noffset, const char **algo);
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len);

int fit_set_timestamp(void *fit, int noffset, time_t timestamp);

/**
 * fit_pre_load_data() - add public key to fdt blob
 *
 * Adds public key to the node pre load.
 *
 * @keydir:	Directory containing keys
 * @keydest:	FDT blob to write public key
 * @fit:	Pointer to the FIT format image header
 *
 * returns:
 *	0, on success
 *	< 0, on failure
 */
int fit_pre_load_data(const char *keydir, void *keydest, void *fit);

int fit_cipher_data(const char *keydir, void *keydest, void *fit,
		    const char *comment, int require_keys,
		    const char *engine_id, const char *cmdname);

#define NODE_MAX_NAME_LEN	80

/**
 * struct image_summary  - Provides information about signing info added
 *
 * @sig_offset: Offset of the node in the blob devicetree where the signature
 *	was wriiten
 * @sig_path: Path to @sig_offset
 * @keydest_offset: Offset of the node in the keydest devicetree where the
 *	public key was written (-1 if none)
 * @keydest_path: Path to @keydest_offset
 */
struct image_summary {
	int sig_offset;
	char sig_path[NODE_MAX_NAME_LEN];
	int keydest_offset;
	char keydest_path[NODE_MAX_NAME_LEN];
};

/**
 * fit_add_verification_data() - add verification data to FIT image nodes
 *
 * @keydir:	Directory containing keys
 * @kwydest:	FDT blob to write public key information to (NULL if none)
 * @fit:	Pointer to the FIT format image header
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * @cmdname:	Command name used when reporting errors
 * @algo_name:	Algorithm name, or NULL if to be read from FIT
 * @summary:	Returns information about what data was written
 *
 * Adds hash values for all component images in the FIT blob.
 * Hashes are calculated for all component images which have hash subnodes
 * with algorithm property set to one of the supported hash algorithms.
 *
 * Also add signatures if signature nodes are present.
 *
 * returns
 *     0, on success
 *     libfdt error code, on failure
 */
int fit_add_verification_data(const char *keydir, const char *keyfile,
			      void *keydest, void *fit, const char *comment,
			      int require_keys, const char *engine_id,
			      const char *cmdname, const char *algo_name,
			      struct image_summary *summary);

/**
 * fit_image_verify_with_data() - Verify an image with given data
 *
 * @fit:	Pointer to the FIT format image header
 * @image_offset: Offset in @fit of image to verify
 * @key_blob:	FDT containing public keys
 * @data:	Image data to verify
 * @size:	Size of image data
 */
int fit_image_verify_with_data(const void *fit, int image_noffset,
			       const void *key_blob, const void *data,
			       size_t size);

int fit_image_verify(const void *fit, int noffset);
#if CONFIG_IS_ENABLED(FIT_SIGNATURE)
int fit_config_verify(const void *fit, int conf_noffset);
#else
static inline int fit_config_verify(const void *fit, int conf_noffset)
{
	return 0;
}
#endif
int fit_all_image_verify(const void *fit);
int fit_config_decrypt(const void *fit, int conf_noffset);
int fit_image_check_os(const void *fit, int noffset, uint8_t os);
int fit_image_check_arch(const void *fit, int noffset, uint8_t arch);
int fit_image_check_type(const void *fit, int noffset, uint8_t type);
int fit_image_check_comp(const void *fit, int noffset, uint8_t comp);

/**
 * fit_check_format() - Check that the FIT is valid
 *
 * This performs various checks on the FIT to make sure it is suitable for
 * use, looking for mandatory properties, nodes, etc.
 *
 * If FIT_FULL_CHECK is enabled, it also runs it through libfdt to make
 * sure that there are no strange tags or broken nodes in the FIT.
 *
 * @fit: pointer to the FIT format image header
 * Return: 0 if OK, -ENOEXEC if not an FDT file, -EINVAL if the full FDT check
 *	failed (e.g. due to bad structure), -ENOMSG if the description is
 *	missing, -EBADMSG if the timestamp is missing, -ENOENT if the /images
 *	path is missing
 */
int fit_check_format(const void *fit, ulong size);

/**
 * fit_conf_find_compat() - find most compatible configuration
 * @fit: pointer to the FIT format image header
 * @fdt: pointer to the device tree to compare against
 *
 * Attempts to find the configuration whose fdt is the most compatible with the
 * passed in device tree
 *
 * Example::
 *
 *    / o image-tree
 *      |-o images
 *      | |-o fdt-1
 *      | |-o fdt-2
 *      |
 *      |-o configurations
 *        |-o config-1
 *        | |-fdt = fdt-1
 *        |
 *        |-o config-2
 *          |-fdt = fdt-2
 *
 *    / o U-Boot fdt
 *      |-compatible = "foo,bar", "bim,bam"
 *
 *    / o kernel fdt1
 *      |-compatible = "foo,bar",
 *
 *    / o kernel fdt2
 *      |-compatible = "bim,bam", "baz,biz"
 *
 * Configuration 1 would be picked because the first string in U-Boot's
 * compatible list, "foo,bar", matches a compatible string in the root of fdt1.
 * "bim,bam" in fdt2 matches the second string which isn't as good as fdt1.
 *
 * As an optimization, the compatible property from the FDT's root node can be
 * copied into the configuration node in the FIT image. This is required to
 * match configurations with compressed FDTs.
 *
 * Returns: offset to the configuration to use if one was found, -1 otherwise
 */
int fit_conf_find_compat(const void *fit, const void *fdt);

/**
 * fit_conf_get_node - get node offset for configuration of a given unit name
 * @fit: pointer to the FIT format image header
 * @conf_uname: configuration node unit name (NULL to use default)
 *
 * fit_conf_get_node() finds a configuration (within the '/configurations'
 * parent node) of a provided unit name. If configuration is found its node
 * offset is returned to the caller.
 *
 * When NULL is provided in second argument fit_conf_get_node() will search
 * for a default configuration node instead. Default configuration node unit
 * name is retrieved from FIT_DEFAULT_PROP property of the '/configurations'
 * node.
 *
 * returns:
 *     configuration node offset when found (>=0)
 *     negative number on failure (FDT_ERR_* code)
 */
int fit_conf_get_node(const void *fit, const char *conf_uname);

int fit_conf_get_prop_node_count(const void *fit, int noffset,
		const char *prop_name);
int fit_conf_get_prop_node_index(const void *fit, int noffset,
		const char *prop_name, int index);

/**
 * fit_conf_get_prop_node() - Get node refered to by a configuration
 * @fit:	FIT to check
 * @noffset:	Offset of conf@xxx node to check
 * @prop_name:	Property to read from the conf node
 * @phase:	Image phase to use, IH_PHASE_NONE for any
 *
 * The conf- nodes contain references to other nodes, using properties
 * like 'kernel = "kernel"'. Given such a property name (e.g. "kernel"),
 * return the offset of the node referred to (e.g. offset of node
 * "/images/kernel".
 */
int fit_conf_get_prop_node(const void *fit, int noffset, const char *prop_name,
			   enum image_phase_t phase);

int fit_check_ramdisk(const void *fit, int os_noffset,
		uint8_t arch, int verify);

int calculate_hash(const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len);

/*
 * At present we only support signing on the host, and verification on the
 * device
 */
#if defined(USE_HOSTCC)
# if CONFIG_IS_ENABLED(FIT_SIGNATURE)
#  define IMAGE_ENABLE_SIGN	1
#  define FIT_IMAGE_ENABLE_VERIFY	1
#  include <openssl/evp.h>
# else
#  define IMAGE_ENABLE_SIGN	0
#  define FIT_IMAGE_ENABLE_VERIFY	0
# endif
#else
# define IMAGE_ENABLE_SIGN	0
# define FIT_IMAGE_ENABLE_VERIFY	CONFIG_IS_ENABLED(FIT_SIGNATURE)
#endif

#ifdef USE_HOSTCC
void *image_get_host_blob(void);
void image_set_host_blob(void *host_blob);
# define gd_fdt_blob()		image_get_host_blob()
#else
# define gd_fdt_blob()		(gd->fdt_blob)
#endif

/*
 * Information passed to the signing routines
 *
 * Either 'keydir',  'keyname', or 'keyfile' can be NULL. However, either
 * 'keyfile', or both 'keydir' and 'keyname' should have valid values. If
 * neither are valid, some operations might fail with EINVAL.
 */
struct image_sign_info {
	const char *keydir;		/* Directory conaining keys */
	const char *keyname;		/* Name of key to use */
	const char *keyfile;		/* Filename of private or public key */
	const void *fit;		/* Pointer to FIT blob */
	int node_offset;		/* Offset of signature node */
	const char *name;		/* Algorithm name */
	struct checksum_algo *checksum;	/* Checksum algorithm information */
	struct padding_algo *padding;	/* Padding algorithm information */
	struct crypto_algo *crypto;	/* Crypto algorithm information */
	const void *fdt_blob;		/* FDT containing public keys */
	int required_keynode;		/* Node offset of key to use: -1=any */
	const char *require_keys;	/* Value for 'required' property */
	const char *engine_id;		/* Engine to use for signing */
	/*
	 * Note: the following two fields are always valid even w/o
	 * RSA_VERIFY_WITH_PKEY in order to make sure this structure is
	 * the same on target and host. Otherwise, vboot test may fail.
	 */
	const void *key;		/* Pointer to public key in DER */
	int keylen;			/* Length of public key */
};

/* A part of an image, used for hashing */
struct image_region {
	const void *data;
	int size;
};

struct checksum_algo {
	const char *name;
	const int checksum_len;
	const int der_len;
	const uint8_t *der_prefix;
#if IMAGE_ENABLE_SIGN
	const EVP_MD *(*calculate_sign)(void);
#endif
	int (*calculate)(const char *name,
			 const struct image_region *region,
			 int region_count, uint8_t *checksum);
};

struct crypto_algo {
	const char *name;		/* Name of algorithm */
	const int key_len;

	/**
	 * sign() - calculate and return signature for given input data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sigp:	Set to an allocated buffer holding the signature
	 * @sig_len:	Set to length of the calculated hash
	 *
	 * This computes input data signature according to selected algorithm.
	 * Resulting signature value is placed in an allocated buffer, the
	 * pointer is returned as *sigp. The length of the calculated
	 * signature is returned via the sig_len pointer argument. The caller
	 * should free *sigp.
	 *
	 * @return: 0, on success, -ve on error
	 */
	int (*sign)(struct image_sign_info *info,
		    const struct image_region region[],
		    int region_count, uint8_t **sigp, uint *sig_len);

	/**
	 * add_verify_data() - Add verification information to FDT
	 *
	 * Add public key information to the FDT node, suitable for
	 * verification at run-time. The information added depends on the
	 * algorithm being used.
	 *
	 * @info:	Specifies key and FIT information
	 * @keydest:	Destination FDT blob for public key data
	 * @return: node offset within the FDT blob where the data was written,
	 *	or -ve on error
	 */
	int (*add_verify_data)(struct image_sign_info *info, void *keydest);

	/**
	 * verify() - Verify a signature against some data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sig:	Signature
	 * @sig_len:	Number of bytes in signature
	 * @return 0 if verified, -ve on error
	 */
	int (*verify)(struct image_sign_info *info,
		      const struct image_region region[], int region_count,
		      uint8_t *sig, uint sig_len);
};

/* Declare a new U-Boot crypto algorithm handler */
#define U_BOOT_CRYPTO_ALGO(__name)						\
ll_entry_declare(struct crypto_algo, __name, cryptos)

struct padding_algo {
	const char *name;
	int (*verify)(struct image_sign_info *info,
		      const uint8_t *pad, int pad_len,
		      const uint8_t *hash, int hash_len);
};

/* Declare a new U-Boot padding algorithm handler */
#define U_BOOT_PADDING_ALGO(__name)						\
ll_entry_declare(struct padding_algo, __name, paddings)

/**
 * image_get_checksum_algo() - Look up a checksum algorithm
 *
 * @param full_name	Name of algorithm in the form "checksum,crypto"
 * Return: pointer to algorithm information, or NULL if not found
 */
struct checksum_algo *image_get_checksum_algo(const char *full_name);

/**
 * image_get_crypto_algo() - Look up a cryptosystem algorithm
 *
 * @param full_name	Name of algorithm in the form "checksum,crypto"
 * Return: pointer to algorithm information, or NULL if not found
 */
struct crypto_algo *image_get_crypto_algo(const char *full_name);

/**
 * image_get_padding_algo() - Look up a padding algorithm
 *
 * @param name		Name of padding algorithm
 * Return: pointer to algorithm information, or NULL if not found
 */
struct padding_algo *image_get_padding_algo(const char *name);

#define IMAGE_PRE_LOAD_SIG_MAGIC		0x55425348
#define IMAGE_PRE_LOAD_SIG_OFFSET_MAGIC		0
#define IMAGE_PRE_LOAD_SIG_OFFSET_IMG_LEN	4
#define IMAGE_PRE_LOAD_SIG_OFFSET_SIG		8

#define IMAGE_PRE_LOAD_PATH			"/image/pre-load/sig"
#define IMAGE_PRE_LOAD_PROP_ALGO_NAME		"algo-name"
#define IMAGE_PRE_LOAD_PROP_PADDING_NAME	"padding-name"
#define IMAGE_PRE_LOAD_PROP_SIG_SIZE		"signature-size"
#define IMAGE_PRE_LOAD_PROP_PUBLIC_KEY		"public-key"
#define IMAGE_PRE_LOAD_PROP_MANDATORY		"mandatory"

/*
 * Information in the device-tree about the signature in the header
 */
struct image_sig_info {
	char *algo_name;	/* Name of the algo (eg: sha256,rsa2048) */
	char *padding_name;	/* Name of the padding */
	uint8_t *key;		/* Public signature key */
	int key_len;		/* Length of the public key */
	uint32_t sig_size;		/* size of the signature (in the header) */
	int mandatory;		/* Set if the signature is mandatory */

	struct image_sign_info sig_info; /* Signature info */
};

/*
 * Header of the signature header
 */
struct sig_header_s {
	uint32_t magic;
	uint32_t version;
	uint32_t header_size;
	uint32_t image_size;
	uint32_t offset_img_sig;
	uint32_t flags;
	uint32_t reserved0;
	uint32_t reserved1;
	uint8_t sha256_img_sig[SHA256_SUM_LEN];
};

#define SIG_HEADER_LEN			(sizeof(struct sig_header_s))

/**
 * image_pre_load() - Manage pre load header
 *
 * Manage the pre-load header before launching the image.
 * It checks the signature of the image. It also set the
 * variable image_load_offset to skip this header before
 * launching the image.
 *
 * @param addr		Address of the image
 * @return: 0 on success, -ve on error
 */
int image_pre_load(ulong addr);

/**
 * fit_image_verify_required_sigs() - Verify signatures marked as 'required'
 *
 * @fit:		FIT to check
 * @image_noffset:	Offset of image node to check
 * @data:		Image data to check
 * @size:		Size of image data
 * @key_blob:		FDT containing public keys
 * @no_sigsp:		Returns 1 if no signatures were required, and
 *			therefore nothing was checked. The caller may wish
 *			to fall back to other mechanisms, or refuse to
 *			boot.
 * Return: 0 if all verified ok, <0 on error
 */
int fit_image_verify_required_sigs(const void *fit, int image_noffset,
		const char *data, size_t size, const void *key_blob,
		int *no_sigsp);

/**
 * fit_image_check_sig() - Check a single image signature node
 *
 * @fit:		FIT to check
 * @noffset:		Offset of signature node to check
 * @data:		Image data to check
 * @size:		Size of image data
 * @keyblob:		Key blob to check (typically the control FDT)
 * @required_keynode:	Offset in the keyblob of the required key node,
 *			if any. If this is given, then the image wil not
 *			pass verification unless that key is used. If this is
 *			-1 then any signature will do.
 * @err_msgp:		In the event of an error, this will be pointed to a
 *			help error string to display to the user.
 * Return: 0 if all verified ok, <0 on error
 */
int fit_image_check_sig(const void *fit, int noffset, const void *data,
			size_t size, const void *key_blob, int required_keynode,
			char **err_msgp);

int fit_image_decrypt_data(const void *fit,
			   int image_noffset, int cipher_noffset,
			   const void *data, size_t size,
			   void **data_unciphered, size_t *size_unciphered);

/**
 * fit_region_make_list() - Make a list of regions to hash
 *
 * Given a list of FIT regions (offset, size) provided by libfdt, create
 * a list of regions (void *, size) for use by the signature creationg
 * and verification code.
 *
 * @fit:		FIT image to process
 * @fdt_regions:	Regions as returned by libfdt
 * @count:		Number of regions returned by libfdt
 * @region:		Place to put list of regions (NULL to allocate it)
 * Return: pointer to list of regions, or NULL if out of memory
 */
struct image_region *fit_region_make_list(const void *fit,
		struct fdt_region *fdt_regions, int count,
		struct image_region *region);

static inline int fit_image_check_target_arch(const void *fdt, int node)
{
#ifndef USE_HOSTCC
	return fit_image_check_arch(fdt, node, IH_ARCH_DEFAULT);
#else
	return 0;
#endif
}

/*
 * At present we only support ciphering on the host, and unciphering on the
 * device
 */
#if defined(USE_HOSTCC)
# if defined(CONFIG_FIT_CIPHER)
#  define IMAGE_ENABLE_ENCRYPT	1
#  define IMAGE_ENABLE_DECRYPT	1
#  include <openssl/evp.h>
# else
#  define IMAGE_ENABLE_ENCRYPT	0
#  define IMAGE_ENABLE_DECRYPT	0
# endif
#else
# define IMAGE_ENABLE_ENCRYPT	0
# define IMAGE_ENABLE_DECRYPT	CONFIG_IS_ENABLED(FIT_CIPHER)
#endif

/* Information passed to the ciphering routines */
struct image_cipher_info {
	const char *keydir;		/* Directory containing keys */
	const char *keyname;		/* Name of key to use */
	const char *ivname;		/* Name of IV to use */
	const void *fit;		/* Pointer to FIT blob */
	int node_noffset;		/* Offset of the cipher node */
	const char *name;		/* Algorithm name */
	struct cipher_algo *cipher;	/* Cipher algorithm information */
	const void *fdt_blob;		/* FDT containing key and IV */
	const void *key;		/* Value of the key */
	const void *iv;			/* Value of the IV */
	size_t size_unciphered;		/* Size of the unciphered data */
};

struct cipher_algo {
	const char *name;		/* Name of algorithm */
	int key_len;			/* Length of the key */
	int iv_len;			/* Length of the IV */

#if IMAGE_ENABLE_ENCRYPT
	const EVP_CIPHER * (*calculate_type)(void);
#endif

	int (*encrypt)(struct image_cipher_info *info,
		       const unsigned char *data, int data_len,
		       unsigned char **cipher, int *cipher_len);

	int (*add_cipher_data)(struct image_cipher_info *info,
			       void *keydest, void *fit, int node_noffset);

	int (*decrypt)(struct image_cipher_info *info,
		       const void *cipher, size_t cipher_len,
		       void **data, size_t *data_len);
};

int fit_image_cipher_get_algo(const void *fit, int noffset, char **algo);
int fit_image_uncipher(const void *fit, int image_noffset,
			   void **data, size_t *size);

struct cipher_algo *image_get_cipher_algo(const char *full_name);
struct andr_image_data;

/**
 * android_image_get_data() - Parse Android boot images
 *
 * This is used to parse boot and vendor-boot header into
 * andr_image_data generic structure.
 *
 * @boot_hdr: Pointer to boot image header
 * @vendor_boot_hdr: Pointer to vendor boot image header
 * @data: Pointer to generic boot format structure
 * Return: true if succeeded, false otherwise
 */
bool android_image_get_data(const void *boot_hdr, const void *vendor_boot_hdr,
			    struct andr_image_data *data);

struct andr_boot_img_hdr_v0;

/**
 * android_image_get_kernel() - Processes kernel part of Android boot images
 *
 * This function returns the os image's start address and length. Also,
 * it appends the kernel command line to the bootargs env variable.
 *
 * @hdr:	Pointer to image header, which is at the start
 *			of the image.
 * @vendor_boot_img : Pointer to vendor boot image header
 * @verify:	Checksum verification flag. Currently unimplemented.
 * @os_data:	Pointer to a ulong variable, will hold os data start
 *			address.
 * @os_len:	Pointer to a ulong variable, will hold os data length.
 * Return: Zero, os start address and length on success,
 *		otherwise on failure.
 */
int android_image_get_kernel(const void *hdr,
			     const void *vendor_boot_img, int verify,
			     ulong *os_data, ulong *os_len);

/**
 * android_image_get_ramdisk() - Extracts the ramdisk load address and its size
 *
 * This extracts the load address of the ramdisk and its size
 *
 * @hdr:	Pointer to image header
 * @vendor_boot_img : Pointer to vendor boot image header
 * @rd_data:	Pointer to a ulong variable, will hold ramdisk address
 * @rd_len:	Pointer to a ulong variable, will hold ramdisk length
 * Return: 0 if succeeded, -1 if ramdisk size is 0
 */
int android_image_get_ramdisk(const void *hdr, const void *vendor_boot_img,
			      ulong *rd_data, ulong *rd_len);

/**
 * android_image_get_second() - Extracts the secondary bootloader address
 * and its size
 *
 * This extracts the address of the secondary bootloader and its size
 *
 * @hdr:	 Pointer to image header
 * @second_data: Pointer to a ulong variable, will hold secondary bootloader address
 * @second_len : Pointer to a ulong variable, will hold secondary bootloader length
 * Return: 0 if succeeded, -1 if secondary bootloader size is 0
 */
int android_image_get_second(const void *hdr, ulong *second_data, ulong *second_len);
bool android_image_get_dtbo(ulong hdr_addr, ulong *addr, u32 *size);

/**
 * android_image_get_dtb_by_index() - Get address and size of blob in DTB area.
 * @hdr_addr: Boot image header address
 * @vendor_boot_img: Pointer to vendor boot image header, which is at the start of the image.
 * @index: Index of desired DTB in DTB area (starting from 0)
 * @addr: If not NULL, will contain address to specified DTB
 * @size: If not NULL, will contain size of specified DTB
 *
 * Get the address and size of DTB blob by its index in DTB area of Android
 * Boot Image in RAM.
 *
 * Return: true on success or false on error.
 */
bool android_image_get_dtb_by_index(ulong hdr_addr, ulong vendor_boot_img,
				    u32 index, ulong *addr, u32 *size);

/**
 * android_image_get_end() - Get the end of Android boot image
 *
 * This returns the end address of Android boot image address
 *
 * @hdr: Pointer to image header
 * @vendor_boot_img : Pointer to vendor boot image header
 * Return: The end address of Android boot image
 */
ulong android_image_get_end(const struct andr_boot_img_hdr_v0 *hdr,
			    const void *vendor_boot_img);

/**
 * android_image_get_kload() - Get the kernel load address
 *
 * This returns the kernel load address. The load address is extracted
 * from the boot image header or the "kernel_addr_r" environment variable
 *
 * @hdr: Pointer to image header
 * @vendor_boot_img : Pointer to vendor boot image header
 * Return: The kernel load address
 */
ulong android_image_get_kload(const void *hdr,
			      const void *vendor_boot_img);

/**
 * android_image_get_kcomp() - Get kernel compression type
 *
 * This gets the kernel compression type from the boot image header
 *
 * @hdr: Pointer to image header
 * @vendor_boot_img : Pointer to vendor boot image header
 * Return: Kernel compression type
 */
ulong android_image_get_kcomp(const void *hdr,
			      const void *vendor_boot_img);

/**
 * android_print_contents() - Prints out the contents of the Android format image
 *
 * This formats a multi line Android image contents description.
 * The routine prints out Android image properties
 *
 * @hdr: Pointer to the Android format image header
 * Return: no returned results
 */
void android_print_contents(const struct andr_boot_img_hdr_v0 *hdr);
bool android_image_print_dtb_contents(ulong hdr_addr);

/**
 * is_android_boot_image_header() - Check the magic of boot image
 *
 * This checks the header of Android boot image and verifies the
 * magic is "ANDROID!"
 *
 * @hdr: Pointer to boot image
 * Return: non-zero if the magic is correct, zero otherwise
 */
bool is_android_boot_image_header(const void *hdr);

/**
 * is_android_vendor_boot_image_header() - Check the magic of vendor boot image
 *
 * This checks the header of Android vendor boot image and verifies the magic
 * is "VNDRBOOT"
 *
 * @vendor_boot_img: Pointer to boot image
 * Return: non-zero if the magic is correct, zero otherwise
 */
bool is_android_vendor_boot_image_header(const void *vendor_boot_img);

/**
 * get_abootimg_addr() - Get Android boot image address
 *
 * Return: Android boot image address
 */
ulong get_abootimg_addr(void);

/**
 * get_avendor_bootimg_addr() - Get Android vendor boot image address
 *
 * Return: Android vendor boot image address
 */
ulong get_avendor_bootimg_addr(void);

/**
 * board_fit_config_name_match() - Check for a matching board name
 *
 * This is used when SPL loads a FIT containing multiple device tree files
 * and wants to work out which one to use. The description of each one is
 * passed to this function. The description comes from the 'description' field
 * in each (FDT) image node.
 *
 * @name: Device tree description
 * Return: 0 if this device tree should be used, non-zero to try the next
 */
int board_fit_config_name_match(const char *name);

/**
 * board_fit_image_post_process() - Do any post-process on FIT binary data
 *
 * This is used to do any sort of image manipulation, verification, decryption
 * etc. in a platform or board specific way. Obviously, anything done here would
 * need to be comprehended in how the images were prepared before being injected
 * into the FIT creation (i.e. the binary blobs would have been pre-processed
 * before being added to the FIT image).
 *
 * @fit: pointer to fit image
 * @node: offset of image node
 * @image: pointer to the image start pointer
 * @size: pointer to the image size
 * Return: no return value (failure should be handled internally)
 */
void board_fit_image_post_process(const void *fit, int node, void **p_image,
				  size_t *p_size);

#define FDT_ERROR	((ulong)(-1))

ulong fdt_getprop_u32(const void *fdt, int node, const char *prop);

/**
 * fit_find_config_node() - Find the node for the best DTB in a FIT image
 *
 * A FIT image contains one or more DTBs. This function parses the
 * configurations described in the FIT images and returns the node of
 * the first matching DTB. To check if a DTB matches a board, this function
 * calls board_fit_config_name_match(). If no matching DTB is found, it returns
 * the node described by the default configuration if it exists.
 *
 * @fdt: pointer to flat device tree
 * Return: the node if found, -ve otherwise
 */
int fit_find_config_node(const void *fdt);

/**
 * Mapping of image types to function handlers to be invoked on the associated
 * loaded images
 *
 * @type: Type of image, I.E. IH_TYPE_*
 * @handler: Function to call on loaded image
 */
struct fit_loadable_tbl {
	int type;
	/**
	 * handler() - Process a loaded image
	 *
	 * @data: Pointer to start of loaded image data
	 * @size: Size of loaded image data
	 */
	void (*handler)(ulong data, size_t size);
};

/*
 * Define a FIT loadable image type handler
 *
 * _type is a valid uimage_type ID as defined in the "Image Type" enum above
 * _handler is the handler function to call after this image type is loaded
 */
#define U_BOOT_FIT_LOADABLE_HANDLER(_type, _handler) \
	ll_entry_declare(struct fit_loadable_tbl, _function, fit_loadable) = { \
		.type = _type, \
		.handler = _handler, \
	}

/**
 * fit_update - update storage with FIT image
 * @fit:        Pointer to FIT image
 *
 * Update firmware on storage using FIT image as input.
 * The storage area to be update will be identified by the name
 * in FIT and matching it to "dfu_alt_info" variable.
 *
 * Return:      0 on success, non-zero otherwise
 */
int fit_update(const void *fit);

#endif	/* __IMAGE_H__ */
