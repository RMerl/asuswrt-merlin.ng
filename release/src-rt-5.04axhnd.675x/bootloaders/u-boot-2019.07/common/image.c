// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <watchdog.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
#include <status_led.h>
#endif

#include <rtc.h>

#include <environment.h>
#include <image.h>
#include <mapmem.h>

#if IMAGE_ENABLE_FIT || IMAGE_ENABLE_OF_LIBFDT
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fpga.h>
#include <xilinx.h>
#endif

#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <linux/errno.h>
#include <asm/io.h>

#ifdef CONFIG_CMD_BDI
extern int do_bdinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
static const image_header_t *image_get_ramdisk(ulong rd_addr, uint8_t arch,
						int verify);
#endif
#else
#include "mkimage.h"
#include <u-boot/md5.h>
#include <time.h>
#include <image.h>

#ifndef __maybe_unused
# define __maybe_unused		/* unimplemented */
#endif
#endif /* !USE_HOSTCC*/

#include <u-boot/crc.h>

#ifndef CONFIG_SYS_BARGSIZE
#define CONFIG_SYS_BARGSIZE 512
#endif

static const table_entry_t uimage_arch[] = {
	{	IH_ARCH_INVALID,	"invalid",	"Invalid ARCH",	},
	{	IH_ARCH_ALPHA,		"alpha",	"Alpha",	},
	{	IH_ARCH_ARM,		"arm",		"ARM",		},
	{	IH_ARCH_I386,		"x86",		"Intel x86",	},
	{	IH_ARCH_IA64,		"ia64",		"IA64",		},
	{	IH_ARCH_M68K,		"m68k",		"M68K",		},
	{	IH_ARCH_MICROBLAZE,	"microblaze",	"MicroBlaze",	},
	{	IH_ARCH_MIPS,		"mips",		"MIPS",		},
	{	IH_ARCH_MIPS64,		"mips64",	"MIPS 64 Bit",	},
	{	IH_ARCH_NIOS2,		"nios2",	"NIOS II",	},
	{	IH_ARCH_PPC,		"powerpc",	"PowerPC",	},
	{	IH_ARCH_PPC,		"ppc",		"PowerPC",	},
	{	IH_ARCH_S390,		"s390",		"IBM S390",	},
	{	IH_ARCH_SH,		"sh",		"SuperH",	},
	{	IH_ARCH_SPARC,		"sparc",	"SPARC",	},
	{	IH_ARCH_SPARC64,	"sparc64",	"SPARC 64 Bit",	},
	{	IH_ARCH_BLACKFIN,	"blackfin",	"Blackfin",	},
	{	IH_ARCH_AVR32,		"avr32",	"AVR32",	},
	{	IH_ARCH_NDS32,		"nds32",	"NDS32",	},
	{	IH_ARCH_OPENRISC,	"or1k",		"OpenRISC 1000",},
	{	IH_ARCH_SANDBOX,	"sandbox",	"Sandbox",	},
	{	IH_ARCH_ARM64,		"arm64",	"AArch64",	},
	{	IH_ARCH_ARC,		"arc",		"ARC",		},
	{	IH_ARCH_X86_64,		"x86_64",	"AMD x86_64",	},
	{	IH_ARCH_XTENSA,		"xtensa",	"Xtensa",	},
	{	IH_ARCH_RISCV,		"riscv",	"RISC-V",	},
	{	-1,			"",		"",		},
};

static const table_entry_t uimage_os[] = {
	{	IH_OS_INVALID,	"invalid",	"Invalid OS",		},
	{       IH_OS_ARM_TRUSTED_FIRMWARE, "arm-trusted-firmware", "ARM Trusted Firmware"  },
	{	IH_OS_LINUX,	"linux",	"Linux",		},
#if defined(CONFIG_LYNXKDI) || defined(USE_HOSTCC)
	{	IH_OS_LYNXOS,	"lynxos",	"LynxOS",		},
#endif
	{	IH_OS_NETBSD,	"netbsd",	"NetBSD",		},
	{	IH_OS_OSE,	"ose",		"Enea OSE",		},
	{	IH_OS_PLAN9,	"plan9",	"Plan 9",		},
	{	IH_OS_RTEMS,	"rtems",	"RTEMS",		},
	{	IH_OS_TEE,	"tee",		"Trusted Execution Environment" },
	{	IH_OS_U_BOOT,	"u-boot",	"U-Boot",		},
	{	IH_OS_VXWORKS,	"vxworks",	"VxWorks",		},
#if defined(CONFIG_CMD_ELF) || defined(USE_HOSTCC)
	{	IH_OS_QNX,	"qnx",		"QNX",			},
#endif
#if defined(CONFIG_INTEGRITY) || defined(USE_HOSTCC)
	{	IH_OS_INTEGRITY,"integrity",	"INTEGRITY",		},
#endif
#ifdef USE_HOSTCC
	{	IH_OS_4_4BSD,	"4_4bsd",	"4_4BSD",		},
	{	IH_OS_DELL,	"dell",		"Dell",			},
	{	IH_OS_ESIX,	"esix",		"Esix",			},
	{	IH_OS_FREEBSD,	"freebsd",	"FreeBSD",		},
	{	IH_OS_IRIX,	"irix",		"Irix",			},
	{	IH_OS_NCR,	"ncr",		"NCR",			},
	{	IH_OS_OPENBSD,	"openbsd",	"OpenBSD",		},
	{	IH_OS_PSOS,	"psos",		"pSOS",			},
	{	IH_OS_SCO,	"sco",		"SCO",			},
	{	IH_OS_SOLARIS,	"solaris",	"Solaris",		},
	{	IH_OS_SVR4,	"svr4",		"SVR4",			},
#endif
#if defined(CONFIG_BOOTM_OPENRTOS) || defined(USE_HOSTCC)
	{	IH_OS_OPENRTOS,	"openrtos",	"OpenRTOS",		},
#endif

	{	-1,		"",		"",			},
};

static const table_entry_t uimage_type[] = {
	{	IH_TYPE_AISIMAGE,   "aisimage",   "Davinci AIS image",},
	{	IH_TYPE_FILESYSTEM, "filesystem", "Filesystem Image",	},
	{	IH_TYPE_FIRMWARE,   "firmware",	  "Firmware",		},
	{	IH_TYPE_FLATDT,     "flat_dt",    "Flat Device Tree",	},
	{	IH_TYPE_GPIMAGE,    "gpimage",    "TI Keystone SPL Image",},
	{	IH_TYPE_KERNEL,	    "kernel",	  "Kernel Image",	},
	{	IH_TYPE_KERNEL_NOLOAD, "kernel_noload",  "Kernel Image (no loading done)", },
	{	IH_TYPE_KWBIMAGE,   "kwbimage",   "Kirkwood Boot Image",},
	{	IH_TYPE_IMXIMAGE,   "imximage",   "Freescale i.MX Boot Image",},
	{	IH_TYPE_IMX8IMAGE,  "imx8image",  "NXP i.MX8 Boot Image",},
	{	IH_TYPE_IMX8MIMAGE, "imx8mimage", "NXP i.MX8M Boot Image",},
	{	IH_TYPE_INVALID,    "invalid",	  "Invalid Image",	},
	{	IH_TYPE_MULTI,	    "multi",	  "Multi-File Image",	},
	{	IH_TYPE_OMAPIMAGE,  "omapimage",  "TI OMAP SPL With GP CH",},
	{	IH_TYPE_PBLIMAGE,   "pblimage",   "Freescale PBL Boot Image",},
	{	IH_TYPE_RAMDISK,    "ramdisk",	  "RAMDisk Image",	},
	{	IH_TYPE_SCRIPT,     "script",	  "Script",		},
	{	IH_TYPE_SOCFPGAIMAGE, "socfpgaimage", "Altera SoCFPGA CV/AV preloader",},
	{	IH_TYPE_SOCFPGAIMAGE_V1, "socfpgaimage_v1", "Altera SoCFPGA A10 preloader",},
	{	IH_TYPE_STANDALONE, "standalone", "Standalone Program", },
	{	IH_TYPE_UBLIMAGE,   "ublimage",   "Davinci UBL image",},
	{	IH_TYPE_MXSIMAGE,   "mxsimage",   "Freescale MXS Boot Image",},
	{	IH_TYPE_ATMELIMAGE, "atmelimage", "ATMEL ROM-Boot Image",},
	{	IH_TYPE_X86_SETUP,  "x86_setup",  "x86 setup.bin",    },
	{	IH_TYPE_LPC32XXIMAGE, "lpc32xximage",  "LPC32XX Boot Image", },
	{	IH_TYPE_RKIMAGE,    "rkimage",    "Rockchip Boot Image" },
	{	IH_TYPE_RKSD,       "rksd",       "Rockchip SD Boot Image" },
	{	IH_TYPE_RKSPI,      "rkspi",      "Rockchip SPI Boot Image" },
	{	IH_TYPE_VYBRIDIMAGE, "vybridimage",  "Vybrid Boot Image", },
	{	IH_TYPE_ZYNQIMAGE,  "zynqimage",  "Xilinx Zynq Boot Image" },
	{	IH_TYPE_ZYNQMPIMAGE, "zynqmpimage", "Xilinx ZynqMP Boot Image" },
	{	IH_TYPE_ZYNQMPBIF,  "zynqmpbif",  "Xilinx ZynqMP Boot Image (bif)" },
	{	IH_TYPE_FPGA,       "fpga",       "FPGA Image" },
	{       IH_TYPE_TEE,        "tee",        "Trusted Execution Environment Image",},
	{	IH_TYPE_FIRMWARE_IVT, "firmware_ivt", "Firmware with HABv4 IVT" },
	{       IH_TYPE_PMMC,        "pmmc",        "TI Power Management Micro-Controller Firmware",},
	{	IH_TYPE_STM32IMAGE, "stm32image", "STMicroelectronics STM32 Image" },
	{	IH_TYPE_MTKIMAGE,   "mtk_image",   "MediaTek BootROM loadable Image" },
	{	-1,		    "",		  "",			},
};

static const table_entry_t uimage_comp[] = {
	{	IH_COMP_NONE,	"none",		"uncompressed",		},
	{	IH_COMP_BZIP2,	"bzip2",	"bzip2 compressed",	},
	{	IH_COMP_GZIP,	"gzip",		"gzip compressed",	},
	{	IH_COMP_LZMA,	"lzma",		"lzma compressed",	},
	{	IH_COMP_LZO,	"lzo",		"lzo compressed",	},
	{	IH_COMP_LZ4,	"lz4",		"lz4 compressed",	},
	{	-1,		"",		"",			},
};

struct table_info {
	const char *desc;
	int count;
	const table_entry_t *table;
};

static const struct table_info table_info[IH_COUNT] = {
	{ "architecture", IH_ARCH_COUNT, uimage_arch },
	{ "compression", IH_COMP_COUNT, uimage_comp },
	{ "operating system", IH_OS_COUNT, uimage_os },
	{ "image type", IH_TYPE_COUNT, uimage_type },
};

/*****************************************************************************/
/* Legacy format routines */
/*****************************************************************************/
int image_check_hcrc(const image_header_t *hdr)
{
	ulong hcrc;
	ulong len = image_get_header_size();
	image_header_t header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, (char *)hdr, image_get_header_size());
	image_set_hcrc(&header, 0);

	hcrc = crc32(0, (unsigned char *)&header, len);

	return (hcrc == image_get_hcrc(hdr));
}

int image_check_dcrc(const image_header_t *hdr)
{
	ulong data = image_get_data(hdr);
	ulong len = image_get_data_size(hdr);
	ulong dcrc = crc32_wd(0, (unsigned char *)data, len, CHUNKSZ_CRC32);

	return (dcrc == image_get_dcrc(hdr));
}

/**
 * image_multi_count - get component (sub-image) count
 * @hdr: pointer to the header of the multi component image
 *
 * image_multi_count() returns number of components in a multi
 * component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     number of components
 */
ulong image_multi_count(const image_header_t *hdr)
{
	ulong i, count = 0;
	uint32_t *size;

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (uint32_t *)image_get_data(hdr);

	/* count non empty slots */
	for (i = 0; size[i]; ++i)
		count++;

	return count;
}

/**
 * image_multi_getimg - get component data address and size
 * @hdr: pointer to the header of the multi component image
 * @idx: index of the requested component
 * @data: pointer to a ulong variable, will hold component data address
 * @len: pointer to a ulong variable, will hold component size
 *
 * image_multi_getimg() returns size and data address for the requested
 * component in a multi component image.
 *
 * Note: no checking of the image type is done, caller must pass
 * a valid multi component image.
 *
 * returns:
 *     data address and size of the component, if idx is valid
 *     0 in data and len, if idx is out of range
 */
void image_multi_getimg(const image_header_t *hdr, ulong idx,
			ulong *data, ulong *len)
{
	int i;
	uint32_t *size;
	ulong offset, count, img_data;

	/* get number of component */
	count = image_multi_count(hdr);

	/* get start of the image payload, which in case of multi
	 * component images that points to a table of component sizes */
	size = (uint32_t *)image_get_data(hdr);

	/* get address of the proper component data start, which means
	 * skipping sizes table (add 1 for last, null entry) */
	img_data = image_get_data(hdr) + (count + 1) * sizeof(uint32_t);

	if (idx < count) {
		*len = uimage_to_cpu(size[idx]);
		offset = 0;

		/* go over all indices preceding requested component idx */
		for (i = 0; i < idx; i++) {
			/* add up i-th component size, rounding up to 4 bytes */
			offset += (uimage_to_cpu(size[i]) + 3) & ~3 ;
		}

		/* calculate idx-th component data address */
		*data = img_data + offset;
	} else {
		*len = 0;
		*data = 0;
	}
}

static void image_print_type(const image_header_t *hdr)
{
	const char __maybe_unused *os, *arch, *type, *comp;

	os = genimg_get_os_name(image_get_os(hdr));
	arch = genimg_get_arch_name(image_get_arch(hdr));
	type = genimg_get_type_name(image_get_type(hdr));
	comp = genimg_get_comp_name(image_get_comp(hdr));

	printf("%s %s %s (%s)\n", arch, os, type, comp);
}

/**
 * image_print_contents - prints out the contents of the legacy format image
 * @ptr: pointer to the legacy format image header
 * @p: pointer to prefix string
 *
 * image_print_contents() formats a multi line legacy image contents description.
 * The routine prints out all header fields followed by the size/offset data
 * for MULTI/SCRIPT images.
 *
 * returns:
 *     no returned results
 */
void image_print_contents(const void *ptr)
{
	const image_header_t *hdr = (const image_header_t *)ptr;
	const char __maybe_unused *p;

	p = IMAGE_INDENT_STRING;
	printf("%sImage Name:   %.*s\n", p, IH_NMLEN, image_get_name(hdr));
	if (IMAGE_ENABLE_TIMESTAMP) {
		printf("%sCreated:      ", p);
		genimg_print_time((time_t)image_get_time(hdr));
	}
	printf("%sImage Type:   ", p);
	image_print_type(hdr);
	printf("%sData Size:    ", p);
	genimg_print_size(image_get_data_size(hdr));
	printf("%sLoad Address: %08x\n", p, image_get_load(hdr));
	printf("%sEntry Point:  %08x\n", p, image_get_ep(hdr));

	if (image_check_type(hdr, IH_TYPE_MULTI) ||
			image_check_type(hdr, IH_TYPE_SCRIPT)) {
		int i;
		ulong data, len;
		ulong count = image_multi_count(hdr);

		printf("%sContents:\n", p);
		for (i = 0; i < count; i++) {
			image_multi_getimg(hdr, i, &data, &len);

			printf("%s   Image %d: ", p, i);
			genimg_print_size(len);

			if (image_check_type(hdr, IH_TYPE_SCRIPT) && i > 0) {
				/*
				 * the user may need to know offsets
				 * if planning to do something with
				 * multiple files
				 */
				printf("%s    Offset = 0x%08lx\n", p, data);
			}
		}
	} else if (image_check_type(hdr, IH_TYPE_FIRMWARE_IVT)) {
		printf("HAB Blocks:   0x%08x   0x0000   0x%08x\n",
				image_get_load(hdr) - image_get_header_size(),
				image_get_size(hdr) + image_get_header_size()
						- 0x1FE0);
	}
}


#ifndef USE_HOSTCC
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
/**
 * image_get_ramdisk - get and verify ramdisk image
 * @rd_addr: ramdisk image start address
 * @arch: expected ramdisk architecture
 * @verify: checksum verification flag
 *
 * image_get_ramdisk() returns a pointer to the verified ramdisk image
 * header. Routine receives image start address and expected architecture
 * flag. Verification done covers data and header integrity and os/type/arch
 * fields checking.
 *
 * returns:
 *     pointer to a ramdisk image header, if image was found and valid
 *     otherwise, return NULL
 */
static const image_header_t *image_get_ramdisk(ulong rd_addr, uint8_t arch,
						int verify)
{
	const image_header_t *rd_hdr = (const image_header_t *)rd_addr;

	if (!image_check_magic(rd_hdr)) {
		puts("Bad Magic Number\n");
		bootstage_error(BOOTSTAGE_ID_RD_MAGIC);
		return NULL;
	}

	if (!image_check_hcrc(rd_hdr)) {
		puts("Bad Header Checksum\n");
		bootstage_error(BOOTSTAGE_ID_RD_HDR_CHECKSUM);
		return NULL;
	}

	bootstage_mark(BOOTSTAGE_ID_RD_MAGIC);
	image_print_contents(rd_hdr);

	if (verify) {
		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc(rd_hdr)) {
			puts("Bad Data CRC\n");
			bootstage_error(BOOTSTAGE_ID_RD_CHECKSUM);
			return NULL;
		}
		puts("OK\n");
	}

	bootstage_mark(BOOTSTAGE_ID_RD_HDR_CHECKSUM);

	if (!image_check_os(rd_hdr, IH_OS_LINUX) ||
	    !image_check_arch(rd_hdr, arch) ||
	    !image_check_type(rd_hdr, IH_TYPE_RAMDISK)) {
		printf("No Linux %s Ramdisk Image\n",
				genimg_get_arch_name(arch));
		bootstage_error(BOOTSTAGE_ID_RAMDISK);
		return NULL;
	}

	return rd_hdr;
}
#endif
#endif /* !USE_HOSTCC */

/*****************************************************************************/
/* Shared dual-format routines */
/*****************************************************************************/
#ifndef USE_HOSTCC
ulong load_addr = CONFIG_SYS_LOAD_ADDR;	/* Default Load Address */
ulong save_addr;			/* Default Save Address */
ulong save_size;			/* Default Save Size (in bytes) */

static int on_loadaddr(const char *name, const char *value, enum env_op op,
	int flags)
{
	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		load_addr = simple_strtoul(value, NULL, 16);
		break;
	default:
		break;
	}

	return 0;
}
U_BOOT_ENV_CALLBACK(loadaddr, on_loadaddr);

ulong env_get_bootm_low(void)
{
	char *s = env_get("bootm_low");
	if (s) {
		ulong tmp = simple_strtoul(s, NULL, 16);
		return tmp;
	}

#if defined(CONFIG_SYS_SDRAM_BASE)
	return CONFIG_SYS_SDRAM_BASE;
#elif defined(CONFIG_ARM)
	return gd->bd->bi_dram[0].start;
#else
	return 0;
#endif
}

phys_size_t env_get_bootm_size(void)
{
	phys_size_t tmp, size;
	phys_addr_t start;
	char *s = env_get("bootm_size");
	if (s) {
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
		return tmp;
	}

#if defined(CONFIG_ARM) && defined(CONFIG_NR_DRAM_BANKS)
	start = gd->bd->bi_dram[0].start;
	size = gd->bd->bi_dram[0].size;
#else
	start = gd->bd->bi_memstart;
	size = gd->bd->bi_memsize;
#endif

	s = env_get("bootm_low");
	if (s)
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
	else
		tmp = start;

	return size - (tmp - start);
}

phys_size_t env_get_bootm_mapsize(void)
{
	phys_size_t tmp;
	char *s = env_get("bootm_mapsize");
	if (s) {
		tmp = (phys_size_t)simple_strtoull(s, NULL, 16);
		return tmp;
	}

#if defined(CONFIG_SYS_BOOTMAPSZ)
	return CONFIG_SYS_BOOTMAPSZ;
#else
	return env_get_bootm_size();
#endif
}

void memmove_wd(void *to, void *from, size_t len, ulong chunksz)
{
	if (to == from)
		return;

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	if (to > from) {
		from += len;
		to += len;
	}
	while (len > 0) {
		size_t tail = (len > chunksz) ? chunksz : len;
		WATCHDOG_RESET();
		if (to > from) {
			to -= tail;
			from -= tail;
		}
		memmove(to, from, tail);
		if (to < from) {
			to += tail;
			from += tail;
		}
		len -= tail;
	}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
	memmove(to, from, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
}
#endif /* !USE_HOSTCC */

void genimg_print_size(uint32_t size)
{
#ifndef USE_HOSTCC
	printf("%d Bytes = ", size);
	print_size(size, "\n");
#else
	printf("%d Bytes = %.2f KiB = %.2f MiB\n",
			size, (double)size / 1.024e3,
			(double)size / 1.048576e6);
#endif
}

#if IMAGE_ENABLE_TIMESTAMP
void genimg_print_time(time_t timestamp)
{
#ifndef USE_HOSTCC
	struct rtc_time tm;

	rtc_to_tm(timestamp, &tm);
	printf("%4d-%02d-%02d  %2d:%02d:%02d UTC\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
#else
	printf("%s", ctime(&timestamp));
#endif
}
#endif

const table_entry_t *get_table_entry(const table_entry_t *table, int id)
{
	for (; table->id >= 0; ++table) {
		if (table->id == id)
			return table;
	}
	return NULL;
}

static const char *unknown_msg(enum ih_category category)
{
	static const char unknown_str[] = "Unknown ";
	static char msg[30];

	strcpy(msg, unknown_str);
	strncat(msg, table_info[category].desc,
		sizeof(msg) - sizeof(unknown_str));

	return msg;
}

/**
 * get_cat_table_entry_name - translate entry id to long name
 * @category: category to look up (enum ih_category)
 * @id: entry id to be translated
 *
 * This will scan the translation table trying to find the entry that matches
 * the given id.
 *
 * @retur long entry name if translation succeeds; error string on failure
 */
const char *genimg_get_cat_name(enum ih_category category, uint id)
{
	const table_entry_t *entry;

	entry = get_table_entry(table_info[category].table, id);
	if (!entry)
		return unknown_msg(category);
#if defined(USE_HOSTCC) || !defined(CONFIG_NEEDS_MANUAL_RELOC)
	return entry->lname;
#else
	return entry->lname + gd->reloc_off;
#endif
}

/**
 * get_cat_table_entry_short_name - translate entry id to short name
 * @category: category to look up (enum ih_category)
 * @id: entry id to be translated
 *
 * This will scan the translation table trying to find the entry that matches
 * the given id.
 *
 * @retur short entry name if translation succeeds; error string on failure
 */
const char *genimg_get_cat_short_name(enum ih_category category, uint id)
{
	const table_entry_t *entry;

	entry = get_table_entry(table_info[category].table, id);
	if (!entry)
		return unknown_msg(category);
#if defined(USE_HOSTCC) || !defined(CONFIG_NEEDS_MANUAL_RELOC)
	return entry->sname;
#else
	return entry->sname + gd->reloc_off;
#endif
}

int genimg_get_cat_count(enum ih_category category)
{
	return table_info[category].count;
}

const char *genimg_get_cat_desc(enum ih_category category)
{
	return table_info[category].desc;
}

/**
 * get_table_entry_name - translate entry id to long name
 * @table: pointer to a translation table for entries of a specific type
 * @msg: message to be returned when translation fails
 * @id: entry id to be translated
 *
 * get_table_entry_name() will go over translation table trying to find
 * entry that matches given id. If matching entry is found, its long
 * name is returned to the caller.
 *
 * returns:
 *     long entry name if translation succeeds
 *     msg otherwise
 */
char *get_table_entry_name(const table_entry_t *table, char *msg, int id)
{
	table = get_table_entry(table, id);
	if (!table)
		return msg;
#if defined(USE_HOSTCC) || !defined(CONFIG_NEEDS_MANUAL_RELOC)
	return table->lname;
#else
	return table->lname + gd->reloc_off;
#endif
}

const char *genimg_get_os_name(uint8_t os)
{
	return (get_table_entry_name(uimage_os, "Unknown OS", os));
}

const char *genimg_get_arch_name(uint8_t arch)
{
	return (get_table_entry_name(uimage_arch, "Unknown Architecture",
					arch));
}

const char *genimg_get_type_name(uint8_t type)
{
	return (get_table_entry_name(uimage_type, "Unknown Image", type));
}

static const char *genimg_get_short_name(const table_entry_t *table, int val)
{
	table = get_table_entry(table, val);
	if (!table)
		return "unknown";
#if defined(USE_HOSTCC) || !defined(CONFIG_NEEDS_MANUAL_RELOC)
	return table->sname;
#else
	return table->sname + gd->reloc_off;
#endif
}

const char *genimg_get_type_short_name(uint8_t type)
{
	return genimg_get_short_name(uimage_type, type);
}

const char *genimg_get_comp_name(uint8_t comp)
{
	return (get_table_entry_name(uimage_comp, "Unknown Compression",
					comp));
}

const char *genimg_get_comp_short_name(uint8_t comp)
{
	return genimg_get_short_name(uimage_comp, comp);
}

const char *genimg_get_os_short_name(uint8_t os)
{
	return genimg_get_short_name(uimage_os, os);
}

const char *genimg_get_arch_short_name(uint8_t arch)
{
	return genimg_get_short_name(uimage_arch, arch);
}

/**
 * get_table_entry_id - translate short entry name to id
 * @table: pointer to a translation table for entries of a specific type
 * @table_name: to be used in case of error
 * @name: entry short name to be translated
 *
 * get_table_entry_id() will go over translation table trying to find
 * entry that matches given short name. If matching entry is found,
 * its id returned to the caller.
 *
 * returns:
 *     entry id if translation succeeds
 *     -1 otherwise
 */
int get_table_entry_id(const table_entry_t *table,
		const char *table_name, const char *name)
{
	const table_entry_t *t;

	for (t = table; t->id >= 0; ++t) {
#ifdef CONFIG_NEEDS_MANUAL_RELOC
		if (t->sname && strcasecmp(t->sname + gd->reloc_off, name) == 0)
#else
		if (t->sname && strcasecmp(t->sname, name) == 0)
#endif
			return (t->id);
	}
	debug("Invalid %s Type: %s\n", table_name, name);

	return -1;
}

int genimg_get_os_id(const char *name)
{
	return (get_table_entry_id(uimage_os, "OS", name));
}

int genimg_get_arch_id(const char *name)
{
	return (get_table_entry_id(uimage_arch, "CPU", name));
}

int genimg_get_type_id(const char *name)
{
	return (get_table_entry_id(uimage_type, "Image", name));
}

int genimg_get_comp_id(const char *name)
{
	return (get_table_entry_id(uimage_comp, "Compression", name));
}

#ifndef USE_HOSTCC
/**
 * genimg_get_kernel_addr_fit - get the real kernel address and return 2
 *                              FIT strings
 * @img_addr: a string might contain real image address
 * @fit_uname_config: double pointer to a char, will hold pointer to a
 *                    configuration unit name
 * @fit_uname_kernel: double pointer to a char, will hold pointer to a subimage
 *                    name
 *
 * genimg_get_kernel_addr_fit get the real kernel start address from a string
 * which is normally the first argv of bootm/bootz
 *
 * returns:
 *     kernel start address
 */
ulong genimg_get_kernel_addr_fit(char * const img_addr,
			     const char **fit_uname_config,
			     const char **fit_uname_kernel)
{
	ulong kernel_addr;

	/* find out kernel image address */
	if (!img_addr) {
		kernel_addr = load_addr;
		debug("*  kernel: default image load address = 0x%08lx\n",
		      load_addr);
#if CONFIG_IS_ENABLED(FIT)
	} else if (fit_parse_conf(img_addr, load_addr, &kernel_addr,
				  fit_uname_config)) {
		debug("*  kernel: config '%s' from image at 0x%08lx\n",
		      *fit_uname_config, kernel_addr);
	} else if (fit_parse_subimage(img_addr, load_addr, &kernel_addr,
				     fit_uname_kernel)) {
		debug("*  kernel: subimage '%s' from image at 0x%08lx\n",
		      *fit_uname_kernel, kernel_addr);
#endif
	} else {
		kernel_addr = simple_strtoul(img_addr, NULL, 16);
		debug("*  kernel: cmdline image address = 0x%08lx\n",
		      kernel_addr);
	}

	return kernel_addr;
}

/**
 * genimg_get_kernel_addr() is the simple version of
 * genimg_get_kernel_addr_fit(). It ignores those return FIT strings
 */
ulong genimg_get_kernel_addr(char * const img_addr)
{
	const char *fit_uname_config = NULL;
	const char *fit_uname_kernel = NULL;

	return genimg_get_kernel_addr_fit(img_addr, &fit_uname_config,
					  &fit_uname_kernel);
}

/**
 * genimg_get_format - get image format type
 * @img_addr: image start address
 *
 * genimg_get_format() checks whether provided address points to a valid
 * legacy or FIT image.
 *
 * New uImage format and FDT blob are based on a libfdt. FDT blob
 * may be passed directly or embedded in a FIT image. In both situations
 * genimg_get_format() must be able to dectect libfdt header.
 *
 * returns:
 *     image format type or IMAGE_FORMAT_INVALID if no image is present
 */
int genimg_get_format(const void *img_addr)
{
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	const image_header_t *hdr;

	hdr = (const image_header_t *)img_addr;
	if (image_check_magic(hdr))
		return IMAGE_FORMAT_LEGACY;
#endif
#if IMAGE_ENABLE_FIT || IMAGE_ENABLE_OF_LIBFDT
	if (fdt_check_header(img_addr) == 0)
		return IMAGE_FORMAT_FIT;
#endif
#ifdef CONFIG_ANDROID_BOOT_IMAGE
	if (android_image_check_header(img_addr) == 0)
		return IMAGE_FORMAT_ANDROID;
#endif

	return IMAGE_FORMAT_INVALID;
}

/**
 * fit_has_config - check if there is a valid FIT configuration
 * @images: pointer to the bootm command headers structure
 *
 * fit_has_config() checks if there is a FIT configuration in use
 * (if FTI support is present).
 *
 * returns:
 *     0, no FIT support or no configuration found
 *     1, configuration found
 */
int genimg_has_config(bootm_headers_t *images)
{
#if IMAGE_ENABLE_FIT
	if (images->fit_uname_cfg)
		return 1;
#endif
	return 0;
}

/**
 * boot_get_ramdisk - main ramdisk handling routine
 * @argc: command argument count
 * @argv: command argument list
 * @images: pointer to the bootm images structure
 * @arch: expected ramdisk architecture
 * @rd_start: pointer to a ulong variable, will hold ramdisk start address
 * @rd_end: pointer to a ulong variable, will hold ramdisk end
 *
 * boot_get_ramdisk() is responsible for finding a valid ramdisk image.
 * Curently supported are the following ramdisk sources:
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
int boot_get_ramdisk(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, ulong *rd_start, ulong *rd_end)
{
	ulong rd_addr, rd_load;
	ulong rd_data, rd_len;
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
	const image_header_t *rd_hdr;
#endif
	void *buf;
#ifdef CONFIG_SUPPORT_RAW_INITRD
	char *end;
#endif
#if IMAGE_ENABLE_FIT
	const char	*fit_uname_config = images->fit_uname_cfg;
	const char	*fit_uname_ramdisk = NULL;
	ulong		default_addr;
	int		rd_noffset;
#endif
	const char *select = NULL;

	*rd_start = 0;
	*rd_end = 0;

#ifdef CONFIG_ANDROID_BOOT_IMAGE
	/*
	 * Look for an Android boot image.
	 */
	buf = map_sysmem(images->os.start, 0);
	if (buf && genimg_get_format(buf) == IMAGE_FORMAT_ANDROID)
		select = (argc == 0) ? env_get("loadaddr") : argv[0];
#endif

	if (argc >= 2)
		select = argv[1];

	/*
	 * Look for a '-' which indicates to ignore the
	 * ramdisk argument
	 */
	if (select && strcmp(select, "-") ==  0) {
		debug("## Skipping init Ramdisk\n");
		rd_len = rd_data = 0;
	} else if (select || genimg_has_config(images)) {
#if IMAGE_ENABLE_FIT
		if (select) {
			/*
			 * If the init ramdisk comes from the FIT image and
			 * the FIT image address is omitted in the command
			 * line argument, try to use os FIT image address or
			 * default load address.
			 */
			if (images->fit_uname_os)
				default_addr = (ulong)images->fit_hdr_os;
			else
				default_addr = load_addr;

			if (fit_parse_conf(select, default_addr,
					   &rd_addr, &fit_uname_config)) {
				debug("*  ramdisk: config '%s' from image at "
						"0x%08lx\n",
						fit_uname_config, rd_addr);
			} else if (fit_parse_subimage(select, default_addr,
						&rd_addr, &fit_uname_ramdisk)) {
				debug("*  ramdisk: subimage '%s' from image at "
						"0x%08lx\n",
						fit_uname_ramdisk, rd_addr);
			} else
#endif
			{
				rd_addr = simple_strtoul(select, NULL, 16);
				debug("*  ramdisk: cmdline image address = "
						"0x%08lx\n",
						rd_addr);
			}
#if IMAGE_ENABLE_FIT
		} else {
			/* use FIT configuration provided in first bootm
			 * command argument. If the property is not defined,
			 * quit silently.
			 */
			rd_addr = map_to_sysmem(images->fit_hdr_os);
			rd_noffset = fit_get_node_from_config(images,
					FIT_RAMDISK_PROP, rd_addr);
			if (rd_noffset == -ENOENT)
				return 0;
			else if (rd_noffset < 0)
				return 1;
		}
#endif

		/*
		 * Check if there is an initrd image at the
		 * address provided in the second bootm argument
		 * check image type, for FIT images get FIT node.
		 */
		buf = map_sysmem(rd_addr, 0);
		switch (genimg_get_format(buf)) {
#if defined(CONFIG_IMAGE_FORMAT_LEGACY)
		case IMAGE_FORMAT_LEGACY:
			printf("## Loading init Ramdisk from Legacy "
					"Image at %08lx ...\n", rd_addr);

			bootstage_mark(BOOTSTAGE_ID_CHECK_RAMDISK);
			rd_hdr = image_get_ramdisk(rd_addr, arch,
							images->verify);

			if (rd_hdr == NULL)
				return 1;

			rd_data = image_get_data(rd_hdr);
			rd_len = image_get_data_size(rd_hdr);
			rd_load = image_get_load(rd_hdr);
			break;
#endif
#if IMAGE_ENABLE_FIT
		case IMAGE_FORMAT_FIT:
			rd_noffset = fit_image_load(images,
					rd_addr, &fit_uname_ramdisk,
					&fit_uname_config, arch,
					IH_TYPE_RAMDISK,
					BOOTSTAGE_ID_FIT_RD_START,
					FIT_LOAD_OPTIONAL_NON_ZERO,
					&rd_data, &rd_len);
			if (rd_noffset < 0)
				return 1;

			images->fit_hdr_rd = map_sysmem(rd_addr, 0);
			images->fit_uname_rd = fit_uname_ramdisk;
			images->fit_noffset_rd = rd_noffset;
			break;
#endif
#ifdef CONFIG_ANDROID_BOOT_IMAGE
		case IMAGE_FORMAT_ANDROID:
			android_image_get_ramdisk((void *)images->os.start,
				&rd_data, &rd_len);
			break;
#endif
		default:
#ifdef CONFIG_SUPPORT_RAW_INITRD
			end = NULL;
			if (select)
				end = strchr(select, ':');
			if (end) {
				rd_len = simple_strtoul(++end, NULL, 16);
				rd_data = rd_addr;
			} else
#endif
			{
				puts("Wrong Ramdisk Image Format\n");
				rd_data = rd_len = rd_load = 0;
				return 1;
			}
		}
	} else if (images->legacy_hdr_valid &&
			image_check_type(&images->legacy_hdr_os_copy,
						IH_TYPE_MULTI)) {

		/*
		 * Now check if we have a legacy mult-component image,
		 * get second entry data start address and len.
		 */
		bootstage_mark(BOOTSTAGE_ID_RAMDISK);
		printf("## Loading init Ramdisk from multi component "
				"Legacy Image at %08lx ...\n",
				(ulong)images->legacy_hdr_os);

		image_multi_getimg(images->legacy_hdr_os, 1, &rd_data, &rd_len);
	} else {
		/*
		 * no initrd image
		 */
		bootstage_mark(BOOTSTAGE_ID_NO_RAMDISK);
		rd_len = rd_data = 0;
	}

	if (!rd_data) {
		debug("## No init Ramdisk\n");
	} else {
		*rd_start = rd_data;
		*rd_end = rd_data + rd_len;
	}
	debug("   ramdisk start = 0x%08lx, ramdisk end = 0x%08lx\n",
			*rd_start, *rd_end);

	return 0;
}

#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
/**
 * boot_ramdisk_high - relocate init ramdisk
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @rd_data: ramdisk data start address
 * @rd_len: ramdisk data length
 * @initrd_start: pointer to a ulong variable, will hold final init ramdisk
 *      start address (after possible relocation)
 * @initrd_end: pointer to a ulong variable, will hold final init ramdisk
 *      end address (after possible relocation)
 *
 * boot_ramdisk_high() takes a relocation hint from "initrd_high" environment
 * variable and if requested ramdisk data is moved to a specified location.
 *
 * Initrd_start and initrd_end are set to final (after relocation) ramdisk
 * start/end addresses if ramdisk image start and len were provided,
 * otherwise set initrd_start and initrd_end set to zeros.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_ramdisk_high(struct lmb *lmb, ulong rd_data, ulong rd_len,
		  ulong *initrd_start, ulong *initrd_end)
{
	char	*s;
	ulong	initrd_high;
	int	initrd_copy_to_ram = 1;

	s = env_get("initrd_high");
	if (s) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, 16);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {
		initrd_high = env_get_bootm_mapsize() + env_get_bootm_low();
	}


	debug("## initrd_high = 0x%08lx, copy_to_ram = %d\n",
			initrd_high, initrd_copy_to_ram);

	if (rd_data) {
		if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
			debug("   in-place initrd\n");
			*initrd_start = rd_data;
			*initrd_end = rd_data + rd_len;
			lmb_reserve(lmb, rd_data, rd_len);
		} else {
			if (initrd_high)
				*initrd_start = (ulong)lmb_alloc_base(lmb,
						rd_len, 0x1000, initrd_high);
			else
				*initrd_start = (ulong)lmb_alloc(lmb, rd_len,
								 0x1000);

			if (*initrd_start == 0) {
				puts("ramdisk - allocation error\n");
				goto error;
			}
			bootstage_mark(BOOTSTAGE_ID_COPY_RAMDISK);

			*initrd_end = *initrd_start + rd_len;
			printf("   Loading Ramdisk to %08lx, end %08lx ... ",
					*initrd_start, *initrd_end);

			memmove_wd((void *)*initrd_start,
					(void *)rd_data, rd_len, CHUNKSZ);

#ifdef CONFIG_MP
			/*
			 * Ensure the image is flushed to memory to handle
			 * AMP boot scenarios in which we might not be
			 * HW cache coherent
			 */
			flush_cache((unsigned long)*initrd_start,
				    ALIGN(rd_len, ARCH_DMA_MINALIGN));
#endif
			puts("OK\n");
		}
	} else {
		*initrd_start = 0;
		*initrd_end = 0;
	}
	debug("   ramdisk load start = 0x%08lx, ramdisk load end = 0x%08lx\n",
			*initrd_start, *initrd_end);

	return 0;

error:
	return -1;
}
#endif /* CONFIG_SYS_BOOT_RAMDISK_HIGH */

int boot_get_setup(bootm_headers_t *images, uint8_t arch,
		   ulong *setup_start, ulong *setup_len)
{
#if IMAGE_ENABLE_FIT
	return boot_get_setup_fit(images, arch, setup_start, setup_len);
#else
	return -ENOENT;
#endif
}

#if IMAGE_ENABLE_FIT
#if defined(CONFIG_FPGA)
int boot_get_fpga(int argc, char * const argv[], bootm_headers_t *images,
		  uint8_t arch, const ulong *ld_start, ulong * const ld_len)
{
	ulong tmp_img_addr, img_data, img_len;
	void *buf;
	int conf_noffset;
	int fit_img_result;
	const char *uname, *name;
	int err;
	int devnum = 0; /* TODO support multi fpga platforms */

	/* Check to see if the images struct has a FIT configuration */
	if (!genimg_has_config(images)) {
		debug("## FIT configuration was not specified\n");
		return 0;
	}

	/*
	 * Obtain the os FIT header from the images struct
	 */
	tmp_img_addr = map_to_sysmem(images->fit_hdr_os);
	buf = map_sysmem(tmp_img_addr, 0);
	/*
	 * Check image type. For FIT images get FIT node
	 * and attempt to locate a generic binary.
	 */
	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_FIT:
		conf_noffset = fit_conf_get_node(buf, images->fit_uname_cfg);

		uname = fdt_stringlist_get(buf, conf_noffset, FIT_FPGA_PROP, 0,
					   NULL);
		if (!uname) {
			debug("## FPGA image is not specified\n");
			return 0;
		}
		fit_img_result = fit_image_load(images,
						tmp_img_addr,
						(const char **)&uname,
						&(images->fit_uname_cfg),
						arch,
						IH_TYPE_FPGA,
						BOOTSTAGE_ID_FPGA_INIT,
						FIT_LOAD_OPTIONAL_NON_ZERO,
						&img_data, &img_len);

		debug("FPGA image (%s) loaded to 0x%lx/size 0x%lx\n",
		      uname, img_data, img_len);

		if (fit_img_result < 0) {
			/* Something went wrong! */
			return fit_img_result;
		}

		if (!fpga_is_partial_data(devnum, img_len)) {
			name = "full";
			err = fpga_loadbitstream(devnum, (char *)img_data,
						 img_len, BIT_FULL);
			if (err)
				err = fpga_load(devnum, (const void *)img_data,
						img_len, BIT_FULL);
		} else {
			name = "partial";
			err = fpga_loadbitstream(devnum, (char *)img_data,
						 img_len, BIT_PARTIAL);
			if (err)
				err = fpga_load(devnum, (const void *)img_data,
						img_len, BIT_PARTIAL);
		}

		if (err)
			return err;

		printf("   Programming %s bitstream... OK\n", name);
		break;
	default:
		printf("The given image format is not supported (corrupt?)\n");
		return 1;
	}

	return 0;
}
#endif

static void fit_loadable_process(uint8_t img_type,
				 ulong img_data,
				 ulong img_len)
{
	int i;
	const unsigned int count =
			ll_entry_count(struct fit_loadable_tbl, fit_loadable);
	struct fit_loadable_tbl *fit_loadable_handler =
			ll_entry_start(struct fit_loadable_tbl, fit_loadable);
	/* For each loadable handler */
	for (i = 0; i < count; i++, fit_loadable_handler++)
		/* matching this type */
		if (fit_loadable_handler->type == img_type)
			/* call that handler with this image data */
			fit_loadable_handler->handler(img_data, img_len);
}

int boot_get_loadable(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, const ulong *ld_start, ulong * const ld_len)
{
	/*
	 * These variables are used to hold the current image location
	 * in system memory.
	 */
	ulong tmp_img_addr;
	/*
	 * These two variables are requirements for fit_image_load, but
	 * their values are not used
	 */
	ulong img_data, img_len;
	void *buf;
	int loadables_index;
	int conf_noffset;
	int fit_img_result;
	const char *uname;
	uint8_t img_type;

	/* Check to see if the images struct has a FIT configuration */
	if (!genimg_has_config(images)) {
		debug("## FIT configuration was not specified\n");
		return 0;
	}

	/*
	 * Obtain the os FIT header from the images struct
	 */
	tmp_img_addr = map_to_sysmem(images->fit_hdr_os);
	buf = map_sysmem(tmp_img_addr, 0);
	/*
	 * Check image type. For FIT images get FIT node
	 * and attempt to locate a generic binary.
	 */
	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_FIT:
		conf_noffset = fit_conf_get_node(buf, images->fit_uname_cfg);

		for (loadables_index = 0;
		     uname = fdt_stringlist_get(buf, conf_noffset,
					FIT_LOADABLE_PROP, loadables_index,
					NULL), uname;
		     loadables_index++)
		{
			fit_img_result = fit_image_load(images,
				tmp_img_addr,
				&uname,
				&(images->fit_uname_cfg), arch,
				IH_TYPE_LOADABLE,
				BOOTSTAGE_ID_FIT_LOADABLE_START,
				FIT_LOAD_OPTIONAL_NON_ZERO,
				&img_data, &img_len);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}

			fit_img_result = fit_image_get_node(buf, uname);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}
			fit_img_result = fit_image_get_type(buf,
							    fit_img_result,
							    &img_type);
			if (fit_img_result < 0) {
				/* Something went wrong! */
				return fit_img_result;
			}

			fit_loadable_process(img_type, img_data, img_len);
		}
		break;
	default:
		printf("The given image format is not supported (corrupt?)\n");
		return 1;
	}

	return 0;
}
#endif

#ifdef CONFIG_SYS_BOOT_GET_CMDLINE
/**
 * boot_get_cmdline - allocate and initialize kernel cmdline
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @cmd_start: pointer to a ulong variable, will hold cmdline start
 * @cmd_end: pointer to a ulong variable, will hold cmdline end
 *
 * boot_get_cmdline() allocates space for kernel command line below
 * BOOTMAPSZ + env_get_bootm_low() address. If "bootargs" U-Boot environment
 * variable is present its contents is copied to allocated kernel
 * command line.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_cmdline(struct lmb *lmb, ulong *cmd_start, ulong *cmd_end)
{
	char *cmdline;
	char *s;

	cmdline = (char *)(ulong)lmb_alloc_base(lmb, CONFIG_SYS_BARGSIZE, 0xf,
				env_get_bootm_mapsize() + env_get_bootm_low());

	if (cmdline == NULL)
		return -1;

	s = env_get("bootargs");
	if (!s)
		s = "";

	strcpy(cmdline, s);

	*cmd_start = (ulong) & cmdline[0];
	*cmd_end = *cmd_start + strlen(cmdline);

	debug("## cmdline at 0x%08lx ... 0x%08lx\n", *cmd_start, *cmd_end);

	return 0;
}
#endif /* CONFIG_SYS_BOOT_GET_CMDLINE */

#ifdef CONFIG_SYS_BOOT_GET_KBD
/**
 * boot_get_kbd - allocate and initialize kernel copy of board info
 * @lmb: pointer to lmb handle, will be used for memory mgmt
 * @kbd: double pointer to board info data
 *
 * boot_get_kbd() allocates space for kernel copy of board info data below
 * BOOTMAPSZ + env_get_bootm_low() address and kernel board info is initialized
 * with the current u-boot board info data.
 *
 * returns:
 *      0 - success
 *     -1 - failure
 */
int boot_get_kbd(struct lmb *lmb, bd_t **kbd)
{
	*kbd = (bd_t *)(ulong)lmb_alloc_base(lmb, sizeof(bd_t), 0xf,
				env_get_bootm_mapsize() + env_get_bootm_low());
	if (*kbd == NULL)
		return -1;

	**kbd = *(gd->bd);

	debug("## kernel board info at 0x%08lx\n", (ulong)*kbd);

#if defined(DEBUG) && defined(CONFIG_CMD_BDI)
	do_bdinfo(NULL, 0, 0, NULL);
#endif

	return 0;
}
#endif /* CONFIG_SYS_BOOT_GET_KBD */

#ifdef CONFIG_LMB
int image_setup_linux(bootm_headers_t *images)
{
	ulong of_size = images->ft_len;
	char **of_flat_tree = &images->ft_addr;
	struct lmb *lmb = &images->lmb;
	int ret;

	if (IMAGE_ENABLE_OF_LIBFDT)
		boot_fdt_add_mem_rsv_regions(lmb, *of_flat_tree);

	if (IMAGE_BOOT_GET_CMDLINE) {
		ret = boot_get_cmdline(lmb, &images->cmdline_start,
				&images->cmdline_end);
		if (ret) {
			puts("ERROR with allocation of cmdline\n");
			return ret;
		}
	}

	if (IMAGE_ENABLE_OF_LIBFDT) {
		ret = boot_relocate_fdt(lmb, of_flat_tree, &of_size);
		if (ret)
			return ret;
	}

	if (IMAGE_ENABLE_OF_LIBFDT && of_size) {
		ret = image_setup_libfdt(images, *of_flat_tree, of_size, lmb);
		if (ret)
			return ret;
	}

	return 0;
}
#endif /* CONFIG_LMB */
#endif /* !USE_HOSTCC */
