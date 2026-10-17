// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <env.h>
#include <display_options.h>
#include <init.h>
#include <lmb.h>
#include <log.h>
#include <malloc.h>
#include <u-boot/crc.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
#include <status_led.h>
#endif

#if CONFIG_IS_ENABLED(FIT) || CONFIG_IS_ENABLED(OF_LIBFDT)
#include <linux/libfdt.h>
#include <fdt_support.h>
#endif

#include <asm/global_data.h>
#include <u-boot/md5.h>
#include <u-boot/sha1.h>
#include <linux/errno.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* Set this if we have less than 4 MB of malloc() space */
#if CONFIG_SYS_MALLOC_LEN < (4096 * 1024)
#define CONSERVE_MEMORY		true
#else
#define CONSERVE_MEMORY		false
#endif

#else /* USE_HOSTCC */
#include "mkimage.h"
#include <linux/kconfig.h>
#include <u-boot/md5.h>
#include <time.h>

#ifndef __maybe_unused
# define __maybe_unused		/* unimplemented */
#endif

#define CONSERVE_MEMORY		false

#endif /* !USE_HOSTCC*/

#include <abuf.h>
#include <bzlib.h>
#include <display_options.h>
#include <gzip.h>
#include <image.h>
#include <imximage.h>
#include <relocate.h>
#include <linux/lzo.h>
#include <linux/zstd.h>
#include <lzma/LzmaTypes.h>
#include <lzma/LzmaDec.h>
#include <lzma/LzmaTools.h>
#include <u-boot/crc.h>
#include <u-boot/lz4.h>

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
	{	IH_OS_OPENSBI,	"opensbi",	"RISC-V OpenSBI",	},
	{	IH_OS_EFI,	"efi",		"EFI Firmware" },

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
	{	IH_TYPE_COPRO, "copro", "Coprocessor Image"},
	{	IH_TYPE_SUNXI_EGON, "sunxi_egon",  "Allwinner eGON Boot Image" },
	{	IH_TYPE_SUNXI_TOC0, "sunxi_toc0",  "Allwinner TOC0 Boot Image" },
	{	IH_TYPE_FDT_LEGACY, "fdt_legacy", "legacy Image with Flat Device Tree ", },
	{	IH_TYPE_RENESAS_SPKG, "spkgimage", "Renesas SPKG Image" },
	{	IH_TYPE_STARFIVE_SPL, "sfspl", "StarFive SPL Image" },
	{	-1,		    "",		  "",			},
};

static const table_entry_t uimage_comp[] = {
	{	IH_COMP_NONE,	"none",		"uncompressed",		},
	{	IH_COMP_BZIP2,	"bzip2",	"bzip2 compressed",	},
	{	IH_COMP_GZIP,	"gzip",		"gzip compressed",	},
	{	IH_COMP_LZMA,	"lzma",		"lzma compressed",	},
	{	IH_COMP_LZO,	"lzo",		"lzo compressed",	},
	{	IH_COMP_LZ4,	"lz4",		"lz4 compressed",	},
	{	IH_COMP_ZSTD,	"zstd",		"zstd compressed",	},
	{	-1,		"",		"",			},
};

static const table_entry_t uimage_phase[] = {
	{	IH_PHASE_NONE,	"none",		"any",		},
	{	IH_PHASE_U_BOOT, "u-boot",	"U-Boot phase",	},
	{	IH_PHASE_SPL,	"spl",		"SPL Phase",	},
	{	-1,		"",		"",		},
};

struct table_info {
	const char *desc;
	int count;
	const table_entry_t *table;
};

static const struct comp_magic_map image_comp[] = {
	{	IH_COMP_BZIP2,	"bzip2",	{0x42, 0x5a},},
	{	IH_COMP_GZIP,	"gzip",		{0x1f, 0x8b},},
	{	IH_COMP_LZMA,	"lzma",		{0x5d, 0x00},},
	{	IH_COMP_LZO,	"lzo",		{0x89, 0x4c},},
	{	IH_COMP_LZ4,    "lz4",          {0x04, 0x22},},
	{	IH_COMP_ZSTD,   "zstd",         {0x28, 0xb5},},
	{	IH_COMP_NONE,	"none",		{},	},
};

static const struct table_info table_info[IH_COUNT] = {
	{ "architecture", IH_ARCH_COUNT, uimage_arch },
	{ "compression", IH_COMP_COUNT, uimage_comp },
	{ "operating system", IH_OS_COUNT, uimage_os },
	{ "image type", IH_TYPE_COUNT, uimage_type },
	{ "phase", IH_PHASE_COUNT, uimage_phase },
};

/*****************************************************************************/
/* Legacy format routines */
/*****************************************************************************/
int image_check_hcrc(const struct legacy_img_hdr *hdr)
{
	ulong hcrc;
	ulong len = image_get_header_size();
	struct legacy_img_hdr header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, (char *)hdr, image_get_header_size());
	image_set_hcrc(&header, 0);

	hcrc = crc32(0, (unsigned char *)&header, len);

	return (hcrc == image_get_hcrc(hdr));
}

int image_check_dcrc(const struct legacy_img_hdr *hdr)
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
ulong image_multi_count(const struct legacy_img_hdr *hdr)
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
void image_multi_getimg(const struct legacy_img_hdr *hdr, ulong idx,
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

static void image_print_type(const struct legacy_img_hdr *hdr)
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
	const struct legacy_img_hdr *hdr = (const struct legacy_img_hdr *)ptr;
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
			(int)(image_get_size(hdr) + image_get_header_size()
			+ sizeof(flash_header_v2_t) - 0x2060));
	}
}

/**
 * print_decomp_msg() - Print a suitable decompression/loading message
 *
 * @type:	OS type (IH_OS_...)
 * @comp_type:	Compression type being used (IH_COMP_...)
 * @is_xip:	true if the load address matches the image start
 * @load:	Load address for printing
 */
static void print_decomp_msg(int comp_type, int type, bool is_xip,
			     ulong load)
{
	const char *name = genimg_get_type_name(type);

	/* Shows "Loading Kernel Image" for example */
	if (comp_type == IH_COMP_NONE)
		printf("   %s %s", is_xip ? "XIP" : "Loading", name);
	else
		printf("   Uncompressing %s", name);

	printf(" to %lx\n", load);
}

int image_decomp_type(const unsigned char *buf, ulong len)
{
	const struct comp_magic_map *cmagic = image_comp;

	if (len < 2)
		return -EINVAL;

	for (; cmagic->comp_id > 0; cmagic++) {
		if (!memcmp(buf, cmagic->magic, 2))
			break;
	}

	return cmagic->comp_id;
}

int image_decomp(int comp, ulong load, ulong image_start, int type,
		 void *load_buf, void *image_buf, ulong image_len,
		 uint unc_len, ulong *load_end)
{
	int ret = -ENOSYS;

	*load_end = load;
	print_decomp_msg(comp, type, load == image_start, load);

	/*
	 * Load the image to the right place, decompressing if needed. After
	 * this, image_len will be set to the number of uncompressed bytes
	 * loaded, ret will be non-zero on error.
	 */
	switch (comp) {
	case IH_COMP_NONE:
		ret = 0;
		if (load == image_start)
			break;
		if (image_len <= unc_len)
			memmove_wd(load_buf, image_buf, image_len, CHUNKSZ);
		else
			ret = -ENOSPC;
		break;
	case IH_COMP_GZIP:
		if (!tools_build() && CONFIG_IS_ENABLED(GZIP))
			ret = gunzip(load_buf, unc_len, image_buf, &image_len);
		break;
	case IH_COMP_BZIP2:
		if (!tools_build() && CONFIG_IS_ENABLED(BZIP2)) {
			uint size = unc_len;

			/*
			 * If we've got less than 4 MB of malloc() space,
			 * use slower decompression algorithm which requires
			 * at most 2300 KB of memory.
			 */
			ret = BZ2_bzBuffToBuffDecompress(load_buf, &size,
				image_buf, image_len, CONSERVE_MEMORY, 0);
			image_len = size;
		}
		break;
	case IH_COMP_LZMA:
		if (!tools_build() && CONFIG_IS_ENABLED(LZMA)) {
			SizeT lzma_len = unc_len;

			ret = lzmaBuffToBuffDecompress(load_buf, &lzma_len,
						       image_buf, image_len);
			image_len = lzma_len;
		}
		break;
	case IH_COMP_LZO:
		if (!tools_build() && CONFIG_IS_ENABLED(LZO)) {
			size_t size = unc_len;

			ret = lzop_decompress(image_buf, image_len, load_buf, &size);
			image_len = size;
		}
		break;
	case IH_COMP_LZ4:
		if (!tools_build() && CONFIG_IS_ENABLED(LZ4)) {
			size_t size = unc_len;

			ret = ulz4fn(image_buf, image_len, load_buf, &size);
			image_len = size;
		}
		break;
	case IH_COMP_ZSTD:
		if (!tools_build() && CONFIG_IS_ENABLED(ZSTD)) {
			struct abuf in, out;

			abuf_init_set(&in, image_buf, image_len);
			abuf_init_set(&out, load_buf, unc_len);
			ret = zstd_decompress(&in, &out);
			if (ret >= 0) {
				image_len = ret;
				ret = 0;
			}
		}
		break;
	}
	if (ret == -ENOSYS) {
		printf("Unimplemented compression type %d\n", comp);
		return ret;
	}
	if (ret)
		return ret;

	*load_end = load + image_len;

	return 0;
}

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
 * genimg_get_cat_name - translate entry id to long name
 * @category: category to look up (enum ih_category)
 * @id: entry id to be translated
 *
 * This will scan the translation table trying to find the entry that matches
 * the given id.
 *
 * Return: long entry name if translation succeeds; error string on failure
 */
const char *genimg_get_cat_name(enum ih_category category, uint id)
{
	const table_entry_t *entry;

	entry = get_table_entry(table_info[category].table, id);
	if (!entry)
		return unknown_msg(category);
	return entry->lname;
}

/**
 * genimg_get_cat_short_name - translate entry id to short name
 * @category: category to look up (enum ih_category)
 * @id: entry id to be translated
 *
 * This will scan the translation table trying to find the entry that matches
 * the given id.
 *
 * Return: short entry name if translation succeeds; error string on failure
 */
const char *genimg_get_cat_short_name(enum ih_category category, uint id)
{
	const table_entry_t *entry;

	entry = get_table_entry(table_info[category].table, id);
	if (!entry)
		return unknown_msg(category);
	return entry->sname;
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
 * genimg_cat_has_id - check whether category has entry id
 * @category: category to look up (enum ih_category)
 * @id: entry id to be checked
 *
 * This will scan the translation table trying to find the entry that matches
 * the given id.
 *
 * Return: true if category has entry id; false if not
 */
bool genimg_cat_has_id(enum ih_category category, uint id)
{
	if (get_table_entry(table_info[category].table, id))
		return true;

	return false;
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
	return table->lname;
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

const char *genimg_get_comp_name(uint8_t comp)
{
	return (get_table_entry_name(uimage_comp, "Unknown Compression",
					comp));
}

const char *genimg_get_phase_name(enum image_phase_t phase)
{
	return get_table_entry_name(uimage_phase, "Unknown Phase", phase);
}

static const char *genimg_get_short_name(const table_entry_t *table, int val)
{
	table = get_table_entry(table, val);
	if (!table)
		return "unknown";
	return table->sname;
}

const char *genimg_get_type_short_name(uint8_t type)
{
	return genimg_get_short_name(uimage_type, type);
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
		if (t->sname && !strcasecmp(t->sname, name))
			return t->id;
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

int genimg_get_phase_id(const char *name)
{
	return get_table_entry_id(uimage_phase, "Phase", name);
}
