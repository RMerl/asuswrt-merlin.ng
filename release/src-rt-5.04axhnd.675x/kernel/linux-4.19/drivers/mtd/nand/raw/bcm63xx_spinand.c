#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 *
 *  drivers/mtd/bcmspinand/bcm63xx-spinand.c
 *
    <:copyright-BRCM:2011:DUAL/GPL:standard
    
       Copyright (c) 2011 Broadcom 
       All Rights Reserved
    
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


    File: bcm63xx-spinand.c

    Description: 
    This is a device driver for the Broadcom SPINAND flash for bcm63xxx boards.

 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <linux/slab.h> 
#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <flash_api.h>
#include "bcmSpiRes.h"

struct nand_ecclayout {
	__u32 eccbytes;
	__u32 eccpos[MTD_MAX_ECCPOS_ENTRIES_LARGE];
	__u32 oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES_LARGE];
};


#define COUNT_BAD_BITS 1 /* check higher granularity of bad bits in a page read */

#define SPI_NAND_CMD_LEN 4 /* length in bytes of a SPI NAND command */

/* Command codes for the flash_command routine */
#define FLASH_PROG          0x02    /* program load data to cache */
#define FLASH_READ          0x03    /* read data from cache */
#define FLASH_WRDI          0x04    /* reset write enable latch */
#define FLASH_WREN          0x06    /* set write enable latch */
#define FLASH_READ_FAST     0x0B    /* read data from cache */
#define FLASH_GFEAT         0x0F    /* get feature option */
#define FLASH_PEXEC         0x10    /* program cache data to memory array */
#define FLASH_PREAD         0x13    /* read from memory array to cache */
#define FLASH_SFEAT         0x1F    /* set feature option */
#define FLASH_SREAD         0x7C    /* get Macronix enhanced bad bit */
#define FLASH_PROG_RAN      0x84    /* program load data to cache at offset */
#define FLASH_DIE_SELECT    0xC2    /* die select command for Winbond and ESMT */
#define FLASH_BERASE        0xD8    /* erase one block in memory array */
#define FLASH_RDID          0x9F    /* read manufacturer and product id */
#define FLASH_RESET         0xFF    /* reset flash */

#define FEATURE_STAT_ENH    0x30
#define FEATURE_PROT_ADDR   0xA0
#define FEATURE_FEAT_ADDR   0xB0
#define FEATURE_STAT_ADDR   0xC0
#define FEATURE_DIE_SEL     0xD0
#define FEATURE_STAT_AUX    0xF0

/* Feature protectin bit defintion */
//#define PROT_BRWD           0x80
//#define PROT_BP_MASK        0x38
//#define PROT_BP_SHIFT       0x3
//#define PROT_BP_ALL         0x7
//#define PROT_BP_NONE        0x0
/* Gigadevice only */
//#define PROT_INV            0x04
//#define PROT_CMP            0x02

/* Feature feature bit defintion */
#define FEAT_OPT_EN         0x40
#define FEAT_ECC_EN         0x10
#define FEAT_DISABLE        0x0
/* Gigadevice only */
//#define FEAT_BBI            0x04
//#define FEAT_QE             0x01

/* Feature status bit definition */
#define STAT_ECC_MASK1      0x30  /* general, Gigadevice */
#define STAT_ECC_MASK2      0x0F  /* Macronix */
#define STAT_ECC_MASK3      0xF0  /* Toshiba */
#define STAT_ECC_GOOD       0x00
//#define STAT_ECC_CORR       0x10  /* correctable error */
#define STAT_ECC_UNCORR     0x20  /* uncorrectable error */
#define STAT_PFAIL          0x8   /* program fail */
#define STAT_EFAIL          0x4   /* erase fail */
#define STAT_WEL            0x2   /* write enable latch */
#define STAT_OIP            0x1   /* operation in progress */

/* Return codes from flash_status */
#define STATUS_READY        0       /* ready for action */
#define STATUS_BUSY         1       /* operation in progress */
#define STATUS_TIMEOUT      2       /* operation timed out */
#define STATUS_ERROR        3       /* unclassified but unhappy status */

/* Micron manufacturer ID */
#define MICRONPART          0x2C
#define ID_MT29F1G01AA      0x12
#define ID_MT29F2G01AA      0x22
#define ID_MT29F4G01AA      0x32
#define ID_MT29F1G01AB      0x14
#define ID_MT29F2G01AB      0x24 // also ESMT F50L2G41XA
#define ID_MT29F4G01AB      0x34 // also ESMT F50L4G41XB
#define ID_MT29F4G01AD      0x36

/* Gigadevice manufacturer ID */
#define GIGADEVPART         0xC8
#define ID_GD5F1GQ4UA       0xF1
#define ID_GD5F2GQ4UA       0xF2
#define ID_GD5F4GQ4UA       0xF4
#define ID_GD5F1GQ4UB       0xD1 // also 4UE
#define ID_GD5F2GQ4UB       0xD2 // also 4UE
#define ID_GD5F4GQ4UB       0xD4 // also 4UE
#define ID_GD5F1GQ5UE       0x51
#define ID_GD5F4GQ6UE       0x55

/* ESMT manufacturer ID */
#define ESMTPART            0xC8
#define ID_F50L1G41A        0x21 // also ISSI IS37SML01G1 (same Pegatron die)
#define ID_F50L1G41LB       0x01
#define ID_F50L2G41LB       0x0A
#define ID_F50L2G41KA       0x41

/* Winbond manufacturer ID, please note we do not support continuous read mode xIG parts, only xIR parts */
#define WINBONDPART         0xEF
#define ID_W25N512GV_1      0xAA
#define ID_W25N512GV_2      0x20
#define ID_W25N01GV_1       0xAA
#define ID_W25N01GV_2       0x21
#define ID_W25N02GV_1       0xAA
#define ID_W25N02GV_2       0x22
#define ID_W25M02GV_1       0xAB
#define ID_W25M02GV_2       0x21
#define ID_W25N02KV_1       0xAA
#define ID_W25N02KV_2       0x22
#define ID_W25N04KV_1       0xAA
#define ID_W25N04KV_2       0x23

/* MXIC Macronix manufacturer ID */
#define MACRONIXPART        0xC2
#define ID_MX35LF1GE4AB     0x12
#define ID_MX35LF2GE4AB     0x22
#define ID_MX35LF2GE4AD     0x26
#define ID_MX35LF4GE4AD     0x37
//#define ID_MX35LF2G14       0x20 // do not support, requires external 4-bit ECC
#define ID_MX35LF2GE4AD_1   0x26
#define ID_MX35LF2GE4AD_2   0x03

/* Toshiba manufacturer ID */
#define TOSHIBAPART         0x98
#define ID_TC58CVG0S        0xC2
#define ID_TC58CVG1S        0xCB
#define ID_TC58CVG1S0HRAIJ  0xEB
#define ID_TC58CVG2S        0xCD
#define ID_TC58CVG2S0HRAIJ  0xED
#define ID_TC58CVG0S3_1     0xE2
#define ID_TC58CVG0S3_2     0x40

/* Etrontech manufacturer ID */
#define ETRONPART           0xD5
#define ID_EM73C044SNB      0x11
#define ID_EM73C044VCD      0x1C
#define ID_EM73C044VCF      0x25
#define ID_EM73D044SNF      0x10
#define ID_EM73D044VCG      0x1F
#define ID_EM73D044VCL      0x2E
#define ID_EM73E044SNA      0x03

/* FM manufacturer ID */
#define FMPART              0xA1
//#define ID_FM25G01B         0xD1 // do not support, has ECC enable in wrong location
#define ID_FM25S01          0xA1
#define ID_FM25S01A         0xE4

/* XTX manufacturer ID
   XTX parts have a few incompatibilities with other SPI NAND parts,
   1. Their ECC status on some devices is extended using the same ECC status bits but redefined which makes it incompatible with standard ECC status values.
   2. Their ECC is multiplexed into the spare area, so when writing to the spare area ECC data with ECC off those are different bytes, when turning ECC back on they will have different data, so can't write custom ECC values
   3. When writing to the part with ECC off you must use FLASH_PROG instead of FLASH_PROG_RAN */
#define XTXPART             0x0B
#define ID_XT26G01A         0xE1
#define ID_XT26G02A         0xE2
//#define ID_XT26G02B         0xF2 // do not support, ECC status incompatible

/* Paragon manufacturer ID */
//#define PARAGONPART         0xA1
//add for DS flash
#define DOSILICON           0xE5
#define ID_DS35Q2GB         0xF2


#define SPI_BUILD_ID(A,B)   \
    ( (A & 0xFFFF0000) | ((B & 0xFF00) ? (A & 0xFF00) : 0) | ((B & 0xFF) ? (A & 0xFF) : 0) )

#define SPI_MAKE_ID(A,B)    \
    ((unsigned int)(A << 24) | (unsigned int)(B << 16))

#define SPI_MAKE_ID_3_BYTE(A,B,C)    \
    ((unsigned int)(A << 24) | (unsigned int)(B << 16) | (unsigned int)(C << 8))

#define SPI_MAKE_ID_4_BYTE(A,B,C,D)    \
    ((unsigned int)(A << 24) | (unsigned int)(B << 16) | (unsigned int)(C << 8) | (unsigned int)D)

#define SPI_MANU_ID(devId)  \
    ((unsigned char)((devId>>24)&0xff))

#define SPI_NAND_DEVICES                                                    \
    {{SPI_MAKE_ID(GIGADEVPART,  ID_GD5F1GQ4UA),  "GigaDevice GD5F1GQ4UA"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F2GQ4UA),  "GigaDevice GD5F2GQ4UA"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F4GQ4UA),  "GigaDevice GD5F4GQ4UA"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F1GQ4UB),  "GigaDevice GD5F1GQ4UB"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F2GQ4UB),  "GigaDevice GD5F2GQ4UB"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F4GQ4UB),  "GigaDevice GD5F4GQ4UB"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F1GQ5UE),  "GigaDevice GD5F1GQ5UE"},  \
     {SPI_MAKE_ID(GIGADEVPART,  ID_GD5F4GQ6UE),  "GigaDevice GD5F4GQ6UE"},  \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F1G01AA), "Micron MT29F1G01AA"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F2G01AA), "Micron MT29F2G01AA"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F4G01AA), "Micron MT29F4G01AA"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F1G01AB), "Micron MT29F1G01AB"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F2G01AB), "Micron MT29F2G01AB"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F4G01AB), "Micron MT29F4G01AB"},     \
     {SPI_MAKE_ID(MICRONPART,   ID_MT29F4G01AD), "Micron MT29F4G01AD"},     \
     {SPI_MAKE_ID(ESMTPART,     ID_F50L1G41A),   "ESMT F50L1G41A"},         \
     {SPI_MAKE_ID(ESMTPART,     ID_F50L1G41LB),  "ESMT F50L1G41LB"},        \
     {SPI_MAKE_ID(ESMTPART,     ID_F50L2G41LB),  "ESMT F50L2G41LB"},        \
     {SPI_MAKE_ID(ESMTPART,     ID_F50L2G41KA),  "ESMT F50L2G41KA"},        \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25N512GV_1, ID_W25N512GV_2), "Winbond W25N512GV"},     \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25N01GV_1,  ID_W25N01GV_2),  "Winbond W25N01GV"},      \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25N02GV_1,  ID_W25N02GV_2),  "Winbond W25N02GV"},      \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25M02GV_1,  ID_W25M02GV_2),  "Winbond W25M02GV"},      \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25N02KV_1,  ID_W25N02KV_2),  "Winbond W25N02KV"},      \
     {SPI_MAKE_ID_3_BYTE(WINBONDPART,  ID_W25N04KV_1,  ID_W25N04KV_2),  "Winbond W25N04KV"},      \
     {SPI_MAKE_ID(MACRONIXPART, ID_MX35LF1GE4AB),  "Macronix MX35LF1GE4AB"},    \
     {SPI_MAKE_ID(MACRONIXPART, ID_MX35LF2GE4AB),  "Macronix MX35LF2GE4AB"},    \
     {SPI_MAKE_ID(MACRONIXPART, ID_MX35LF2GE4AD),  "Macronix MX35LF2GE4AD"},    \
     {SPI_MAKE_ID_3_BYTE(MACRONIXPART,  ID_MX35LF2GE4AD_1,  ID_MX35LF2GE4AD_2),  "Macronix MX35LF2GE4AD"},      \
     {SPI_MAKE_ID(MACRONIXPART, ID_MX35LF4GE4AD),  "Macronix MX35LF4GE4AD"},    \
     {SPI_MAKE_ID(TOSHIBAPART,  ID_TC58CVG0S),   "Toshiba TC58CVG0S"},      \
     {SPI_MAKE_ID(TOSHIBAPART,  ID_TC58CVG1S),   "Toshiba TC58CVG1S"},      \
     {SPI_MAKE_ID(TOSHIBAPART,  ID_TC58CVG1S0HRAIJ),   "Toshiba TC58CVG1S0HRAIJ"},      \
     {SPI_MAKE_ID(TOSHIBAPART,  ID_TC58CVG2S),   "Toshiba TC58CVG2S"},      \
     {SPI_MAKE_ID(TOSHIBAPART,  ID_TC58CVG2S0HRAIJ),   "Toshiba TC58CVG2S0HRAIJ"},      \
     {SPI_MAKE_ID_3_BYTE(TOSHIBAPART, ID_TC58CVG0S3_1,  ID_TC58CVG0S3_2),  "Toshiba TC58CVG0S3"}, \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73C044SNB), "Etron EM73C044SNB"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73C044VCD), "Etron EM73C044VCD"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73C044VCF), "Etron EM73C044VCF"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73D044SNF), "Etron EM73D044SNF"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73D044VCG), "Etron EM73D044VCG"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73D044VCL), "Etron EM73D044VCL"},      \
     {SPI_MAKE_ID(ETRONPART,    ID_EM73E044SNA), "Etron EM73E044SNA"},      \
     {SPI_MAKE_ID(XTXPART,      ID_XT26G01A),    "XTX XT26G01A"},           \
     {SPI_MAKE_ID(XTXPART,      ID_XT26G02A),    "XTX XT26G02A"},           \
     {SPI_MAKE_ID(FMPART,       ID_FM25S01),     "FM 25S01"},               \
     {SPI_MAKE_ID(DOSILICON,    ID_DS35Q2GB),    "DS DS35Q2GB"},            \
     {0,""}                                                                 \
    }


#define ECC_LAYOUT 1

#if defined(ECC_LAYOUT)
static struct nand_ecclayout spinand_oob_gigadevice_2k_A =
{ // Gigadevice A parts
    .eccbytes = 69,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,                                                       12,  13,  14,  15, // these must be in numerical order
                                                                 28,  29,  30,  31,
                                                                 44,  45,  46,  47,
                                                                 60,  61,  62,  63,
                    67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
                    83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
                    99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
                   115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
#ifndef _CFE_
    .oobavail = 59,
    .oobfree = {
        {.offset = 1,
         .length = 11},
        {.offset = 16,
         .length = 12},
        {.offset = 24,
         .length = 12},
        {.offset = 48,
         .length = 12},
        {.offset = 64,
         .length = 3},
        {.offset = 80,
         .length = 3},
        {.offset = 96,
         .length = 3},
        {.offset = 112,
         .length = 3}
    }
#endif
};

static struct nand_ecclayout spinand_oob_gigadevice_2k_B =
{ // Gigadevice B parts
    .eccbytes = 53,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, // these must be in numerical order



                    67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
                    83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
                    99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
                   115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
    .oobavail = 75,
    .oobfree = {
        {.offset = 1,
         .length = 63},
        {.offset = 64,
         .length = 3},
        {.offset = 80,
         .length = 3},
        {.offset = 96,
         .length = 3},
        {.offset = 112,
         .length = 3}
    }
};

static struct nand_ecclayout spinand_oob_gigadevice_4k =
{
    .eccbytes = 105,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, // these must be in numerical order







                   131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
                   147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
                   163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
                   179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
                   195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
                   211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
                   227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
                   243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
    },
    .oobavail = 151,
    .oobfree = {
        {.offset = 1,
         .length = 130},
        {.offset = 144,
         .length = 3},
        {.offset = 160,
         .length = 3},
        {.offset = 176,
         .length = 3},
        {.offset = 192,
         .length = 3},
        {.offset = 208,
         .length = 3},
        {.offset = 224,
         .length = 3},
        {.offset = 240,
         .length = 3}
    }
};

static struct nand_ecclayout spinand_oob_micron_aa =
{
    .eccbytes = 33,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,                                    8,   9,  10,  11,  12,  13,  14,  15, // these must be in numerical order
                                             24,  25,  26,  27,  28,  29,  30,  31,
                                             40,  41,  42,  43,  44,  45,  46,  47,
                                             56,  57,  58,  59,  60,  61,  62,  63
    },
    .oobavail = 31,
    .oobfree = {
        {.offset = 1,
         .length = 7},
        {.offset = 16,
         .length = 8},
        {.offset = 24,
         .length = 8},
        {.offset = 48,
         .length = 8}
    }
};

static struct nand_ecclayout spinand_oob_toshiba_micron_ab =
{
    .eccbytes = 65,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, // these must be in numerical order



       64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
       80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
       96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
    .oobavail = 63,
    .oobfree = {
        {.offset = 1,
         .length = 63}
    }
};

static struct nand_ecclayout spinand_oob_esmt =
{
    .eccbytes = 29,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,   1,   2,   3,   4,   5,   6,   7, // these must be in numerical order
            17,  18,  19,  20,  21,  22,  23,
            33,  34,  35,  36,  37,  38,  39,
            49,  50,  51,  52,  53,  54,  55
    },
    .oobavail = 35,
    .oobfree = {
        {.offset = 8,
         .length = 9},
        {.offset = 24,
         .length = 9},
        {.offset = 40,
         .length = 9},
        {.offset = 56,
         .length = 8}
    }
};

static struct nand_ecclayout spinand_oob_esmt2 =
{
    .eccbytes = 40,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,   1,                               8,   9,  10,  11,  12,  13,  14,  15, // these must be in numerical order
       16,  17,                              24,  25,  26,  27,  28,  29,  30,  31,
       32,  33,                              40,  41,  42,  43,  44,  45,  46,  47,
       48,  49,                              56,  57,  58,  59,  60,  61,  62,  63
    },
    .oobavail = 24,
    .oobfree = {
        {.offset = 2,
         .length = 6},
        {.offset = 18,
         .length = 6},
        {.offset = 34,
         .length = 6},
        {.offset = 48,
         .length = 6}
    }
};

static struct nand_ecclayout spinand_oob_mxic =
{
    .eccbytes = 1,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0 // these must be in numerical order
    },
    .oobavail = 63,
    .oobfree = {
        {.offset = 1,
         .length = 63}
    }
};

static struct nand_ecclayout spinand_oob_mxic_ad =
{
    .eccbytes = 72,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, 1, 8, 9, 16, 17, 24, 25, // these must be in numerical order



       64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
       80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
       96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
#ifndef _CFE_
    .oobavail = 56,
    .oobfree = {
        {.offset = 2,
         .length = 6},
        {.offset = 10,
         .length = 6},
        {.offset = 18,
         .length = 6},
        {.offset = 26,
         .length = 6}

    }
#endif
};

static struct nand_ecclayout spinand_oob_etron =
{
    .eccbytes = 62,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,                                                                            // these must be in numerical order
       16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
                                                                             46,  47,
       48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,
                                                                   76,  77,  78,  79,
       80,  81,  82,  83,  84,  85,  86,  87,  88,
                                                        106, 107, 108, 109, 110, 111,
      112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
    .oobavail = 66,
    .oobfree = {
        {.offset = 1,
         .length = 15},
        {.offset = 29,
         .length = 17},
        {.offset = 59,
         .length = 17},
        {.offset = 89,
         .length = 17}
    }
};

static struct nand_ecclayout spinand_oob_etron2 =
{
    .eccbytes = 57,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,                                                                          // these must be in numerical order
                 18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,

                 50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,

                 82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,

                114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127
    },
    .oobavail = 71,
    .oobfree = {
        {.offset = 1,
         .length = 17},
        {.offset = 32,
         .length = 18},
        {.offset = 64,
         .length = 18},
        {.offset = 96,
         .length = 18}
    }
};

static struct nand_ecclayout spinand_oob_xtx =
{
    .eccbytes = 17,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, // these must be in numerical order


       48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63
    },
    .oobavail = 47,
    .oobfree = {
        {.offset = 1,
         .length = 47}
    }
};

static struct nand_ecclayout spinand_oob_toshiba =
{
    .eccbytes = 129,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0, // these must be in numerical order







      128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
      144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
      160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 274, 175,
      176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
      192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
      208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
      224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
      240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
    },
    .oobavail = 127,
    .oobfree = {
        {.offset = 1,
         .length = 127}
    }
};

/*
static struct nand_ecclayout spinand_oob_paragon =
{
    .eccbytes = 53,
    .eccpos = { // for ease of use, call the bad block marker an ECC byte as well
        0,                            6,   7,   8,   9,  10,  11,  12,  13,  14,  15, // these must be in numerical order
       16,  17,  18,            21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
       32,  33,            36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47, 
       48,            51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63




    },
    .oobavail = 75,
    .oobfree = {
        {.offset = 1,
         .length = 5},
        {.offset = 19,
         .length = 2},
        {.offset = 34,
         .length = 2},
        {.offset = 49,
         .length = 2},
        {.offset = 64,
         .length = 64}
    }
};
*/
#endif /* ECC_LAYOUT */


/* the controller will handle operations that are greater than the FIFO size
   code that relies on READ_BUF_LEN_MAX, READ_BUF_LEN_MIN or spi_max_op_len
   could be changed */
#define READ_BUF_LEN_MAX   544    /* largest of the maximum transaction sizes for SPI */
#define READ_BUF_LEN_MIN   60     /* smallest of the maximum transaction sizes for SPI */
#define SPI_BUF_LEN        512    /* largest of the maximum transaction sizes for SPI */
/* this is the slave ID of the SPI flash for use with the SPI controller */
#define SPI_FLASH_SLAVE_DEV_ID    0
/* clock defines for the flash */
#define SPI_FLASH_DEF_CLOCK       781000
#define SPARE_MAX_SIZE          (27 * 16)
#define CTRLR_CACHE_SIZE        512
#define ECC_MASK_BIT(ECCMSK, OFS)   (ECCMSK[OFS / 8] & (1 << (OFS % 8)))

/* default to legacy controller - updated later */
static int spi_flash_clock  = SPI_FLASH_DEF_CLOCK;
static int spi_flash_busnum = LEG_SPI_BUS_NUM;

//#undef DEBUG_NAND
//#define DEBUG_NAND
#if defined(DEBUG_NAND)
#define DBG_PRINTF printk
#else
#define DBG_PRINTF(...)
#endif


#define STATUS_DEFAULT NAND_STATUS_TRUE_READY|NAND_STATUS_READY|NAND_STATUS_WP

// device information bytes required to identify device for SPI NAND
#define SPI_NAND_ID_LENGTH  4

// device information bytes required to identify device for Linux NAND
#define NAND_ID_LENGTH  4

#define SPI_CONTROLLER_STATE_SET             (1 << 31)
#define SPI_CONTROLLER_STATE_CPHA_EXT        (1 << 30)
#define SPI_CONTROLLER_STATE_GATE_CLK_SSOFF  (1 << 29)
#define SPI_CONTROLLER_STATE_ASYNC_CLOCK     (1 << 28)

#define SPI_CONTROLLER_MAX_SYNC_CLOCK 30000000

/* set mode and controller state based on CHIP defaults
   these values do not apply to the legacy controller
   legacy controller uses SPI_MODE_3 and clock is not
   gated */

#define SPI_MODE_DEFAULT              SPI_MODE_0
#define SPI_CONTROLLER_STATE_DEFAULT  (SPI_CONTROLLER_STATE_GATE_CLK_SSOFF)


static char mtd_id[32];
static unsigned int spi_max_op_len = SPI_BUF_LEN;

spinlock_t chip_lock;


struct SpiNandChip
{
    unsigned char *chip_name;
    unsigned char chip_device_id[SPI_NAND_ID_LENGTH];
    unsigned long chip_total_size;
    unsigned int chip_num_blocks;
    unsigned int chip_block_size;
    unsigned int chip_page_size;
    unsigned int chip_spare_size;
    unsigned int chip_ecc_offset;
    struct nand_ecclayout *ecclayout;
    unsigned short chip_block_shift;
    unsigned short chip_page_shift;
    unsigned short chip_num_planes;
    unsigned long chip_die_sel;
    unsigned char chip_ecc; // ECC bits
    unsigned char chip_ecc_corr; // threshold to fix correctable bits
    unsigned char chip_ecc_enh; // enhanced bad bit detection by chip
    unsigned char chip_subpage_shift; // 2^ shift amount based on number of subpages, typically 4
    unsigned long chip_clock_speed;
};

static struct SpiNandChip * pchip;


static struct SpiNandChip SpiDevInfo[] =
{
    { // 1Gb
        .chip_name = "GigaDevice GD5F1GQ4UA",
        .chip_device_id = {GIGADEVPART, ID_GD5F1GQ4UA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_2k_A,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 120000000,
    },
    { // 2Gb
        .chip_name = "GigaDevice GD5F2GQ4UA",
        .chip_device_id = {GIGADEVPART, ID_GD5F2GQ4UA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_2k_A,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 120000000,
    },
    { // 1Gb
        .chip_name = "GigaDevice GD5F1GQ4UB", // also GD5F1GQ4UE with integrated controller instead of seperate controller die
        .chip_device_id = {GIGADEVPART, ID_GD5F1GQ4UB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_2k_B,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip (6/8)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 120000000,
    },
    { // 2Gb
        .chip_name = "GigaDevice GD5F2GQ4UB",
        .chip_device_id = {GIGADEVPART, ID_GD5F2GQ4UB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_2k_B,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip (6/8)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 120000000,
    },
    { // 4Gb
        .chip_name = "GigaDevice GD5F4GQ4UB",
        .chip_device_id = {GIGADEVPART, ID_GD5F4GQ4UB, 0, 0},
        .chip_page_size = 4096,
        .chip_page_shift = 12,
        .chip_block_size = 64 * 4096, // 64 pages per block x chip_page_size
        .chip_block_shift = 18,       // block size of 0x40000 (256KB)
        .chip_spare_size = 256,
        .chip_ecc_offset = 0x1080,    // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 4096 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_4k,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip (6/8)
        .chip_subpage_shift = 3, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 120000000,
    },
    { // 1Gb
        .chip_name = "Micron MT29F1G01AA",
        .chip_device_id = {MICRONPART, ID_MT29F1G01AA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 50000000,
    },
    { // 2Gb
        .chip_name = "Micron MT29F2G01AA",
        .chip_device_id = {MICRONPART, ID_MT29F2G01AA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 2,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 50000000,
    },
    { // 4Gb
        .chip_name = "Micron MT29F4G01AA",
        .chip_device_id = {MICRONPART, ID_MT29F4G01AA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 4096,      // 4096 blocks total
        .chip_num_planes = 2,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 4096, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 50000000,
    },
    { // 1Gb
        .chip_name = "Micron MT29F1G01AB",
        .chip_device_id = {MICRONPART, ID_MT29F1G01AB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 130000000,
    },
    { // 2Gb
        .chip_name = "Micron MT29F2G01AB",
        .chip_device_id = {MICRONPART, ID_MT29F2G01AB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 2,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 130000000,
    },
    { // 4Gb
        .chip_name = "Micron MT29F4G01AB",
        .chip_device_id = {MICRONPART, ID_MT29F4G01AB, 0, 0},
        .chip_page_size = 4096,
        .chip_page_shift = 12,
        .chip_block_size = 64 * 4096, // 64 pages per block x chip_page_size
        .chip_block_shift = 18,       // block size of 0x40000 (256KB)
        .chip_spare_size = 256,
        .chip_ecc_offset = 0x1000,    // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0x10000000,   // 2Gb cutoff
        .chip_total_size = 64 * 4096 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 130000000,
    },
    { // 4Gb
        .chip_name = "Micron MT29F4G01AD",
        .chip_device_id = {MICRONPART, ID_MT29F4G01AD, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 4096,      // 4096 blocks total
        .chip_num_planes = 2,
        .chip_die_sel = 0x10000000,   // 2Gb cutoff
        .chip_total_size = 64 * 2048 * 4096, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 130000000,
    },
    { // 1Gb
        .chip_name = "ESMT F50L1G41A",
        .chip_device_id = {ESMTPART, ID_F50L1G41A, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_esmt,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25N02KV",
        .chip_device_id = {WINBONDPART, ID_W25N02KV_1, ID_W25N02KV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,    // 1Gb cutoff
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 1Gb
        .chip_name = "ESMT F50L1G41LB",
        .chip_device_id = {ESMTPART, ID_F50L1G41LB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_esmt2,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "ESMT F50L2G41LB",
        .chip_device_id = {ESMTPART, ID_F50L2G41LB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0x8000000,    // 1Gb cutoff
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_esmt2,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "ESMT F50L2G41KA",
        .chip_device_id = {ESMTPART, ID_F50L2G41KA, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_esmt2,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 512Mb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25N512GV",
        .chip_device_id = {WINBONDPART, ID_W25N512GV_1, ID_W25N512GV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 512, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 1Gb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25N01GV",
        .chip_device_id = {WINBONDPART, ID_W25N01GV_1, ID_W25N01GV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25N02GV",
        .chip_device_id = {WINBONDPART, ID_W25N02GV_1, ID_W25N02GV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25M02GV",
        .chip_device_id = {WINBONDPART, ID_W25M02GV_1, ID_W25M02GV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0x8000000,    // 1Gb cutoff
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 4Gb, please note we do not support contiuous read mode xIG parts, only xIR parts
        .chip_name = "Winbond W25N04KV",
        .chip_device_id = {WINBONDPART, ID_W25N04KV_1, ID_W25N04KV_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 4096,      // 4096 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 4096, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 1Gb
        .chip_name = "Macronix MX35LF1GE4AB",
        .chip_device_id = {MACRONIXPART, ID_MX35LF1GE4AB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_mxic,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "Macronix MX35LF2GE4AB",
        .chip_device_id = {MACRONIXPART, ID_MX35LF2GE4AB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_mxic,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "Macronix MX35LF2GE4AD",
        .chip_device_id = {MACRONIXPART, ID_MX35LF2GE4AD, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 4Gb
        .chip_name = "Macronix MX35LF4GE4AD",
        .chip_device_id = {MACRONIXPART, ID_MX35LF4GE4AD, 0, 0},
        .chip_page_size = 4096,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 256,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 4096 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "Macronix MX35LF2GE4AD",
        .chip_device_id = {MACRONIXPART, ID_MX35LF2GE4AD_1, ID_MX35LF2GE4AD_2, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_mxic_ad,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 1Gb
        .chip_name = "Toshiba TC58CVG0S",
        .chip_device_id = {TOSHIBAPART, ID_TC58CVG0S, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip, must do this for Toshiba because ECC is not visible when enabled
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "Toshiba TC58CVG1S",
        .chip_device_id = {TOSHIBAPART, ID_TC58CVG1S, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip, must do this for Toshiba because ECC is not visible when enabled
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 2Gb
        .chip_name = "Toshiba TC58CVG1S0HRAIJ",
        .chip_device_id = {TOSHIBAPART, ID_TC58CVG1S0HRAIJ, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip, must do this for Toshiba because ECC is not visible when enabled
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 4Gb
        .chip_name = "Toshiba TC58CVG2S",
        .chip_device_id = {TOSHIBAPART, ID_TC58CVG2S, 0, 0},
        .chip_page_size = 4096,
        .chip_page_shift = 12,
        .chip_block_size = 64 * 4096, // 64 pages per block x chip_page_size
        .chip_block_shift = 18,       // block size of 0x40000 (256KB)
        .chip_spare_size = 256,
        .chip_ecc_offset = 0x1000,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 4096 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_gigadevice_4k,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip, must do this for Toshiba because ECC is not visible when enabled
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 4Gb
        .chip_name = "Toshiba TC58CVG2S0HRAIJ",
        .chip_device_id = {TOSHIBAPART, ID_TC58CVG2S0HRAIJ, 0, 0},
        .chip_page_size = 4096,
        .chip_page_shift = 12,
        .chip_block_size = 64 * 4096, // 64 pages per block x chip_page_size
        .chip_block_shift = 18,       // block size of 0x40000 (256KB)
        .chip_spare_size = 256,
        .chip_ecc_offset = 0x1000,    // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 4096 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 1, // enhanced bad bit detection by chip, must do this for Toshiba because ECC is not visible when enabled
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
    { // 1Gb
        .chip_name = "Etron EM73C044SNB",
        .chip_device_id = {ETRONPART, ID_EM73C044SNB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_etron,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 80000000,
    },
    { // 1Gb
        .chip_name = "Etron EM73C044VCD",
        .chip_device_id = {ETRONPART, ID_EM73C044VCD, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 80000000,
    },
    { // 2Gb
        .chip_name = "Etron EM73D044SNF",
        .chip_device_id = {ETRONPART, ID_EM73D044SNF, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_etron2,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 80000000,
    },
    { // 2Gb
        .chip_name = "Etron EM73D044VCG",
        .chip_device_id = {ETRONPART, ID_EM73D044VCG, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 80000000,
    },
    { // 1Gb
        .chip_name = "XTX XT26G01A", // fix extended ECC
        .chip_device_id = {XTXPART, ID_XT26G01A, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_xtx,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 90000000,
    },
    { // 2Gb
        .chip_name = "XTX XT26G02A",
        .chip_device_id = {XTXPART, ID_XT26G02A, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 2048 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_xtx,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 90000000,
    },
    { // 1Gb
        .chip_name = "FM 25S01",
        .chip_device_id = {FMPART, ID_FM25S01, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_etron,
        .chip_ecc = 4, // ECC bits
        .chip_ecc_corr = 3, // threshold to fix correctable bits (3/4)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
/*
    { // 1Gb
        .chip_name = "FM 25S01A",
        .chip_device_id = {FMPART, ID_FM25S01A, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/1)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,
    },
*/
    { // 2Gb
        .chip_name = "DOSILICON DS35Q2GB",               //add for DS flash
        .chip_device_id = {DOSILICON, ID_DS35Q2GB, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 128,
        .chip_ecc_offset = 0x840,     // location of ECC bytes
        .chip_num_blocks = 2048,      // 1024 blocks total
        .chip_num_planes = 2,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 2048, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_toshiba_micron_ab,
        .chip_ecc = 8, // ECC bits
        .chip_ecc_corr = 6, // threshold to fix correctable bits (6/8)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 104000000,                    //end
    },
    { // 1Gb
        .chip_name = "!!! UNKNOWN !!! Default mode",
        .chip_device_id = {0, 0, 0, 0},
        .chip_page_size = 2048,
        .chip_page_shift = 11,
        .chip_block_size = 64 * 2048, // 64 pages per block x chip_page_size
        .chip_block_shift = 17,       // block size of 0x20000 (128KB)
        .chip_spare_size = 64,
        .chip_ecc_offset = 0x800,     // location of ECC bytes
        .chip_num_blocks = 1024,      // 1024 blocks total
        .chip_num_planes = 1,
        .chip_die_sel = 0,
        .chip_total_size = 64 * 2048 * 1024, // chip_block_size x chip_num_blocks
        .ecclayout = &spinand_oob_micron_aa,
        .chip_ecc = 1, // ECC bits
        .chip_ecc_corr = 1, // threshold to fix correctable bits (1/?)
        .chip_ecc_enh = 0, // enhanced bad bit detection by chip (none)
        .chip_subpage_shift = 2, // 2^ shift amount based on number of subpages (4)
        .chip_clock_speed = 50000000,
    }
};


static struct spi_device * pSpiDevice; // handle for SPI NAND device

static unsigned char * pageBuf;
static unsigned int pageBufI;
static int pageAddr, pageOffset;
static int status = STATUS_DEFAULT;
static bool SpiNandDeviceRegistered = 0;

/** Prototypes. **/
static int spi_nand_read_page(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len);
static int spi_nand_write_page(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len);
static int spi_nand_is_blk_bad(unsigned long addr);
static int spi_nand_mark_blk_bad(unsigned long addr);
static int spi_nand_write_enable(void);
static int spi_nand_write_disable(void);
static void spi_nand_row_addr(unsigned int page_addr, unsigned char* buf);
static void spi_nand_col_addr(unsigned int page_addr, unsigned int page_offset, unsigned char* buf);
static void spi_nand_get_device_id(unsigned char * buf, unsigned int len);
static int spi_nand_wel(void);

static int spiRead( struct spi_transfer *xfer );
static int spiWrite( unsigned char *msg_buf, int nbytes );
static void spi_nand_read_cfg(void);

static int spi_nand_device_reset(void);
static int spi_nand_status(void);
static int spi_nand_ready(void);
static int spi_nand_ecc(void);
static int spi_nand_sector_erase_int(unsigned long addr);

static int spi_nand_get_cmd(unsigned char command, unsigned char feat_addr);
static void spi_nand_set_feat(unsigned char feat_addr, unsigned char feat_val);

static void bcm63xx_cmd(struct mtd_info *mtd, unsigned int command, int column, int page);
static unsigned char bcm63xx_read_byte(struct mtd_info *mtd);
static void bcm63xx_read_buf(struct mtd_info *mtd, uint8_t *buf, int len);
static void bcm63xx_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len);
static int bcm63xx_status(struct mtd_info *mtd, struct nand_chip *chip);
static int bcm63xx_block_isbad(struct mtd_info *mtd, loff_t ofs);
static int bcm63xx_block_markbad(struct mtd_info *mtd, loff_t ofs);
static void bcm63xx_select(struct mtd_info *mtd, int chip);


static int spiRead(struct spi_transfer *xfer)
{
    if (!SpiNandDeviceRegistered)
    {
        printk("ERROR!! SPI NAND read without SPI NAND Linux device registration\n");
        return(0);
    }

    {
        struct spi_message  message;

        spi_message_init(&message);
        spi_message_add_tail(xfer, &message);
#if defined(CONFIG_SPI_BCM63XX_HSSPI)
        return(spi_sync(pSpiDevice, &message));
#else
        /* the controller does not support asynchronous transfer,
           when spi_async returns the transfer will be complete
           don't use spi_sync (to avoid the call to schedule),
           scheduling will conflict with atomic operations
           such as writing image from Linux */
        return(spi_async(pSpiDevice, &message));
#endif
    }
}


static int spiWrite(unsigned char *msg_buf, int nbytes)
{
    if (!SpiNandDeviceRegistered)
    {
        printk("ERROR!! SPI NAND write without SPI NAND Linux device registration\n");
        return(0);
    }

    {
        struct spi_message  message;
        struct spi_transfer xfer;

        spi_message_init(&message);
        memset(&xfer, 0, (sizeof xfer));
        xfer.prepend_cnt = 0;
        xfer.len         = nbytes;
        xfer.speed_hz    = spi_flash_clock;
        xfer.rx_buf      = NULL;
        xfer.tx_buf      = msg_buf;

        spi_message_add_tail(&xfer, &message);

#if defined(CONFIG_SPI_BCM63XX_HSSPI)
        return(spi_sync(pSpiDevice, &message));
#else
        /* the controller does not support asynchronous transfer
           when spi_async returns the transfer will be complete
           don't use spi_sync to avoid the call to schedule */
        return(spi_async(pSpiDevice, &message));
#endif
    }
}

static void spi_nand_read_cfg(void)
{ // search through SPI NAND devices to find match
    unsigned char buf[SPI_NAND_ID_LENGTH];
    int i = 0;

    spi_nand_get_device_id(buf, SPI_NAND_ID_LENGTH);

    do
    {
        if ((SpiDevInfo[i].chip_device_id[0] == buf[0]) &&
            (SpiDevInfo[i].chip_device_id[1] == buf[1]) && 
           ((SpiDevInfo[i].chip_device_id[2] == 0) || (SpiDevInfo[i].chip_device_id[2] == buf[2])) && 
           ((SpiDevInfo[i].chip_device_id[3] == 0) || (SpiDevInfo[i].chip_device_id[3] == buf[3])))
            break;
        i++;
    } while((SpiDevInfo[i].chip_device_id[0] != 0) || (SpiDevInfo[i].chip_device_id[1] != 0));

    pchip = &SpiDevInfo[i];

    if ((pchip->chip_device_id[0] == 0) && (pchip->chip_device_id[1] == 0))
    {
        printk("ERROR!!! Unsupported SPI NAND device ID = 0x");
        for (i = 0; i < SPI_NAND_ID_LENGTH; i++)
            printk("%02x", buf[i]);

        printk(", halting system\n");
        while(1) ;
    }

    memcpy(pchip->chip_device_id, buf, SPI_NAND_ID_LENGTH);
}

/***********************************************************************/
/* reset SPI NAND device and get configuration information             */
/* some devices such as Micron MT29F1G01 require explicit reset before */
/* access to the device.                                               */
/***********************************************************************/
static int spi_nand_device_reset(void)
{
    unsigned char buf[4];
#if defined(CONFIG_BRCM_IKOS)
    unsigned int i;
    for( i = 0; i < 250; i++);
#else
    udelay(300);
#endif
    if (!spin_is_locked(&chip_lock)) // show status only if initial reset since Linux NAND code resets chip during every block erase
        printk("SPI NAND device reset\n");
    buf[0]        = FLASH_RESET;
    spiWrite(buf, 1);

#if defined(CONFIG_BRCM_IKOS)
    for( i = 0; i < 3000; i++);
#else
    /* device is availabe after 5ms */
    mdelay(5);
#endif
    while(!spi_nand_ready()); // do we need this here??

    spi_nand_set_feat(FEATURE_PROT_ADDR, FEAT_DISABLE); // disable block locking

    spi_nand_read_cfg();

    return(FLASH_API_OK);
}

/*****************************************************************************************/
/*  row address is 24 bit length. so buf must be at least 3 bytes.                       */
/*  For gigadevcie GD5F1GQ4 part(2K page size, 64 page per block and 1024 blocks)        */
/*  Row Address. RA<5:0> selects a page inside a block, and RA<15:6> selects a block and */
/*  first byte is dummy byte                                                             */
/*****************************************************************************************/
static void spi_nand_row_addr(unsigned int page_addr, unsigned char* buf)
{
    buf[0] = (unsigned char)(page_addr>>(pchip->chip_page_shift+16)); //dummy byte
    buf[1] = (unsigned char)(page_addr>>(pchip->chip_page_shift+8));
    buf[2] = (unsigned char)(page_addr>>(pchip->chip_page_shift));

    return;
}

/*********************************************************************************************************************/
/*  column address select the offset within the page. For gigadevcie GD5F1GQ4 part(2K page size and 2112 with spare) */
/*  is 12 bit length. so buf must be at least 2 bytes. The 12 bit address is capable of address from 0 to 4095 bytes */
/*  however only byte 0 to 2111 are valid.                                                                           */
/*********************************************************************************************************************/
static void spi_nand_col_addr(unsigned int page_addr, unsigned int page_offset, unsigned char* buf)
{
    page_offset = page_offset&((1<<(pchip->chip_page_shift+1))-1);  /* page size + spare area size */

    /* the upper 4 bits of buf[0] is either wrap bits for gigadevice or dummy bit[3:1] + plane select bit[0] for micron
     */
    if( pchip->chip_num_planes > 1 )
    {
        /* setup plane bit if more than one plane. otherwise that bit is always 0 */
        buf[0] = (unsigned char)(((page_offset>>8)&0xf)|((page_addr>>pchip->chip_block_shift)&0x1)<<4); //plane bit is the first bit of the block number RowAddr[6]
    }
    else
    {
        /* use default wrap option 0, wrap length 2112 */
        buf[0] = (unsigned char)((page_offset>>8)&0xFF);
    }
    buf[1] = (unsigned char)(page_offset&0xFF);

    return;
}

/***************************************************************************
 * Function Name: spi_xfr
 * Description  : Commonly used SPI transfer function.
 * Returns      : nothing
 ***************************************************************************/
static void spi_xfr(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len)
{
    int maxread;
    unsigned char buf[4];
    struct spi_transfer xfer;

    while (len > 0)
    { // break up NAND buffer read into SPI buffer sized chunks
       /* Random data read (0Bh or 03h) command to read the page data from the cache
          The RANDOM DATA READ command requires 4 dummy bits, followed by a 12-bit column
          address for the starting byte address and a dummy byte for waiting data.
          This is only for 2K page size, the format will change for other page size.
       */

        maxread = (len < spi_max_op_len) ? len : spi_max_op_len;

        buf[0] = FLASH_READ;
        spi_nand_col_addr(page_addr, page_offset, buf+1);
        buf[3] = 0; //dummy byte

        if ((page_offset < pchip->chip_page_size) && ((maxread + page_offset) > pchip->chip_page_size))
            maxread = pchip->chip_page_size - page_offset; // snap address to OOB boundary to let chip know we want OOB

        if ((page_offset < pchip->chip_ecc_offset) && ((maxread + page_offset) > pchip->chip_ecc_offset))
            maxread = pchip->chip_ecc_offset - page_offset; // snap address to ECC boundary to let chip know we want ECC

        DBG_PRINTF("spi_xfr - spi cmd 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0],buf[1],buf[2],buf[3]);
        DBG_PRINTF("spi_xfr - spi read len 0x%x, offset 0x%x, remaining 0x%x\n", maxread, page_offset, len);

        memset(&xfer, 0, sizeof(struct spi_transfer));
        xfer.tx_buf      = buf;
        xfer.rx_buf      = buffer;
        xfer.len         = maxread;
        xfer.speed_hz    = spi_flash_clock;
        xfer.prepend_cnt = 4;
        xfer.addr_len    = 3; // length of address field (max 4 bytes)
        xfer.addr_offset = 1; // offset of first addr byte in header
        xfer.hdr_len     = 4; // length of header
        xfer.unit_size   = 1; // data for each transfer will be divided into multiples of unit_size
        spiRead(&xfer);
        while (!spi_nand_ready());

        buffer += maxread;
        len -= maxread;
        page_offset += maxread;
    }
}

#if defined(COUNT_BAD_BITS)
/***************************************************************************
 * Function Name: count_bits
 * Description  : Counts the bit differences between two buffers.
 * Returns      : Bit difference count
 ***************************************************************************/
static int count_bits(unsigned char * buf1, unsigned char * buf2, int len)
{
    int i, count = 0;
    unsigned char hold;

    for(i = 0; i < len; i++)
    {
        hold = buf1[i] ^ buf2[i];
        while(hold)
        {
            hold &= (hold-1);
            count++;
        }
    }

    return(count);
}
#endif // COUNT_BAD_BITS

/***************************************************************************
 * Function Name: spi_nand_read_page
 * Description  : Reads up to a NAND block of pages into the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR or FLASH_API_CORR
 ***************************************************************************/
int bit_flips;

static int spi_nand_read_page(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len)
{
    int status = FLASH_API_OK;
    unsigned char buf[4];

    bit_flips = 0;

    if ((page_offset + len) > (pchip->chip_page_size + pchip->chip_spare_size)) // check to see if reading within page/OOB boundary
    {
        printk("spi_nand_read_page(): Attempt to read past page boundary, offset 0x%x, length 0x%x, into page address 0x%x\n", page_offset, len, (unsigned int)page_addr);
        return (FLASH_API_ERROR);
    }

    if (pchip->chip_die_sel)
    {
        if ( (pchip->chip_device_id[0] == WINBONDPART) || (pchip->chip_device_id[0] == ESMTPART) )
        {
            while(!spi_nand_ready());

            buf[0] = FLASH_DIE_SELECT;
            buf[1] = (pchip->chip_die_sel & page_addr) ? 1 : 0;
            spiWrite(buf, 2);
        }

        if (pchip->chip_device_id[0] == MICRONPART)
            spi_nand_set_feat(FEATURE_DIE_SEL, (pchip->chip_die_sel & page_addr) ? FEAT_OPT_EN : FEAT_DISABLE);
    }

    if (len != 1 || (page_offset != pchip->chip_page_size))
        spi_nand_set_feat(FEATURE_FEAT_ADDR, FEAT_ECC_EN); // not reading from bad block marker, enable ECC and even if there's a failure should still fill buffer
    else
        spi_nand_set_feat(FEATURE_FEAT_ADDR, FEAT_DISABLE); // else reading bad block marker so don't enable ECC

    /* The PAGE READ (13h) command transfers the data from the NAND Flash array to the
     * cache register.  The PAGE READ command requires a 24-bit address consisting of
     * 8 dummy bits followed by a 16-bit block/page address.
     */
    buf[0] = FLASH_PREAD;
    spi_nand_row_addr(page_addr, buf+1);
    DBG_PRINTF("spi_nand_read_page - spi cmd 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3]);
    spiWrite(buf, 4);

    /* GET FEATURES (0Fh)  command to read the status */
    while(!spi_nand_ready());

    if (page_offset < pchip->chip_page_size)
        status = spi_nand_ecc();

    if (!len)
        return(status);

    spi_xfr(page_addr, page_offset, buffer, len);

    if(status == FLASH_API_CORR)
    { // count bad bits to see if we exceed threshold
        bit_flips = 1; // there's at least 1 bit flip when we have a correctable status

        if(pchip->chip_ecc_enh)
        { // chip has enhanced bad bit detection
            if (pchip->chip_device_id[0] == GIGADEVPART)
                bit_flips = ((spi_nand_get_cmd(FLASH_GFEAT, FEATURE_STAT_AUX) & STAT_ECC_MASK1) >> 4) | 0x4;
            else if (pchip->chip_device_id[0] == MACRONIXPART)
                bit_flips = spi_nand_get_cmd(FLASH_SREAD, 0) & STAT_ECC_MASK2;
            else if (pchip->chip_device_id[0] == TOSHIBAPART)
                bit_flips = (spi_nand_get_cmd(FLASH_GFEAT, FEATURE_STAT_ENH) & STAT_ECC_MASK3) >> 4;
			else if ((pchip->chip_device_id[0] == WINBONDPART) && (pchip->chip_device_id[1] == ID_W25N02KV_1) && (pchip->chip_device_id[2] == ID_W25N02KV_2))
                bit_flips = (spi_nand_get_cmd(FLASH_GFEAT, FEATURE_STAT_ENH) & STAT_ECC_MASK3) >> 4;
        }
#if defined(COUNT_BAD_BITS)
        else if (pchip->chip_ecc_corr != 1) // If correctable status and threshold is 1 bit no need to check we are already at correctable reporting threshold
        {
            unsigned char *buf_ecc = kmalloc(sizeof(unsigned char) * (pchip->chip_page_size + pchip->chip_spare_size), GFP_ATOMIC);
            unsigned char *buf_noecc = kmalloc(sizeof(unsigned char) * (pchip->chip_page_size + pchip->chip_spare_size), GFP_ATOMIC);
            int i, size, count;
			
            if((NULL == buf_ecc) || (NULL == buf_noecc))
            {
                printk("spi_nand_read_page(): kmalloc failed to allocate memory\n");

                if(buf_ecc)
                    kfree(buf_ecc);

                if(buf_noecc)
                    kfree(buf_noecc);

                return (FLASH_API_ERROR);
            }			

            spi_xfr(page_addr, 0, buf_ecc, pchip->chip_page_size + pchip->chip_spare_size);

            spi_nand_set_feat(FEATURE_FEAT_ADDR, FEAT_DISABLE); // now grab data with ecc turned off

            /* The PAGE READ (13h) command transfers the data from the NAND Flash array to the
             * cache register.  The PAGE READ command requires a 24-bit address consisting of
             * 8 dummy bits followed by a 16-bit block/page address.
             */
            buf[0] = FLASH_PREAD;
            spi_nand_row_addr(page_addr, buf+1);
            spiWrite(buf, 4);

            while(!spi_nand_ready());

            spi_xfr(page_addr, 0, buf_noecc, pchip->chip_page_size + pchip->chip_spare_size);

            for(i = 0; i < (1 << pchip->chip_subpage_shift); i++)
            {
                count = 0;

                size = pchip->chip_page_size >> pchip->chip_subpage_shift;
                count += count_bits(buf_ecc + (size * i), buf_noecc + (size * i), size);

                size = (pchip->chip_spare_size - (pchip->chip_ecc_offset - pchip->chip_page_size)) >> pchip->chip_subpage_shift;
                count += count_bits(buf_ecc + pchip->chip_page_size + (size * i), buf_noecc + pchip->chip_page_size + (size * i), size);

                if(pchip->chip_page_size != pchip->chip_ecc_offset)
                    count += count_bits(buf_ecc + pchip->chip_ecc_offset + (size * i), buf_noecc + pchip->chip_ecc_offset + (size * i), size);

                if (count > bit_flips)
                    bit_flips = count;
            }

            kfree(buf_ecc);
            kfree(buf_noecc);			
        }
#endif // COUNT_BAD_BITS

       if (bit_flips < pchip->chip_ecc_corr)
          status = FLASH_API_OK;

    }

    return(status);
}

/*********************************************************************/
/* Flash_status return the feature status byte                       */
/*********************************************************************/
static int spi_nand_status(void)
{
    return spi_nand_get_cmd(FLASH_GFEAT, FEATURE_STAT_ADDR);
}

/* check device ready bit */
static int spi_nand_ready(void)
{
    return (spi_nand_status()&STAT_OIP) ? 0 : 1;
}

/*********************************************************************/
/*  spi_nand_get_cmd return the resultant byte at command, address   */
/*********************************************************************/
static int spi_nand_get_cmd(unsigned char command, unsigned char feat_addr)
{
    unsigned char buf[4];
    struct spi_transfer xfer;

    /* check device is ready */
    memset(&xfer, 0, sizeof(struct spi_transfer));

    buf[0]           = command;
    buf[1]           = feat_addr;
    xfer.tx_buf      = buf;
    xfer.rx_buf      = buf;
    xfer.len         = 1;
    xfer.speed_hz    = spi_flash_clock;
    xfer.prepend_cnt = 2;
    spiRead(&xfer);

    DBG_PRINTF("spi_nand_get_cmd at 0x%x 0x%x\n", feat_addr, buf[0]);

    return buf[0];
}

/*********************************************************************/
/*  spi_nand_set_feat set the feature byte at feat_addr              */
/*********************************************************************/
static void spi_nand_set_feat(unsigned char feat_addr, unsigned char feat_val)
{
    unsigned char buf[3];

    /* check device is ready */
    while(!spi_nand_ready());

    buf[0]           = FLASH_SFEAT;
    buf[1]           = feat_addr;
    buf[2]           = feat_val;
    spiWrite(buf, 3);

    return;
}

static int spi_nand_ecc(void)
{
    int status;

    status = spi_nand_get_cmd(FLASH_GFEAT, FEATURE_STAT_ADDR);
    status = status & STAT_ECC_MASK1;

    if (status == STAT_ECC_GOOD)
        return(FLASH_API_OK);

    if (status == STAT_ECC_UNCORR) // uncorrectable errors
        return(FLASH_API_ERROR);

     return(FLASH_API_CORR); // everything else is correctable
}

/*********************************************************************/
/* Flash_sector__int() wait until the erase is completed before      */
/* returning control to the calling function.  This can be used in   */
/* cases which require the program to hold until a sector is erased, */
/* without adding the wait check external to this function.          */
/*********************************************************************/
static int spi_nand_sector_erase_int(unsigned long addr)
{
    unsigned char buf[11];
    int status;

    addr &= ~(pchip->chip_block_size - 1);

    DBG_PRINTF("spi_nand_sector_erase_int block at address 0x%lx\n", addr);

    if (spi_nand_is_blk_bad(addr))
    {
        printk("spi_nand_sector_erase_int(): Attempt to erase failed due to bad block 0x%lx (address 0x%lx)\n", addr >> pchip->chip_block_shift, addr);
        return (FLASH_API_ERROR);
    }

    if (pchip->chip_die_sel)
    {
        if ( (pchip->chip_device_id[0] == WINBONDPART) || (pchip->chip_device_id[0] == ESMTPART) )
        {
            while(!spi_nand_ready());

            buf[0] = FLASH_DIE_SELECT;
            buf[1] = (pchip->chip_die_sel & addr) ? 1 : 0;
            spiWrite(buf, 2);
        }

        if (pchip->chip_device_id[0] == MICRONPART)
            spi_nand_set_feat(FEATURE_DIE_SEL, (pchip->chip_die_sel & addr) ? FEAT_OPT_EN : FEAT_DISABLE);
    }

    { // erase dirty block
        spi_nand_write_enable();
        buf[0] = FLASH_BERASE;
        spi_nand_row_addr(addr, buf+1);
        spiWrite(buf, 4);
        while(!spi_nand_ready()) ;

        status = spi_nand_status();
        if( status & STAT_EFAIL )
        {
            printk("spi_nand_sector_erase_int(): Erase block 0x%lx failed, sts 0x%x\n",  addr >> pchip->chip_block_shift, status);
            return(FLASH_API_ERROR);
        }

        spi_nand_write_disable();
    }

    return (FLASH_API_OK);
}

/************************************************************************/
/* flash_write_enable() must be called before any change to the         */
/* device such as write, erase. It also unlocks the blocks if they were */
/* previouly locked.                                                    */
/************************************************************************/
static int spi_nand_write_enable(void)
{
    unsigned char buf[4], prot;

    /* make sure it is not locked first */
    prot = spi_nand_get_cmd(FLASH_GFEAT, FEATURE_PROT_ADDR);
    if( prot != 0 )
    {
        prot = 0;
        spi_nand_set_feat(FEATURE_PROT_ADDR, prot);
    }

    /* send write enable cmd and check feature status WEL latch bit */
    buf[0] = FLASH_WREN;
    spiWrite(buf, 1);
    while(!spi_nand_ready());
    while(!spi_nand_wel());

    return(FLASH_API_OK);
}

static int spi_nand_write_disable(void)
{
    unsigned char buf[4];

    buf[0] = FLASH_WRDI;
    spiWrite(buf, 1);
    while(!spi_nand_ready());
    while(spi_nand_wel());

    return(FLASH_API_OK);
}

/***************************************************************************
 * Function Name: spi_nand_write_page
 * Description  : Writes up to a NAND block of pages from the specified buffer.
 * Returns      : FLASH_API_OK or FLASH_API_ERROR
 ***************************************************************************/
static int spi_nand_write_page(unsigned long page_addr, unsigned int page_offset, unsigned char *buffer, int len)
{
    int maxwrite, status;
    unsigned int page_ofs = page_offset;
    int count=0;
    unsigned char *spi_buf  = kmalloc(sizeof(unsigned char) * spi_max_op_len, GFP_ATOMIC);  /* HS_SPI_BUFFER_LEN SPI controller fifo size is currently 512 bytes */
    size_t xfer_buf_size = sizeof(unsigned char) * (pchip->chip_page_size + pchip->chip_spare_size);
    unsigned char *xfer_buf = kmalloc(xfer_buf_size, GFP_ATOMIC);

    if((NULL == spi_buf) || (NULL == xfer_buf))
    {
        printk("spi_nand_write_page(): kmalloc failed to allocate memory\n");

        if(spi_buf)
            kfree(spi_buf);
        if(xfer_buf)
            kfree(xfer_buf);

        return (FLASH_API_ERROR);
    }

    if (!len)
    {
        printk("spi_nand_write_page(): Not writing any data to page addr 0x%x, page_offset 0x%x, len 0x%x\n", (unsigned int)page_addr, page_offset, len);
        kfree(spi_buf);
        kfree(xfer_buf);
        return (FLASH_API_OK);
    }

    if ((page_offset + len) > (pchip->chip_page_size + pchip->chip_spare_size))
    {
        printk("spi_nand_write_page(): Attempt to write past page boundary, offset 0x%x, length 0x%x, into page address 0x%x\n", page_offset, len, (unsigned int)page_addr);
        kfree(spi_buf);
        kfree(xfer_buf);
        return (FLASH_API_ERROR);
    }

    if (pchip->chip_die_sel)
    {
        if ( (pchip->chip_device_id[0] == WINBONDPART) || (pchip->chip_device_id[0] == ESMTPART) )
        {
            while(!spi_nand_ready());

            spi_buf[0] = FLASH_DIE_SELECT;
            spi_buf[1] = pchip->chip_die_sel & page_addr ? 1 : 0;
            spiWrite(spi_buf, 2);
        }

        if (pchip->chip_device_id[0] == MICRONPART)
            spi_nand_set_feat(FEATURE_DIE_SEL, (pchip->chip_die_sel & page_addr) ? FEAT_OPT_EN : FEAT_DISABLE);
    }

    if (page_ofs < pchip->chip_page_size)
    { /* writing into page area, if writing into spare area is allowed then must read page first to fill write buffer
       * because we don't know if JFFS2 clean marker is there or not and this clean marker would initially have
       * been written with ECC off, but will now be included in the ECC calculation along with the page data */
        spi_nand_set_feat(FEATURE_FEAT_ADDR, FEAT_ECC_EN); // enable ECC if writing to page
    }
    else
    { // not writing into page area
        if (len != 1 || (page_ofs != pchip->chip_page_size))
        {
            kfree(spi_buf);
            kfree(xfer_buf);
            return(FLASH_API_OK); // only allowed write is the bad block marker; return if not that
        }

        spi_nand_set_feat(FEATURE_FEAT_ADDR, FEAT_DISABLE); // else don't write ECC
    }

    memset(xfer_buf, 0xff, xfer_buf_size);
    memcpy(xfer_buf + page_offset, buffer, len);
    len = pchip->chip_page_size + pchip->chip_spare_size;
    page_offset = 0;

    while (len > 0)
    {
        /* Send Program Load Random Data Command (0x84) to load data to cache register.
         * PROGRAM LOAD consists of an 8-bit Op code, followed by 4 bit dummy and a
         * 12-bit column address, then the data bytes to be programmed. */
        if(pchip->chip_device_id[0] == DOSILICON)
        {
           if(count == 0)
			   spi_buf[0] = FLASH_PROG;
           else
               spi_buf[0] = FLASH_PROG_RAN;

        }
		else
            spi_buf[0] = FLASH_PROG_RAN;

        spi_nand_col_addr(page_addr, page_offset, spi_buf + 1);

        maxwrite = (len > (spi_max_op_len - 5)) ? (spi_max_op_len - 5) : len;

        if ((page_offset < pchip->chip_page_size) && ((maxwrite + page_offset) > pchip->chip_page_size))
            maxwrite = pchip->chip_page_size - page_offset; // snap address to OOB boundary to let chip know we want OOB

        if ((page_offset < pchip->chip_ecc_offset) && ((maxwrite + page_offset) > pchip->chip_ecc_offset))
            maxwrite = pchip->chip_ecc_offset - page_offset; // snap address to ECC boundary to let chip know we want ECC

        memcpy(&spi_buf[3], xfer_buf + page_offset, maxwrite);
        DBG_PRINTF("spi_nand_write_page - spi cmd 0x%x, 0x%x, 0x%x\n", spi_buf[0], spi_buf[1], spi_buf[2]);
        DBG_PRINTF("spi_nand_write_page - spi write len 0x%x, offset 0x%x, remaining 0x%x\n", maxwrite, offset, len-maxwrite);

        spi_nand_write_enable();
        spiWrite(spi_buf, maxwrite + 3);

        len -= maxwrite;
        page_offset += maxwrite;

        while(!spi_nand_ready()); // do we need this here??
    }

    /* Send Program Execute command (0x10) to write cache data to memory array
     * Send address (24bit): 8 bit dummy + 16 bit address (page/Block)
     */
    /* Send Write enable Command (0x06) */
    spi_nand_write_enable();

    spi_buf[0] = FLASH_PEXEC;
    spi_nand_row_addr(page_addr, spi_buf + 1);
    DBG_PRINTF("spi_nand_write_page - spi cmd 0x%x, 0x%x, 0x%x, 0x%x\n", spi_buf[0], spi_buf[1], spi_buf[2], spi_buf[3]);
    spiWrite(spi_buf, 4);
    while(!spi_nand_ready());

    status = spi_nand_status();
    spi_nand_write_disable();

    if(status & STAT_PFAIL)
    {
        printk("Page program failed at address 0x%x, sts 0x%x\n", (unsigned int)page_addr, status);
        kfree(spi_buf);
        kfree(xfer_buf);
        return(FLASH_API_ERROR);
    }

    if (page_ofs < pchip->chip_page_size)
    {
        unsigned char *buf = kmalloc((sizeof(unsigned char) * pchip->chip_page_size), GFP_ATOMIC);  

        if(NULL == buf)
        {
            printk("spi_nand_write_page(): kmalloc failed to allocate memory\n");
            kfree(spi_buf);
            kfree(xfer_buf);			
            return (-ENOMEM);
        }
	
        status = spi_nand_read_page(page_addr, 0, buf, pchip->chip_page_size);

        if (status == FLASH_API_ERROR)
        {
            printk("Write verify failed reading back page at address 0x%lx\n", page_addr);
            kfree(spi_buf);
            kfree(xfer_buf);
            kfree(buf);
            return(FLASH_API_ERROR);
        }

        if (memcmp(xfer_buf, buf, pchip->chip_page_size))
        {
            printk("Write data did not match read data at address 0x%lx\n", page_addr);
            kfree(spi_buf);
            kfree(xfer_buf);
            kfree(buf);
            return(FLASH_API_ERROR);
        }

        if (status == FLASH_API_CORR)
        {
            printk("Write verify correctable errors at address 0x%lx\n", page_addr);
            kfree(spi_buf);
            kfree(xfer_buf);
            kfree(buf);
            return(FLASH_API_CORR);
        }
		
        kfree(buf);
    }

    if (spi_buf)
        kfree(spi_buf);

    if (xfer_buf)
        kfree(xfer_buf);

    return (FLASH_API_OK);
}

/* check device write enable latch bit */
static int spi_nand_wel(void)
{
    return (spi_nand_status() & STAT_WEL) ? 1 : 0;
}

/*********************************************************************/
/* flash_get_device_id() return the device id of the component.      */
/*********************************************************************/
static void spi_nand_get_device_id(unsigned char * buf, unsigned int len)
{
    unsigned char buffer[2];
    struct spi_transfer xfer;

    while(!spi_nand_ready());

    memset(&xfer, 0, sizeof(struct spi_transfer));
    buffer[0]        = FLASH_RDID;
    buffer[1]        = 0;
    xfer.tx_buf      = buffer;
    xfer.rx_buf      = buf;
    xfer.len         = len;
    xfer.speed_hz    = spi_flash_clock;
    xfer.prepend_cnt = 2;
    spiRead(&xfer);
    while(!spi_nand_ready());

    DBG_PRINTF("spi_nand_get_device_id 0x%x 0x%x\n", buf[0], buf[1]);
}

static int spi_nand_is_blk_bad(unsigned long addr)
{
    unsigned char buf;

    if (addr < pchip->chip_block_size)
        return 0; // always return good for block 0, because if it's a bad chip quite possibly the board is useless

    addr &= ~(pchip->chip_block_size - 1);

    spi_nand_read_page(addr, pchip->chip_page_size, &buf, 1);

    if (0xFF != buf)
        return(1);

    return(0);
}

static int spi_nand_mark_blk_bad(unsigned long addr)
{
    int ret1, ret2;

    addr &= ~(pchip->chip_block_size - 1);

    printk("Marking block 0x%lx bad (address 0x%lx)\n", addr >> pchip->chip_block_shift, addr);

    ret1 = spi_nand_write_page(addr, pchip->chip_page_size, "\0", 1); // write bad block marker into first page
    ret2 = spi_nand_write_page(addr + pchip->chip_page_size, pchip->chip_page_size, "\0", 1); // write bad block marker into second page

    if ((ret1 != FLASH_API_OK) && (ret2 != FLASH_API_OK))
    {
        printk("Unable to mark block 0x%lx bad\n", addr >> pchip->chip_block_shift);
        return(FLASH_API_ERROR);
    }

    return(FLASH_API_OK);
}

static void bcm63xx_cmd(struct mtd_info *mtd, unsigned int command, int column, int page)
{
    unsigned long addr = page * mtd->writesize;

    spin_lock(&chip_lock);

    switch(command)
    {
        case NAND_CMD_READ0:
        case NAND_CMD_READ1: // step 1/2 for read, execute SPI NAND read command and transfer SPI NAND data to local buffer

            status = STATUS_DEFAULT;

            if (addr > mtd->size)
            {
                printk("SPI NAND ERROR!! Trying to read past end of chip\n");
                status |= NAND_STATUS_FAIL;
            }
            else
            {
                int temp = spi_nand_read_page(page * mtd->writesize, column, pageBuf, mtd->writesize + mtd->oobsize);

                if (FLASH_API_ERROR == temp)
                {
                    //printk("SPI NAND ERROR Reading page!!\n");
                    status |= NAND_STATUS_FAIL;
                    mtd->ecc_stats.failed++;
                    bit_flips = -1;
                }
                else if (FLASH_API_CORR == temp) // this is taken care of in nand_read_page_hwecc
                    mtd->ecc_stats.corrected++;

                pageBufI = 0;
            }
            break;

        case NAND_CMD_READOOB: // step 1/2 for read, execute SPI NAND read command and transfer SPI NAND data to local buffer

            status = STATUS_DEFAULT;

            if (addr > mtd->size)
            {
                printk("SPI NAND ERROR!! Trying to read past end of chip\n");
                status |= NAND_STATUS_FAIL;
            }
            else
            {
                int temp = spi_nand_read_page(page * mtd->writesize, mtd->writesize, pageBuf + mtd->writesize, mtd->oobsize);

                if (FLASH_API_ERROR == temp)
                {
                    //printk("SPI NAND ERROR Reading page OOB!!\n");
                    status |= NAND_STATUS_FAIL;
                    mtd->ecc_stats.failed++;
                }
                else if (FLASH_API_CORR == temp)
                    mtd->ecc_stats.corrected++;

                pageBufI = mtd->writesize;
            }
            break;

        case NAND_CMD_RESET:
            status = STATUS_DEFAULT;

            if (FLASH_API_ERROR == spi_nand_device_reset())
            {
                printk("ERROR resetting SPI NAND device!!\n");
                status |= NAND_STATUS_FAIL;
            }
            break;

        case NAND_CMD_READID:
            status = STATUS_DEFAULT;

            memset(pageBuf, 0, NAND_ID_LENGTH); // spoof the Linux NAND driver for grabbing the device ID so that function nand_get_flash_type in nand_base.c makes a call to the SPI NAND driver via busw = chip->init_size(mtd, chip, id_data) to initialize the NAND parameters

            pageBufI = 0;
            break;

        case NAND_CMD_STATUS: // NAND infrastructure only uses this to determine if write protect is set
            *(pageBuf + mtd->writesize + mtd->oobsize - 1) = status;
            pageBufI = mtd->writesize + mtd->oobsize - 1; // set pointer to end of buffer so we have a limit to the amount of data read
            break;

        case NAND_CMD_SEQIN: // step 1/3 for write, capture address
            status = STATUS_DEFAULT;

            if (addr > mtd->size)
            {
                printk("ERROR!! Trying to program past end of chip\n");
                status |= NAND_STATUS_FAIL;
            }
            else
            {
                pageAddr = addr;
                pageOffset = column;
                pageBufI = 0;
            }
            break;

        case NAND_CMD_PAGEPROG: // step 3/3 for write, transfer local buffer to SPI NAND device and execute SPI NAND write command
        {
            int error = 0;

            addr = pageAddr & ~(mtd->erasesize - 1); // block address

            if ((status = spi_nand_write_page(pageAddr, pageOffset, pageBuf, pageBufI)) == FLASH_API_ERROR)
                error = 1;

            if (!error && (status == FLASH_API_CORR) && (pageAddr >= mtd->erasesize))
            { // read/erase/write block to see if we can get rid of the bit errors, but only if not block zero
                int offset;
                unsigned char * buffer;

                printk("Correctible errors, SPI NAND Rewriting block\n");

                buffer = kmalloc(mtd->erasesize, GFP_ATOMIC);
                if (!buffer)
                { // unfortunately can't attempt to fix block in this case
                    printk("Error allocating buffer!!\n");
                    error = 1;
                }

                // read block
                for (offset = 0; !error && (offset < mtd->erasesize); offset += mtd->writesize)
                {
                    status = spi_nand_read_page(addr + offset, 0, buffer + offset, mtd->writesize);
                    if (status == FLASH_API_ERROR)
                        error = 1;
                }

                // erase block
                if (!error)
                {
                    status = spi_nand_sector_erase_int(addr);
                    if (status == FLASH_API_ERROR)
                        error = 1;
                }

                // write block
                if (!error)
                {
                    for (offset = 0; offset < mtd->erasesize; offset += mtd->writesize)
                    {
                        status = spi_nand_write_page(addr + offset, 0, buffer + offset, mtd->writesize);
                        if (status != FLASH_API_OK)
                            error = 1; // essentially failed, but finish writing out all the data anyway to hopefully be recovered later
                    }
                }

                if(buffer)
                    kfree(buffer);
            }

            status = STATUS_DEFAULT;

            if (error)
            {
                printk("SPI NAND ERROR Writing page!!\n");
                status |= NAND_STATUS_FAIL;
                spi_nand_mark_blk_bad(addr); // JFFS2 will do this automatically
            }

            break;
        }

        case NAND_CMD_ERASE1:
            status = STATUS_DEFAULT;

            if (addr >= mtd->size)
            {
                printk("ERROR!! Trying to erase past end of chip\n");
                status |= NAND_STATUS_FAIL;
            }
            else if (FLASH_API_ERROR == spi_nand_sector_erase_int(addr))
            {
                printk("SPI NAND ERROR Erasing block!!\n");
                status |= NAND_STATUS_FAIL;
            }
        case NAND_CMD_ERASE2:
            break;

            default:
                printk("ERROR!! Unkonwn NAND command 0x%x\n", command);
                status |= NAND_STATUS_FAIL;
        }

    spin_unlock(&chip_lock);
}

static unsigned char bcm63xx_read_byte(struct mtd_info *mtd)
{
    unsigned char ret;

    spin_lock(&chip_lock);

    ret = *(pageBuf + pageBufI++);

    spin_unlock(&chip_lock);

    return(ret);
}

static void bcm63xx_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{ // step 2/2 for read, read data from local buffer
    spin_lock(&chip_lock);

    if ((pageBufI + len) > (mtd->writesize + mtd->oobsize))
        printk("ERROR!! Trying to read past end of buffer\n");
    else
    {
        memcpy(buf, pageBuf+pageBufI, len);
        pageBufI += len;
    }

    spin_unlock(&chip_lock);
}

// step 2/3 for write, fill local buffer
static void bcm63xx_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{ // write to buffer
    spin_lock(&chip_lock);

    if ((pageBufI + len) > (mtd->writesize + mtd->oobsize))
        printk("ERROR!! Trying to write past end of buffer\n");
    else
    {
        memcpy(pageBuf+pageBufI, buf, len);
        pageBufI += len;
    }

    spin_unlock(&chip_lock);
}

static int bcm63xx_status(struct mtd_info *mtd, struct nand_chip *chip)
{ // NAND infrastructure used this to not only determine when a command has finished (spinlocks will take care of that)
    // but also to return the status

    spin_lock(&chip_lock);

    spin_unlock(&chip_lock);

    return(status);
}

static int bcm63xx_init_size(struct nand_chip *chip)
{ // overwrite possibly incorrectly detected values from Linux NAND driver
//    struct mtd_info *mtd = nand_to_mtd(chip);
    struct mtd_info *mtd = &chip->mtd;
    static int splash = 0;
    int i;

    mtd = nand_to_mtd(chip);
//    mtd = &chip->mtd;

    if (!splash)
    {
        printk("SPI NAND device %s\n", pchip->chip_name);
        printk("   device id    = 0x");
        for (i = 0; i < SPI_NAND_ID_LENGTH; i++)
            printk("%x", pchip->chip_device_id[i]);
        printk("\n");
        printk("   ecc          = %x/%x bits\n", pchip->chip_ecc_corr, pchip->chip_ecc);
        printk("   page size    = 0x%x (%d) bytes\n", pchip->chip_page_size, pchip->chip_page_size);
        printk("   block size   = 0x%x (%d) bytes\n", pchip->chip_block_size, pchip->chip_block_size);
        printk("   total blocks = 0x%x (%d)\n", pchip->chip_num_blocks, pchip->chip_num_blocks);
        printk("   total size   = 0x%lx (%ld) bytes\n", pchip->chip_total_size, pchip->chip_total_size);

        splash = 1;
    }

    mtd->writesize = pchip->chip_page_size;
    mtd->oobsize = pchip->chip_spare_size;
    mtd->erasesize = pchip->chip_block_size;
    chip->chipsize = mtd->size = pchip->chip_total_size;
    /* 
     * switch to the same mtd id as the mainline spi nand driver 
     * in case we need to switch between them and bootloader does 
     * not need to change the mtd_id in the mtdparts cmdline 
     */     
    snprintf(mtd_id, 32, "spi%d.0", spi_flash_busnum);
    mtd->name = mtd_id;
  
    /* no MLC for SPINAND yet */
    chip->bits_per_cell = 1;
    chip->numchips = 1;

    return(0);
}


static int bcm63xx_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
    int ret;

    spin_lock(&chip_lock);

    ret = spi_nand_is_blk_bad(ofs);

    spin_unlock(&chip_lock);

    return(ret);
}

static int bcm63xx_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
    int ret;

    spin_lock(&chip_lock);

    ret = spi_nand_mark_blk_bad(ofs);

    spin_unlock(&chip_lock);

    if (ret == FLASH_API_OK)
        return(0);

    return(-EBADMSG);

}

static int bcm63xx_ecc_calculate(struct mtd_info *mtd, const unsigned char *buf, unsigned char *code)
{ // dummy function, ecc calculation was already completed during the read
    return 0;
}

static int bcm63xx_bit_flips(struct mtd_info *mtd, uint8_t *p, uint8_t *ecc_code, uint8_t *ecc_calc)
{
    return bit_flips;
}


static int bcm63xx_ecc_region(struct mtd_info *mtd, int section, struct mtd_oob_region *oobecc)
{
    if (!section)
    { // report only one byte of ecc
        oobecc->offset = 0;
        oobecc->length = 1;

        return(0);
    }

    return(0);
}

static int bcm63xx_ecc_free(struct mtd_info *mtd, int section, struct mtd_oob_region *oobfree)
{
    if (!section)
    {
        oobfree->offset = pchip->ecclayout->oobfree[section].offset;
        oobfree->length = pchip->ecclayout->oobfree[section].length;

        return(0);
    }

    return(-ERANGE);
};

static void bcm63xx_select(struct mtd_info *mtd, int chip)
{ // dummy function, chip is always selected as far as the NAND infrastructure is concerned
}

static int bcm63xx_erase(struct mtd_info *mtd, int page)
{
    int ret;

    spin_lock(&chip_lock);

    ret = (spi_nand_sector_erase_int(page << pchip->chip_page_shift) == FLASH_API_OK ? 0 : -1);

    status = STATUS_DEFAULT;
    if (ret < 0)    
        status |= NAND_STATUS_FAIL;

    spin_unlock(&chip_lock);

    return (ret);
}

int bcm63xx_read(struct mtd_info *mtd, loff_t addr, size_t len, size_t *retlen, unsigned char *buffer)
{ // can only read one block at a time, otherwise if one block is good and another is bad then whole read is invalid
    //PCFE_SPI_NAND_CHIP pchip = &g_spinand_chip;
    int ret;
    //unsigned int addr;
    unsigned int page_addr;
    unsigned int page_offset;
    unsigned int page_boundary;
    unsigned int size;

    spin_lock(&chip_lock);

   *retlen = 0;
    ret = len;

    //addr = (blk * pchip->chip_block_size) + offset;
    page_addr = addr & ~(pchip->chip_page_size - 1);
    page_offset = addr - page_addr;
    page_boundary = page_addr + pchip->chip_page_size;

    size = page_boundary - addr;

    if(size > len)
        size = len;

    DBG_PRINTF(">> spi_nand_read_buf - 1, offset=%d, len=%u, size=%d\n", offset, len, size);

    if ((addr + len) > pchip->chip_total_size)
    {
        printk("bcm63xx_read(): Attempt to read beyond end of NAND\n");

        ret = -ENOTSUPP;
        goto EXIT;
    }

    if ( ((addr & (pchip->chip_block_size-1)) + len) > pchip->chip_block_size)
    { // cannot read past block boundary, otherwise if one block is good and another is bad then whole read is invalid
        printk("bcm63xx_read(): Attempt to read past block boundary, address 0x%llx, length 0x%x\n", addr, len);

        ret = -ENOTSUPP;
        goto EXIT;
    }

   *retlen = len;
    while (len)
    {
        if (spi_nand_read_page(page_addr, page_offset, buffer, size) == FLASH_API_ERROR)
        {
            ret = -EBADMSG;
            goto EXIT;
        }

        len -= size;
        if (len)
        {
            page_addr += pchip->chip_page_size;
            page_offset = 0;
            buffer += size;
            if(len > pchip->chip_page_size)
                size = pchip->chip_page_size;
            else
                size = len;
        }
    }

    ret = bit_flips;

    if (spi_nand_is_blk_bad(addr))
    { /* don't check for bad block during page read/write since may be reading/writing to bad block marker,
         check for bad block after read to allow for data recovery */
        printk("spi_nand_read_buf(): Attempt to read bad nand block at address 0x%llx, block 0x%llx\n", addr, addr >> pchip->chip_block_shift);

        ret = -EBADMSG;
    }

EXIT:
    DBG_PRINTF(">> spi_nand_read_buf - ret=%d\n", ret);

    status = STATUS_DEFAULT;
    if (ret < 0)    
        status |= NAND_STATUS_FAIL;

    spin_unlock(&chip_lock);

    return(ret);
}


static int bcm63xx_write(struct mtd_info *mtd, loff_t addr, size_t len, size_t *retlen, const unsigned char *buffer)
{ // can only write one block at a time, otherwise if one block is good and another is bad then whole write is invalid

    int ret, stat, error;
    //unsigned long addr;
    unsigned long page_addr;
    unsigned int page_offset;
    unsigned long page_boundary;
    //unsigned long block_addr;
    unsigned int size, ofs;
    //unsigned int numBlocksInChip = pchip->chip_num_blocks;

    spin_lock(&chip_lock);

    ret = 0;
   *retlen = 0;

    DBG_PRINTF(">> bcm63xx_write - 1 addr=0x%8.8lx, offset=%d, len=%d\n", addr, offset, len);

    //block_addr = blk * pchip->chip_block_size;
    //addr = block_addr + offset;
    page_addr = addr & ~(pchip->chip_page_size - 1); // snap address to page
    page_offset = addr - page_addr;
    page_boundary = page_addr + pchip->chip_page_size;

    size = page_boundary - addr;

    if(size > len)
        size = len;

    if (spi_nand_is_blk_bad(addr))
    {
        printk("bcm63xx_write(): Attempt to write bad nand block at address 0x%llx\n", addr);

        ret = -EBADMSG;
        goto QUIT;
    }

    if ((addr + len) > pchip->chip_total_size)
    {
        printk("bcm63xx_write(): Attempt to read beyond end of NAND\n");

        ret = -ENOTSUPP;
        goto QUIT;
    }

    if ( ((addr & (pchip->chip_block_size-1)) + len) > pchip->chip_block_size)
    { // cannot write past block boundary, otherwise if one block is good and another is bad then whole write is invalid
        printk("bcm63xx_write(): Attempt to write past block boundary, address 0x%llx, length 0x%lx\n", addr, len);

        ret = -ENOTSUPP;
        goto QUIT;
    }

   *retlen = len;

    if (len)
    {
        unsigned char *buff = kmalloc(pchip->chip_block_size, GFP_ATOMIC);
        //unsigned char *xfer_buf = KMALLOC(pchip->chip_page_size + pchip->chip_spare_size, 0);
        //unsigned char *spi_buf = KMALLOC(SPI_BUF_LEN, 0);
        //unsigned char *buf = KMALLOC(pchip->chip_page_size, 0);
        //unsigned char *buf_ecc = KMALLOC(pchip->chip_page_size + pchip->chip_spare_size, 0);
        //unsigned char *buf_noecc = KMALLOC(pchip->chip_page_size + pchip->chip_spare_size, 0);

        if (!buff /*|| !xfer_buf || !spi_buf || !buf || !buf_ecc || !buf_noecc*/)
        {
            ret = -ENOMEM;
           *retlen = 0;
            goto QUIT;
        }

        ofs = 0;
        error = 0;

        do
        {
            if( (stat = spi_nand_write_page(page_addr, page_offset, buffer + ofs, size) == FLASH_API_ERROR ) )
                error = 1;

            if (!error && (stat == FLASH_API_CORR) && (page_addr >= pchip->chip_block_size))
            { // read/erase/write block to see if we can get rid of the bit errors, but only if not block zero
                int offset;

                // read block
                for (offset = 0; !error && (offset < pchip->chip_block_size); offset += pchip->chip_page_size)
                {
                    stat = spi_nand_read_page(page_addr, offset, buff + offset, pchip->chip_page_size);
                    if (stat == FLASH_API_ERROR)
                        error = 1;
                }

                // erase block
                if (!error)
                {
                    spi_nand_sector_erase_int(page_addr);
                    if (stat == FLASH_API_ERROR)
                        error = 1;
                }

                // write block
                if (!error)
                {
                    for (offset = 0; offset < pchip->chip_block_size; offset += pchip->chip_page_size)
                    {
                        stat = spi_nand_write_page(page_addr, offset, buff + offset, pchip->chip_page_size);
                        if (stat != FLASH_API_OK)
                            error = 1; // essentially failed, but finish writing out all the data anyway to hopefully be recovered later
                    }
                }
            }

            if (error)
            { // mark block bad
                printk("SPI NAND ERROR Writing page!!\n");
                spi_nand_mark_blk_bad(page_addr);

                ret = -EBADMSG;
               *retlen = 0;
                goto EXIT;
            }

            len -= size;
            if( len )
            {
                DBG_PRINTF(">> nand_flash_write_buf- 2 addr=0x%8.8lx, len=%d\n", addr, len);

                page_addr += pchip->chip_page_size;
                page_offset = 0;
                ofs += size;
                if(len > pchip->chip_page_size)
                    size = pchip->chip_page_size;
                else
                    size = len;
            }
        } while(len);
EXIT:
        if (buff)
            kfree(buff);
        //if (xfer_buf)
            //KFREE(xfer_buf);
        //if (spi_buf)
            //KFREE(spi_buf);
        //if (buf)
            //KFREE(buf);
        //if (buf_ecc)
            //KFREE(buf_ecc);
        //if (buf_noecc)
            //KFREE(buf_noecc);
    }

QUIT:
    DBG_PRINTF(">> nand_flash_write_buf - ret=%d\n", ret);

    status = STATUS_DEFAULT;
    if (ret < 0)    
        status |= NAND_STATUS_FAIL;

    spin_unlock(&chip_lock);

    return( ret ) ;
}




static struct spi_board_info bcmSpiDevInfo =
{
    .modalias      = "bcm_SpiDev",
    .chip_select   = 0,
    .max_speed_hz  = 781000,
    .bus_num       = LEG_SPI_BUS_NUM,
    .mode          = SPI_MODE_3,
};

static struct spi_driver bcmSpiDevDrv =
{
    .driver =
        {
        .name     = "bcm_SpiDev",
        .bus      = &spi_bus_type,
        .owner    = THIS_MODULE,
        },
};


void bcmspinand_probe(struct mtd_info * mtd)
{
    struct nand_chip * nand = mtd_to_nand(mtd);
    struct nand_controller_ops * cops = nand->controller->ops;
//    struct nand_manufacturer_ops * mops = nand->manufacturer->desc->;
    struct spi_master * pSpiMaster;
    unsigned int spiCtrlState;
    static const struct mtd_ooblayout_ops ooblayout = {.ecc = bcm63xx_ecc_region, .free = bcm63xx_ecc_free};
    //spinlock_t *lock = &nand->controller->lock;

    printk("SPI NAND Device Linux Registration\n");

    /* micron MT29F1G01 only support up to 50MHz, update to 50Mhz if it is more than that */
    spi_flash_busnum = HS_SPI_BUS_NUM;
    spi_flash_clock = 50000000;

    /* retrieve the maximum read/write transaction length from the SPI controller */
    spi_max_op_len = SPI_BUF_LEN;

    /* set the controller state, spi_mode_0 */
    spiCtrlState = (unsigned int)SPI_CONTROLLER_STATE_DEFAULT;

    if ( spi_flash_clock > SPI_CONTROLLER_MAX_SYNC_CLOCK )
       spiCtrlState |= SPI_CONTROLLER_STATE_ASYNC_CLOCK;

    bcmSpiDevInfo.max_speed_hz    = spi_flash_clock;
    bcmSpiDevInfo.controller_data = (void *)(uintptr_t)spiCtrlState;
    bcmSpiDevInfo.mode            = SPI_MODE_DEFAULT;
    bcmSpiDevInfo.chip_select     = SPI_FLASH_SLAVE_DEV_ID;
    bcmSpiDevInfo.bus_num         = spi_flash_busnum;

    pSpiMaster = spi_busnum_to_master( spi_flash_busnum );
    pSpiDevice = spi_new_device(pSpiMaster, &bcmSpiDevInfo);

    /* register as SPI device */
    spi_register_driver(&bcmSpiDevDrv);

    SpiNandDeviceRegistered = 1;

    printk("SPI NAND Linux Registration\n");

    spin_lock_init(&chip_lock);
//    spin_lock_init(lock);

    spi_nand_device_reset(); // reset and set configuration information

    spi_flash_clock = pchip->chip_clock_speed; /* switch to the max supported clock speed */

    nand->ecc.size = pchip->chip_page_size;
    //nand->ecc.bytes = pchip->ecclayout->eccbytes + pchip->ecclayout->oobavail;
    nand->ecc.bytes = 1; // we don't want software to iterate through ECC bytes, we've already done the calculation 
    nand->ecc.total = 1; // we don't want software to iterate through ECC bytes, we've already done the calculation 
    nand->ecc.steps = 1; // must set to something otherwise nand_read_page_hwecc won't report ecc flipped bits
    nand->ecc.strength = pchip->chip_ecc;
    mtd->ecc_strength = pchip->chip_ecc;
    mtd->_read = bcm63xx_read;
    mtd->_write = bcm63xx_write;

    mtd->bitflip_threshold = pchip->chip_ecc_corr;
    //mtd->ooblayout->ecc = bcm63xx_ecc_region;
    //mtd->ooblayout->free = bcm63xx_ecc_free;
    mtd->ooblayout = &ooblayout;

    nand->pagemask = -1;
    nand->erase = bcm63xx_erase; 
    nand->page_shift = pchip->chip_page_shift;
    nand->phys_erase_shift = pchip->chip_block_shift;
    nand->chipsize = pchip->chip_total_size;

    pageBuf = kmalloc(pchip->chip_page_size + pchip->chip_spare_size, GFP_KERNEL);

    nand->options |= NAND_NO_SUBPAGE_WRITE | NAND_SKIP_BBTSCAN; // bus width must match part in nand_ids.c

    nand->chip_delay = 0;
    nand->read_byte = bcm63xx_read_byte;
    nand->read_buf = bcm63xx_read_buf;
    nand->ecc.mode = NAND_ECC_HW;
    nand->ecc.correct = bcm63xx_bit_flips;
    nand->ecc.calculate = bcm63xx_ecc_calculate;
    nand->ecc.hwctl = bcm63xx_select;

    nand->select_chip = bcm63xx_select;
    nand->write_buf  = bcm63xx_write_buf;
    nand->block_bad = bcm63xx_block_isbad;
    nand->block_markbad = bcm63xx_block_markbad;
    nand->cmdfunc = bcm63xx_cmd;
    nand->waitfunc = bcm63xx_status;
    cops->attach_chip = bcm63xx_init_size; // return value of this function needs to be 0, may want to stub this off and put init function under manuf->desc->ops->init
}

#endif //CONFIG_BCM_KF_MTD_BCMNAND
