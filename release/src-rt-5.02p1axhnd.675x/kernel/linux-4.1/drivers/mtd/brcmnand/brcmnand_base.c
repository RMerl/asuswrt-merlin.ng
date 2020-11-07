#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 *  drivers/mtd/brcmnand/brcmnand_base.c
 *
   <:copyright-BRCM:2002:GPL/GPL:standard
   
      Copyright (c) 2002 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>
#include <linux/reboot.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/compiler.h>

#include <asm/io.h>
#include <asm/bug.h>
#include <asm/delay.h>
#include <linux/mtd/mtd64.h>
#include <asm-generic/gcclib.h>
#include <linux/slab.h>
#include "bcm_map_part.h"
#include "board.h"
#include "shared_utils.h"

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
#define NAND_COMPLEX_OOB_WRITE  0x00400000
#endif

#define DEBUG(...) do { } while (0)

//#include "bbm.h"

#include "brcmnand_priv.h"

#define PRINTK(...) do {} while (0)
//#define PRINTK printk
//static char brcmNandMsg[1024];

//#define DEBUG_HW_ECC

//#define BRCMNAND_READ_VERIFY
#undef BRCMNAND_READ_VERIFY

//#ifdef CONFIG_MTD_BRCMNAND_VERIFY_WRITE
//#define BRCMNAND_WRITE_VERIFY
//#endif
#undef BRCMNAND_WRITE_VERIFY

//#define DEBUG_ISR
#undef DEBUG_ISR
#if defined( DEBUG_ISR )  || defined(BRCMNAND_READ_VERIFY) \
	|| defined(BRCMNAND_WRITE_VERIFY)
#if defined(DEBUG_ISR )  || defined(BRCMNAND_READ_VERIFY)
#define EDU_DEBUG_4
#endif
#if defined(DEBUG_ISR )  || defined(BRCMNAND_WRITE_VERIFY)
#define EDU_DEBUG_5
#endif
#endif


#define PLATFORM_IOFLUSH_WAR()


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
// Block0
#define BCHP_NAND_ACC_CONTROL_0_BCH_4           (BRCMNAND_ECC_BCH_4 << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT)

// Block n > 0
#define BCHP_NAND_ACC_CONTROL_N_BCH_4           (BRCMNAND_ECC_BCH_4 << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT)
#endif

int gdebug = 0;
extern int edu_debug;


// Whether we should clear the BBT to fix a previous error.
/* This will eventually be on the command line, to allow a user to
 * clean the flash
 */
extern int gClearBBT;

/* Number of NAND chips, only applicable to v1.0+ NAND controller */
extern int gNumNandCS;

/* The Chip Select [0..7] for the NAND chips from gNumNand above, only applicable to v1.0+ NAND controller */
extern int gNandCS[];
extern uint32_t gNandConfig[];
extern uint32_t gAccControl[];

// If wr_preempt_en is enabled, need to disable IRQ during NAND I/O
int wr_preempt_en = 0;

// Last known good ECC sector offset (512B sector that does not generate ECC error).
// used in HIF_INTR2 WAR.
loff_t gLastKnownGoodEcc;

#define DRIVER_NAME     "brcmnand"

#define HW_AUTOOOB_LAYOUT_SIZE          32 /* should be enough */


#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
/* Avoid infinite recursion between brcmnand_refresh_blk() and brcmnand_read_ecc() */
static atomic_t inrefresh = ATOMIC_INIT(0);
static int brcmnand_refresh_blk(struct mtd_info *, loff_t);
static int brcmnand_erase_nolock(struct mtd_info *, struct erase_info *, int);
#endif

/*
 * ID options
 */
#define BRCMNAND_ID_HAS_BYTE3           0x00000001
#define BRCMNAND_ID_HAS_BYTE4           0x00000002
#define BRCMNAND_ID_HAS_BYTE5           0x00000004
#define BRCMNAND_ID_HYNIX_LEGACY        0x00010000

// TYPE2
#define BRCMNAND_ID_HAS_BYTE4_T2                0x00000008
#define BRCMNAND_ID_HAS_BYTE5_T2                0x00000010
#define BRCMNAND_ID_HAS_BYTE6_T2                0x00000020

#define BRCMNAND_ID_EXT_BYTES \
	(BRCMNAND_ID_HAS_BYTE3 | BRCMNAND_ID_HAS_BYTE4 | BRCMNAND_ID_HAS_BYTE5)

#define BRCMNAND_ID_EXT_BYTES_TYPE2 \
	(BRCMNAND_ID_HAS_BYTE3 | BRCMNAND_ID_HAS_BYTE4_T2 | \
	 BRCMNAND_ID_HAS_BYTE5_T2 | BRCMNAND_ID_HAS_BYTE6_T2)


// MICRON M60A is similar to Type 1 with a few exceptions.
#define BRCMNAND_ID_HAS_MICRON_M60A     0x00020000
#define BRCMNAND_ID_EXT_MICRON_M60A     BRCMNAND_ID_EXT_BYTES

// MICRON M61A ID encoding is a totally different (and dying beast, temporarily until ONFI)
#define BRCMNAND_ID_HAS_MICRON_M61A     0x00040000

#define BRCMNAND_ID_EXT_MICRON_M61A (BRCMNAND_ID_HAS_MICRON_M61A)

#define BRCMNAND_ID_HAS_MICRON_M68A     0x00080000
#define BRCMNAND_ID_EXT_MICRON_M68A \
	(BRCMNAND_ID_HAS_MICRON_M60A | BRCMNAND_ID_HAS_MICRON_M68A)


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0

#define ONFI_RDPARAM_SIGNATURE_OFS        0
#define ONFI_NBR_PARAM_PAGE_OFS          14
#define ONFI_RDPARAM_PAGESIZE_OFS        80
#define ONFI_RDPARAM_OOBSIZE_OFS         84
#define ONFI_RDPARAM_ECC_LEVEL_OFS      112
#define ONFI_NBR_BITS_PER_CELL_OFS      102

/*
 * The following def is for a dev with 3 replica of the param page
 * Need to be adjusted according based on the actual nbr of param pages.
 */

#define ONFI_EXTPARAM_OFS               768
#define ONFI_EXTPARAM_SIG1_OFS          768
#define ONFI_EXTPARAM_SIG2_OFS          772
#define ONFI_EXTPARAM_EXT_ECC_OFS       800
#define ONFI_EXTPARAM_CODEWORK_OFS      801



#define ONFI_SIGNATURE          0x4F4E4649      /* "ONFI" */
#define ONFI_EXTPARAM_SIG       0x45505053      /* "EPPS" */

#endif

typedef struct brcmnand_chip_Id {
	uint8 mafId, chipId;
	uint8 chipId345[3];             /* ID bytes 3,4,5: Resolve ambiguity in chipId */
	const char* chipIdStr;
	uint32 eccLevel;                /* Only for Samsung Type 2 */
	uint32 sectorSize;              /* Only for Samsung Type 2 */
	uint32 nbrBlocks;               // Only for devices that do not encode Size into ID string.
	uint32 options;
	uint32 idOptions;               // Whether chip has all 5 ID bytes
	uint32 timing1, timing2;        // Specify a non-zero value to override the default timings.
	int nop;                        // Number of partial writes per page
	unsigned int ctrlVersion;       // Required controller version if different than 0
} brcmnand_chip_Id;

/*
 * List of supported chip
 */
static brcmnand_chip_Id brcmnand_chips[] = {
	{       /* 0a */
		.chipId		= SAMSUNG_K9F1G08U0E,
		.chipId345	= { 0x00, 0x95, 0x41 },
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9F1G08U0E",
		.options	= NAND_BBT_USE_FLASH | NAND_COMPLEX_OOB_WRITE,  /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE  /* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 0b */
		.chipId		= SAMSUNG_K9F1G08U0A,
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9F1G08U0A/B/C/D",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= 0,
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.timing1	= 0,                                            //00070000,
		.timing2	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,    /* THT Verified on data-sheet 7/10/08: Allows 4 on main and 4 on OOB */
	},

	{       /* 1 */
		.chipId		= ST_NAND512W3A,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST ST_NAND512W3A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,   //0x6474555f,
		.timing2	= 0,   //0x00000fc7,
		.nop		= 8,
		.ctrlVersion	= 0,
	},
	{       /* 2 */
		.chipId		= ST_NAND256W3A,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST ST_NAND256W3A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,  //0x6474555f,
		.timing2	= 0,  //0x00000fc7,
		.nop		= 8,
		.ctrlVersion	= 0,
	},
#if 0           // EOL
	{       /* 4 */
		.chipId		= HYNIX_HY27UF081G2M,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "HYNIX HY27UF081G2M",
		.options	= NAND_BBT_USE_FLASH
		,
	},
#endif
	/* This is the new version of HYNIX_HY27UF081G2M which is EOL.
	 * Both use the same DevID
	 */
	{       /* 3 */
		.chipId		= HYNIX_HY27UF081G2A,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "Hynix HY27UF081G2A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

#if 0
/* Obsoleted by the new Micron flashes */
	{       /* 4 */
		.chipId		= MICRON_MT29F2G08AAB,
		.mafId		= FLASHTYPE_MICRON,
		.chipIdStr	= "MICRON_MT29F2G08AAB",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,						.timing2 = 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},
/* This is just the 16 bit version of the above?
        {
                .chipId = MICRON_MT29F2G16AAB,
                .mafId = FLASHTYPE_MICRON,
                .chipIdStr = "MICRON_MT29F2G16AAB",
                .options = NAND_BBT_USE_FLASH
                        ,
        }
 */
#endif
	{       /* 5 */
		.chipId		= SAMSUNG_K9F2G08U0A,
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9F2G08U0A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_1,
	},

#if 0
/*
 * SW3556-862, SWLINUX-1459
 * Samsung replaced this SLC part with a new SLC part, different block size and page size but re-use the same ID
 * Side effect: The old flash part can no longer be supported.
 */
	{       /* 6 */
		.chipId		= SAMSUNG_K9K8G08U0A,
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9K8G08U0A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,						.timing2 = 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_1,
	},
#else
	{       /* 6 Same old ID 0xD3, new part, so the old #define macro is kept, but IDstr is changed to reflect new part number */
		.chipId		= SAMSUNG_K9K8G08U0A,
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9F8G08U0M",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= BRCMNAND_ID_EXT_BYTES,                        /* New Samsung SLC has all 5 ID bytes defined */
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_1,
	},
#endif


	{       /* 7 */
		.chipId		= HYNIX_HY27UF082G2A,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "Hynix HY27UF082G2A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1 = 0x00420000,
		.timing2 = 0x00000005,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

#if 0
/* EOL replaced by the following entry, with reduced NOP */

	{       /* 8 */
		.chipId		= HYNIX_HY27UF084G2M,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "Hynix HY27UF084G2M",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,						.timing2 = 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},
#endif

	{       /* 8 */
		.chipId		= HYNIX_HY27U4G8F2D,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "Hynix HY27U4G8F2D",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= BRCMNAND_ID_EXT_BYTES | BRCMNAND_ID_HYNIX_LEGACY,
		.timing1 = 0x00420000,
		.timing2 = 0x00000005,
		.nop		= 4,
		.ctrlVersion	= 0,
	},

	{       /* 9 */
		.chipId		= SPANSION_S30ML512P_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML512P_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 10 */
		.chipId		= SPANSION_S30ML512P_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML512P_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 11 */
		.chipId		= SPANSION_S30ML256P_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML256P_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 12 */
		.chipId		= SPANSION_S30ML256P_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML256P_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 13 */
		.chipId		= SPANSION_S30ML128P_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML128P_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 14 */
		.chipId		= SPANSION_S30ML128P_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION S30ML128P_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 15 */
		.chipId		= SPANSION_S30ML01GP_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML01GP_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 16 */
		.chipId		= SPANSION_S30ML01GP_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML01GP_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 17 */
		.chipId		= SPANSION_S30ML02GP_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML02GP_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 18 */
		.chipId		= SPANSION_S30ML02GP_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML02GP_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 19 */
		.chipId		= SPANSION_S30ML04GP_08,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML04GP_08",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 20 */
		.chipId		= SPANSION_S30ML04GP_16,
		.mafId		= FLASHTYPE_SPANSION,
		.chipIdStr	= "SPANSION_S30ML04GP_16",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	{       /* 21 */
		.chipId		= ST_NAND128W3A,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND128W3A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 8,
		.ctrlVersion	= 0,
	},

	/* The following 6 ST chips only allow 4 writes per page, and requires version2.1 (4) of the controller or later */
	{       /* 22 */
		.chipId		= ST_NAND01GW3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND01GW3B2B",
		.nbrBlocks	= 1024,                                         /* size=128MB, bsize=128K */
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

#if 0
//R version = 1.8V
	{       /* 23 */
		.chipId		= ST_NAND01GR3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND01GR3B2B",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,						.timing2 = 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

	{       /* 24 */
		.chipId		= ST_NAND02GR3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND02GR3B2C",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,						.timing2 = 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},
#endif
	{       /* 25 */
		.chipId		= ST_NAND02GW3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND02GW3B2C",
		.nbrBlocks	= 2048,                                         /* size=256MB, bsize=128K */
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

	{       /* 26 */
		.chipId		= ST_NAND04GW3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND04GW3B2B",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},
	{       /* 27 */
		.chipId		= ST_NAND08GW3B,
		.mafId		= FLASHTYPE_ST,
		.chipIdStr	= "ST NAND08GW3B2A",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= 0,
		.timing1	= 0,
		.timing2 	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

	{       /* 28a */
		.chipId		= SAMSUNG_K9LBG08U0M,
		.chipId345	= { 0x55, 0xB6,  0x78 },
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9LBG08U0M",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 28b */
		.chipId		= SAMSUNG_K9LBG08U0D,
		.chipId345	= { 0xD5, 0x29,  0x38 },
		.nbrBlocks	= 8192,
		//.eccLevel = 8, ,  Will decode from ID string
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9LBG08UD",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES_TYPE2,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{                                                                                       /* 28c */
		.chipId		= SAMSUNG_K9LBG08U0E,
		.chipId345	= { 0xC5, 0x72,  0x54 },  /* C5h, 72h, 54h, 42h */
		.nbrBlocks	= 4096,                                                         /* 4GB flash */
		//.eccLevel = 24 per 1KB,  Will decode from ID string
		.mafId		= FLASHTYPE_SAMSUNG,
		.chipIdStr	= "Samsung K9LBG08UD",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES_TYPE2,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_5_0,
	},

	{       /* 29a */
		.chipId		= SAMSUNG_K9GAG08U0D,
		.chipId345	= { 0x94, 0x29,  0x34 },
		.mafId		= FLASHTYPE_SAMSUNG,
		.nbrBlocks	= 4096,
		//.eccLevel = 8 ,  Will decode from ID string
		.chipIdStr	= "Samsung K9GAG08U0D",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES_TYPE2,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{                                                                                       /* 29b */
		.chipId		= SAMSUNG_K9GAG08U0E,
		.chipId345	= { 0x84, 0x72,  0x50 },  /* 84h, 72h, 50h, 42h */
		.mafId		= FLASHTYPE_SAMSUNG,
		.nbrBlocks	= 2048,
		//.eccLevel = 24 per 1KB,  Will decode from ID string
		.chipIdStr	= "Samsung K9GAG08U0E",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES_TYPE2,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_5_0,
	},

	{       /* 30 */
		.chipId		= HYNIX_HY27UT088G2A,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "HYNIX_HY27UT088G2A",
		.options	= NAND_BBT_USE_FLASH | NAND_SCAN_BI_3RD_PAGE,   /* BBT on flash + BI on (last-2) page */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1 = 0x00420000,
		.timing2 = 0x00000005,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 31 */
		.chipId		= HYNIX_HY27UAG8T2M,
		.mafId		= FLASHTYPE_HYNIX,
		.chipIdStr	= "HYNIX_HY27UAG8T2M",
		.options	= NAND_BBT_USE_FLASH | NAND_SCAN_BI_3RD_PAGE,   /* BBT on flash + BI on (last-2) page */
		//| NAND_COMPLEX_OOB_WRITE	/* Write data together with OOB for write_oob */
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 1,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 32 */
		.chipId		= TOSHIBA_TC58NVG0S3ETA00,
		.mafId		= FLASHTYPE_TOSHIBA,
		.chipIdStr	= "TOSHIBA TC58NVG0S3ETA00",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.eccLevel	= 1,
		.nbrBlocks	= 1024,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

	{       /* 33 */
		.chipId		= TOSHIBA_TC58NVG1S3ETAI5,
		.mafId		= FLASHTYPE_TOSHIBA,
		.chipIdStr	= "TOSHIBA TC58NVG1S3ETAI5",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.eccLevel	= 1,
		.nbrBlocks	= 2048,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_2_0,
	},

	{       /* 34 */
		.chipId		= TOSHIBA_TC58NVG3S0ETA00,
		.mafId		= FLASHTYPE_TOSHIBA,
		.chipIdStr	= "TOSHIBA TC58NVG3S0ETA00",
		.options	= NAND_BBT_USE_FLASH,
		.idOptions	= BRCMNAND_ID_EXT_BYTES,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.eccLevel	= 4,
		.nbrBlocks	= 4096,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 35 */
		.chipId		= MICRON_MT29F1G08ABA,
		.mafId		= FLASHTYPE_MICRON,
		.chipIdStr	= "MICRON MT29F1G08ABA",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M68A,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 36 */
		.chipId		= MICRON_MT29F2G08ABA,
		.mafId		= FLASHTYPE_MICRON,
		.chipIdStr	= "MICRON MT29F2G08ABA",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M68A,                  /* 69A actually */
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 37 */
		.chipId		= MICRON_MT29F4G08ABA,
		.mafId		= FLASHTYPE_MICRON,
		.chipIdStr	= "MICRON MT29F4G08ABA",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M60A,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

	{       /* 38 */
		.chipId		= MICRON_MT29F8G08ABA,
		.mafId		= FLASHTYPE_MICRON,
		.chipIdStr	= "MICRON MT29F8G08ABA",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M61A,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,
	},

#if 0
/* New Chip ID scheme in place and working, but as of 2631-2.5 these do not work yet, for some unknown reason */

	{       /* 37 */
		.mafId		= FLASHTYPE_MICRON,
		.chipId		= MICRON_MT29F16G08ABA,
		.chipId345	= { 0x00,					0x26,  0x89 },
		.chipIdStr	= "MICRON MT29F16G08ABA SLC",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M61A,
		.timing1	= 0xFFFFFFFF,
		.timing2	= 0xFFFFFFFF,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_0,                 /* Require BCH-8 only */
	},

	{       /* 38 */
		.mafId		= FLASHTYPE_MICRON,
		.chipId		= MICRON_MT29F16G08CBA,
		.chipId345	= { 0x04,					0x46,  0x85 },
		.chipIdStr	= "MICRON MT29F16G08CBA MLC",
		.options	= NAND_BBT_USE_FLASH,                           /* Use BBT on flash */
		.idOptions	= BRCMNAND_ID_EXT_MICRON_M61A,
		.timing1	= 0,
		.timing2	= 0,
		.nop		= 4,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_3_3,                 /* Require BCH-12 */
	},
#endif

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0

	{       /* ONFI ENTRY */
		.chipId		= 0xFF,
		.mafId		= 0xFF,
		.chipIdStr	= "ONFI NAND CHIP",
		.options	= NAND_BBT_USE_FLASH,
		.timing1	= 0,						.timing2 = 0,
		.ctrlVersion	= CONFIG_MTD_BRCMNAND_VERS_4_0,                 /* ONFI capable NAND controllers */
	},
#endif

	{       /* LAST DUMMY ENTRY */
		.chipId		= 0,
		.mafId		= 0,
		.chipIdStr	= "UNKNOWN NAND CHIP",
		.options	= NAND_BBT_USE_FLASH,
		.timing1	= 0,						.timing2 = 0,
		.ctrlVersion	= 0,
	}
};



// Max chip account for the last dummy entry
#define BRCMNAND_MAX_CHIPS (ARRAY_SIZE(brcmnand_chips) - 1)

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0

#define BRCMNAND_ONFI_IDX (BRCMNAND_MAX_CHIPS - 1)
#endif

#include <mtd/brcmnand_oob.h> /* BRCMNAND controller defined OOB */

static unsigned char ffchars[BRCMNAND_FCACHE_SIZE];

//static unsigned char eccmask[128]; // Will be initialized during probe


static uint32_t brcmnand_registerHoles[] = {

	// 3.2 and earlier
	0x1c,
	0x44, 0x4c,
	0x5c,
	0x88, 0x8c,
	0xb8, 0xbc,
#if CONFIG_MTD_BRCMNAND_VERSION >=  CONFIG_MTD_BRCMNAND_VERS_3_3
	0xc4, 0xc8,  0xcc,
#ifndef BCHP_NAND_ACC_CONTROL_CS3
	0xf0, 0xf4,  0xf8,  0xfc,
#endif
  #if CONFIG_MTD_BRCMNAND_VERSION >=  CONFIG_MTD_BRCMNAND_VERS_3_4
	0x100,0x104, 0x108, 0x10c,
  #endif
	0x110,0x114, 0x118, 0x11c,
	0x120,0x124, 0x128, 0x12c,
#endif
};

static int brcmnand_wait(struct mtd_info *mtd, int state, uint32_t* pStatus, int timeout);
static int brcmnand_read_page(struct mtd_info *mtd, uint8_t *outp_buf, uint8_t* outp_oob, uint64_t page);

// Is there a register at the location
static int inRegisterHoles(uint32_t reg)
{
	int i;
	// Alas, 7420c0 and later register offsets are 0x0044xxxx compared to 0x0000xxxx in earlier versions
	uint32_t regOffset = reg - BCHP_NAND_REVISION;

	for (i = 0; i < ARRAY_SIZE(brcmnand_registerHoles); i++) {
		if (regOffset == brcmnand_registerHoles[i])
			return 1;       // In register hole
	}
	return 0;                       // Not in hole
}


static uint32_t brcmnand_ctrl_read(uintptr_t nandCtrlReg)
{
	uintptr_t pReg = (BRCMNAND_CTRL_REGS
			  + nandCtrlReg - BCHP_NAND_REVISION);

	if (nandCtrlReg < BCHP_NAND_REVISION || nandCtrlReg > BCHP_NAND_LAST_REG ||
	    (nandCtrlReg & 0x3) != 0) {
		//printk("brcmnand_ctrl_read: Invalid register value %08x\n", nandCtrlReg);
		return 0;
	}
	if (gdebug > 3) printk("%s: CMDREG=%p xval=0x%08x\n", __FUNCTION__,
			       (void*)nandCtrlReg, (uint32_t)BDEV_RD(pReg));

	return (uint32_t)BDEV_RD(pReg);
}


static void brcmnand_ctrl_write(uintptr_t nandCtrlReg, uint32_t val)
{
	uintptr_t pReg = (uintptr_t)(BRCMNAND_CTRL_REGS + nandCtrlReg - BCHP_NAND_REVISION);

	if (nandCtrlReg < BCHP_NAND_REVISION || nandCtrlReg > BCHP_NAND_LAST_REG ||
	    (nandCtrlReg & 0x3) != 0) {
		printk( "brcmnand_ctrl_read: Invalid register value %p\n", (void*)nandCtrlReg);
	}

	BDEV_WR(pReg, val);

	if (gdebug > 3) printk("%s: CMDREG=%p val=%08x\n", __FUNCTION__, (void*)nandCtrlReg, val);
}


/*
 * chip: BRCM NAND handle
 * offset: offset from start of mtd, not necessarily the same as offset from chip.
 * cmdEndAddr: 1 for CMD_END_ADDRESS, 0 for CMD_ADDRESS
 *
 * Returns the real ldw of the address w.r.t. the chip.
 */

#if 0 // CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_3
/*
 * Old codes assume all CSes have the same flash
 * Here offset is the offset from CS0.
 */
static uint32_t brcmnand_ctrl_writeAddr(struct brcmnand_chip* chip, loff_t offset, int cmdEndAddr)
{
#if CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_0_1
	uint32_t pAddr = offset + chip->pbase;
	uint32_t ldw = 0;

	chip->ctrl_write(cmdEndAddr ? BCHP_NAND_CMD_END_ADDRESS : BCHP_NAND_CMD_ADDRESS, pAddr);

#else
	uint32_t udw, ldw, cs;
	DIunion chipOffset;

//char msg[24];


	// cs is the index into chip->ctrl->CS[]
	cs = (uint32_t)(offset >> chip->chip_shift);
	// chipOffset is offset into the current CS

	chipOffset.ll = offset & (chip->chipSize - 1);

	if (cs >= chip->ctrl->numchips) {
		printk(KERN_ERR "%s: Offset=%0llx outside of chip range cs=%d, chip->ctrl->CS[cs]=%d\n",
		       __FUNCTION__,  offset, cs, chip->ctrl->CS[cs]);
		BUG();
		return 0;
	}

	if (gdebug) printk("CS=%d, chip->ctrl->CS[cs]=%d\n", cs, chip->ctrl->CS[cs]);
	// ldw is lower 32 bit of chipOffset, need to add pbase when on CS0 and XOR is ON.
	if (!chip->xor_disable[cs]) {
		ldw = chipOffset.s.low + chip->pbase;
	}else  {
		ldw = chipOffset.s.low;
	}

	udw = chipOffset.s.high | (chip->ctrl->CS[cs] << 16);

	if (gdebug > 3) printk("%s: offset=%0llx  cs=%d ldw = %08x, udw = %08x\n", __FUNCTION__, offset, cs,  ldw, udw);
	chip->ctrl_write(cmdEndAddr ? BCHP_NAND_CMD_END_ADDRESS : BCHP_NAND_CMD_ADDRESS, ldw);
	chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, udw);


#endif
	return (ldw); //(ldw ^ 0x1FC00000);
}

#else
/*
 * Controller v3.3 or later allows heterogenous flashes
 * Here offset is the offset from the start of the flash (CSn), as each flash has its own mtd handle
 */

static uint32_t brcmnand_ctrl_writeAddr(struct brcmnand_chip* chip, loff_t offset, int cmdEndAddr)
{
	uint32_t udw, ldw, cs;
	DIunion chipOffset;

	chipOffset.ll = offset & (chip->chipSize - 1);
	cs = chip->ctrl->CS[chip->csi];
//if (gdebug) printk("CS=%d, chip->ctrl->CS[cs]=%d\n", cs, chip->ctrl->CS[chip->csi]);
	// ldw is lower 32 bit of chipOffset, need to add pbase when on CS0 and XOR is ON.
	if (!chip->xor_disable) {
		ldw = chipOffset.s.low + chip->pbase;
	}else  {
		ldw = chipOffset.s.low;
	}

	udw = chipOffset.s.high | (cs << 16);

	if (gdebug > 3) printk("%s: offset=%0llx  cs=%d ldw = %08x, udw = %08x\n", __FUNCTION__, offset, cs,  ldw, udw);
	chip->ctrl_write(cmdEndAddr ? BCHP_NAND_CMD_END_ADDRESS : BCHP_NAND_CMD_ADDRESS, ldw);
	chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, udw);

	return (ldw);
}

#endif

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_3
/*
 * Workaround until threshold register is replicated for each CS
 */
static void
brcmnand_reset_corr_threshold(struct brcmnand_chip* chip)
{
	static int once[NUM_NAND_CS];

	if (chip->ecclevel != 0 && chip->ecclevel != BRCMNAND_ECC_HAMMING) {
		uint32_t corr_threshold = brcmnand_ctrl_read(BCHP_NAND_CORR_STAT_THRESHOLD) & BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_MASK;
		uint32_t seventyfivepc;

		seventyfivepc = (chip->ecclevel * 3) / 4;
		if (!once[chip->csi]) {
			once[chip->csi] = 1;
			printk(KERN_INFO "%s: default CORR ERR threshold  %d bits for CS%1d\n",
			       __FUNCTION__, corr_threshold, chip->ctrl->CS[chip->csi]);
			PRINTK("ECC level threshold default value is %d bits for CS%1d\n", corr_threshold, chip->ctrl->CS[chip->csi]);
		}
		if (seventyfivepc != corr_threshold) {
			if ((once[chip->csi])++ < 2) {
				printk(KERN_INFO "%s: CORR ERR threshold changed to %d bits for CS%1d\n",
				       __FUNCTION__, seventyfivepc, chip->ctrl->CS[chip->csi]);
			}
			seventyfivepc <<= BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_SHIFT;
			seventyfivepc |= (brcmnand_ctrl_read(BCHP_NAND_CORR_STAT_THRESHOLD) & ~BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_MASK);
			brcmnand_ctrl_write(BCHP_NAND_CORR_STAT_THRESHOLD, seventyfivepc);
		}
	}
}

#else
#define brcmnand_reset_corr_threshold(chip)
#endif

/*
 * Disable ECC, and return the original ACC register (for restore)
 */
uint32_t brcmnand_disable_read_ecc(int cs)
{
	uint32_t acc0;
	uint32_t acc;

	/* Disable ECC */
	acc0 = brcmnand_ctrl_read(bchp_nand_acc_control(cs));
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
	acc = acc0 & ~(BCHP_NAND_ACC_CONTROL_RD_ECC_EN_MASK | BCHP_NAND_ACC_CONTROL_RD_ECC_BLK0_EN_MASK);
#else
	acc = acc0 & ~(BCHP_NAND_ACC_CONTROL_RD_ECC_EN_MASK);
#endif
	brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc);

	return acc0;
}


void brcmnand_restore_ecc(int cs, uint32_t orig_acc0)
{
	brcmnand_ctrl_write(bchp_nand_acc_control(cs), orig_acc0);
}

// Restore acc

#if 0
/* Dont delete, may be useful for debugging */

static void __maybe_unused print_diagnostics(struct brcmnand_chip* chip)
{
	uint32_t nand_acc_control = brcmnand_ctrl_read(BCHP_NAND_ACC_CONTROL);
	uint32_t nand_select = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);
	uint32_t nand_config = brcmnand_ctrl_read(BCHP_NAND_CONFIG);
	uint32_t flash_id = brcmnand_ctrl_read(BCHP_NAND_FLASH_DEVICE_ID);
	uint32_t pageAddr = brcmnand_ctrl_read(BCHP_NAND_PROGRAM_PAGE_ADDR);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	uint32_t pageAddrExt = brcmnand_ctrl_read(BCHP_NAND_PROGRAM_PAGE_EXT_ADDR);
#endif


	//unsigned long nand_timing1 = brcmnand_ctrl_read(BCHP_NAND_TIMING_1);
	//unsigned long nand_timing2 = brcmnand_ctrl_read(BCHP_NAND_TIMING_2);

	printk(KERN_INFO "NAND_SELECT=%08x ACC_CONTROL=%08x, \tNAND_CONFIG=%08x, FLASH_ID=%08x\n",
	       nand_select, nand_acc_control, nand_config, flash_id);
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	printk("PAGE_EXT_ADDR=%08x\n", pageAddrExt);
#endif
	if (chip->ctrl->CS[0] == 0) {
		uint32_t ebiCSBase0 = BDEV_RD(BCHP_EBI_CS_BASE_0);
		printk(KERN_INFO "PAGE_ADDR=%08x, \tCS0_BASE=%08x\n", pageAddr, ebiCSBase0);
	}else  {
		uint32_t csNandBaseN = BDEV_RD(BCHP_EBI_CS_BASE_0 + 8 * chip->ctrl->CS[0]);

		printk(KERN_INFO "PAGE_ADDR=%08x, \tCS%-d_BASE=%08x\n", pageAddr, chip->ctrl->CS[0], csNandBaseN);
		printk(KERN_INFO "pbase=%08lx, vbase=%p\n", chip->pbase, chip->vbase);
	}
}
#endif

static void print_config_regs(struct mtd_info* mtd)
{
	struct brcmnand_chip * chip = mtd->priv;

	unsigned int cs = chip->ctrl->CS[chip->csi];
	uint32_t nand_acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));
	uint32_t nand_config = brcmnand_ctrl_read(bchp_nand_config(cs));

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_1
	uint32_t nand_config_ext = brcmnand_ctrl_read(BCHP_NAND_CONFIG_EXT);
#endif
	uint32_t flash_id; // = brcmnand_ctrl_read(BCHP_NAND_FLASH_DEVICE_ID);
	uint32_t nand_timing1 = brcmnand_ctrl_read(bchp_nand_timing1(cs));
	uint32_t nand_timing2 = brcmnand_ctrl_read(bchp_nand_timing2(cs));
	uint32_t status;

	/*
	 * Set CS before reading ID, same as in brcmnand_read_id
	 */

	/* Wait for CTRL_Ready */
	brcmnand_wait(mtd, BRCMNAND_FL_READY, &status, 10000);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	/* Older version do not have EXT_ADDR registers */
	chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
	chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, cs << BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);
#endif  // Set EXT address if version >= 1.0

	/* Send the command for reading device ID from controller */
	chip->ctrl_write(BCHP_NAND_CMD_START, OP_DEVICE_ID_READ);

	/* Wait for CTRL_Ready */
	brcmnand_wait(mtd, BRCMNAND_FL_READY, &status, 10000);


	flash_id = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_1
	printk(KERN_INFO "\nFound NAND on CS%1d: ACC=%08x, cfg=%08x, cfg_ext=%08x, flashId=%08x, tim1=%08x, tim2=%08x\n",
	       cs, nand_acc_control, nand_config, nand_config_ext, flash_id, nand_timing1, nand_timing2);
#else
	printk(KERN_INFO "\nFound NAND on CS%1d: ACC=%08x, cfg=%08x, flashId=%08x, tim1=%08x, tim2=%08x\n",
	       cs, nand_acc_control, nand_config, flash_id, nand_timing1, nand_timing2);
#endif
}

#define NUM_NAND_REGS   (1 + ((BCHP_NAND_LAST_REG - BCHP_NAND_REVISION) / 4))

static void __maybe_unused print_nand_ctrl_regs(void)
{
	int i;

/* Avoid garbled output */
	int saveDebug = gdebug;

	gdebug = 0;

	for (i = 0; i < NUM_NAND_REGS; i++) {
		uint32_t reg = (uint32_t)(BCHP_NAND_REVISION + (i * 4));
		uint32_t regval;
		//uint32_t regoff = reg - BCHP_NAND_REVISION; // i*4

		if ((i % 4) == 0) {
			printk("\n%08x:", reg);
		}

		if (inRegisterHoles(reg)) {
			regval = 0;
		}else  {
			regval = (uint32_t)brcmnand_ctrl_read(reg);
		}
		printk("  %08x", regval);
	}
	gdebug = saveDebug;
}

void print_NandCtrl_Status(void)
{
}

#if 1
void print_oobbuf(const unsigned char* buf, int len)
{
	int i;


	if (!buf) {
		printk("NULL"); return;
	}
	for (i = 0; i < len; i++) {
		if (i % 16 == 0 && i != 0) printk("\n");
		else if (i % 4 == 0) printk(" ");
		printk("%02x", buf[i]);
	}
	printk("\n");
}

void print_databuf(const unsigned char* buf, int len)
{
	int i;


	for (i = 0; i < len; i++) {
		if (i % 32 == 0) printk("\n%04x: ", i);
		else if (i % 4 == 0) printk(" ");
		printk("%02x", buf[i]);
	}
	printk("\n");
}

void print_oobreg(struct brcmnand_chip* chip)
{
	int i;

	printk("OOB Register:");
	for (i = 0; i < 4; i++) {
		printk("%08x ",  chip->ctrl_read(BCHP_NAND_SPARE_AREA_READ_OFS_0 + i * 4));
	}
	printk("\n");
}
#endif

/*
 * BRCMNAND controller always copies the data in 4 byte chunk, and in Big Endian mode
 * from and to the flash.
 * This routine assumes that dest and src are 4 byte aligned, and that len is a multiple of 4
   (Restriction removed)

 * TBD: 4/28/06: Remove restriction on count=512B, but do restrict the read from within a 512B section.
 * Change brcmnand_memcpy32 to be 2 functions, one to-flash, and one from-flash,
 * enforcing reading from/writing to flash on a 4B boundary, but relaxing on the buffer being on 4 byte boundary.
 */


static int brcmnand_from_flash_memcpy32(struct brcmnand_chip* chip, void* dest, loff_t offset, int len)
{
	volatile uint32_t* flash = (volatile uint32_t*)chip->vbase;
	volatile uint32_t* pucDest = (volatile uint32_t*)dest;
	volatile uint32_t* pucFlash = (volatile uint32_t*)flash;
	int i;

#if 0
	if (unlikely(((unsigned int)dest) & 0x3)) {
		printk(KERN_ERR "brcmnand_memcpy32 dest=%p not DW aligned\n", dest);
		return -EINVAL;
	}
#endif
	if (unlikely(((uintptr_t)flash) & 0x3)) {
		printk(KERN_ERR "brcmnand_memcpy32 src=%p not DW aligned\n", flash);
		return -EINVAL;
	}
	if (unlikely(len & 0x3)) {
		printk(KERN_ERR "brcmnand_memcpy32 len=%d not DW aligned\n", len);
		return -EINVAL;
	}

	/* THT: 12/04/08.  memcpy plays havoc with the NAND controller logic
	 * We have removed the alignment test, so we rely on the following codes to take care of it
	 */
	if (unlikely(((unsigned long)dest) & 0x3)) {
		for (i = 0; i < (len >> 2); i++) {
			// Flash is guaranteed to be DW aligned.  This forces the NAND controller
			// to read 1-DW at a time, w/o peep-hole optimization allowed.
			volatile uint32_t tmp = pucFlash[i];
			u8* pSrc = (u8*)&tmp;
			u8* pDest = (u8*)&pucDest[i];
			pDest[0] = pSrc[0];
			pDest[1] = pSrc[1];
			pDest[2] = pSrc[2];
			pDest[3] = pSrc[3];
		}
	}else  {
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0
		memcpy(dest, (void*)pucFlash, len);
#else
		for (i = 0; i < (len >> 2); i++) {
			pucDest[i] = pucFlash[i];
		}
#endif
	}

	return 0;
}


/*
 * Write to flash 512 bytes at a time.
 *
 * Can't just do a simple memcpy, since the HW NAND controller logic do the filtering
 * (i.e. ECC correction) on the fly 4 bytes at a time
 * This routine also takes care of alignment.
 */
static int brcmnand_to_flash_memcpy32(struct brcmnand_chip* chip, loff_t offset, const void* src, int len)
{
	u_char* flash = chip->vbase;
	int i;
	volatile uint32_t* pDest = (volatile uint32_t*)flash;
	volatile uint32_t* pSrc = (volatile uint32_t*)src;


	if (unlikely((uintptr_t)flash & 0x3)) {
		printk(KERN_ERR "brcmnand_to_flash_memcpy32 dest=%p not DW aligned\n", flash);
		BUG();
	}

	if (unlikely(len & 0x3)) {
		printk(KERN_ERR "brcmnand_to_flash_memcpy32 len=%d not DW aligned\n", len);
		BUG();
	}

	if (gdebug) printk("%s: flash=%p, len=%d, src=%p\n", __FUNCTION__, flash, len, src);


	/*
	 * THT: 12/08/08.  memcpy plays havoc with the NAND controller logic
	 * We have removed the alignment test, so we need these codes to take care of it
	 */
	if (unlikely((unsigned long)pSrc & 0x3)) {
		for (i = 0; i < (len >> 2); i++) {
			u8 *tmp = (u8*)&pSrc[i];
#if defined(CONFIG_CPU_LITTLE_ENDIAN)
			pDest[i] = ((uint32_t)(tmp[3] << 24) | (uint32_t)(tmp[2] << 16)
				    | (uint32_t)(tmp[1] << 8) | (uint32_t)(tmp[0] << 0));

#else
			pDest[i] = ((uint32_t)(tmp[0] << 24) | (uint32_t)(tmp[1] << 16)
				    | (uint32_t)(tmp[2] << 8) | (uint32_t)(tmp[3] << 0));
#endif
		}
	} else {
		for (i = 0; i < (len >> 2); i++) {
			pDest[i] = pSrc[i];
		}
	}

	return 0;
}

//#define uint8_t unsigned char



/* The BCHP_HIF_INTR2_xxx registers don't exist on DSL chips so the old way of
 * verifying ECC is used.
 */
#if defined(CONFIG_BCM_KF_MTD_BCMNAND) && CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
/*
 * SWLINUX-1584: Use HIF status register to check for errors.
 * In the past we rely on the fact that the registers
 *      BCHP_NAND_ECC_CORR_EXT_ADDR/BCHP_NAND_ECC_UNC_EXT_ADDR
 * are not zeroes, but the indicators are ambiguous when the address is 0
 *
 * Notes: 2618 still use the old way, because we are reluctant to change codes that
 * are already in production.  In 2618 this is only called when address==0
 */
#define HIF_INTR2_ERR_MASK ( \
		BCHP_HIF_INTR2_CPU_STATUS_NAND_CORR_INTR_MASK | \
		BCHP_HIF_INTR2_CPU_STATUS_NAND_UNC_INTR_MASK)

/*
 * Returns	 0: BRCMNAND_SUCCESS:	No errors
 *			 1: Correctable error
 *			-1: Uncorrectable error
 */
static int brcmnand_ctrl_verify_ecc(struct brcmnand_chip* chip, int state, uint32_t notUsed)
{
	uint32_t intr_status = BDEV_RD(BCHP_HIF_INTR2_CPU_STATUS);

	if (gdebug > 3 ) {
		printk("%s: intr_status = %08x\n", __FUNCTION__, intr_status);
	}

	/* Only make sense on read */
	if (state != BRCMNAND_FL_READING)
		return BRCMNAND_SUCCESS;

	if (intr_status & BCHP_HIF_INTR2_CPU_STATUS_NAND_UNC_INTR_MASK) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
		// Clear Status Mask for sector 0 workaround
		BDEV_WR(BCHP_HIF_INTR2_CPU_CLEAR,
			HIF_INTR2_ERR_MASK | BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_MASK);
#endif

#if 0           /* Already cleared with cpu-clear */
		intr_status &= ~HIF_INTR2_ERR_MASK;
		BDEV_WR(BCHP_HIF_INTR2_CPU_STATUS, intr_status);
#endif
		return BRCMNAND_UNCORRECTABLE_ECC_ERROR;
	}else if (intr_status & BCHP_HIF_INTR2_CPU_STATUS_NAND_CORR_INTR_MASK) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
		BDEV_WR(BCHP_HIF_INTR2_CPU_CLEAR,
			HIF_INTR2_ERR_MASK | BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_MASK);
#endif

#if 0           /* Already cleared with cpu-clear */
		intr_status &= ~HIF_INTR2_ERR_MASK;
		BDEV_WR(BCHP_HIF_INTR2_CPU_STATUS, intr_status);
#endif
		return BRCMNAND_CORRECTABLE_ECC_ERROR;
	}

	return BRCMNAND_SUCCESS;
}


#else
/* Old ways of doing it: is ambiguous when offset == 0 */

/*
 * Returns	 0: BRCMNAND_SUCCESS:	No errors
 *			 1: Correctable error
 *			-1: Uncorrectable error
 */
static int brcmnand_ctrl_verify_ecc(struct brcmnand_chip* chip, int state, uint32_t notUsed)
{
	int err = 0;
	uint32_t addr;
	uint32_t extAddr = 0;

	if (gdebug > 3 ) {
		printk("-->%s\n", __FUNCTION__);
	}

	/* Only make sense on read */
	if (state != BRCMNAND_FL_READING)
		return BRCMNAND_SUCCESS;

	addr = chip->ctrl_read(BCHP_NAND_ECC_CORR_ADDR);
	if (addr) {

		extAddr = chip->ctrl_read(BCHP_NAND_ECC_CORR_EXT_ADDR);
		// Clear it
		chip->ctrl_write(BCHP_NAND_ECC_CORR_EXT_ADDR, 0);

		// Clear it
		chip->ctrl_write(BCHP_NAND_ECC_CORR_ADDR, 0);
		printk(KERN_WARNING "%s: Correctable ECC error at %08x:%08x\n", __FUNCTION__, extAddr, addr);

		/* Check to see if error occurs in Data or ECC */
		err = BRCMNAND_CORRECTABLE_ECC_ERROR;
	}

	addr = chip->ctrl_read(BCHP_NAND_ECC_UNC_ADDR);
	if (addr) {
		extAddr = chip->ctrl_read(BCHP_NAND_ECC_UNC_EXT_ADDR);
		// Clear it
		chip->ctrl_write(BCHP_NAND_ECC_UNC_EXT_ADDR, 0);
		chip->ctrl_write(BCHP_NAND_ECC_UNC_ADDR, 0);

		/*
		 * If the block was just erased, and have not yet been written to, this will be flagged,
		 * so this could be a false alarm
		 */

		err = BRCMNAND_UNCORRECTABLE_ECC_ERROR;
	}
	return err;
}

#endif

#if 0
static int (*brcmnand_verify_ecc) (struct brcmnand_chip* chip, int state, uint32_t intr) = brcmnand_ctrl_verify_ecc;
#endif


/**
 * brcmnand_wait - [DEFAULT] wait until the command is done
 * @param mtd		MTD device structure
 * @param state		state to select the max. timeout value
 *
 * Wait for command done. This applies to all BrcmNAND command
 * Read can take up to 53, erase up to ?s and program up to 30 clk cycle ()
 * according to general BrcmNAND specs
 */
static int brcmnand_wait(struct mtd_info *mtd, int state, uint32_t* pStatus, int tout)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned long timeout;
	uint32_t ready;
	uint32_t wait_for = BRCMNAND_FL_WRITING == state
			    ? BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK | BCHP_NAND_INTFC_STATUS_FLASH_READY_MASK
			    : BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK;

	/* The 20 msec is enough */
	timeout = jiffies + msecs_to_jiffies(tout);
	while (time_before(jiffies, timeout)) {
		PLATFORM_IOFLUSH_WAR();
		ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);

		if ((ready & wait_for) == wait_for) {
			*pStatus = ready;
			return 0;
		}

		if (state != BRCMNAND_FL_READING && (!wr_preempt_en) && !in_interrupt())
			cond_resched();
		else
			udelay(1);
		//touch_softlockup_watchdog();
	}

	/*
	 * Get here on timeout
	 */
	return -ETIMEDOUT;
}



/*
 * Returns       1: Success, no errors
 *                       0: Timeout
 *			-1: Errors
 */
static int brcmnand_spare_is_valid(struct mtd_info* mtd,  int state, int raw, int tout)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned long timeout;
	uint32_t ready;

	if (gdebug > 3 ) {
		printk("-->%s, raw=%d\n", __FUNCTION__, raw);
	}


	/* The 20 msec is enough */
	timeout = jiffies + msecs_to_jiffies(tout);  // 3 sec timeout for now
	while (time_before(jiffies, timeout)) {
		PLATFORM_IOFLUSH_WAR();
		ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);

		if (ready & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK &&
		    (ready & BCHP_NAND_INTFC_STATUS_SPARE_AREA_VALID_MASK)) {


#if 0
// THT 6/15/09: Reading OOB would not affect ECC
			int ecc;

			if (!raw) {
				ecc = brcmnand_ctrl_verify_ecc(chip, state, 0);
				if (ecc < 0) {
//printk("%s: Uncorrectable ECC error at offset %08x\n", __FUNCTION__, (unsigned long) offset);
					return -1;
				}
			}
#endif
			return 1;
		}
		if (state != BRCMNAND_FL_READING && !wr_preempt_en && !in_interrupt())
			cond_resched();
		else
			udelay(1);
	}

	return 0; // Timed out
}



/*
 * Returns: Good: >= 0
 *		    Error:  < 0
 *
 * BRCMNAND_CORRECTABLE_ECC_ERROR		(1)
 * BRCMNAND_SUCCESS					(0)
 * BRCMNAND_UNCORRECTABLE_ECC_ERROR	(-1)
 * BRCMNAND_FLASH_STATUS_ERROR			(-2)
 * BRCMNAND_TIMED_OUT					(-3)
 *
 * Is_Valid in the sense that the data is valid in the cache.
 * It does not means that the data is either correct or correctable.
 */

static int brcmnand_cache_is_valid(struct mtd_info* mtd,  int state, loff_t offset, int tout)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned long timeout;
	uint32_t ready;

	if (gdebug > 3 ) {
		printk("%s: offset=%0llx\n", __FUNCTION__, offset);
	}

	/* The 20 msec is enough */
	timeout = jiffies + msecs_to_jiffies(tout); // 3 sec timeout for now
	while (time_before(jiffies, timeout)) {
		PLATFORM_IOFLUSH_WAR();
		ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);

		if ((ready & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK)
		    && (ready & BCHP_NAND_INTFC_STATUS_CACHE_VALID_MASK)) {
			int ecc;

			ecc = brcmnand_ctrl_verify_ecc(chip, state, 0);
// Let caller handle it
//printk("%s: Possible Uncorrectable ECC error at offset %08x\n", __FUNCTION__, (unsigned long) offset);
			if (gdebug > 3 && ecc) {
				printk("<--%s: ret = %d\n", __FUNCTION__, ecc);
			}
			return ecc;

		}
		if (state != BRCMNAND_FL_READING && (!wr_preempt_en) && !in_interrupt())
			cond_resched();
		else
			udelay(1);

	}

	if (gdebug > 3 ) {
		printk("<--%s: ret = TIMEOUT\n", __FUNCTION__);
		print_nand_ctrl_regs();
	}
	return BRCMNAND_TIMED_OUT; // TimeOut
}


#if 0
static int brcmnand_select_cache_is_valid(struct mtd_info* mtd,  int state, loff_t offset)
{
	int ret = 0;

	ret =   brcmnand_cache_is_valid(mtd, state, offset);
	return ret;
}
#endif


/*
 * Returns 1 on success,
 *		  0 on error
 */


static int brcmnand_ctrl_write_is_complete(struct mtd_info *mtd, int* outp_needBBT)
{
	int err;
	uint32_t status;
	uint32_t flashStatus = 0;

	*outp_needBBT = 1;
	err = brcmnand_wait(mtd, BRCMNAND_FL_WRITING, &status, 10000);
	if (!err) {
		if (status & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK) {
			flashStatus = status & 0x01;
			if (flashStatus) {
				printk(KERN_INFO "%s: INTF Status = %08x\n", __FUNCTION__, status);
			}
			*outp_needBBT = flashStatus; // 0 = write completes with no errors
			return 1;
		}else  {
			return 0;
		}
	}
	return 0;
}





static int (*brcmnand_write_is_complete) (struct mtd_info*, int*) = brcmnand_ctrl_write_is_complete;



/**
 * brcmnand_transfer_oob - [Internal] Transfer oob from chip->oob_poi to client buffer
 * @chip:	nand chip structure
 * @oob:	oob destination address
 * @ops:	oob ops structure
 * @len: OOB bytes to transfer
 *
 * Returns the pointer to the OOB where next byte should be read
 */
uint8_t *
brcmnand_transfer_oob(struct brcmnand_chip *chip, uint8_t *oob,
		      struct mtd_oob_ops *ops, int len)
{
	//size_t len = ops->ooblen;

	switch (ops->mode) {

	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		memcpy(oob, chip->oob_poi + ops->ooboffs, len);
		return oob + len;

	case MTD_OPS_AUTO_OOB: {
		struct nand_oobfree *free = chip->ecclayout->oobfree;
		uint32_t boffs = 0, roffs = ops->ooboffs;
		size_t bytes = 0;

		for (; free->length && len; free++, len -= bytes) {
			/* Read request not from offset 0 ? */
			if (unlikely(roffs)) {
				if (roffs >= free->length) {
					roffs -= free->length;
					continue;
				}
				boffs = free->offset + roffs;
				bytes = min_t(size_t, len,
					      (free->length - roffs));
				roffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
#ifdef DEBUG_ISR
			printk("%s: AUTO: oob=%p, chip->oob_poi=%p, ooboffs=%d, len=%d, bytes=%d, boffs=%d\n",
			       __FUNCTION__, oob, chip->oob_poi, ops->ooboffs, len, bytes, boffs);
#endif
			memcpy(oob, chip->oob_poi + boffs, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		BUG();
	}
	return NULL;
}




#undef DEBUG_UNCERR
#ifdef DEBUG_UNCERR
static uint32_t uncErrOob[7];
static u_char uncErrData[512];
#endif



void brcmnand_post_mortem_dump(struct mtd_info* mtd, loff_t offset)
{
	int i;

//Avoid garbled output
	int saveDebug = gdebug;

	gdebug = 0;

	printk("%s at offset %llx\n", __FUNCTION__, offset);
	dump_stack();

	printk("NAND registers snapshot \n");
	for (i = 0; i < NUM_NAND_REGS; i++) {
		uint32_t reg = BCHP_NAND_REVISION + (i * 4);
		uint32_t regval;

		if (inRegisterHoles(reg)) { // No NAND register at 0x281c
			regval = 0;
		}else  {
			regval = brcmnand_ctrl_read(reg);
		}
		if ((i % 4) == 0) {
			printk("\n%08x:", reg);
		}
		printk("  %08x", regval);
	}

	gdebug = saveDebug;

}



#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_4
/*
 * Read the OOB bytes beyond 16B
 *
 * i:		DW index into OOB area
 * p32:	DW pointer into OOB area
 */
static inline
void read_ext_spare_area(struct brcmnand_chip* chip, int i, uint32_t* p32)
{
	uint32_t dwoob;
	int j;
	int oobi;                       /* Byte index into OOB area */
	u_char* p8 = (u_char*)p32;      /* Byte pointer into OOB area */
	u_char* q = (u_char*)&dwoob;

	/* If HW support it, copy OOB bytes beyond 16 bytes */

	/* p8 and oobi index into byte-wise OOB, p32 index into DW-wise OOB */
	oobi = i * 4;

	for (; i < 8 && oobi < chip->eccOobSize; i++, oobi += 4) {


		/* This takes care of Endian-ness of the registers */
		dwoob = be32_to_cpu(chip->ctrl_read(BCHP_NAND_SPARE_AREA_READ_OFS_10 + (i - 4) * 4));
		if (gdebug > 3) {
			printk("%s: dwoob=%08x\n", __FUNCTION__, dwoob);
		}

		/* OOB size is an odd 27 bytes */
		if (oobi + 4 <= chip->eccOobSize) {
			p32[i] = dwoob;
		}else  { /* Trailing 3 bytes, column=pgSize+24,25,26*/
			// remember that p8 = (u_char*) &p32[0];
			for (j = 0; oobi + j < chip->eccOobSize; j++) {
				p8[oobi + j] = q[j];
			}
			break; /* Out of i loop */
		}
	}
}

#else
#define read_ext_spare_area(...)
#endif


#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
static int brcmnand_get_ecc_strength(struct brcmnand_chip *chip)
{
	uint32_t acc;
	int ecclevel, eccstrength;

	acc = chip->ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));
	ecclevel = (acc & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK) 
		>> BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;

	if (ecclevel == BRCMNAND_ECC_HAMMING)
		eccstrength = 1;
	else if (ecclevel == BRCMNAND_ECC_DISABLE)
		eccstrength = 0;
	else	
		eccstrength = ecclevel;

	return eccstrength;
}


/*
 * Check a page to see if it is erased (w/ bitflips) after an uncorrectable ECC
 * error
 *
 * Because the HW ECC signals an ECC error if an erase paged has even a single
 * bitflip, we must check each ECC error to see if it is actually an erased
 * page with bitflips, not a truly corrupted page.
 *
 * On a real error, return a negative error code (-EBADMSG for ECC error), and
 * buf will contain raw data.
 * Otherwise, fill buf with 0xff and return the maximum number of
 * bitflips-per-ECC-sector to the caller.
 *
 */

static int brcmnand_handle_false_read_ecc_unc_errors(struct mtd_info* mtd, 
		void* buffer, u_char* oobarea, loff_t offset)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*) mtd->priv;
	uint32_t acc0;
	int i, sas, oob_nbits, data_nbits;
	unsigned int max_bitflips = 0;
	int ret;
	struct nand_ecclayout *ecclayout = chip->ecclayout;
	int oobofs_limit = 0, eccpos_idx = 0, check_ecc = 1, ecc_pos;
	unsigned long ecc;
	u_char* oobs;

	acc0 = brcmnand_disable_read_ecc(chip->ctrl->CS[chip->csi]);
	if (buffer == NULL) {
		buffer = chip->ctrl->buffers->databuf;
		/* Invalidate page cache */
		chip->pagebuf = -1;
	}
	if (oobarea == NULL)
		oobarea = chip->oob_poi;
	  
	ret = brcmnand_read_page(mtd, buffer, oobarea, offset>>chip->page_shift);
	brcmnand_restore_ecc(chip->ctrl->CS[chip->csi], acc0);
	if (ret)
		return ret;

	oobs = oobarea;
	sas = mtd->oobsize / chip->eccsteps;
	oob_nbits = sizeof(ecc)<<3;
	data_nbits = chip->eccsize << 3;

	for (i = 0; i < chip->eccsteps; i++, oobarea += sas) {
		unsigned int bitflips = 0;

		oobofs_limit = (i + 1)*sas;

		/* only check for ECC bytes because JFFS2 may already write OOB */
		/* check number ecc bit flip within each ecc step size */
		while ( eccpos_idx < MTD_MAX_ECCPOS_ENTRIES_LARGE && check_ecc ) {
			ecc_pos = ecclayout->eccpos[eccpos_idx];
			if (ecc_pos == 0) {
				/* no more ecc bytes all done */
				check_ecc = 0;
				break;
			} else if (ecc_pos < oobofs_limit) {
				/* this ecc bytes belong to this subpage, count any bit flip */
				ecc = (unsigned long)oobs[ecc_pos];
				bitflips += 8 - bitmap_weight(&ecc, oob_nbits); /* only lowest 8 bit matters */
				eccpos_idx++;
			} else {
				/* done with this subpage */
				break;
			}
		}

		bitflips += data_nbits - bitmap_weight(buffer, data_nbits);

		buffer += chip->eccsize;
		offset += chip->eccsize;

		/* Too many bitflips */
		if (bitflips > brcmnand_get_ecc_strength(chip))
			return -EBADMSG;

		max_bitflips = max(max_bitflips, bitflips);
	}

	return max_bitflips;

}


/* This function handle the correctable and uncorrect ecc error for page read 
 *  and return the approriate buffer and return code to upper layer 
 */
static int 
brcmnand_handle_ecc_errors(struct mtd_info *mtd, uint8_t *buf, 
				uint8_t* oob, loff_t offset, int error)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*) mtd->priv;
	struct nand_ecclayout *ecclayout = chip->ecclayout;
	int eccpos, eccpos_idx = 0;
	int ret = 0;

	/* verify ECC error in whole page base */
	if (error == BRCMNAND_UNCORRECTABLE_ECC_ERROR) {
		ret = brcmnand_handle_false_read_ecc_unc_errors(mtd, buf, oob, offset);
		if (ret < 0) {
			printk("uncorrectable error at 0x%llx\n", (unsigned long long)offset);
			mtd->ecc_stats.failed++;
			/* NAND layer expects zero on ECC errors */
			ret = 0;
		} else {
			/* uncorrectable ecc error caused by erase page and number of bit flip within ecc
			 * strength. Fill buffer and oob with 0xff 
			 */
			if (buf)
				memset(buf, 0xff, chip->eccsize*chip->eccsteps);
			if (oob) {
				/* only restore 0xff on the ecc bytes only because JFFS2 may already 
				 * write cleanmarker in oob 
				 */
				while ((eccpos = ecclayout->eccpos[eccpos_idx])) {
			 		oob[eccpos] = 0xff;
					eccpos_idx++;
				}
			}

			if (ret) 
				printk("corrected %d bitflips in blank page at 0x%llx\n",
					ret, (unsigned long long)offset);
			mtd->ecc_stats.corrected += ret;
			ret = 0;
		}
	}

	if (error == BRCMNAND_CORRECTABLE_ECC_ERROR) {
		printk("corrected error at 0x%llx\n", (unsigned long long)offset);
		mtd->ecc_stats.corrected++;
		ret = 0;
	}

	return ret;
}

#else
/*
 * Returns 0 on success
 * Expect a controller read was done before hand, and that the OOB data are read into NAND registers.
 */
static int brcmnand_handle_false_read_ecc_unc_errors(
	struct mtd_info* mtd,
	void* buffer, u_char* oobarea, loff_t offset)
{
	struct brcmnand_chip* chip = mtd->priv;
	//int retries = 2;
	static uint32_t oobbuf[8]; // Sparea Area to handle ECC workaround, aligned on DW boundary
	uint32_t* p32 = (oobarea ?  (uint32_t*)oobarea :  (uint32_t*)&oobbuf[0]);
	u_char* p8 = (u_char*)p32;
	int ret = 0;

	/* Flash chip returns errors

	 || There is a bug in the controller, where if one reads from an erased block that has NOT been written to,
	 || this error is raised.
	 || (Writing to OOB area does not have any effect on this bug)
	 || The workaround is to also look into the OOB area, to see if they are all 0xFF

	 */
	//u_char oobbuf[16];
	int erased, allFF;
	int i;

	for (i = 0; i < 4; i++) {
		p32[i] = be32_to_cpu(chip->ctrl_read(BCHP_NAND_SPARE_AREA_READ_OFS_0 + i * 4));
	}

	read_ext_spare_area(chip, i, p32);

	if (chip->ecclevel == BRCMNAND_ECC_HAMMING) {
		/*
		 * THT 9/16/10: Also guard against the case where all data bytes are 0x11 or 0x22,
		 * in which case, this is a bonafide Uncorrectable error
		 *
		 * Look at first 4 bytes from the flash, already guaranteed to be 512B aligned
		 */
		uint32_t* pFirstDW = (uint32_t*)chip->vbase;

		erased = (p8[6] == 0xff && p8[7] == 0xff && p8[8] == 0xff);
		/* If first 4 bytes of data are not 0xFFFFFFFF, then this is a real UNC error */
		allFF = (p8[6] == 0x00 && p8[7] == 0x00 && p8[8] == 0x00 && *pFirstDW == 0xFFFFFFFF);

		if (gdebug > 3 ) {
			printk("%s: offset=%0llx, erased=%d, allFF=%d\n",
			       __FUNCTION__, offset, erased, allFF);
			print_oobbuf(p8, 16);
		}
	}else if (chip->ecclevel >= BRCMNAND_ECC_BCH_1 && chip->ecclevel <= BRCMNAND_ECC_BCH_12) {
		erased = 1;
		allFF = 0; // Not sure for BCH.
		// For BCH-n, the ECC bytes are at the end of the OOB area
		for (i = chip->eccOobSize - chip->eccbytes; i < min(16, chip->eccOobSize); i++) {
			erased = erased && (p8[i] == 0xff);
			if (!erased) {
				if (gdebug > 3 )
					printk("p8[%d]=%02x\n", i, p8[i]);
				break;
			}
		}
		if (gdebug > 3 ) {
			printk("%s: offset=%0llx, i=%d from %d to %d, eccOobSize=%d, eccbytes=%d, erased=%d, allFF=%d\n",
			       __FUNCTION__, offset, i, chip->eccOobSize - chip->eccbytes, chip->eccOobSize,
			       chip->eccOobSize, chip->eccbytes, erased, allFF);
		}
	}else  {
		printk("BUG: Unsupported ECC level %d\n", chip->ecclevel);
		BUG();
	}

	if ( erased || allFF) {
		/*
		 * For the first case, the slice is an erased block, and the ECC bytes are all 0xFF,
		 * for the 2nd, all bytes are 0xFF, so the Hamming Codes for it are all zeroes.
		 * The current version of the BrcmNAND controller treats these as un-correctable errors.
		 * For either case, fill data buffer with 0xff and return success.  The error has already
		 * been cleared inside brcmnand_verify_ecc.
		 * Both case will be handled correctly by the BrcmNand controller in later releases.
		 */
		p32 = (uint32_t*)buffer;
		for (i = 0; i < ECCSIZE(mtd) / 4; i++) {
			p32[i] = 0xFFFFFFFF;
		}
		ret = 0; // Success
	}else  {
		/* Real error: Disturb read returns uncorrectable errors */
		ret = BRCMNAND_UNCORRECTABLE_ECC_ERROR;
		if (gdebug > 3 ) {
			printk("<-- %s: indeed uncorrectable ecc error\n", __FUNCTION__);
		}

#ifdef DEBUG_UNCERR

		// Copy the data buffer
		brcmnand_from_flash_memcpy32(chip, uncErrData, offset, ECCSIZE(mtd));
		for (i = 0; i < 4; i++) {
			uncErrOob[i] = p32[i];
		}

		printk("%s: Uncorrectable error at offset %llx\n", __FUNCTION__, offset);

		printk("Data:\n");
		print_databuf(uncErrData, ECCSIZE(mtd));
		printk("Spare Area\n");
		print_oobbuf((unsigned char*)&uncErrOob[0], 16);

		brcmnand_post_mortem_dump(mtd, offset);

#endif
	}

	return ret;
}
#endif

// THT PR50928: if wr_preempt is disabled, enable it to clear error
int brcmnand_handle_ctrl_timeout(struct mtd_info* mtd, int retry)
{
	uint32_t acc;
	struct brcmnand_chip* __maybe_unused chip = mtd->priv;

	// First check to see if WR_PREEMPT is disabled
	acc = brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));
	if (retry <= 2 && 0 == (acc & BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK)) {
		acc |= BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK;
		brcmnand_ctrl_write(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]), acc);
		printk("Turn on WR_PREEMPT_EN\n");
		return 1;
	}

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	brcmnand_restore_ecc(chip->ctrl->CS[chip->csi], brcmnand_disable_read_ecc(chip->ctrl->CS[chip->csi]));
#endif

	return 0;
}

void  brcmnand_Hamming_ecc(const uint8_t *data, uint8_t *ecc_code)
{

	int i, j;
	static uint8_t o_ecc[24], temp[10];
	static uint32_t b_din[128];
	uint32_t* i_din = &b_din[0];
	unsigned long pre_ecc;

#if 0
	// THT Use this block if there is a need for endian swapping
	uint32_t i_din[128];
	uint32_t* p32 = (uint32_t*)data; //  alignment guaranteed by caller.


	for (i = 0; i < 128; i++) {
		//i_din[i/4] = (long)(data[i+3]<<24 | data[i+2]<<16 | data[i+1]<<8 | data[i]);
		i_din[i] = /*le32_to_cpu */ (p32[i]);
		//printk( "i_din[%d] = 0x%08.8x\n", i/4, i_din[i/4] );
	}

#else
	if (unlikely((uintptr_t)data & 0x3)) {
		memcpy((uint8_t*)i_din, data, 512);
	}else  {
		i_din = (uint32_t*)data;
	}
#endif

	memset(o_ecc, 0, sizeof(o_ecc));

	for (i = 0; i < 128; i++) {
		memset(temp, 0, sizeof(temp));

		for (j = 0; j < 32; j++) {
			temp[0] ^= ((i_din[i] & 0x55555555) >> j) & 0x1;
			temp[1] ^= ((i_din[i] & 0xAAAAAAAA) >> j) & 0x1;
			temp[2] ^= ((i_din[i] & 0x33333333) >> j) & 0x1;
			temp[3] ^= ((i_din[i] & 0xCCCCCCCC) >> j) & 0x1;
			temp[4] ^= ((i_din[i] & 0x0F0F0F0F) >> j) & 0x1;
			temp[5] ^= ((i_din[i] & 0xF0F0F0F0) >> j) & 0x1;
			temp[6] ^= ((i_din[i] & 0x00FF00FF) >> j) & 0x1;
			temp[7] ^= ((i_din[i] & 0xFF00FF00) >> j) & 0x1;
			temp[8] ^= ((i_din[i] & 0x0000FFFF) >> j) & 0x1;
			temp[9] ^= ((i_din[i] & 0xFFFF0000) >> j) & 0x1;
		}

		for (j = 0; j < 10; j++)
			o_ecc[j] ^= temp[j];

		//o_ecc[0]^=temp[0];//P1'
		//o_ecc[1]^=temp[1];//P1
		//o_ecc[2]^=temp[2];//P2'
		//o_ecc[3]^=temp[3];//P2
		//o_ecc[4]^=temp[4];//P4'
		//o_ecc[5]^=temp[5];//P4
		//o_ecc[6]^=temp[6];//P8'
		//o_ecc[7]^=temp[7];//P8
		//o_ecc[8]^=temp[8];//P16'
		//o_ecc[9]^=temp[9];//P16

		if (i % 2) {
			for (j = 0; j < 32; j++)
				o_ecc[11] ^= (i_din[i] >> j) & 0x1; //P32
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[10] ^= (i_din[i] >> j) & 0x1; //P32'
		}

		if ((i & 0x3) < 2) {
			for (j = 0; j < 32; j++)
				o_ecc[12] ^= (i_din[i] >> j) & 0x1; //P64'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[13] ^= (i_din[i] >> j) & 0x1; //P64
		}

		if ((i & 0x7) < 4) {
			for (j = 0; j < 32; j++)
				o_ecc[14] ^= (i_din[i] >> j) & 0x1; //P128'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[15] ^= (i_din[i] >> j) & 0x1; //P128
		}

		if ((i & 0xF) < 8) {
			for (j = 0; j < 32; j++)
				o_ecc[16] ^= (i_din[i] >> j) & 0x1; //P256'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[17] ^= (i_din[i] >> j) & 0x1; //P256
		}

		if ((i & 0x1F) < 16) {
			for (j = 0; j < 32; j++)
				o_ecc[18] ^= (i_din[i] >> j) & 0x1; //P512'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[19] ^= (i_din[i] >> j) & 0x1; //P512
		}

		if ((i & 0x3F) < 32) {
			for (j = 0; j < 32; j++)
				o_ecc[20] ^= (i_din[i] >> j) & 0x1; //P1024'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[21] ^= (i_din[i] >> j) & 0x1; //P1024
		}

		if ((i & 0x7F) < 64) {
			for (j = 0; j < 32; j++)
				o_ecc[22] ^= (i_din[i] >> j) & 0x1; //P2048'
		}else  {
			for (j = 0; j < 32; j++)
				o_ecc[23] ^= (i_din[i] >> j) & 0x1; //P2048
		}
		// print intermediate value
		pre_ecc = 0;
		for (j = 23; j >= 0; j--) {
			pre_ecc = (pre_ecc << 1) | (o_ecc[j] ? 1 : 0 );
		}
//        printf( "pre_ecc[%d] = 0x%06.6x\n", i, pre_ecc );
	}
	//xprintf("P16':%x P16:%x P8':%x P8:%x\n",o_ecc[8],o_ecc[9],o_ecc[6],o_ecc[7]);
	//xprintf("P1':%x P1:%x P2':%x P2:%x\n",o_ecc[0],o_ecc[1],o_ecc[2],o_ecc[3]);
	// ecc_code[0] = ~(o_ecc[13]<<7 | o_ecc[12]<<6 | o_ecc[11]<<5 | o_ecc[10]<<4 | o_ecc[9]<<3 | o_ecc[8]<<2 | o_ecc[7]<<1 | o_ecc[6]);
	// ecc_code[1] = ~(o_ecc[21]<<7 | o_ecc[20]<<6 | o_ecc[19]<<5 | o_ecc[18]<<4 | o_ecc[17]<<3 | o_ecc[16]<<2 | o_ecc[15]<<1 | o_ecc[14]);
	// ecc_code[2] = ~(o_ecc[5]<<7 | o_ecc[4]<<6 | o_ecc[3]<<5 | o_ecc[2]<<4 | o_ecc[1]<<3 | o_ecc[0]<<2 | o_ecc[23]<<1 | o_ecc[22]);

	ecc_code[0] = (o_ecc[ 7] << 7 | o_ecc[ 6] << 6 | o_ecc[ 5] << 5 | o_ecc[ 4] << 4 | o_ecc[ 3] << 3 | o_ecc[ 2] << 2 | o_ecc[ 1] << 1 | o_ecc[ 0]);
	ecc_code[1] = (o_ecc[15] << 7 | o_ecc[14] << 6 | o_ecc[13] << 5 | o_ecc[12] << 4 | o_ecc[11] << 3 | o_ecc[10] << 2 | o_ecc[ 9] << 1 | o_ecc[ 8]);
	ecc_code[2] = (o_ecc[23] << 7 | o_ecc[22] << 6 | o_ecc[21] << 5 | o_ecc[20] << 4 | o_ecc[19] << 3 | o_ecc[18] << 2 | o_ecc[17] << 1 | o_ecc[16]);
	// printf("BROADCOM          ECC:0x%02X 0x%02X 0x%02X \n",ecc_code[0],ecc_code[1],ecc_code[2]);
	//xprintf("BROADCOM          ECC:0x%02X 0x%02X 0x%02X \n",test[0],test[1],test[2]);
}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_1_1
/* No workaround needed, fixed in HW */
#define brcmnand_Hamming_WAR(...) (0)

#else

/*
 * Workaround for Hamming ECC when correctable error is in the ECC bytes.
 * Returns 0 if error was in data (no action needed), 1 if error was in ECC (use uncorrected data instead)
 */
static int brcmnand_Hamming_WAR(struct mtd_info* mtd, loff_t offset, void* buffer,
				u_char* inp_hwECC, u_char* inoutp_swECC)
{
	struct brcmnand_chip* chip = mtd->priv;
	static uint32_t ucdata[128];
	u_char* uncorr_data = (u_char*)ucdata;
	uint32_t acc0;
	int valid;
	//unsigned long irqflags;

	int ret = 0, retries = 2;

	/* Disable ECC */
	acc0 = brcmnand_disable_read_ecc(chip->ctrl->CS[chip->csi]);

	while (retries >= 0) {
		if (wr_preempt_en) {
			//local_irq_save(irqflags);
		}

/* This register doesn't exist on DSL chips. */
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
		// Mask Interrupt
		BDEV_WR(BCHP_HIF_INTR2_CPU_MASK_SET, HIF_INTR2_ERR_MASK);
		// Clear Status Mask for sector 0 workaround
		BDEV_WR(BCHP_HIF_INTR2_CPU_CLEAR,
			HIF_INTR2_ERR_MASK | BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_MASK);
#endif

#if 0
		/* Already cleared with cpu-clear */
		intr_status = BDEV_RD(BCHP_HIF_INTR2_CPU_STATUS);
		intr_status &= ~(HIF_INTR2_ERR_MASK);
		BDEV_WR(BCHP_HIF_INTR2_CPU_STATUS, intr_status);
#endif

		chip->ctrl_writeAddr(chip, offset, 0);
		PLATFORM_IOFLUSH_WAR();
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PAGE_READ);

		// Wait until cache is filled up
		valid = brcmnand_cache_is_valid(mtd, BRCMNAND_FL_READING, offset, 100);

		if (wr_preempt_en) {
			//local_irq_restore(irqflags);
		}

		if (valid ==  BRCMNAND_TIMED_OUT) {
			//Read has timed out
			ret = -ETIMEDOUT;
			retries--;
			// THT PR50928: if wr_preempt is disabled, enable it to clear error
			wr_preempt_en = brcmnand_handle_ctrl_timeout(mtd, retries);
			continue;  /* Retry */
		}else  {
			ret = 0;
			break;
		}
	}

	if (retries < 0) {
		goto restore_ecc;
	}

	// Reread the uncorrected buffer.
	brcmnand_from_flash_memcpy32(chip, uncorr_data, offset, ECCSIZE(mtd));

	// Calculate Hamming Codes
	brcmnand_Hamming_ecc(uncorr_data, inoutp_swECC);

	// Compare ecc0 against ECC from HW
	if ((inoutp_swECC[0] == inp_hwECC[0] && inoutp_swECC[1] == inp_hwECC[1] &&
	     inoutp_swECC[2] == inp_hwECC[2])
	    || (inoutp_swECC[0] == 0x0 && inoutp_swECC[1] == 0x0 && inoutp_swECC[2] == 0x0 &&
		inp_hwECC[0] == 0xff && inp_hwECC[1] == 0xff && inp_hwECC[2] == 0xff)) {
		// Error was in data bytes, correction made by HW is good,
		// or block was erased and no data written to it yet,
		// send corrected data.
		// printk("CORR error was handled properly by HW\n");
		ret = 0;
	}else  { // Error was in ECC, send uncorrected data
		memcpy(buffer, uncorr_data, 512);

		ret = 1;
		printk("CORR error was handled by SW at offset %0llx, HW=%02x%02x%02x, SW=%02x%02x%02x\n",
		       offset, inp_hwECC[0], inp_hwECC[1], inp_hwECC[2],
		       inoutp_swECC[0], inoutp_swECC[1], inoutp_swECC[2]);
	}

 restore_ecc:
	// Restore acc
	brcmnand_restore_ecc(chip->ctrl->CS[chip->csi], acc0);
	return ret;
}
#endif



/**
 * brcmnand_posted_read_cache - [BrcmNAND Interface] Read the 512B cache area
 * Assuming brcmnand_get_device() has been called to obtain exclusive lock
 * @param mtd		MTD data structure
 * @param oobarea	Spare area, pass NULL if not interested
 * @param buffer	the databuffer to put/get data, pass NULL if only spare area is wanted.
 * @param offset	offset to read from or write to, must be 512B aligned.
 *
 * Caller is responsible to pass a buffer that is
 * (1) large enough for 512B for data and optionally an oobarea large enough for 16B.
 * (2) 4-byte aligned.
 *
 * Read the cache area into buffer.  The size of the cache is mtd-->eccsize and is always 512B.
 */

//****************************************
int in_verify;
//****************************************

static int brcmnand_ctrl_posted_read_cache(struct mtd_info* mtd,
					   void* buffer, u_char* oobarea, loff_t offset)
{
	struct brcmnand_chip* chip = mtd->priv;
	loff_t sliceOffset = offset & (~(ECCSIZE(mtd) - 1));
	int i, ret = 0;
	static uint32_t oob0[4]; // Sparea Area to handle ECC workaround, aligned on DW boundary
	uint32_t* p32 = (oobarea ?  (uint32_t*)oobarea :  (uint32_t*)&oob0[0]);
	u_char* __maybe_unused p8 = (u_char*)p32;

	//unsigned long irqflags;
	int retries = 5, done = 0;
	int valid = 0;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
	uint32_t intr_status;
#endif

	if (gdebug > 3 ) {
		printk("%s: offset=%0llx, oobarea=%p\n", __FUNCTION__, offset, oobarea);
	}


	if (unlikely(offset - sliceOffset)) {
		printk(KERN_ERR "%s: offset %08x is not cache aligned, sliceOffset=%08lx, CacheSize=%d\n",
		       __FUNCTION__, (unsigned int)offset, (unsigned long)sliceOffset, ECCSIZE(mtd));
		return -EINVAL;
	}

	while (retries > 0 && !done) {
/* This register doesn't exist on DSL chips. */
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
		uint32_t intr_status;

		if (wr_preempt_en) {
			//local_irq_save(irqflags);
		}

		// Mask Interrupt
		BDEV_WR(BCHP_HIF_INTR2_CPU_MASK_SET, HIF_INTR2_ERR_MASK);
		// Clear Status Mask for sector 0 workaround
		BDEV_WR(BCHP_HIF_INTR2_CPU_CLEAR,
			HIF_INTR2_ERR_MASK | BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_MASK);
		if (gdebug > 3) {
			intr_status = BDEV_RD(BCHP_HIF_INTR2_CPU_STATUS);
			printk("%s: before intr_status=%08x\n", __FUNCTION__, intr_status);
		}
#endif

#if defined(CONFIG_BCM_KF_MTD_BCMNAND) && CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
		intr_status = BDEV_RD(BCHP_HIF_INTR2_CPU_STATUS);
		intr_status &= ~(HIF_INTR2_ERR_MASK);
		BDEV_WR(BCHP_HIF_INTR2_CPU_STATUS, intr_status);
#endif

		chip->ctrl_writeAddr(chip, sliceOffset, 0);
		PLATFORM_IOFLUSH_WAR();
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PAGE_READ);

		// Wait until cache is filled up
		valid = brcmnand_cache_is_valid(mtd, BRCMNAND_FL_READING, offset, 100);

		if (wr_preempt_en) {
			//local_irq_restore(irqflags);
		}

		switch (valid) {

		case BRCMNAND_SUCCESS: /* Success, no errors */
			// Remember last good 512B-sector read.  Needed for HIF_INTR2 war.
			//if (0 == gLastKnownGoodEcc)
			gLastKnownGoodEcc = offset;

		/* FALLTHROUGH */

		case BRCMNAND_CORRECTABLE_ECC_ERROR:
			if (buffer) {
				brcmnand_from_flash_memcpy32(chip, buffer, offset, ECCSIZE(mtd));
			}

#ifndef DEBUG_HW_ECC
			if (oobarea || (ret == BRCMNAND_CORRECTABLE_ECC_ERROR))
#endif
			{
				PLATFORM_IOFLUSH_WAR();
				for (i = 0; i < 4; i++) {
					p32[i] =  be32_to_cpu(chip->ctrl_read(BCHP_NAND_SPARE_AREA_READ_OFS_0 + i * 4));
				}

				read_ext_spare_area(chip, i, p32);

				if (gdebug > 3) {
					printk("%s: offset=%0llx, oob=\n", __FUNCTION__, sliceOffset); print_oobbuf(oobarea, chip->eccOobSize);
				}
			}

 #ifndef DEBUG_HW_ECC   // Comment out for debugging

			/* Make sure error was not in ECC bytes */
			if (ret == BRCMNAND_CORRECTABLE_ECC_ERROR &&
			    chip->ecclevel == BRCMNAND_ECC_HAMMING)
 #endif

			{

				char ecc0[3]; // SW ECC, manually calculated

				if (brcmnand_Hamming_WAR(mtd, offset, buffer, &p8[6], &ecc0[0])) {
					/* Error was in ECC, update it from calculated value */
					if (oobarea) {
						oobarea[6] = ecc0[0];
						oobarea[7] = ecc0[1];
						oobarea[8] = ecc0[2];
					}
				}

			}


			// SWLINUX-1495:
			if (valid == BRCMNAND_CORRECTABLE_ECC_ERROR)
				ret = BRCMNAND_CORRECTABLE_ECC_ERROR;
			else
				ret = 0;

			done = 1;
			break;

		case BRCMNAND_UNCORRECTABLE_ECC_ERROR:
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
			ret = brcmnand_handle_false_read_ecc_unc_errors(mtd, buffer, oobarea, offset);
#else
			ret = BRCMNAND_UNCORRECTABLE_ECC_ERROR;
#endif
			done = 1;
			break;

		case BRCMNAND_FLASH_STATUS_ERROR:
			printk(KERN_ERR "brcmnand_cache_is_valid returns 0\n");
			ret = -EBADMSG;
			done = 1;
			break;

		case BRCMNAND_TIMED_OUT:
			//Read has timed out
			ret = -ETIMEDOUT;
			if (!wr_preempt_en) {
				retries--;
				// THT PR50928: if wr_preempt is disabled, enable it to clear error
				wr_preempt_en = brcmnand_handle_ctrl_timeout(mtd, retries);
				continue;  /* Retry */
			}else  {
				done = 1;
				break;
			}

		default:
			BUG_ON(1);
			/* Should never gets here */
			ret = -EFAULT;
			done = 1;
		}

	}

	if (wr_preempt_en) {
		uint32_t acc;

		acc = brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));

		acc &= ~BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK;
		brcmnand_ctrl_write(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]), acc);
	}


	if (gdebug > 3 ) {
		printk("<-- %s: offset=%0llx\n", __FUNCTION__, offset);
		print_databuf(buffer, 32);
	}

#if defined( EDU_DEBUG ) || defined (BRCMNAND_READ_VERIFY )
//if (in_verify <=0)
	if (chip->ecclevel == BRCMNAND_ECC_HAMMING) {
		u_char edu_sw_ecc[4];

		brcmnand_Hamming_ecc(buffer, edu_sw_ecc);

		if ((p8[6] != edu_sw_ecc[0] || p8[7] != edu_sw_ecc[1] || p8[8] != edu_sw_ecc[2])
		    && !(p8[6] == 0xff && p8[7] == 0xff && p8[8] == 0xff &&
			 edu_sw_ecc[0] == 0x0 && edu_sw_ecc[1] == 0x0 && edu_sw_ecc[2] == 0x0)
		    ) {
			printk("!!!!!!!!! %s: offset=%0llx ECC=%02x%02x%02x, OOB:",
			       in_verify < 0 ? "WR" : "RD",
			       offset, edu_sw_ecc[0], edu_sw_ecc[1], edu_sw_ecc[2]);
			print_oobbuf(p8, 16);
			BUG();
		}
	}
#endif

//gdebug=0;

	return ret;
}


/*
 * Clear the controller cache by reading at a location we don't normally read
 */
static void __maybe_unused debug_clear_ctrl_cache(struct mtd_info* mtd)
{
	/* clear the internal cache by writing a new address */
	struct brcmnand_chip* chip = mtd->priv;
	loff_t offset = chip->chipSize - chip->blockSize; // Start of BBT region

	//uint32_t intr_status;

/* This register doesn't exist on DSL chips. */
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
	// Mask Interrupt
	BDEV_WR(BCHP_HIF_INTR2_CPU_MASK_SET, HIF_INTR2_ERR_MASK);
	// Clear Status Mask for sector 0 workaround
	BDEV_WR(BCHP_HIF_INTR2_CPU_CLEAR,
		HIF_INTR2_ERR_MASK | BCHP_HIF_INTR2_CPU_STATUS_NAND_CTLRDY_INTR_MASK);
#endif

#if 0
	/* Already cleared with cpu-clear */
	intr_status = BDEV_RD(BCHP_HIF_INTR2_CPU_STATUS);
	intr_status &= ~(HIF_INTR2_ERR_MASK);
	BDEV_WR(BCHP_HIF_INTR2_CPU_STATUS, intr_status);
#endif

	chip->ctrl_writeAddr(chip, offset, 0);
	PLATFORM_IOFLUSH_WAR();
	chip->ctrl_write(BCHP_NAND_CMD_START, OP_PAGE_READ);

	// Wait until cache is filled up
	(void)brcmnand_cache_is_valid(mtd, BRCMNAND_FL_READING, offset, 100);
}

static int (*brcmnand_posted_read_cache)(struct mtd_info*,
					 void*, u_char*, loff_t) = brcmnand_ctrl_posted_read_cache;

/**
 * brcmnand_posted_read_oob - [BrcmNAND Interface] Read the spare area
 * @param mtd		MTD data structure
 * @param oobarea	Spare area, pass NULL if not interested
 * @param offset	offset to read from or write to
 *
 * This is a little bit faster than brcmnand_posted_read, making this command useful for improving
 * the performance of BBT management.
 * The 512B flash cache is invalidated.
 *
 * Read the cache area into buffer.  The size of the cache is mtd->writesize and is always 512B,
 * for this version of the BrcmNAND controller.
 */
static int brcmnand_posted_read_oob(struct mtd_info* mtd,
				    u_char* oobarea, loff_t offset, int raw)
{
	struct brcmnand_chip* chip = mtd->priv;
	loff_t sliceOffset = offset & (~(ECCSIZE(mtd) - 1));
	int i, ret = 0, valid, done = 0;
	int retries = 5;

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
	uint32_t acc1 = brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));
	//unsigned long irqflags;

//char msg[20];

	static uint8_t myBuf2[512 + 31]; // Place holder only.
	static uint8_t* myBuf = NULL;

	/*
	 * Force alignment on 32B boundary
	 */
	if (!myBuf) {
		myBuf = (uint8_t*)((((uintptr_t)&myBuf2[0]) + 31) & (~31));
	}

  #if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_3_0
	// Revert to cache read if acc is enabled
	if (acc1 & BCHP_NAND_ACC_CONTROL_RD_ECC_EN_MASK) {
		// PR2516.  Not a very good WAR, but the affected chips (3548A0,7443A0) have been EOL'ed
		return brcmnand_ctrl_posted_read_cache(mtd, (void*)myBuf, oobarea, offset);
	}

  #else /* 3.1 or later */
	// If BCH codes, force full page read to activate ECC correction on OOB bytes.
	// relies on the fact that brcmnand_disable_read_ecc() turns off both bllk0 and blkn bits
	if ((acc1 & BCHP_NAND_ACC_CONTROL_RD_ECC_EN_MASK) &&
	    chip->ecclevel != BRCMNAND_ECC_HAMMING &&
	    chip->ecclevel != BRCMNAND_ECC_DISABLE) {
		return brcmnand_ctrl_posted_read_cache(mtd, (void*)myBuf, oobarea, offset);
	}
  #endif
#endif

	if (gdebug > 3 ) PRINTK("->%s: offset=%0llx\n", __FUNCTION__, offset);
	if (gdebug > 3 ) PRINTK("->%s: sliceOffset=%0llx\n", __FUNCTION__, sliceOffset);
	if (gdebug > 3 ) PRINTK("eccsize = %d\n", ECCSIZE(mtd));

	if (gdebug > 3 ) {
		printk("-->%s: offset=%0llx\n", __FUNCTION__,  offset);
	}

	while (retries > 0 && !done) {
		if (unlikely(sliceOffset - offset)) {
			printk(KERN_ERR "%s: offset %0llx is not cache aligned\n",
			       __FUNCTION__, offset);
			return -EINVAL;
		}

		if (wr_preempt_en) {
			//local_irq_save(irqflags);
		}

		chip->ctrl_writeAddr(chip, sliceOffset, 0);
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_SPARE_AREA_READ);

		// Wait until spare area is filled up

		valid = brcmnand_spare_is_valid(mtd, BRCMNAND_FL_READING, raw, 100);
		if (wr_preempt_en) {
			//local_irq_restore(irqflags);
		}
		switch (valid) {
		case 1:
			if (oobarea) {
				uint32_t* p32 = (uint32_t*)oobarea;

				for (i = 0; i < 4; i++) {
					p32[i] = be32_to_cpu(chip->ctrl_read(BCHP_NAND_SPARE_AREA_READ_OFS_0 + (i << 2)));
				}

				read_ext_spare_area(chip, i, p32);

				if (gdebug > 3) {
					printk("%s: offset=%0llx, oob=\n", __FUNCTION__, sliceOffset);
					print_oobbuf(oobarea, chip->eccOobSize);
				}

			}

			ret = 0;
			done = 1;
			break;

		case -1:
			ret = -EBADMSG;
//if (gdebug > 3 )
			{ PRINTK("%s: ret = -EBADMSG\n", __FUNCTION__); }
			/* brcmnand_spare_is_valid also clears the error bit, so just retry it */

			retries--;
			break;

		case 0:
			//Read has timed out
			ret = -ETIMEDOUT;
			{ PRINTK("%s: ret = -ETIMEDOUT\n", __FUNCTION__); }
			retries--;
			// THT PR50928: if wr_preempt is disabled, enable it to clear error
			wr_preempt_en = brcmnand_handle_ctrl_timeout(mtd, retries);
			continue;  /* Retry */

		default:
			BUG_ON(1);
			/* NOTREACHED */
			ret = -EINVAL;
			done = 1;
			break; /* Should never gets here */
		}

	}
	if (wr_preempt_en) {
		uint32_t acc;

		acc = brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));

		acc &= ~BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK;
		brcmnand_ctrl_write(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]), acc);
	}

//if (gdebug > 3 )
	if (0) { // == (offset & (mtd->erasesize-1)))
		PRINTK("<--%s: offset=%08x\n", __FUNCTION__, (uint32_t)offset);
		print_oobbuf(oobarea, 16);
	}
	return ret;
}




/**
 * brcmnand_posted_write - [BrcmNAND Interface] Write a buffer to the flash cache
 * Assuming brcmnand_get_device() has been called to obtain exclusive lock
 *
 * @param mtd		MTD data structure
 * @param buffer		the databuffer to put/get data
 * @param oobarea	Spare area, pass NULL if not interested
 * @param offset	offset to write to, and must be 512B aligned
 *
 * Write to the cache area TBD 4/26/06
 */
static int brcmnand_ctrl_posted_write_cache(struct mtd_info *mtd,
					    const void* buffer, const u_char* oobarea, loff_t offset)
{
	struct brcmnand_chip* chip = mtd->priv;
	loff_t sliceOffset = offset & (~(ECCSIZE(mtd) - 1));
	uint32_t* p32;
	int i, needBBT = 0;
	int ret;

	//char msg[20];


	if (gdebug > 3 ) {
		printk("--> %s: offset=%0llx\n", __FUNCTION__, offset);
		print_databuf(buffer, 32);
	}

	if (unlikely(sliceOffset - offset)) {
		printk(KERN_ERR "%s: offset %0llx is not cache aligned\n",
		       __FUNCTION__, offset);

		ret =  -EINVAL;
		goto out;
	}
	chip->ctrl_writeAddr(chip, sliceOffset, 0);


	if (buffer) {
		if (gdebug > 3 ) {
			print_databuf(buffer, 32);
		}
		brcmnand_to_flash_memcpy32(chip, offset, buffer, ECCSIZE(mtd));
	}
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	/* Must write data when NAND_COMPLEX_OOB_WRITE */
	else if (chip->options & NAND_COMPLEX_OOB_WRITE) {
		brcmnand_to_flash_memcpy32(chip, offset, ffchars, ECCSIZE(mtd));
	}
#endif


//printk("30\n");
	if (oobarea) {
		p32 = (uint32_t*)oobarea;
		if (gdebug > 3) {
			printk("%s: oob=\n", __FUNCTION__); print_oobbuf(oobarea, 16);
		}
	}else  {
		// Fill with 0xFF if don't want to change OOB
		p32 = (uint32_t*)&ffchars[0];
	}

//printk("40\n");
	for (i = 0; i < 4; i++) {
		chip->ctrl_write(BCHP_NAND_SPARE_AREA_WRITE_OFS_0 + i * 4, cpu_to_be32(p32[i]));
	}

	PLATFORM_IOFLUSH_WAR();
	chip->ctrl_write(BCHP_NAND_CMD_START, OP_PROGRAM_PAGE);
//printk("50\n");

	// Wait until flash is ready
	if (brcmnand_ctrl_write_is_complete(mtd, &needBBT)) {
		if (!needBBT) {
			ret = 0;
			goto out;
		}else  { // Need BBT
			printk(KERN_WARNING "%s: Flash Status Error @%0llx\n", __FUNCTION__,  offset);
//printk("80 block mark bad\n");
			// SWLINUX-1495: Let UBI do it on returning -EIO
			ret = -EIO;
			chip->block_markbad(mtd, offset);
			goto out;
		}
	}
	//Write has timed out or read found bad block. TBD: Find out which is which
	printk(KERN_INFO "%s: Timeout\n", __FUNCTION__);
	ret = -ETIMEDOUT;

 out:
//printk("99\n");

	return ret;
}



static int (*brcmnand_posted_write_cache)(struct mtd_info*,
					  const void*, const u_char*, loff_t) = brcmnand_ctrl_posted_write_cache;




/**
 * brcmnand_posted_write_oob - [BrcmNAND Interface] Write the spare area
 * @param mtd		MTD data structure
 * @param oobarea	Spare area, pass NULL if not interested.  Must be able to
 *					hold mtd->oobsize (16) bytes.
 * @param offset	offset to write to, and must be 512B aligned
 *
 */
static int brcmnand_posted_write_oob(struct mtd_info *mtd,
				     const u_char* oobarea, loff_t offset, int isFromMarkBadBlock)
{
	struct brcmnand_chip* chip = mtd->priv;
	loff_t sliceOffset = offset & (~(ECCSIZE(mtd) - 1));
	uint32_t* p32;
	int i, needBBT = 0;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	uint32_t partial_page_wr_dis;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0
	uint32_t acc;

	acc = chip->ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));
	partial_page_wr_dis = !(acc & BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_MASK);
#else
	partial_page_wr_dis = 0;
#endif
#endif

	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx\n", __FUNCTION__,  offset);
		print_oobbuf(oobarea, 16);
	}


	if (unlikely(sliceOffset - offset)) {
		printk(KERN_ERR "%s: offset %0llx is not cache aligned\n",
		       __FUNCTION__, offset);
	}

	chip->ctrl_writeAddr(chip, sliceOffset, 0);

	// assert oobarea here
	BUG_ON(!oobarea);
	p32 = (uint32_t*)oobarea;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	/* Must write data when NAND_COMPLEX_OOB_WRITE option is set.  Wite 0xFFs
	 * to data and ECC locations.
	 */
	if ((chip->options & NAND_COMPLEX_OOB_WRITE) || partial_page_wr_dis) {
		u_char* p8 = (u_char*)p32;
		struct nand_ecclayout *oobinfo = chip->ecclayout;

		brcmnand_to_flash_memcpy32(chip, offset, ffchars, ECCSIZE(mtd));
		for (i = 0; i < oobinfo->eccbytes; i++) {
			p8[oobinfo->eccpos[i]] = 0xff;
		}
	}
#endif

	for (i = 0; i < 4; i++) {
		chip->ctrl_write(BCHP_NAND_SPARE_AREA_WRITE_OFS_0 + i * 4,  cpu_to_be32(p32[i]));
	}

	PLATFORM_IOFLUSH_WAR();
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if ((chip->options & NAND_COMPLEX_OOB_WRITE) || partial_page_wr_dis) {

		/* Disable ECC so 0xFFs are stored in the ECC offsets. Doing
		 * this allows the next page write to store the ECC correctly.
		 * If the ECC is not disabled here, then a ECC value will be
		 * stored at the ECC offsets.  This will cause the ECC value
		 * on the next write to be stored incorrectly.
		 */
		uint32_t acc = chip->ctrl_read(BCHP_NAND_ACC_CONTROL);
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
		chip->ctrl_write(BCHP_NAND_ACC_CONTROL,
				 (acc & ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK |
					  BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK)));
#else
		chip->ctrl_write(BCHP_NAND_ACC_CONTROL,
				 (acc & ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK)));
#endif
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PROGRAM_PAGE);

		// Wait until flash is ready
		if (brcmnand_ctrl_write_is_complete(mtd, &needBBT)) {
			chip->ctrl_write(BCHP_NAND_ACC_CONTROL, acc);
			return 0;
		}

		chip->ctrl_write(BCHP_NAND_ACC_CONTROL, acc);
	}else  {
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PROGRAM_SPARE_AREA);

		// Wait until flash is ready
		if (brcmnand_ctrl_write_is_complete(mtd, &needBBT)) {
			return 0;
		}
	}
#else
#if 0
	if (chip->options & NAND_COMPLEX_OOB_WRITE) {
//printk("****** Workaround, using OP_PROGRAM_PAGE instead of OP_PROGRAM_SPARE_AREA\n");
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PROGRAM_PAGE);
	}else
#endif
	{
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PROGRAM_SPARE_AREA);
	}

	// Wait until flash is ready
	if (brcmnand_ctrl_write_is_complete(mtd, &needBBT)) {
		return 0;
	}
#endif  /* CONFIG_BCM_KF_MTD_BCMNAND */


	if (needBBT) {

		int ret;

		printk(KERN_WARNING "%s: Flash Status Error @%0llx\n", __FUNCTION__,  offset);

		// SWLINUX-1495: Let UBI do it on returning -EIO
		ret = -EIO;

		if (!isFromMarkBadBlock)
			chip->block_markbad(mtd, offset);

		return (ret);
	}

	return -ETIMEDOUT;

}



/**
 * brcmnand_get_device - [GENERIC] Get chip for selected access
 * @param mtd		MTD device structure
 * @param new_state	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
static int brcmnand_get_device(struct mtd_info *mtd, int new_state)
{
	struct brcmnand_chip * chip = mtd->priv;

	if (chip) {
		DECLARE_WAITQUEUE(wait, current);

		/*
		 * Grab the lock and see if the device is available
		 */
		while (1) {
			spin_lock(&chip->ctrl->chip_lock);

			if (chip->ctrl->state == BRCMNAND_FL_READY) {
				chip->ctrl->state = new_state;
				spin_unlock(&chip->ctrl->chip_lock);
				break;
			}
			if (new_state == BRCMNAND_FL_PM_SUSPENDED) {
				spin_unlock(&chip->ctrl->chip_lock);
				return (chip->ctrl->state == BRCMNAND_FL_PM_SUSPENDED) ? 0 : -EAGAIN;
			}
			set_current_state(TASK_UNINTERRUPTIBLE);
			add_wait_queue(&chip->ctrl->wq, &wait);
			spin_unlock(&chip->ctrl->chip_lock);
			if (!wr_preempt_en && !in_interrupt())
				schedule();
			remove_wait_queue(&chip->ctrl->wq, &wait);
		}

		return 0;
	}else
		return -EINVAL;
}

#if 0
/* No longer used */
static struct brcmnand_chip*
brcmnand_get_device_exclusive(void)
{
	struct brcmnand_chip * chip = (struct brcmnand_chip*)get_brcmnand_handle();
	struct mtd_info *mtd;
	int ret;

	mtd = (struct mtd_info*)chip->priv;

	if (mtd) {
		ret = brcmnand_get_device(mtd, BRCMNAND_FL_XIP);
	}else
		ret = -1;
	if (0 == ret)
		return chip;
	else
		return ((struct brcmnand_chip *)0);
}


#endif

/**
 * brcmnand_release_device - [GENERIC] release chip
 * @param mtd		MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void brcmnand_release_device(struct mtd_info *mtd)
{
	struct brcmnand_chip * chip = mtd->priv;

	/* Release the chip */
	spin_lock(&chip->ctrl->chip_lock);
	chip->ctrl->state = BRCMNAND_FL_READY;
	wake_up(&chip->ctrl->wq);
	spin_unlock(&chip->ctrl->chip_lock);
}



/**
 * brcmnand_read_page - {REPLACEABLE] hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure.  The OOB buf is stored here on return
 * @buf:	buffer to store read data
 *
 * Not for syndrome calculating ecc controllers which need a special oob layout
 */
static int
brcmnand_read_page(struct mtd_info *mtd,
		   uint8_t *outp_buf, uint8_t* outp_oob, uint64_t page)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int eccstep;
	int dataRead = 0;
	int oobRead = 0;
	int ret = 0, error = 0;
	uint64_t offset = ((uint64_t)page) << chip->page_shift;
	int corrected = 0;      // Only update stats once per page
	int uncorrected = 0;    // Only update stats once per page

	if (gdebug > 3 ) {
		printk("-->%s, page=%0llx\n", __FUNCTION__, page);
	}

	chip->pagebuf = page;

	for (eccstep = 0; eccstep < chip->eccsteps && ret == 0; eccstep++) {
		ret = brcmnand_posted_read_cache(mtd, &outp_buf[dataRead],
						 outp_oob ? &outp_oob[oobRead] : NULL,
						 offset + dataRead);
		if (gdebug > 3 && ret) printk("%s 1: calling brcmnand_posted_read_cache returns %d\n",
					      __FUNCTION__, ret);
		if (ret == BRCMNAND_CORRECTABLE_ECC_ERROR) {
			if ( !corrected) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
				(mtd->ecc_stats.corrected)++;
#else
				if (error != BRCMNAND_UNCORRECTABLE_ECC_ERROR)
					error = ret;
#endif
				corrected = 1;
			}
			ret = 0;
		}else if (ret == BRCMNAND_UNCORRECTABLE_ECC_ERROR) {
			if ( !uncorrected) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
				(mtd->ecc_stats.failed)++;
#else
				error = ret;
#endif
				uncorrected = 1;
			}
			ret = 0;
		}else if (ret < 0) {
			printk(KERN_ERR "%s: 3: brcmnand_posted_read_cache failed at offset=%0llx, ret=%d\n",
			       __FUNCTION__, offset + dataRead, ret);
			return ret;
		}

		dataRead += chip->eccsize;
		oobRead += chip->eccOobSize;
	}

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (error)
		ret = brcmnand_handle_ecc_errors(mtd, outp_buf, outp_oob, offset, error);
#endif
	return ret;
}



/**
 * brcmnand_read_page_oob - {REPLACABLE] hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure.  The OOB buf is stored in the oob_poi ptr on return
 *
 * Not for syndrome calculating ecc controllers which need a special oob layout
 */
static int
brcmnand_read_page_oob(struct mtd_info *mtd,
		       uint8_t* outp_oob, uint64_t page)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int eccstep;
	int dataRead = 0;
	int oobRead = 0;
	int corrected = 0;      // Only update stats once per page
	int uncorrected = 0;    // Only update stats once per page
	int ret = 0, error = 0;
	uint64_t offset = page << chip->page_shift;


	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx\n", __FUNCTION__, offset);
	}

	chip->pagebuf = page;

	for (eccstep = 0; eccstep < chip->eccsteps && ret == 0; eccstep++) {
//gdebug=4;
		ret = brcmnand_posted_read_oob(mtd, &outp_oob[oobRead],
					       offset + dataRead, 1);
//gdebug=0;
		if (gdebug > 3 && ret) printk("%s 2: calling brcmnand_posted_read_oob returns %d\n",
					      __FUNCTION__, ret);
		if (ret == BRCMNAND_CORRECTABLE_ECC_ERROR) {
			if ( !corrected) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
				(mtd->ecc_stats.corrected)++;
#else
				if (error != BRCMNAND_UNCORRECTABLE_ECC_ERROR)
					error = ret;
#endif
				corrected = 1;
			}
			ret = 0;
		}else if (ret == BRCMNAND_UNCORRECTABLE_ECC_ERROR) {
			if ( !uncorrected) {
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
				(mtd->ecc_stats.failed)++;
#else
				error = ret;
#endif
				uncorrected = 1;
			}
			ret = 0;
		}else if (ret < 0) {
			printk(KERN_ERR "%s: 3: posted read oob failed at offset=%0llx, ret=%d\n",
			       __FUNCTION__, offset + dataRead, ret);
			return ret;
		}
		dataRead += chip->eccsize;
		oobRead += chip->eccOobSize;
	}

	if (gdebug > 3 && ret) printk("%s returns %d\n",
				      __FUNCTION__, ret);

	if (gdebug > 3 ) {
		printk("<--%s offset=%0llx, ret=%d\n", __FUNCTION__, offset, ret);
		print_oobbuf(outp_oob, mtd->oobsize);
	}

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (error)
		ret = brcmnand_handle_ecc_errors(mtd, NULL, outp_oob, offset, error);
#endif
	return ret;
}

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
static int brcmnand_refresh_blk(struct mtd_info *mtd, loff_t from)
{
	struct brcmnand_chip *chip = mtd->priv;
	int i, j, k, numpages, ret, count = 0, nonecccount = 0;
	uint8_t *blk_buf;       /* Store one block of data (including OOB) */
	unsigned int pg_idx, oob_idx;
	uint64_t realpage;
	struct erase_info *instr;
	//int gdebug = 1;
	struct nand_ecclayout *oobinfo;
	uint8_t *oobptr;
	uint32_t *oobptr32;
	loff_t blkbegin;
	unsigned int block_size;


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	chip->ctrl_write(BCHP_NAND_ECC_CORR_EXT_ADDR, 0);
#endif
	chip->ctrl_write(BCHP_NAND_ECC_CORR_ADDR, 0);

	DEBUG(MTD_DEBUG_LEVEL3, "Inside %s: from=%0llx\n", __FUNCTION__, from);
	printk(KERN_INFO "%s: Performing block refresh for correctable ECC error at %0llx\n",
	       __FUNCTION__, from);
	pg_idx = 0;
	oob_idx = mtd->writesize;
	numpages = mtd->erasesize / mtd->writesize;
	block_size = (1 << chip->erase_shift);
	blkbegin = (from & (~(mtd->erasesize - 1)));
	realpage = blkbegin >> chip->page_shift;

	blk_buf = (uint8_t*)BRCMNAND_malloc(numpages * (mtd->writesize + mtd->oobsize));

	if (unlikely(blk_buf == NULL)) {
		printk(KERN_ERR "%s: buffer allocation failed\n", __FUNCTION__);
		return -1;
	}

	memset(blk_buf, 0xff, numpages * (mtd->writesize + mtd->oobsize));

	if (unlikely(gdebug > 0)) {
		printk("---> %s: from = %0llx, numpages = %d, realpage = %0llx\n", \
		       __FUNCTION__,  from, numpages, realpage);
		printk("     Locking flash for read ... \n");
	}

	/* Read an entire block */
	brcmnand_get_device(mtd, BRCMNAND_FL_READING);
	for (i = 0; i < numpages; i++) {
		ret = chip->read_page(mtd, blk_buf + pg_idx, blk_buf + oob_idx, realpage);
		if (ret < 0) {
			BRCMNAND_free(blk_buf);
// #else re-use for EDU
			brcmnand_release_device(mtd);
			return -1;
		}
		//printk("DEBUG -> Reading %d realpage = %x %x ret = %d oob = %x\n", i, realpage, *(blk_buf+pg_idx), ret, *(blk_buf + oob_idx));
		//print_oobbuf(blk_buf+oob_idx, mtd->oobsize);
		pg_idx += mtd->writesize + mtd->oobsize;
		oob_idx += mtd->oobsize + mtd->writesize;
		realpage++;
	}
	if (unlikely(gdebug > 0)) {
		printk("---> %s:  Read -> erase\n", __FUNCTION__);
	}
	chip->ctrl->state = BRCMNAND_FL_ERASING;

	/* Erase the block */
	instr = kmalloc(sizeof(struct erase_info), GFP_KERNEL);
	if (instr == NULL) {
		printk(KERN_WARNING "kmalloc for erase_info failed\n");
		BRCMNAND_free(blk_buf);
// #else re-use for EDU
		brcmnand_release_device(mtd);
		return -ENOMEM;
	}
	memset(instr, 0, sizeof(struct erase_info));
	instr->mtd = mtd;
	instr->addr = blkbegin;
	instr->len = mtd->erasesize;
	if (unlikely(gdebug > 0)) {
		printk("DEBUG -> erasing %0llx, %0llx %d\n", instr->addr, instr->len, chip->ctrl->state);
	}
	ret = brcmnand_erase_nolock(mtd, instr, 0);
	if (ret) {
		BRCMNAND_free(blk_buf);
// #else re-use for EDU
		kfree(instr);
		brcmnand_release_device(mtd);
		printk(KERN_WARNING " %s Erase failed %d\n", __FUNCTION__, ret);
		return ret;
	}
	kfree(instr);

	/* Write the entire block */
	pg_idx = 0;
	oob_idx = mtd->writesize;
	realpage = blkbegin >> chip->page_shift;
	if (unlikely(gdebug > 0)) {
		printk("---> %s: Erase -> write ... %d\n", __FUNCTION__, chip->ctrl->state);
	}
	oobinfo = chip->ecclayout;
	chip->ctrl->state = BRCMNAND_FL_WRITING;
	for (i = 0; i < numpages; i++) {
		/* Avoid writing empty pages */
		count = 0;
		nonecccount = 0;
		oobptr = (uint8_t*)(blk_buf + oob_idx);
		oobptr32 = (uint32_t*)(blk_buf + oob_idx);
		for (j = 0; j < oobinfo->eccbytes; j++) {
			if (oobptr[oobinfo->eccpos[j]] == 0xff) {
				count++;
			}
		}
		for (k = 0; k < mtd->oobsize / 4; k++) {
			if (oobptr32[k] == 0xffffffff) {
				nonecccount++;
			}
		}
		/* Skip this page if ECC is 0xff */
		if (count == j && nonecccount == k) {
			pg_idx += mtd->writesize + mtd->oobsize;
			oob_idx += mtd->oobsize + mtd->writesize;
			realpage++;
			continue;
		}
		/* Skip this page, but write the OOB */
		if (count == j && nonecccount != k) {
			ret = chip->write_page_oob(mtd, blk_buf + oob_idx, realpage, 0);
			if (ret) {
				BRCMNAND_free(blk_buf);
// #else re-use for EDU
				brcmnand_release_device(mtd);
				return ret;
			}
			pg_idx += mtd->writesize + mtd->oobsize;
			oob_idx += mtd->oobsize + mtd->writesize;
			realpage++;
			continue;
		}
		for (j = 0; j < oobinfo->eccbytes; j++) {
			oobptr[oobinfo->eccpos[j]] = 0xff;
		}
		ret = chip->write_page(mtd, blk_buf + pg_idx, blk_buf + oob_idx, realpage);
		if (ret) {
			BRCMNAND_free(blk_buf);
// #else re-use for EDU
			brcmnand_release_device(mtd);
			return ret;
		}
		pg_idx += mtd->writesize + mtd->oobsize;
		oob_idx += mtd->oobsize + mtd->writesize;
		realpage++;
	}
	brcmnand_release_device(mtd);
	BRCMNAND_free(blk_buf);
// #else re-use for EDU
	printk(KERN_INFO "%s: block refresh success\n", __FUNCTION__);

	return 0;
}
#endif



/**
 * brcmnand_do_read_ops - [Internal] Read data with ECC
 *
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:		oob ops structure
 * @raw:		read raw data format when TRUE
 *
 * Internal function. Called with chip held.
 */

//#define EDU_DEBUG_1
#undef EDU_DEBUG_1

static int brcmnand_do_read_ops(struct mtd_info *mtd, loff_t from,
				struct mtd_oob_ops *ops)
{
	unsigned int bytes, col;
	uint64_t realpage;
	int aligned;
	struct brcmnand_chip *chip = mtd->priv;
	struct mtd_ecc_stats stats;
	//int blkcheck = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;
	//int sndcmd = 1;
	int ret = 0;
	uint32_t readlen = ops->len;
	uint8_t *bufpoi, *oob, *buf;
	int __maybe_unused numPages;
	int __maybe_unused buffer_aligned = 0;
	int ooblen;


	if (ops->mode == MTD_OPS_AUTO_OOB) {
		ooblen = mtd->ecclayout->oobavail;
	}else  {
		ooblen = mtd->oobsize;
	}
//int nonBatch = 0;

	/* Remember the current CORR error count */
	stats = mtd->ecc_stats;

	// THT: BrcmNAND controller treats multiple chip as one logical chip.
	//chipnr = (int)(from >> chip->chip_shift);
	//chip->select_chip(mtd, chipnr);

	realpage = (uint64_t)from >> chip->page_shift;
	//page = realpage & chip->pagemask;

	col = mtd64_ll_low(from & (mtd->writesize - 1));

/* Debugging 12/27/08 */
	chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);

	buf = ops->datbuf;
	oob = ops->oobbuf;

#ifdef CONFIG_MTD_BRCMNAND_ISR_QUEUE
	/*
	 * Group several pages for submission for small page NAND
	 */
	if (chip->pageSize == chip->eccsize && ops->mode != MTD_OPS_RAW) {
		while (1) {
//nonBatch = 0;
			bytes = min(mtd->writesize - col, readlen);
			// (1) Writing partial or full page
			aligned = (bytes == mtd->writesize);

			// If writing full page, use user buffer, otherwise, internal buffer
			bufpoi = aligned ? buf : chip->ctrl->buffers->databuf;

			// (2) Buffer satisfies 32B alignment required by EDU?
			buffer_aligned = EDU_buffer_OK(bufpoi, EDU_READ);

			// (3) Batch mode if writing more than 1 pages.
			numPages = min(MAX_JOB_QUEUE_SIZE, (int)readlen >> chip->page_shift);

			// Only do Batch mode if all 3 conditions are satisfied.
			if (!aligned || !buffer_aligned || numPages <= 1) {
				/* Submit 1 page at a time */

				numPages = 1; // We count partial page read
				ret = chip->read_page(mtd, bufpoi, chip->oob_poi, realpage);

				if (ret < 0)
					break;

				/* Transfer not aligned data */
				if (!aligned) {
					chip->pagebuf = realpage;
					memcpy(buf, &bufpoi[col], bytes);
				}
				buf += bytes;

				if (unlikely(oob)) {
					/* if (ops->mode != MTD_OPS_RAW) */
					oob = brcmnand_transfer_oob(chip, oob, ops, ooblen);

				}

			}else  {
				/*
				 * Batch job possible, all 3 conditions are met
				 * bufpoi = Data buffer from FS driver
				 * oob = OOB buffer from FS driver
				 */
				bytes = numPages * mtd->writesize;

				ret = brcmnand_isr_read_pages(mtd, bufpoi, oob ? &oob : NULL, realpage, numPages, ops);

				if (ret < 0)
					break;

				buf += bytes; /* Advance Read pointer */

			}


			readlen -= bytes;

			if (!readlen)
				break;

			/* For subsequent reads align to page boundary. */
			col = 0;
			/* Increment page address */
			realpage += numPages;
		}
		goto out;
	}else
#endif
	{
		while (1) {
			bytes = min(mtd->writesize - col, readlen);
			aligned = (bytes == mtd->writesize);

			bufpoi = aligned ? buf : chip->ctrl->buffers->databuf;

			ret = chip->read_page(mtd, bufpoi, oob ? chip->oob_poi : NULL, realpage);

			if (ret < 0)
				break;

			/* Transfer not aligned data */
			if (!aligned) {
				chip->pagebuf = realpage;
				memcpy(buf, &bufpoi[col], bytes);
			}

			buf += bytes;

			if (unlikely(oob)) {
				/* Raw mode does data:oob:data:oob */
				if (ops->mode != MTD_OPS_RAW)
					oob = brcmnand_transfer_oob(chip, oob, ops, ooblen);
				else {
					buf = brcmnand_transfer_oob(chip, buf, ops, ooblen);
				}
			}


			readlen -= bytes;

			if (!readlen)
				break;

			/* For subsequent reads align to page boundary. */
			col = 0;
			/* Increment page address */
			realpage++;

		}
	}

 out: __maybe_unused
//gdebug=0;

	ops->retlen = ops->len - (size_t)readlen;


	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	return mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}



/**
 * brcmnand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int brcmnand_read(struct mtd_info *mtd, loff_t from, size_t len,
			 size_t *retlen, uint8_t *buf)
{
	struct brcmnand_chip *chip = mtd->priv;
	int ret;

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
	int status;
#endif

	DEBUG(MTD_DEBUG_LEVEL3, "%s: from=%0llx\n", __FUNCTION__, from);

	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx, len=%08x\n", __FUNCTION__, from, (unsigned int)len);
	}

	/* Do not allow reads past end of device */

	if (unlikely((from + len) > device_size(mtd)))
		return -EINVAL;

	if (!len)
		return 0;

	brcmnand_get_device(mtd, BRCMNAND_FL_READING);

	chip->ops.mode = MTD_OPS_AUTO_OOB;
	chip->ops.len = len;
	chip->ops.datbuf = buf;
	chip->ops.oobbuf = NULL;

	brcmnand_reset_corr_threshold(chip);

	ret = brcmnand_do_read_ops(mtd, from, &chip->ops);

	*retlen = chip->ops.retlen;

	brcmnand_release_device(mtd);

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
	/* use atomic_inc_return instead two seperate atomic_read and atomic_inc call because
	   there is race condition between these two calls if it is preempted after first call but
	   right before the second atomic call */
	if (unlikely(ret == -EUCLEAN)) {
		if (atomic_inc_return(&inrefresh) == 1) {
			if (brcmnand_refresh_blk(mtd, from) == 0) {
				ret = 0;
			}
			if (likely(chip->cet)) {
				if (likely(chip->cet->flags != BRCMNAND_CET_DISABLED)) {
					if (brcmnand_cet_update(mtd, from, &status) == 0) {

/*
 * PR57272: Provide workaround for BCH-n ECC HW bug when # error bits >= 4
 * We will not mark a block bad when the a correctable error already happened on the same page
 */
#if CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_3_4
						ret = 0;
#else
						if (status) {
							ret = -EUCLEAN;
						} else {
							ret = 0;
						}
#endif
					}
					if (gdebug > 3) {
						printk(KERN_INFO "DEBUG -> %s ret = %d, status = %d\n", __FUNCTION__, ret, status);
					}
				}
			}
		}
		atomic_dec(&inrefresh);
	}
#endif
	return ret;
}



/**
 * brcmnand_do_read_oob - [Intern] BRCMNAND read out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operations description structure
 *
 * BRCMNAND read out-of-band data from the spare area
 */
static int brcmnand_do_read_oob(struct mtd_info *mtd, loff_t from,
				struct mtd_oob_ops *ops)
{
	int realpage = 1;
	struct brcmnand_chip *chip = mtd->priv;
	//int blkcheck = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;
	int toBeReadlen = ops->ooblen;
	int readlen = 0;
	int len; /* Number of OOB bytes to read each page */
	uint8_t *buf = ops->oobbuf;
	int ret = 0;

	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx, buf=%p, ooblen=%d\n", __FUNCTION__, from, buf, toBeReadlen);
	}

	DEBUG(MTD_DEBUG_LEVEL3, "%s: from = 0x%08Lx, len = %i\n",
	      __FUNCTION__, (unsigned long long)from, toBeReadlen);

	//chipnr = (int)(from >> chip->chip_shift);
	//chip->select_chip(mtd, chipnr);

	if (ops->mode == MTD_OPS_AUTO_OOB)
		len = chip->ecclayout->oobavail;
	else
		len = mtd->oobsize;

	if (unlikely(ops->ooboffs >= len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "nand_read_oob: "
		      "Attempt to start read outside oob\n");
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(from >= mtd->size ||
		     ops->ooboffs + readlen > ((mtd->size >> chip->page_shift) -
					       (from >> chip->page_shift)) * len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "nand_read_oob: "
		      "Attempt read beyond end of device\n");
		return -EINVAL;
	}


	/* Shift to get page */
	realpage = (int)(from >> chip->page_shift);
	//page = realpage & chip->pagemask;

	chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
	brcmnand_reset_corr_threshold(chip);

	while (toBeReadlen > 0) {
		ret = chip->read_page_oob(mtd, chip->oob_poi, realpage);
		if (ret) { // Abnormal return
			ops->oobretlen = readlen;
			return ret;
		}

		buf = brcmnand_transfer_oob(chip, buf, ops, len);

		toBeReadlen -= len;
		readlen += len;

		/* Increment page address */
		realpage++;

	}

	ops->oobretlen = ops->ooblen;
	return ret;
}


/**
 * brcmnand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int brcmnand_read_oob(struct mtd_info *mtd, loff_t from,
			     struct mtd_oob_ops *ops)
{
//	struct brcmnand_chip *chip = mtd->priv;
	int ret = -ENOTSUPP;

	//int raw;

	if (gdebug > 3 ) {
		printk("-->%s, offset=%lx len=%x, databuf=%p\n", __FUNCTION__,
		       (unsigned long)from, (unsigned)ops->len, ops->datbuf);
	}

	DEBUG(MTD_DEBUG_LEVEL3, "%s: from=%0llx\n", __FUNCTION__, from);

	ops->retlen = 0;

	/* Do not allow reads past end of device */

	brcmnand_get_device(mtd, BRCMNAND_FL_READING);

#if 0
	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
		raw = 0;
		break;

	case MTD_OPS_RAW:
		raw = 1;
		break;

	default:
		goto out;
	}
#endif

	if (!ops->datbuf) {
		ret = brcmnand_do_read_oob(mtd, from, ops);
	} else {
		if (unlikely((from + ops->len) > device_size(mtd))) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt read beyond end of device\n", __FUNCTION__);
			ret = -EINVAL;
		} else {
			ret = brcmnand_do_read_ops(mtd, from, ops);
		}
	}


// out:
	brcmnand_release_device(mtd);
	if (gdebug > 3 ) {
		printk("<-- %s: ret=%d\n", __FUNCTION__, ret);
	}
	return ret;
}





#ifdef CONFIG_MTD_BRCMNAND_VERIFY_WRITE

#if 0
/*
 * Returns 0 on success,
 */
static int brcmnand_verify_pageoob_priv(struct mtd_info *mtd, loff_t offset,
					const u_char* fsbuf, int fslen, u_char* oob_buf, int ooblen, struct nand_oobinfo* oobsel,
					int autoplace, int raw)
{
	//struct brcmnand_chip * chip = mtd->priv;
	int ret = 0;
	int complen;


	if (autoplace) {

		complen = min_t(int, ooblen, fslen);

		/* We may have read more from the OOB area, so just compare the min of the 2 */
		if (complen == fslen) {
			ret = memcmp(fsbuf, oob_buf, complen);
			if (ret) {
				{
					printk("Autoplace Comparison failed at %08x, ooblen=%d fslen=%d left=\n",
					       __ll_low(offset), ooblen, fslen);
					print_oobbuf(fsbuf, fslen);
					printk("\nRight=\n"); print_oobbuf(oob_buf, ooblen);
					dump_stack();
				}
				goto comparison_failed;
			}
		}else  {
			printk("%s: OOB comparison failed, ooblen=%d is less than fslen=%d\n",
			       __FUNCTION__, ooblen, fslen);
			return -EBADMSG;
		}
	}else  { // No autoplace.  Skip over non-freebytes

		/*
		 * THT:
		 * WIth YAFFS1, the FS codes overwrite an already written chunks quite a lot
		 * (without erasing it first, that is!!!!!)
		 * For those write accesses, it does not make sense to check the write ops
		 * because they are going to fail every time
		 */


#if 0
		int i, len;

		for (i = 0; oobsel->oobfree[i][1] && i < ARRAY_SIZE(oobsel->oobfree); i++) {
			int from = oobsel->oobfree[i][0];
			int num = oobsel->oobfree[i][1];
			int len1 = num;

			if (num == 0) break; // End of oobsel

			if ((from + num) > fslen) len1 = fslen - from;
			ret = memcmp(&fsbuf[from], &oob_buf[from], len1);
			if (ret) {
				printk(KERN_ERR "%s: comparison at offset=%08x, i=%d from=%d failed., num=%d\n",
				       __FUNCTION__, i, __ll_low(offset), from, num);
				if (gdebug > 3) {
					printk("No autoplace Comparison failed at %08x, ooblen=%d fslen=%d left=\n",
					       __ll_low(offset), ooblen, fslen);
					print_oobbuf(&fsbuf[0], fslen);
					printk("\nRight=\n"); print_oobbuf(&oob_buf[0], ooblen);
					dump_stack();
				}
				goto comparison_failed;
			}
			if ((from + num) >= fslen) break;
			len += num;
		}
#endif
	}
	return ret;


 comparison_failed:
	{
		//unsigned long nand_timing1 = brcmnand_ctrl_read(BCHP_NAND_TIMING_1);
		//unsigned long nand_timing2 = brcmnand_ctrl_read(BCHP_NAND_TIMING_2);
		//u_char raw_oob[NAND_MAX_OOBSIZE];
		//int retlen;
		//struct nand_oobinfo noauto_oobsel;

		printk("Comparison Failed\n");
		print_diagnostics(chip);

		//noauto_oobsel = *oobsel;
		//noauto_oobsel.useecc = MTD_NANDECC_PLACEONLY;
		//brcmnand_read_pageoob(mtd, offset, raw_oob, &retlen, &noauto_oobsel, 0, raw);
//if (gdebug) { printk("oob="); print_oobbuf(raw_oob, retlen);}
//printk("<-- %s: comparison failed\n", __FUNCTION__);


		return -EBADMSG;
	}
}
#endif


/**
 * brcmnand_verify_page - [GENERIC] verify the chip contents after a write
 * @param mtd		MTD device structure
 * @param dbuf		the databuffer to verify
 * @param dlen		the length of the data buffer, and should beequal to mtd->writesize
 * @param oobbuf		the length of the file system OOB data and should be exactly
 *                             chip->oobavail (for autoplace) or mtd->oobsize otherise
 *					bytes to verify. (ignored for Hamming)
 * @param ooblen
 *
 * Returns 0 on success, 1 on errors.
 * Assumes that lock on.  Munges the internal data and OOB buffers.
 */
//#define MYDEBUG
#if 0
static u_char verify_buf[NAND_MAX_PAGESIZE + 512];
static u_char v_oob_buf [NAND_MAX_OOBSIZE];
static int brcmnand_verify_page(struct mtd_info *mtd, loff_t addr,
				const u_char *dbuf, int dlen,
				const u_char* inp_oob, int ooblen
				)
{
	struct brcmnand_chip * chip = mtd->priv;

	int ret = 0; // Matched
	//int ooblen=0, datalen=0;
	//int complen;
	u_char* oobbuf = v_oob_buf;
	uint64_t page;
	int eccstep;
	// Align Vbuf on 512B
	u_char* vbuf = (u_char*)( ((unsigned long)verify_buf + chip->eccsize - 1)
				  & ~( chip->eccsize - 1));

	if (gdebug > 3) printk("-->%s: addr=%0llx\n", __FUNCTION__, addr);

	/*
	 * Only do it for Hamming codes because
	 * (1) We can't do it for BCH until we can read the full OOB area for BCH-8
	 * (2) OOB area is included in ECC calculation for BCH, so no need to check it
	 *      separately.
	 */


#if 1
	page = ((uint64_t)addr) >> chip->page_shift;
	// Must read entire page
	ret = chip->read_page(mtd, vbuf, oobbuf, page);
	if (ret) {
		printk(KERN_ERR "%s: brcmnand_read_page at %08x failed ret=%d\n",
		       __FUNCTION__, (unsigned int)addr, ret);
		brcmnand_post_mortem_dump(mtd, addr);
		return ret;
	}

#endif

	if (chip->ecclevel != BRCMNAND_ECC_HAMMING) {
		return ret; // We won't verify the OOB if not Hamming
	}

	/*
	 * If there are no Input Buffer, there is nothing to verify.
	 * Reading the page should be enough.
	 */
	if (!dbuf || dlen <= 0)
		return 0;

	for (eccstep = 0; eccstep < chip->eccsteps; eccstep++) {
		int pageOffset = eccstep * chip->eccsize;
		int oobOffset = eccstep * chip->eccOobSize;
		u_char sw_ecc[4];                       // SW ECC
		u_char* oobp = &oobbuf[oobOffset];      // returned from read op, contains HW ECC.

		brcmnand_Hamming_ecc(&dbuf[pageOffset], sw_ecc);

		if (sw_ecc[0] != oobp[6] || sw_ecc[1] != oobp[7] || sw_ecc[2] != oobp[8]) {
			if (oobp[6] == 0xff && oobp[7] == 0xff && oobp[8] == 0xff
			    && sw_ecc[0] == 0 && sw_ecc[1] == 0 && sw_ecc[2] == 0)
				; // OK
			else {
				printk("%s: Verification failed at %0llx.  HW ECC=%02x%02x%02x, SW ECC=%02x%02x%02x\n",
				       __FUNCTION__, addr,
				       oobp[6], oobp[7], oobp[8], sw_ecc[0], sw_ecc[1], sw_ecc[2]);
				ret = 1;
				break;
			}
		}

		// Verify the OOB if not NULL
		if (inp_oob) {
			if (memcmp(&inp_oob[oobOffset], oobp, 6) || memcmp(&inp_oob[oobOffset + 9], &oobp[9], 7)) {
				printk("+++++++++++++++++++++++ %s: OOB comp Hamming failed\n", __FUNCTION__);
				printk("In OOB:\n"); print_oobbuf(&inp_oob[oobOffset], 16);
				printk("\nVerify OOB:\n"); print_oobbuf(oobp, 16);
				ret = (-2);
				break;
			}
		}
	}

	return ret;
}
#endif

#if 1

#define brcmnand_verify_pageoob(...)            (0)

#else

/**
 * brcmnand_verify_pageoob - [GENERIC] verify the chip contents after a write
 * @param mtd		MTD device structure
 * @param dbuf		the databuffer to verify
 * @param dlen		the length of the data buffer, and should be less than mtd->writesize
 * @param fsbuf		the file system OOB data
 * @param fslen		the length of the file system buffer
 * @param oobsel		Specify how to write the OOB data
 * @param autoplace	Specify how to write the OOB data
 * @param raw		Ignore the Bad Block Indicator when true
 *
 * Assumes that lock on.  Munges the OOB internal buffer.
 */
static int brcmnand_verify_pageoob(struct mtd_info *mtd, loff_t addr, const u_char* fsbuf, int fslen,
				   struct nand_oobinfo *oobsel, int autoplace, int raw)
{
//	struct brcmnand_chip * chip = mtd->priv;
	//u_char* data_buf = chip->data_buf;
	u_char oob_buf[NAND_MAX_OOBSIZE]; // = chip->oob_buf;
	int ret = 0;
	//int complen;
	//char tmpfsbuf[NAND_MAX_OOBSIZE]; // Max oob size we support.
	int ooblen = 0;

	if (gdebug) printk("-->%s addr=%08x, fslen=%d, autoplace=%d, raw=%d\n", __FUNCTION__, __ll_low(addr),
			   fslen, autoplace, raw);

	// Must read entire page
	ret = brcmnand_read_pageoob(mtd, addr, oob_buf, &ooblen, oobsel, autoplace, raw);

	if (ret) {
		printk(KERN_ERR "%s: brcmnand_read_page at %p failed ret=%d\n",
		       __FUNCTION__, (void*)addr, ret);
		return ret;
	}

	if (gdebug) printk("%s: Calling verify_pageoob_priv(addr=%08x, fslen=%d, ooblen=%d\n",
			   __FUNCTION__, __ll_low(addr), fslen, ooblen);
	ret = brcmnand_verify_pageoob_priv(mtd, addr, fsbuf, fslen, oob_buf, ooblen, oobsel, autoplace, raw);

	return ret;
}

#endif

#else
#define brcmnand_verify_page(...)       (0)
#define brcmnand_verify_pageoob(...)            (0)
//#define brcmnand_verify_oob(...)		(0)
#endif



/**
 * brcmnand_write_page - [INTERNAL] write one page
 * @mtd:	MTD device structure
 * @chip:	NAND chip descriptor
 * @inp_buf:	the data to write
 * @inp_oob:	the spare area to write
 * @page:	page number to write
 * @cached:	cached programming [removed]
 */
static int
brcmnand_write_page(struct mtd_info *mtd,
		    const uint8_t *inp_buf, const uint8_t* inp_oob, uint64_t page)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int eccstep;
	int dataWritten = 0;
	int oobWritten = 0;
	int ret = 0;
	uint64_t offset = page << chip->page_shift;


	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx\n", __FUNCTION__, offset);
	}

	chip->pagebuf = page;

	for (eccstep = 0; eccstep < chip->eccsteps && ret == 0; eccstep++) {
		ret = brcmnand_posted_write_cache(mtd, &inp_buf[dataWritten],
						  inp_oob ? &inp_oob[oobWritten]  : NULL,
						  offset + dataWritten);

		if (ret < 0) {
			printk(KERN_ERR "%s: brcmnand_posted_write_cache failed at offset=%0llx, ret=%d\n",
			       __FUNCTION__, offset + dataWritten, ret);
			// TBD: Return the the number of bytes written at block boundary.
			dataWritten = 0;
			return ret;
		}
		dataWritten += chip->eccsize;
		oobWritten += chip->eccOobSize;
	}

	// TBD
#ifdef BRCMNAND_WRITE_VERIFY
	if (0 == ret) {
		int vret;
//gdebug = 0;
		vret = brcmnand_verify_page(mtd, offset, inp_buf, mtd->writesize, inp_oob, chip->eccOobSize);
//gdebug=save_debug;
		if (vret) BUG();
	}
#endif


	return ret;
}

#ifdef CONFIG_MTD_BRCMNAND_ISR_QUEUE

/*
 * Queue the entire page, then wait for completion
 */
static int
brcmnand_isr_write_page(struct mtd_info *mtd,
			const uint8_t *inp_buf, const uint8_t* inp_oob, uint64_t page)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int eccstep;
	int dataWritten = 0;
	int oobWritten = 0;
	int ret = 0;
	uint64_t offset = page << chip->page_shift;

	int submitted = 0;
	unsigned long flags;

	if (gdebug > 3 ) {
		printk("-->%s, page=%0llx\n", __FUNCTION__, page);
	}


#if 0   // No need to check, we are aligned on a page
	if (unlikely(offset - sliceOffset)) {
		printk(KERN_ERR "%s: offset %0llx is not cache aligned, sliceOffset=%0llx, CacheSize=%d\n",
		       __FUNCTION__, offset, sliceOffset, ECCSIZE(mtd));
		ret = -EINVAL;
		goto out;
	}
#endif


	if (unlikely(!EDU_buffer_OK((volatile void*)inp_buf, EDU_WRITE))) {
		if (gdebug > 3) printk("++++++++++++++++++++++++ %s: buffer not 32B aligned, trying non-EDU read\n", __FUNCTION__);
		/* EDU does not work on non-aligned buffers */
		ret = brcmnand_write_page(mtd, inp_buf, inp_oob, page);
		return (ret);
	}

	chip->pagebuf = page;

	spin_lock_irqsave(&gJobQ.lock, flags);
	if (!list_empty(&gJobQ.jobQ)) {
		printk("%s: Start read page but job queue not empty\n", __FUNCTION__);
		BUG();
	}
	gJobQ.cmd = EDU_WRITE;
	gJobQ.needWakeUp = 0;


	for (eccstep = 0; eccstep < chip->eccsteps && ret == 0; eccstep++) {
		eduIsrNode_t* req;
		/*
		 * Queue the 512B sector read, then read the EDU pending bit,
		 * and issue read command, if EDU is available for read.
		 */
		req = ISR_queue_write_request(mtd, &inp_buf[dataWritten],
					      inp_oob ? &inp_oob[oobWritten]  : NULL,
					      offset + dataWritten);

		dataWritten += chip->eccsize;
		oobWritten += chip->eccOobSize;
	}


	/*
	 * Kick start it.  The ISR will submit the next job
	 */
	if (!submitted) {
		submitted = brcmnand_isr_submit_job();
	}

	while (!list_empty(&gJobQ.jobQ)) {
		spin_unlock_irqrestore(&gJobQ.lock, flags);
		ret = ISR_wait_for_queue_completion();
		if (ret) {
			dataWritten = 0;
		}
		spin_lock_irqsave(&gJobQ.lock, flags);
	}
	spin_unlock_irqrestore(&gJobQ.lock, flags);
	return ret;

}

/*
 * Queue the several pages, then wait for completion
 * For 512B page sizes only.
 */
static int
brcmnand_isr_write_pages(struct mtd_info *mtd,
			 const uint8_t *inp_buf, const uint8_t* inp_oob, uint64_t startPage, int numPages)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int dataWritten = 0;
	int oobWritten = 0;
	int ret = 0;
	uint64_t offset = startPage << chip->page_shift;
	int page;

	int submitted = 0;
	unsigned long flags;

#if 0
	/* Already checked by caller */
	if (unlikely(!EDU_buffer_OK(inp_buf, EDU_WRITE))) {
		if (gdebug > 3) printk("++++++++++++++++++++++++ %s: buffer not 32B aligned, trying non-EDU read\n", __FUNCTION__);
		/* EDU does not work on non-aligned buffers */
		ret = brcmnand_write_page(mtd, inp_buf, inp_oob, startPage);
		return (ret);
	}
#endif
	/* Paranoia */
	if (chip->pageSize != chip->eccsize) {
		printk("%s: Can only be called on small page flash\n", __FUNCTION__);
		BUG();
	}

	spin_lock_irqsave(&gJobQ.lock, flags);
	if (!list_empty(&gJobQ.jobQ)) {
		printk("%s: Start read page but job queue not empty\n", __FUNCTION__);
		BUG();
	}
	gJobQ.cmd = EDU_WRITE;
	gJobQ.needWakeUp = 0;

//gdebug=4;
	for (page = 0; page < numPages && ret == 0; page++) {
		eduIsrNode_t* req;
		/*
		 * Queue the 512B sector read, then read the EDU pending bit,
		 * and issue read command, if EDU is available for read.
		 */

		req = ISR_queue_write_request(mtd, &inp_buf[dataWritten],
					      inp_oob ? &inp_oob[oobWritten]  : NULL,
					      offset + dataWritten);

		dataWritten += chip->eccsize;
		oobWritten += chip->eccOobSize;
	}
//gdebug=0;


	/*
	 * Kick start it.  The ISR will submit the next job
	 * We do it here, in order to avoid having to obtain the queue lock
	 * inside the ISR, in preparation for an RCU implementation.
	 */
	if (!submitted) {
		submitted = brcmnand_isr_submit_job();
	}

	while (!list_empty(&gJobQ.jobQ)) {
		spin_unlock_irqrestore(&gJobQ.lock, flags);
		ret = ISR_wait_for_queue_completion();
		if (ret) {
			dataWritten = 0;
		}
		spin_lock_irqsave(&gJobQ.lock, flags);
	}
	spin_unlock_irqrestore(&gJobQ.lock, flags);


	return ret;

}


#endif



/**
 * brcmnand_fill_oob - [Internal] Transfer client buffer to oob
 * @chip:	nand chip structure
 * @oob:	oob data buffer
 * @ops:	oob ops structure
 *
 * Returns the pointer to the OOB where next byte should be written to
 */
uint8_t *
brcmnand_fill_oob(struct brcmnand_chip *chip, uint8_t *oob, struct mtd_oob_ops *ops)
{
	// Already written in previous passes, relying on oob being intialized to ops->oobbuf
	size_t writtenLen = oob - ops->oobbuf;
	size_t len = ops->ooblen - writtenLen;


	switch (ops->mode) {

	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_RAW:
		memcpy(chip->oob_poi + ops->ooboffs, oob, len);
		return oob + len;

	case MTD_OPS_AUTO_OOB: {
		struct nand_oobfree *free = chip->ecclayout->oobfree;
		uint32_t boffs = 0, woffs = ops->ooboffs;
		size_t bytes = 0;

		memset(chip->oob_poi + ops->ooboffs, 0xff, chip->eccOobSize - ops->ooboffs);

		for (; free->length && len; free++, len -= bytes) {
			/* Write request not from offset 0 ? */
			if (unlikely(woffs)) {
				if (woffs >= free->length) {
					woffs -= free->length;
					continue;
				}
				boffs = free->offset + woffs;
				bytes = min_t(size_t, len,
					      (free->length - woffs));
				woffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(chip->oob_poi + boffs, oob, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		BUG();
	}
	return NULL;
}


#define NOTALIGNED(x) ((int)(x & (mtd->writesize - 1)) != 0)

/**
 * brcmnand_do_write_ops - [Internal] BRCMNAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operations description structure
 *
 * BRCMNAND write with ECC
 */
static int brcmnand_do_write_ops(struct mtd_info *mtd, loff_t to,
				 struct mtd_oob_ops *ops)
{
	uint64_t realpage;
	int blockmask;
	struct brcmnand_chip *chip = mtd->priv;
	uint32_t writelen = ops->len;
	uint8_t *oob = ops->oobbuf; //brcmnand_fill_oob relies on this
	uint8_t *buf = ops->datbuf;
	int bytes = mtd->writesize;
	int ret = 0;
	int numPages;
	int buffer_aligned = 0;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	uint8_t oobarea[512];
	int read_oob = 0;
	if ( !oob &&
	     ((chip->options & NAND_COMPLEX_OOB_WRITE) != 0 ||
	      (chip->ecclevel >= BRCMNAND_ECC_BCH_1 &&
	  chip->ecclevel <= BRCMNAND_ECC_BCH_12)) ) {
		read_oob = 1;
		oob = (uint8_t*)((uintptr_t)(oobarea + 0x0f) & ~0x0f);
		brcmnand_read_page_oob(mtd, oob, to >> chip->page_shift);
		ops->mode = MTD_OPS_PLACE_OOB;
		ops->ooboffs = 0;
		ops->ooblen = chip->eccsteps * chip->eccOobSize;
		ops->oobbuf = oob;
	}
#endif

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s, offset=%0llx\n", __FUNCTION__, to);

	ops->retlen = 0;

	/* reject writes, which are not page aligned */
	if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
		printk(KERN_NOTICE "nand_write: "
		       "Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	if (!writelen)
		return 0;

/* BrcmNAND multi-chips are treated as one logical chip *
        chipnr = (int)(to >> chip->chip_shift);
        chip->select_chip(mtd, chipnr);
 */



	realpage = to >> chip->page_shift;
	//page = realpage & chip->pagemask;
	blockmask = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;

	/* Invalidate the page cache, when we write to the cached page */
	if ((chip->pagebuf !=  -1LL) &&
	    (to <= (chip->pagebuf << chip->page_shift)) &&
	    ((to + ops->len) > (chip->pagebuf << chip->page_shift) )) {
		chip->pagebuf = -1LL;
	}

	/* THT: Provide buffer for brcmnand_fill_oob */
	if (unlikely(oob)) {
		chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
	}else  {
		chip->oob_poi = NULL;
	}

#ifdef  CONFIG_MTD_BRCMNAND_ISR_QUEUE
	/* Buffer must be aligned for EDU */
	buffer_aligned = EDU_buffer_OK(buf, EDU_WRITE);

#else   /* Dont care */
	buffer_aligned = 0;
#endif

	while (1) {

#ifdef  CONFIG_MTD_BRCMNAND_ISR_QUEUE
		/*
		 * Group several pages for submission for small page NAND
		 */
		numPages = min(MAX_JOB_QUEUE_SIZE, (int)writelen >> chip->page_shift);

		// If Batch mode
		if (buffer_aligned && numPages > 1 && chip->pageSize == chip->eccsize) {
			int j;

			/* Submit min(queueSize, len/512B) at a time */
			//numPages = min(MAX_JOB_QUEUE_SIZE, writelen>>chip->page_shift);
			bytes = chip->eccsize * numPages;

			if (unlikely(oob)) {
				//u_char* newoob;
				for (j = 0; j < numPages; j++) {
					oob = brcmnand_fill_oob(chip, oob, ops);
					/* THT: oob now points to where to read next,
					 * chip->oob_poi contains the OOB to be written
					 */
					/* In batch mode, we advance the OOB pointer to the next OOB slot
					 * using chip->oob_poi
					 */
					chip->oob_poi += chip->eccOobSize;
				}
				// Reset chip->oob_poi to beginning of OOB buffer for submission.
				chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
			}

			ret = brcmnand_isr_write_pages(mtd, buf, chip->oob_poi, realpage, numPages);
			if (ret) {
				ops->retlen = 0;
				return ret;
			}

		}else /* Else submit one page at a time */

#endif
		/* Submit one page at a time */
		{
			numPages = 1;
			bytes = mtd->writesize;

			if (unlikely(oob)) {
				chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
				oob = brcmnand_fill_oob(chip, oob, ops);
				/* THT: oob now points to where to read next,
				 * chip->oob_poi contains the OOB to be written
				 */
			}

			ret = chip->write_page(mtd, buf, chip->oob_poi, realpage);

		}

		if (ret)
			break;

		writelen -= bytes;
		if (!writelen)
			break;

		buf += bytes;
		realpage += numPages;
	}

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if ( read_oob ) {
		ops->ooboffs = 0;
		ops->ooblen = 0;
		ops->oobbuf = NULL;
	}
#endif

	ops->retlen = ops->len - writelen;
	if (unlikely(oob))
		ops->oobretlen = ops->ooblen;
	DEBUG(MTD_DEBUG_LEVEL3, "<-- %s\n", __FUNCTION__);
	return ret;
}


/**
 * BRCMnand_write - [MTD Interface] brcmNAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * BRCMNAND write with ECC
 */
static int brcmnand_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const uint8_t *buf)
{
	struct brcmnand_chip *chip = mtd->priv;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: to=%0llx\n", __FUNCTION__, to);

	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx\n", __FUNCTION__, to);
	}

	if ( kerSysIsDyingGaspTriggered() ) {
		printk("system is losing power, abort nand write offset %lx len %x,\n", (unsigned long)to, (unsigned int)len);
		return -EINVAL;
	}


	/* Do not allow writes past end of device */
	if (unlikely((to + len) > device_size(mtd))) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt to write beyond end of device\n",
		      __FUNCTION__);
		printk("brcmnand_write Attempt to write beyond end of device to 0x%x, len 0x%x, size of device 0x%x\n", (int)to, (unsigned int)len, (int)device_size(mtd) );
	}
	if (!len)
		return 0;

	brcmnand_get_device(mtd, BRCMNAND_FL_WRITING);

	chip->ops.len = len;
	chip->ops.datbuf = (uint8_t*)buf;
	chip->ops.oobbuf = NULL;

	ret = brcmnand_do_write_ops(mtd, to, &chip->ops);

	*retlen = chip->ops.retlen;

	brcmnand_release_device(mtd);
	return ret;
}


/**
 * brcmnand_write_page_oob - [INTERNAL] write one page
 * @mtd:	MTD device structure
 * @chip:	NAND chip descriptor.  The oob_poi pointer points to the OOB buffer.
 * @page:	page number to write
 */
static int brcmnand_write_page_oob(struct mtd_info *mtd,
				   const uint8_t* inp_oob, uint64_t page, int isFromMarkBadBlock)
{
	struct brcmnand_chip *chip = (struct brcmnand_chip*)mtd->priv;
	int eccstep;
	int oobWritten = 0;
	int ret = 0;
	uint64_t offset = page << chip->page_shift;

	chip->pagebuf = page;

	for (eccstep = 0; eccstep < chip->eccsteps && ret == 0; eccstep++) {
		ret = brcmnand_posted_write_oob(mtd,  &inp_oob[oobWritten],
						offset, isFromMarkBadBlock);
//gdebug=0;
		if (ret < 0) {
			printk(KERN_ERR "%s: brcmnand_posted_write_oob failed at offset=%0llx, ret=%d\n",
			       __FUNCTION__, offset, ret);
			return ret;
		}
		offset = offset + chip->eccsize;
		oobWritten += chip->eccOobSize;
	}

	// TBD
	ret = brcmnand_verify_pageoob();

	if (gdebug > 3 ) {
		printk("<--%s offset=%0llx\n", __FUNCTION__,  page << chip->page_shift);
		print_oobbuf(inp_oob, mtd->oobsize);
	}
	return ret;
}


/**
 * brcmnand_do_write_oob - [Internal] BrcmNAND write out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 *
 * BrcmNAND write out-of-band, no data.
 */
static int
brcmnand_do_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
	int numPages;
#endif
	int page;
	int status = 0;
	struct brcmnand_chip *chip = mtd->priv;
	u_char* oob = ops->oobbuf;;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: to = 0x%08x, len = %i\n", __FUNCTION__,
	      (unsigned int)to, (int)ops->len);
	if (gdebug > 3 ) {
		printk("-->%s, to=%08x, len=%d\n", __FUNCTION__, (uint32_t)to, (int)ops->len);
	}

	/* Do not allow write past end of page */
	if ((ops->ooboffs + ops->len) > mtd->oobsize) {
		DEBUG(MTD_DEBUG_LEVEL0, "nand_write_oob: "
		      "Attempt to write past end of page\n");
		return -EINVAL;
	}

/* BrcmNAND treats multiple chips as a single logical chip
        chipnr = (int)(to >> chip->chip_shift);
        chip->select_chip(mtd, chipnr);
 */

	/* Shift to get page */
	page = to >> chip->page_shift;

	/* Invalidate the page cache, if we write to the cached page */
	if ((int64_t)page == chip->pagebuf)
		chip->pagebuf = -1LL;

/* The #else case executes in an infinite loop. */
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (unlikely(oob)) {
		chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
		memset(chip->oob_poi, 0xff, mtd->oobsize);
		oob = brcmnand_fill_oob(chip, oob, ops);
		/* THT: oob now points to where to read next,
		 * chip->oob_poi contains the OOB to be written
		 */
	}

	status = chip->write_page_oob(mtd, chip->oob_poi, page, 0);
#else
	while (1) {
		/* Submit one page at a time */

		numPages = 1;

		if (unlikely(oob)) {
			chip->oob_poi = BRCMNAND_OOBBUF(chip->ctrl->buffers);
			memset(chip->oob_poi, 0xff, mtd->oobsize);
			oob = brcmnand_fill_oob(chip, oob, ops);
			/* THT: oob now points to where to read next,
			 * chip->oob_poi contains the OOB to be written
			 */
		}

		status |= chip->write_page_oob(mtd, chip->oob_poi, page, 0);

		if (status)
			break;

		page += numPages;
	} // Write 1 page OOB
#endif

	if (status)
		return status;

	ops->oobretlen = ops->ooblen;

	return 0;
}

/**
 * brcmnand_write_oob - [MTD Interface] BrcmNAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int
brcmnand_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct brcmnand_chip *chip = mtd->priv;
	int ret = -ENOTSUPP;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if ((chip->nop == 1) && ops->ooblen && !ops->len) { // quit if writing OOB only to NOP=1 parallel NAND device
		ops->retlen = 0;
		ops->oobretlen = ops->ooblen;
		return(0);
	}
#endif
	DEBUG(MTD_DEBUG_LEVEL3, "%s: to=%0llx\n", __FUNCTION__, to);

	if (gdebug > 3 ) {
		printk("-->%s, offset=%0llx, len=%08x\n", __FUNCTION__,  to, (int)ops->len);
	}

	if ( kerSysIsDyingGaspTriggered() ) {
		printk("system is losing power, abort nand write oob offset %lx\n", (unsigned long)to);
		return -EINVAL;
	}

	ops->retlen = 0;
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (!ops->datbuf) {
		ops->len = ops->ooblen;
	}
#endif

	/* Do not allow writes past end of device */

	if (unlikely((to + ops->len) > device_size(mtd))) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt to write beyond end of device\n",
		      __FUNCTION__);
		printk("brcmnand_write_oob Attempt to write beyond end of device to 0x%x, len 0x%x, size of device 0x%x\n", (int)to, (unsigned int)ops->len, (int)device_size(mtd) );
		return -EINVAL;
	}

	brcmnand_get_device(mtd, BRCMNAND_FL_WRITING);


	if (!ops->datbuf)
		ret = brcmnand_do_write_oob(mtd, to, ops);
	else
		ret = brcmnand_do_write_ops(mtd, to, ops);

#if 0
	if (unlikely(ops->mode == MTD_OPS_RAW))
		chip->ecc.write_page = write_page;
#endif

	//out:
	brcmnand_release_device(mtd);
	return ret;
}

/**
 * brcmnand_writev - [MTD Interface] compabilty function for brcmnand_writev_ecc
 * @param mtd		MTD device structure
 * @param vecs		the iovectors to write
 * @param count		number of vectors
 * @param to		offset to write to
 * @param retlen	pointer to variable to store the number of written bytes
 *
 * BrcmNAND write with kvec.
 */
static int brcmnand_writev(struct mtd_info *mtd, const struct kvec *vecs,
			   unsigned long count, loff_t to, size_t *retlen)
{
	int i, len, total_len, ret = -EIO, written = 0,  buflen;
	uint32_t page;
	int numpages = 0;
	struct brcmnand_chip * chip = mtd->priv;
	//int	ppblock = (1 << (chip->phys_erase_shift - chip->page_shift));
	u_char *bufstart = NULL;
	//u_char tmp_oob[NAND_MAX_OOBSIZE];
	u_char *data_buf;


	if (gdebug > 3 ) {
		printk("-->%s, offset=%08x\n", __FUNCTION__, (uint32_t)to);
	}

	if ( kerSysIsDyingGaspTriggered() )
		return -EINVAL;


	/* Preset written len for early exit */
	*retlen = 0;

	/* Calculate total length of data */
	total_len = 0;
	for (i = 0; i < count; i++)
		total_len += vecs[i].iov_len;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: to = 0x%08x, len = %i, count = %ld, eccbuf=%p, total_len=%d\n",
	      __FUNCTION__, (unsigned int)to, (unsigned int)total_len, count, NULL, total_len);

	/* Do not allow write past end of the device */


	if (unlikely((to + total_len) > device_size(mtd))) {
		DEBUG(MTD_DEBUG_LEVEL0, "brcmnand_writev_ecc: Attempted write past end of device\n");
		return -EINVAL;
	}

	/* Reject writes, which are not page aligned */
	if (unlikely(NOTALIGNED(to)) || unlikely(NOTALIGNED(total_len))) {
		DEBUG(MTD_DEBUG_LEVEL0, "brcmnand_writev_ecc: Attempt to write data not aligned to page\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	brcmnand_get_device(mtd, BRCMNAND_FL_WRITING);

	/* Setup start page, we know that to is aligned on page boundary */
	page = to > chip->page_shift;

	data_buf = BRCMNAND_malloc(sizeof(u_char) * mtd->writesize);
	if (unlikely(data_buf == NULL)) {
		printk(KERN_ERR "%s: vmalloc failed\n", __FUNCTION__);
		return -ENOMEM;
	}
	/* Loop until all keve's data has been written */
	len = 0;        // How many data from current iovec has been written
	written = 0;    // How many bytes have been written so far in all
	buflen = 0;     // How many bytes from the buffer has been copied to.
	while (count) {
		/* If the given tuple is >= pagesize then
		 * write it out from the iov
		 */
		// THT: We must also account for the partial buffer left over from previous iovec
		if ((buflen + vecs->iov_len - len) >= mtd->writesize) {
			/* Calc number of pages we can write
			 * out of this iov in one go */
			numpages = (buflen + vecs->iov_len - len) >> chip->page_shift;


			//oob = 0;
			for (i = 0; i < numpages; i++) {
				if (0 == buflen) {      // If we start a new page
					bufstart = &((u_char*)vecs->iov_base)[len];
				}else  {                // Reuse existing partial buffer, partial refill to end of page
					memcpy(&bufstart[buflen], &((u_char*)vecs->iov_base)[len], mtd->writesize - buflen);
				}

				ret = chip->write_page(mtd, bufstart, NULL, page);
				bufstart = NULL;

				if (ret) {
					printk("%s: brcmnand_write_page failed, ret=%d\n", __FUNCTION__, ret);
					goto out;
				}
				len += mtd->writesize - buflen;
				buflen = 0;
				//oob += oobretlen;
				page++;
				written += mtd->writesize;
			}
			/* Check, if we have to switch to the next tuple */
			if (len >= (int)vecs->iov_len) {
				vecs++;
				len = 0;
				count--;
			}
		} else { // (vecs->iov_len - len) <  mtd->writesize)
			 /*
			  * We must use the internal buffer, read data out of each
			  * tuple until we have a full page to write
			  */


			/*
			 * THT: Changed to use memcpy which is more efficient than byte copying, does not work yet
			 *  Here we know that 0 < vecs->iov_len - len < mtd->writesize, and len is not necessarily 0
			 */
			// While we have iovec to write and a partial buffer to fill
			while (count && (buflen < mtd->writesize)) {

				// Start new buffer?
				if (0 == buflen) {
					bufstart = data_buf;
				}
				if (vecs->iov_base != NULL && (vecs->iov_len - len) > 0) {
					// We fill up to the page
					int fillLen = min_t(int, vecs->iov_len - len, mtd->writesize - buflen);

					memcpy(&data_buf[buflen], &((u_char*)vecs->iov_base)[len], fillLen);
					buflen += fillLen;
					len += fillLen;
				}
				/* Check, if we have to switch to the next tuple */
				if (len >= (int)vecs->iov_len) {
					vecs++;
					len = 0;
					count--;
				}

			}
			// Write out a full page if we have enough, otherwise loop back to the top
			if (buflen == mtd->writesize) {

				numpages = 1;

				ret = chip->write_page(mtd, bufstart, NULL, page);
				if (ret) {
					printk("%s: brcmnand_write_page failed, ret=%d\n", __FUNCTION__, ret);
					goto out;
				}
				page++;
				written += mtd->writesize;

				bufstart = NULL;
				buflen = 0;
			}
		}

		/* All done ? */
		if (!count) {
			if (buflen) { // Partial page left un-written.  Imposible, as we check for totalLen being multiple of pageSize above.
				printk("%s: %d bytes left unwritten with writev_ecc at offset %0llx\n",
				       __FUNCTION__, buflen, ((uint64_t)page) << chip->page_shift);
				BUG();
			}
			break;
		}

	}
	ret = 0;
 out:
	/* Deselect and wake up anyone waiting on the device */
	brcmnand_release_device(mtd);

	BRCMNAND_free(data_buf);
	*retlen = written;
//if (*retlen <= 0) printk("%s returns retlen=%d, ret=%d, startAddr=%08x\n", __FUNCTION__, *retlen, ret, startAddr);
//printk("<-- %s: retlen=%d\n", __FUNCTION__, *retlen);
	return ret;
}

#if 0
/**
 * brcmnand_block_bad - [DEFAULT] Read bad block marker from the chip
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int brcmnand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int res = 0, ret = 0;
	uint32_t page;
	struct brcmnand_chip *chip = mtd->priv;
	u16 bad;
	uint8_t oob[NAND_MAX_OOBSIZE];

	//uint8_t* saved_poi;

	if (getchip) {
		page = __ll_RightShift(ofs, chip->page_shift);

#if 0
		chipnr = (int)(ofs >> chip->chip_shift);
#endif

		brcmnand_get_device(mtd, BRCMNAND_FL_READING);

#if 0
		/* Select the NAND device */
		chip->select_chip(mtd, chipnr);
#endif
	}
	page = __ll_RightShift(ofs, chip->page_shift);

	ret = chip->read_page_oob(mtd, oob, page);
	if (ret) {
		return 1;
	}

	if (chip->options & NAND_BUSWIDTH_16) {
		bad = (u16)cpu_to_le16(*(uint16*)(oob[chip->badblockpos]));
		if (chip->badblockpos & 0x1)
			bad >>= 8;
		if ((bad & 0xFF) != 0xff)
			res = 1;
	} else {
		if (oob[chip->badblockpos] != 0xff)
			res = 1;
	}

	if (getchip)
		brcmnand_release_device(mtd);

	return res;
}
#endif


/**
 * brcmnand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @param mtd		MTD device structure
 * @param ofs		offset from device start
 * @param getchip	0, if the chip is already selected
 * @param allowbbt	1, if its allowed to access the bbt area
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
static int brcmnand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip, int allowbbt)
{
	struct brcmnand_chip * chip = mtd->priv;
	int res;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (ofs < mtd->erasesize)
		return(0);
#endif
	if (getchip) {
		brcmnand_get_device(mtd, BRCMNAND_FL_READING);
	}

	// BBT already initialized
	if (chip->isbad_bbt) {

		/* Return info from the table */
		res = chip->isbad_bbt(mtd, ofs, allowbbt);
	}else  {
		res = brcmnand_isbad_raw(mtd, ofs);
	}

	if (getchip) {
		brcmnand_release_device(mtd);
	}

// if (res) PRINTK("%s: on Block at %0llx, ret=%d\n", __FUNCTION__, ofs, res);

	return res;
}

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
/**
 * brcmnand_erase_nolock - [Private] erase block(s)
 * @param mtd		MTD device structure
 * @param instr		erase instruction
 * @allowBBT			allow erase of BBT
 *
 * Erase one ore more blocks
 * ** FIXME ** This code does not work for multiple chips that span an address space > 4GB
 * Similar to BBT, except does not use locks and no alignment checks
 * Assumes lock held by caller
 */
static int brcmnand_erase_nolock(struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned int block_size;
	loff_t addr;
	int len;
	int ret = 0;
	int needBBT;

	block_size = (1 << chip->erase_shift);
	instr->fail_addr = 0xffffffffffffffffULL;

	/* Clear ECC registers */
	chip->ctrl_write(BCHP_NAND_ECC_CORR_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_ADDR, 0);
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	chip->ctrl_write(BCHP_NAND_ECC_CORR_EXT_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_EXT_ADDR, 0);
#endif

	/* Loop throught the pages */
	len = instr->len;
	addr = instr->addr;
	instr->state = MTD_ERASING;

	while (len) {
		/* Check if we have a bad block, we do not erase bad blocks */
		if (brcmnand_block_checkbad(mtd, addr, 0, allowbbt)) {
			printk(KERN_ERR "%s: attempt to erase a bad block at addr 0x%08x\n", __FUNCTION__, (unsigned int)addr);
			instr->state = MTD_ERASE_FAILED;
			goto erase_one_block;
		}
		chip->ctrl_writeAddr(chip, addr, 0);
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_BLOCK_ERASE);

		/* Wait until flash is ready */
		ret = brcmnand_ctrl_write_is_complete(mtd, &needBBT);

		/* Check, if it is write protected: TBD */
		if (needBBT ) {
			if ( !allowbbt) {
				printk(KERN_ERR "brcmnand_erase: Failed erase, block %d, flash status=%08x\n",
				       (unsigned int)(addr >> chip->erase_shift), needBBT);
				instr->state = MTD_ERASE_FAILED;
				instr->fail_addr = addr;
				printk(KERN_WARNING "%s: Marking bad block @%08x\n", __FUNCTION__, (unsigned int)addr);
				(void)chip->block_markbad(mtd, addr);
				goto erase_one_block;
			}
		}
 erase_one_block:
		len -= block_size;
		addr = addr + block_size;
	}

	instr->state = MTD_ERASE_DONE;
	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;
	/* Do call back function */
	if (!ret) {
		mtd_erase_callback(instr);
	}

	return ret;
}
#endif



/**
 * brcmnand_erase_bbt - [Private] erase block(s)
 * @param mtd		MTD device structure
 * @param instr		erase instruction
 * @allowBBT			allow erase of BBT
 * @doNotUseBBT		Do not look up in BBT
 *
 * Erase one ore more blocks
 * ** FIXME ** This code does not work for multiple chips that span an address space > 4GB
 */
static int
brcmnand_erase_bbt(struct mtd_info *mtd, struct erase_info *instr, int allowbbt, int doNotUseBBT)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned int block_size;
	loff_t addr;
	int len;
	int ret = 0;
	int needBBT;



	DEBUG(MTD_DEBUG_LEVEL3, "%s: start = %0llx, len = %08x\n", __FUNCTION__,
	      instr->addr, (unsigned int)instr->len);
//PRINTK( "%s: start = 0x%08x, len = %08x\n", __FUNCTION__, (unsigned int) instr->addr, (unsigned int) instr->len);

	block_size = (1 << chip->erase_shift);


	/* Start address must align on block boundary */
	if (unlikely(instr->addr & (block_size - 1))) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __FUNCTION__);
//if (gdebug > 3 )
		{ printk( "%s: Unaligned address\n", __FUNCTION__); }
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (unlikely(instr->len & (block_size - 1))) {
		DEBUG(MTD_DEBUG_LEVEL0,
		      "%s: Length not block aligned, len=%08x, blocksize=%08x, chip->blkSize=%08x, chip->erase=%d\n",
		      __FUNCTION__, (unsigned int)instr->len, (unsigned int)block_size,
		      (unsigned int)chip->blockSize, chip->erase_shift);
		PRINTK(
			"%s: Length not block aligned, len=%08x, blocksize=%08x, chip->blkSize=%08x, chip->erase=%d\n",
			__FUNCTION__, (unsigned int)instr->len, (unsigned int)block_size,
			(unsigned int)chip->blockSize, chip->erase_shift);
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if (unlikely((instr->len + instr->addr) > device_size(mtd))) {

		DEBUG(MTD_DEBUG_LEVEL0, "%s: Erase past end of device\n", __FUNCTION__);
//if (gdebug > 3 )
		{ printk(KERN_WARNING "%s: Erase past end of device, instr_addr=%016llx, instr->len=%08x, mtd->size=%16llx\n",
			 __FUNCTION__, (unsigned long long)instr->addr,
			 (unsigned int)instr->len, device_size(mtd)); }

		return -EINVAL;
	}


	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

	/*
	 * Clear ECC registers
	 */
	chip->ctrl_write(BCHP_NAND_ECC_CORR_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_ADDR, 0);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	chip->ctrl_write(BCHP_NAND_ECC_CORR_EXT_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_EXT_ADDR, 0);
#endif

	/* Loop throught the pages */
	len = instr->len;
	addr = instr->addr;
	instr->state = MTD_ERASING;

	while (len) {


/* THT: We cannot call brcmnand_block_checkbad which just look at the BBT,
   // since this code is also called when we create the BBT.
   // We must look at the actual bits, or have a flag to tell the driver
   // to read the BI directly from the OOB, bypassing the BBT
 */
		/* Check if we have a bad block, we do not erase bad blocks */
		if (brcmnand_block_checkbad(mtd, addr, 0, allowbbt)) {
			printk(KERN_ERR "%s: attempt to erase a bad block at addr 0x%08x\n", __FUNCTION__, (unsigned int)addr);
			// THT I believe we should allow the erase to go on to the next block in this case.
			instr->state = MTD_ERASE_FAILED;
//dump_stack();
			goto erase_exit;
		}

		//chip->command(mtd, ONENAND_CMD_ERASE, addr, block_size);

		chip->ctrl_writeAddr(chip, addr, 0);

		chip->ctrl_write(BCHP_NAND_CMD_START, OP_BLOCK_ERASE);

		// Wait until flash is ready
		ret = brcmnand_ctrl_write_is_complete(mtd, &needBBT);

		/* Check, if it is write protected: TBD */
		if (needBBT ) {
			if ( !allowbbt) {
				printk(KERN_ERR "brcmnand_erase: Failed erase, block %d, flash status=%08x\n",
				       (unsigned int)(addr >> chip->erase_shift), needBBT);

				instr->state = MTD_ERASE_FAILED;
				instr->fail_addr = addr;

				printk(KERN_WARNING "%s: Marking bad block @%08x\n", __FUNCTION__, (unsigned int)addr);
				(void)chip->block_markbad(mtd, addr);
				goto erase_exit;
			}
		}
		len -= block_size;
		addr = addr + block_size;
	}

	instr->state = MTD_ERASE_DONE;

 erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;
	/* Do call back function */
	if (!ret) {
		mtd_erase_callback(instr);
	}

	DEBUG(MTD_DEBUG_LEVEL0, "<--%s\n", __FUNCTION__);
	return ret;
}


/**
 * brcmnand_erase - [MTD Interface] erase block(s)
 * @param mtd		MTD device structure
 * @param instr		erase instruction
 *
 * Erase one ore more blocks
 */
static int brcmnand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct brcmnand_chip * chip = mtd->priv;
	int ret = 0;
	unsigned int block_size;
	int allowbbt = 0;

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
	loff_t addr;
	int len;

	/* save the instr len and addr first because JFFS2 caller free instr when instr->callback is called */
	len = instr->len;
	addr = instr->addr;


#endif

	if ( kerSysIsDyingGaspTriggered() ) {
		printk("system is losing power, abort nand erase offset %lx len %x,\n", (unsigned long)instr->addr, (int)instr->len);
		return -EINVAL;
	}

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s addr=%08lx, len=%d\n", __FUNCTION__,
	      (unsigned long)instr->addr, (int)instr->len);

	/* Grab the lock and see if the device is available */
	brcmnand_get_device(mtd, BRCMNAND_FL_ERASING);

	block_size = (1 << chip->erase_shift);

	ret = brcmnand_erase_bbt(mtd, instr, allowbbt, 0); // Do not allow erase of BBT, and use BBT

	/* Deselect and wake up anyone waiting on the device */
	brcmnand_release_device(mtd);

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
	if (chip->cet) {
		if (chip->cet->flags != BRCMNAND_CET_DISABLED &&
		    chip->cet->flags != BRCMNAND_CET_LAZY && allowbbt != 1) {
			while (len) {
				/* Skip if bad block */
				if (brcmnand_block_checkbad(mtd, addr, 0, allowbbt)) {
					printk(KERN_ERR "%s: attempt to erase a bad block at addr 0x%08x\n", __FUNCTION__, (unsigned int)addr);
					len -= block_size;
					addr = addr + block_size;
					continue;
				}
				if (brcmnand_cet_erasecallback(mtd, addr) < 0) {
					printk(KERN_INFO "Error in CET erase callback, disabling CET\n");
					chip->cet->flags = BRCMNAND_CET_DISABLED;
				}
				len -= block_size;
				addr = addr + block_size;
			}
		}
	}
#endif
	return ret;
}

/**
 * brcmnand_sync - [MTD Interface] sync
 * @param mtd		MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void brcmnand_sync(struct mtd_info *mtd)
{
	DEBUG(MTD_DEBUG_LEVEL3, "brcmnand_sync: called\n");

	/* Grab the lock and see if the device is available */
	brcmnand_get_device(mtd, BRCMNAND_FL_SYNCING);

	PLATFORM_IOFLUSH_WAR();

	/* Release it and go back */
	brcmnand_release_device(mtd);
}


/**
 * brcmnand_block_isbad - [MTD Interface] Check whether the block at the given offset is bad
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 *
 * Check whether the block is bad
 */
static int brcmnand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	//struct brcmnand_chip * chip = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s ofs=%0llx\n", __FUNCTION__, ofs);

	/* Check for invalid offset */
	if (ofs > device_size(mtd)) {
		return -EINVAL;
	}

	return brcmnand_block_checkbad(mtd, ofs, 1, 0);
}

/**
 * brcmnand_default_block_markbad - [DEFAULT] mark a block bad
 * @param mtd		MTD device structure
 * @param ofs		offset from device start
 *
 * This is the default implementation, which can be overridden by
 * a hardware specific driver.
 */
static int brcmnand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct brcmnand_chip * chip = mtd->priv;
	//struct bbm_info *bbm = chip->bbm;
	// THT: 3/20/07: We restrict ourselves to only support x8.
	// Revisit this for x16.
	u_char bbmarker[1] = { 0 };  // CFE and BBS uses 0x0F, Linux by default uses 0
	//so we can use this to mark the difference
	u_char buf[NAND_MAX_OOBSIZE];
	//size_t retlen;
	uint32_t block, page;
	int dir;
	uint64_t ulofs;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s ofs=%0llx\n", __FUNCTION__,  ofs);
//printk("-->%s ofs=%0llx\n", __FUNCTION__,  ofs);

	// Page align offset
	ulofs = ((uint64_t)ofs) & (~chip->page_mask);


	/* Get block number.  Block is guaranteed to be < 2*32 */
	block = (uint32_t)(ulofs >> chip->erase_shift);

	// Block align offset
	ulofs = ((uint64_t)block) << chip->erase_shift;

	if (!NAND_IS_MLC(chip)) { // SLC chip, mark first and 2nd page as bad.
		printk(KERN_INFO "Mark SLC flash as bad at offset %0llx, badblockpos=%d\n", ofs, chip->badblockpos);
		page = block << (chip->erase_shift - chip->page_shift);
		dir = 1;
	}else  { // MLC chip, mark last and previous page as bad.
		printk(KERN_INFO "Mark MLC flash as bad at offset %0llx\n", ofs);
		page = ((block + 1) << (chip->erase_shift - chip->page_shift)) - 1;
		dir = -1;
	}
	if (chip->bbt) {
		chip->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);
	}

	memcpy(buf, ffchars, sizeof(buf));
	memcpy(&buf[chip->badblockpos], bbmarker, sizeof(bbmarker));

	// Lock already held by caller, so cant call mtd->write_oob directly
	ret = chip->write_page_oob(mtd, buf, page, 1);
	if (ret) {
		printk(KERN_INFO "Mark bad page %d failed with retval=%d\n", page, ret);
	}

	// Mark 2nd page as bad, ignoring last write
	page += dir;
	// Lock already held by caller, so cant call mtd->write_oob directly
	DEBUG(MTD_DEBUG_LEVEL3, "%s Calling chip->write_page(page=%08x)\n", __FUNCTION__, page);
	ret = chip->write_page_oob(mtd, buf, page, 1);
	if (ret) {
		printk(KERN_INFO "Mark bad page %d failed with retval=%d\n", page, ret);
	}

	/*
	 * According to the HW guy, even if the write fails, the controller have written
	 * a 0 pattern that certainly would have written a non 0xFF value into the BI marker.
	 *
	 * Ignoring ret.  Even if we fail to write the BI bytes, just ignore it,
	 * and mark the block as bad in the BBT
	 */
	DEBUG(MTD_DEBUG_LEVEL3, "%s Calling brcmnand_update_bbt(ulofs=%0llx))\n", __FUNCTION__, ulofs);
	(void)brcmnand_update_bbt(mtd, ulofs);
	//if (!ret)
	mtd->ecc_stats.badblocks++;
	return ret;
}

/**
 * brcmnand_block_markbad - [MTD Interface] Mark the block at the given offset as bad
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 *
 * Mark the block as bad
 */
static int brcmnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct brcmnand_chip * chip = mtd->priv;
	int ret;

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s ofs=%08x\n", __FUNCTION__, (unsigned int)ofs);

	if ( kerSysIsDyingGaspTriggered() )
		return -EINVAL;

	ret = brcmnand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing */
		if (ret > 0)
			return 0;
		return ret;
	}

	return chip->block_markbad(mtd, ofs);
}

/**
 * brcmnand_unlock - [MTD Interface] Unlock block(s)
 * @param mtd		MTD device structure
 * @param ofs		offset relative to mtd start
 * @param len		number of bytes to unlock
 *
 * Unlock one or more blocks
 */
static int brcmnand_unlock(struct mtd_info *mtd, loff_t llofs, uint64_t len)
{

#if 0
// Only Samsung Small flash uses this.

	struct brcmnand_chip * chip = mtd->priv;
	int status;
	uint64_t blkAddr, ofs = (uint64_t)llofs;

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s llofs=%08x, len=%d\n", __FUNCTION__,
	      (unsigned long)llofs, (int)len);



	/* Block lock scheme */
	for (blkAddr = ofs; blkAddr <  (ofs + len); blkAddr = blkAddr + chip->blockSize) {

		/* The following 2 commands share the same CMD_EXT_ADDR, as the block never cross a CS boundary */
		chip->ctrl_writeAddr(chip, blkAddr, 0);
		/* Set end block address */
		chip->ctrl_writeAddr(chip, blkAddr + chip->blockSize - 1, 1);
		/* Write unlock command */
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_BLOCKS_UNLOCK);


		/* There's no return value */
		chip->wait(mtd, BRCMNAND_FL_UNLOCKING, &status);

		if (status & 0x0f)
			printk(KERN_ERR "block = %0llx, wp status = 0x%x\n", blkAddr, status);

		/* Check lock status */
		chip->ctrl_writeAddr(chip, blkAddr, 0);
		chip->ctrl_write(BCHP_NAND_CMD_START, OP_READ_BLOCKS_LOCK_STATUS);
		status = chip->ctrl_read(BCHP_NAND_BLOCK_LOCK_STATUS);
		//status = chip->read_word(chip->base + ONENAND_REG_WP_STATUS);

	}
#endif
	return 0;
}


/**
 * brcmnand_print_device_info - Print device ID
 * @param device        device ID
 *
 * Print device ID
 */
static void brcmnand_print_device_info(brcmnand_chip_Id* chipId, struct mtd_info* mtd)
{
	struct brcmnand_chip * chip = mtd->priv;
	int cs = chip->ctrl->CS[chip->csi];

	printk(KERN_INFO "BrcmNAND mfg %x %x %s %dMB on CS%d\n",
	       chipId->mafId, chipId->chipId, chipId->chipIdStr, \
	       mtd64_ll_low(chip->chipSize >> 20), cs);

	print_config_regs(mtd);

}

/*
 * Calculate the bit fields FUL_ADR_BYTES, COL_ADR_BYTES and BLK_ADR_BYTES
 * without which, Micron flashes - which do not have traditional decode-ID opcode 90H-00H -
 * would not work.
 *
 * @chip: Structure containing the page size, block size, and device size.
 * @nand_config: nand_config register with page size, block size, device size already encoded.
 *
 * returns the updated nand_config register.
 */
uint32_t
brcmnand_compute_adr_bytes(struct brcmnand_chip* chip, uint32_t nand_config)
{

	uint32_t nbrPages;
	uint32_t fulAdrBytes, colAdrBytes, blkAdrBytes, nbrPagesShift;

	colAdrBytes = 2;

	PRINTK("-->%s, chip->chipSize=%llx\n", __FUNCTION__, chip->chipSize);

	nbrPages = (uint32_t)(chip->chipSize >> chip->page_shift);
	nbrPagesShift = ffs(nbrPages) - 1; /* = power of 2*/
	blkAdrBytes =  (nbrPagesShift + 7) / 8;

	fulAdrBytes = colAdrBytes + blkAdrBytes;

	nand_config &= ~(BCHP_NAND_CONFIG_FUL_ADR_BYTES_MASK
			 | BCHP_NAND_CONFIG_COL_ADR_BYTES_MASK
			 | BCHP_NAND_CONFIG_BLK_ADR_BYTES_MASK);
	nand_config |= (fulAdrBytes << BCHP_NAND_CONFIG_FUL_ADR_BYTES_SHIFT)
		       | (colAdrBytes << BCHP_NAND_CONFIG_COL_ADR_BYTES_SHIFT)
		       | (blkAdrBytes << BCHP_NAND_CONFIG_BLK_ADR_BYTES_SHIFT);
	PRINTK("%s: nbrPages=%x, blkAdrBytes=%d, colAdrBytes=%d, nand_config=%08x\n",
	       __FUNCTION__, nbrPages, blkAdrBytes, colAdrBytes, nand_config);
	return nand_config;
}

/*
 * bit 31:      1 = OTP read-only
 *      v2.1 and earlier: 30:           Page Size: 0 = PG_SIZE_512, 1 = PG_SIZE_2KB version
 * 28-29:       Block size: 3=512K, 1 = 128K, 0 = 16K, 2 = 8K
 * 24-27:	Device_Size
 *			0:	4MB
 *			1:	8MB
 *			2:      16MB
 *			3:	32MB
 *			4:	64MB
 *			5:	128MB
 *			6:      256MB
 *			7:	512MB
 *			8:	1GB
 *			9:	2GB
 *			10:	4GB  << Hit limit of MTD struct here.
 *			11:	8GB
 *			12:	16GB
 *			13:	32GB
 *			14:	64GB
 *			15:	128GB
 * 23:		Dev_Width 0 = Byte8, 1 = Word16
 *   v2.1 and earlier:22-19:    Reserved
 *   v2.2 and later:  21:20	page Size
 * 18:16:	Full Address Bytes
 * 15		Reserved
 * 14:12	Col_Adr_Bytes
 * 11:		Reserved
 * 10-08	Blk_Adr_Bytes
 * 07-00	Reserved
 */

void
brcmnand_decode_config(struct brcmnand_chip* chip, uint32_t nand_config)
{
	unsigned int chipSizeShift;
	unsigned int blk_size_cfg;
	unsigned int page_size_cfg;

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_1
	uint32_t nand_config_ext = brcmnand_ctrl_read(BCHP_NAND_CONFIG_EXT);

	blk_size_cfg = (nand_config_ext & BCHP_NAND_CONFIG_BLOCK_SIZE_MASK)
		       >> BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT;
	page_size_cfg = (nand_config_ext & BCHP_NAND_CONFIG_PAGE_SIZE_MASK)
			>> BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT;
#else
	blk_size_cfg = (nand_config & BCHP_NAND_CONFIG_BLOCK_SIZE_MASK)
		       >> BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT;
	page_size_cfg = (nand_config & BCHP_NAND_CONFIG_PAGE_SIZE_MASK)
			>> BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT;
#endif
	//chip->chipSize = (nand_config & 0x07000000) >> (24 - 20);

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_2_2
	// Version 2.1 or earlier: 2 bit field 28:29
	switch (blk_size_cfg) {
	case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB:
		chip->blockSize = 512 << 10;
		break;
	case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8KB:
		chip->blockSize = 8 << 10;
		break;
	case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB:
		chip->blockSize = 128 << 10;
		break;
	case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB:
		chip->blockSize = 16 << 10;
		break;
	}
#else
	// Version 2.2 or later: 3 bits 28:30
	if (chip->blockSize != (1 << 20)) {
		switch (blk_size_cfg) {
  #if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_1
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8192KB:
			chip->blockSize = 8192 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_4096KB:
			chip->blockSize = 4096 << 10;
			break;
  #endif
  #if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_1024KB:
			chip->blockSize = 1024 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_2048KB:
			chip->blockSize = 2048 << 10;
			break;
  #endif
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB:
			chip->blockSize = 256 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB:
			chip->blockSize = 512 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_8KB:
			chip->blockSize = 8 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB:
			chip->blockSize = 128 << 10;
			break;
		case BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB:
			chip->blockSize = 16 << 10;
			break;
		}
	}
	/*
	 * 1MB block size:
	 * Nothing to do, we have already recorded it
	 */
#endif


	chip->erase_shift = ffs(chip->blockSize) - 1;
	printk("Block size=%08x, erase shift=%d\n", chip->blockSize, chip->erase_shift);

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_2_2
	// Version 2.1 or earlier: Bit 30
	switch (page_size_cfg) {
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512:
		chip->pageSize = 512;
		break;
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB:
		chip->pageSize = 2048;
		break;
	}

#else
	// Version 2.2 or later, bits 20:21
	switch (page_size_cfg) {
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512:
		chip->pageSize = 512;
		break;
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB:
		chip->pageSize = 2048;
		break;
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB:
		chip->pageSize = 4096;
		break;
  #if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB:
		chip->pageSize = 8192;
		break;
  #elif CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_4
	case BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB:
	{
		uint32_t ctrlVersion = brcmnand_ctrl_read(BCHP_NAND_REVISION);

		/* Only if the controller supports it: */
		chip->pageSize = 8192;
		if (!(ctrlVersion & BCHP_NAND_REVISION_8KB_PAGE_SUPPORT_MASK)) {
			printk(KERN_ERR "Un-supported page size 8KB\n");
			BUG();
		}
	}
	break;
  #else         /* Version 3.3 or earlier */
	case 3:
		printk(KERN_ERR "Un-supported page size\n");
		chip->pageSize = 0;         // Let it crash
		BUG();
		break;
  #endif

	}
#endif

	chip->page_shift = ffs(chip->pageSize) - 1;
	chip->page_mask = (1 << chip->page_shift) - 1;

	chipSizeShift = (nand_config & BCHP_NAND_CONFIG_DEVICE_SIZE_MASK)
			>> BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT;

	chip->chipSize = 4ULL << (20 + chipSizeShift);

	chip->busWidth = 1 + ((nand_config & BCHP_NAND_CONFIG_DEVICE_WIDTH_MASK)
			      >> BCHP_NAND_CONFIG_DEVICE_WIDTH_SHIFT);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_1
	printk(KERN_INFO "NAND Config: Reg=%08x, Config Ext: Reg=%08x, chipSize=%d MB, blockSize=%dK, erase_shift=%x\n",
	       nand_config, nand_config_ext, mtd64_ll_low(chip->chipSize >> 20), chip->blockSize >> 10, chip->erase_shift);
#else
	printk(KERN_INFO "NAND Config: Reg=%08x, chipSize=%d MB, blockSize=%dK, erase_shift=%x\n",
	       nand_config, mtd64_ll_low(chip->chipSize >> 20), chip->blockSize >> 10, chip->erase_shift);
#endif
	printk(KERN_INFO "busWidth=%d, pageSize=%dB, page_shift=%d, page_mask=%08x\n",
	       chip->busWidth, chip->pageSize, chip->page_shift, chip->page_mask);

}

/*
 * Adjust timing pattern if specified in chip ID list
 * Also take dummy entries, but no adjustments will be made.
 */
static void brcmnand_adjust_timings(struct brcmnand_chip *this, brcmnand_chip_Id* chip)
{
	int csi = this->csi;
	int __maybe_unused cs = this->ctrl->CS[this->csi];

	unsigned long nand_timing1 = this->ctrl_read(bchp_nand_timing1(cs));
	unsigned long nand_timing1_b4;
	unsigned long nand_timing2 = this->ctrl_read(bchp_nand_timing2(cs));
	unsigned long nand_timing2_b4;
	extern uint32_t gNandTiming1[];
	extern uint32_t gNandTiming2[];



	/*
	 * Override database values with kernel command line values
	 */
	if (0 != gNandTiming1[csi] || 0 != gNandTiming2[csi]) {
		if (0 != gNandTiming1[csi] ) {
			chip->timing1 = gNandTiming1[csi];
			//this->ctrl_write(BCHP_NAND_TIMING_1, gNandTiming1);
		}
		if (0 != gNandTiming2[csi]) {
			chip->timing2 = gNandTiming2[csi];
			//this->ctrl_write(BCHP_NAND_TIMING_2, gNandTiming2);
		}
		//return;
	}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0
	if ((chip->timing1 == 0) && (chip->timing2 == 0)) {
		// if we don't know better, force max read speed
		chip->timing1 = 0x00320000;
		chip->timing2 = 0x00000004;
	}
#endif
	// Adjust NAND timings from database or command line
	if (chip->timing1) {
		nand_timing1_b4 = nand_timing1;

		if (chip->timing1 & BCHP_NAND_TIMING_1_tWP_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tWP_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tWP_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tWH_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tWH_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tWH_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tRP_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tRP_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tRP_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tREH_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tREH_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tREH_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tCS_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tCS_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tCS_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tCLH_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tCLH_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tCLH_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tALH_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tALH_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tALH_MASK);
		}
		if (chip->timing1 & BCHP_NAND_TIMING_1_tADL_MASK) {
			nand_timing1 &= ~BCHP_NAND_TIMING_1_tADL_MASK;
			nand_timing1 |= (chip->timing1 & BCHP_NAND_TIMING_1_tADL_MASK);
		}


		this->ctrl_write(bchp_nand_timing1(cs), nand_timing1);

		if (gdebug > 3 ) {
			printk("Adjust timing1: Was %08lx, changed to %08lx\n", nand_timing1_b4, nand_timing1);
		}
	}else  {
		printk("timing1 not adjusted: %08lx\n", nand_timing1);
	}

	// Adjust NAND timings:
	if (chip->timing2) {
		nand_timing2_b4 = nand_timing2;

		if (chip->timing2 & BCHP_NAND_TIMING_2_tWB_MASK) {
			nand_timing2 &= ~BCHP_NAND_TIMING_2_tWB_MASK;
			nand_timing2 |= (chip->timing2 & BCHP_NAND_TIMING_2_tWB_MASK);
		}
		if (chip->timing2 & BCHP_NAND_TIMING_2_tWHR_MASK) {
			nand_timing2 &= ~BCHP_NAND_TIMING_2_tWHR_MASK;
			nand_timing2 |= (chip->timing2 & BCHP_NAND_TIMING_2_tWHR_MASK);
		}
		if (chip->timing2 & BCHP_NAND_TIMING_2_tREAD_MASK) {
			nand_timing2 &= ~BCHP_NAND_TIMING_2_tREAD_MASK;
			nand_timing2 |= (chip->timing2 & BCHP_NAND_TIMING_2_tREAD_MASK);
		}

		this->ctrl_write(bchp_nand_timing2(cs), nand_timing2);

		if (gdebug > 3 ) {
			printk("Adjust timing2: Was %08lx, changed to %08lx\n", nand_timing2_b4, nand_timing2);
		}
	}else  {
		printk("timing2 not adjusted: %08lx\n", nand_timing2);
	}
}


#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_2_2
static int
is_ecc_strong(int registerEcc, int requiredEcc)
{
	if (registerEcc == BRCMNAND_ECC_HAMMING)
		registerEcc = 1;

	else if (registerEcc == BRCMNAND_ECC_DISABLE)
		return 1; // Internal ECC is always stronger

	if (requiredEcc == BRCMNAND_ECC_HAMMING)
		requiredEcc = 1;

	return (registerEcc >= requiredEcc);
}

static void
brcmnand_set_acccontrol(struct brcmnand_chip * chip, unsigned int chipSelect,
			uint32_t pageSize, uint16_t oobSizePerPage, int reqEcc, int codeWorkSize, int nbrBitsPerCell)
{
	int actualReqEcc = reqEcc;
	int eccLevel;
	uint32_t b1Ksector = 0;
	uint32_t acc0, acc;
	int oobPerSector = oobSizePerPage / (pageSize / 512);
	uint32_t cellinfo;


	PRINTK("-->%s(oob=%d, ecc=%d, cw=%d)\n", __FUNCTION__, oobSizePerPage, reqEcc, codeWorkSize);

	if (oobPerSector == 28)
		oobPerSector = 27; /* Reduce Micron oobsize to match other vendors in order to share codes */

	if (codeWorkSize == 1024) {
		actualReqEcc = reqEcc / 2;
		b1Ksector = 1;
	}

	acc = acc0 = chip->ctrl_read(bchp_nand_acc_control(chipSelect));
	eccLevel = (acc & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK)
		   >> BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;
	if (!is_ecc_strong(eccLevel, actualReqEcc)) {
		eccLevel = actualReqEcc;
	}

	acc &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		 | BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_MASK
#endif
		 | BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK);

	printk("eccLevel=%d, 1Ksector=%d, oob=%d\n", eccLevel, b1Ksector, oobPerSector);

	acc |= eccLevel << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
	acc |= b1Ksector << BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_SHIFT;
#endif
	acc |= oobPerSector << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT;

	if (chipSelect == 0) {
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
		acc &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
			 | BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_0_MASK
#endif
			 | BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_MASK);
		acc |= eccLevel << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		acc |= b1Ksector << BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_0_SHIFT;
#endif
		acc |= oobPerSector << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_SHIFT;
#endif
	}

	/* Clear FAST_PGM_RDIN, PARTIAL_PAGE_EN if MLC */
	cellinfo = ffs(nbrBitsPerCell) - 2;

	PRINTK("cellinfo=%d\n", cellinfo);

	chip->cellinfo = cellinfo << 2; /* Mask is 0x0C */

	printk("nbrBitsPerCell=%d, cellinfo=%d, chip->cellinfo=%08x\n", nbrBitsPerCell, cellinfo, chip->cellinfo);
	if (NAND_IS_MLC(chip)) {
		acc  &= ~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
			BCHP_NAND_ACC_CONTROL_FAST_PGM_RDIN_MASK |
#endif
			BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_MASK);
	}


	chip->ctrl_write(bchp_nand_acc_control(chipSelect), acc);
	printk("<--%s: acc b4: %08x, after: %08x\n", __FUNCTION__, acc0, acc);

}


/*
 * Override database values with kernel command line values and
 * set internal data structure values - when the flash ID is NOT in the database -
 * using the values set by the CFE
 */
static void brcmnand_adjust_acccontrol(struct brcmnand_chip *chip, int isONFI, int inIdTable, int idTableIdx)
{
	int cs = chip->ctrl->CS[chip->csi];
	unsigned long nand_acc_b4 = chip->ctrl_read(bchp_nand_acc_control(cs));
	unsigned long nand_acc = nand_acc_b4;
	int eccLevel;
	int oobSize;
	int updateInternalData = 0;

	PRINTK("%s: gAccControl[CS=%d]=%08x, ACC=%08lx\n",
	       __FUNCTION__, cs, gAccControl[chip->csi], nand_acc_b4);

	if (cs != 0 && 0 != gAccControl[chip->csi]) {
		nand_acc = gAccControl[chip->csi];
		chip->ctrl_write(bchp_nand_acc_control(cs), nand_acc);
		printk("NAND ACC CONTROL on CS%1d changed to %08x, from %08lx,\n", cs,
		       chip->ctrl_read(bchp_nand_acc_control(cs)), nand_acc_b4);

		updateInternalData = 1;
	}else if (!inIdTable && !isONFI) {
		updateInternalData = 1;
	}
	/* Update ACC-CONTROL when not on CS0 */
	else if (cs != 0 && inIdTable) {
		int oobSizePerPage = chip->eccOobSize * (chip->pageSize / 512);
		brcmnand_set_acccontrol(chip, cs,
					chip->pageSize, oobSizePerPage, chip->reqEccLevel, chip->eccSectorSize, 2 + NAND_IS_MLC(chip));
	}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0 
	if( chip->pageSize > 512 ) {
		nand_acc |= 1 << BCHP_NAND_ACC_CONTROL_PREFETCH_EN_SHIFT;
		chip->ctrl_write(bchp_nand_acc_control(cs), nand_acc);
	}
#endif


	/*
	 * update InternalData is TRUE only when
	 * (a) We are on CS0, and the chip is not in the database, in which case we use the values
	 *		used by the CFE.
	 * (b) an ACC value was passed on the command line
	 */

	/* Update ECC level */
	if (updateInternalData) {
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_3
		eccLevel = (nand_acc & BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_MASK) >>
			   BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_SHIFT;
		oobSize = (nand_acc & BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_MASK) >>
			  BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_SHIFT;

#else
		eccLevel = (nand_acc & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK) >>
			   BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;
		oobSize = (nand_acc & BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK) >>
			  BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT;
#endif

		chip->ecclevel = eccLevel;
		printk("ECC level changed to %d\n", eccLevel);

		chip->eccOobSize = oobSize;
		printk("OOB size changed to %d\n", oobSize);

		/* Assume MLC if both RDIN and PARTIAL_PAGE are disabled */
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
		/* this section needs to be commented out because we now turn off partial page writes
		 * to support NOP=1 devices and this code may trigger MLC=true for an unidentified NAND device,
		 * causing the NAND device to be accessed improperly
		   if (0 == (nand_acc & (
		                BCHP_NAND_ACC_CONTROL_FAST_PGM_RDIN_MASK |
		                BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_MASK))) {
		        chip->cellinfo = 0x04; // MLC NAND
		        printk("Flash type changed to MLC\n");
		   }
		 */
#endif
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		if (nand_acc & BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_MASK) {
			chip->eccSectorSize = 1024;
			printk("Sector size changed to 1024\n");
		}
#endif
	}
}
#endif


static void
brcmnand_read_id(struct mtd_info *mtd, unsigned int chipSelect, unsigned long* dev_id)
{
	struct brcmnand_chip * chip = mtd->priv;
	uint32_t status;
	uint32_t nandConfig = chip->ctrl_read(bchp_nand_config(chipSelect));
	uint32_t csNandSelect = 0;
	uint32_t nandSelect = 0;

	if (chipSelect > 0) { // Do not re-initialize when on CS0, Bootloader already done that

  #if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
		/* Older version do not have EXT_ADDR registers */
		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect << BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);
  #endif        //

		chip->ctrl_write(BCHP_NAND_CMD_START,
				 BCHP_NAND_CMD_START_OPCODE_FLASH_RESET << BCHP_NAND_CMD_START_OPCODE_SHIFT);

	}
/* Wait for CTRL_Ready */
	brcmnand_wait(mtd, BRCMNAND_FL_READY, &status, 10000);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_0_1
	nandSelect = chip->ctrl_read(BCHP_NAND_CS_NAND_SELECT);

	printk("B4: NandSelect=%08x, nandConfig=%08x, chipSelect=%d\n", nandSelect, nandConfig, chipSelect);


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	/* Older version do not have EXT_ADDR registers */
	chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
	chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect << BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);
#endif  // Set EXT address if version >= 1.0

	// Has CFE initialized the register?
	if (0 == (nandSelect & BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK)) {

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_0_1
		csNandSelect = 1 << (BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_SHIFT + chipSelect);

// v1.0 does not define it
#elif CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_2_0
		csNandSelect = 1 << (BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_USES_NAND_SHIFT + chipSelect);

#endif          // If brcmNAND Version >= 1.0

		nandSelect = BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK | csNandSelect;
		chip->ctrl_write(BCHP_NAND_CS_NAND_SELECT, nandSelect);
	}

#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_7_0
	/*NAND controller rev7 perform auto detect again when _AUTO_DEVICE_ID is set on receiving
	   read id cmd and update the nand config reg. CFE overwrite this config register in case hw
	   auto detect is wrong(such as MXIC 512Mb MX30LF1208AA) when system boot.Clear the AUTO Dev
	   Id bit to avoid incorrect config setting */
	nandSelect = chip->ctrl_read(BCHP_NAND_CS_NAND_SELECT);
	nandSelect &= ~BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK;
	chip->ctrl_write(BCHP_NAND_CS_NAND_SELECT, nandSelect);
#endif

	/* Send the command for reading device ID from controller */
	chip->ctrl_write(BCHP_NAND_CMD_START, OP_DEVICE_ID_READ);

	/* Wait for CTRL_Ready */
	brcmnand_wait(mtd, BRCMNAND_FL_READY, &status, 10000);

#endif  // if BrcmNAND Version >= 0.1


	*dev_id = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID);

	printk("%s: CS%1d: dev_id=%08x\n", __FUNCTION__, chipSelect, (unsigned int)*dev_id);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_0_1
	nandSelect = chip->ctrl_read(BCHP_NAND_CS_NAND_SELECT);
#endif

	nandConfig = chip->ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi]));

	printk("After: NandSelect=%08x, nandConfig=%08x\n", nandSelect, nandConfig);
}


#if (CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0 && CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_7_0)
/*
 * Type-1 ID string, called from brcmnand_probe with the following condition
 * if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE4) &&
 *	(brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE5))
 *
 * returns the updated nand_config register value.
 *
 * This routine will need to set chip->chipSize and chip->page_shift in order to compute
 * the address bytes in the NAND_CONFIG register.
 */
static uint32_t
decode_ID_type1(struct brcmnand_chip * chip,
		unsigned char brcmnand_maf_id, unsigned char brcmnand_dev_id, uint32_t idOptions, uint32_t nbrBlocks)
{
	uint32_t nand_config = chip->ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi])); // returned value


/* Read 5th ID byte if MLC type */
//if (chip->cellinfo)

/* THT SWLINUX 1459: Some new SLCs have 5th ID byte defined, not just MLC */
// if (brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE5)


	unsigned long devIdExt = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID_EXT);
	unsigned char devId5thByte = (devIdExt & 0xff000000) >> 24;
	unsigned int nbrPlanes = 0;
	unsigned int planeSizeMB = 0, chipSizeMB, nandConfigChipSize;
	unsigned char devId4thdByte =  (chip->device_id  & 0xff);
	unsigned int pageSize = 0, pageSizeBits = 0;
	unsigned int blockSize = 0, blockSizeBits = 0;

	//unsigned int oobSize;

	PRINTK("%s: mafID=%02x, devID=%02x, ID4=%02x, ID5=%02x\n",
	       __FUNCTION__, brcmnand_maf_id, brcmnand_dev_id,
	       devId4thdByte, devId5thByte);

	// if (brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE4)

/*---------------- 4th ID byte: page size, block size and OOB size ---------------- */
	switch (brcmnand_maf_id) {
	case FLASHTYPE_SAMSUNG:
	case FLASHTYPE_HYNIX:
	case FLASHTYPE_TOSHIBA:
	case FLASHTYPE_MICRON:
		pageSize = 1024 << (devId4thdByte & SAMSUNG_4THID_PAGESIZE_MASK);
		blockSize = (64 * 1024) << ((devId4thdByte & SAMSUNG_4THID_BLKSIZE_MASK) >> 4);
		//oobSize = devId4thdByte & SAMSUNG_4THID_OOBSIZE_MASK ? 16 : 8;
		chip->page_shift = ffs(pageSize) - 1;
		chip->erase_shift = ffs(blockSize) - 1;


		PRINTK("Updating Config Reg: Block & Page Size: B4: %08x, blockSize=%08x, pageSize=%d\n",
		       nand_config, blockSize, pageSize);
		/* Update Config Register: Block Size */
		switch (blockSize) {
		case 512 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB;
			break;
		case 128 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB;
			break;
		case 16 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB;
			break;
		case 256 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB;
			break;
		}
		nand_config &= ~(BCHP_NAND_CONFIG_BLOCK_SIZE_MASK);
		nand_config |= (blockSizeBits << BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT);

		/* Update Config Register: Page Size */
		switch (pageSize) {
		case 512:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512;
			break;
		case 2048:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB;
			break;
		case 4096:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB;
			break;
		}
		nand_config &= ~(BCHP_NAND_CONFIG_PAGE_SIZE_MASK);
		nand_config |= (pageSizeBits << BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT);
		chip->ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);
		PRINTK("Updating Config Reg: Block & Page Size: After: %08x\n", nand_config);
		break;

	default:
		printk(KERN_ERR "4th ID Byte: Device requiring Controller V3.0 in database, but not handled\n");
		//BUG();
	}


	if (nbrBlocks > 0) {
		chip->chipSize = ((uint64_t)nbrBlocks) << chip->erase_shift;
		PRINTK("nbrBlocks=%d, blockSize=%d, blkShift=%d, chip Size = %llx\n", nbrBlocks, blockSize, chip->erase_shift, chip->chipSize);
		chipSizeMB = (uint32_t)(chip->chipSize >> 20);
	}else  { /* Use 5th byte plane size & nbrPlanes to compute chip size */
/*---------------- 5th ID byte ------------------------- */
		switch (brcmnand_maf_id) {
		case FLASHTYPE_SAMSUNG:
		case FLASHTYPE_HYNIX:
		case FLASHTYPE_TOSHIBA:
		case FLASHTYPE_MICRON:
			PRINTK("5th ID byte = %02x, extID = %08lx\n", devId5thByte, devIdExt);

			switch (devId5thByte & SAMSUNG_5THID_NRPLANE_MASK) {
			case SAMSUNG_5THID_NRPLANE_1:
				nbrPlanes = 1;
				break;
			case SAMSUNG_5THID_NRPLANE_2:
				nbrPlanes = 2;
				break;
			case SAMSUNG_5THID_NRPLANE_4:
				nbrPlanes = 4;
				break;
			case SAMSUNG_5THID_NRPLANE_8:
				nbrPlanes = 8;
				break;
			}
			PRINTK("nbrPlanes = %d\n", nbrPlanes);
		}

		switch (brcmnand_maf_id) {
		case FLASHTYPE_SAMSUNG:
		case FLASHTYPE_MICRON:
			if (idOptions & BRCMNAND_ID_HAS_MICRON_M68A) {
				planeSizeMB = 128;
			}else  {
				/* Samsung Plane Size
				   #define SAMSUNG_5THID_PLANESZ_64Mb	0x00
				   #define SAMSUNG_5THID_PLANESZ_128Mb	0x10
				   #define SAMSUNG_5THID_PLANESZ_256Mb	0x20
				   #define SAMSUNG_5THID_PLANESZ_512Mb	0x30
				   #define SAMSUNG_5THID_PLANESZ_1Gb	0x40
				   #define SAMSUNG_5THID_PLANESZ_2Gb	0x50
				   #define SAMSUNG_5THID_PLANESZ_4Gb	0x60
				   #define SAMSUNG_5THID_PLANESZ_8Gb	0x70
				 */
				// planeSize starts at (64Mb/8) = 8MB;
				planeSizeMB = 8 << ((devId5thByte & SAMSUNG_5THID_PLANESZ_MASK) >> 4);
			}
			PRINTK("planSizeMB = %dMB\n", planeSizeMB);
			break;

		case FLASHTYPE_HYNIX:
			if (idOptions & BRCMNAND_ID_HYNIX_LEGACY) {
				// planeSize starts at (64Mb/8) = 8MB, same as Samsung
				planeSizeMB = 8 << ((devId5thByte & HYNIX_5THID_LEG_PLANESZ_MASK) >> 4);
			}else  {
				/* Hynix Plane Size, Type 2
				   #define HYNIX_5THID_PLANESZ_MASK	0x70
				   #define HYNIX_5THID_PLANESZ_512Mb	0x00
				   #define HYNIX_5THID_PLANESZ_1Gb		0x10
				   #define HYNIX_5THID_PLANESZ_2Gb		0x20
				   #define HYNIX_5THID_PLANESZ_4Gb		0x30
				   #define HYNIX_5THID_PLANESZ_8Gb		0x40
				   #define HYNIX_5THID_PLANESZ_RSVD1	0x50
				   #define HYNIX_5THID_PLANESZ_RSVD2	0x60
				   #define HYNIX_5THID_PLANESZ_RSVD3	0x70
				 */
				// planeSize starts at (512Mb/8) = 64MB;
				planeSizeMB = 64 << ((devId5thByte & SAMSUNG_5THID_PLANESZ_MASK) >> 4);
			}
			break;

		case FLASHTYPE_TOSHIBA:
			/* No Plane Size defined */
			// THT Nothing to do, size is hardcoded in chip array.
			// planeSizeMB = 64; /* hard-coded for TC58NVG0S3ETA00 */
			break;

			/* TBD Add other mfg ID here */

		} /* End 5th ID byte */

		chipSizeMB = planeSizeMB * nbrPlanes;
		chip->chipSize = ((uint64_t)chipSizeMB) << 20;
		PRINTK("planeSizeMB = %d, chipSizeMB=%d,0x%04x, planeSizeMask=%08x\n", planeSizeMB, chipSizeMB, chipSizeMB, devId5thByte & SAMSUNG_5THID_PLANESZ_MASK);
	}

	/* NAND Config register starts at 4MB for chip size */
	nandConfigChipSize = ffs(chipSizeMB >> 2) - 1;

	PRINTK("nandConfigChipSize = %04x\n", nandConfigChipSize);
	/* Correct chip Size accordingly, bit 24-27 */
	nand_config &= ~(BCHP_NAND_CONFIG_DEVICE_SIZE_MASK);
	nand_config |= (nandConfigChipSize << BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT);

	return nand_config;
}


/*
 * Type-2 ID string, called from brcmnand_probe with the following condition
 * if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_BYTES_TYPE2) ==
 *				BRCMNAND_ID_EXT_BYTES_TYPE2)
 *
 * returns the updated nand_config register value.
 *
 * This routine will need to set chip->chipSize and chip->page_shift in order to compute
 * the address bytes in the NAND_CONFIG register.
 */
static uint32_t
decode_ID_type2(struct brcmnand_chip * chip,
		unsigned char brcmnand_maf_id, unsigned char brcmnand_dev_id, uint32_t nbrBlocks,
		uint32* pEccLevel, uint32* pSectorSize)
{
	uint32_t nand_config = chip->ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi])); // returned value
	unsigned char devId4thdByte =  (chip->device_id  & 0xff);
	unsigned int pageSize = 0, pageSizeBits = 0;
	unsigned int blockSize = 0, blockSizeBits = 0;
	//unsigned int oobSize;
	unsigned int oobSize, oobSizePerPage = 0;
	uint32_t chipSizeMB, nandConfigChipSize;
	uint32_t devIdExt, eccShift, reqEcc;
	unsigned char devId5thByte;

	PRINTK("%s: mafID=%02x, devID=%02x, ID4=%02x\n",
	       __FUNCTION__, brcmnand_maf_id, brcmnand_dev_id,
	       devId4thdByte);

	/*---------------- 4th ID byte: page size, block size and OOB size ---------------- */
	switch (brcmnand_maf_id) {
	case FLASHTYPE_SAMSUNG:
	case FLASHTYPE_HYNIX:
		pageSize = 2048 << (devId4thdByte & SAMSUNG2_4THID_PAGESIZE_MASK);
		/* **FIXME**, when Samsung use the Reserved bits */
		blockSize = (128 * 1024) << ((devId4thdByte & SAMSUNG2_4THID_BLKSIZE_MASK) >> 4);
		chip->blockSize = blockSize;
		switch (devId4thdByte & SAMSUNG2_4THID_OOBSIZE_MASK) {
		case SAMSUNG2_4THID_OOBSIZE_PERPAGE_128:
			oobSizePerPage = 128;
			break;

		case SAMSUNG2_4THID_OOBSIZE_PERPAGE_218:
			oobSizePerPage = 218;
			break;

		case SAMSUNG2_4THID_OOBSIZE_PERPAGE_400:  /* 16 per 512B */
			oobSizePerPage = 400;
			break;

		case SAMSUNG2_4THID_OOBSIZE_PERPAGE_436: /* 27.5 per 512B */
			oobSizePerPage = 436;
			break;
		}
		oobSize = oobSizePerPage / (pageSize / 512);
		// Record it here, but will check it when we know about the ECC level.
		chip->eccOobSize = oobSize;
		PRINTK("oobSizePerPage=%d, eccOobSize=%d, pageSize=%u, blockSize=%u\n",
		       oobSizePerPage, chip->eccOobSize, pageSize, blockSize);
		PRINTK("Updating Config Reg T2: Block & Page Size: B4: %08x\n", nand_config);

		chip->page_shift = ffs(pageSize) - 1;

		/* Update Config Register: Block Size */
		switch (blockSize) {
		case 512 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB;
			break;
		case 128 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB;
			break;
		case 16 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB;
			break;
		case 256 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB;
			break;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		case 1024 * 1024:
			blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_1024KB;
			break;
#endif
		}
		PRINTK("%s:  blockSizeBits=%08x, NANDCONFIG B4=%08x\n", __FUNCTION__, blockSizeBits, nand_config);
		nand_config &= ~(BCHP_NAND_CONFIG_BLOCK_SIZE_MASK);
		nand_config |= (blockSizeBits << BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT);
		PRINTK("%s:   NANDCONFIG After=%08x\n", __FUNCTION__,  nand_config);

		/* Update Config Register: Page Size */
		switch (pageSize) {
		case 512:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512;
			break;
		case 2048:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB;
			break;
		case 4096:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB;
			break;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_4
		case 8192:
			pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB;
			break;
#endif
		}
		PRINTK("%s:  pageSizeBits=%08x, NANDCONFIG B4=%08x\n", __FUNCTION__, pageSizeBits, nand_config);
		nand_config &= ~(BCHP_NAND_CONFIG_PAGE_SIZE_MASK);
		nand_config |= (pageSizeBits << BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT);


		break;

	default:
		printk(KERN_ERR "4th ID Byte: Device requiring Controller V3.0 in database, but not handled\n");
		//BUG();
	}

#if 1
/*
 * Now we hard-code the flash size in the ID array, because Samsung type 2 flashes are MLC flashes,
 * so tend to be used on CSn, n != 0, and thus the CFE may not configure it properly
 */
	PRINTK("nbrBlocks=%d, blockSize=%d\n",  nbrBlocks, chip->blockSize);
	chip->chipSize = ((uint64_t)nbrBlocks) * chip->blockSize;
	chipSizeMB = (uint32_t)(chip->chipSize >> 20);
	nandConfigChipSize = ffs(chipSizeMB >> 2) - 1;

	PRINTK("chipSize=%dMB, nandConfigChipSize = %04x\n", chipSizeMB, nandConfigChipSize);
	/* Encode chip Size accordingly, bit 24-27 */
	nand_config &= ~(BCHP_NAND_CONFIG_DEVICE_SIZE_MASK);
	nand_config |= (nandConfigChipSize << BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT);

	PRINTK("Updating Config Reg on CS%1d:  %08x\n", chip->ctrl->CS[chip->csi], nand_config);
	chip->ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);

/*---------------- 5th ID byte: ECC level and plane number ---------------- */
	devIdExt = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID_EXT);
	devId5thByte = (devIdExt & 0xff000000) >> 24;
	reqEcc = (devId5thByte & SAMSUNG2_5THID_ECCLVL_MASK);
	switch (reqEcc) {
	case SAMSUNG2_5THID_ECCLVL_1BIT:        /* 0x00 */
	case SAMSUNG2_5THID_ECCLVL_2BIT:        /*	 0x10 */
	case SAMSUNG2_5THID_ECCLVL_4BIT:        /*	 0x20 */
	case SAMSUNG2_5THID_ECCLVL_8BIT:        /*	 0x30 */
	case SAMSUNG2_5THID_ECCLVL_16BIT:       /* 0x40 */
		eccShift = reqEcc >> 4;
		*pEccLevel =  1 << eccShift;
		*pSectorSize = 512;
		break;

	case SAMSUNG2_5THID_ECCLVL_24BIT_1KB: /* 0x50 */
		*pEccLevel = 24;
		*pSectorSize = 1024;
	}

	PRINTK("Required ECC level = %ld, devIdExt=%08x, eccShift=%02x, sector Size=%ld\n",
	       *pEccLevel, devIdExt, eccShift, *pSectorSize);

#else
	/* For type 2, ID bytes do not yield flash Size, but CONFIG registers have that info */

	chipSizeShift = (nand_config & BCHP_NAND_CONFIG_DEVICE_SIZE_MASK) >> BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT;
	chip->chipSize = 4ULL << (20 + chipSizeShift);

#endif

	return nand_config;
}


/*
 * Type-2 ID string, called from brcmnand_probe with the following condition
 * if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_BYTES_TYPE2) ==
 *				BRCMNAND_ID_EXT_BYTES_TYPE2)
 *
 * returns the updated nand_config register value.
 *
 * This routine will need to set chip->chipSize and chip->page_shift in order to compute
 * the address bytes in the NAND_CONFIG register.
 */
static uint32_t
decode_ID_M61A(struct brcmnand_chip * chip,
	       unsigned char brcmnand_maf_id, unsigned char brcmnand_dev_id)
{
	uint32_t nand_config = chip->ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi])); // returned value
	unsigned char devId4thdByte =  (chip->device_id  & 0xff);
	unsigned int pageSize = 0, pageSizeBits = 0;
	unsigned int blockSize = 0, blockSizeBits = 0;
	//unsigned int oobSize;
	unsigned int oobSize, oobSizePerPage = 0;
	uint32_t pagesPerBlock, pagesPerBlockBits;
	unsigned long devIdExt = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID_EXT);
	unsigned char devId5thByte = (devIdExt & 0xff000000) >> 24;
	unsigned int nbrPlanes = 0;
	unsigned int chipSizeMB, nandConfigChipSize;
	unsigned int blkPerLun, nbrBlksBits;

	PRINTK("%s: mafID=%02x, devID=%02x, ID4=%02x\n",
	       __FUNCTION__, brcmnand_maf_id, brcmnand_dev_id,
	       devId4thdByte);

	/* Byte2: Voltage and size are not reliable */

	/* 3rd ID byte, same as Samsung Type 1 */

	/*---------------- 4th ID byte: page size, block size and OOB size ---------------- */
	pageSize = 1024 << (devId4thdByte & SAMSUNG_4THID_PAGESIZE_MASK);
	chip->page_shift = ffs(pageSize) - 1;

	/* Block Size */
	pagesPerBlockBits = (devId4thdByte & MICRON_M61A_4THID_PGPBLK_MASK) >> 4;
	pagesPerBlock = 32 << pagesPerBlockBits;
	blockSize = pagesPerBlock * pageSize;


	switch (devId4thdByte & MICRON_M61A_4THID_OOBSIZE_MASK) {
	case MICRON_M61A_4THID_OOBSIZE_28B:
		oobSize = 27; /* Use only 27 to conform to other vendors */
		oobSizePerPage = oobSize * (pageSize / 512);
		break;

	}


	// Record it here, but will check it when we know about the ECC level.
	chip->eccOobSize = oobSize;
	PRINTK("oobSizePerPage=%d, eccOobSize=%d, pageSize=%u, blockSize=%u\n",
	       oobSizePerPage, chip->eccOobSize, pageSize, blockSize);

	PRINTK("Updating Config Reg M61A: Block & Page Size: B4: %08x\n", nand_config);

	/* Update Config Register: Block Size */
	switch (blockSize) {
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
	case 1024 * 1024:
		blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_1024KB;
		break;
#else
	case 1024 * 1024:
		/* For version 3.x controller, we don't have a bit defined for this */
		/* FALLTHROUGH */
#endif
	case 512 * 1024:
		blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_512KB;
		break;
	case 128 * 1024:
		blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_128KB;
		break;
	case 16 * 1024:
		blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_16KB;
		break;
	case 256 * 1024:
		blockSizeBits = BCHP_NAND_CONFIG_BLOCK_SIZE_BK_SIZE_256KB;
		break;

	}
	/* Record Block Size, since we can't rely on NAND_CONFIG */
	chip->blockSize = blockSize;

	PRINTK("%s:  blockSizeBits=%08x, NANDCONFIG B4=%08x\n", __FUNCTION__, blockSizeBits, nand_config);
	nand_config &= ~(BCHP_NAND_CONFIG_BLOCK_SIZE_MASK);
	nand_config |= (blockSizeBits << BCHP_NAND_CONFIG_BLOCK_SIZE_SHIFT);
	PRINTK("%s:   NANDCONFIG After=%08x\n", __FUNCTION__,  nand_config);

	/* Update Config Register: Page Size */
	switch (pageSize) {
	case 512:
		pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_512;
		break;
	case 2048:
		pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_2KB;
		break;
	case 4096:
		pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_4KB;
		break;
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_4
	case 8192:
		pageSizeBits = BCHP_NAND_CONFIG_PAGE_SIZE_PG_SIZE_8KB;
		break;
#endif

	}


/*---------------- 5th ID byte ------------------------- */

	PRINTK("5th ID byte = %02x, extID = %08lx\n", devId5thByte, devIdExt);


	nbrPlanes = 1 << (devId5thByte & MICRON_M61A_5THID_PLN_PER_LUN_MASK);
	nbrBlksBits = (devId5thByte & MICRON_M61A_5THID_BLK_PER_LUN_MASK) >> 2;
	blkPerLun = 1024 << nbrBlksBits;

	chipSizeMB = (blkPerLun * blockSize) >> 20;
	PRINTK("chipSizeMB=%d,0x%04x, planeSizeMask=%08x\n",  chipSizeMB, chipSizeMB, devId5thByte & SAMSUNG_5THID_PLANESZ_MASK);
	/* NAND Config register starts at 4MB for chip size */
	nandConfigChipSize = ffs(chipSizeMB >> 2) - 1;

	chip->chipSize = ((uint64_t)chipSizeMB) << 20;

	PRINTK("nandConfigChipSize = %04x\n", nandConfigChipSize);
	/* Correct chip Size accordingly, bit 24-27 */
	nand_config &= ~(BCHP_NAND_CONFIG_DEVICE_SIZE_MASK);
	nand_config |= (nandConfigChipSize << BCHP_NAND_CONFIG_DEVICE_SIZE_SHIFT);
	chip->ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);


	PRINTK("%s:  pageSizeBits=%08x, NANDCONFIG B4=%08x\n", __FUNCTION__, pageSizeBits, nand_config);
	nand_config &= ~(BCHP_NAND_CONFIG_PAGE_SIZE_MASK);
	nand_config |= (pageSizeBits << BCHP_NAND_CONFIG_PAGE_SIZE_SHIFT);
	chip->ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);
	PRINTK("Updating Config Reg on CS%1d: Block & Page Size: After: %08x\n", chip->ctrl->CS[chip->csi], nand_config);

	return nand_config;
}
#endif


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0

/* Returns the 32bit integer at the aligned offset */
static inline uint32_t brcmnand_flashcache_read(void* pDest, uint32_t ofs, int size)
{
	uint32_t aligned_ofs = ofs & 0xFFFFFFFC;
	volatile uint32_t* p32FCache = (volatile uint32_t*)BVIRTADDR(BCHP_NAND_FLASH_CACHEi_ARRAY_BASE);
	uintptr_t pReg = (BCHP_NAND_FLASH_CACHEi_ARRAY_BASE + aligned_ofs);
	volatile u_char* p8FCache = (volatile u_char*)p32FCache;
	uint32_t u32;
	u_char* p8 = (u_char*)&u32;

	if ((size + ofs) > (aligned_ofs + 4)) {
		printk("%s: Cannot read across DW boundary ofs=%d, size=%d\n", __FUNCTION__, ofs, size);
		return 0;
	}

	u32 = be32_to_cpu((uint32_t)BDEV_RD(pReg));
	if (pDest) {
		memcpy(pDest, &p8[ofs - aligned_ofs], size);
	}

	if (gdebug > 3) {
		printk("%s: OFS=%d, EBIAddr=%p val=%08x, p8=%02x%02x%02x%02x\n", __FUNCTION__,
		       (unsigned int)aligned_ofs, (void*)pReg, (uint32_t)BDEV_RD(pReg),
		       p8FCache[aligned_ofs], p8FCache[aligned_ofs + 1], p8FCache[aligned_ofs + 2], p8FCache[aligned_ofs + 3]);
	}

	return ((uint32_t)BDEV_RD(pReg));
}


static void __maybe_unused
debug_print_flashcache(struct mtd_info *mtd)
{
	struct brcmnand_chip * chip = mtd->priv;
	static uint32_t fbuffer[128];
	volatile uint32_t* fcache = (volatile uint32_t*)chip->vbase;
	int i;
	int saveDebug = gdebug;

	gdebug = 0;

	// Copy the data buffer

	for (i = 0; i < 128; i++) {
		fbuffer[i] = (uint32_t)(fcache[i]);
	}

	printk("Flash Cache:\n");
	print_databuf((u_char*)&fbuffer[0], 512);

	brcmnand_post_mortem_dump(mtd, 0);
	gdebug = saveDebug;
}

#if 0
/*
 * Return 0 for ready, TIMEOUT for error
 */
static int brcmnand_wait_for_cache_ready(struct brcmnand_chip* chip)
{
	unsigned long timeout;
	uint32_t ready;

	//udelay(100000); /* 100 ms */
	/* The 20 msec is enough */
	timeout = jiffies + msecs_to_jiffies(3000);
	while (time_before(jiffies, timeout)) {
		//PLATFORM_IOFLUSH_WAR();
		ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);

		if ((ready & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK)
		    && (ready & BCHP_NAND_INTFC_STATUS_CACHE_VALID_MASK)) {

			return BRCMNAND_SUCCESS;

		}
		//if ( !in_interrupt())
		//	cond_resched();
		//else
		//	udelay(10000);
	}

	if (gdebug > 3 ) {
		printk("<--%s: ret = TIMEOUT\n", __FUNCTION__);
		print_nand_ctrl_regs();
	}
	return BRCMNAND_TIMED_OUT; // TimeOut
}
#endif

typedef enum {
	BRCMNAND_READY,
	BRCMNAND_CTRL_BUSY,
	BRCMNAND_CTRL_READY,
	BRCMNAND_CACHE_VALID
} brcmnand_ctrl_state_t;

static int brcmnand_monitor_intfc(struct mtd_info *mtd, brcmnand_ctrl_state_t waitfor, uint32_t* pStatus)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned long timeout;
	uint32_t ready;
	brcmnand_ctrl_state_t state = BRCMNAND_READY;
	int ret =  -ETIMEDOUT;
	//unsigned long irqflags;

// Dont want printk to cause missing a transition of INTFC
	int save_debug = gdebug;
	uint32_t prev_ready;

//gdebug = 0;

	//local_irq_save(irqflags);
	prev_ready = *pStatus = ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);
	timeout = jiffies + msecs_to_jiffies(2000); // THT: 1000 msec, for now
	while (time_before(jiffies, timeout) ) {
		switch (state) {
		case BRCMNAND_READY: /* Wait for ctrl-busy */
			if (!(ready & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK)) {
				state = BRCMNAND_CTRL_BUSY;
				if (save_debug) printk("%s: waitfor=%d, Got ctrl-busy, intfc=%08x\n", __FUNCTION__, waitfor, ready);
			}
			/* If we cgot cache valid, skip ctrl-busy */
			if ((waitfor == BRCMNAND_CACHE_VALID)
			    && (ready & BCHP_NAND_INTFC_STATUS_CACHE_VALID_MASK)) {
				state = BRCMNAND_CTRL_READY;
				ret = BRCMNAND_SUCCESS;
				goto exit_monitor;
			}
			break;
		case BRCMNAND_CTRL_BUSY: /* Wait for ctrl-ready */
			if ((waitfor == BRCMNAND_CTRL_READY) &&
			    (ready & BCHP_NAND_INTFC_STATUS_CTLR_READY_MASK)) {
				state = BRCMNAND_CTRL_READY;
				ret = BRCMNAND_SUCCESS;
				goto exit_monitor;
			}else if ((waitfor == BRCMNAND_CACHE_VALID)
				  && (ready & BCHP_NAND_INTFC_STATUS_CACHE_VALID_MASK)) {
				state = BRCMNAND_CTRL_READY;
				ret = BRCMNAND_SUCCESS;
				goto exit_monitor;
			}
			break;
		case BRCMNAND_CTRL_READY:
			if (waitfor == BRCMNAND_CTRL_READY) {
				ret = BRCMNAND_SUCCESS;
				goto exit_monitor;
			}
			break;
		case BRCMNAND_CACHE_VALID:
			if (waitfor == BRCMNAND_CACHE_VALID) {
				ret = BRCMNAND_SUCCESS;
				goto exit_monitor;
			}
			break;
		}
		if (prev_ready != ready) printk("prev_ready=%08x, ready=%08x\n", prev_ready, ready);
		prev_ready = ready;
		*pStatus = ready = chip->ctrl_read(BCHP_NAND_INTFC_STATUS);
	}

 exit_monitor:
	gdebug = save_debug;

	//local_irq_restore(irqflags);

	if (save_debug) printk("%s: waitfor=%d, return %d, intfc=%08x\n", __FUNCTION__, waitfor, ret, *pStatus);
	return ret;
}




/*
 * Decode flash geometry using ONFI
 * returns 1 on success, 0 on failure
 */
static int
brcmnand_ONFI_decode(struct mtd_info *mtd, unsigned int chipSelect,
		     uint32_t* outp_pageSize, uint16_t* outp_oobSize, int* outp_reqEcc, int* outp_codeWorkSize)
{
	int skipDecodeID = 0; /* Returned value */
	struct brcmnand_chip * chip = mtd->priv;
	uint32_t u32;
	uint8_t eccLevel = 0;
	uint32_t nand_config0, nand_config;
	uint32_t acc;
	int status, retries;
	uint32_t nand_select;
	int ret;
	uint32_t timing2;
	uint8_t nbrParamPages, nbrBitsPerCell;
	uint32_t extParamOffset, extParamFCacheOffset;

//gdebug=4;
	if (gdebug > 3) printk("-->%s, chipSelect=%d\n", __FUNCTION__, chipSelect);

#if 1
	/* Skip ONFI if on CS0, Boot loader already done that */
	if (chipSelect == 0) { // Do not re-initialize when on CS0, Bootloader already done that
		return 0;
	}
#else
	/*
	 * Even though we cannot boot on CS0 on 7422a0, we still need to go through the
	 * ONFI decode procedure, in order to initialize internal data structure
	 */
	if (chipSelect == 0) { // Do not re-initialize when on CS0, Bootloader already done that
		//TBD
		return 0;
	}
#endif

	chip->vbase = (void*)BVIRTADDR(BCHP_NAND_FLASH_CACHEi_ARRAY_BASE);

#if 1
	if (chipSelect != 0) {
		uint32_t nand_acc;

		if (gNandConfig[chip->csi] != 0) {
			nand_config = gNandConfig[chip->csi];
			chip->ctrl_write(bchp_nand_config(chipSelect), nand_config);

			if (chip->csi == 0) /* No NAND on CS0 */
				chip->ctrl_write(BCHP_NAND_CONFIG, nand_config);
		}

		if (0 != gAccControl[chip->csi]) {
			nand_acc = gAccControl[chip->csi];
			chip->ctrl_write(bchp_nand_acc_control(chipSelect), nand_acc);
			if (chip->csi == 0)
				chip->ctrl_write(BCHP_NAND_ACC_CONTROL, nand_acc);
		}
	}

#endif




	retries = 1;
	while (retries > 0) {

		PRINTK("************  Retries = %d\n", retries);

#if 1
		nand_config0 = brcmnand_ctrl_read(bchp_nand_config(chipSelect));


#endif


		/* Setup READ ID and AUTOCONFIG */
		nand_select = chip->ctrl_read(BCHP_NAND_CS_NAND_SELECT);
		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect <<
				 BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);
		nand_select |= BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK;
		nand_select &= ~(1 << (chipSelect + BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_SHIFT));
		chip->ctrl_write(BCHP_NAND_CS_NAND_SELECT, nand_select);

		//udelay(10000); /* 10 ms */


		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect <<
				 BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);

		chip->ctrl_write(BCHP_NAND_CMD_START,
				 BCHP_NAND_CMD_START_OPCODE_NULL << BCHP_NAND_CMD_START_OPCODE_SHIFT);

		/* Wait for controller busy then ready */
		ret = brcmnand_monitor_intfc(mtd, BRCMNAND_CTRL_READY, &status);



		// udelay(1000);  // 1 sec

		// Change timing to conform to ONFI
		timing2 = chip->ctrl_read(bchp_nand_timing2(chipSelect));
		PRINTK("Old timing2 value=%08x\n", timing2);
		timing2 &= ~(BCHP_NAND_TIMING_2_tWHR_MASK);
		timing2 |= 11 << BCHP_NAND_TIMING_2_tWHR_SHIFT;
		PRINTK("New timing2 value=%08x\n", timing2);
		chip->ctrl_write(bchp_nand_timing2(chipSelect), timing2);


		nand_config = brcmnand_ctrl_read(bchp_nand_config(chipSelect));
		PRINTK("B4 status READ, nand_config0=%08x, nand_config1=%08x, ret=%d\n", nand_config0, nand_config, ret);



		nand_config = brcmnand_ctrl_read(bchp_nand_config(chipSelect));
		PRINTK("B4 PARAM READ, nand_config0=%08x, nand_config1=%08x, ret=%d\n", nand_config0, nand_config, ret);


		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect <<
				 BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);

		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PARAMETER_READ);

		/* Wait for controller busy then cache-valid */
		ret = brcmnand_monitor_intfc(mtd, BRCMNAND_CACHE_VALID, &status);


		/*
		 * Verify ONFI capability
		 */
		u32 = brcmnand_flashcache_read(NULL, ONFI_RDPARAM_SIGNATURE_OFS, sizeof(u32));


		if (u32 == ONFI_SIGNATURE) {
			printk("%s: Found ONFI signature.  Looking for %08x found %08x, ret=%d\n",
			       __FUNCTION__, ONFI_SIGNATURE, u32, ret);

			break;
		}


		retries--;

		/* Flash Reset */
		brcmnand_wait(mtd, BRCMNAND_FL_READING, &status, 10000);
		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, 0);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, chipSelect <<
				 BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);
		chip->ctrl_write(BCHP_NAND_CMD_START,  OP_FLASH_RESET);

		brcmnand_wait(mtd, BRCMNAND_FL_READING, &status, 10000);

	}


	PRINTK("ONFI sig=%08x\n", *((volatile unsigned int*)chip->vbase));


	/*
	 * Verify ONFI capability
	 */
	u32 = brcmnand_flashcache_read(NULL, ONFI_RDPARAM_SIGNATURE_OFS, sizeof(u32));


	if (u32 != ONFI_SIGNATURE) {
		printk("%s: Cannot find ONFI signature.  Looking for %08x found %08x\n",
		       __FUNCTION__, ONFI_SIGNATURE, u32);

		// debug_print_flashcache(mtd);
		skipDecodeID = 0;
		goto onfi_exit;
	}


	// ONFI read-parameter was successful
	nand_config = brcmnand_ctrl_read(bchp_nand_config(chipSelect));

	if (nand_config != nand_config0) {
		printk("Original nand_config=%08x, ONFI nand_config=%08x\n",
		       nand_config0, nand_config);
	}

	/* Page Size */
	u32 = brcmnand_flashcache_read(outp_pageSize, ONFI_RDPARAM_PAGESIZE_OFS, sizeof(u32));


	/* OOB Size */
	u32 = brcmnand_flashcache_read(outp_oobSize, ONFI_RDPARAM_OOBSIZE_OFS, sizeof(*outp_oobSize));
	//*outp_oobSize = be16_to_cpu(*outp_oobSize);
	PRINTK("oobSize = %d, u32=%08x\n", *outp_oobSize, u32);

	/* MLC or SLC */
	u32 = brcmnand_flashcache_read(&nbrBitsPerCell, ONFI_NBR_BITS_PER_CELL_OFS, sizeof(nbrBitsPerCell));
	PRINTK("nbrBitsPerCell = %d, u32=%08x\n", nbrBitsPerCell, u32);

	/* Required ECC level */
	u32 = brcmnand_flashcache_read(&eccLevel, ONFI_RDPARAM_ECC_LEVEL_OFS, sizeof(eccLevel));

	PRINTK("EccLevel = [%08x], %02x, pageSize=%d, oobSize=%d\n", u32, eccLevel, *outp_pageSize, *outp_oobSize);

	if (eccLevel != 0xFF) { /* Codework is 512B */
		*outp_reqEcc = eccLevel;
		*outp_codeWorkSize = 512;
		skipDecodeID = 1;
	}else  { /* Codework is NOT 512B */
		//int offset = 512;
		uint32_t extParamSig = 0;

		/* First find out how many param pages there are */
		(void)brcmnand_flashcache_read(&nbrParamPages, ONFI_NBR_PARAM_PAGE_OFS, sizeof(nbrParamPages));


		extParamOffset = (256 * nbrParamPages);
		extParamFCacheOffset = extParamOffset & ~(512 - 1); // ALign on 512B
		PRINTK("nbrParamPages = %d, offset=%d, extCacheOffset=%d\n", nbrParamPages, extParamOffset, extParamFCacheOffset);

//gdebug=4;

		/* Turn off 1KB sector size */
		acc = chip->ctrl_read(bchp_nand_acc_control(chipSelect));
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		acc &= ~(BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_MASK);
#endif
		//acc &= ~(BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_MASK);
		//acc |= (*oobSize) << BCHP_NAND_ACC_CONTROL_CS1_SPARE_AREA_SIZE_SHIFT;
		chip->ctrl_write(bchp_nand_acc_control(chipSelect), acc);


		/* Bring in next 512B */
		chip->ctrl_write(BCHP_NAND_CMD_ADDRESS, extParamFCacheOffset);
		chip->ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS,
				 chipSelect << BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_SHIFT);

		chip->ctrl_write(BCHP_NAND_CMD_START, OP_PARAMETER_CHANGE_COL);

		/* Wait for controller busy then ready */
		ret = brcmnand_monitor_intfc(mtd, BRCMNAND_CACHE_VALID, &status);

		/*
		 * Verify EXT PARAM signature
		 * Need to adjust offset based on the number of Read-Param pages
		 */

		u32 = brcmnand_flashcache_read(NULL,
					       ONFI_EXTPARAM_SIG1_OFS - ONFI_EXTPARAM_OFS + (extParamOffset - extParamFCacheOffset), 2);
		extParamSig = (u32 & 0xFFFF) << 16;
		PRINTK("EPPS1: u32=%08x, eppsig=%08x\n", u32, extParamSig);
		u32 = brcmnand_flashcache_read(NULL,
					       ONFI_EXTPARAM_SIG2_OFS - ONFI_EXTPARAM_OFS + (extParamOffset - extParamFCacheOffset), 2);
		extParamSig |= (u32 >> 16);
		PRINTK("EPPS2: u32=%08x, eppsig=%08x\n", u32, extParamSig);

		if (ONFI_EXTPARAM_SIG != extParamSig) {
			printk("%s: EXT PARAM not found, looking for %08x, found %08x\n",
			       __FUNCTION__, ONFI_EXTPARAM_SIG, extParamSig);
			debug_print_flashcache(mtd);
			skipDecodeID = 0;
		}else  {
			uint8_t powerOf2 = 0;
			uint8_t eccLevel = 0;


			u32 = brcmnand_flashcache_read(&powerOf2,
						       ONFI_EXTPARAM_CODEWORK_OFS - ONFI_EXTPARAM_OFS + (extParamOffset - extParamFCacheOffset),
						       sizeof(powerOf2));

			//powerOf2 = (u32 & (0x00FF0000)) >> 16;
			*outp_codeWorkSize = 1 << powerOf2;
			PRINTK("codeWorkSize power = %d, codeWorkSize=%d, u32=%08x\n", powerOf2, *outp_codeWorkSize, u32);
			u32 = brcmnand_flashcache_read(&eccLevel,
						       ONFI_EXTPARAM_EXT_ECC_OFS - ONFI_EXTPARAM_OFS + (extParamOffset - extParamFCacheOffset),
						       sizeof(eccLevel));
			*outp_reqEcc = eccLevel;
			PRINTK("eccLevel=%d, u32=%08x\n", *outp_reqEcc, u32);
			skipDecodeID = 1;

		}

	}

	if (skipDecodeID) {
		printk("reqEcc=%d, codeWork=%d\n", *outp_reqEcc, *outp_codeWorkSize);
		brcmnand_set_acccontrol(chip, chipSelect,
					*outp_pageSize, *outp_oobSize, *outp_reqEcc, *outp_codeWorkSize, nbrBitsPerCell);
	}
//gdebug = 0;

 onfi_exit:

	//local_irq_restore(irqflags);

	return skipDecodeID;
}

#else
/* Non-ONFI chips */

#define brcmnand_ONFI_decode(...) (0)

#endif


/**
 * brcmnand_probe - [BrcmNAND Interface] Probe the BrcmNAND device
 * @param mtd		MTD device structure
 *
 * BrcmNAND detection method:
 *   Compare the the values from command with ones from register
 *
 * 8/13/08:
 * V3.0+: Add celltype probe for MLC
 */
static int brcmnand_probe(struct mtd_info *mtd, unsigned int chipSelect)
{
	struct brcmnand_chip * chip = mtd->priv;
	unsigned char brcmnand_maf_id, brcmnand_dev_id;
	uint32_t nand_config = 0;
	int version_id;
	//int density;
	int i = BRCMNAND_MAX_CHIPS + 1;
	int isONFI = 0;         /* Set when chips (flash & ctrl) are ONFI capable */
	int foundInIdTable = 0; /* Set when flash ID found in ID table */
	int skipIdLookup = 0;
	uint32_t __maybe_unused pageSize = 0;
	uint16_t __maybe_unused oobSize = 0;
	int __maybe_unused reqEcc = 0;
	uint32_t __maybe_unused codeWork = 0;


	/*
	 * Special treatment for Spansion OrNand chips which do not conform to standard ID
	 */

	chip->disableECC = 0;
	chip->cellinfo = 0;     // default to SLC, will read 3rd byte ID later for v3.0+ controller
	chip->eccOobSize = 16;  // Will fix it if we have a Type2 ID flash (from which we know the actual OOB size */


	isONFI = brcmnand_ONFI_decode(mtd, chipSelect,
				      &pageSize, &oobSize, &reqEcc, &codeWork);


#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_4_0
	if (isONFI) { /* ONFI capable */
		/* NAND CONFIG register already encoded by NAND controller */
		chip->eccSectorSize = codeWork;
		chip->eccOobSize = oobSize / (pageSize / BRCMNAND_FCACHE_SIZE);
		if (codeWork == BRCMNAND_FCACHE_SIZE) {
			chip->reqEccLevel = reqEcc;
		}else  {
			chip->reqEccLevel = (reqEcc * BRCMNAND_FCACHE_SIZE) / codeWork;
		}
		/* TBD Check for required ECC level here */
		nand_config = chip->ctrl_read(bchp_nand_config(chipSelect));
		i = BRCMNAND_ONFI_IDX;
	}
	/* Else fallback to Read ID */
	else
#endif
	{
		/* Read manufacturer and device IDs from Controller */
		brcmnand_read_id(mtd, chipSelect, &chip->device_id);

		if (chip->device_id == 0) {
			printk(KERN_ERR "NAND Flash not detected\n");
			return (-EINVAL);
		}

		brcmnand_maf_id = (chip->device_id >> 24) & 0xff;
		brcmnand_dev_id = (chip->device_id >> 16) & 0xff;

		/* Look up in our table for infos on device */
		for (i = 0; i < BRCMNAND_MAX_CHIPS; i++) {
			if (brcmnand_dev_id == brcmnand_chips[i].chipId
			    && brcmnand_maf_id == brcmnand_chips[i].mafId) {

				/* No ambiguity in ID#3,4,5 */
				if (brcmnand_chips[i].chipId345[0] == 0x0
				    && brcmnand_chips[i].chipId345[1] == 0x0
				    && brcmnand_chips[i].chipId345[2] == 0x0) {
					foundInIdTable = 1;
					break;
				}
				/* Must resolve ambiguity */
				else if (brcmnand_dev_id == brcmnand_chips[i + 1].chipId
					 && brcmnand_maf_id == brcmnand_chips[i + 1].mafId) {

					uint32_t extID;
					uint8_t id3, id4, id5;

					id3 = (chip->device_id >> 8) & 0xff;
					id4 = (chip->device_id & 0xff);

					extID = chip->ctrl_read(BCHP_NAND_FLASH_DEVICE_ID_EXT);
					id5 = (extID & 0xff000000) >> 24;

					if (brcmnand_chips[i].chipId345[0] == id3
					    && brcmnand_chips[i].chipId345[1] == id4
					    && brcmnand_chips[i].chipId345[2] == id5) {

						foundInIdTable = 1;
						break;
					}else if (brcmnand_chips[i + 1].chipId345[0] == id3
						  && brcmnand_chips[i + 1].chipId345[1] == id4
						  && brcmnand_chips[i + 1].chipId345[2] == id5) {

						i = i + 1;
						foundInIdTable = 1;
						break;
					}
					/* Else not match */
				}
			}
		}

		if (i >= BRCMNAND_MAX_CHIPS) {
#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_0_0
			printk(KERN_ERR "DevId %08x may not be supported\n", (unsigned int)chip->device_id);
			/* Because of the bug in the controller in the first version,
			 * if we can't identify the chip, we punt
			 */
			return (-EINVAL);
#else
			printk(KERN_WARNING "DevId %08x may not be supported.  Will use config info\n", (unsigned int)chip->device_id);
#endif
		}else  {
			// Record NOP if known
			chip->nop = brcmnand_chips[i].nop;
		}

		/*
		 * Check to see if the NAND chip requires any special controller version
		 */
		if (brcmnand_chips[i].ctrlVersion > CONFIG_MTD_BRCMNAND_VERSION) {
			printk(KERN_ERR "#########################################################\n");
			printk(KERN_ERR "DevId %s requires controller version %d or later, but STB is version %d\n",
			       brcmnand_chips[i].chipIdStr, brcmnand_chips[i].ctrlVersion, CONFIG_MTD_BRCMNAND_VERSION);
			printk(KERN_ERR "#########################################################\n");
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
			return (-EINVAL);
#endif
		}


		// If not on CS0 && config is passed as command line, use it and skip decoding ID.
		if (chip->csi != 0 && gNandConfig[chip->csi] != 0) {
			skipIdLookup = 1;
			nand_config = gNandConfig[chip->csi];
			brcmnand_ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);
		}else  {
			nand_config = brcmnand_ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi]));
		}

		/*------------- 3rd ID byte --------------------*/
#if !defined(CONFIG_BCM_KF_MTD_BCMNAND)
		if (!skipIdLookup && FLASHTYPE_SPANSION == brcmnand_maf_id) {
			unsigned char devId3rdByte =  (chip->device_id >> 8) & 0xff;

			switch (devId3rdByte) {
			case 0x04:
			case 0x00:
				/* ECC Needed, device with up to 2% bad blocks */
				break;

			case 0x01:
			case 0x03:
				/* ECC NOT Needed, device is 100% valid blocks */
				chip->disableECC = 1;
				break;
			}
			/* Correct erase Block Size to read 512K for all Spansion OrNand chips */
			nand_config &= ~(0x3 << 28);
			nand_config |= (0x3 << 28); // bit 29:28 = 3 ===> 512K erase block
			brcmnand_ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);
		}
		/* Else if NAND is found in suppported table */
		else if (foundInIdTable) {
#else
		if (foundInIdTable) {
#endif


#if CONFIG_MTD_BRCMNAND_VERSION == CONFIG_MTD_BRCMNAND_VERS_0_0
			// Workaround for bug in 7400A0 returning invalid config
			switch (i) {
			case 0:         /* SamSung NAND 1Gbit */
			case 1:         /* ST NAND 1Gbit */
			case 4:
			case 5:
				/* Large page, 128K erase block
				   PAGE_SIZE = 0x1 = 1b = PG_SIZE_2KB
				   BLOCK_SIZE = 0x1 = 01b = BK_SIZE_128KB
				   DEVICE_SIZE = 0x5 = 101b = DVC_SIZE_128MB
				   DEVICE_WIDTH = 0x0 = 0b = DVC_WIDTH_8
				   FUL_ADR_BYTES = 5 = 101b
				   COL_ADR_BYTES = 2 = 010b
				   BLK_ADR_BYTES = 3 = 011b
				 */
				nand_config &= ~0x30000000;
				nand_config |= 0x10000000;         // bit 29:28 = 1 ===> 128K erase block
				//nand_config = 0x55042200; //128MB, 0x55052300  for 256MB
				brcmnand_ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);

				break;

			case 2:
			case 3:
				/* Small page, 16K erase block
				   PAGE_SIZE = 0x0 = 0b = PG_SIZE_512B
				   BLOCK_SIZE = 0x0 = 0b = BK_SIZE_16KB
				   DEVICE_SIZE = 0x5 = 101b = DVC_SIZE_128MB
				   DEVICE_WIDTH = 0x0 = 0b = DVC_WIDTH_8
				   FUL_ADR_BYTES = 5 = 101b
				   COL_ADR_BYTES = 2 = 010b
				   BLK_ADR_BYTES = 3 = 011b
				 */
				nand_config &= ~0x70000000;
				brcmnand_ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);

				break;

			default:
				printk(KERN_ERR "%s: DevId %08x not supported\n", __FUNCTION__, (unsigned int)chip->device_id);
				BUG();
				break;
			}
/* NAND VERSION 7.1 use two config register, need to update all of decode_id_xxx function. But these special chips
   should already be supported in 7.1 and no manual id decoding is needed */
#elif (CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0 && CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_7_0)

			if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_BYTES) ==
			    BRCMNAND_ID_EXT_BYTES ||
			    (brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_BYTES_TYPE2) ==
			    BRCMNAND_ID_EXT_BYTES_TYPE2 ||
			    (brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_MICRON_M60A) ==
			    BRCMNAND_ID_EXT_MICRON_M60A ||
			    (brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_MICRON_M61A) ==
			    BRCMNAND_ID_EXT_MICRON_M61A
			    ) {
				unsigned char devId3rdByte =  (chip->device_id >> 8) & 0xff;

				chip->cellinfo = devId3rdByte & NAND_CI_CELLTYPE_MSK;

				/* Read 5th ID byte if MLC type */
				//if (chip->cellinfo)

				/* THT SWLINUX 1459: Some new SLCs have 5th ID byte defined, not just MLC */
				/* Type-1 ID string */
				if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE4) &&
				    (brcmnand_chips[i].idOptions & BRCMNAND_ID_HAS_BYTE5)) {
					nand_config = decode_ID_type1(chip, brcmnand_maf_id, brcmnand_dev_id,
								      brcmnand_chips[i].idOptions, brcmnand_chips[i].nbrBlocks);
				}
				/* Type-2 ID string */
				else if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_BYTES_TYPE2) ==
					 BRCMNAND_ID_EXT_BYTES_TYPE2) {
					brcmnand_chips[i].eccLevel = 0;
					nand_config = decode_ID_type2(chip, brcmnand_maf_id, brcmnand_dev_id,
								      brcmnand_chips[i].nbrBlocks,
								      &brcmnand_chips[i].eccLevel,
								      &brcmnand_chips[i].sectorSize);
				}else if ((brcmnand_chips[i].idOptions & BRCMNAND_ID_EXT_MICRON_M61A) ==
					  BRCMNAND_ID_EXT_MICRON_M61A) {
					nand_config = decode_ID_M61A(chip, brcmnand_maf_id, brcmnand_dev_id);
				}

				if (!skipIdLookup) {

					/* Make sure that ColAddrBytes bits are correct */
					nand_config = brcmnand_compute_adr_bytes(chip, nand_config);

					chip->ctrl_write(bchp_nand_config(chip->ctrl->CS[chip->csi]), nand_config);

					PRINTK("%s: NAND_CONFIG=%08x\n", __FUNCTION__, nand_config);
				}


			}

			/* Else no 3rd ID byte, rely on NAND controller to identify the chip
			   else {
			   }
			 */
#endif                  // V3.0 Controller
			if (foundInIdTable && brcmnand_chips[i].eccLevel) {
				if (brcmnand_chips[i].sectorSize == 1024) {
					chip->reqEccLevel = brcmnand_chips[i].eccLevel;
					chip->eccSectorSize = 1024;
				}else  {
					chip->reqEccLevel = brcmnand_chips[i].eccLevel;
					chip->eccSectorSize = 512;
				}
				switch (chip->reqEccLevel) {
				case 15:
					chip->ecclevel = BRCMNAND_ECC_HAMMING;
					break;
				case 4:
					chip->ecclevel = BRCMNAND_ECC_BCH_4;
					break;
				case 8:
					chip->ecclevel = BRCMNAND_ECC_BCH_8;
					break;
				case 12:
				case 24:
					chip->ecclevel = BRCMNAND_ECC_BCH_12;
					break;
				}
				printk("%s: Ecc level set to %d, sectorSize=%d from ID table\n", __FUNCTION__, chip->reqEccLevel, chip->eccSectorSize);
			}
		}
		/* ID not in table, and no CONFIG REG was passed at command line */
		else if (!skipIdLookup && !foundInIdTable) {
#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_2_2
			uint32_t acc;

			/*
			 * else chip ID not found in table, just use what the NAND controller says.
			 * We operate under the premise that if it goes this far, the controller/CFE may
			 * have done something right.  It is not guaranteed to work, however
			 */
			/*
			 * Do nothing, we will decode the controller CONFIG register for
			 * for flash geometry
			 */

			/*
			 * Also, we need to find out the size of the OOB from ACC_CONTROL reg
			 */
			acc = brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi]));
			chip->eccOobSize =
				(acc & BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK) >>
				BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT;

			printk("Spare Area Size = %dB/512B\n", chip->eccOobSize);
#endif
			nand_config = chip->ctrl_read(bchp_nand_config(chip->ctrl->CS[chip->csi]));
		}
	}

	/*
	 * else ID not in database, but CONFIG reg was passed at command line, already handled
	 */

	/*
	 * For some ID case, the ID decode does not yield all informations,
	 * so we read it back, making sure that NAND CONFIG register and chip-> struct
	 * have matching infos.
	 */
	brcmnand_decode_config(chip, nand_config);

	// Also works for dummy entries, but no adjustments possible
	brcmnand_adjust_timings(chip, &brcmnand_chips[i]);

#if CONFIG_MTD_BRCMNAND_VERSION > CONFIG_MTD_BRCMNAND_VERS_2_2
	// Adjust perchip NAND ACC CONTROL
	// updateInternalData = not ONFI .or. not in ID table
	brcmnand_adjust_acccontrol(chip, isONFI, foundInIdTable, i);
#endif

	/* Flash device information */
	brcmnand_print_device_info(&brcmnand_chips[i], mtd);
	chip->options = brcmnand_chips[i].options;

	/* BrcmNAND page size & block size */
	mtd->writesize = chip->pageSize;
	mtd->writebufsize = mtd->writesize;
	// OOB size for MLC NAND varies depend on the chip
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_0
	mtd->oobsize = mtd->writesize >> 5; // tht - 16 byte OOB for 512B page, 64B for 2K page
#else
	chip->eccsteps = chip->pageSize / chip->eccsize;
	mtd->oobsize = chip->eccOobSize * chip->eccsteps;
#endif
	mtd->erasesize = chip->blockSize;

	/* Fix me: When we have both a NOR and NAND flash on board */
	/* For now, we will adjust the mtd->size for version 0.0 and 0.1 later in scan routine */

	if (chip->ctrl->numchips == 0)
		chip->ctrl->numchips = 1;

#if 0
/* This is old codes, now after we switch to support multiple configs, size is per chip size  */
	chip->mtdSize = chip->chipSize * chip->ctrl->numchips;

	/*
	 * THT: This is tricky.  We use mtd->size == 0 as an indicator whether the size
	 * fit inside a uint32_t.  In the case it overflow, size is returned by
	 * the inline function device_size(mtd), which is num_eraseblocks*block_size
	 */
	if (mtd64_ll_high(chip->mtdSize)) { // Beyond 4GB limit
		mtd->size = 0;
	}else  {
		mtd->size = mtd64_ll_low(chip->mtdSize);
	}
/*  */
#endif

	mtd->size = chip->mtdSize = chip->chipSize;


	//mtd->num_eraseblocks = chip->mtdSize >> chip->erase_shift;

	/* Version ID */
	version_id = chip->ctrl_read(BCHP_NAND_REVISION);

	printk(KERN_INFO "BrcmNAND version = 0x%04x %dMB @%08lx\n",
	       version_id, mtd64_ll_low(chip->chipSize >> 20), chip->pbase);

	gdebug = 0;

	return 0;
}

/**
 * brcmnand_suspend - [MTD Interface] Suspend the BrcmNAND flash
 * @param mtd		MTD device structure
 */
static int brcmnand_suspend(struct mtd_info *mtd)
{
	DEBUG(MTD_DEBUG_LEVEL3, "-->%s  \n", __FUNCTION__);
	return brcmnand_get_device(mtd, BRCMNAND_FL_PM_SUSPENDED);
}

/**
 * brcmnand_resume - [MTD Interface] Resume the BrcmNAND flash
 * @param mtd		MTD device structure
 */
static void brcmnand_resume(struct mtd_info *mtd)
{
	struct brcmnand_chip * chip = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3, "-->%s  \n", __FUNCTION__);
	if (chip->ctrl->state == BRCMNAND_FL_PM_SUSPENDED)
		brcmnand_release_device(mtd);
	else
		printk(KERN_ERR "resume() called for the chip which is not"
		       "in suspended state\n");
}

#if 0

static void fill_ecccmp_mask(struct mtd_info *mtd)
{
	struct brcmnand_chip * chip = mtd->priv;
	int i, len;

	struct nand_oobfree *free = chip->ecclayout->oobfree;
	unsigned char* myEccMask = (unsigned char*)eccmask;  // Defeat const

	/*
	 * Should we rely on eccmask being zeroed out
	 */
	for (i = 0; i < ARRAY_SIZE(eccmask); i++) {
		myEccMask[i] = 0;
	}
	/* Write 0xFF where there is a free byte */
	for (i = 0, len = 0;
	     len < chip->oobavail && len < mtd->oobsize && i < MTD_MAX_OOBFREE_ENTRIES;
	     i++) {
		int to = free[i].offset;
		int num = free[i].length;

		if (num == 0) break; // End marker reached
		memcpy(&myEccMask[to], ffchars, num);
		len += num;
	}
}
#endif


#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_3
/* Not needed when version >=3.3, as newer chip allow different NAND */

/*
 * Make sure that all NAND chips have same ID
 */
static int
brcmnand_validate_cs(struct mtd_info *mtd )
{

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_3
	struct brcmnand_chip* chip = (struct brcmnand_chip*)mtd->priv;
	int i;
	unsigned long dev_id;

	// Now verify that a NAND chip is at the CS
	for (i = 0; i < chip->ctrl->numchips; i++) {
		brcmnand_read_id(mtd, chip->ctrl->CS[i], &dev_id);

		if (dev_id != chip->device_id) {
			printk(KERN_ERR "Device ID for CS[%1d] = %08lx, Device ID for CS[%1d] = %08lx\n",
			       chip->ctrl->CS[0], chip->device_id, chip->ctrl->CS[i], dev_id);
			return 1;
		}

		printk("Found NAND flash on Chip Select %d, chipSize=%dMB, usable size=%dMB, base=%lx\n",
		       chip->ctrl->CS[i], mtd64_ll_low(chip->chipSize >> 20),
		       mtd64_ll_low(device_size(mtd) >> 20), chip->pbase);

	}
	return 0;

#else
	/* Version 3.3 and later allows multiple IDs */
	struct brcmnand_chip* chip = (struct brcmnand_chip*)mtd->priv;
	int i;
	unsigned long dev_id;

	// Now verify that a NAND chip is at the CS
	for (i = 0; i < chip->ctrl->numchips; i++) {
		brcmnand_read_id(mtd, chip->ctrl->CS[i], &dev_id);

/*
                if (dev_id != chip->device_id) {
                        printk(KERN_ERR "Device ID for CS[%1d] = %08lx, Device ID for CS[%1d] = %08lx\n",
                                chip->ctrl->CS[0], chip->device_id, chip->ctrl->CS[i], dev_id);
                        return 1;
                }
 */
		printk("Found NAND flash on Chip Select %d, chipSize=%dMB, usable size=%dMB, base=%lx\n",
		       chip->ctrl->CS[i], mtd64_ll_low(chip->chipSize >> 20),
		       mtd64_ll_low(device_size(mtd) >> 20), chip->pbase);

	}
	return 0;
#endif
}

#endif /* Version < 3.3 */

#if     0       /* jipeng - avoid undefined variable error in 7408A0 */
/*
 * CS0 reset values are gone by now, since the bootloader disabled CS0 before booting Linux
 * in order to give the EBI address space to NAND.
 * We will need to read strap_ebi_rom_size in order to reconstruct the CS0 values
 * This will not be a problem, since in order to boot with NAND on CSn (n != 0), the board
 * must be strapped for NOR.
 */
static unsigned int __maybe_unused
get_rom_size(unsigned long* outp_cs0Base)
{
	volatile unsigned long strap_ebi_rom_size, sun_top_ctrl_strap_value;
	uint32_t romSize = 0;

#if defined(BCHP_SUN_TOP_CTRL_STRAP_VALUE_0_strap_ebi_rom_size_MASK)
	sun_top_ctrl_strap_value = (volatile unsigned long)BDEV_RD(BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);
	strap_ebi_rom_size = sun_top_ctrl_strap_value & BCHP_SUN_TOP_CTRL_STRAP_VALUE_0_strap_ebi_rom_size_MASK;
	strap_ebi_rom_size >>= BCHP_SUN_TOP_CTRL_STRAP_VALUE_0_strap_ebi_rom_size_SHIFT;
#elif defined(BCHP_SUN_TOP_CTRL_STRAP_VALUE_strap_ebi_rom_size_MASK)
	sun_top_ctrl_strap_value = (volatile unsigned long)BDEV_RD(BCHP_SUN_TOP_CTRL_STRAP_VALUE);
	strap_ebi_rom_size = sun_top_ctrl_strap_value & BCHP_SUN_TOP_CTRL_STRAP_VALUE_strap_ebi_rom_size_MASK;
	strap_ebi_rom_size >>= BCHP_SUN_TOP_CTRL_STRAP_VALUE_strap_ebi_rom_size_SHIFT;
#elif defined(BCHP_SUN_TOP_CTRL_STRAP_VALUE_0_strap_bus_mode_MASK)
	romSize = 512 << 10; /* 512K */
	*outp_cs0Base = 0x1FC00000;
	return romSize;
#elif !defined(CONFIG_BRCM_HAS_NOR)
	printk("FIXME: no strap option for rom size on 3548/7408\n");
	BUG();
#else
	/* all new 40nm chips */
	return 64 << 20;
#endif

	// Here we expect these values to remain the same across platforms.
	// Some customers want to have a 2MB NOR flash, but I don't see how that is possible.
	switch (strap_ebi_rom_size) {
	case 0:
		romSize = 64 << 20;
		*outp_cs0Base = (0x20000000 - romSize) | BCHP_EBI_CS_BASE_0_size_SIZE_64MB;
		break;
	case 1:
		romSize = 16 << 20;
		*outp_cs0Base = (0x20000000 - romSize) | BCHP_EBI_CS_BASE_0_size_SIZE_16MB;
		break;
	case 2:
		romSize = 8 << 20;
		*outp_cs0Base = (0x20000000 - romSize) | BCHP_EBI_CS_BASE_0_size_SIZE_8MB;
		break;
	case 3:
		romSize = 4 << 20;
		*outp_cs0Base = (0x20000000 - romSize) | BCHP_EBI_CS_BASE_0_size_SIZE_4MB;
		break;
	default:
		printk("%s: Impossible Strap Value %08lx for BCHP_SUN_TOP_CTRL_STRAP_VALUE\n",
		       __FUNCTION__, sun_top_ctrl_strap_value);
		BUG();
	}
	return romSize;
}
#endif


static void brcmnand_prepare_reboot_priv(struct mtd_info *mtd)
{
	/*
	 * Must set NAND back to Direct Access mode for reboot, but only if NAND is on CS0
	 */

	struct brcmnand_chip* this;

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
	/* Flush pending in-mem CET to flash before exclusive lock */
	if (mtd) {
		brcmnand_cet_prepare_reboot(mtd);
	}
#endif
	if (mtd) {
		this = (struct brcmnand_chip*)mtd->priv;
		brcmnand_get_device(mtd, BRCMNAND_FL_XIP);
	}else
		/* Nothing we can do without an mtd handle */
		return;

#if 0
/* No longer used.  We now required the mtd handle */
	else {
		/*
		 * Prevent further access to the NAND flash, we are rebooting
		 */
		this = brcmnand_get_device_exclusive();
	}
#endif

#if     0       /* jipeng - avoid undefined variable error in 7408A0 */
	// PR41560: Handle boot from NOR but open NAND flash for access in Linux
	//if (!is_bootrom_nand()) {
	if (0) {
		// Restore CS0 in order to allow boot from NOR.

		//int ret = -EFAULT;
		int i;
		int csNand; // Which CS is NAND
		volatile unsigned long cs0Base, cs0Cnfg, cs0BaseAddr, csNandSelect, extAddr;
		volatile unsigned long csNandBase[MAX_NAND_CS], csNandCnfg[MAX_NAND_CS];
		unsigned int romSize;

		romSize = get_rom_size((unsigned long*)&cs0Base);
//printk("ROM size is %dMB\n", romSize >>20);

		cs0BaseAddr = cs0Base & BCHP_EBI_CS_BASE_0_base_addr_MASK;

		cs0Cnfg = *(volatile unsigned long*)(0xb0000000 | BCHP_EBI_CS_CONFIG_0);

		// Turn off NAND CS
		for (i = 0; i < this->numchips; i++) {
			csNand = this->CS[i];

			if (csNand == 0) {
				printk("%s: Call this routine only if NAND is not on CS0\n", __FUNCTION__);
			}

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_1_0
			BUG_ON(csNand > 5);
#else
			BUG_ON(csNand > 8);
#endif
			csNandBase[i] = *(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_BASE_0 + 8 * csNand);
			csNandCnfg[i] = *(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_CONFIG_0 + 8 * csNand);

			// Turn off NAND, must turn off both NAND_CS_NAND_SELECT and CONFIG.
			// We turn off the CS_CONFIG here, and will turn off NAND_CS_NAND_SELECT for all CS at once,
			// outside the loop.
			*(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_CONFIG_0 + 8 * csNand) =
				csNandCnfg[i] & (~BCHP_EBI_CS_CONFIG_0_enable_MASK);

		}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_0_1
		csNandSelect = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);


		csNandSelect &=
			~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_1_0
				BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_SEL_MASK
				| BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_SEL_MASK
				| BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_SEL_MASK
				| BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_SEL_MASK
				| BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_SEL_MASK
				| BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK
#else
				0x0000003E      /* Not documented on V1.0+ */
#endif // Version < 1.0
				);
#endif          // version >= 0.1
		brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, csNandSelect);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
		// THT from TM/RP: 020609: Clear NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG
		csNandSelect &= ~(BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK);
		brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, csNandSelect);

		// THT from TM/RP: 020609: Clear NAND_CMD_EXT_ADDRESS_CS_SEL
		extAddr = brcmnand_ctrl_read(BCHP_NAND_CMD_EXT_ADDRESS);
		extAddr &= ~(BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_MASK);
		brcmnand_ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, extAddr);
#endif

//printk("Turn on NOR\n");
		// Turn on NOR on CS0
		*(volatile unsigned long*)(0xb0000000 | BCHP_EBI_CS_CONFIG_0) =
			cs0Cnfg | BCHP_EBI_CS_CONFIG_0_enable_MASK;

//printk("returning from reboot\n");
		// We have turned on NOR, just return, leaving NAND locked
		// The CFE will straighten out everything.
		return;
	}
#endif  /* 0 */

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	// Otherwise if NAND is on CS0, turn off direct access before rebooting
	if (this->ctrl->CS[0] == 0) { // Only if on CS0
		volatile unsigned long nand_select, ext_addr;

		// THT: Set Direct Access bit
		nand_select = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);
		//printk("%s: B4 nand_select = %08x\n", __FUNCTION__, (uint32_t) nand_select);
		nand_select |= BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK;

		// THT from TM/RP: 020609: Clear NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG
		nand_select &= ~(BCHP_NAND_CS_NAND_SELECT_AUTO_DEVICE_ID_CONFIG_MASK);
		brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, nand_select);
		//nand_select = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);
		//printk("%s: After nand_select = %08x\n", __FUNCTION__, (uint32_t)  nand_select);

		// THT from TM/RP: 020609: Clear NAND_CMD_EXT_ADDRESS_CS_SEL
		ext_addr = brcmnand_ctrl_read(BCHP_NAND_CMD_EXT_ADDRESS);
		ext_addr &= ~(BCHP_NAND_CMD_EXT_ADDRESS_CS_SEL_MASK);
		brcmnand_ctrl_write(BCHP_NAND_CMD_EXT_ADDRESS, ext_addr);
	}

#endif  //#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0


	return;
}

#if 0
// In case someone reboot w/o going thru the MTD notifier mechanism.
void brcmnand_prepare_reboot(void)
{
	brcmnand_prepare_reboot_priv(NULL);
}
EXPORT_SYMBOL(brcmnand_prepare_reboot);
#endif


static int brcmnand_reboot_cb(struct notifier_block *nb, unsigned long val, void *v)
{
	struct mtd_info *mtd;

	mtd = container_of(nb, struct mtd_info, reboot_notifier);
	brcmnand_prepare_reboot_priv(mtd);
	return NOTIFY_DONE;
}

static void initialize_chip(struct brcmnand_chip* chip)
{

	/* Initialize chip level routines */

	if (!chip->ctrl_read)
		chip->ctrl_read = brcmnand_ctrl_read;
	if (!chip->ctrl_write)
		chip->ctrl_write = brcmnand_ctrl_write;
	if (!chip->ctrl_writeAddr)
		chip->ctrl_writeAddr = brcmnand_ctrl_writeAddr;

#if 0
	if (!chip->read_raw)
		chip->read_raw = brcmnand_read_raw;
	if (!chip->read_pageoob)
		chip->read_pageoob = brcmnand_read_pageoob;
#endif

	if (!chip->write_is_complete)
		chip->write_is_complete = brcmnand_write_is_complete;

	if (!chip->wait)
		chip->wait = brcmnand_wait;

	if (!chip->block_markbad)
		chip->block_markbad = brcmnand_default_block_markbad;
	if (!chip->scan_bbt)
		chip->scan_bbt = brcmnand_default_bbt;
	if (!chip->erase_bbt)
		chip->erase_bbt = brcmnand_erase_bbt;

	chip->eccsize = BRCMNAND_FCACHE_SIZE;  // Fixed for Broadcom controller


	/*
	 * For now initialize ECC read ops using the controller version, will switch to ISR version after
	 * EDU has been enabled
	 */

	if (!chip->read_page)
		chip->read_page = brcmnand_read_page;
	if (!chip->write_page)
		chip->write_page = brcmnand_write_page;
	if (!chip->read_page_oob)
		chip->read_page_oob = brcmnand_read_page_oob;
	if (!chip->write_page_oob)
		chip->write_page_oob = brcmnand_write_page_oob;

	if (!chip->read_oob)
		chip->read_oob = brcmnand_do_read_ops;
	if (!chip->write_oob)
		chip->write_oob = brcmnand_do_write_ops;
}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_2_0
static void handle_xor(struct brcmnand_chip* chip)
{
	//int i;
	uint32_t nand_xor;
	uint32_t __maybe_unused nand_select;

	/*
	 * 2618-7.3: For v2.0 or later, set xor_disable according to NAND_CS_NAND_XOR:00 bit
	 */

	nand_xor = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_XOR);
	printk("NAND_CS_NAND_XOR=%08x\n", nand_xor);
	//
#ifdef CONFIG_MTD_BRCMNAND_DISABLE_XOR
/* Testing 1,2,3: Force XOR disable on CS0, if not done by CFE */
	if (chip->ctrl->CS[0] == 0) {
		printk("Disabling XOR: Before: SEL=%08x, XOR=%08x\n", nand_select, nand_xor);

		nand_select &= ~BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK;
		nand_xor &= ~BCHP_NAND_CS_NAND_XOR_EBI_CS_0_ADDR_1FC0_XOR_MASK;

		brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, nand_select);
		brcmnand_ctrl_write(BCHP_NAND_CS_NAND_XOR, nand_xor);

		printk("Disabling XOR: After: SEL=%08x, XOR=%08x\n", nand_select, nand_xor);
	}
#endif
	/* Translate nand_xor into our internal flag, for brcmnand_writeAddr */
	// for (i=0; i<chip->ctrl->numchips; i++)
	//i = chip->csi;


	/* Set xor_disable, 1 for each NAND chip */
	if (!(nand_xor & (BCHP_NAND_CS_NAND_XOR_EBI_CS_0_ADDR_1FC0_XOR_MASK << chip->ctrl->CS[chip->csi]))) {
		PRINTK("Disabling XOR on CS#%1d\n", chip->ctrl->CS[chip->csi]);
		chip->xor_disable = 1;
	}


}
#endif /* v2.0 or later */

#if CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_0_1

/*
 * Version 0.1 can only have Hamming, so
 * the problem is handle the flash EBI base address
 */
static void handle_ecclevel_v0_1(struct mtd_info *mtd, struct brcmnand_chip* chip, int cs)
{
	if (cs) {
		volatile unsigned long wr_protect;
		volatile unsigned long acc_control;

		chip->ctrl->numchips = 1;

		/* Set up base, based on flash size */
		if (chip->chipSize >= (256 << 20)) {
			chip->pbase = 0x12000000;
			mtd->size = 0x20000000 - chip->pbase; // THT: This is different than chip->chipSize
		} else {
			/* We know that flash endAddr is 0x2000_0000 */
			chip->pbase = 0x20000000 - chip->chipSize;
			mtd->size = chip->chipSize;
		}

		printk("Found NAND chip on Chip Select %d, chipSize=%dMB, usable size=%dMB, base=%08x\n",
		       (int)cs, mtd64_ll_low(chip->chipSize >> 20), mtd64_ll_low(device_size(mtd) >> 20), (unsigned int)chip->pbase);



		/*
		 * When NAND is on CS0, it reads the strap values and set up accordingly.
		 * WHen on CS1, some configurations must be done by SW
		 */

		// Set Write-Unprotect.  This register is sticky, so if someone already set it, we are out of luck
		wr_protect = brcmnand_ctrl_read(BCHP_NAND_BLK_WR_PROTECT);
		if (wr_protect) {
			printk("Unprotect Register B4: %08x.  Please do a hard power recycle to reset\n", (unsigned int)wr_protect);
			// THT: Actually we should punt here, as we cannot zero the register.
		}
		brcmnand_ctrl_write(BCHP_NAND_BLK_WR_PROTECT, 0); // This will not work.
		if (wr_protect) {
			printk("Unprotect Register after: %08x\n", brcmnand_ctrl_read(BCHP_NAND_BLK_WR_PROTECT));
		}

		// Enable HW ECC.  This is another sticky register.
		acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));
		printk("ACC_CONTROL B4: %08x\n", (unsigned int)acc_control);

		brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control | BCHP_NAND_ACC_CONTROL_RD_ECC_BLK0_EN_MASK);
		if (!(acc_control & BCHP_NAND_ACC_CONTROL_RD_ECC_BLK0_EN_MASK)) {
			printk("ACC_CONTROL after: %08x\n", brcmnand_ctrl_read(bchp_nand_acc_control(cs)));
		}
	}else  {
		/* NAND chip on Chip Select 0 */
		chip->ctrl->CS[0] = 0;

		chip->ctrl->numchips = 1;

		/* Set up base, based on flash size */
		if (chip->chipSize >= (256 << 20)) {
			chip->pbase = 0x12000000;
			mtd->size = 0x20000000 - chip->pbase; // THT: This is different than chip->chipSize
		} else {
			/* We know that flash endAddr is 0x2000_0000 */
			chip->pbase = 0x20000000 - chip->chipSize;
			mtd->size = chip->chipSize;
		}
		//mtd->size_hi = 0;
		chip->mtdSize = mtd->size;

		printk("Found NAND chip on Chip Select 0, size=%dMB, base=%08x\n", mtd->size >> 20, (unsigned int)chip->pbase);

	}
	chip->vbase = (void*)KSEG1ADDR(chip->pbase);
}

#elif CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
/* Version 3.0 or later */
static uint32_t
handle_acc_control(struct mtd_info *mtd, struct brcmnand_chip* chip, int cs)
{
	volatile unsigned long acc_control, org_acc_control;
	int csi = chip->csi; // Index into chip->ctrl->CS array
	unsigned long eccLevel = 0, eccLevel_0, eccLevel_n;
	uint32_t eccOobSize;

	if (gAccControl[csi] != 0) {
		// Already done in brcmnand_adjust_acccontrol()
		printk("ECC level from command line=%d\n", chip->ecclevel);
		return chip->ecclevel; // Do nothing, take the overwrite value
	}

  #if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_3


	PRINTK("100 CS=%d, chip->ctrl->CS[%d]=%d\n", cs, chip->csi, chip->ctrl->CS[chip->csi]);

	org_acc_control = acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));

	/*
	 * For now, we only support same ECC level for both block0 and other blocks
	 */
	// Verify BCH-4 ECC: Handle CS0 block0

	// ECC level for block-0
	eccLevel = eccLevel_0 = (acc_control & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK) >>
				BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT;
	// ECC level for all other blocks.
	eccLevel_n = (acc_control & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK) >>
		     BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;

	// make sure that block-0 and block-n use the same ECC level.
	if (eccLevel_0 != eccLevel_n) {
		// Use eccLevel_0 for eccLevel_n, unless eccLevel_0 is 0.
		if (eccLevel_0 == 0) {
			eccLevel = eccLevel_n;
		}
		acc_control &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK |
				 BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK);
		acc_control |= (eccLevel <<  BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT) |
			       (eccLevel << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT);
		brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );

		if (eccLevel == eccLevel_0) {
			printk("Corrected ECC on block-n to ECC on block-0: ACC = %08lx from %08lx\n",
			       acc_control, org_acc_control);
		}else  {
			printk("Corrected ECC on block-0 to ECC on block-n: ACC = %08lx from %08lx\n",
			       acc_control, org_acc_control);
		}

	}
	chip->ecclevel = eccLevel;


	switch (eccLevel) {
	case BRCMNAND_ECC_HAMMING:
		if (NAND_IS_MLC(chip)) {
			printk(KERN_INFO "Only BCH-4 or better is supported on MLC flash\n");
			chip->ecclevel  = BRCMNAND_ECC_BCH_4;
			acc_control &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK |
					 BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK);
			acc_control |= (BRCMNAND_ECC_BCH_4 <<  BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT) |
				       (BRCMNAND_ECC_BCH_4 << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
			printk("Corrected ECC to BCH-4 for MLC flashes: ACC_CONTROL = %08lx from %08lx\n", acc_control, org_acc_control);
		}
		break;

	case BRCMNAND_ECC_BCH_4:
	case BRCMNAND_ECC_BCH_8:
	case BRCMNAND_ECC_BCH_12:
		// eccOobSize is initialized to the board strap of ECC-level
		eccOobSize = (acc_control & BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK) >>
			     BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT;
		printk("ACC: %d OOB bytes per 512B ECC step; from ID probe: %d\n", eccOobSize, chip->eccOobSize);
		//Make sure that the OOB size is >= 27
		if (eccLevel == BRCMNAND_ECC_BCH_12 && chip->eccOobSize < 27) {
			printk(KERN_INFO "BCH-12 requires >=27 OOB bytes per ECC step.\n");
			printk(KERN_INFO "Please fix your board straps. Aborting to avoid file system damage\n");
			BUG();
		}
		// We have recorded chip->eccOobSize during probe, let's compare it against value from straps:
		if (chip->eccOobSize < eccOobSize) {
			printk("Flash says it has %d OOB bytes, eccLevel=%lu, but board strap says %d bytes, fixing it...\n",
			       chip->eccOobSize, eccLevel, eccOobSize);
			acc_control &= ~(BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_MASK \
					 | BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK);
			acc_control |= (chip->eccOobSize << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_SHIFT)
				       | (chip->eccOobSize << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT);
			printk("ACC_CONTROL adjusted to %08x\n", (unsigned int)acc_control);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
		}

		break;

	default:
		printk(KERN_ERR "Unsupported ECC level %lu\n", eccLevel);
		BUG();

	}


	chip->ecclevel = eccLevel;
	//csi++; // Look at next CS


	/*
	 * PR57272: Workaround for BCH-n error,
	 * reporting correctable errors with 4 or more bits as uncorrectable:
	 */
	if (chip->ecclevel != 0 && chip->ecclevel != BRCMNAND_ECC_HAMMING) {
		int corr_threshold;

		if (chip->ecclevel >  BRCMNAND_ECC_BCH_4) {
			printk(KERN_WARNING "%s: Architecture cannot support ECC level %d\n", __FUNCTION__, chip->ecclevel);
			corr_threshold = 3;
		}else if ( chip->ecclevel ==  BRCMNAND_ECC_BCH_4) {
			corr_threshold = 3;     // Changed from 2, since refresh is costly and vulnerable to AC-ON/OFF tests.
		}else  {
			corr_threshold = 1;     // 1 , default for Hamming
		}

		printk(KERN_INFO "%s: CORR ERR threshold set to %d bits\n", __FUNCTION__, corr_threshold);
		corr_threshold <<= BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_SHIFT;
		brcmnand_ctrl_write(BCHP_NAND_CORR_STAT_THRESHOLD, corr_threshold);
	}

  #else /* NAND version 3.3 or later */

	PRINTK("100 CS=%d, chip->ctrl->CS[%d]=%d\n", cs, chip->csi, chip->ctrl->CS[chip->csi]);

	org_acc_control = acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));

	/*
	 * For now, we only support same ECC level for both block0 and other blocks
	 */
	// Verify BCH-4 ECC: Handle CS0 block0
	if (chip->ctrl->CS[chip->csi] == 0) {
		// ECC level for all other blocks.
		eccLevel_n = (acc_control & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK) >>
			     BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT;
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
		// ECC level for block-0
		eccLevel = eccLevel_0 = (acc_control & BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK) >>
					BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT;

		// make sure that block-0 and block-n use the same ECC level.
		if (eccLevel_0 != eccLevel_n) {
			// Use eccLevel_0 for eccLevel_n, unless eccLevel_0 is 0.
			if (eccLevel_0 == 0) {
				eccLevel = eccLevel_n;
			}
			acc_control &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK |
					 BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK);
			acc_control |= (eccLevel <<  BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT) |
				       (eccLevel << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );

			if (eccLevel == eccLevel_0) {
				printk("Corrected ECC on block-n to ECC on block-0: ACC = %08lx from %08lx\n",
				       acc_control, org_acc_control);
			}else  {
				printk("Corrected ECC on block-0 to ECC on block-n: ACC = %08lx from %08lx\n",
				       acc_control, org_acc_control);
			}

		}
		chip->ecclevel = eccLevel;
#else
		chip->ecclevel = eccLevel_n;
		eccLevel = eccLevel_n;
#endif
		/*
		 * Make sure that threshold is set at 75% of #bits the ECC can correct.
		 * This should be done for each CS!!!!!
		 */
		if (chip->ecclevel != 0 && chip->ecclevel != BRCMNAND_ECC_HAMMING) {
			uint32_t corr_threshold = brcmnand_ctrl_read(BCHP_NAND_CORR_STAT_THRESHOLD) & BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_MASK;
			uint32_t seventyfivepc;

			seventyfivepc = (chip->ecclevel * 3) / 4;
			printk(KERN_INFO "%s: default CORR ERR threshold  %d bits\n", __FUNCTION__, corr_threshold);
			PRINTK("ECC level threshold set to %d bits\n", corr_threshold);
			if (seventyfivepc < corr_threshold) {
				printk(KERN_INFO "%s: CORR ERR threshold set to %d bits\n", __FUNCTION__, seventyfivepc);
				seventyfivepc <<= BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_SHIFT;
				seventyfivepc |= (brcmnand_ctrl_read(BCHP_NAND_CORR_STAT_THRESHOLD) & ~BCHP_NAND_CORR_STAT_THRESHOLD_CORR_STAT_THRESHOLD_MASK);
				brcmnand_ctrl_write(BCHP_NAND_CORR_STAT_THRESHOLD, seventyfivepc);
			}
		}
		PRINTK("ECC level %d, threshold at %d bits\n",
		       chip->ecclevel, brcmnand_ctrl_read(BCHP_NAND_CORR_STAT_THRESHOLD));

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		acc_control &= ~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
			BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_0_MASK |
#endif
			BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_MASK);
		if (chip->eccSectorSize == 1024) {
			acc_control |= (
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
				BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_0_MASK |
#endif
				BCHP_NAND_ACC_CONTROL_SECTOR_SIZE_1K_MASK);
		}
		brcmnand_ctrl_write(bchp_nand_acc_control(0), acc_control );
#endif
	}else  { // CS != 0

		eccLevel = eccLevel_0 = (acc_control & BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_MASK) >>
					BCHP_NAND_ACC_CONTROL_CS1_ECC_LEVEL_SHIFT;
		chip->ecclevel = eccLevel;

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_5_0
		acc_control &= ~(BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_MASK);
		if (chip->eccSectorSize == 1024) {
			acc_control |= (BCHP_NAND_ACC_CONTROL_CS1_SECTOR_SIZE_1K_MASK);
		}
		brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
#endif
	}


	switch (eccLevel) {
	case BRCMNAND_ECC_HAMMING:
		if (NAND_IS_MLC(chip)) {
			printk(KERN_INFO "Only BCH-4 or better is supported on MLC flash\n");
			eccLevel = chip->ecclevel  = BRCMNAND_ECC_BCH_4;
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
			acc_control &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_MASK |
					 BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK);
			acc_control |= (BRCMNAND_ECC_BCH_4 <<  BCHP_NAND_ACC_CONTROL_ECC_LEVEL_0_SHIFT) |
				       (BRCMNAND_ECC_BCH_4 << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT);
#else
			acc_control &= ~(BCHP_NAND_ACC_CONTROL_ECC_LEVEL_MASK);
			acc_control |= (BRCMNAND_ECC_BCH_4 << BCHP_NAND_ACC_CONTROL_ECC_LEVEL_SHIFT);
#endif
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
			printk("Corrected ECC to BCH-4 for MLC flashes: ACC_CONTROL = %08lx from %08lx\n", acc_control, org_acc_control);
		}
		break;

	case BRCMNAND_ECC_BCH_4:
	case BRCMNAND_ECC_BCH_8:
	case BRCMNAND_ECC_BCH_12:
		// eccOobSize is initialized to the board strap of ECC-level
		eccOobSize = (acc_control & BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK) >>
			     BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT;
		printk("ACC: %d OOB bytes per 512B ECC step; from ID probe: %d\n", eccOobSize, chip->eccOobSize);

		/* Temporary workarond. Id probe function does not set the ecc size. Need to implmenent this.*/
		if ( eccOobSize >= 27 && eccOobSize  > chip->eccOobSize ) {
			chip->eccOobSize = eccOobSize;
			mtd->oobsize = chip->eccOobSize * chip->eccsteps;
			printk(KERN_INFO "Use strap setting for ecc size %d bytes, mtd->oobsize %d.\n", eccOobSize, mtd->oobsize);
		}

		//Make sure that the OOB size is >= 27
		if (eccLevel == BRCMNAND_ECC_BCH_12 && chip->eccOobSize < 27) {
			printk(KERN_INFO "BCH-12 requires >=27 OOB bytes per ECC step.\n");
			printk(KERN_INFO "Please use the NAND part with enough spare eara and fix your board straps. Aborting to avoid file system damage\n");
			BUG();
		}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
		//NAND 7.0 requires more ECC byte for BCH-8
		if (eccLevel == BRCMNAND_ECC_BCH_8 && chip->eccOobSize < 27) {
			printk(KERN_INFO "BCH-8 requires >=27 OOB bytes per ECC step on NAND controller 7.0 or later.\n");
			printk(KERN_INFO "Please use the NAND part with enough spare eara and fix your board straps. Aborting to avoid file system damage\n");
			BUG();
		}
#endif

		// We have recorded chip->eccOobSize during probe, let's compare it against value from straps:
		if (chip->eccOobSize < eccOobSize) {
			printk("Flash says it has %d OOB bytes, eccLevel=%lu, but board strap says %d bytes, fixing it...\n",
			       chip->eccOobSize, eccLevel, eccOobSize);
			acc_control &= ~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
				BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_MASK |
#endif
				BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_MASK);
			acc_control |=
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
				(chip->eccOobSize << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_0_SHIFT) |
#endif
				(chip->eccOobSize << BCHP_NAND_ACC_CONTROL_SPARE_AREA_SIZE_SHIFT);
			printk("ACC_CONTROL adjusted to %08x\n", (unsigned int)acc_control);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control);
		}

		break;

	default:
		printk(KERN_ERR "Unsupported ECC level %lu\n", eccLevel);
		BUG();

	}


	chip->ecclevel = eccLevel;

#endif  /* else NAND version 3.3 or later */

	/*
	 * This is just a warning
	 */
	PRINTK("reqEccLevel=%d, eccLevel=%d\n", chip->reqEccLevel, chip->ecclevel);
	if (chip->reqEccLevel != 0 && chip->ecclevel != BRCMNAND_ECC_DISABLE) {
		if (chip->reqEccLevel == BRCMNAND_ECC_HAMMING) {
			; /* Nothing, lowest requirement */
		}
		/* BCH */
		else if (chip->reqEccLevel > 0 && chip->reqEccLevel <= BRCMNAND_ECC_BCH_12) {
			if (chip->reqEccLevel  > chip->ecclevel) {
				printk(KERN_WARNING "******* Insufficient ECC level, required=%d, strapped for %d ********\n",
				       chip->reqEccLevel,  chip->ecclevel);
			}
		}
	}

	return eccLevel;

	/* No need to worry about correctable error for V3.3 or later, just take the default */
}





// else nothing to do for v2.x
#endif /* if controller v0.1 else 2.0 or later */

#ifdef CONFIG_BCM3548
/*
 * Check to see if this is a 3548L or 3556,
 * in which case, disable WR_PREEMPT to avoid data corruption
 *
 * returns the passed-in acc-control register value with WR_PREEMPT disabled.
 */
static uint32_t check_n_disable_wr_preempt(uint32_t acc_control)
{
	uint32_t otp_option = BDEV_RD(BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS);

	printk("mcard_disable=%08x\n", otp_option);
	// Is there any device on the EBI bus: mcard_disable==0 means there is (a device hanging off the EBI bus)
	if (!(otp_option & BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_otp_option_mcard_in_disable_MASK)) {
		/* THT PR50928: Disable WR_PREEMPT for 3548L and 3556 */
		acc_control &= ~(BCHP_NAND_ACC_CONTROL_WR_PREEMPT_EN_MASK);
		brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
		printk("Disable WR_PREEMPT: ACC_CONTROL = %08x\n", acc_control);
	}
	return acc_control;
}
#endif

/**
 * brcmnand_scan - [BrcmNAND Interface] Scan for the BrcmNAND device
 * @param mtd		MTD device structure
 * @cs			        Chip Select number
 * @param numchips	Number of chips  (from CFE or from nandcs= kernel arg)
 * @lastChip			Start actual scan for bad blocks only on last chip
 *
 * This fills out all the not initialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 *
 */
int brcmnand_scan(struct mtd_info *mtd, int cs, int numchips )
{
	struct brcmnand_chip* chip = (struct brcmnand_chip*)mtd->priv;
	//unsigned char brcmnand_maf_id;
	int err, i;
	static int __maybe_unused notFirstChip;
	volatile unsigned long nand_select;
	unsigned int version_id;
	unsigned int version_major;
	unsigned int version_minor;

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	memset(ffchars, 0xff, sizeof(ffchars));
#endif

	PRINTK("-->%s: CS=%d, numchips=%d, csi=%d\n", __FUNCTION__, cs, numchips, chip->csi);

	chip->ctrl->CS[chip->csi] = cs;


	initialize_chip(chip);
	chip->ecclevel = BRCMNAND_ECC_HAMMING;

	printk(KERN_INFO "mtd->oobsize=%d, mtd->eccOobSize=%d\n", mtd->oobsize, chip->eccOobSize);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_2_0
	handle_xor(chip);

#endif  // if version >= 2.0 XOR

//	for (i=0; i<chip->ctrl->numchips; i++) {
//		cs = chip->ctrl->CS[i];

//gdebug=4;
	PRINTK("brcmnand_scan: Calling brcmnand_probe for CS=%d\n", cs);
	if (brcmnand_probe(mtd, cs)) {
		return -ENXIO;
	}
//gdebug=0;

/*
 * With version 3.3, we allow per-CS mtd handle, so it is handled in bcm7xxx-nand.c
 */
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0 && \
	CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_3_3
	if (chip->ctrl->numchips > 0) {
		if (brcmnand_validate_cs(mtd))
			return (-EINVAL);
	}
#endif

	PRINTK("brcmnand_scan: Done brcmnand_probe\n");


#if CONFIG_MTD_BRCMNAND_VERSION <= CONFIG_MTD_BRCMNAND_VERS_0_1
	handle_ecclevel_v0_1(mtd, chip, cs);

#else
	/*
	 * v1.0 controller and after
	 */
	// This table is in the Architecture Doc
	// pbase is the physical address of the "logical" start of flash.  Logical means how Linux sees it,
	// and is given by the partition table defined in bcm7xxx-nand.c
	// The "physical" start of the flash is always at 1FC0_0000


	if (chip->chipSize <= (256 << 20))
		chip->pbase = 0x20000000 - chip->chipSize;
	else    // 512MB and up
		chip->pbase = 0;

	// vbase is the address of the flash cache array
	chip->vbase = (void*)BVIRTADDR(BCHP_NAND_FLASH_CACHEi_ARRAY_BASE);   // Start of Buffer Cache
	// Already set in probe mtd->size = chip->chipSize * chip->ctrl->numchips;
	// Make sure we use Buffer Array access, not direct access, Clear CS0
	nand_select = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);
	printk("%s: B4 nand_select = %08x\n", __FUNCTION__, (uint32_t)nand_select);

	nand_select = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);
	printk("%s: After nand_select = %08x\n", __FUNCTION__, (uint32_t)nand_select);
	chip->directAccess = !(nand_select & BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK);



	/*
	 * Handle RD_ERASED_ECC bit, make sure it is not set
	 */
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_2_1
	{
		uint32_t acc0 = brcmnand_ctrl_read(bchp_nand_acc_control(cs));

		if (acc0 & BCHP_NAND_ACC_CONTROL_RD_ERASED_ECC_EN_MASK) {
			acc0 &= ~(BCHP_NAND_ACC_CONTROL_RD_ERASED_ECC_EN_MASK);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc0);
		}
	}
#endif



	/* Handle Partial Write Enable configuration for MLC
	 * {FAST_PGM_RDIN, PARTIAL_PAGE_EN}
	 * {0, 0} = 1 write per page, no partial page writes (required for MLC flash, suitable for SLC flash)
	 * {1, 1} = 4 partial page writes per 2k page (SLC flash only)
	 * {0, 1} = 8 partial page writes per 2k page (not recommended)
	 * {1, 0} = RESERVED, DO NOT USE
	 */
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
	//if (4)
	{
		/* For MLC, we only support BCH-4 or better */
		/* THT for 2.6.31-2.3: Nowadays, some SLC chips require higher ECC levels */

		//int eccOobSize;
		uint32_t eccLevel, acc_control, org_acc_control;
		int nrSectorPP = chip->pageSize / 512; // Number of sectors per page == controller's NOP

		org_acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));
		eccLevel = handle_acc_control(mtd, chip, cs);
		acc_control = brcmnand_ctrl_read(bchp_nand_acc_control(cs));


		PRINTK("190 eccLevel=%d, chip->ecclevel=%d, acc=%08x\n", eccLevel, chip->ecclevel, acc_control);

#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
		if (NAND_IS_MLC(chip)) {
			printk("Setting NAND_COMPLEX_OOB_WRITE\n");
			chip->options |= NAND_COMPLEX_OOB_WRITE;
		}
#endif

/*
 * For 3556 and 3548L, disable WR_PREEMPT
 */
#ifdef CONFIG_BCM3548
		acc_control = check_n_disable_wr_preempt(acc_control);
#endif

		/*
		 * Some SLC flashes have page size of 4KB, or more, and may need to disable Partial Page Programming
		 */
		if (NAND_IS_MLC(chip) || ((chip->nop > 0) && (nrSectorPP > chip->nop))) {
			/* Set FAST_PGM_RDIN, PARTIAL_PAGE_EN  to {0, 0} for NOP=1 */
			acc_control &= ~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
				BCHP_NAND_ACC_CONTROL_FAST_PGM_RDIN_MASK |
#endif
				BCHP_NAND_ACC_CONTROL_PARTIAL_PAGE_EN_MASK);
			brcmnand_ctrl_write(bchp_nand_acc_control(cs), acc_control );
			printk("Corrected for NOP=1: ACC_CONTROL = %08x\n", acc_control);
		}

	}


#endif  // NAND version 3.0 or later

#endif  // Version 1.0+

	PRINTK("%s 10\n", __FUNCTION__);

	PRINTK("200 CS=%d, chip->ctrl->CS[%d]=%d\n", cs, chip->csi, chip->ctrl->CS[chip->csi]);
	PRINTK("200 chip->ecclevel=%d, acc=%08x\n", chip->ecclevel,
	       brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi])));

	chip->bbt_erase_shift =  ffs(mtd->erasesize) - 1;

	/* Calculate the address shift from the page size */
	chip->page_shift = ffs(mtd->writesize) - 1;
	chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;
	chip->chip_shift = mtd64_ll_ffs(chip->chipSize) - 1;

	printk(KERN_INFO "page_shift=%d, bbt_erase_shift=%d, chip_shift=%d, phys_erase_shift=%d\n",
	       chip->page_shift, chip->bbt_erase_shift, chip->chip_shift, chip->phys_erase_shift);

	/* Set the bad block position */
	/* NAND_LARGE_BADBLOCK_POS also holds for MLC NAND */
	chip->badblockpos = mtd->writesize > 512 ?
			    NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;




	PRINTK("%s 220\n", __FUNCTION__);



	/* The number of bytes available for the filesystem to place fs dependend
	 * oob data */
//PRINTK( "Determining chip->oobavail, chip->autooob=%p \n", chip->autooob);

	/* Version ID */
	version_id = chip->ctrl_read(BCHP_NAND_REVISION);
	version_major = (version_id & 0xff00) >> 8;
	version_minor = (version_id & 0xff);

	printk(KERN_INFO "Brcm NAND controller version = %x.%x NAND flash size %dMB @%08x\n",
	       version_major, version_minor, mtd64_ll_low(chip->chipSize >> 20), (uint32_t)chip->pbase);


	PRINTK("%s 230\n", __FUNCTION__);
	/*
	 * Initialize the eccmask array for ease of verifying OOB area.
	 */
	//fill_ecccmp_mask(mtd);


	/* Store the number of chips and calc total size for mtd */
	//chip->ctrl->numchips = i;
	//mtd->size = i * chip->chipSize;

	/* Preset the internal oob write buffer */
	memset(BRCMNAND_OOBBUF(chip->ctrl->buffers), 0xff, mtd->oobsize);

	/*
	 * If no default placement scheme is given, select an appropriate one
	 * We should make a table for this convoluted mess. (TBD)
	 */
	PRINTK("%s 40, mtd->oobsize=%d, chip->ecclayout=%08x\n", __FUNCTION__, mtd->oobsize,
	       (unsigned int)chip->ecclayout);
	if (!chip->ecclayout) {
		PRINTK("%s 42, mtd->oobsize=%d, chip->ecclevel=%d, isMLC=%d, chip->cellinfo=%d\n", __FUNCTION__,
		       mtd->oobsize, chip->ecclevel, NAND_IS_MLC(chip), chip->cellinfo);
		switch (mtd->oobsize) {
		case 16: /* Small size NAND */
			if (chip->ecclevel == BRCMNAND_ECC_HAMMING) {
				chip->ecclayout = &brcmnand_oob_16;
			}else if (chip->ecclevel == BRCMNAND_ECC_BCH_4) {
				printk("ECC layout=brcmnand_oob_bch4_512\n");
				chip->ecclayout = &brcmnand_oob_bch4_512;
			}else if (chip->ecclevel != BRCMNAND_ECC_DISABLE) {
				printk(KERN_ERR "Unsupported ECC level for page size of %d\n", mtd->writesize);
				BUG();
			}
			break;

		case 64: /* Large page NAND 2K page */
			if (NAND_IS_MLC(chip) || chip->ecclevel == BRCMNAND_ECC_BCH_4
			    || chip->ecclevel == BRCMNAND_ECC_BCH_8
			    ) {
				switch (mtd->writesize) {
				case 4096: /* Impossible for 64B OOB per page */
					printk(KERN_ERR "Unsupported page size of %d\n", mtd->writesize);
					BUG();
/*
   printk("ECC layout=brcmnand_oob_bch4_4k\n");
                                        chip->ecclayout = &brcmnand_oob_bch4_4k;
 */
					break;
				case 2048:
					if (chip->ecclevel == BRCMNAND_ECC_BCH_4 ) {
						printk("ECC layout=brcmnand_oob_bch4_2k\n");
						chip->ecclayout = &brcmnand_oob_bch4_2k;
					}else if (chip->ecclevel == BRCMNAND_ECC_BCH_8 ) {
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
						if (chip->eccOobSize == 16) {
							printk("ECC layout=brcmnand_oob_bch8_16_2k\n");
							chip->ecclayout = &brcmnand_oob_bch8_16_2k;
						}else if (chip->eccOobSize >= 27) {
							printk("ECC layout=brcmnand_oob_bch8_27_2k\n");
							chip->ecclayout = &brcmnand_oob_bch8_27_2k;
						}
#else
						printk("ECC layout=brcmnand_oob_bch8_27_2k\n");
						chip->ecclayout = &brcmnand_oob_bch8_27_2k;
#endif
					}

					break;
				default:
					printk(KERN_ERR "Unsupported page size of %d\n", mtd->writesize);
					BUG();
					break;
				}
			}else if (chip->ecclevel == BRCMNAND_ECC_BCH_12) {
				printk("ECC layout=brcmnand_oob_bch12_27_2k\n");
				chip->ecclayout = &brcmnand_oob_bch12_27_2k;
			}else if (chip->ecclevel == BRCMNAND_ECC_HAMMING) {
				printk("ECC layout=brcmnand_oob_bch4_4k\n");
				chip->ecclayout = &brcmnand_oob_64;
			}else  {
				printk(KERN_ERR "Unsupported ECC code %d with only 64B OOB per page\n", chip->ecclevel);
				BUG();
			}
			break;

		case 128: /* Large page NAND 4K page or MLC */
			if (NAND_IS_MLC(chip)) {
				switch (mtd->writesize) {
				case 4096:
					switch (chip->ecclevel) {
					case BRCMNAND_ECC_BCH_4:
						printk("ECC layout=brcmnand_oob_bch4_4k\n");
						chip->ecclayout = &brcmnand_oob_bch4_4k;
						break;
					case BRCMNAND_ECC_BCH_8:
						if (chip->eccOobSize == 16) {
							printk("ECC layout=brcmnand_oob_bch8_16_4k\n");
							chip->ecclayout = &brcmnand_oob_bch8_16_4k;
						}
#if 1
						else if (chip->eccOobSize >= 27) {
							printk("ECC layout=brcmnand_oob_bch8_27_4k\n");
							chip->ecclayout = &brcmnand_oob_bch8_27_4k;
						}
						break;
					case BRCMNAND_ECC_BCH_12:
						printk("ECC layout=brcmnand_oob_bch12_27_4k\n");
						chip->ecclayout = &brcmnand_oob_bch12_27_4k;
						break;
#endif

					default:
						printk(KERN_ERR "Unsupported ECC code %d for MLC with pageSize=%d\n", chip->ecclevel, mtd->writesize);
						BUG();
					}
					break;
				default:
					printk(KERN_ERR "Unsupported page size of %d\n", mtd->writesize);
					BUG();
					break;
				}
			}else  { /* SLC chips, there are now some SLCs that require BCH-4 or better */
				switch (mtd->writesize) {
				case 4096:
					if (chip->ecclevel == BRCMNAND_ECC_HAMMING) {
						printk("ECC layout=brcmnand_oob_128\n");
						chip->ecclayout = &brcmnand_oob_128;
					}else if (chip->ecclevel == BRCMNAND_ECC_BCH_4) {
						printk("ECC layout=brcmnand_oob_bch4_4k\n");
						chip->ecclayout = &brcmnand_oob_bch4_4k;
					}else if (chip->ecclevel == BRCMNAND_ECC_BCH_8) {
						if (chip->eccOobSize == 16) {
							printk("ECC layout=brcmnand_oob_bch8_16_4k\n");
							chip->ecclayout = &brcmnand_oob_bch8_16_4k;
						}else if (chip->eccOobSize >= 27) {
							printk("ECC layout=brcmnand_oob_bch8_27_4k\n");
							chip->ecclayout = &brcmnand_oob_bch8_27_4k;

						}
					}else if (chip->ecclevel == BRCMNAND_ECC_BCH_12) {
						printk("ECC layout=brcmnand_oob_bch12_27_4k\n");
						chip->ecclayout = &brcmnand_oob_bch12_27_4k;
					}
					break;

				default:
					printk(KERN_ERR "Unsupported page size of %d\n", mtd->writesize);
					BUG();
					break;
				}
			} /* else SLC chips */
			break; /* 128B OOB case */

		default: /* 27.25/28 or greater OOB size */
			PRINTK("27B OOB\n");
			PRINTK("300 chip->ecclevel=%d, acc=%08x\n", chip->ecclevel, brcmnand_ctrl_read(bchp_nand_acc_control(chip->ctrl->CS[chip->csi])));
			if (mtd->writesize == 2048) {
				switch (chip->ecclevel) {
				case BRCMNAND_ECC_BCH_4:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch4_2k\n");
					chip->ecclayout = &brcmnand_oob_bch4_2k;
					break;
				case BRCMNAND_ECC_BCH_8:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch8_27_2k\n");
					chip->ecclayout = &brcmnand_oob_bch8_27_2k;
					break;
				case BRCMNAND_ECC_BCH_12:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch12_27_2k\n");
					chip->ecclayout = &brcmnand_oob_bch12_27_2k;
					break;
				default:
					printk(KERN_ERR "Unsupported ECC code %d with pageSize=%d\n", chip->ecclevel, mtd->writesize);
					BUG();
				}

			}else if (mtd->writesize == 4096) {
				switch (chip->ecclevel) {
				case BRCMNAND_ECC_BCH_4:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch4_4k\n");
					chip->ecclayout = &brcmnand_oob_bch4_4k;
					break;
				case BRCMNAND_ECC_BCH_8:
					if (chip->eccOobSize == 16) {
						printk(KERN_INFO "ECC layout=brcmnand_oob_bch8_16_4k\n");
						chip->ecclayout = &brcmnand_oob_bch8_16_4k;
					}else if (chip->eccOobSize >= 27) {
						printk(KERN_INFO "ECC layout=brcmnand_oob_bch8_27_4k\n");
						chip->ecclayout = &brcmnand_oob_bch8_27_4k;
					}
					break;
				case BRCMNAND_ECC_BCH_12:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch12_27_4k\n");
					chip->ecclayout = &brcmnand_oob_bch12_27_4k;
					break;
				default:
					printk(KERN_ERR "Unsupported ECC code %d  with pageSize=%d\n", chip->ecclevel, mtd->writesize);
					BUG();
				}

			}else if (mtd->writesize == 8192) { // 8KB page
				switch (chip->ecclevel) {
				case BRCMNAND_ECC_BCH_4:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch4_8k\n");
					chip->ecclayout = &brcmnand_oob_bch4_8k;
					break;
				case BRCMNAND_ECC_BCH_8:
					if (chip->eccOobSize == 16) {
						printk(KERN_INFO "ECC layout=brcmnand_oob_bch8_16_8k\n");
						chip->ecclayout = &brcmnand_oob_bch8_16_8k;
					}else if (chip->eccOobSize >= 27) {
						printk(KERN_INFO "ECC layout=brcmnand_oob_bch8_27_8k\n");
						chip->ecclayout = &brcmnand_oob_bch8_27_8k;
					}
					break;
				case BRCMNAND_ECC_BCH_12:
					printk(KERN_INFO "ECC layout=brcmnand_oob_bch12_27_8k\n");
					chip->ecclayout = &brcmnand_oob_bch12_27_8k;
					break;
				default:
					printk(KERN_ERR "Unsupported ECC code %d for MLC with pageSize=%d\n", chip->ecclevel, mtd->writesize);
					BUG();
				}
			}else  {
				printk(KERN_ERR "Unsupported page size of %d and oobsize %d\n", mtd->writesize, mtd->oobsize);
				BUG();
				break;
			}
			break; /* 27B OOB */
		}
	}



	/*
	 * The number of bytes available for a client to place data into
	 * the out of band area
	 */
	printk(KERN_INFO "%s:  mtd->oobsize=%d\n", __FUNCTION__, mtd->oobsize);
	chip->ecclayout->oobavail = 0;
	for (i = 0; chip->ecclayout->oobfree[i].length; i++)
		chip->ecclayout->oobavail +=
			chip->ecclayout->oobfree[i].length;

	mtd->oobavail = chip->ecclayout->oobavail;

	printk(KERN_INFO "%s: oobavail=%d, eccsize=%d, writesize=%d\n", __FUNCTION__,
	       chip->ecclayout->oobavail, chip->eccsize, mtd->writesize);

	/*
	 * Set the number of read / write steps for one page depending on ECC
	 * mode
	 */

	chip->eccsteps = mtd->writesize / chip->eccsize;
	chip->eccbytes = brcmnand_eccbytes[chip->ecclevel];
	printk(KERN_INFO "%s, eccsize=%d, writesize=%d, eccsteps=%d, ecclevel=%d, eccbytes=%d\n", __FUNCTION__,
	       chip->eccsize, mtd->writesize, chip->eccsteps, chip->ecclevel, chip->eccbytes);
//udelay(2000000);
	if (chip->eccsteps * chip->eccsize != mtd->writesize) {
		printk(KERN_WARNING "Invalid ecc parameters\n");

//udelay(2000000);
		BUG();
	}
	chip->ecctotal = chip->eccsteps * chip->eccbytes;
	//ECCSIZE(mtd) = chip->eccsize;

	/* Initialize state */
	chip->ctrl->state = BRCMNAND_FL_READY;

#if 0
	/* De-select the device */
	chip->select_chip(mtd, -1);
#endif

	/* Invalidate the pagebuffer reference */
	chip->pagebuf = -1LL;

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;

	/*
	 * Now that we know what kind of NAND it is (SLC vs MLC),
	 * tell the MTD layer how to test it.
	 * ** 01/23/08: Special case: SLC with BCH ECC will be treated as MLC -- at the MTD level --
	 * **                    by the high level test MTD_IS_MLC()
	 * The low level test NAND_IS_MLC() still tells whether the flash is actually SLC or MLC
	 * (so that BBT codes know where to find the BI marker)
	 */
	if (NAND_IS_MLC(chip)) {
		mtd->flags = MTD_CAP_MLC_NANDFLASH;
	}
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
	/*
	 * If controller is version 3 or later, allow SLC to have BCH-n ECC,
	 * -- ONLY IF THE CFE SAYS SO --
	 * in which case, it is treated as if it is an MLC flash by file system codes
	 */
	else if (chip->ecclevel > BRCMNAND_ECC_DISABLE && chip->ecclevel < BRCMNAND_ECC_HAMMING) {
		// CFE wants BCH codes on SLC Nand
		mtd->flags = MTD_CAP_MLC_NANDFLASH;
	}
#endif
	else {
		mtd->flags = MTD_CAP_NANDFLASH;
	}
	//mtd->ecctype = MTD_ECC_SW;
#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
	if (chip->nop == 1)
		mtd->flags |= MTD_NAND_NOP1;
#endif
	PRINTK("300 CS=%d, chip->ctrl->CS[%d]=%d\n", cs, chip->csi, chip->ctrl->CS[chip->csi]);


	mtd->_erase = brcmnand_erase;
	mtd->_point = NULL;
	mtd->_unpoint = NULL;
	mtd->_read = brcmnand_read;
	mtd->_write = brcmnand_write;
	mtd->_read_oob = brcmnand_read_oob;
	mtd->_write_oob = brcmnand_write_oob;

	// Not needed?
	mtd->_writev = brcmnand_writev;

	mtd->_sync = brcmnand_sync;
	mtd->_lock = NULL;
	mtd->_unlock = brcmnand_unlock;
	mtd->_suspend = brcmnand_suspend;
	mtd->_resume = brcmnand_resume;

	mtd->_block_isbad = brcmnand_block_isbad;
	mtd->_block_markbad = brcmnand_block_markbad;

	/* propagate ecc.layout to mtd_info */
	mtd->ecclayout = chip->ecclayout;

	mtd->reboot_notifier.notifier_call = brcmnand_reboot_cb;
	register_reboot_notifier(&mtd->reboot_notifier);

	mtd->owner = THIS_MODULE;







	/*
	 * Clear ECC registers
	 */
	chip->ctrl_write(BCHP_NAND_ECC_CORR_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_ADDR, 0);

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
	chip->ctrl_write(BCHP_NAND_ECC_CORR_EXT_ADDR, 0);
	chip->ctrl_write(BCHP_NAND_ECC_UNC_EXT_ADDR, 0);
#endif


#if 0
	/* Unlock whole block */
	if (mtd->unlock) {
		PRINTK("Calling mtd->unlock(ofs=0, MTD Size=%016llx\n", device_size(mtd));
		mtd->unlock(mtd, 0x0, device_size(mtd));
	}
#endif





//	if (!lastChip)
//		return 0;



//gdebug = 4;
	PRINTK("500 chip=%p, CS=%d, chip->ctrl->CS[%d]=%d\n", chip, cs, chip->csi, chip->ctrl->CS[chip->csi]);
	err =  chip->scan_bbt(mtd);
//gdebug = 0;

//

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING

	if (brcmnand_create_cet(mtd) < 0) {
		printk(KERN_INFO "%s: CET not created\n", __FUNCTION__);
	}
#endif

	PRINTK("%s 99\n", __FUNCTION__);

	return err;

}



#if defined( CONFIG_BCM7401C0 ) || defined( CONFIG_BCM7118A0 )  || defined( CONFIG_BCM7403A0 )
static DEFINE_SPINLOCK(bcm9XXXX_lock);
static unsigned long misb_war_flags;

static inline void
HANDLE_MISB_WAR_BEGIN(void)
{
	/* if it is 7401C0, then we need this workaround */
	if (brcm_ebi_war) {
		spin_lock_irqsave(&bcm9XXXX_lock, misb_war_flags);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
		BDEV_WR(0x00400b1c, 0xFFFF);
	}
}

static inline void
HANDLE_MISB_WAR_END(void)
{
	if (brcm_ebi_war) {
		spin_unlock_irqrestore(&bcm9XXXX_lock, misb_war_flags);
	}
}

#else
#define HANDLE_MISB_WAR_BEGIN()
#define HANDLE_MISB_WAR_END()
#endif


#if 0
/*
 * @ buff		Kernel buffer to hold the data read from the NOR flash, must be able to hold len bytes,
 *			and aligned on word boundary.
 * @ offset	Offset of the data from CS0 (on NOR flash), must be on word boundary.
 * @ len		Number of bytes to be read, must be even number.
 *
 * returns 0 on success, negative error codes on failure.
 *
 * The caller thread may block until access to the NOR flash can be granted.
 * Further accesses to the NAND flash (from other threads) will be blocked until this routine returns.
 * The routine performs the required swapping of CS0/CS1 under the hood.
 */
int brcmnand_readNorFlash(struct mtd_info *mtd, void* buff, unsigned int offset, int len)
{
	struct brcmnand_chip* chip = (struct brcmnand_chip*)mtd->priv;
	int ret = -EFAULT;
	int i;
	int csNand; // Which CS is NAND
	volatile unsigned long cs0Base, cs0Cnfg, cs0BaseAddr, csNandSelect;
	volatile unsigned long csNandBase[MAX_NAND_CS], csNandCnfg[MAX_NAND_CS];
	unsigned int romSize;
	volatile uint16_t* pui16 = (volatile uint16_t*)buff;
	volatile uint16_t* fp;

#if 1
/*
 * THT 03/12/09: This should never be called since the CFE no longer disable CS0
 * when CS1 is on NAND
 */
	printk("%s should never be called\n", __FUNCTION__);
	BUG();
#else

	if (!chip) { // When booting from CRAMFS/SQUASHFS using /dev/romblock
		chip = brcmnand_get_device_exclusive();
		mtd = (struct mtd_info*)chip->priv;
	}else if (brcmnand_get_device(mtd, BRCMNAND_FL_EXCLUSIVE))
		return ret;

	romSize = get_rom_size((unsigned long*)&cs0Base);

	cs0BaseAddr = cs0Base & BCHP_EBI_CS_BASE_0_base_addr_MASK;

	if ((len + offset) > romSize) {
		printk("%s; Attempt to read past end of CS0, (len+offset)=%08x, romSize=%dMB\n",
		       __FUNCTION__, len + offset, romSize >> 20);
		ret = (-EINVAL);
		goto release_device_and_out;
	}

	cs0Cnfg = *(volatile unsigned long*)(0xb0000000 | BCHP_EBI_CS_CONFIG_0);

	// Turn off NAND CS
	for (i = 0; i < chip->ctrl->numchips; i++) {
		csNand = chip->ctrl->CS[i];

		if (csNand == 0) {
			printk("%s: Call this routine only if NAND is not on CS0\n", __FUNCTION__);
			ret = (-EINVAL);
			goto release_device_and_out;
		}

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_1_0
		BUG_ON(csNand > 5);
#else
		BUG_ON(csNand > 7);
#endif
		csNandBase[i] = *(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_BASE_0 + 8 * csNand);
		csNandCnfg[i] = *(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_CONFIG_0 + 8 * csNand);

		// Turn off NAND, must turn off both NAND_CS_NAND_SELECT and CONFIG.
		// We turn off the CS_CONFIG here, and will turn off NAND_CS_NAND_SELECT for all CS at once,
		// outside the loop.
		*(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_CONFIG_0 + 8 * csNand) =
			csNandCnfg[i] & (~BCHP_EBI_CS_CONFIG_0_enable_MASK);

	}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_0_1
	csNandSelect = brcmnand_ctrl_read(BCHP_NAND_CS_NAND_SELECT);

	brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, csNandSelect &
			    ~(
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_1_0
				    BCHP_NAND_CS_NAND_SELECT_EBI_CS_5_SEL_MASK
				    | BCHP_NAND_CS_NAND_SELECT_EBI_CS_4_SEL_MASK
				    | BCHP_NAND_CS_NAND_SELECT_EBI_CS_3_SEL_MASK
				    | BCHP_NAND_CS_NAND_SELECT_EBI_CS_2_SEL_MASK
				    | BCHP_NAND_CS_NAND_SELECT_EBI_CS_1_SEL_MASK
				    | BCHP_NAND_CS_NAND_SELECT_EBI_CS_0_SEL_MASK
#else
				    0x0000003E /* Not documented on V1.0+ */
#endif
				    ));
#endif

	// Turn on NOR on CS0
	*(volatile unsigned long*)(0xb0000000 | BCHP_EBI_CS_CONFIG_0) =
		cs0Cnfg | BCHP_EBI_CS_CONFIG_0_enable_MASK;

	// Take care of MISB Bridge bug on 7401c0/7403a0/7118a0
	HANDLE_MISB_WAR_BEGIN();

	// Read NOR, 16 bits at a time, we have already checked the out-of-bound condition above.
	fp = (volatile uint16_t*)(KSEG1ADDR(cs0BaseAddr + offset));
	for (i = 0; i < (len >> 1); i++) {
		pui16[i] = fp[i];
	}

	HANDLE_MISB_WAR_END();

	// Turn Off NOR
	*(volatile unsigned long*)(0xb0000000 | BCHP_EBI_CS_CONFIG_0) =
		cs0Cnfg & (~BCHP_EBI_CS_CONFIG_0_enable_MASK);

	// Turn NAND back on
	for (i = 0; i < chip->ctrl->numchips; i++) {
		csNand = chip->ctrl->CS[i];
		if (csNand == 0) {
			printk("%s: Call this routine only if NAND is not on CS0\n", __FUNCTION__);
			ret = (-EINVAL);
			goto release_device_and_out;
		}
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_1_0
		BUG_ON(csNand > 5);
#else
		BUG_ON(csNand > 7);
#endif
		*(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_BASE_0 + 8 * csNand) = csNandBase[i];
		*(volatile unsigned long*)(0xb0000000 + BCHP_EBI_CS_CONFIG_0 + 8 * csNand) = csNandCnfg[i];
	}

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_0_1
	// Restore NAND_CS_SELECT
	brcmnand_ctrl_write(BCHP_NAND_CS_NAND_SELECT, csNandSelect);
#endif
	udelay(10000); // Wait for ID Configuration to stabilize

 release_device_and_out:
	brcmnand_release_device(mtd);
//printk("<-- %s\n", __FUNCTION__);

#endif
	return ret;
}
EXPORT_SYMBOL(brcmnand_readNorFlash);
#endif

/**
 * brcmnand_release - [BrcmNAND Interface] Free resources held by the BrcmNAND device
 * @param mtd		MTD device structure
 */
void brcmnand_release(struct mtd_info *mtd)
{
	//struct brcmnand_chip * chip = mtd->priv;

	/* Unregister reboot notifier */
	brcmnand_prepare_reboot_priv(mtd);
	unregister_reboot_notifier(&mtd->reboot_notifier);
	mtd->reboot_notifier.notifier_call = NULL;

	/* Deregister the device (unregisters partitions as well) */
	mtd_device_unregister(mtd);



#if 0
	/* Buffer allocated by brcmnand_scan */
	if (chip->options & NAND_DATABUF_ALLOC)
		kfree(chip->data_buf);

	/* Buffer allocated by brcmnand_scan */
	if (chip->options & NAND_OOBBUF_ALLOC)
		kfree(chip->oob_buf);
#endif

}

#endif // CONFIG_BCM_KF_MTD_BCMNAND
