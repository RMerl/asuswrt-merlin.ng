#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 <:copyright-BRCM:2012:GPL/GPL:standard 
 
    Copyright (c) 2012 Broadcom 
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

 * drivers/mtd/brcmnand/brcmnand.h
 *
 *  
 *
 * Data structures for Broadcom NAND controller
 * 
 * when     who     what
 * 20060729 tht     Original coding
 */


#ifndef _BRCM_NAND_H_
#define _BRCM_NAND_H_

#include <linux/version.h>
#include <generated/autoconf.h>
#include <linux/mtd/nand.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#if 0
/*
 * Conversion between Kernel Kconfig and Controller version number
 * Legacy codes 2.6.18
 */

#define CONFIG_MTD_BRCMNAND_VERS_0_0        0
#define CONFIG_MTD_BRCMNAND_VERS_0_1        1
#define CONFIG_MTD_BRCMNAND_VERS_1_0        2

/* The followings revs are not implemented for 2.6.12 */
#define CONFIG_MTD_BRCMNAND_VERS_2_0        3
#define CONFIG_MTD_BRCMNAND_VERS_2_1        4
#define CONFIG_MTD_BRCMNAND_VERS_2_2        5

/* Supporting MLC NAND */
#define CONFIG_MTD_BRCMNAND_VERS_3_0        6
#define CONFIG_MTD_BRCMNAND_VERS_3_1_0      7   /* RDB reads as 3.0 */
#define CONFIG_MTD_BRCMNAND_VERS_3_1_1      8   /* RDB reads as 3.0 */
#define CONFIG_MTD_BRCMNAND_VERS_3_2        9   
#define CONFIG_MTD_BRCMNAND_VERS_3_3        10  
#define CONFIG_MTD_BRCMNAND_VERS_3_4		11
#endif

/*
 * New way of using verison numbers
 */
#define BRCMNAND_VERSION(major, minor,int_minor)	((major<<16) | (minor<<8) | int_minor)

/*
 * BRCMNAND_INT_MINOR: Internal version number, not reflected on the silicon
 */
#if defined( CONFIG_BCM7601 ) || defined( CONFIG_BCM7400A0 )
#define BRCMNAND_INT_MINOR	1
#else
#define BRCMNAND_INT_MINOR	0
#endif
#define CONFIG_MTD_BRCMNAND_VERSION	\
	BRCMNAND_VERSION(CONFIG_BRCMNAND_MAJOR_VERS, CONFIG_BRCMNAND_MINOR_VERS, BRCMNAND_INT_MINOR)


#define CONFIG_MTD_BRCMNAND_VERS_0_0		BRCMNAND_VERSION(0,0,1) /* (0,0,0) is DONT-CARE */
#define CONFIG_MTD_BRCMNAND_VERS_0_1		BRCMNAND_VERSION(0,1,0)
#define CONFIG_MTD_BRCMNAND_VERS_1_0		BRCMNAND_VERSION(1,0,0)

/* The followings revs are not implemented for 2.6.12 */
#define CONFIG_MTD_BRCMNAND_VERS_2_0		BRCMNAND_VERSION(2,0,0)
#define CONFIG_MTD_BRCMNAND_VERS_2_1		BRCMNAND_VERSION(2,1,0)
#define CONFIG_MTD_BRCMNAND_VERS_2_2		BRCMNAND_VERSION(2,2,0)

/* Supporting MLC NAND */
#define CONFIG_MTD_BRCMNAND_VERS_3_0		BRCMNAND_VERSION(3,0,0)
#define CONFIG_MTD_BRCMNAND_VERS_3_1_0		BRCMNAND_VERSION(3,1,0)	/* RDB reads as 3.0 */
#define CONFIG_MTD_BRCMNAND_VERS_3_1_1		BRCMNAND_VERSION(3,1,1)	/* RDB reads as 3.0 */
#define CONFIG_MTD_BRCMNAND_VERS_3_2		BRCMNAND_VERSION(3,2,0)	
#define CONFIG_MTD_BRCMNAND_VERS_3_3		BRCMNAND_VERSION(3,3,0)	
#define CONFIG_MTD_BRCMNAND_VERS_3_4		BRCMNAND_VERSION(3,4,0)

/* Supporting ONFI */
#define CONFIG_MTD_BRCMNAND_VERS_4_0		BRCMNAND_VERSION(4,0,0)

/* Supporting 1KB ECC subpage */
#define CONFIG_MTD_BRCMNAND_VERS_5_0		BRCMNAND_VERSION(5,0,0)

/* Add 40-bit ECC support. Remove ECC_LEVEL_0 and SPARE_AREA_SIZE_0 fields. 
  Expand ECC_LEVEL and SPARE_AREA_SIZE field */
#define CONFIG_MTD_BRCMNAND_VERS_6_0		BRCMNAND_VERSION(6,0,0)

/* Remove FAST_PGM_RDIN bit. Always sets true internally when PARTIAL_PAGE_EN=1.*/
#define CONFIG_MTD_BRCMNAND_VERS_7_0		BRCMNAND_VERSION(7,0,0)
#define CONFIG_MTD_BRCMNAND_VERS_7_1		BRCMNAND_VERSION(7,1,0)

#ifdef CONFIG_MTD_BRCMNAND_EDU
#define CONFIG_MTD_BRCMNAND_USE_ISR		1
#endif

#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_1_0
#define MAX_NAND_CS 8 // Upper limit, actual limit varies depending on platfrom

#else
#define MAX_NAND_CS 1
#endif


//ST NAND flashes
#ifndef FLASHTYPE_ST
    #define FLASHTYPE_ST            0x20
#endif
#define ST_NAND128W3A           0x73
#define ST_NAND256R3A           0x35
#define ST_NAND256W3A           0x75
#define ST_NAND256R4A           0x45
#define ST_NAND256W4A           0x55
#define ST_NAND512R3A           0x36    //Used on Bcm97400
#define ST_NAND512W3A           0x76
#define ST_NAND512R4A           0x46
#define ST_NAND512W4A           0x56
#define ST_NAND01GR3A           0x39
#define ST_NAND01GW3A           0x79
#define ST_NAND01GR4A           0x49
#define ST_NAND01GW4A           0x59
#define ST_NAND01GR3B           0xA1
#define ST_NAND01GW3B           0xF1
#define ST_NAND01GR4B           0xB1
#define ST_NAND01GW4B           0xC1
#define ST_NAND02GR3B           0xAA
#define ST_NAND02GW3B           0xDA
#define ST_NAND02GR4B           0xBA
#define ST_NAND02GW4B           0xCA
#define ST_NAND04GR3B           0xAC
#define ST_NAND04GW3B           0xDC
#define ST_NAND04GR4B           0xBC
#define ST_NAND04GW4B           0xCC
#define ST_NAND08GR3B           0xA3
#define ST_NAND08GW3B           0xD3
#define ST_NAND08GR4B           0xB3
#define ST_NAND08GW4B           0xC3

//Samsung NAND flash
#define FLASHTYPE_SAMSUNG       0xEC
#define SAMSUNG_K9F1G08R0A      0xA1
#define SAMSUNG_K9F1G08U0A      0xF1
#define SAMSUNG_K9F1G08U0E      0xF1
#define SAMSUNG_K9F2G08U1A      0xF1
#define SAMSUNG_K9F2G08U0A      0xDA
#define SAMSUNG_K9K8G08U0A      0xD3
#define SAMSUNG_K9F8G08U0M	0xD3


//K9F5608(R/U/D)0D
#define SAMSUNG_K9F5608R0D      0x35
#define SAMSUNG_K9F5608U0D      0x75
#define SAMSUNG_K9F5608D0D      0x75
//K9F1208(R/B/U)0B
#define SAMSUNG_K9F1208R0B      0x36
#define SAMSUNG_K9F1208B0B      0x76
#define SAMSUNG_K9F1208U0B      0x76

/*--------- Chip ID decoding for Samsung MLC NAND flashes -----------------------*/
#define SAMSUNG_K9LBG08U0M	0xD7	/* 55h, B6h, 78h */
#define SAMSUNG_K9LBG08U0D	0xD7	/* D5h, 29h, 41h */
#define SAMSUNG_K9LBG08U0E	0xD7	/* C5h, 72h, 54h, 42h */

#define SAMSUNG_K9GAG08U0D	0xD5	/* 94h, 29h, 34h */
#define SAMSUNG_K9GAG08U0E	0xD5	/* 84h, 72h, 50h, 42h */

#define SAMSUNG_3RDID_INT_CHIPNO_MASK   NAND_CI_CHIPNR_MSK

#define SAMSUNG_3RDID_CELLTYPE_MASK NAND_CI_CELLTYPE_MSK
#define SAMSUNG_3RDID_CELLTYPE_SLC  0x00
#define SAMSUNG_3RDID_CELLTYPE_4LV  0x04
#define SAMSUNG_3RDID_CELLTYPE_8LV  0x08
#define SAMSUNG_3RDID_CELLTYPE_16LV 0x0C

// Low level MLC test as compared to the high level test in mtd-abi.h
#define NAND_IS_MLC(chip) ((chip)->cellinfo & NAND_CI_CELLTYPE_MSK)

#define SAMSUNG_3RDID_NOP_MASK		0x30
#define SAMSUNG_3RDID_NOP_1         0x00
#define SAMSUNG_3RDID_NOP_2         0x10
#define SAMSUNG_3RDID_NOP_4         0x20
#define SAMSUNG_3RDID_NOP_8         0x30

#define SAMSUNG_3RDID_INTERLEAVE        0x40

#define SAMSUNG_3RDID_CACHE_PROG        0x80

#define SAMSUNG_4THID_PAGESIZE_MASK 0x03
#define SAMSUNG_4THID_PAGESIZE_1KB  0x00
#define SAMSUNG_4THID_PAGESIZE_2KB  0x01
#define SAMSUNG_4THID_PAGESIZE_4KB  0x02
#define SAMSUNG_4THID_PAGESIZE_8KB  0x03

#define SAMSUNG_4THID_OOBSIZE_MASK  0x04
#define SAMSUNG_4THID_OOBSIZE_8B        0x00
#define SAMSUNG_4THID_OOBSIZE_16B   0x04

#define SAMSUNG_4THID_BLKSIZE_MASK	0x30
#define SAMSUNG_4THID_BLKSIZE_64KB	0x00
#define SAMSUNG_4THID_BLKSIZE_128KB	0x10
#define SAMSUNG_4THID_BLKSIZE_256KB	0x20
#define SAMSUNG_4THID_BLKSIZE_512KB	0x30



#define SAMSUNG2_4THID_PAGESIZE_MASK	0x03
#define SAMSUNG2_4THID_PAGESIZE_2KB	0x00
#define SAMSUNG2_4THID_PAGESIZE_4KB	0x01
#define SAMSUNG2_4THID_PAGESIZE_8KB	0x02
#define SAMSUNG2_4THID_PAGESIZE_RSV	0x03

#define SAMSUNG2_4THID_BLKSIZE_MASK	0xB0
#define SAMSUNG2_4THID_BLKSIZE_128KB	0x00
#define SAMSUNG2_4THID_BLKSIZE_256KB	0x10
#define SAMSUNG2_4THID_BLKSIZE_512KB	0x20
#define SAMSUNG2_4THID_BLKSIZE_1MB	0x30
#define SAMSUNG2_4THID_BLKSIZE_RSVD1	0x80
#define SAMSUNG2_4THID_BLKSIZE_RSVD2	0x90
#define SAMSUNG2_4THID_BLKSIZE_RSVD3	0xA0
#define SAMSUNG2_4THID_BLKSIZE_RSVD4	0xB0

#define SAMSUNG2_4THID_OOBSIZE_MASK			0x4c
#define SAMSUNG2_4THID_OOBSIZE_PERPAGE_128	0x04
#define SAMSUNG2_4THID_OOBSIZE_PERPAGE_218	0x08 /* 27.4 per 512B */
#define SAMSUNG2_4THID_OOBSIZE_PERPAGE_400	0x0C /* 16 per 512B */
#define SAMSUNG2_4THID_OOBSIZE_PERPAGE_436	0x40 /* 27.5 per 512B */

#define SAMSUNG_5THID_NRPLANE_MASK  0x0C
#define SAMSUNG_5THID_NRPLANE_1     0x00
#define SAMSUNG_5THID_NRPLANE_2     0x04
#define SAMSUNG_5THID_NRPLANE_4     0x08
#define SAMSUNG_5THID_NRPLANE_8     0x0C

#define SAMSUNG_5THID_PLANESZ_MASK  0x70
#define SAMSUNG_5THID_PLANESZ_64Mb  0x00
#define SAMSUNG_5THID_PLANESZ_128Mb 0x10
#define SAMSUNG_5THID_PLANESZ_256Mb 0x20
#define SAMSUNG_5THID_PLANESZ_512Mb 0x30
#define SAMSUNG_5THID_PLANESZ_1Gb   0x40
#define SAMSUNG_5THID_PLANESZ_2Gb   0x50
#define SAMSUNG_5THID_PLANESZ_4Gb   0x60
#define SAMSUNG_5THID_PLANESZ_8Gb   0x70

#define SAMSUNG2_5THID_ECCLVL_MASK	0x70
#define SAMSUNG2_5THID_ECCLVL_1BIT	0x00
#define SAMSUNG2_5THID_ECCLVL_2BIT	0x10
#define SAMSUNG2_5THID_ECCLVL_4BIT	0x20
#define SAMSUNG2_5THID_ECCLVL_8BIT	0x30
#define SAMSUNG2_5THID_ECCLVL_16BIT	0x40
#define SAMSUNG2_5THID_ECCLVL_24BIT_1KB	0x50




/*--------- END Samsung MLC NAND flashes -----------------------*/

//Hynix NAND flashes
#define FLASHTYPE_HYNIX         0xAD
//Hynix HY27(U/S)S(08/16)561A
#define HYNIX_HY27US08561A      0x75
#define HYNIX_HY27US16561A      0x55
#define HYNIX_HY27SS08561A      0x35
#define HYNIX_HY27SS16561A      0x45
//Hynix HY27(U/S)S(08/16)121A
#define HYNIX_HY27US08121A      0x76
#define HYNIX_HY27US16121A      0x56
#define HYNIX_HY27SS08121A      0x36
#define HYNIX_HY27SS16121A      0x46
//Hynix HY27(U/S)F(08/16)1G2M
#define HYNIX_HY27UF081G2M      0xF1
#define HYNIX_HY27UF161G2M      0xC1
#define HYNIX_HY27SF081G2M      0xA1
#define HYNIX_HY27SF161G2M      0xAD

/* This is the new version of HYNIX_HY27UF081G2M .  The 2M version is EOL */
#define HYNIX_HY27UF081G2A      0xF1

#define HYNIX_HY27UF082G2A      0xDA

// #define HYNIX_HY27UF084G2M     0xDC /* replaced by the next one */
#define HYNIX_HY27U4G8F2D		0xDC

/* Hynix MLC flashes, same infos as Samsung, except the 5th Byte */
#define HYNIX_HY27UT088G2A  0xD3

/* Hynix MLC flashes, same infos as Samsung, except the 5th Byte */
#define HYNIX_HY27UAG8T2M		0xD5	/* 14H, B6H, 44H: 3rd,4th,5th ID bytes */

/* Number of Planes, same as Samsung */

/* Plane Size Type 2 */
#define HYNIX_5THID_PLANESZ_MASK    0x70
#define HYNIX_5THID_PLANESZ_512Mb   0x00
#define HYNIX_5THID_PLANESZ_1Gb 0x10
#define HYNIX_5THID_PLANESZ_2Gb 0x20
#define HYNIX_5THID_PLANESZ_4Gb 0x30
#define HYNIX_5THID_PLANESZ_8Gb 0x40
#define HYNIX_5THID_PLANESZ_RSVD1   0x50
#define HYNIX_5THID_PLANESZ_RSVD2   0x60
#define HYNIX_5THID_PLANESZ_RSVD3   0x70

/* Legacy Hynix on H27U4G8F2D */
/* Plane Size */
#define HYNIX_5THID_LEG_PLANESZ_MASK		0x70
#define HYNIX_5THID_LEG_PLANESZ_64Mb		0x00
#define HYNIX_5THID_LEG_PLANESZ_128Mb	0x10
#define HYNIX_5THID_LEG_PLANESZ_256Mb	0x20
#define HYNIX_5THID_LEG_PLANESZ_512Mb	0x30
#define HYNIX_5THID_LEG_PLANESZ_1Gb		0x40
#define HYNIX_5THID_LEG_PLANESZ_2Gb		0x50
#define HYNIX_5THID_LEG_PLANESZ_4Gb		0x60
#define HYNIX_5THID_LEG_PLANESZ_8Gb		0x70


/*--------- END Hynix MLC NAND flashes -----------------------*/

//Micron flashes
#define FLASHTYPE_MICRON        0x2C
//MT29F2G(08/16)AAB
#define MICRON_MT29F2G08AAB     0xDA
#define MICRON_MT29F2G16AAB     0xCA

#define MICRON_MT29F1G08ABA	0xF1
#define MICRON_MT29F2G08ABA	0xDA
#define MICRON_MT29F4G08ABA	0xDC

#define MICRON_MT29F8G08ABA	0x38
#define MICRON_MT29F16G08ABA	0x48 /* SLC, 2Ch, 48h, 00h, 26h, 89h */

#define MICRON_MT29F16G08CBA	0x48 /* MLC, 2Ch, 48h, 04h, 46h, 85h
										have same dev ID as the SLC part, bytes 3,4,5 are different however */

/*
 * Micron M60A & M68A ID encoding are similar to Samsung Type 1.
 */

#define MICRON_3RDID_INT_CHIPNO_MASK	NAND_CI_CHIPNR_MSK

#define MICRON_3RDID_CELLTYPE_MASK	NAND_CI_CELLTYPE_MSK
#define MICRON_3RDID_CELLTYPE_SLC	0x00
#define MICRON_3RDID_CELLTYPE_4LV	0x04
//#define MICRON_3RDID_CELLTYPE_8LV	0x08
//#define MICRON_3RDID_CELLTYPE_16LV	0x0C


/* Nbr of simultaneously programmed pages */
#define MICRON_3RDID_SIMPG_MASK		0x30
#define MICRON_3RDID_SIMPG_1			0x00
#define MICRON_3RDID_SIMPG_2			0x10
//#define MICRON_3RDID_SIM_4			0x20
//#define MICRON_3RDID_SIM_8			0x30

#define MICRON_3RDID_INTERLEAVE		0x40

#define MICRON_3RDID_CACHE_PROG		0x80

#define MICRON_4THID_PAGESIZE_MASK	0x03
#define MICRON_4THID_PAGESIZE_1KB		0x00
#define MICRON_4THID_PAGESIZE_2KB		0x01
#define MICRON_4THID_PAGESIZE_4KB		0x02
#define MICRON_4THID_PAGESIZE_8KB		0x03

#define MICRON_4THID_OOBSIZE_MASK	0x04
#define MICRON_4THID_OOBSIZE_8B		0x00
#define MICRON_4THID_OOBSIZE_16B		0x04

#define MICRON_4THID_BLKSIZE_MASK		0x30
#define MICRON_4THID_BLKSIZE_64KB		0x00
#define MICRON_4THID_BLKSIZE_128KB	0x10
#define MICRON_4THID_BLKSIZE_256KB	0x20
#define MICRON_4THID_BLKSIZE_512KB	0x30

/* Required ECC level */
#define MICRON_5THID_ECCLVL_MASK		0x03
#define MICRON_5THID_ECCLVL_4BITS		0x02

#define MICRON_5THID_NRPLANE_MASK	0x0C
#define MICRON_5THID_NRPLANE_1		0x00
#define MICRON_5THID_NRPLANE_2		0x04
#define MICRON_5THID_NRPLANE_4		0x08
//#define SAMSUNG_5THID_NRPLANE_8		0x0C

#define MICRON_5THID_PLANESZ_MASK	0x70
#define MICRON_5THID_PLANESZ_64Mb	0x00
#define MICRON_5THID_PLANESZ_128Mb	0x10
#define MICRON_5THID_PLANESZ_256Mb	0x20
#define MICRON_5THID_PLANESZ_512Mb	0x30
#define MICRON_5THID_PLANESZ_1Gb		0x40
#define MICRON_5THID_PLANESZ_2Gb		0x50
#define MICRON_5THID_PLANESZ_4Gb		0x60
#define MICRON_5THID_PLANESZ_8Gb		0x70

#define MICRON_5THID_INT_ECC_MASK	0x80
#define MICRON_5THID_INT_ECC_ENA		0x80


/*
 * Micron M61A ID encoding will be phased out in favor of ONFI
 */
 #define MICRON_M61A_2NDID_VOLTAGE_MASK		0x0F
 #define MICRON_M61A_2NDID_3_3V				0x08

/* Not strictly followed, must rely on 5th ID byte for density */
#define MICRON_M61A_2NDID_DENSITY_MASK		0xF0
#define MICRON_M61A_2NDID_2Gb					0x10
#define MICRON_M61A_2NDID_4Gb					0x20 
#define MICRON_M61A_2NDID_8Gb					0x30 
#define MICRON_M61A_2NDID_16Gb				0x40 

/* M61A_3RDID_SLC is same as standard Samsung Type 1 */
/* M61A_4THID_PAGESIZE same as standard Samsung Type 1 */

#define MICRON_M61A_4THID_OOBSIZE_MASK		0x0C
#define MICRON_M61A_4THID_OOBSIZE_28B		0x04	/* 224 per 4KB page */

/* Pages per block ==> Block Size */
#define MICRON_M61A_4THID_PGPBLK_MASK		0x70
#define MICRON_M61A_4THID_128PG_PERBLK		0x20	/* 128 pages per block =512KB blkSize*/

#define MICRON_M61A_4THID_MULTI_LUN_MASK	0x80
#define MICRON_M61A_4THID_MLUN_SUPPORTED	0x80	/* 128 pages per block */


#define MICRON_M61A_5THID_PLN_PER_LUN_MASK	0x03
#define MICRON_M61A_5THID_2PLN				0x01	/* 2 planes per LUN */

#define MICRON_M61A_5THID_BLK_PER_LUN_MASK	0x1C
#define MICRON_M61A_5THID_2048BLKS			0x04	/* 2048 blks per LUN */

//Spansion flashes
#ifndef FLASHTYPE_SPANSION
    #define FLASHTYPE_SPANSION      0x01
#endif
/* Large Page */
#define SPANSION_S30ML01GP_08   0xF1    //x8 mode
#define SPANSION_S30ML01GP_16   0xC1    //x16 mode
#define SPANSION_S30ML02GP_08   0xDA    //x8 mode
#define SPANSION_S30ML02GP_16   0xCA    //x16 mode
#define SPANSION_S30ML04GP_08   0xDC    //x8 mode
#define SPANSION_S30ML04GP_16   0xCC    //x16 mode

/* Small Page */
#define SPANSION_S30ML512P_08   0x76    //64MB x8 mode
#define SPANSION_S30ML512P_16   0x56    //64MB x16 mode
#define SPANSION_S30ML256P_08   0x75    //32MB x8 mode
#define SPANSION_S30ML256P_16   0x55    //32MB x16 mode
#define SPANSION_S30ML128P_08   0x73    //x8 mode
#define SPANSION_S30ML128P_16   0x53    //x16 mode


/* -------- Toshiba NAND E2PROM -----------------*/
#define FLASHTYPE_TOSHIBA		0x98

#define TOSHIBA_TC58NVG0S3ETA00	0xD1
#define TOSHIBA_TC58NVG1S3ETAI5	0xDA
#define TOSHIBA_TC58NVG3S0ETA00	0xD3

/*---------------------------------------------------------------------------------------*/

// Low level MLC test as compared to the high level test in mtd-abi.h
#define NAND_IS_MLC(chip) ((chip)->cellinfo & NAND_CI_CELLTYPE_MSK)


//Command Opcode
#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_7_0
#define OP_PAGE_READ                0x01
#define OP_SPARE_AREA_READ          0x02
#define OP_STATUS_READ              0x03
#define OP_PROGRAM_PAGE             0x04
#define OP_PROGRAM_SPARE_AREA       0x05
#define OP_COPY_BACK                0x06
#define OP_DEVICE_ID_READ           0x07
#define OP_BLOCK_ERASE              0x08
#define OP_FLASH_RESET              0x09
#define OP_BLOCKS_LOCK              0x0A
#define OP_BLOCKS_LOCK_DOWN         0x0B
#define OP_BLOCKS_UNLOCK            0x0C
#define OP_READ_BLOCKS_LOCK_STATUS  0x0D
#define OP_PARAMETER_READ           0x0E
#define OP_PARAMETER_CHANGE_COL     0x0F
#define OP_LOW_LEVEL_OP             0x10
#else
#define OP_PAGE_READ                0x01000000
#define OP_SPARE_AREA_READ          0x02000000
#define OP_STATUS_READ              0x03000000
#define OP_PROGRAM_PAGE             0x04000000
#define OP_PROGRAM_SPARE_AREA       0x05000000
#define OP_COPY_BACK                0x06000000
#define OP_DEVICE_ID_READ           0x07000000
#define OP_BLOCK_ERASE              0x08000000
#define OP_FLASH_RESET              0x09000000
#define OP_BLOCKS_LOCK              0x0A000000
#define OP_BLOCKS_LOCK_DOWN         0x0B000000
#define OP_BLOCKS_UNLOCK            0x0C000000
#define OP_READ_BLOCKS_LOCK_STATUS  0x0D000000
#define OP_PARAMETER_READ           0x0E000000
#define OP_PARAMETER_CHANGE_COL     0x0F000000
#define OP_LOW_LEVEL_OP             0x10000000
#endif

//NAND flash controller 
#define NFC_FLASHCACHE_SIZE     512

#if CONFIG_MTD_BRCMNAND_VERSION <=  CONFIG_MTD_BRCMNAND_VERS_3_2
#define BCHP_NAND_LAST_REG		BCHP_NAND_BLK_WR_PROTECT

#elif CONFIG_MTD_BRCMNAND_VERSION <=  CONFIG_MTD_BRCMNAND_VERS_3_3
  #ifdef BCHP_NAND_TIMING_2_CS3
#define BCHP_NAND_LAST_REG		BCHP_NAND_TIMING_2_CS3
  #else
#define BCHP_NAND_LAST_REG		BCHP_NAND_TIMING_2_CS2
  #endif
#elif CONFIG_MTD_BRCMNAND_VERSION <=  CONFIG_MTD_BRCMNAND_VERS_5_0
#define BCHP_NAND_LAST_REG		BCHP_NAND_SPARE_AREA_READ_OFS_1C 
#else
#define BCHP_NAND_LAST_REG		BCHP_NAND_SPARE_AREA_WRITE_OFS_1C 
#endif

#define BRCMNAND_CTRL_REGS		(BCHP_NAND_REVISION)
#define BRCMNAND_CTRL_REGS_END		(BCHP_NAND_LAST_REG)


/**
 * brcmnand_state_t - chip states
 * Enumeration for BrcmNAND flash chip state
 */
typedef enum {
    BRCMNAND_FL_READY = FL_READY,
    BRCMNAND_FL_READING = FL_READING,
    BRCMNAND_FL_WRITING = FL_WRITING,
    BRCMNAND_FL_ERASING = FL_ERASING,
    BRCMNAND_FL_SYNCING = FL_SYNCING,
    BRCMNAND_FL_CACHEDPRG = FL_CACHEDPRG,
    BRCMNAND_FL_UNLOCKING = FL_UNLOCKING,
    BRCMNAND_FL_LOCKING = FL_LOCKING,
    BRCMNAND_FL_RESETING = FL_RESETING,
    BRCMNAND_FL_OTPING = FL_OTPING,
    BRCMNAND_FL_PM_SUSPENDED = FL_PM_SUSPENDED,
    BRCMNAND_FL_EXCLUSIVE = FL_UNKNOWN+10,  // Exclusive access to NOR flash, prevent all NAND accesses.
    BRCMNAND_FL_XIP,            // Exclusive access to XIP part of the flash
} brcmnand_state_t;

//#if CONFIG_MTD_BRCMNAND_VERSION >= CONFIG_MTD_BRCMNAND_VERS_3_0
/*
 * ECC levels, corresponding to BCHP_NAND_ACC_CONTROL_ECC_LEVEL
 */
typedef enum {
    BRCMNAND_ECC_DISABLE    = 0u,
    BRCMNAND_ECC_BCH_1      = 1u,
    BRCMNAND_ECC_BCH_2      = 2u,
    BRCMNAND_ECC_BCH_3      = 3u,
    BRCMNAND_ECC_BCH_4      = 4u,
    BRCMNAND_ECC_BCH_5      = 5u,
    BRCMNAND_ECC_BCH_6      = 6u,
    BRCMNAND_ECC_BCH_7      = 7u,
    BRCMNAND_ECC_BCH_8      = 8u,
    BRCMNAND_ECC_BCH_9      = 9u,
    BRCMNAND_ECC_BCH_10     = 10u,
    BRCMNAND_ECC_BCH_11     = 11u,
    BRCMNAND_ECC_BCH_12     = 12u,
    BRCMNAND_ECC_RESVD_1    = 13u,
    BRCMNAND_ECC_RESVD_2    = 14u,
    BRCMNAND_ECC_HAMMING    = 15u,
} brcmnand_ecc_level_t;

/*
 * Number of required ECC bytes per 512B slice
 */
#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
static const unsigned int brcmnand_eccbytes[16] = {
    [BRCMNAND_ECC_DISABLE]  = 0,
    [BRCMNAND_ECC_BCH_1]    = 2,
    [BRCMNAND_ECC_BCH_2]    = 4,
    [BRCMNAND_ECC_BCH_3]    = 5,
    [BRCMNAND_ECC_BCH_4]    = 7,
    [BRCMNAND_ECC_BCH_5]    = 9,
    [BRCMNAND_ECC_BCH_6]    = 10,
    [BRCMNAND_ECC_BCH_7]    = 12,
    [BRCMNAND_ECC_BCH_8]    = 13,
    [BRCMNAND_ECC_BCH_9]    = 15,
    [BRCMNAND_ECC_BCH_10]   = 17,
    [BRCMNAND_ECC_BCH_11]   = 18,
    [BRCMNAND_ECC_BCH_12]   = 20,
    [BRCMNAND_ECC_RESVD_1]  = 0,
    [BRCMNAND_ECC_RESVD_2]  = 0,
    [BRCMNAND_ECC_HAMMING]  = 3,
};
#else
static const unsigned int brcmnand_eccbytes[16] = {
    [BRCMNAND_ECC_DISABLE]  = 0,
    [BRCMNAND_ECC_BCH_1]    = 2,
    [BRCMNAND_ECC_BCH_2]    = 4,
    [BRCMNAND_ECC_BCH_3]    = 6,
    [BRCMNAND_ECC_BCH_4]    = 7,
    [BRCMNAND_ECC_BCH_5]    = 9,
    [BRCMNAND_ECC_BCH_6]    = 11,
    [BRCMNAND_ECC_BCH_7]    = 13,
    [BRCMNAND_ECC_BCH_8]    = 14,
    [BRCMNAND_ECC_BCH_9]    = 16,
    [BRCMNAND_ECC_BCH_10]   = 18,
    [BRCMNAND_ECC_BCH_11]   = 20,
    [BRCMNAND_ECC_BCH_12]   = 21,
    [BRCMNAND_ECC_RESVD_1]  = 0,
    [BRCMNAND_ECC_RESVD_2]  = 0,
    [BRCMNAND_ECC_HAMMING]  = 3,
};

#endif

//#endif

/* For backaward compatiblity. BRCM NAND driver still rely on the old nand_buffers structure
   which is changed to use dynamic buf alloction in 3.16 kernel. So copy the old nand_buffer
   definition here */

/*
 * This constant declares the max. oobsize / page, which
 * is supported now. If you add a chip with bigger oobsize/page
 * adjust this accordingly.
 */
#define NAND_MAX_OOBSIZE	576
#define NAND_MAX_PAGESIZE	8192

/**
 * struct nand_buffers - buffer structure for read/write
 * @ecccalc:    buffer for calculated ECC
 * @ecccode:    buffer for ECC read from flash
 * @databuf:    buffer for data - dynamically sized
 *
 * Do not change the order of buffers. databuf and oobrbuf must be in
 * consecutive order.
 */
struct brcmnand_buffers {
    uint8_t ecccalc[NAND_MAX_OOBSIZE];
    uint8_t ecccode[NAND_MAX_OOBSIZE];
    uint8_t databuf[NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE];
};

/**
 * struct brcmnand_chip - BrcmNAND Private Flash Chip Data
 * @param base      [BOARDSPECIFIC] address to access Broadcom NAND controller
 * @param chipsize  [INTERN] the size of one chip for multichip arrays
 * @param device_id [INTERN] device ID
 * @param verstion_id   [INTERN] version ID
 * @param options   [BOARDSPECIFIC] various chip options. They can partly be set to inform brcmnand_scan about
 * @param erase_shift   [INTERN] number of address bits in a block
 * @param page_shift    [INTERN] number of address bits in a page
 * @param ppb_shift [INTERN] number of address bits in a pages per block
 * @param page_mask [INTERN] a page per block mask
 * @cellinfo:           [INTERN] MLC/multichip data from chip ident
 * @param readw     [REPLACEABLE] hardware specific function for read short
 * @param writew    [REPLACEABLE] hardware specific function for write short
 * @param command   [REPLACEABLE] hardware specific function for writing commands to the chip
 * @param wait      [REPLACEABLE] hardware specific function for wait on ready
 * @param read_bufferram    [REPLACEABLE] hardware specific function for BufferRAM Area
 * @param write_bufferram   [REPLACEABLE] hardware specific function for BufferRAM Area
 * @param read_word [REPLACEABLE] hardware specific function for read register of BrcmNAND
 * @param write_word    [REPLACEABLE] hardware specific function for write register of BrcmNAND
 * @param scan_bbt  [REPLACEALBE] hardware specific function for scaning Bad block Table
 * @param chip_lock [INTERN] spinlock used to protect access to this structure and the chip
 * @param wq        [INTERN] wait queue to sleep on if a BrcmNAND operation is in progress
 * @param state     [INTERN] the current state of the BrcmNAND device
 * @param autooob   [REPLACEABLE] the default (auto)placement scheme
 * @param bbm       [REPLACEABLE] pointer to Bad Block Management
 * @param priv      [OPTIONAL] pointer to private chip date
 */

/*
 * Global members, shared by all ChipSelect, one per controller
 */
struct brcmnand_ctrl {
	spinlock_t			chip_lock;
	//atomic_t			semCount; // Used to lock out NAND access for NOR, TBD
	wait_queue_head_t		wq;
	brcmnand_state_t		state;
	
	struct brcmnand_buffers* 	buffers; // THT 2.6.18-5.3: Changed to pointer to accomodate EDU
#define BRCMNAND_OOBBUF(pbuf) (&((pbuf)->databuf[NAND_MAX_PAGESIZE]))

	unsigned int		numchips; // Always 1 in v0.0 and 0.1, up to 8 in v1.0+
	int 				CS[MAX_NAND_CS];	// Value of CS selected one per chip, in ascending order of chip Select (enforced)..
										// Say, user uses CS0, CS2, and CS5 for NAND, then the first 3 entries
										// have the values 0, 2 and 5, and numchips=3.
};

struct brcmnand_chip {

	
	/* Shared by all Chip select */
	struct brcmnand_ctrl* ctrl;

	/*
	 *	Private members, 
	 *
	  */
    //unsigned long     regs;   /* Register page */
    unsigned char __iomem       *vbase; /* Virtual address of start of flash */
    unsigned long       pbase; // Physical address of vbase
    unsigned long       device_id;

    //THT: In BrcmNAND, the NAND controller  keeps track of the 512B Cache
    // so there is no need to manage the buffer ram.
    //unsigned int      bufferram_index;
    //struct brcmnand_bufferram bufferram;

    int (*command)(struct mtd_info *mtd, int cmd, loff_t address, size_t len);
    int (*wait)(struct mtd_info *mtd, int state, uint32_t* pStatus, int timeout );
    
    unsigned short (*read_word)(void __iomem *addr);
    void (*write_word)(unsigned short value, void __iomem *addr);

    // THT: Sync Burst Read, not supported.
    //void (*mmcontrol)(struct mtd_info *mtd, int sync_read);

    // Private methods exported from BBT
    int (*block_bad)(struct mtd_info *mtd, loff_t ofs, int getchip);    
    int (*block_markbad)(struct mtd_info *mtd, loff_t ofs);
    int (*scan_bbt)(struct mtd_info *mtd);
    int (*erase_bbt)(struct mtd_info *mtd, struct erase_info *instr, int allowbbt, int doNotUseBBT);

    uint32_t (*ctrl_read) (uintptr_t command);
    void (*ctrl_write) (uintptr_t command, uint32_t val);
    uint32_t (*ctrl_writeAddr)(struct brcmnand_chip* chip, loff_t addr, int cmdEndAddr);

    /*
     * THT: Private methods exported to BBT, equivalent to the methods defined in struct ecc_nand_ctl
     * The caller is responsible to hold locks before calling these routines
     * Input and output buffers __must__ be aligned on a DW boundary (enforced inside the driver).
     * EDU may require that the buffer be aligned on a 512B boundary.
     */
    int (*read_page)(struct mtd_info *mtd,  
        uint8_t *outp_buf, uint8_t* outp_oob, uint64_t page);
    int (*write_page)(struct mtd_info *mtd, 
        const uint8_t *inp_buf, const uint8_t* inp_oob, uint64_t page);
    int (*read_page_oob)(struct mtd_info *mtd, uint8_t* outp_oob, uint64_t page);
    int (*write_page_oob)(struct mtd_info *mtd,  const uint8_t* inp_oob, uint64_t page, int isMarkBadBlock);
    
    int (*write_is_complete)(struct mtd_info *mtd, int* outp_needBBT);

    /*
     * THT: Same as the mtd calls with same name, except that locks are 
     * expected to be already held by caller.  Mostly used by BBT codes
     */
    int (*read_oob) (struct mtd_info *mtd, loff_t from,
             struct mtd_oob_ops *ops);
    int (*write_oob) (struct mtd_info *mtd, loff_t to,
             struct mtd_oob_ops *ops);

    uint64_t            chipSize;
	
    int                 directAccess;       // For v1,0+, use directAccess or EBI address   
	int				xor_disable;	// Value of  !NAND_CS_NAND_XOR:00
	int				csi; /* index into the CS array.  chip->CS[chip->csi] yield the value of HW ChipSelect */

    unsigned int        chip_shift; // How many bits shold be shifted.
    uint64_t            mtdSize;    // Total size of NAND flash, 64 bit integer for V1.0.  This supercedes mtd->size which is
                                // currently defined as a uint32_t.

    /* THT Added */
    unsigned int        busWidth, pageSize, blockSize; /* Actually page size from chip, as reported by the controller */

    unsigned int        erase_shift;
    unsigned int        page_shift;
    int                 phys_erase_shift;   
    int                 bbt_erase_shift;
    //unsigned int      ppb_shift;  /* Pages per block shift */
    unsigned int        page_mask;
    //int               subpagesize;
    uint8_t             cellinfo;
	uint8_t			nop;

    //u_char*           data_buf;   // Replaced by buffers
    //u_char*           oob_buf;
    int                 oobdirty;
    uint8_t*            data_poi;
    uint8_t*            oob_poi;
    unsigned int        options;
    int                 badblockpos;
    
    //unsigned long     chipsize;
    int                 pagemask;
    int64_t             pagebuf; /* Cached page number.  This can be a 36 bit signed integer. 
                          * -1LL denotes NULL/invalidated page cache. */
    int                 oobavail; // Number of free bytes per page
    int                 disableECC; /* Turn on for 100% valid chips that don't need ECC 
                         * might need in future for Spansion flash */
                
    struct nand_ecclayout *ecclayout;

	
	int			reqEccLevel;	/* Required ECC level, from chipID string (Samsung Type 2, Micron) 
								 * or from datasheet otherwise */

    // THT Used in lieu of struct nand_ecc_ctrl ecc;
	brcmnand_ecc_level_t ecclevel;	// Actual ECC scheme used, must be >= reqEccLevel
	int			ecctotal; // total number of ECC bytes per page, 3 for Small pages, 12 for large pages.
    int                 eccsize; // Size of the ECC block, always 512 for Brcm Nand Controller
	int			eccbytes; // How many bytes are used for ECC per eccsize (3 for Hamming)
	int			eccsteps; // How many ECC block per page (4 for 2K page, 1 for 512B page, 8 for 4K page etc...
	int			eccOobSize; // # of oob byte per ECC step, mostly 16, 27 for BCH-8

	int			eccSectorSize; // Sector size, not necessarily 512B for new flashes
	
    
    //struct nand_hw_control hwcontrol;

    struct mtd_oob_ops  ops;

    
    uint8_t             *bbt;
	uint32_t		bbtSize;
    int (*isbad_bbt)(struct mtd_info *mtd, loff_t ofs, int allowbbt);
    struct nand_bbt_descr   *bbt_td;
    struct nand_bbt_descr   *bbt_md;
    struct nand_bbt_descr   *badblock_pattern;
#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING
    struct brcmnand_cet_descr *cet;     /* CET descriptor */
#endif

    void                *priv;
};

#ifdef CONFIG_MTD_BRCMNAND_CORRECTABLE_ERR_HANDLING

#define BRCMNAND_CET_DISABLED   0x01    /* CET is disabled due to a serious error */
#define BRCMNAND_CET_LAZY   0x02    /* Reload CET when needed */
#define BRCMNAND_CET_LOADED 0x04    /* CET is in memory */
/*
 * struct brcmnand_cet_descr - Correctable Error Table (CET) descriptor
 * @offs        Offset in OOB where the CET signature begins
 * @len         Length (in bytes) of the CET signature
 * @startblk        Block address starting where CET begins
 * @sign        Growth of CET (top->down or down->top) 
 *          Inverse direct of BBT's sign
 * @flags       Status of CET disabled/lazy/loaded
 * @cerr_count      Total correctable errors encountered so far
 * @numblks     Number of blocks that CET spans
 * @maxblks     Maximum blocks that CET can have 2*numblks
 * @brcmnand_cet_memtable   Pointer to in-memory CET
 * @pattern     Identifier used to recognize CET
 * @cet_flush       Kernel work queue to handle flush of in-mem
 *          CET to the flash 
 */
struct brcmnand_cet_descr {
    uint8_t offs;       
    uint8_t len;        
    int startblk;   
    char sign;      /* 1 => bottom->top -1 => top->bottom - inverse of BBT */
    char flags;     
    uint32_t cerr_count;    
    int numblks;        
    int maxblks;        
    struct brcmnand_cet_memtable  *memtbl;  
    char *pattern;      
	struct mtd_info *mtd;
    struct delayed_work cet_flush;
};

/*
 * Copy of the CET in memory for faster access and easy rewrites
 * @isdirty     dirty = true => flush data to flash 
 * @blk         the physical block# (flash) that this bitvec belongs to
 * @bitvec      pointer to one block (blk#) of data
 */
struct brcmnand_cet_memtable {
    char isdirty;       
    int blk;        
    char *bitvec;       
};
#endif


/*
 * Options bits
 */
#define BRCMNAND_CONT_LOCK      (0x0001)


//extern void brcmnand_prepare_reboot(void);

/*
 * @ mtd        The MTD interface handle from opening "/dev/mtd<n>" or "/dev/mtdblock<n>"
 * @ buff       Buffer to hold the data read from the NOR flash, must be able to hold len bytes, and aligned on
 *          word boundary.
 * @ offset Offset of the data from CS0 (on NOR flash), must be on word boundary.
 * @ len        Number of bytes to be read, must be even number.
 *
 * returns 0 on success, negative error codes on failure.
 *
 * The caller thread may block until access to the NOR flash can be granted.
 * Further accesses to the NAND flash (from other threads) will be blocked until this routine returns.
 * The routine performs the required swapping of CS0/CS1 under the hood.
 */
extern int brcmnand_readNorFlash(struct mtd_info *mtd, void* buff, unsigned int offset, int len);

#if (CONFIG_BRCMNAND_MAJOR_VERS == 7)
#include "bchp_nand_7x.h"
#elif (CONFIG_BRCMNAND_MAJOR_VERS == 4)
#include "bchp_nand_40.h"
#elif (CONFIG_BRCMNAND_MAJOR_VERS == 2)
#include "bchp_nand_21_22.h"
#endif

#endif
#endif
