/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2012:DUAL/GPL:standard 

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>

*/                       

/***************************************************************************
 * File Name  : nandflash.c
 *
 * Description: This file implements the Broadcom DSL defined flash api for
 *              for NAND flash parts.
 ***************************************************************************/

/** Includes. **/

#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "lib_malloc.h"
// #include "bcm_map_part.h"  
#include "bcm_map.h"  
#include "bcmtypes.h"
#include "bcm_hwdefs.h"
#include "flash_api.h"
#include "jffs2.h"
#if defined(CFG_RAMAPP)
#include "cfe_timer.h"
#endif

#if defined(_BCM96838_)
/* Not used on non-BCM963268 chips but prevents compile errors. */
#define NC_BLK_SIZE_2048K       0x60000000
#define NC_BLK_SIZE_1024K       0x50000000
#define NC_BLK_SIZE_256K        0x40000000
#define NC_PG_SIZE_8K           0x00300000
#define NC_PG_SIZE_4K           0x00200000
#define NAC_FAST_PGM_RDIN       0x10000000
#define NAC_PARTIAL_PAGE_EN     0x04000000
#define NAC_ECC_LVL_0_SHIFT     20     
#define NAC_ECC_LVL_0_MASK      0x00f00000
#define NAC_ECC_LVL_SHIFT       16     
#define NAC_ECC_LVL_MASK        0x000f0000
#define NAC_SPARE_SZ_SHIFT      0
#define NAC_SPARE_SZ_MASK       0x0000003f
#define NAC_ECC_LVL_DISABLE     0
#define NAC_ECC_LVL_BCH_1       1
#define NAC_ECC_LVL_BCH_2       2
#define NAC_ECC_LVL_BCH_3       3
#define NAC_ECC_LVL_BCH_4       4
#define NAC_ECC_LVL_BCH_5       5
#define NAC_ECC_LVL_BCH_6       6
#define NAC_ECC_LVL_BCH_7       7
#define NAC_ECC_LVL_BCH_8       8
#define NAC_ECC_LVL_BCH_9       9
#define NAC_ECC_LVL_BCH_10      10
#define NAC_ECC_LVL_BCH_11      11
#define NAC_ECC_LVL_BCH_12      12
#define NAC_ECC_LVL_RESVD_1     13
#define NAC_ECC_LVL_RESVD_2     14
#define NAC_ECC_LVL_HAMMING     15
#define NandSpareAreaReadOfs10  NandSpareAreaReadOfs0
#endif

/* for debugging in jtag */
#if !defined(CFG_RAMAPP)
#define static 
#endif

#ifndef INC_BTRM_BUILD
#define INC_BTRM_BUILD 0
#endif

/** Defines. **/

#define SPARE_MAX_SIZE          (27 * 16)
#define CTRLR_CACHE_SIZE        512

#define NAND_CI_CELLTYPE_MSK    0x00000c00
#define NAND_IS_MLC(chip)               \
    (NAND->NandRevision > 0x00000202 && \
     (chip)->chip_page_size > 512 &&    \
     ((chip)->chip_device_id & NAND_CI_CELLTYPE_MSK))
#define NAND_CHIPID(chip)       ((chip)->chip_device_id >> 16)

/* Flash manufacturers. */
#define FLASHTYPE_SAMSUNG       0xec
#define FLASHTYPE_ST            0x20
#define FLASHTYPE_MICRON        0x2c
#define FLASHTYPE_HYNIX         0xad
#define FLASHTYPE_TOSHIBA       0x98
#define FLASHTYPE_MXIC          0xc2
#define FLASHTYPE_SPANSION      0x01

/* Samsung flash parts. */
#define SAMSUNG_K9F5608U0A      0x55
#define SAMSUNG_K9F1208U0       0x76
#define SAMSUNG_K9F1G08U0       0xf1

/* ST flash parts. */
#define ST_NAND512W3A2CN6       0x76
#define ST_NAND01GW3B2CN6       0xf1

/* Micron flash parts. */
#define MICRON_MT29F1G08AAC     0xf1
#define MICRON_MT29F2G08ABA     0xda
#define MICRON_MT29F4G08ABA     0xdc
#define MICRON_MT29F8G08ABA     0x38
#define MICRON_MT29F8G16ABA     0xd3

/* Hynix flash parts. */
#define HYNIX_H27U1G8F2B        0xf1
#define HYNIX_H27U518S2C        0x76

/* MXIC flash parts */
#define MXIC_MX30LF1208AA       0xf0
#define MXIC_MX30LF1G08AA       0xf1

/* SPANSION flash parts */
#define SPANSION_S34ML01G1      0xf1
#define SPANSION_S34ML02G1      0xda
#define SPANSION_S34ML04G1      0xdc

/* Flash id to name mapping. */
#define NAND_MAKE_ID(A,B)    \
    (((unsigned short) (A) << 8) | ((unsigned short) B & 0xff))

#define NAND_FLASH_DEVICES                                                    \
  {{NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F5608U0A),"Samsung K9F5608U0"},  \
   {NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1208U0),"Samsung K9F1208U0"},   \
   {NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1G08U0),"Samsung K9F1G08U0"},   \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND512W3A2CN6),"ST NAND512W3A2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND01GW3B2CN6),"ST NAND01GW3B2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F1G08AAC),"Micron MT29F1G08AAC"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F2G08ABA),"Micron MT29F2G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F4G08ABA),"Micron MT29F4G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F8G08ABA),"Micron MT29F8G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F8G16ABA),"Micron MT29F8G16ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U1G8F2B),"Hynix H27U1G8F2B"},       \
   {NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U518S2C),"Hynix H27U518S2C"},       \
   {NAND_MAKE_ID(FLASHTYPE_MXIC,MXIC_MX30LF1208AA),"MXIC MX30LF1208AA"},      \
   {NAND_MAKE_ID(FLASHTYPE_MXIC,MXIC_MX30LF1G08AA),"MXIC MX30LF1G08AA"},      \
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML01G1),"Spansion S34ML01G1"},\
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML02G1),"Spansion S34ML02G1"},\
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML04G1),"Spansion S34ML04G1"},\
   {0,""}                                                                     \
  }

#define NAND_FLASH_MANUFACTURERS        \
  {{FLASHTYPE_SAMSUNG, "Samsung"},      \
   {FLASHTYPE_ST, "ST"},                \
   {FLASHTYPE_MICRON, "Micron"},        \
   {FLASHTYPE_HYNIX, "Hynix"},          \
   {FLASHTYPE_TOSHIBA, "Toshiba"},      \
   {FLASHTYPE_MXIC, "MXIC"},            \
   {FLASHTYPE_SPANSION, "Spansion"},    \
   {0,""}                               \
  }

/* Condition to determine the spare layout. */
#define LAYOUT_PARMS(L,S,P)     \
    (((unsigned int)(L)<<28) | ((unsigned int)(S)<<16) | (P))

/* Each bit in the ECCMSK array represents a spare area byte. Bits that are
 * set correspond to spare area bytes that are reserved for the ECC or bad
 * block indicator. Bits that are not set can be used for data such as the
 * JFFS2 clean marker. This macro returns 0 if the spare area byte at offset,
 * OFS, is available and non-0 if it is being used for the ECC or BI.
 */
#define ECC_MASK_BIT(ECCMSK, OFS)   (ECCMSK[OFS / 8] & (1 << (OFS % 8)))

#define SPARE_BI_MARKER         0
#define SPARE_GOOD_MARKER       0xFF

#if defined(__MIPSEL) || defined(__ARMEL__) || defined(__AARCH64EL__)
#define JFFS2_CLEANMARKER      {JFFS2_MAGIC_BITMASK, \
    JFFS2_NODETYPE_CLEANMARKER, 0x0008, 0x0000}
#else
#define JFFS2_CLEANMARKER      {JFFS2_MAGIC_BITMASK, \
    JFFS2_NODETYPE_CLEANMARKER, 0x0000, 0x0008}
#endif


/* ONFI data offsets */

#define ONFI_SIGNATURE                        0
#define ONFI_REVISION                         4
#define ONFI_FEATURES                         6
#define ONFI_OPTIONAL_COMMANDS                8
#define ONFI_RESERVED                         10
#define ONFI_MANUFACTURER                     32
#define ONFI_MODEL                            44
#define ONFI_MANUFACTURER_ID                  64
#define ONFI_DATE                             65
#define ONFI_RESERVED2                        67
#define ONFI_DATA_BYTES_PER_PAGE              80
#define ONFI_SPARE_BYTES_PER_PAGE             84
#define ONFI_DATA_BYTES_PER_PARTIAL_PAGE      86
#define ONFI_SPARE_BYTES_PER_PARTIAL_PAGE     90
#define ONFI_PAGES_PER_BLOCK                  92
#define ONFI_BLOCKS_PER_UNIT                  96
#define ONFI_LOGICAL_UNITS                    100
#define ONFI_ADDRESS_CYCLES                   101
#define ONFI_BITS_PER_CELL                    102
#define ONFI_MAX_BAD_BLOCKS_PER_UNIT          103
#define ONFI_BLOCK_ENDURANCE                  105
#define ONFI_VALID_BLOCKS                     107
#define ONFI_VALID_BLOCKS_ENDURANCE           108
#define ONFI_PROGRAMS_PER_PAGE                110
#define ONFI_PARTIAL_PROGRAM_ATTRIBUTES       111
#define ONFI_ECC_BITS                         112
#define ONFI_INTERLEAVED_ADDRESS_BITS         113
#define ONFI_INTERLEAVED_OPERATION_ATTRIBUTES 114
#define ONFI_RESERVED3                        115
#define ONFI_IO_PIN_CAPACITANCE               128
#define ONFI_TIMING_MODE_SUPPORT              129
#define ONFI_PROGRAM_CACHE_TIMING             131
#define ONFI_MAX_PAGE_PROGRAM_TIME            133
#define ONFI_MAX_BLOCK_ERASE_TIME             135
#define ONFI_MAX_RANDOM_READ_TIME             137
#define ONFI_tCCS_MINIMUM                     139
#define ONFI_RESERVED4                        141
#define ONFI_VENDOR_SPECIFIC_REVISION_NUM     164
#define ONFI_VENDOR_SPECIFIC                  166
#define ONFI_CRC                              254


#undef DEBUG_NAND
#if defined(DEBUG_NAND) && defined(CFG_RAMAPP)
#define DBG_PRINTF printf
#else
#define DBG_PRINTF(...)
#endif

/** Externs. **/

extern void board_setleds(unsigned int);


/** Structs. **/

typedef struct SpareLayout
{
    unsigned char sl_bi_ofs[2];
    unsigned char sl_spare_mask[];
} SPARE_LAYOUT, *PSPARE_LAYOUT;

typedef struct CfeNandChip
{
    char *chip_name;
    unsigned long chip_device_id;
    unsigned long chip_base;
    unsigned long chip_total_size;
    unsigned long chip_block_size;
    unsigned long chip_page_size;
    unsigned long chip_spare_size;
    unsigned long chip_spare_step_size;
    unsigned char *chip_spare_mask;
    unsigned long chip_bi_index_1;
    unsigned long chip_bi_index_2;
} CFE_NAND_CHIP, *PCFE_NAND_CHIP;

static void nand_reset_device(void);
extern inline void nandflash_copy_from_cache(unsigned char *buffer,
    int offset, int numbytes);
extern inline int nandflash_wait_cmd(void);
extern inline int nandflash_wait_cache(void);
#if defined(CFG_RAMAPP)
/** Prototypes for CFE RAM. **/
int nand_flash_init(flash_device_info_t **flash_info);
int mpinand_flash_init(flash_device_info_t **flash_info);
static void nand_init_cleanmarker(PCFE_NAND_CHIP pchip);
static int nand_adjust_cfg(PCFE_NAND_CHIP pchip);
static int nand_is_blk_cleanmarker(PCFE_NAND_CHIP pchip, unsigned long start_addr);
static int nand_is_blk_bad(PCFE_NAND_CHIP pchip, unsigned short blockNum);
static int nand_initialize_spare_area(PCFE_NAND_CHIP pchip, int eraseBadBlocks);
static void nand_mark_bad_blk(PCFE_NAND_CHIP pchip, unsigned long page_addr);
static int nand_flash_sector_erase_int(unsigned short blk);
static int nand_flash_read_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len);
static int nand_flash_write_buf(unsigned short blk, int offset,
    unsigned char *buffer, int numbytes);
static int nand_flash_get_numsectors(void);
static int nand_flash_get_sector_size(unsigned short sector);
static unsigned char *nand_flash_get_memptr(unsigned short sector);
static int nand_flash_get_blk(int addr);
static int nand_flash_get_total_size(void);
static int nand_flash_dev_specific_cmd(unsigned int command, void * inBuf, void * outBuf);
static int nandflash_wait_status(unsigned int status_mask);
extern inline int nandflash_wait_device(void);
extern inline int nandflash_wait_cmd(void);
static int nandflash_read_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len);
static int nandflash_write_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len);
static int nandflash_read_page(PCFE_NAND_CHIP pchip,
    unsigned long start_addr, unsigned char *buffer, int len);
static int nandflash_write_page(PCFE_NAND_CHIP pchip, unsigned long page_addr,
    unsigned int page_offset, unsigned char *buffer, int len);
static int nandflash_block_erase(PCFE_NAND_CHIP pchip, unsigned long blk_addr, int force);
#else

/** Prototypes for CFE ROM. **/
void rom_nand_flash_init(void);
static int nand_is_blk_cleanmarker(PCFE_NAND_CHIP pchip, unsigned long start_addr);
static int nand_adjust_cfg(PCFE_NAND_CHIP pchip);
int nand_flash_get_sector_size(unsigned short sector);
int nand_flash_get_numsectors(void);
static int nandflash_wait_status(unsigned int status_mask);

#if (INC_BTRM_BUILD==1)
int nand_flash_btrm_read_buf(unsigned char *buffer, unsigned int numDups);
static void nandflash_btrm_ecc_setleds(UINT32 cfeRomMirror);
static int nandflash_btrm_read(PCFE_NAND_CHIP pchip, UINT32 page_addr, 
    UINT32 cache_offset);
static int nandflash_btrm_nand_mirroring(PCFE_NAND_CHIP pchip,
    unsigned char *buffer, int len, unsigned long start_addr,
    UINT32 start_blk);
static int nandflash_btrm_skip_bad_block(PCFE_NAND_CHIP pchip,
    unsigned char *buffer, int len);
static int nandflash_btrm_itis_what_itis(PCFE_NAND_CHIP pchip,
    unsigned char *buffer, int len);
static void nandflash_btrm_find_bad_blocks(PCFE_NAND_CHIP pchip,
    unsigned long *start_addr, UINT32 *start_blk);
static int nandflash_btrm_get_content_from_block(PCFE_NAND_CHIP pchip, UINT32 blkNum,
    unsigned char *buffer, int len);
static int nandflash_btrm_check_ecc(void);
static int check_btrm_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step);
#endif

static int nandflash_read_page(PCFE_NAND_CHIP pchip, unsigned long start_addr,
    unsigned char *buffer, int len);
int nand_flash_read_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len);
extern inline void nandflash_copy_from_spare(unsigned char *buffer,
    int numbytes);
static int nandflash_wait_status(unsigned int status_mask);
extern inline int nandflash_wait_device(void);
extern inline int nandflash_wait_spare(void);
static int nandflash_check_ecc(void);
static int check_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step);
static int nandflash_read_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len);
static int nand_is_blk_bad(PCFE_NAND_CHIP pchip, unsigned short blockNum);
#endif


#if defined(CFG_RAMAPP)
/** Variables for CFE RAM. **/
CFE_NAND_CHIP g_chip = {NULL,0,0,0,0,0,0,0,NULL,0,0};
static unsigned char g_spare_cleanmarker[SPARE_MAX_SIZE];

static flash_device_info_t flash_nand_dev =
    {
        0xffff,
        FLASH_IFC_NAND,
        "",
        nand_flash_sector_erase_int, // fn_flash_sector_erase_int in flash_api.c
        nand_flash_read_buf,         // fn_flash_read_buf in flash_api.c
        nand_flash_write_buf,        // fn_flash_write_buf in flash_api.c
        nand_flash_get_numsectors,   // fn_flash_get_numsectors in flash_api.c
        nand_flash_get_sector_size,  // fn_flash_get_sector_size in flash_api.c
        nand_flash_get_memptr,       // fn_flash_get_memptr in flash_api.c
        nand_flash_get_blk,          // fn_flash_get_blk in flash_api.c
        nand_flash_get_total_size,   // fn_flash_get_total_size in flash_api.c
        nand_flash_dev_specific_cmd  // fn_flash_dev_specific_cmd in flash_api.c
    };

// persistant flags for nand_flash_dev_specific_cmd function
static int g_no_ecc = 0;

/* 0,0,0,0,0,B,E,E-E,0,0,0,0,0,0,0 */
SPARE_LAYOUT brcmnand_oob_16 =
    {{5, 5}, {0xe0, 0x01}};

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_64 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_128 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
              0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

/* 0,0,0,0,0,B,0,0-0,E,E,E,E,E,E,E */
SPARE_LAYOUT brcmnand_oob_bch4_512 =
    {{5, 5}, {0x20, 0xfe}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_2k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_4k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe}};

/* B,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch4_8k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
              0x00, 0xfe}};

/* For 63138 the NAND controller use new ECC algorithm that requires more ECC
   bytes. ECC_Bytes_Reqd (per 512 data Bytes) = roundup(ECC_LEVEL * M/8) where
   M is the BCH finite field order. For early chip, M is 13. For 63138, M is 14.
   So it does not affect the Hamming and BCH4. But for BCH8 and BCH12, 63138 use 
   one more byte. On 63138, for BCH8 2K page size, there is not enough spare area 
   for cleanmarker if spare area is 16 bytes. So only the NAND part with 27 bytes 
   spare area is supported   */

#if defined(_BCM963268_) || defined(_BCM96838_)
/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_2k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_2k =
    {{0, 0}, {0x01, 0xc0, 0xff, 0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff,
              0x01, 0x80, 0xff, 0x0f}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_4k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_4k =
    {{0, 0}, {0x01, 0xc0, 0xff, 0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff,
              0x01, 0x80, 0xff, 0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff,
              0x03, 0x00, 0xff, 0x1f, 0x00, 0xf8, 0xff}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_8k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_8k =
    {{0, 0}, {0x01, 0xc0, 0xff, 0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff,
              0x01, 0x80, 0xff, 0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff,
              0x03, 0x00, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0x00, 0xc0, 0xff,
              0x07, 0x00, 0xfe, 0x3f, 0x00, 0xf0, 0xff, 0x01, 0x80, 0xff,
              0x0f, 0x00, 0xfc, 0x7f, 0x00, 0xe0, 0xff, 0x03, 0x00, 0xff,
              0x1f, 0x00, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_2k =
    {{0, 0}, {0x81, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff,
              0x01, 0xff, 0xff, 0x0f}};

/* B,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_4k =
    {{0, 0}, {0x81, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff,
              0x01, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff,
              0x03, 0xfe, 0xff, 0x1f, 0xf0, 0xff, 0xff}};

/* B,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_8k =
    {{0, 0}, {0x81, 0xff, 0xff, 0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff,
              0x01, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff,
              0x03, 0xfe, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x81, 0xff, 0xff,
              0x07, 0xfc, 0xff, 0x3f, 0xe0, 0xff, 0xff, 0x01, 0xff, 0xff,
              0x0f, 0xf8, 0xff, 0x7f, 0xc0, 0xff, 0xff, 0x03, 0xfe, 0xff,
              0x1f, 0xf0, 0xff, 0xff}};


#else
/* B,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_2k =
    {{0, 0}, {0x01, 0xe0, 0xff, 0x07, 0x00, 0xff, 0x3f, 0x00, 0xf8, 0xff,
              0x01, 0xc0, 0xff, 0x0f}};

/* B,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E,-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_4k =
    {{0, 0}, {0xfd, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff,
              0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_4k =
    {{0, 0}, {0x01, 0xe0, 0xff, 0x07, 0x00, 0xff, 0x3f, 0x00, 0xf8, 0xff,
              0x01, 0xc0, 0xff, 0x0f, 0x00, 0xfe, 0x7f, 0x00, 0xf0, 0xff,
              0x03, 0x80, 0xff, 0x1f, 0x00, 0xfc, 0xff}};

/* B,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_8k =
    {{0, 0}, {0xfd, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff,
              0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff,
              0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff,
              0xfc, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,0,0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,0,0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-0,0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,0,0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_8k =
    {{0, 0}, {0x01, 0xe0, 0xff, 0x07, 0x00, 0xff, 0x3f, 0x00, 0xf8, 0xff,
              0x01, 0xc0, 0xff, 0x0f, 0x00, 0xfe, 0x7f, 0x00, 0xf0, 0xff,
              0x03, 0x80, 0xff, 0x1f, 0x00, 0xfc, 0xff, 0x00, 0xe0, 0xff, 
              0x07, 0x00, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0x01, 0xc0, 0xff, 
              0x0f, 0x00, 0xfe, 0x7f, 0x00, 0xf0, 0xff, 0x03, 0x80, 0xff,
              0x1f, 0x00, 0xfc, 0xff}};

/* B,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_2k =
    {{0, 0}, {0xc1, 0xff, 0xff, 0x07, 0xfe, 0xff, 0x3f, 0xf0, 0xff, 0xff,
              0x81, 0xff, 0xff, 0x0f}};

/* B,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_4k =
    {{0, 0}, {0xc1, 0xff, 0xff, 0x07, 0xfe, 0xff, 0x3f, 0xf0, 0xff, 0xff,
              0x81, 0xff, 0xff, 0x0f, 0xfc, 0xff, 0x7f, 0xe0, 0xff, 0xff,
              0x03, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff}};

/* B,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 * 0,0,0,0-0,0,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E
 * 0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E
 * 0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E
 * 0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch12_27_8k =
    {{0, 0}, {0xc1, 0xff, 0xff, 0x07, 0xfe, 0xff, 0x3f, 0xf0, 0xff, 0xff,
              0x81, 0xff, 0xff, 0x0f, 0xfc, 0xff, 0x7f, 0xe0, 0xff, 0xff,
              0x03, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0xc0, 0xff, 0xff,
              0x07, 0xfe, 0xff, 0x3f, 0xf0, 0xff, 0xff, 0x81, 0xff, 0xff, 
              0x0f, 0xfc, 0xff, 0x7f, 0xe0, 0xff, 0xff, 0x03, 0xff, 0xff, 
              0x1f, 0xf8, 0xff, 0xff}};

#endif

#else
/** Variables for CFE ROM. **/
/* DO NOT define any global variables, static variable and
 * any literal strings for CFE ROM code. It is shared by
 * the CFE util which does not load all the data segments
 * because the total cfe util size exceeds 16KB NAND limitation.
 * Global variable are defined in the init_utils_mips.S to make
 * sure they are within 16KB.
 */
#ifdef CFG_BOOTUTILS
extern
#endif
CFE_NAND_CHIP g_chip;

#endif



static void nand_reset_device(void)
{
    NAND->NandNandBootConfig = NBC_AUTO_DEV_ID_CFG | 1;
#if (INC_SPI_FLASH_DRIVER==1)
    cfe_usleep(1000);
#endif
    nandflash_wait_device();
    NAND->NandCsNandXor = 0;

    return;
}

#if defined(CFG_RAMAPP)
/***************************************************************************
 * Function Name: nand_flash_init
 * Description  : Initialize flash part.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int nand_flash_init(flash_device_info_t **flash_info)
{
    static int initialized = 0;
    int ret = FLASH_API_OK;

    if( initialized == 0 )
    {
        PCFE_NAND_CHIP pchip = &g_chip;
        static struct flash_name_from_id fnfi[] = NAND_FLASH_DEVICES;
        static struct flash_name_from_id fmfi[] = NAND_FLASH_MANUFACTURERS;
        struct flash_name_from_id *fnfi_ptr;

        DBG_PRINTF(">> nand_flash_init - entry\n");

        /* Enable NAND data on MII ports. */
#if defined(_BCM963268_)
        PERF->blkEnables |= NAND_CLK_EN;
#endif
#if defined(_BCM963268_) && (INC_SPI_FLASH_DRIVER==1)
        GPIO->GPIOBaseMode |= NAND_GPIO_OVERRIDE;
#endif

#if (INC_SPI_FLASH_DRIVER==1)
        {
            unsigned int acc = 0; 

            /* when boot from SPI, the nand controller always set ECC to hamming. update it based on strap */
            acc = NAND->NandAccControl;
#if defined(_BCM963268_)
            acc &= ~(NAC_ECC_LVL_MASK|NAC_ECC_LVL_0_MASK);
            switch(MISC->miscStrapBus&MISC_STRAP_BUS_NAND_ECC_SELECT_MASK)
            {
            case MISC_STRAP_BUS_NAND_ECC_8_BIT:
                 acc |= (NAC_ECC_LVL_BCH_8 << NAC_ECC_LVL_SHIFT)|(NAC_ECC_LVL_BCH_8 << NAC_ECC_LVL_0_SHIFT);
                 acc &= ~(NAC_SPARE_SZ_MASK|NAC_SPARE_SZ_0_MASK);
                 acc |= (27<<NAC_SPARE_SZ_SHIFT)|(27<<NAC_SPARE_SZ_0_SHIFT);
                 break;

            case MISC_STRAP_BUS_NAND_ECC_4_BIT:
                 acc |= (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_SHIFT)|(NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_0_SHIFT);
                 break;

            case MISC_STRAP_BUS_NAND_ECC_1_BIT:
                 acc |= (NAC_ECC_LVL_HAMMING << NAC_ECC_LVL_SHIFT)|(NAC_ECC_LVL_HAMMING << NAC_ECC_LVL_0_SHIFT);
                 break;

            default:
                 printf("NAND ECC setting not supported\n");
                 *flash_info = NULL;
                 return FLASH_API_ERROR;
            }

#elif !defined(_BCM96838_) && !defined(_BCM949178_)
            acc &= ~NAC_ECC_LVL_MASK;
            switch(MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_ECC_MASK)
            {
            case MISC_STRAP_BUS_BOOT_NAND_ECC_12_BIT:
                 acc |= (NAC_ECC_LVL_BCH_12 << NAC_ECC_LVL_SHIFT);
                 acc &= ~NAC_SPARE_SZ_MASK;
                 acc |= (27<<NAC_SPARE_SZ_SHIFT);
                 break;

            case MISC_STRAP_BUS_BOOT_NAND_ECC_8_BIT:
                 acc |= (NAC_ECC_LVL_BCH_8 << NAC_ECC_LVL_SHIFT);
                 acc &= ~NAC_SPARE_SZ_MASK;
                 acc |= (27<<NAC_SPARE_SZ_SHIFT);
                 break;

            case MISC_STRAP_BUS_BOOT_NAND_ECC_4_BIT:
                 acc |= (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_SHIFT);
                 break;

            case MISC_STRAP_BUS_BOOT_NAND_ECC_1_BIT:
                 acc |= (NAC_ECC_LVL_HAMMING << NAC_ECC_LVL_SHIFT);
                 break;

            default:
                 printf("NAND ECC setting not supported\n");
                 *flash_info = NULL;
                 return FLASH_API_ERROR;
            }
#endif


            NAND->NandAccControl = acc;
        }
#endif

        nand_reset_device();

        NAND->NandCmdAddr = 0;
        NAND->NandCmdExtAddr = 0;
        NAND->NandCmdStart = NCMD_DEV_ID_READ;
        nandflash_wait_cmd();

        /* Read the NAND flash chip id. */
        pchip->chip_device_id = NAND->NandFlashDeviceId;
        flash_nand_dev.flash_device_id = NAND_CHIPID(pchip);

        for( fnfi_ptr = fnfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ )
        {
            if( fnfi_ptr->fnfi_id == flash_nand_dev.flash_device_id )
            {
                strcpy(flash_nand_dev.flash_device_name, fnfi_ptr->fnfi_name);
                break;
            }
        }

        /* If NAND chip is not in the list of NAND chips, try to identify the
         * manufacturer.
         */
        if( flash_nand_dev.flash_device_name[0] == '\0' )
        {
            for( fnfi_ptr = fmfi; fnfi_ptr->fnfi_id != 0; fnfi_ptr++ )
            {
                if( fnfi_ptr->fnfi_id == flash_nand_dev.flash_device_id )
                {
                    strcpy(flash_nand_dev.flash_device_name, fnfi_ptr->fnfi_name);
                    break;
                }
            }
        }

        *flash_info = &flash_nand_dev;

        pchip->chip_base = 0;
        ret = nand_adjust_cfg(pchip);
        if( ret != FLASH_API_OK )
	{
            *flash_info = NULL;
	    return ret;
	}
        nand_init_cleanmarker(pchip);

        DBG_PRINTF(">> nand_flash_init - return %d\n", ret);

        initialized = 1;
    }
    else
        *flash_info = &flash_nand_dev;

    return( ret );
} /* nand_flash_init */

/***************************************************************************
 * Function Name: nand_init_cleanmarker
 * Description  : Initializes the JFFS2 clean marker buffer.
 * Returns      : None.
 ***************************************************************************/
static void nand_init_cleanmarker(PCFE_NAND_CHIP pchip)
{
    unsigned short cleanmarker[] = JFFS2_CLEANMARKER;
    unsigned char *pcm = (unsigned char *) cleanmarker;
    unsigned char *spare_mask = pchip->chip_spare_mask;
    int i, j;

    /* Skip spare area offsets reserved for ECC bytes. */
    for( i = 0, j = 0; i < pchip->chip_spare_size; i++ )
    {
        if( ECC_MASK_BIT(spare_mask, i) == 0 && j < sizeof(cleanmarker))
            g_spare_cleanmarker[i] = pcm[j++];
        else
            g_spare_cleanmarker[i] = 0xff;
    }
} /* nand_init_cleanmarker */

#else
/***************************************************************************
 * Function Name: rom_nand_flash_init
 * Description  : Initialize flash part just enough to read blocks.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
void rom_nand_flash_init(void)
{
    PCFE_NAND_CHIP pchip = &g_chip;

    /* Enable NAND data on MII ports. */
#if defined(_BCM963268_)
    PERF->blkEnables |= NAND_CLK_EN;
#endif

    nand_reset_device();
    pchip->chip_base = 0;

    /* Read the chip id. Only use the most signficant 16 bits. */
    pchip->chip_device_id = NAND->NandFlashDeviceId;
    nand_adjust_cfg(pchip);

} /* nand_flash_init */
#endif

/***************************************************************************
 * Function Name: nand_adjust_cfg
 * Description  : Reads and stores the chip configuration.
 * Returns      : None.
 ***************************************************************************/
static int nand_adjust_cfg(PCFE_NAND_CHIP pchip)
{
    /* Read chip configuration. */
    unsigned int cfg = NAND->NandConfig;
    int ret = FLASH_API_OK;

    /* Special case changes from what the NAND controller configured. */
    switch(NAND_CHIPID(pchip))
    {
    case NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U1G8F2B):
        cfg &= ~(NC_DEV_SIZE_MASK | NC_FUL_ADDR_MASK | (0xff << NC_BLK_ADDR_SHIFT));

        /* 128 MB device size, 4 full address bytes, 2 column address bytes, 2 block address bytes */
        cfg |= (5 << NC_DEV_SIZE_SHIFT) | (0x04 << NC_FUL_ADDR_SHIFT) | (0x22 << NC_BLK_ADDR_SHIFT);
        NAND->NandConfig = cfg;
        break;

    case NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F5608U0A):
    case NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1208U0):
    case NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1G08U0):
    case NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U518S2C):
        /* Set device id "cell type" to 0 (SLC). */
        pchip->chip_device_id &= ~NAND_CI_CELLTYPE_MSK;
        NAND->NandFlashDeviceId = pchip->chip_device_id;
        break;

    case NAND_MAKE_ID(FLASHTYPE_MXIC,MXIC_MX30LF1208AA):
        /* This 64MB device was detected as 256MB device on 63268. Manually update
         * device size in the cfg register.
         */
        cfg &= ~NC_DEV_SIZE_MASK;
        cfg |= (0x04<<NC_DEV_SIZE_SHIFT);
        NAND->NandConfig = cfg;
        break;

    case NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML01G1):
        /* Set device size to 128MB, it is misconfigured to 512MB. */
        cfg &= ~NC_DEV_SIZE_MASK;
        cfg |= (0x05<<NC_DEV_SIZE_SHIFT);
        NAND->NandConfig = cfg;
        break;

    case NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML02G1):
        /* Set device size to 256MB, it is misconfigured to 512MB. */
        cfg &= ~NC_DEV_SIZE_MASK;
        cfg |= (0x06<<NC_DEV_SIZE_SHIFT);
        NAND->NandConfig = cfg;
        break;

    case NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML04G1):
        /* Set block size to 128KB, it is misconfigured to 512MB. */
#if defined (_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || defined(_BCM963138_) || defined(_BCM947189_)
        cfg &= ~NC_BLK_SIZE_MASK;
        cfg |= NC_BLK_SIZE_128K;
        NAND->NandConfig = cfg;
#endif
        break;

#if defined(CONFIG_BRCM_IKOS)
    case 0x2cda:
    case 0x2cdc:
        /* Set device id "cell type" to 0 (SLC). */
        pchip->chip_device_id &= ~NAND_CI_CELLTYPE_MSK;

        /* simulate only 4MB */
        cfg &= ~NC_DEV_SIZE_MASK;
        NAND->NandConfig = cfg;
        printf("Simulated Micron NAND device id: 0x%x\n", NAND_CHIPID(pchip));
        break;

    case 0x2c75:
        /* simulate only 4MB */
        cfg &= ~NC_DEV_SIZE_MASK;
        NAND->NandConfig = cfg;
      
        cfg = NAND->NandConfigExt;
        cfg &= ~NC_BLK_SIZE_MASK;
        cfg |= NC_BLK_SIZE_16K;
        NAND->NandConfigExt = cfg;

        printf("Simulated Micron NAND device id: 0x%x\n", NAND_CHIPID(pchip));
        break;
#endif
    }

    pchip->chip_total_size =
        (4 * (1 << ((cfg & NC_DEV_SIZE_MASK) >> NC_DEV_SIZE_SHIFT))) << 20;

    /* block size and page size move to ext config register in NAND controller rev7.1 and later */
#if defined(_BCM963148_) || defined(_BCM96848_) || defined(_BCM94908_) || defined(_BCM96858_) || \
    defined(_BCM963158_) || defined(_BCM96846_) || defined(_BCM96856_) || defined(_BCM963178_) || defined(_BCM947622_) || defined(_BCM96878_)
    cfg = NAND->NandConfigExt;
#endif
    switch( (cfg & NC_BLK_SIZE_MASK) )
    {
    case NC_BLK_SIZE_2048K:
        pchip->chip_block_size = 2048 * 1024;
        break;

    case NC_BLK_SIZE_1024K:
        pchip->chip_block_size = 1024 * 1024;
        break;

    case NC_BLK_SIZE_512K:
        pchip->chip_block_size = 512 * 1024;
        break;

    case NC_BLK_SIZE_256K:
        pchip->chip_block_size = 256 * 1024;
        break;

    case NC_BLK_SIZE_128K:
        pchip->chip_block_size = 128 * 1024;
        break;

    case NC_BLK_SIZE_16K:
        pchip->chip_block_size = 16 * 1024;
        break;

    case NC_BLK_SIZE_8K:
        pchip->chip_block_size = 8 * 1024;
        break;
    }

    switch( (cfg & NC_PG_SIZE_MASK) )
    {
    case NC_PG_SIZE_8K:
        pchip->chip_page_size = 8 * 1024;
        break;

    case NC_PG_SIZE_4K:
        pchip->chip_page_size = 4 * 1024;
        break;

    case NC_PG_SIZE_2K:
        pchip->chip_page_size = 2 * 1024;
        break;

    case NC_PG_SIZE_512B:
        pchip->chip_page_size = 512;
        break;
    }

#if defined(CFG_RAMAPP)
    {  
        unsigned int acc;
        unsigned int acc_save;

        acc = NAND->NandAccControl;
        acc_save = acc;

        /* always treat parallel NAND flash parts as requiring the whole page to be written at once for new chips */
#if !defined(_BCM963268_) && !defined(_BCM96838_)
        if( (acc & NAC_PARTIAL_PAGE_EN) != 0 )
#else
        if( (acc & (NAC_FAST_PGM_RDIN | NAC_PARTIAL_PAGE_EN)) != 0 )
#endif
        {
            DBG_PRINTF("Disable NAND fast program read and partial page enable\n");
#if !defined(_BCM963268_) && !defined(_BCM96838_)
            acc &= ~NAC_PARTIAL_PAGE_EN;
#else
            acc &= ~(NAC_FAST_PGM_RDIN | NAC_PARTIAL_PAGE_EN);
#endif
        }

        if (NAND_IS_MLC(pchip) && (((acc & NAC_ECC_LVL_MASK)>>NAC_ECC_LVL_SHIFT)==NAC_ECC_LVL_HAMMING))
        {
            DBG_PRINTF("Changing ECC to BCH-4 from Hamming for MLC part\n");
#if !defined(_BCM963268_) && !defined(_BCM96838_)
            acc &= ~NAC_ECC_LVL_MASK;
            acc |= (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_SHIFT);
#else
            acc &= ~(NAC_ECC_LVL_0_MASK | NAC_ECC_LVL_MASK);
            acc |= (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_0_SHIFT) |
                 (NAC_ECC_LVL_BCH_4 << NAC_ECC_LVL_SHIFT);
#endif
        }

#if defined(NAC_PREFETCH_EN)  && !defined(_BCM947189_)
        /* turn on prefetch bit for performance*/
        if( NAND->NandRevision >= 0x00000700 && pchip->chip_page_size > 512 )
            acc |= NAC_PREFETCH_EN;
#endif
        if( acc != acc_save )
        {
            NAND->NandAccControl = acc;
            DBG_PRINTF("NAND Access Control register = 0x%8.8lx, was 0x%8.8lx\n", acc, acc_save);
        }
    }

#if !(INC_BTRM_BOOT==1) && !defined(CONFIG_BRCM_IKOS)
    if( (NAND->NandRevision & 0xffff) >= 0x00000400 )
    {
        /* adjust reading timing */
        /* Default of TRP=4 and TREAD=5 for Hynix parts on 63268 */
        /* Almost all parts could use TRP=3 and TREAD=4 */
        NAND->NandTiming1 &= ~(NT_TREH_MASK|NT_TRP_MASK);
        NAND->NandTiming1 |= (2<<NT_TREH_SHIFT)|(4<<NT_TRP_SHIFT);
        NAND->NandTiming2 &= ~NT_TREAD_MASK;
        NAND->NandTiming2 |= (5<<NT_TREAD_SHIFT);
    }
#endif

    if( NAND->NandRevision <= 0x00000202 )
    {
        PSPARE_LAYOUT spare_layout = NULL;

        pchip->chip_spare_step_size = 16;
        pchip->chip_spare_size = pchip->chip_page_size >> 5;
        switch(pchip->chip_page_size)
        {
        case 2048:
            spare_layout = &brcmnand_oob_64;
            break;

        case 4096:
            spare_layout = &brcmnand_oob_128;
            break;

        /* case 512: */
        default:
            spare_layout = &brcmnand_oob_16;
            break;
        }

        pchip->chip_spare_mask = spare_layout->sl_spare_mask;
        pchip->chip_bi_index_1 = spare_layout->sl_bi_ofs[0];
        pchip->chip_bi_index_2 = spare_layout->sl_bi_ofs[1];
    }
    else
    {
        PSPARE_LAYOUT spare_layout = NULL;
        unsigned int acc = NAND->NandAccControl;
        unsigned int ecc_lvl = (acc & NAC_ECC_LVL_MASK) >> NAC_ECC_LVL_SHIFT;
        unsigned int layout_parms;

        /* The access control register spare size is the number of spare area
         * bytes per 512 bytes of data.  The chip_spare_size is the number
         * of spare area bytes per page.
         */
        pchip->chip_spare_step_size =
            ((acc & NAC_SPARE_SZ_MASK) >> NAC_SPARE_SZ_SHIFT);
        pchip->chip_spare_size = pchip->chip_spare_step_size *
            (pchip->chip_page_size >> 9);

        layout_parms =
            LAYOUT_PARMS(ecc_lvl, pchip->chip_spare_size, pchip->chip_page_size);
        DBG_PRINTF(">> nand_adjust_cfg - layout_parms=0x%8.8lx\n", layout_parms);
        if( ecc_lvl == NAC_ECC_LVL_DISABLE )
	{
#if defined(CONFIG_BRCM_IKOS)
            ecc_lvl = NAC_ECC_LVL_HAMMING;
            printf("ECC force to hamming for IKOS\n ");
            layout_parms = LAYOUT_PARMS(ecc_lvl, pchip->chip_spare_size, pchip->chip_page_size);
#else
            printf("ECC Disabled is not supported\n ");
            return FLASH_API_ERROR;
#endif
	}
        else if( ecc_lvl == NAC_ECC_LVL_HAMMING )
            printf("NAND ECC Hamming, ");
        else
            printf("NAND ECC BCH-%d, ", ecc_lvl);
        printf("page size 0x%x bytes, spare size used %d bytes\n", pchip->chip_page_size, pchip->chip_spare_size);
        switch(layout_parms)
        {
        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,16,512)):
            spare_layout = &brcmnand_oob_16;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,64,2048)):
            spare_layout = &brcmnand_oob_64;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,128,4096)):
            spare_layout = &brcmnand_oob_128;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,16,512)):
            spare_layout = &brcmnand_oob_bch4_512;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,64,2048)):
            spare_layout = &brcmnand_oob_bch4_2k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,128,4096)):
            spare_layout = &brcmnand_oob_bch4_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,256,8192)):
            spare_layout = &brcmnand_oob_bch4_8k;
            break;

#if defined(_BCM963268_) || defined(_BCM96838_)
        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,64,2048)):
            spare_layout = &brcmnand_oob_bch8_16_2k;
            break;
#endif
        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,108,2048)):
            spare_layout = &brcmnand_oob_bch8_27_2k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,128,4096)):
            spare_layout = &brcmnand_oob_bch8_16_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,216,4096)):
            spare_layout = &brcmnand_oob_bch8_27_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,256,8192)):
            spare_layout = &brcmnand_oob_bch8_16_8k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,432,8192)):
            spare_layout = &brcmnand_oob_bch8_27_8k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,108,2048)):
            spare_layout = &brcmnand_oob_bch12_27_2k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,216,4096)):
            spare_layout = &brcmnand_oob_bch12_27_4k;
            break;

        case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,432,8192)):
            spare_layout = &brcmnand_oob_bch12_27_8k;
            break;

        default:
            printf("No valid spare layout for level=%lu, spare size=%lu,"
                " page size=%lu\n", ecc_lvl, pchip->chip_spare_size,
                  pchip->chip_page_size);
            return FLASH_API_ERROR;
        }

        pchip->chip_spare_mask = spare_layout->sl_spare_mask;
        pchip->chip_bi_index_1 = spare_layout->sl_bi_ofs[0];
        pchip->chip_bi_index_2 = spare_layout->sl_bi_ofs[1];
    }

    DBG_PRINTF(">> nand_adjust_cfg - size=%luMB, block=%luKB, page=%luB, "
        "spare=%lu\n", pchip->chip_total_size / (1024 * 1024),
        pchip->chip_block_size / 1024, pchip->chip_page_size,
        pchip->chip_spare_size);
#else
    /* for cfe rom, only need to know the bbi location */
    {
        uint32_t acc = NAND->NandAccControl;
        uint32_t ecc_lvl = (acc & NAC_ECC_LVL_MASK) >> NAC_ECC_LVL_SHIFT;

        /* The access control register spare size is the number of spare area
         * bytes per 512 bytes of data.  The chip_spare_size is the number
         * of spare area bytes per page.
         */
        pchip->chip_spare_step_size =
            ((acc & NAC_SPARE_SZ_MASK) >> NAC_SPARE_SZ_SHIFT);
        pchip->chip_spare_size = pchip->chip_spare_step_size *
            (pchip->chip_page_size >> 9);

        if( ecc_lvl == NAC_ECC_LVL_HAMMING ) {
            if( pchip->chip_page_size == 512 ) {
                pchip->chip_bi_index_1 = pchip->chip_bi_index_2 = 5;
            }
            else {
                pchip->chip_bi_index_1 = 0;
                pchip->chip_bi_index_2 = 1;
            }
        }
        else if( ecc_lvl == NAC_ECC_LVL_BCH_4 && pchip->chip_page_size == 512 ) {
            pchip->chip_bi_index_1 = pchip->chip_bi_index_2 = 5;
        }
        else
            pchip->chip_bi_index_1 = pchip->chip_bi_index_2 = 0;
    }
#endif

    NAND->NandCmdStart = NCMD_PARAM_READ;

    if( ( nandflash_wait_cmd() == FLASH_API_OK ) && ( nandflash_wait_cache() == FLASH_API_OK ) )
    { // Hardware NAND controller does not take into account LUNs, so if this value is not 1 we calculate the die stack NAND size
        unsigned char onfi[256];

        nandflash_copy_from_cache(onfi, 0, 256);

        if ( (onfi[ONFI_SIGNATURE] == 'O') && (onfi[ONFI_SIGNATURE+1] == 'N') && (onfi[ONFI_SIGNATURE+2] == 'F') && (onfi[ONFI_SIGNATURE+3] == 'I') && (onfi[ONFI_LOGICAL_UNITS] != 1) )
        { // adjust size based on # of luns
            unsigned int cfg;
            unsigned int size = ((onfi[ONFI_DATA_BYTES_PER_PAGE+3] << 24) | (onfi[ONFI_DATA_BYTES_PER_PAGE+2] << 16) | (onfi[ONFI_DATA_BYTES_PER_PAGE+1] << 8) | (onfi[ONFI_DATA_BYTES_PER_PAGE])) * // bytes per page
                                ((onfi[ONFI_PAGES_PER_BLOCK+3] << 24) | (onfi[ONFI_PAGES_PER_BLOCK+2] << 16) | (onfi[ONFI_PAGES_PER_BLOCK+1] << 8) | (onfi[ONFI_PAGES_PER_BLOCK])) * // pages per block
                                ((onfi[ONFI_BLOCKS_PER_UNIT+3] << 24) | (onfi[ONFI_BLOCKS_PER_UNIT+2] << 16) | (onfi[ONFI_BLOCKS_PER_UNIT+1] << 8) | (onfi[ONFI_BLOCKS_PER_UNIT])) * // blocks per LUN
                                 (onfi[ONFI_LOGICAL_UNITS]);  // LUNs
            DBG_PRINTF("ONFI detected, old size 0x%x, new size 0x%x\n", pchip->chip_total_size, size);
            pchip->chip_total_size = size;

            cfg = 0;
            size >>= 23;

            while(size)
            {
                cfg++;
                size >>= 1;
            }

            NAND->NandConfig = (NAND->NandConfig & ~NC_DEV_SIZE_MASK) | (cfg << NC_DEV_SIZE_SHIFT);
        }
    }

    return ret;
} /* nand_adjust_cfg */

/* Check if the block is good or bad. If bad returns 1, if good returns 0 */
static int nand_is_blk_bad(PCFE_NAND_CHIP pchip, unsigned short blockNum)
{
#ifdef CONFIG_BRCM_IKOS
    /* spare area not supported in sim/emu environment */
    return 0; // good block
#else
#if defined(CFG_RAMAPP)
    unsigned char *spare;
#else
    /* CFE ROM does not have real heap. KMALLOC simply use mem_heapstart memory. 
     * as caller of this function already use mem_heapstart, we can't use heap again
     * must switch to stack. KFREE is a nop for CFE ROM */
    unsigned char spare[SPARE_MAX_SIZE];
#endif
    UINT32 page_addr = (blockNum * pchip->chip_block_size) &  ~(pchip->chip_page_size - 1);
    int i;


    if (blockNum == 0)
        return 0; // always return good for block 0, because if it's a bad chip quite possibly the board is useless

#if defined(CFG_RAMAPP)
    spare = KMALLOC(SPARE_MAX_SIZE, 0);
#endif

    /* Read the spare area of first page and check for bad block indicator */
    for (i=0; i<2; i+=1, page_addr += pchip->chip_page_size)
    {
        if (nandflash_read_spare_area(pchip,  page_addr, spare, pchip->chip_spare_size) == FLASH_API_OK)
        {
            if ( (spare[pchip->chip_bi_index_1] != SPARE_GOOD_MARKER) || (spare[pchip->chip_bi_index_2] != SPARE_GOOD_MARKER))
            {
                KFREE(spare);
                return 1; // bad block
            }
        }
        else
        {
            KFREE(spare);
            return 1;//bad block
        }
    }

    KFREE(spare);
    return 0; // good block
#endif
}

#if !defined(CFG_RAMAPP)
/* This function always returns that a NAND flash block spare area contains a
 * JFFS2 clean marker for the CFE ROM build.  It does not do the validation in
 * order to save code and data space.  However, the CFE ROM boot code verifies
 * that the first two bytes of each NAND flash data block contain the JFFS2
 * magic number so there is NAND flash block validation.
 */

static int nand_is_blk_cleanmarker(PCFE_NAND_CHIP pchip, unsigned long start_addr)
{
    /* Always return TRUE in CFE ROM. */
    return(1);
}
#else
/***************************************************************************
 * Function Name: nand_is_blk_cleanmarker
 * Description  : Compares a buffer to see if it a JFFS2 cleanmarker.
 * Returns      : 1 - is cleanmarker, 0 - is not cleanmarker
 ***************************************************************************/
static int nand_is_blk_cleanmarker(PCFE_NAND_CHIP pchip, unsigned long start_addr)
{
    unsigned short cleanmarker[] = JFFS2_CLEANMARKER;
    unsigned char *pcm = (unsigned char *) cleanmarker;
    unsigned char *spare = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned char *comparebuf = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned char *spare_mask = pchip->chip_spare_mask;
    unsigned int i, j;
    int ret = 1;

    if( nandflash_read_spare_area( pchip, start_addr, spare,
        pchip->chip_spare_size) == FLASH_API_OK )
    {
        /* Skip spare offsets that are reserved for the ECC.  Make spare data
         * bytes contiguous in the spare buffer.
         */
        for( i = 0, j = 0; i < pchip->chip_spare_size; i++ )
            if( ECC_MASK_BIT(spare_mask, i) == 0 )
                comparebuf[j++] = spare[i];

        /* Compare spare area data to the JFFS2 cleanmarker. */
        for( i = 0, ret = 1; i < sizeof(cleanmarker) && ret == 1; i++ )
            if( comparebuf[i] != pcm[i])
                ret = 0;
    }

    if( (spare[pchip->chip_bi_index_1] != SPARE_GOOD_MARKER) ||
        (spare[pchip->chip_bi_index_2] != SPARE_GOOD_MARKER)  )
    {
        /* This is a bad block */
        ret = 0;   
    }

    KFREE(spare);
    KFREE(comparebuf);

    return( ret );
} /* nand_is_blk_cleanmarker */


/***************************************************************************
 * Function Name: nand_initialize_spare_area
 * Description  : Initializes the spare area of the first page of each block 
 *                to a cleanmarker.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nand_initialize_spare_area(PCFE_NAND_CHIP pchip, int eraseBadBlocks)
{
    unsigned char *spare = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned int i;
    int ret;

    DBG_PRINTF(">> nand_initialize_spare_area - entry\n");

    for( i = 0; i < pchip->chip_total_size; i += pchip->chip_block_size )
    {
        /* Read the current spare area. */
        ret = nandflash_read_spare_area(pchip,i,spare,pchip->chip_spare_size);
        if(ret == FLASH_API_OK)   
        {
            /* if eraseBadBlocks is set, erase bad blocks, else check spare area to see if 
             * the block is good and then erase.
             */
            int writeSpare = eraseBadBlocks || 
                            ( (spare[pchip->chip_bi_index_1] == SPARE_GOOD_MARKER)
                              && (spare[pchip->chip_bi_index_2] == SPARE_GOOD_MARKER) );
            if( writeSpare && (nandflash_block_erase(pchip, i, eraseBadBlocks) != FLASH_API_OK) )
            {
                printf("\nInitialization spare area error blk=%d writeSpare=%d\n", i/pchip->chip_block_size, writeSpare);
            }
        }
    }

    KFREE(spare);
    return( FLASH_API_OK );
} /* nand_initialize_spare_area */


/***************************************************************************
 * Function Name: nand_mark_bad_blk
 * Description  : Marks the specified block as bad by writing 0xFFs to the
 *                spare area and updating the in memory bad block table.
 * Returns      : None.
 ***************************************************************************/
static void nand_mark_bad_blk(PCFE_NAND_CHIP pchip, unsigned long page_addr)
{
    static int marking_bad_blk = 0;
    unsigned char *spare;
    unsigned int blk = page_addr/pchip->chip_block_size;
    
    if (nand_is_blk_bad(pchip, blk))
    {
        printf("nand_mark_bad_blk() : Attempting to mark a bad block %d that is already bad.\n", blk);
        return ;
    }

    spare = KMALLOC(SPARE_MAX_SIZE, 0);

    if( marking_bad_blk == 0 )
    {
        marking_bad_blk = 1;
        DBG_PRINTF(">> nand_mark_bad_blk - addr=0x%8.8lx, block=0x%8.8lx\n",
            page_addr, page_addr / pchip->chip_block_size);

        nandflash_block_erase(pchip, page_addr, 0);
        memset(spare, 0xff, pchip->chip_spare_size);
        spare[pchip->chip_bi_index_1] = SPARE_BI_MARKER;
        spare[pchip->chip_bi_index_2] = SPARE_BI_MARKER;
        nandflash_write_spare_area(pchip,page_addr,spare,pchip->chip_spare_size);

#if (INC_BTRM_BOOT==1) && defined(_BCM963268_)
        if( page_addr / pchip->chip_block_size > 2 )
#endif
        {
            /* Also mark the first page in the block as bad so the Linux NAND
             * flash driver finds it when updating the BBT.
             */
            page_addr &= ~(pchip->chip_block_size - 1);
            nandflash_write_spare_area(pchip, page_addr, spare,
                pchip->chip_spare_size);

            /* Mark second page of the block as well */
            page_addr += pchip->chip_page_size;
            nandflash_write_spare_area(pchip, page_addr, spare,
                pchip->chip_spare_size);
        }
        marking_bad_blk = 0;
    }

    KFREE(spare);
} /* nand_mark_bad_blk */


/***************************************************************************
 * Function Name: nand_flash_sector_erase_int
 * Description  : Erase the specfied flash sector.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nand_flash_sector_erase_int(unsigned short blk)
{
    int ret = FLASH_API_OK;
    PCFE_NAND_CHIP pchip = &g_chip;

    {
        unsigned long page_addr = blk * pchip->chip_block_size;

        if( (ret = nandflash_block_erase(pchip, page_addr, 0)) != FLASH_API_OK )
            printf("\nError erasing flash block, blk=%d\n", blk);
#if defined(_BCM963138_) || defined(_BCM963148_)
        else
        {
            /* write clean marker in block 0 page 0 for backward compatiblity for 416L01 in 63148/63138 as 
               these chip's cfe rom/nvram starts from 64KB offset. When everybody move to 416L02, we can remove this code */
            if( page_addr == 0 )
                ret = nandflash_write_spare_area(pchip, page_addr, g_spare_cleanmarker, pchip->chip_spare_size);
        }
#endif
    }

    return( ret );
} /* nand_flash_sector_erase_int */
#endif

/***************************************************************************
 * Function Name: nand_flash_read_buf
 * Description  : Reads from flash memory.
 * Returns      : number of bytes read or FLASH_API_ERROR
 ***************************************************************************/
#if defined(CFG_RAMAPP)
static
#endif
int nand_flash_read_buf(unsigned short blk, int offset, unsigned char *buffer, int len)
{
    int ret;
    PCFE_NAND_CHIP pchip = &g_chip;
    UINT32 start_addr;
    UINT32 blk_addr;
    UINT32 blk_offset;
    UINT32 size;
    UINT32 numBlocksInChip = pchip->chip_total_size/pchip->chip_block_size;    
#if defined(CFG_RAMAPP)
    bool flash_features;

    flash_features = (len & FLASH_FEATURES);
    len &= ~FLASH_FEATURES;
#endif
    ret = len;

    DBG_PRINTF(">> nand_flash_read_buf - 1 blk=0x%8.8lx, offset=%d, len=%lu\n",
        blk, offset, len);

    start_addr = (blk * pchip->chip_block_size) + offset;
    blk_addr = start_addr & ~(pchip->chip_block_size - 1);
    blk_offset = start_addr - blk_addr;
    size = pchip->chip_block_size - blk_offset;

    if(size > len)
        size = len;

    if (blk >= numBlocksInChip)
    {
        printf("Attempt to read block number(%d) beyond the nand max blk(%d) \n", blk, numBlocksInChip-1);
        return (FLASH_API_ERROR);
    }
        
#if defined(CFG_RAMAPP)
    if (flash_features)
        ret = nand_is_blk_cleanmarker(pchip, blk_addr);
#endif

    if (len)
        do
        {
            if (nandflash_read_page(pchip,start_addr,buffer,size) != FLASH_API_OK)
            {
               ret = FLASH_API_ERROR;
               break;
            }

            len -= size;
            if( len )
            {
                blk++;

                DBG_PRINTF(">> nand_flash_read_buf - 2 blk=0x%8.8lx, len=%lu\n",
                    blk, len);

                start_addr = blk * pchip->chip_block_size;
                buffer += size;
                if(len > pchip->chip_block_size)
                    size = pchip->chip_block_size;
                else
                    size = len;
            }
        } while(len);

    if (nand_is_blk_bad(pchip, blk))
    { /* don't check for bad block during page read/write since may be reading/writing to bad block marker,
         check for bad block after read to allow for data recovery */
        printf("nand_flash_read_buf(): Attempt to read bad nand block %d\n", blk);
        return(FLASH_API_ERROR);
    }

    DBG_PRINTF(">> nand_flash_read_buf - ret=%d\n", ret);

    return( ret ) ;
} /* nand_flash_read_buf */

#if defined(CFG_RAMAPP)
/***************************************************************************
 * Function Name: nand_flash_write_buf
 * Description  : Writes to flash memory.
 * Returns      : number of bytes written or FLASH_API_ERROR
 ***************************************************************************/
static int nand_flash_write_buf(unsigned short blk, int offset,
    unsigned char *buffer, int len)
{
    int ret = len;
    PCFE_NAND_CHIP pchip = &g_chip;
    unsigned int addr;
    unsigned int page_addr;
    unsigned int page_offset;
    unsigned int page_boundary;
    unsigned int size;
    unsigned int numBlocksInChip = pchip->chip_total_size/pchip->chip_block_size;

    DBG_PRINTF(">> nand_flash_write_buf - 1 blk=0x%8.8lx, offset=%d, len=%d\n",
        blk, offset, len);

    addr = (blk * pchip->chip_block_size) + offset;
    page_addr = addr & ~(pchip->chip_page_size - 1);
    page_offset = addr - page_addr;
    page_boundary = page_addr + pchip->chip_page_size;

    size = page_boundary - addr;

    if(size > len)
        size = len;

    if (blk >= numBlocksInChip)
    {
        printf("Attempt to write block number(%d) beyond the nand max blk(%d) \n", blk, numBlocksInChip-1);
        return(ret - len);
    }

    if (nand_is_blk_bad(pchip, blk))
    {
        printf("nand_flash_write_buf(): Attempt to write bad nand block %d\n", blk);
        return(ret - len);
    }

    if ( ((addr & (pchip->chip_block_size-1)) + len) > pchip->chip_block_size)
    { // cannot write past block boundary, otherwise if one block is good and another is bad then whole write is invalid
        printf("nand_flash_write_buf(): Attempt to write past block boundary, blk 0x%x, address 0x%x, length 0x%x\n", blk, addr, len);
        return (ret - len);
    }

    do
    {
        if(nandflash_write_page(pchip, page_addr, page_offset, buffer, size) != FLASH_API_OK)
        {
            ret = ret - len;
            break;
        }

        len -= size;
        if( len )
        {
            DBG_PRINTF(">> nand_flash_write_buf- 2 blk=0x%8.8lx, len=%d\n",
                blk, len);

            page_addr += pchip->chip_page_size;
            page_offset = 0;
            buffer += size;
            if(len > pchip->chip_page_size)
                size = pchip->chip_page_size;
            else
                size = len;
        }
    } while(len);

    DBG_PRINTF(">> nand_flash_write_buf - ret=%d\n", ret);

    return( ret ) ;
} /* nand_flash_write_buf */

/***************************************************************************
 * Function Name: nand_flash_get_memptr
 * Description  : Returns the base MIPS memory address for the specfied flash
 *                sector.
 * Returns      : Base MIPS memory address for the specfied flash sector.
 ***************************************************************************/
static unsigned char *nand_flash_get_memptr(unsigned short sector)
{
    /* Bad things will happen if this pointer is referenced.  But it can
     * be used for pointer arithmetic to deterine sizes.
     */
    return((unsigned char *) (FLASH_BASE + (sector * g_chip.chip_block_size)));
} /* nand_flash_get_memptr */

/***************************************************************************
 * Function Name: nand_flash_get_blk
 * Description  : Returns the flash sector for the specfied MIPS address.
 * Returns      : Flash sector for the specfied MIPS address.
 ***************************************************************************/
static int nand_flash_get_blk(int addr)
{
    return((int) ((unsigned long) addr - FLASH_BASE) / g_chip.chip_block_size);
} /* nand_flash_get_blk */

/***************************************************************************
 * Function Name: nand_flash_get_total_size
 * Description  : Returns the number of bytes in the "CFE Linux code"
 *                partition.
 * Returns      : Number of bytes
 ***************************************************************************/
static int nand_flash_get_total_size(void)
{
    return(g_chip.chip_total_size);
} /* nand_flash_get_total_size */

/***************************************************************************
 * Function Name: nand_flash_dev_specific_cmd
 * Description  : Triggers a device specific feature, used to access 
 *                non-standard features.
 * Returns      : Feature specific return code.
 ***************************************************************************/
static int nand_flash_dev_specific_cmd(unsigned int command, void * inBuf, void * outBuf)
{
    PCFE_NAND_CHIP pchip = &g_chip;

    switch(command)
    {
    case WRITE_PAGE:
    { // write to page and spare area
        flash_write_data_t *fwd = inBuf;
        int status;

        g_no_ecc = *(int*)outBuf;

        status = nandflash_write_page(pchip, (fwd->block * pchip->chip_block_size) + (fwd->page * pchip->chip_page_size), fwd->offset, fwd->data, fwd->amount);

        g_no_ecc = 0;
        return(status);
    }

    case READ_PAGE:
    {
        flash_write_data_t *fwd = inBuf;
        int status;

        g_no_ecc = *(int*)outBuf;

        if ((fwd->offset + fwd->amount) > pchip->chip_page_size)
        { // reading from spare area, offset into spare area seems always from beginning?
            int size = (fwd->offset < pchip->chip_page_size) ? fwd->amount - (pchip->chip_page_size - fwd->offset) : fwd->amount;

            status = nandflash_read_spare_area(pchip, (fwd->block * pchip->chip_block_size) + (fwd->page * pchip->chip_page_size), fwd->data + (fwd->amount - size), size);
        }

        if (fwd->offset < pchip->chip_page_size)
        { // reading from page area
            int size = (fwd->offset + fwd->amount) > pchip->chip_page_size ? pchip->chip_page_size - fwd->offset : fwd->amount;

            status = nandflash_read_page(pchip, (fwd->block * pchip->chip_block_size) + (fwd->page * pchip->chip_page_size), fwd->data, size); // != FLASH_API_OK)
        }

        g_no_ecc = 0;
        return(status);
    }

    case NAND_REINIT_FLASH:
        return(nand_initialize_spare_area(pchip, 0));

    case NAND_REINIT_FLASH_BAD:
        return(nand_initialize_spare_area(pchip, 1));

    case CHECK_BAD_BLOCK:
        return(nand_is_blk_bad(pchip, *(unsigned int *)inBuf));

    case MARK_BLOCK_BAD:
    {
        unsigned long page_addr = *(unsigned int *)inBuf * pchip->chip_block_size;

        nand_mark_bad_blk(pchip, page_addr);
        break;
    }

    case FORCE_ERASE:
        return(nandflash_block_erase(pchip, *(unsigned int *)inBuf * pchip->chip_block_size, 1));

    case GET_PAGE_SIZE:
        return(pchip->chip_page_size);

    case GET_SPARE_SIZE:
        return(pchip->chip_spare_size);

    }

    return 0;
}
#endif

/***************************************************************************
 * Function Name: nand_flash_get_sector_size
 * Description  : Returns the number of bytes in the specfied flash sector.
 * Returns      : Number of bytes in the specfied flash sector.
 ***************************************************************************/
#if defined(CFG_RAMAPP)
static
#endif
int nand_flash_get_sector_size(unsigned short sector)
{
    return(g_chip.chip_block_size);
} /* nand_flash_get_sector_size */

/***************************************************************************
 * Function Name: nand_flash_get_numsectors
 * Description  : Returns the number of blocks in the "CFE Linux code"
 *                partition.
 * Returns      : Number of blocks
 ***************************************************************************/
#if defined(CFG_RAMAPP)
static
#endif
int nand_flash_get_numsectors(void)
{
    return(g_chip.chip_total_size / g_chip.chip_block_size);
} /* nand_flash_get_numsectors */


/***************************************************************************
 * NAND Flash Implementation Functions
 ***************************************************************************/

/***************************************************************************
 * Function Name: nandflash_copy_from_cache
 * Description  : Copies data from the chip NAND cache to a local memory
 *                buffer.
 * Returns      : None.
 ***************************************************************************/
extern inline void nandflash_copy_from_cache(unsigned char *buffer,
    int offset, int numbytes)
{
    volatile unsigned char *cache = NAND_CACHE;
    unsigned int i, tmp;
    unsigned int* buf = (unsigned int*)buffer;

#if defined(_BCM947189_)
                {
                        uint32 ioctrl=NAND_FLASH_CTRL_WRAP->ioctrl;
                        ioctrl = ioctrl | NAND_APB_LITTLE_ENDIAN;
                        NAND_FLASH_CTRL_WRAP->ioctrl=ioctrl;
                }
#endif

    if( ((uintptr_t)buffer&0x3) )
    {
        for( i = 0; i < numbytes; i += sizeof(int) )
        {
            tmp = *(unsigned int*) &cache[i+offset];

#if defined(__ARMEL__) || defined(__MIPSEL) || defined(__AARCH64EL__)
            *buffer++ = (unsigned char) ((tmp >>  0) & 0xff);
            *buffer++ = (unsigned char) ((tmp >>  8) & 0xff);
            *buffer++ = (unsigned char) ((tmp >> 16) & 0xff);
            *buffer++ = (unsigned char) ((tmp >> 24) & 0xff);
#else
            *buffer++ = (unsigned char) ((tmp >> 24) & 0xff);
            *buffer++ = (unsigned char) ((tmp >> 16) & 0xff);
            *buffer++ = (unsigned char) ((tmp >>  8) & 0xff);
            *buffer++ = (unsigned char) ((tmp >>  0) & 0xff);
#endif
       }
    }
    else
    {
#if defined(__ARMEL__) && defined(CFG_ARM_ARCH_7A)
        if( (numbytes&0x1f) == 0 )  /* use ldm for 32 bytes burt read */
        {
            for( i = 0; i < numbytes; i += 32 )
            {
                __asm__ __volatile__ (
                    "LDMIA %0, {r0-r7} \n"
                    "STMIA %1, {r0-r7} \n"
                    : : "r"(&cache[i+offset]), "r"(buf) 
                    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
                );
                buf += 8;
            }
        }
        else
#endif
#if ((defined(__ARMEL__) && defined(CFG_ARMV8_AARCH64)) || defined(__AARCH64EL__))
        if( (numbytes&0xf) == 0 && (((uintptr_t)buffer)&0x7) == 0 )  /* use ldm for 16 bytes burt read if buffer is 8 bytes aligned*/
        {
            for( i = 0; i < numbytes; i += 16 )
            {
                __asm__ __volatile__ (
                    "LDP x0, x1, [%0] \n"
                    "STP x0, x1, [%1] \n"
                    : : "r"(&cache[i+offset]), "r"(buf) 
                    : "x0", "x1"
                );
                buf += 4;
            }
        }
        else
#endif
        {
            for( i = 0; i < numbytes; i += sizeof(int) )
            {
                *buf = *(unsigned int *) &cache[i+offset];
                buf++;
            }
        }
    }

#if defined(_BCM947189_)
                {
                        uint32 ioctrl=NAND_FLASH_CTRL_WRAP->ioctrl;
                        ioctrl = ioctrl & ~NAND_APB_LITTLE_ENDIAN;
                        NAND_FLASH_CTRL_WRAP->ioctrl=ioctrl;
                }
#endif

} /* nandflash_copy_from_cache */

/***************************************************************************
 * Function Name: nandflash_copy_from_spare
 * Description  : Copies data from the chip NAND spare registers to a local
 *                memory buffer.
 * Returns      : None.
 ***************************************************************************/
extern inline void nandflash_copy_from_spare(unsigned char *buffer,
    int numbytes)
{
    volatile unsigned int *spare_area = (volatile unsigned int *) &NAND->NandSpareAreaReadOfs0;
    unsigned char *pbuff = buffer;
    unsigned int i, j, k, l;

    for(i=0; i < 4; ++i)
    {
         j = spare_area[i];
         *pbuff++ = (unsigned char) ((j >> 24) & 0xff);
         *pbuff++ = (unsigned char) ((j >> 16) & 0xff);
         *pbuff++ = (unsigned char) ((j >>  8) & 0xff);
         *pbuff++ = (unsigned char) ((j >>  0) & 0xff);
    }

    /* Spare bytes greater than 16 are for the ECC. */
    if( NAND->NandRevision > 0x00000202 && numbytes > 16 )
    {
        spare_area = (unsigned int *) &NAND->NandSpareAreaReadOfs10;
        for(i=0, k=16; i < 4 && k < numbytes; ++i)
        {
            j = spare_area[i];
            l = 0;
            while( k < numbytes && l <= 24 )
            {
                *pbuff++ = (unsigned char) ((j >> (24 - l)) & 0xff);
                k++;
                l += 8;
            }
        }
    }


} /* nandflash_copy_from_spare */

#if defined(CFG_RAMAPP)
/***************************************************************************
 * Function Name: nandflash_copy_to_cache
 * Description  : Copies data from a local memory buffer to the the chip NAND
 *                cache.
 * Returns      : None.
 ***************************************************************************/
extern inline void nandflash_copy_to_cache(unsigned char *buffer, int offset,
    int numbytes)
{
    unsigned char *cache = (unsigned char *) NAND_CACHE;
    unsigned int i;
    unsigned int* buf = (unsigned int*)buffer;

#if defined(_BCM947189_)
                {
                        uint32 ioctrl=NAND_FLASH_CTRL_WRAP->ioctrl;
                        ioctrl = ioctrl | NAND_APB_LITTLE_ENDIAN;
                        NAND_FLASH_CTRL_WRAP->ioctrl=ioctrl;
                }
#endif

    if( ((uintptr_t)buffer&0x3) )
    {
        for( i = 0; i < numbytes; i += sizeof(int) )
        {
#if defined(__ARMEL__) || defined(__MIPSEL) || defined(__AARCH64EL__)
            *(unsigned int *) &cache[i] =
                ((unsigned int) buffer[i + 0] <<  0) |
                ((unsigned int) buffer[i + 1] <<  8) |
                ((unsigned int) buffer[i + 2] << 16) |
                ((unsigned int) buffer[i + 3] << 24);
#else
            *(unsigned int *) &cache[i] =
                ((unsigned int) buffer[i + 0] << 24) |
                ((unsigned int) buffer[i + 1] << 16) |
                ((unsigned int) buffer[i + 2] <<  8) |
                ((unsigned int) buffer[i + 3] <<  0);
#endif
        }
    }
    else
    {
        for( i = 0; i < numbytes; i += sizeof(int) )
        {
            *(unsigned int *) &cache[i] = *buf;
            buf++;
        }
    }

#if defined(_BCM947189_)
                {
                        uint32 ioctrl=NAND_FLASH_CTRL_WRAP->ioctrl;
                        ioctrl = ioctrl & ~NAND_APB_LITTLE_ENDIAN;
                        NAND_FLASH_CTRL_WRAP->ioctrl=ioctrl;
                }
#endif

} /* nandflash_copy_to_cache */

/***************************************************************************
 * Function Name: nandflash_copy_to_spare
 * Description  : Copies data from a local memory buffer to the the chip NAND
 *                spare registers.
 * Returns      : None.
 ***************************************************************************/
extern inline void nandflash_copy_to_spare(unsigned char *buffer,int numbytes)
{
    unsigned int *spare_area = (unsigned int *) &NAND->NandSpareAreaWriteOfs0;
    unsigned char *pbuff = buffer;
    int i, j;


    /* Spare bytes greater than 16 are for the ECC. */
    if( (numbytes > 16) && !g_no_ecc )
        numbytes = 16;
    /* controller use big endian for spare area bytes. On LE cpu, the following 
       code convert LE to BE. On BE MIPS, it actually does nothing because the
       pbuff are already in BE */
    for(i=0; i< numbytes / sizeof(unsigned int); ++i, pbuff += sizeof(int))
    {
        j = ((unsigned int) pbuff[0] << 24) |
            ((unsigned int) pbuff[1] << 16) |
            ((unsigned int) pbuff[2] <<  8) |
            ((unsigned int) pbuff[3] <<  0);
        spare_area[i] = j;
    }


} /* nandflash_copy_to_spare */
#endif

/***************************************************************************
 * Function Name: nandflash_wait_status
 * Description  : Polls the NAND status register waiting for a condition.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_wait_status(unsigned int status_mask)
{

    const unsigned int nand_poll_max = 2000000;

    unsigned int data;
    unsigned int poll_count = 0;
    int ret = FLASH_API_OK;

    do
    {
        data = NAND->NandIntfcStatus;
    } while(!(status_mask & data) && (++poll_count < nand_poll_max));

    if(!(status_mask & NAND->NandIntfcStatus))
    {
        printf("Status wait timeout: nandsts=0x%8.8lx mask=0x%8.8lx, count="
            "%lu\n", NAND->NandIntfcStatus, status_mask, poll_count);
        ret = FLASH_API_ERROR;
    }

    return( ret );
} /* nandflash_wait_status */

extern inline int nandflash_wait_cmd(void)
{
    return nandflash_wait_status(NIS_CTLR_READY);
} /* nandflash_wait_cmd */

extern inline int nandflash_wait_device(void)
{
    return nandflash_wait_status(NIS_FLASH_READY);
} /* nandflash_wait_device */

extern inline int nandflash_wait_cache(void)
{
    return nandflash_wait_status(NIS_CACHE_VALID);
} /* nandflash_wait_cache */

extern inline int nandflash_wait_spare(void)
{
    return nandflash_wait_status(NIS_SPARE_VALID);
} /* nandflash_wait_spare */

#if !defined(CFG_RAMAPP)
static int nandflash_check_ecc(void)
{
    return( FLASH_API_OK );
}
static int check_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step)
{
    return( 1 );
}
#else
/***************************************************************************
 * Function Name: turn_off_ecc
 * Description  : Turns off ECC
 * Returns      : access control register initial value
 ***************************************************************************/
static unsigned int turn_off_ecc(void)
{
    unsigned int acc = NAND->NandAccControl;
    unsigned int acc_save = acc;

#if !defined(_BCM963268_) && !defined(_BCM96838_)
    acc &= ~NAC_ECC_LVL_MASK;
    acc |= (NAC_ECC_LVL_DISABLE << NAC_ECC_LVL_SHIFT);
#else
    acc &= ~(NAC_ECC_LVL_0_MASK | NAC_ECC_LVL_MASK);
    acc |=  (NAC_ECC_LVL_DISABLE << NAC_ECC_LVL_0_SHIFT) |
            (NAC_ECC_LVL_DISABLE << NAC_ECC_LVL_SHIFT);
#endif
    NAND->NandAccControl = acc;

    return(acc_save);
}

/***************************************************************************
 * Function Name: nandflash_check_ecc
 * Description  : Reads ECC status.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_check_ecc(void)
{
    int ret = FLASH_API_OK;
    UINT32 intrCtrl;
    UINT32 accessCtrl;

    /* read interrupt status */
    intrCtrl = NAND_INTR->NandInterrupt;
    accessCtrl = NAND->NandAccControl;


    if( (intrCtrl & NINT_ECC_ERROR_UNC) != 0 )
        ret = FLASH_API_ERROR;

    if( (intrCtrl & NINT_ECC_ERROR_CORR) != 0 )
    {
        printf("Correctable ECC Error detected: addr=0x%8.8lx, intrCtrl=0x"
            "%08X, accessCtrl=0x%08X\n", NAND->NandEccCorrAddr, (UINT)intrCtrl,
            (UINT)accessCtrl);
    }

    return( ret );
}

/***************************************************************************
 * Function Name: check_ecc_for_ffs
 * Description  : Checks if the ECC bytes are all 0xFFs
 * Returns      : 1 if all ECC bytes are 0xff, 0 if not
 ***************************************************************************/
static int check_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step)
{
    int ret = 1;
    UINT8 *spare = KMALLOC(32, 0);
    UINT8 *spare_mask = pchip->chip_spare_mask;
    UINT32 i, j = step * pchip->chip_spare_step_size;

    memset(spare, 0xff, 32);
    nandflash_copy_from_spare(spare, pchip->chip_spare_step_size);
    for( i = 0; i < pchip->chip_spare_step_size; i++, j++ )
    {
        if( ECC_MASK_BIT(spare_mask, j) != 0 && spare[i] != 0xff )
        {
            ret = 0;
            break;
        }
    }

    KFREE(spare);
    return( ret );
}

/***************************************************************************
 * Function Name: nandflash_write_spare_area
 * Description  : Writes the spare area for the specified page.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_write_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len)
{
    int ret = FLASH_API_OK;
    unsigned char * nand_cache = KMALLOC(CTRLR_CACHE_SIZE, 0);
    unsigned char * spare = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned int is_mlc = 1;

    if( len <= pchip->chip_spare_size )
    {
        unsigned int acc_save;
                        
        UINT32 steps = pchip->chip_spare_size / pchip->chip_spare_step_size;
        UINT32 i;

        if( is_mlc )
            memset(nand_cache, 0xff, CTRLR_CACHE_SIZE);

        memset(spare, 0xff, pchip->chip_spare_size);
        memcpy(spare, buffer, len);
        for( i = 0; i < steps; i++ )
        {
            NAND->NandCmdAddr = pchip->chip_base + page_addr +
                (i * CTRLR_CACHE_SIZE);
            NAND->NandCmdExtAddr = 0;

            if( is_mlc )
            {
                acc_save = turn_off_ecc();

                /* MLC parts use PROGRAM_PAGE to program spare area. */
                nandflash_copy_to_cache(nand_cache, 0, CTRLR_CACHE_SIZE);
                nandflash_copy_to_spare(spare+(i*pchip->chip_spare_step_size),
                    pchip->chip_spare_step_size);
                NAND->NandCmdStart = NCMD_PROGRAM_PAGE;
            }
            else
            {
                /* SLC parts use PROGRAM_SPARE to program spare area. */
                nandflash_copy_to_spare(spare+(i*pchip->chip_spare_step_size),
                    pchip->chip_spare_step_size);
                NAND->NandCmdStart = NCMD_PROGRAM_SPARE;
            }

            if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
            {
                unsigned int sts = NAND->NandIntfcStatus;

                if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
                {
                    printf("Error writing to spare area, sts=0x%8.8lx\n", sts);
                    nand_mark_bad_blk(pchip, page_addr);
                    ret = FLASH_API_ERROR;
                }
            }

            if( is_mlc )
                NAND->NandAccControl = acc_save;
        }
    }
    else
        ret = FLASH_API_ERROR;

    KFREE(nand_cache);
    KFREE(spare);

    return( ret );
} /* nandflash_write_spare_area */
#endif

/***************************************************************************
 * Function Name: nandflash_read_spare_area
 * Description  : Reads the spare area for the specified page.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_read_spare_area(PCFE_NAND_CHIP pchip,
    unsigned long page_addr, unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

    if( len >= pchip->chip_spare_size )
    {
        UINT32 steps = pchip->chip_spare_size / pchip->chip_spare_step_size;
        UINT32 i;

#if defined(CFG_RAMAPP)
        unsigned int acc_save;

        if( g_no_ecc )
             acc_save = turn_off_ecc();
#endif
        for( i = 0; i < steps; i++ )
        {
            NAND->NandCmdAddr = pchip->chip_base + page_addr +
                (i * CTRLR_CACHE_SIZE);
            NAND->NandCmdExtAddr = 0;
            NAND->NandCmdStart = NCMD_SPARE_READ;

            if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
            {
                /* wait until data is available in the spare area registers */
                if( (ret = nandflash_wait_spare()) == FLASH_API_OK )
                    nandflash_copy_from_spare(buffer +
                        (i * pchip->chip_spare_step_size),
                        pchip->chip_spare_step_size);
                else
                    break;
            }
            else
                break;
        }
#if defined(CFG_RAMAPP)
        if( g_no_ecc )
            NAND->NandAccControl=acc_save;
#endif
    }

    return ret;
} /* nandflash_read_spare_area */

/***************************************************************************
 * Function Name: nandflash_read_page
 * Description  : Reads up to a NAND block of pages into the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_read_page(PCFE_NAND_CHIP pchip, unsigned long start_addr,
    unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

    if( len <= pchip->chip_block_size )
    {
        UINT32 page_addr = start_addr & ~(pchip->chip_page_size - 1);
        UINT32 index = 0;
        UINT32 subpage;
        INT32 length = len;
#if defined(CFG_RAMAPP)
        unsigned int acc_save_ecc;

        if( g_no_ecc )
             acc_save_ecc = turn_off_ecc();
#endif
        do
        {
            for( subpage = 0, ret = FLASH_API_OK; (subpage < pchip->chip_page_size) &&
                 (ret == FLASH_API_OK); subpage += CTRLR_CACHE_SIZE)
            {
                /* clear interrupts, so we can check later for ECC errors */
#if !defined(_BCM963268_) && !defined(_BCM96838_)
                /* write zero to clear in 63138 */
                NAND_INTR->NandInterrupt = 0;
#else
                NAND_INTR->NandInterrupt = NINT_STS_MASK;
#endif
                /* send command */
                NAND->NandCmdAddr = pchip->chip_base + page_addr + subpage;
                NAND->NandCmdExtAddr = 0;
                NAND->NandCmdStart = NCMD_PAGE_READ;

                if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
                {
                    /* wait until data is available in the cache */
                    if( (ret = nandflash_wait_cache()) == FLASH_API_OK )
                    {
                        if((ret = nandflash_check_ecc())==FLASH_API_ERROR)
                        {
                            if(check_ecc_for_ffs(pchip, subpage/CTRLR_CACHE_SIZE))
                                ret = FLASH_API_OK;
                            else
                            {
#if defined(CFG_RAMAPP)
                                printf("Uncorrectable ECC Error: intr 0x%x, addr="
                                        "0x%8.8lx, blk=%d,\n", NAND_INTR->NandInterrupt, NAND->NandEccUncAddr,
                                        NAND->NandEccUncAddr/pchip->chip_block_size);
#if 1
                                ret = FLASH_API_OK;
#else
                                nand_mark_bad_blk(pchip,
                                    start_addr & ~(pchip->chip_page_size-1));
#endif
#endif /* CFG_RAMAPP */
                            }
                        }
                    }

                    if( (ret == FLASH_API_OK) && (start_addr < (page_addr + subpage + CTRLR_CACHE_SIZE)) && ((start_addr + len) > page_addr + subpage) )
                    { // copy from cache only if buffer is within the subpage
                        UINT32 copy_size, offset;

                        if ( start_addr <= page_addr + subpage )
                        {
                            offset = 0;

                            if ( (start_addr + len) >= (page_addr + subpage + CTRLR_CACHE_SIZE) )
                                copy_size = CTRLR_CACHE_SIZE;
                            else
                                copy_size = (start_addr + len) - (page_addr + subpage);
                        }
                        else
                        { // start_addr > page_addr + subpage
                            offset = start_addr - (page_addr + subpage);

                            if ( (start_addr + len) >= (page_addr + subpage + CTRLR_CACHE_SIZE) )
                                copy_size = page_addr + subpage + CTRLR_CACHE_SIZE - start_addr;
                            else
                                copy_size = start_addr + len - start_addr;
                        }

                        nandflash_copy_from_cache(&buffer[index], offset, copy_size);

                        index += copy_size;
                        length -= copy_size;
                    }
                }
            }

            if(ret != FLASH_API_OK)
                break;

            page_addr += pchip->chip_page_size;

        } while(length);
#if defined(CFG_RAMAPP)
        if( g_no_ecc )
            NAND->NandAccControl=acc_save_ecc;
#endif
    }

    return( ret ) ;
} /* nandflash_read_page */

#if (INC_BTRM_BUILD==1) && !defined(_BCM96838_) && !defined(_BCM963381_)

#if defined(_BCM963268_)

/* CFE ROM is padded out to 48K (3x16K) because security is supported in bootrom */
#define NUM_16K_IN_CFE_ROM	3 

#endif

#define SMALPG_BLK_SIZE		16 * 1024
#define CFE_ROM_SIZE		NUM_16K_IN_CFE_ROM * SMALPG_BLK_SIZE
#define MAX_NUM_BTLDR_DUP	3

static unsigned int totalCfeRoms = 0;
static int cfeRom1Ecc = 0;
static int cfeRom2Ecc = 0;
static int cfeRom3Ecc = 0;
static int badBlk[MAX_NUM_BTLDR_DUP * NUM_16K_IN_CFE_ROM];

/***************************************************************************
 * Function Name: check_btrm_ecc_for_ffs
 * Description  : Checks if the ECC bytes are all 0xFFs
 * Returns      : 1 if all ECC bytes are 0xff, 0 if not
 ***************************************************************************/
static int check_btrm_ecc_for_ffs(PCFE_NAND_CHIP pchip, UINT32 step)
{
    int ret = 1;
    UINT8 *spare = KMALLOC(32, 0);
    UINT8 *spare_mask = pchip->chip_spare_mask;
    UINT32 i, j = step * pchip->chip_spare_step_size;

    nandflash_copy_from_spare(spare, pchip->chip_spare_step_size);
    for( i = 0; i < pchip->chip_spare_step_size; i++, j++ )
    {
        if( ECC_MASK_BIT(spare_mask, j) != 0 && spare[i] != 0xff )
        {
            ret = 0;
            break;
        }
    }

    KFREE(spare);

    return( ret );
}

/***************************************************************************
 * Function Name: nandflash_btrm_ecc_setleds
 * Description  : prints out ECC1, ECC2 or ECC3 if ECC page errors are 
 * 		  detected in CFE ROM 1, 2 or 3 respectively. The print occurs
 * 		  only once for each CFE ROM if more than one ECC error is
 * 		  detected within the CFE ROM image
 * Returns      : nothing
 ***************************************************************************/
static void nandflash_btrm_ecc_setleds(UINT32 cfeRomMirror)
{
    if (cfeRomMirror == 0)
    {
        if (cfeRom1Ecc == 0)
        {
            board_setleds(0x42504730);
            cfeRom1Ecc = 1;
        }
    }
    else
    {
        if (cfeRomMirror == CFE_ROM_SIZE)
        {
            if (cfeRom2Ecc == 0)
            {
                board_setleds(0x42504731);
                cfeRom2Ecc = 1;
            }
        }
        else
        {
            if (cfeRomMirror == 2 * CFE_ROM_SIZE)
            {
                if (cfeRom3Ecc == 0)
                {
                    board_setleds(0x42504732);
                    cfeRom3Ecc = 1;
                }
            }
        }
    }
}

/***************************************************************************
 * Function Name: nandflash_btrm_check_ecc
 * Description  : Reads ECC status.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_check_ecc(void)
{
    int ret = FLASH_API_OK;
    UINT32 intrCtrl;

    /* read interrupt status */
    intrCtrl = NAND_INTR->NandInterrupt;

    if( (intrCtrl & NINT_ECC_ERROR_UNC) != 0 )
        ret = FLASH_API_ERROR;

    return( ret );
}

/***************************************************************************
 * Function Name: nandflash_btrm_read
 * Description  : Sends commands to perform read from NAND flash
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_read(PCFE_NAND_CHIP pchip, UINT32 page_addr, 
    UINT32 page_offset)
{
    int ret = FLASH_API_ERROR;

    /* clear interrupts, so we can check later for ECC errors */
#if !defined(_BCM963268_) && !defined(_BCM96838_)
    /* write zero to clear in 63138 */
    NAND_INTR->NandInterrupt = 0;
#else
    NAND_INTR->NandInterrupt = NINT_STS_MASK;
#endif
    /* send command */
    NAND->NandCmdAddr = pchip->chip_base + page_addr + page_offset;
    NAND->NandCmdExtAddr = 0;
    NAND->NandCmdStart = NCMD_PAGE_READ;

    if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
    {
        /* wait until data is available in the cache */
        if( (ret = nandflash_wait_cache()) == FLASH_API_OK )
        {
            /* Set the ret value to FLASH_API_ERROR if the ECC fails */
            ret = nandflash_btrm_check_ecc();
        }
    }

    return ret;
}


/***************************************************************************
 * Function Name: nandflash_btrm_itis_what_itis
 * Description  : Reads CFE ROM bootloader into the specified buffer.
 * 		  An internal bootrom is retrieving the CFE ROM from flash.
 *		  The flash uses no strategy:
 *  Arguements  :
 *         len  - either 16KB (no security) or 48KB (with security)
 *                if 16KB, then it doesn't matter whether it is small or
 *                large page nand because 16KB fits entirely into block 0
 *                which is always considered good. Just return block 0. 
 *                if 48KB and small page nand, return first three blocks
 *                if 48KB and large page nand, return block 0
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_itis_what_itis(PCFE_NAND_CHIP pchip, unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

#if defined(_BCM963268_)

    int blkSize = pchip->chip_block_size;
    if (blkSize == SMALPG_BLK_SIZE)
    {
        // no bad block mgmt is used; grab blocks 0, 1 and 2

        // retrieve the first part of the bootloader from block 0
        ret = nandflash_btrm_get_content_from_block(pchip, 0, buffer, SMALPG_BLK_SIZE);

        if (ret == FLASH_API_OK)
        {
            // retrieve the second part of the bootloader from block 1
            ret = nandflash_btrm_get_content_from_block(pchip, 1, buffer + SMALPG_BLK_SIZE, SMALPG_BLK_SIZE);

            if (ret == FLASH_API_OK)
            {
                // retrieve the third part of the bootloader from block 2
                ret = nandflash_btrm_get_content_from_block(pchip, 2, buffer + (2 * SMALPG_BLK_SIZE), SMALPG_BLK_SIZE);
            }
        }
    }
    else
    {
        // 48KB len will fit entirely within block 0 which is assumed to be good.
        ret = nandflash_btrm_get_content_from_block(pchip, 0, buffer, len);
    }

#endif

    return( ret ) ;
}


/***************************************************************************
 * Function Name: nandflash_btrm_skip_bad_block
 * Description  : Reads CFE ROM bootloader into the specified buffer.
 * 		  An internal bootrom is retrieving the CFE ROM from flash.
 *		  The flash uses the skip bad block strategy:
 *  Arguements  :
 *         len  - either 16KB (no security) or 48KB (with security)
 *                if 16KB, then it doesn't matter whether it is small or
 *                large page nand because 16KB fits entirely into block 0
 *                which is always considered good. Just return block 0. 
 *                if 48KB, then only small page nand requires skip bad block
 *                because 48KB fits entirely into a large page block 0
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_skip_bad_block(PCFE_NAND_CHIP pchip, unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;

#if defined(_BCM963268_)

    int blkSize = pchip->chip_block_size;
    if (blkSize == SMALPG_BLK_SIZE)
    {
        // skip bad block is used; block 0 is good, but block 1 and 2 need to be checked

        // retrieve the first part of the bootloader from block 0
        ret = nandflash_btrm_get_content_from_block(pchip, 0, buffer, SMALPG_BLK_SIZE);

        if (ret == FLASH_API_OK)
        {
            // block 0 retrieved just fine (as expected)
            UINT32 i;
            UINT32 j = 0;
            UINT32 k = 0;

            // retrieve the next two good blocks
            for (i = 1; i < 3; i++)
            {
                ret = FLASH_API_ERROR;

                while ( ret == FLASH_API_ERROR)
                {
                    j += SMALPG_BLK_SIZE;
                    k++;

                    /* Check the first page of this block to see if block is marked bad. */
                    /* If the func below returns FLASH_API_ERROR, it could be the block  */
                    /* is marked bad, or the nand controller is stating that an          */
                    /* uncorrectable ECC has occurred. Either way, this block is no good */
                    /* and just skip this block. If the func returns, FLASH_API_OK, then */
                    /* at a minimum, the block is not marked bad, and therefore attempt  */
                    /* to read it                                                        */
                    ret = nandflash_btrm_read(pchip, j, 0);
                }

                /* Retrieve the part of the bootloader. It could return FLASH_API_ERROR         */
                /* due to read disturb wear and tear, but there is nothing we can do about that */
                ret = nandflash_btrm_get_content_from_block(pchip, k, buffer + (i * SMALPG_BLK_SIZE), SMALPG_BLK_SIZE);
            }

        }
    }
    else
    {
        // 48KB len will fit entirely within block 0 which is assumed to be good.
        ret = nandflash_btrm_get_content_from_block(pchip, 0, buffer, len);
    }

#endif

    return( ret ) ;
}


/***************************************************************************
 * Function Name: nandflash_btrm_get_content_from_block
 * Description  : Reads up to an entire block's worth of nand data into the 
 *                buffer. If any uncorrectable ECC occurs, the operation is 
 *                aborted, and FLASH_API_ERROR is returned; 
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_get_content_from_block(PCFE_NAND_CHIP pchip, UINT32 blkNum,
                                                 unsigned char *buffer, int len)
{
    int ret = FLASH_API_ERROR;
    int blkSize = pchip->chip_block_size;
    unsigned long start_addr = blkSize * blkNum;
    UINT32 page_addr = start_addr & ~(pchip->chip_page_size - 1);
    UINT32 page_offset = start_addr - page_addr;
    UINT32 size = pchip->chip_page_size - page_offset;
    UINT32 index = 0;
    UINT32 i;

    if(size > len)
        size = len;

    do
    {
        /* Nand controller retrieves 512 bytes at a time. A nand page can*/
        /* be 512 bytes or multiples of this. Loop as necessary in order */
        /* to retrieve the entire page via one nand cache at a time      */
        for( i = 0, ret = FLASH_API_OK; i < pchip->chip_page_size &&
             ret == FLASH_API_OK; i += CTRLR_CACHE_SIZE)
        {
            /* retrieve one nand cache worth of data (512 bytes)   */
            ret = nandflash_btrm_read(pchip, page_addr, i);

            if( ret == FLASH_API_OK )
            {
                /* nand cache data is good to use in creating page  */
                if( i < size )
                {
                    UINT32 copy_size = (i + CTRLR_CACHE_SIZE <= size) ? CTRLR_CACHE_SIZE : size - i;
                    nandflash_copy_from_cache(&buffer[index + i], page_offset, copy_size);
                }
            }
            else
            {
                /* An uncorrectable ECC occurred, we are done and the read has failed */
                /* Break out of the for loop */
                break;
            }
        }

        /* If an uncorrectable ECC occurred, we are done and the read has failed */
        /* Break out of the do loop */
        if(ret != FLASH_API_OK)
            break;

        /* Page read was successful. Get ready to retrieve the next page */
        page_offset = 0;
        page_addr += pchip->chip_page_size;
        index += size;
        len -= size;

        if(len > pchip->chip_page_size)
            size = pchip->chip_page_size;
        else
            size = len;

    } while(len);

    return( ret ) ;
}


/***************************************************************************
 * Function Name: nandflash_btrm_nand_mirroring
 * Description  : Reads CFE ROM bootloader into the specified buffer.
 * 		  An internal bootrom is retrieving the CFE ROM from flash.
 * 		  If only nand mirroring: 
 * 		  and small page NAND, the CFE ROM is duplicated across the 
 * 		  first one, two, or three NAND blocks. If large page NAND, 
 * 		  the CFE ROM is duplicated one, two or three times but all 
 * 		  within the first block. 
 * 		  If nand mirroring and security: 
 * 		  and small page NAND, the CFE ROM takes up three blocks, 
 * 		  so three mirrors will use nine NAND blocks. If large page NAND, 
 * 		  the CFE ROM is duplicated one, two or three times but the 
 * 		  restriction is it must all be within the first block. 
 * 		  If any page read has ECC errors, the mirror pages are used 
 * 		  from the other duplicates.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_btrm_nand_mirroring(PCFE_NAND_CHIP pchip, unsigned char *buffer, int len, unsigned long start_addr, UINT32 start_blk)
{
    int ret = FLASH_API_ERROR;
    int blkSize = pchip->chip_block_size;

    UINT32 index = 0;
    UINT32 i, k, cur_blk;
#if defined(_BCM963268_)
    UINT32 blk, blkBndry;
#endif

    UINT32 page_addr = start_addr & ~(pchip->chip_page_size - 1);
    UINT32 page_offset = start_addr - page_addr;
    UINT32 size = pchip->chip_page_size - page_offset;

    if(size > len)
        size = len;

    /* Retrieve the entire CFE ROM */
    do
    {
        /* Nand controller retrieves 512 bytes at a time. A nand page can*/
        /* be 512 bytes or multiples of this. Loop as necessary in order */
        /* to retrieve the entire page via one nand cache at a time      */
        for( i = 0, ret = FLASH_API_OK; i < pchip->chip_page_size &&
            ret == FLASH_API_OK; i += CTRLR_CACHE_SIZE)
        {
            cur_blk = start_blk;

            /*There could be up to three mirrors of the current nand cache*/
            /* content being retrieved. Loop until a good one is found,but*/
            /* exclude any within a known bad block                       */
            for( k = 0, ret = FLASH_API_ERROR;
                k < ((totalCfeRoms * CFE_ROM_SIZE) - start_addr) &&
                ret == FLASH_API_ERROR; k += CFE_ROM_SIZE)
            {
                if (! badBlk[cur_blk])
                {
                    /* retrieve one nand cache worth of data (512 bytes)   */
                    /* from a specific nand page                           */
                    ret = nandflash_btrm_read(pchip, page_addr + k, i);

                    if( ret == FLASH_API_OK )
                    {
                        /* nand cache data is good to use in creating page  */
                        if( i < size )
                        {
                            UINT32 copy_size = (i + CTRLR_CACHE_SIZE <= size)
                                                ? CTRLR_CACHE_SIZE : size - i;
                            nandflash_copy_from_cache(&buffer[index + i],
                                    page_offset, copy_size);
                            break;
                        }
                    }
                    else
                        nandflash_btrm_ecc_setleds(start_addr + k);
                }

                if (blkSize == SMALPG_BLK_SIZE)
                    cur_blk += NUM_16K_IN_CFE_ROM;
            }

            if (ret != FLASH_API_OK)
            {
                /* The content within the same page offset in all NAND  */
                /* mirrors are bad.This unit is toast.What are the odds?*/
                board_setleds(0x4641494c);
                break;
            }
        }

        /* Check to see if this page retrieval was good (FLASH_API_OK)*/
        /* or, if this page is bad in all mirrors (FLASH_API_ERROR)   */
        /* If bad page in all dup mirrors, this flash is toast        */
        if(ret != FLASH_API_OK)
            break;

        /* Page read was successful. Get ready to retrieve the next page */
        page_offset = 0;
        page_addr += pchip->chip_page_size;
        index += size;
        len -= size;

#if defined(_BCM963268_)
        /* Check if we are operating on a new block */
        if (blkSize == SMALPG_BLK_SIZE)
        {
            for (blkBndry = 1; blkBndry < NUM_16K_IN_CFE_ROM; blkBndry++)
            {
                if (index == (SMALPG_BLK_SIZE * blkBndry))
                {
                    /* The 1st or 2nd of three blocks has been read */
                    /* Get ready to read the next block             */
                    ret = FLASH_API_ERROR;
                    for (blk = blkBndry;
                         blk < (totalCfeRoms * NUM_16K_IN_CFE_ROM);
                         blk += NUM_16K_IN_CFE_ROM)
                    {
                        if (!badBlk[blk])
                        {
                            start_addr = blk * SMALPG_BLK_SIZE;
                            page_addr = start_addr;
                            start_blk = blk;
                            ret = FLASH_API_OK;
                            break;
                        }
                    }

                    /* Check to see if no good blks exist */
                    if (ret == FLASH_API_ERROR)
                        return( ret ) ;
                }
            }
        }
#endif

        if(len > pchip->chip_page_size)
            size = pchip->chip_page_size;
        else
            size = len;

    } while(len);

    return( ret ) ;
} /* nandflash_btrm_nand_mirroring */


/***************************************************************************
 * Function Name: nandflash_btrm_find_bad_blocks
 * Description  : loads up badBlk struct
 * Returns      : none
 ***************************************************************************/
static void nandflash_btrm_find_bad_blocks(PCFE_NAND_CHIP pchip, 
                   unsigned long *start_addr, UINT32 *start_blk)
{
    int ret;

    /* If small page nand, verify if any of the CFE ROM blocks are marked */
    /* bad due to failed block erasing or programming. If large page nand,*/
    /* there is only one block which is assumed to be good                */
    if (pchip->chip_block_size == SMALPG_BLK_SIZE)
    {
        UINT32 i, k = 0, serOut = 0x42424b30;
        for (i = 0; i < (totalCfeRoms * CFE_ROM_SIZE); i += SMALPG_BLK_SIZE)
        {
            /* Assume the current block is good */
            badBlk[k] = 0;

            ret = nandflash_btrm_read(pchip, i, 0);
            if( ret == FLASH_API_ERROR )
            {
                /* Could be a bad erase took place on this block and the block is */
                /* marked as bad, or it might be an uncorrectable ECC due to read */
                /* disturb. Find out which it is (if read disturb, the rest of    */
                /* this block can still be used)                                  */
                if (check_btrm_ecc_for_ffs(pchip,0))
                {
                    /* It is a bad block */
                    badBlk[k] = 1;
                    board_setleds(serOut + k);

                    if (k == 0)
                    {
                        /* Block 0 is bad, but it has a mirror block. Start at block 0's 2nd mirror */
                        *start_addr = CFE_ROM_SIZE;
                        *start_blk = NUM_16K_IN_CFE_ROM;
                    }

                    if ((*start_blk == k) && (totalCfeRoms > 2))
                    {
                        /* Block 0 and it's 2nd mirror are bad, but there is a 3rd mirror. Start there */
                        *start_addr = CFE_ROM_SIZE * 2;
                        *start_blk = NUM_16K_IN_CFE_ROM * 2;
                    }
                }
            }

            k++;
        }
    }
    else
        badBlk[0] = 0;
}


/***************************************************************************
 * Function Name: nand_flash_btrm_read_buf
 * Description  : Retrieves a CFE ROM bootloader. 
 * Returns      : FALSH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
int nand_flash_btrm_read_buf(unsigned char *buffer, unsigned int numDups)
{
    int ret;
    PCFE_NAND_CHIP pchip = &g_chip;
    unsigned long start_addr = 0;
    UINT32 start_blk = 0;

    /* numDups can be 0, 1, 2, or 3.				          */
    /* 0: skip bad block is to be used, and is only useful with security  */
    /*    on small page nand.             				  */
    /* 1: nand mirror is to be used, in which one copy of CFE ROM exists  */ 
    /* 2: nand mirror is to be used, in which two copies of CFE ROM exists*/
    /* 3: nand mirror is to be used, in which three copies exist          */
    totalCfeRoms = numDups;

    /* Now build up the 16k (or 48k) CFE ROM bootloader */
    switch (totalCfeRoms)
    {
       case 0:
          ret = nandflash_btrm_skip_bad_block(pchip, buffer, CFE_ROM_SIZE);
          break;
       case 1:
          ret = nandflash_btrm_itis_what_itis(pchip, buffer, CFE_ROM_SIZE);
          break;
       case 2: 
       case 3:
          nandflash_btrm_find_bad_blocks(pchip, &start_addr, &start_blk);
          ret = nandflash_btrm_nand_mirroring(pchip, buffer, CFE_ROM_SIZE, start_addr, start_blk);
          break;
       default:
          ret = FLASH_API_ERROR;
          break;
    }

    return( ret ) ;

} /* nand_flash_btrm_read_buf */

#endif /* #if (INC_BTRM_*) */


#if defined(CFG_RAMAPP)
/***************************************************************************
 * Function Name: nandflash_write_page
 * Description  : Writes a NAND page from the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_write_page(PCFE_NAND_CHIP pchip, unsigned long page_addr,
    unsigned int page_offset, unsigned char *buffer, int len)
{
    unsigned char * xfer_buf;
    int ret = FLASH_API_ERROR;
    unsigned int acc_save;

    { // check if write is blank
        unsigned int i;

        /* Don't write page if data is all 0xFFs.  This allows the
         * NAND flash driver to successfully write the Linux page.
         */
        ret = FLASH_API_OK_BLANK;
        for( i = 0; i < len; i++ )
        { // do this on a byte basis because buffer may not be aligned/whole page
            if( *(buffer + i) != 0xff )
            {
                ret = FLASH_API_OK;
                break;
            }
        }
    }

    if (ret == FLASH_API_OK_BLANK)
    { // don't write to page if data is all FF's
        return(FLASH_API_OK);
    }

    // all these calculations are made with the assumpation that there is page data to write (with a possible offset into the page) and if there's spare data to write it immediately follows the page data
    xfer_buf = KMALLOC(pchip->chip_page_size + pchip->chip_spare_size, 0);

    memset(xfer_buf, 0xff, pchip->chip_page_size + pchip->chip_spare_size);
    memcpy(xfer_buf + page_offset, buffer, len);
    page_offset = 0;

    if( g_no_ecc )
        acc_save = turn_off_ecc();

    for (len = 0; len < (pchip->chip_spare_size / pchip->chip_spare_step_size); len++)
    {
        NAND->NandCmdAddr = pchip->chip_base + page_addr + page_offset;
        NAND->NandCmdExtAddr = 0;

        nandflash_copy_to_cache(xfer_buf + page_offset, 0, CTRLR_CACHE_SIZE);

        /* write JFFS2 clean marker into spare area buffer if writing into data area of first page of block */
        if ( !g_no_ecc && ((page_addr & (pchip->chip_block_size - 1)) == 0) )
            nandflash_copy_to_spare(g_spare_cleanmarker + (len * pchip->chip_spare_step_size), pchip->chip_spare_step_size);
        else // write spare area data, which may be all 0xFF
            nandflash_copy_to_spare(&xfer_buf[pchip->chip_page_size + (len * pchip->chip_spare_step_size)], pchip->chip_spare_step_size + 3); // make sure it snaps to a value divisible by 4 which still includes the data, i.e. 27 bytes spare will get divided down /4 in nandflash_copy_to_spare

        NAND->NandCmdStart = NCMD_PROGRAM_PAGE;
        if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
        {
            unsigned int sts = NAND->NandIntfcStatus;

            if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
            {
                printf("Error writing to block, sts=0x%8.8lx\n", sts);
                nand_mark_bad_blk(pchip, page_addr & ~(pchip->chip_page_size - 1));
                ret = FLASH_API_ERROR;
            }
        }

        page_offset += CTRLR_CACHE_SIZE;
    }

    KFREE(xfer_buf);

    if( g_no_ecc )
        NAND->NandAccControl = acc_save;

    return( ret );
} /* nandflash_write_page */

/***************************************************************************
 * Function Name: nandflash_block_erase
 * Description  : Erases a block.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int nandflash_block_erase(PCFE_NAND_CHIP pchip, unsigned long blk_addr, int force )
{

    int ret = FLASH_API_OK;
    unsigned int blk = blk_addr/pchip->chip_block_size;

    if (nand_is_blk_bad(pchip, blk))
    {
        printf("nandflash_block_erase(): Attempt to erase bad nand block %d force %d\n", blk, force);
        if( force == 0 )
            return  FLASH_API_ERROR;
    }

    /* send command */
    NAND->NandCmdAddr = pchip->chip_base + blk_addr;
    NAND->NandCmdExtAddr = 0;
    NAND->NandCmdStart = NCMD_BLOCK_ERASE;
    if( (ret = nandflash_wait_cmd()) == FLASH_API_OK )
    {
        unsigned int sts = NAND->NandIntfcStatus;

        if( (sts & NIS_PGM_ERASE_ERROR) != 0 )
        {
            printf("\nError erasing block 0x%8.8lx, sts=0x%8.8lx\n", \
                blk_addr, sts);
            nand_mark_bad_blk(pchip, blk_addr);
            ret = FLASH_API_ERROR;
        }
    }

    DBG_PRINTF(">> nandflash_block_erase - addr=0x%8.8lx, ret=%d\n", blk_addr,
        ret);

    return( ret );
} /* nandflash_block_erase */

void dump_spare(void);
void dump_spare(void)
{
    PCFE_NAND_CHIP pchip = &g_chip;
    unsigned char * spare = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned int i;

    for( i = 0; i < pchip->chip_total_size; i += pchip->chip_block_size )
    {
        if( nandflash_read_spare_area(pchip, i, spare,
            pchip->chip_spare_size) == FLASH_API_OK )
        {
            printf("%8.8x: %8.8x %8.8x %8.8x %8.8x\n", i,
                *(unsigned int *) &spare[0], *(unsigned int *) &spare[4],
                *(unsigned int *) &spare[8], *(unsigned int *) &spare[12]);
            if( pchip->chip_spare_size > 16 )
            {
                printf("%8.8x: %8.8x %8.8x %8.8x %8.8x\n", i,
                    *(unsigned int *)&spare[16],*(unsigned int *)&spare[20],
                    *(unsigned int *)&spare[24],*(unsigned int *)&spare[28]);
                printf("%8.8x: %8.8x %8.8x %8.8x %8.8x\n", i,
                    *(unsigned int *)&spare[32],*(unsigned int *)&spare[36],
                    *(unsigned int *)&spare[40],*(unsigned int *)&spare[44]);
                printf("%8.8x: %8.8x %8.8x %8.8x %8.8x\n", i,
                    *(unsigned int *)&spare[48],*(unsigned int *)&spare[52],
                    *(unsigned int *)&spare[56],*(unsigned int *)&spare[60]);
            }
        }
        else
            printf("Error reading spare 0x%8.8x\n", i);
    }

    KFREE(spare);
}

int read_spare_data(int blk, int offset, unsigned char *buf, int bufsize, int read_all_oob);
int read_spare_data(int blk, int offset, unsigned char *buf, int bufsize, int read_all_oob)
{
    PCFE_NAND_CHIP pchip = &g_chip;
    unsigned char * spare = KMALLOC(SPARE_MAX_SIZE, 0);
    unsigned long page_addr = (blk * pchip->chip_block_size) + offset;
    unsigned int i, j;
    int ret;

    if( (ret = nandflash_read_spare_area( pchip, page_addr, spare,
        pchip->chip_spare_size)) == FLASH_API_OK )
    {
        if( read_all_oob )
        {
            memset(buf, 0x00, bufsize);
            memcpy(buf, spare, (pchip->chip_spare_size > bufsize)
                ? bufsize : pchip->chip_spare_size);
        }
        else
        {
            unsigned char *spare_mask = pchip->chip_spare_mask;

            /* Skip spare offsets that are reserved for the ECC.  Make
             * spare data bytes contiguous in the spare buffer.
             */
            for( i = 0, j = 0; i < pchip->chip_spare_size; i++ )
                if( ECC_MASK_BIT(spare_mask, i) == 0 && j < bufsize )
                    buf[j++] = spare[i];
        }
    }

    KFREE(spare);
    return(ret);
}

int dump_spare_pages(int blk);
int dump_spare_pages(int blk)
{
    PCFE_NAND_CHIP pchip = &g_chip;
    unsigned char * spare = KMALLOC(SPARE_MAX_SIZE / 4, 0);
    unsigned long page_addr = blk * pchip->chip_block_size;
    unsigned int i;
    int ret = 0;

    for( i = 0; i < 6; i++ )
    {
        if( (ret = nandflash_read_spare_area( pchip, page_addr +
            (i * pchip->chip_page_size), (unsigned char *) spare,
            pchip->chip_spare_size)) == FLASH_API_OK )
        {
            printf("%8.8x %8.8x %8.8x %8.8x\n", spare[0], spare[1],
                spare[2], spare[3]);
            if( pchip->chip_spare_size > 16 )
            {
                printf("%8.8x %8.8x %8.8x %8.8x\n", spare[4], spare[5],
                    spare[6], spare[7]);
                printf("%8.8x %8.8x %8.8x %8.8x\n", spare[8], spare[9],
                    spare[10], spare[11]);
                printf("%8.8x %8.8x %8.8x %8.8x\n\n", spare[12], spare[13],
                    spare[14], spare[15]);
            }
        }
    }

    KFREE(spare);
    return(ret);
}
#endif

