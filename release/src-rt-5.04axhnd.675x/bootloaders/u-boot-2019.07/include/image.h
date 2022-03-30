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

/* new uImage format support enabled on host */
#define IMAGE_ENABLE_FIT	1
#define IMAGE_ENABLE_OF_LIBFDT	1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */
#define CONFIG_FIT_ENABLE_RSASSA_PSS_SUPPORT 1
#define CONFIG_FIT_ENABLE_SHA256_SUPPORT
#define CONFIG_SHA1
#define CONFIG_SHA256

#define IMAGE_ENABLE_IGNORE	0
#define IMAGE_INDENT_STRING	""

#else

#include <lmb.h>
#include <asm/u-boot.h>
#include <command.h>

/* Take notice of the 'ignore' property for hashes */
#define IMAGE_ENABLE_IGNORE	1
#define IMAGE_INDENT_STRING	"   "

#define IMAGE_ENABLE_FIT	CONFIG_IS_ENABLED(FIT)
#define IMAGE_ENABLE_OF_LIBFDT	CONFIG_IS_ENABLED(OF_LIBFDT)

#endif /* USE_HOSTCC */

#if IMAGE_ENABLE_FIT
#include <hash.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
# ifdef CONFIG_SPL_BUILD
#  ifdef CONFIG_SPL_CRC32_SUPPORT
#   define IMAGE_ENABLE_CRC32	1
#  endif
#  ifdef CONFIG_SPL_MD5_SUPPORT
#   define IMAGE_ENABLE_MD5	1
#  endif
#  if CONFIG_IS_ENABLED(SHA1_SUPPORT)
#   define IMAGE_ENABLE_SHA1	1
#  endif
# else
#  define IMAGE_ENABLE_CRC32	1
#  define IMAGE_ENABLE_MD5	1
#  define IMAGE_ENABLE_SHA1	1
# endif

#ifndef IMAGE_ENABLE_CRC32
#define IMAGE_ENABLE_CRC32	0
#endif

#ifndef IMAGE_ENABLE_MD5
#define IMAGE_ENABLE_MD5	0
#endif

#ifndef IMAGE_ENABLE_SHA1
#define IMAGE_ENABLE_SHA1	0
#endif

#if defined(CONFIG_FIT_ENABLE_SHA256_SUPPORT) || \
	defined(CONFIG_SPL_SHA256_SUPPORT)
#define IMAGE_ENABLE_SHA256	1
#else
#define IMAGE_ENABLE_SHA256	0
#endif

#endif /* IMAGE_ENABLE_FIT */

#ifdef CONFIG_SYS_BOOT_GET_CMDLINE
# define IMAGE_BOOT_GET_CMDLINE		1
#else
# define IMAGE_BOOT_GET_CMDLINE		0
#endif

#ifdef CONFIG_OF_BOARD_SETUP
# define IMAGE_OF_BOARD_SETUP		1
#else
# define IMAGE_OF_BOARD_SETUP		0
#endif

#ifdef CONFIG_OF_SYSTEM_SETUP
# define IMAGE_OF_SYSTEM_SETUP	1
#else
# define IMAGE_OF_SYSTEM_SETUP	0
#endif

enum ih_category {
	IH_ARCH,
	IH_COMP,
	IH_OS,
	IH_TYPE,

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

enum {
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

	IH_COMP_COUNT,
};

#define LZ4F_MAGIC	0x184D2204	/* LZ4 Magic Number		*/
#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/* Reused from common.h */
#define ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

typedef struct image_info {
	ulong		start, end;		/* start/end of blob */
	ulong		image_start, image_len; /* start of image within blob, len of image */
	ulong		load;			/* load addr for the image */
	uint8_t		comp, type, os;		/* compression, type of image, os type */
	uint8_t		arch;			/* CPU architecture */
} image_info_t;

/*
 * Legacy and FIT format headers used by do_bootm() and do_bootm_<os>()
 * routines.
 */
typedef struct bootm_headers {
	/*
	 * Legacy os image header, if it is a multi component image
	 * then boot_get_ramdisk() and get_fdt() will attempt to get
	 * data from second and third component accordingly.
	 */
	image_header_t	*legacy_hdr_os;		/* image header pointer */
	image_header_t	legacy_hdr_os_copy;	/* header copy */
	ulong		legacy_hdr_valid;

#if IMAGE_ENABLE_FIT
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
#endif

#ifndef USE_HOSTCC
	image_info_t	os;		/* os image info */
	ulong		ep;		/* entry point of OS */

	ulong		rd_start, rd_end;/* ramdisk start/end */

	char		*ft_addr;	/* flat dev tree address */
	ulong		ft_len;		/* length of flat device tree */

	ulong		initrd_start;
	ulong		initrd_end;
	ulong		cmdline_start;
	ulong		cmdline_end;
	bd_t		*kbd;
#endif

	int		verify;		/* env_get("verify")[0] != 'n' */

#define	BOOTM_STATE_START	(0x00000001)
#define	BOOTM_STATE_FINDOS	(0x00000002)
#define	BOOTM_STATE_FINDOTHER	(0x00000004)
#define	BOOTM_STATE_LOADOS	(0x00000008)
#define	BOOTM_STATE_RAMDISK	(0x00000010)
#define	BOOTM_STATE_FDT		(0x00000020)
#define	BOOTM_STATE_OS_CMDLINE	(0x00000040)
#define	BOOTM_STATE_OS_BD_T	(0x00000080)
#define	BOOTM_STATE_OS_PREP	(0x00000100)
#define	BOOTM_STATE_OS_FAKE_GO	(0x00000200)	/* 'Almost' run the OS */
#define	BOOTM_STATE_OS_GO	(0x00000400)
	int		state;

#ifdef CONFIG_LMB
	struct lmb	lmb;		/* for memory mgmt */
#endif
} bootm_headers_t;

extern bootm_headers_t images;

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
 * @return OS short name, or "unknown" if unknown
 */
const char *genimg_get_os_short_name(uint8_t comp);

const char *genimg_get_arch_name(uint8_t arch);

/**
 * genimg_get_arch_short_name() - get the short name for an architecture
 *
 * @param arch	Architecture type (IH_ARCH_...)
 * @return architecture short name, or "unknown" if unknown
 */
const char *genimg_get_arch_short_name(uint8_t arch);

const char *genimg_get_type_name(uint8_t type);

/**
 * genimg_get_type_short_name() - get the short name for an image type
 *
 * @param type	Image type (IH_TYPE_...)
 * @return image short name, or "unknown" if unknown
 */
const char *genimg_get_type_short_name(uint8_t type);

const char *genimg_get_comp_name(uint8_t comp);

/**
 * genimg_get_comp_short_name() - get the short name for a compression method
 *
 * @param comp	compression method (IH_COMP_...)
 * @return compression method short name, or "unknown" if unknown
 */
const char *genimg_get_comp_short_name(uint8_t comp);

/**
 * genimg_get_cat_name() - Get the name of an item in a category
 *
 * @category:	Category of item
 * @id:		Item ID
 * @return name of item, or "Unknown ..." if unknown
 */
const char *genimg_get_cat_name(enum ih_category category, uint id);

/**
 * genimg_get_cat_short_name() - Get the short name of an item in a category
 *
 * @category:	Category of item
 * @id:		Item ID
 * @return short name of item, or "Unknown ..." if unknown
 */
const char *genimg_get_cat_short_name(enum ih_category category, uint id);

/**
 * genimg_get_cat_count() - Get the number of items in a category
 *
 * @category:	Category to check
 * @return the number of items in the category (IH_xxx_COUNT)
 */
int genimg_get_cat_count(enum ih_category category);

/**
 * genimg_get_cat_desc() - Get the description of a category
 *
 * @return the description of a category, e.g. "architecture". This
 * effectively converts the enum to a string.
 */
const char *genimg_get_cat_desc(enum ih_category category);

int genimg_get_os_id(const char *name);
int genimg_get_arch_id(const char *name);
int genimg_get_type_id(const char *name);
int genimg_get_comp_id(const char *name);
void genimg_print_size(uint32_t size);

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || \
	defined(USE_HOSTCC)
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

int boot_get_setup(bootm_headers_t *images, uint8_t arch, ulong *setup_start,
		   ulong *setup_len);

#ifndef USE_HOSTCC
/* Image format types, returned by _get_format() routine */
#define IMAGE_FORMAT_INVALID	0x00
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
#define IMAGE_FORMAT_LEGACY	0x01	/* legacy image_header based format */
#endif
#define IMAGE_FORMAT_FIT	0x02	/* new, libfdt based format */
#define IMAGE_FORMAT_ANDROID	0x03	/* Android boot image */

ulong genimg_get_kernel_addr_fit(char * const img_addr,
			         const char **fit_uname_config,
			         const char **fit_uname_kernel);
ulong genimg_get_kernel_addr(char * const img_addr);
int genimg_get_format(const void *img_addr);
int genimg_has_config(bootm_headers_t *images);

int boot_get_fpga(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, const ulong *ld_start, ulong * const ld_len);
int boot_get_ramdisk(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, ulong *rd_start, ulong *rd_end);

/**
 * boot_get_loadable - routine to load a list of binaries to memory
 * @argc: Ignored Argument
 * @argv: Ignored Argument
 * @images: pointer to the bootm images structure
 * @arch: expected architecture for the image
 * @ld_start: Ignored Argument
 * @ld_len: Ignored Argument
 *
 * boot_get_loadable() will take the given FIT configuration, and look
 * for a field named "loadables".  Loadables, is a list of elements in
 * the FIT given as strings.  exe:
 *   loadables = "linux_kernel", "fdt-2";
 * this function will attempt to parse each string, and load the
 * corresponding element from the FIT into memory.  Once placed,
 * no aditional actions are taken.
 *
 * @return:
 *     0, if only valid images or no images are found
 *     error code, if an error occurs during fit_image_load
 */
int boot_get_loadable(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, const ulong *ld_start, ulong * const ld_len);
#endif /* !USE_HOSTCC */

int boot_get_setup_fit(bootm_headers_t *images, uint8_t arch,
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
 * @return node offset of base image, or -ve error code on error
 */
int boot_get_fdt_fit(bootm_headers_t *images, ulong addr,
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
 * @param image_type	Required image type (IH_TYPE_...). If this is
 *			IH_TYPE_KERNEL then we allow IH_TYPE_KERNEL_NOLOAD
 *			also.
 * @param bootstage_id	ID of starting bootstage to use for progress updates.
 *			This will be added to the BOOTSTAGE_SUB values when
 *			calling bootstage_mark()
 * @param load_op	Decribes what to do with the load address
 * @param datap		Returns address of loaded image
 * @param lenp		Returns length of loaded image
 * @return node offset of image, or -ve error code on error
 */
int fit_image_load(bootm_headers_t *images, ulong addr,
		   const char **fit_unamep, const char **fit_uname_configp,
		   int arch, int image_type, int bootstage_id,
		   enum fit_load_op load_op, ulong *datap, ulong *lenp);

#ifndef USE_HOSTCC
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
int fit_get_node_from_config(bootm_headers_t *images, const char *prop_name,
			ulong addr);

int boot_get_fdt(int flag, int argc, char * const argv[], uint8_t arch,
		 bootm_headers_t *images,
		 char **of_flat_tree, ulong *of_size);
void boot_fdt_add_mem_rsv_regions(struct lmb *lmb, void *fdt_blob);
int boot_relocate_fdt(struct lmb *lmb, char **of_flat_tree, ulong *of_size);

int boot_ramdisk_high(struct lmb *lmb, ulong rd_data, ulong rd_len,
		  ulong *initrd_start, ulong *initrd_end);
int boot_get_cmdline(struct lmb *lmb, ulong *cmd_start, ulong *cmd_end);
#ifdef CONFIG_SYS_BOOT_GET_KBD
int boot_get_kbd(struct lmb *lmb, bd_t **kbd);
#endif /* CONFIG_SYS_BOOT_GET_KBD */
#endif /* !USE_HOSTCC */

/*******************************************************************/
/* Legacy format specific code (prefixed with image_) */
/*******************************************************************/
static inline uint32_t image_get_header_size(void)
{
	return (sizeof(image_header_t));
}

#define image_get_hdr_l(f) \
	static inline uint32_t image_get_##f(const image_header_t *hdr) \
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
	static inline uint8_t image_get_##f(const image_header_t *hdr) \
	{ \
		return hdr->ih_##f; \
	}
image_get_hdr_b(os)		/* image_get_os */
image_get_hdr_b(arch)		/* image_get_arch */
image_get_hdr_b(type)		/* image_get_type */
image_get_hdr_b(comp)		/* image_get_comp */

static inline char *image_get_name(const image_header_t *hdr)
{
	return (char *)hdr->ih_name;
}

static inline uint32_t image_get_data_size(const image_header_t *hdr)
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
static inline ulong image_get_data(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_header_size());
}

static inline uint32_t image_get_image_size(const image_header_t *hdr)
{
	return (image_get_size(hdr) + image_get_header_size());
}
static inline ulong image_get_image_end(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_image_size(hdr));
}

#define image_set_hdr_l(f) \
	static inline void image_set_##f(image_header_t *hdr, uint32_t val) \
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
	static inline void image_set_##f(image_header_t *hdr, uint8_t val) \
	{ \
		hdr->ih_##f = val; \
	}
image_set_hdr_b(os)		/* image_set_os */
image_set_hdr_b(arch)		/* image_set_arch */
image_set_hdr_b(type)		/* image_set_type */
image_set_hdr_b(comp)		/* image_set_comp */

static inline void image_set_name(image_header_t *hdr, const char *name)
{
	strncpy(image_get_name(hdr), name, IH_NMLEN);
}

int image_check_hcrc(const image_header_t *hdr);
int image_check_dcrc(const image_header_t *hdr);
#ifndef USE_HOSTCC
ulong env_get_bootm_low(void);
phys_size_t env_get_bootm_size(void);
phys_size_t env_get_bootm_mapsize(void);
#endif
void memmove_wd(void *to, void *from, size_t len, ulong chunksz);

static inline int image_check_magic(const image_header_t *hdr)
{
	return (image_get_magic(hdr) == IH_MAGIC);
}
static inline int image_check_type(const image_header_t *hdr, uint8_t type)
{
	return (image_get_type(hdr) == type);
}
static inline int image_check_arch(const image_header_t *hdr, uint8_t arch)
{
	return (image_get_arch(hdr) == arch) ||
		(image_get_arch(hdr) == IH_ARCH_ARM && arch == IH_ARCH_ARM64);
}
static inline int image_check_os(const image_header_t *hdr, uint8_t os)
{
	return (image_get_os(hdr) == os);
}

ulong image_multi_count(const image_header_t *hdr);
void image_multi_getimg(const image_header_t *hdr, ulong idx,
			ulong *data, ulong *len);

void image_print_contents(const void *hdr);

#ifndef USE_HOSTCC
static inline int image_check_target_arch(const image_header_t *hdr)
{
#ifndef IH_ARCH_DEFAULT
# error "please define IH_ARCH_DEFAULT in your arch asm/u-boot.h"
#endif
	return image_check_arch(hdr, IH_ARCH_DEFAULT);
}
#endif /* USE_HOSTCC */

/**
 * Set up properties in the FDT
 *
 * This sets up properties in the FDT that is to be passed to linux.
 *
 * @images:	Images information
 * @blob:	FDT to update
 * @of_size:	Size of the FDT
 * @lmb:	Points to logical memory block structure
 * @return 0 if ok, <0 on failure
 */
int image_setup_libfdt(bootm_headers_t *images, void *blob,
		       int of_size, struct lmb *lmb);

/**
 * Set up the FDT to use for booting a kernel
 *
 * This performs ramdisk setup, sets up the FDT if required, and adds
 * paramters to the FDT if libfdt is available.
 *
 * @param images	Images information
 * @return 0 if ok, <0 on failure
 */
int image_setup_linux(bootm_headers_t *images);

/**
 * bootz_setup() - Extract stat and size of a Linux xImage
 *
 * @image: Address of image
 * @start: Returns start address of image
 * @end : Returns end address of image
 * @return 0 if OK, 1 if the image was not recognised
 */
int bootz_setup(ulong image, ulong *start, ulong *end);

/**
 * Return the correct start address and size of a Linux aarch64 Image.
 *
 * @image: Address of image
 * @start: Returns start address of image
 * @size : Returns size image
 * @force_reloc: Ignore image->ep field, always place image to RAM start
 * @return 0 if OK, 1 if the image was not recognised
 */
int booti_setup(ulong image, ulong *relocated_addr, ulong *size,
		bool force_reloc);

/*******************************************************************/
/* New uImage format specific code (prefixed with fit_) */
/*******************************************************************/

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"

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

#define FIT_MAX_HASH_LEN	HASH_MAX_DIGEST_SIZE

#if IMAGE_ENABLE_FIT
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
int fit_image_get_data_and_size(const void *fit, int noffset,
				const void **data, size_t *size);

int fit_image_hash_get_algo(const void *fit, int noffset, char **algo);
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len);

int fit_set_timestamp(void *fit, int noffset, time_t timestamp);

/**
 * fit_add_verification_data() - add verification data to FIT image nodes
 *
 * @keydir:	Directory containing keys
 * @kwydest:	FDT blob to write public key information to
 * @fit:	Pointer to the FIT format image header
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @engine_id:	Engine to use for signing
 * @cmdname:	Command name used when reporting errors
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
int fit_add_verification_data(const char *keydir, void *keydest, void *fit,
			      const char *comment, int require_keys,
			      const char *engine_id, const char *cmdname);

int fit_image_verify_with_data(const void *fit, int image_noffset,
			       const void *data, size_t size);
int fit_image_verify(const void *fit, int noffset);
int fit_config_verify(const void *fit, int conf_noffset);
int fit_all_image_verify(const void *fit);
int fit_image_check_os(const void *fit, int noffset, uint8_t os);
int fit_image_check_arch(const void *fit, int noffset, uint8_t arch);
int fit_image_check_type(const void *fit, int noffset, uint8_t type);
int fit_image_check_comp(const void *fit, int noffset, uint8_t comp);
int fit_check_format(const void *fit);

int fit_conf_find_compat(const void *fit, const void *fdt);
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
 *
 * The conf- nodes contain references to other nodes, using properties
 * like 'kernel = "kernel"'. Given such a property name (e.g. "kernel"),
 * return the offset of the node referred to (e.g. offset of node
 * "/images/kernel".
 */
int fit_conf_get_prop_node(const void *fit, int noffset,
		const char *prop_name);

int fit_check_ramdisk(const void *fit, int os_noffset,
		uint8_t arch, int verify);

int calculate_hash(const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len);

/*
 * At present we only support signing on the host, and verification on the
 * device
 */
#if defined(USE_HOSTCC)
# if defined(CONFIG_FIT_SIGNATURE)
#  define IMAGE_ENABLE_SIGN	1
#  define IMAGE_ENABLE_VERIFY	1
#  include <openssl/evp.h>
# else
#  define IMAGE_ENABLE_SIGN	0
#  define IMAGE_ENABLE_VERIFY	0
# endif
#else
# define IMAGE_ENABLE_SIGN	0
# define IMAGE_ENABLE_VERIFY	CONFIG_IS_ENABLED(FIT_SIGNATURE)
#endif

#ifdef USE_HOSTCC
void *image_get_host_blob(void);
void image_set_host_blob(void *host_blob);
# define gd_fdt_blob()		image_get_host_blob()
#else
# define gd_fdt_blob()		(gd->fdt_blob)
#endif

#ifdef CONFIG_FIT_BEST_MATCH
#define IMAGE_ENABLE_BEST_MATCH	1
#else
#define IMAGE_ENABLE_BEST_MATCH	0
#endif

/* Information passed to the signing routines */
struct image_sign_info {
	const char *keydir;		/* Directory conaining keys */
	const char *keyname;		/* Name of key to use */
	void *fit;			/* Pointer to FIT blob */
	int node_offset;		/* Offset of signature node */
	const char *name;		/* Algorithm name */
	struct checksum_algo *checksum;	/* Checksum algorithm information */
	struct padding_algo *padding;	/* Padding algorithm information */
	struct crypto_algo *crypto;	/* Crypto algorithm information */
	const void *fdt_blob;		/* FDT containing public keys */
	int required_keynode;		/* Node offset of key to use: -1=any */
	const char *require_keys;	/* Value for 'required' property */
	const char *engine_id;		/* Engine to use for signing */
};
#endif /* Allow struct image_region to always be defined for rsa.h */

/* A part of an image, used for hashing */
struct image_region {
	const void *data;
	int size;
};

#if IMAGE_ENABLE_FIT

#if IMAGE_ENABLE_VERIFY
# include <u-boot/rsa-checksum.h>
#endif
struct checksum_algo {
	const char *name;
	const int checksum_len;
	const int der_len;
	const uint8_t *der_prefix;
#if IMAGE_ENABLE_SIGN
	const EVP_MD *(*calculate_sign)(void);
#endif
	int (*calculate)(const char *name,
			 const struct image_region region[],
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
	 * @return: 0, on success, -ve on error
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

struct padding_algo {
	const char *name;
	int (*verify)(struct image_sign_info *info,
		      uint8_t *pad, int pad_len,
		      const uint8_t *hash, int hash_len);
};

/**
 * image_get_checksum_algo() - Look up a checksum algorithm
 *
 * @param full_name	Name of algorithm in the form "checksum,crypto"
 * @return pointer to algorithm information, or NULL if not found
 */
struct checksum_algo *image_get_checksum_algo(const char *full_name);

/**
 * image_get_crypto_algo() - Look up a cryptosystem algorithm
 *
 * @param full_name	Name of algorithm in the form "checksum,crypto"
 * @return pointer to algorithm information, or NULL if not found
 */
struct crypto_algo *image_get_crypto_algo(const char *full_name);

/**
 * image_get_padding_algo() - Look up a padding algorithm
 *
 * @param name		Name of padding algorithm
 * @return pointer to algorithm information, or NULL if not found
 */
struct padding_algo *image_get_padding_algo(const char *name);

/**
 * fit_image_verify_required_sigs() - Verify signatures marked as 'required'
 *
 * @fit:		FIT to check
 * @image_noffset:	Offset of image node to check
 * @data:		Image data to check
 * @size:		Size of image data
 * @sig_blob:		FDT containing public keys
 * @no_sigsp:		Returns 1 if no signatures were required, and
 *			therefore nothing was checked. The caller may wish
 *			to fall back to other mechanisms, or refuse to
 *			boot.
 * @return 0 if all verified ok, <0 on error
 */
int fit_image_verify_required_sigs(const void *fit, int image_noffset,
		const char *data, size_t size, const void *sig_blob,
		int *no_sigsp);

/**
 * fit_image_check_sig() - Check a single image signature node
 *
 * @fit:		FIT to check
 * @noffset:		Offset of signature node to check
 * @data:		Image data to check
 * @size:		Size of image data
 * @required_keynode:	Offset in the control FDT of the required key node,
 *			if any. If this is given, then the image wil not
 *			pass verification unless that key is used. If this is
 *			-1 then any signature will do.
 * @err_msgp:		In the event of an error, this will be pointed to a
 *			help error string to display to the user.
 * @return 0 if all verified ok, <0 on error
 */
int fit_image_check_sig(const void *fit, int noffset, const void *data,
		size_t size, int required_keynode, char **err_msgp);

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
 * @return pointer to list of regions, or NULL if out of memory
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

#ifdef CONFIG_FIT_VERBOSE
#define fit_unsupported(msg)	printf("! %s:%d " \
				"FIT images not supported for '%s'\n", \
				__FILE__, __LINE__, (msg))

#define fit_unsupported_reset(msg)	printf("! %s:%d " \
				"FIT images not supported for '%s' " \
				"- must reset board to recover!\n", \
				__FILE__, __LINE__, (msg))
#else
#define fit_unsupported(msg)
#define fit_unsupported_reset(msg)
#endif /* CONFIG_FIT_VERBOSE */
#endif /* CONFIG_FIT */

#if defined(CONFIG_ANDROID_BOOT_IMAGE)
struct andr_img_hdr;
int android_image_check_header(const struct andr_img_hdr *hdr);
int android_image_get_kernel(const struct andr_img_hdr *hdr, int verify,
			     ulong *os_data, ulong *os_len);
int android_image_get_ramdisk(const struct andr_img_hdr *hdr,
			      ulong *rd_data, ulong *rd_len);
int android_image_get_second(const struct andr_img_hdr *hdr,
			      ulong *second_data, ulong *second_len);
ulong android_image_get_end(const struct andr_img_hdr *hdr);
ulong android_image_get_kload(const struct andr_img_hdr *hdr);
ulong android_image_get_kcomp(const struct andr_img_hdr *hdr);
void android_print_contents(const struct andr_img_hdr *hdr);

#endif /* CONFIG_ANDROID_BOOT_IMAGE */

/**
 * board_fit_config_name_match() - Check for a matching board name
 *
 * This is used when SPL loads a FIT containing multiple device tree files
 * and wants to work out which one to use. The description of each one is
 * passed to this function. The description comes from the 'description' field
 * in each (FDT) image node.
 *
 * @name: Device tree description
 * @return 0 if this device tree should be used, non-zero to try the next
 */
int board_fit_config_name_match(const char *name);

#if defined(CONFIG_SPL_FIT_IMAGE_POST_PROCESS) || \
	defined(CONFIG_FIT_IMAGE_POST_PROCESS)
/**
 * board_fit_image_post_process() - Do any post-process on FIT binary data
 *
 * This is used to do any sort of image manipulation, verification, decryption
 * etc. in a platform or board specific way. Obviously, anything done here would
 * need to be comprehended in how the images were prepared before being injected
 * into the FIT creation (i.e. the binary blobs would have been pre-processed
 * before being added to the FIT image).
 *
 * @image: pointer to the image start pointer
 * @size: pointer to the image size
 * @return no return value (failure should be handled internally)
 */
void board_fit_image_post_process(void **p_image, size_t *p_size);
#endif /* CONFIG_SPL_FIT_IMAGE_POST_PROCESS */

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
 * @return the node if found, -ve otherwise
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

#endif	/* __IMAGE_H__ */
