#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
 *  include/mtd/brcmnand_oob.h
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

#ifndef __BRCMNAND_OOB_H
#define __BRCMNAND_OOB_H

#include <linux/version.h>
#include <generated/autoconf.h>

#ifndef CONFIG_BRCMNAND_MTD_EXTENSION
#define UNDERSIZED_ECCPOS_API	1
#endif


/*
 * Assuming proper include that precede this has the typedefs for struct nand_oobinfo
 */

/**
 * brcmnand_oob oob info for 2K page
 */

/**
 * brcmnand_oob oob info for 512 page
 */
static struct nand_ecclayout brcmnand_oob_16 = {
	.eccbytes	= 3,
	.eccpos		= {
		6,7,8
		},
	.oobfree	= { {.offset=0, .length=5}, 
				{.offset=9,.length=7}, /* Byte 5 (6th byte) used for BI */
				{.offset=0, .length=0}		/* End marker */
			   }
			/* THT Bytes offset 4&5 are used by BBT.  Actually only byte 5 is used, but in order to accomodate
			 * for 16 bit bus width, byte 4 is also not used.  If we only use byte-width chip, (We did)
			 * then we can also use byte 4 as free bytes.
			 */
};

static struct nand_ecclayout brcmnand_oob_64 = {
	.eccbytes	= 12,
	.eccpos		= {
		6,7,8,
		22,23,24,
		38,39,40,
		54,55,56
		},
	.oobfree	= { /* 0-1 used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 2 bytes for BBT */
				{.offset=2, .length=4}, 
				{.offset=9,.length=13}, 		/* First slice {9,7} 2nd slice {16,6}are combined */ 
									/* ST uses 6th byte (offset=5) as Bad Block Indicator, 
									  * in addition to the 1st byte, and will be adjusted at run time */
				{.offset=25, .length=13},				/* 2nd slice  */
				{.offset=41, .length=13},				/* 3rd slice */
				{.offset=57, .length=7},				/* 4th slice */
	            {.offset=0, .length=0}				/* End marker */
			}
};


/*
 * 4K page SLC with Hamming ECC 
 */
static struct nand_ecclayout brcmnand_oob_128 = {
	.eccbytes	= 24,
	.eccpos		= {
		6,7,8,
		22,23,24,
		38,39,40,
		54,55,56,
		70,71,72,
		86,87,88,
		102,103,104,
		118,119,120
		},
	.oobfree	= { /* 0-1 used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 2 bytes for BBT */
				{.offset=2, .length=4}, 
				{.offset=9,.length=13}, 		
				{.offset=25, .length=13},				/* 2nd slice  */
				{.offset=41, .length=13},				/* 3rd slice */
				{.offset=57, .length=13},				/* 4th slice */
				{.offset=73, .length=13},				/* 5th slice  */
				{.offset=89, .length=13},				/* 6th slice */
				{.offset=105, .length=13},				/* 7th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=121, .length=7},				/* 8th slice */
	            {.offset=0, .length=0}				/* End marker */
#endif
			}
};


/* Small page with BCH-4 */
static struct nand_ecclayout brcmnand_oob_bch4_512 = {
	.eccbytes	= 7,
	.eccpos		= {
		9,10,11,12,13,14,15
		},
	.oobfree	= { 	{.offset=0, .length=5}, 
				{.offset=6,.length=3}, /* Byte 5 (6th byte) used for BI */
				{.offset=0, .length=0}		/* End marker */
			   }
};

/*
 * 2K page SLC/MLC with BCH-4 ECC, uses 7 ECC bytes per 512B ECC step
 */
static struct nand_ecclayout brcmnand_oob_bch4_2k = {
	.eccbytes	= 7*4,  /* 7*4 = 28 bytes */
	.eccpos		= { 
		9,10,11,12,13,14,15,
		25,26,27,28,29,30,31,
		41,42,43,44,45,46,47,
		57,58,59,60,61,62,63
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=8}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=9}, 		/* 2nd slice  */
				{.offset=32, .length=9},		/* 3rd slice  */
				{.offset=48, .length=9},		/* 4th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-4 ECC, uses 7 ECC bytes per 512B ECC step
 */
static struct nand_ecclayout brcmnand_oob_bch4_4k = {
	.eccbytes	= 7*8,  /* 7*8 = 56 bytes */
	.eccpos		= { 
		9,10,11,12,13,14,15,
		25,26,27,28,29,30,31,
		41,42,43,44,45,46,47,
		57,58,59,60,61,62,63,
		73,74,75,76,77,78,79,
		89,90,91,92,93,94,95,
		105,106,107,108,109,110,111,
		121,122,123,124,125,126,127
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=8}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=9}, 		/* 2nd slice  */
				{.offset=32, .length=9},		/* 3rd slice  */
				{.offset=48, .length=9},		/* 4th slice */
				{.offset=64, .length=9},		/* 5th slice */
				{.offset=80, .length=9},		/* 6th slice */
				{.offset=96, .length=9},		/* 7th slice */
				{.offset=112, .length=9},		/* 8th slice */
	            		//{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page MLC with BCH-4 ECC, uses 7 ECC bytes per 512B ECC step
 */
static struct nand_ecclayout brcmnand_oob_bch4_8k = {
	.eccbytes	= 7*16,  /* 7*16 = 112 bytes */
	.eccpos		= { 
		9,10,11,12,13,14,15,
		25,26,27,28,29,30,31,
		41,42,43,44,45,46,47,
		57,58,59,60,61,62,63,
		73,74,75,76,77,78,79,
		89,90,91,92,93,94,95,
		105,106,107,108,109,110,111,
		121,122,123,124,125,126,127,
#if ! defined(UNDERSIZED_ECCPOS_API)		
		137,138,139,140,141,142,142,
		153,154,155,156,157,158,159,
		169,170,171,172,173,174,175,
		185,186,187,188,189,190,191,
		201,202,203,204,205,206,207,
		217,208,209,210,211,212,218,
		233,204,205,206,207,208,209,
		249,250,251,252,253,254,255
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=8}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=9}, 		/* 2nd slice  */
				{.offset=32, .length=9},		/* 3rd slice  */
				{.offset=48, .length=9},		/* 4th slice */
				{.offset=64, .length=9},		/* 5th slice */
				{.offset=80, .length=9},		/* 6th slice */
				{.offset=96, .length=9},		/* 7th slice */
				{.offset=112, .length=9},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)	
				{.offset=128, .length=9},		/* 9th slice */
				{.offset=144, .length=9},		/* 10th slice */
				{.offset=160, .length=9},		/* 11th slice */
				{.offset=176, .length=9},		/* 12th slice */
				{.offset=192, .length=9},		/* 13th slice */
				{.offset=208, .length=9},		/* 14th slice */
				{.offset=240, .length=9},		/* 15th slice */	
#endif
	            		//{.offset=0, .length=0}			/* End marker */
			}
};

/* For NAND controller REV 7.0 or later, it  use new ECC algorithm that requires more ECC
   bytes. ECC_Bytes_Reqd (per 512 data Bytes) = roundup(ECC_LEVEL * M/8) where
   M is the BCH finite field order. For early chip, M is 13. For 63138, M is 14.
   It does not affect the Hamming and BCH4. But for BCH8 and BCH12, 63138 use 
   one more byte. On 63138, for BCH8 2K page size, there is not enough spare area 
   for cleanmarker if spare area is 16 bytes. So only the NAND part with 27 bytes 
   spare area is supported   */

#if CONFIG_MTD_BRCMNAND_VERSION < CONFIG_MTD_BRCMNAND_VERS_7_0
/*
 * 2K page SLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, and only have 16B OOB
 * Rely on the fact that the UBI/UBIFS layer does not store anything in the OOB
 */
static struct nand_ecclayout brcmnand_oob_bch8_16_2k = {
	.eccbytes	= 13*4,  /* 13*4 = 52 bytes */
	.eccpos		= { 
		3,4,5,6,7,8,9,10,11,12,13,14,15,
		19,20,21,22,23,24,25,26,27,28,29,30,31,
		35,36,37,38,39,40,41,42,43,44,45,46,47,
		51,52,53,54,55,56,57,58,59,60,61,62,63,
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=2}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=3}, 		/* 2nd slice  */
				{.offset=32, .length=3},		/* 3rd slice  */
				{.offset=48, .length=3},		/* 4th slice */
	            		//{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 2K page SLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, 27B+ OOB size 
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_2k = {
	.eccbytes	= 13*4,  /* 13*4 = 52 bytes */
	.eccpos		= { 
        	14,15,16,17,18,19,20,21,22,23,24,25,26,
		41,42,43,44,45,46,47,48,49,50,51,52,53,
		68,69,70,71,72,73,74,75,76,77,78,79,80,
		95,96,97,98,99,100,101,102,103,104,105,106,107,
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=13}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=14}, 		/* 2nd slice  */
				{.offset=54, .length=14},		/* 3rd slice  */
				{.offset=81, .length=14},		/* 4th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page SLC/MLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, and only have 16B OOB
 * Rely on the fact that the UBI/UBIFS layer does not store anything in the OOB
 */
static struct nand_ecclayout brcmnand_oob_bch8_16_4k = {
	.eccbytes	= 13*8,  /* 13*8 = 104 bytes */
	.eccpos		= { 
		3,4,5,6,7,8,9,10,11,12,13,14,15,
		19,20,21,22,23,24,25,26,27,28,29,30,31,
		35,36,37,38,39,40,41,42,43,44,45,46,47,
		51,52,53,54,55,56,57,58,59,60,61,62,63,
#if ! defined(UNDERSIZED_ECCPOS_API)
		67,68,69,70,71,72,73,74,75,76,77,78,79,
		83,84,85,86,87,88,89,90,91,92,93,94,95,
		99,100,101,102,103,104,105,106,107,108,109,110,111,
		115,116,117,118,119,120,121,122,123,124,125,126,127,
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=2}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=3}, 		/* 2nd slice  */
				{.offset=32, .length=3},		/* 3rd slice  */
				{.offset=48, .length=3},		/* 4th slice */
				{.offset=64, .length=3},		/* 5th slice */
				{.offset=80, .length=3},		/* 6th slice */
				{.offset=96, .length=3},		/* 7th slice */
				{.offset=112, .length=3},		/* 8th slice */
	            		//{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_4k = {
	.eccbytes	= 13*8,  /* 13*8 = 104 bytes */
	.eccpos		= { 
		14,15,16,17,18,19,20,21,22,23,24,25,26,
		41,42,43,44,45,46,47,48,49,50,51,52,53,
		68,69,70,71,72,73,74,75,76,77,78,79,80,
		95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		122,123,124,125,126,127,128,129,130,131,132,133,134,
		149,150,151,152,153,154,155,156,157,158,159,160,161,
		176,177,178,179,180,181,182,183,184,185,186,187,188,
		203,204,205,206,207,208,209,210,211,212,213,214,215
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=13}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=14}, 		/* 2nd slice  */
				{.offset=54, .length=14},		/* 3rd slice  */
				{.offset=81, .length=14},		/* 4th slice */
				{.offset=108, .length=14},		/* 5th slice */
				{.offset=135, .length=14},		/* 6th slice */
				{.offset=162, .length=14},		/* 7th slice */
				{.offset=189, .length=14},		/* 8th slice */
	            		//{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page SLC/MLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, and only have 16B OOB
 * Rely on the fact that the UBI/UBIFS layer does not store anything in the OOB
 */
static struct nand_ecclayout brcmnand_oob_bch8_16_8k = {
	.eccbytes	= 13*16,  /* 13*8 = 208 bytes */
	.eccpos		= { 
		3,4,5,6,7,8,9,10,11,12,13,14,15,
		19,20,21,22,23,24,25,26,27,28,29,30,31,
		35,36,37,38,39,40,41,42,43,44,45,46,47,
		51,52,53,54,55,56,57,58,59,60,61,62,63,
#if ! defined(UNDERSIZED_ECCPOS_API)
		67,68,69,70,71,72,73,74,75,76,77,78,79,
		83,84,85,86,87,88,89,90,91,92,93,94,95,
		99,100,101,102,103,104,105,106,107,108,109,110,111,
		115,116,117,118,119,120,121,122,123,124,125,126,127,

		131,132,133,134,135,136,137,138,139,140,141,142,143,
		147,148,149,150,151,152,153,154,155,156,157,158,159,
		163,164,165,166,167,168,169,170,171,172,173,174,175,
		179,180,181,182,183,184,185,186,187,188,189,190,191,
		195,196,197,198,199,200,201,202,203,204,205,206,207,
		211,212,213,214,215,216,217,218,219,220,221,222,223,
		227,228,229,230,231,232,233,234,235,236,237,238,239,
		243,244,245,246,247,248,249,250,251,252,253,254,255
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=2}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=3}, 		/* 2nd slice  */
				{.offset=32, .length=3},		/* 3rd slice  */
				{.offset=48, .length=3},		/* 4th slice */
				{.offset=64, .length=3},		/* 5th slice */
				{.offset=80, .length=3},		/* 6th slice */
				{.offset=96, .length=3},		/* 7th slice */
				{.offset=112, .length=3},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=128, .length=3},		/* 9th slice */
				{.offset=144, .length=3},		/* 10th slice */
				{.offset=128, .length=3},		/* 11th slice */
				{.offset=144, .length=3},		/* 12th slice */
				{.offset=160, .length=3},		/* 13th slice */
				{.offset=176, .length=3},		/* 14th slice */
				{.offset=192, .length=3},		/* 15th slice */
				{.offset=208, .length=3},		/* 16th slice */	
#endif
	            		//{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-8 ECC, uses 13 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_8k = {
	.eccbytes	= 13*16,  /* 13*16 = 208 bytes */
	.eccpos		= { 
		14,15,16,17,18,19,20,21,22,23,24,25,26,
		41,42,43,44,45,46,47,48,49,50,51,52,53,
		68,69,70,71,72,73,74,75,76,77,78,79,80,
		95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		122,123,124,125,126,127,128,129,130,131,132,133,134,
		149,150,151,152,153,154,155,156,157,158,159,160,161,
		176,177,178,179,180,181,182,183,184,185,186,187,188,
		203,204,205,206,207,208,209,210,211,212,213,214,215,

		230,231,232,233,234,235,236,237,238,239,240,241,242,
		257,258,259,260,261,262,263,264,265,266,267,268,269,
		284,285,286,287,288,289,290,291,292,293,294,295,296,
		311,312,313,314,315,316,317,318,319,320,321,322,323,
		338,339,340,341,342,343,344,345,346,347,348,349,350,
		365,366,367,368,369,370,371,372,373,374,375,376,377,
		392,393,394,395,396,397,398,399,400,401,402,403,404,
		419,420,421,422,423,424,425,426,427,428,429,430,431
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=13}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=14}, 		/* 2nd slice  */
				{.offset=54, .length=14},		/* 3rd slice  */
				{.offset=81, .length=14},		/* 4th slice */
				{.offset=108, .length=14},		/* 5th slice */
				{.offset=135, .length=14},		/* 6th slice */
				{.offset=162, .length=14},		/* 7th slice */
				{.offset=189, .length=14},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=216, .length=14},		/* 9th slice */
				{.offset=243, .length=14},		/* 10th slice */
				{.offset=270, .length=14},		/* 11th slice */
				{.offset=297, .length=14},		/* 12th slice */
				{.offset=324, .length=14},		/* 13th slice */
				{.offset=351, .length=14},		/* 14th slice */
				{.offset=378, .length=14},		/* 15th slice */
				{.offset=405, .length=14},		/* 16th slice */
#endif
			}
};

/*
 * 2K page with BCH-12 ECC, uses 20 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_2k = {
	.eccbytes	= 20*4,  /* 20*8 = 160 bytes */
	.eccpos		= { 
		 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=6}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=7}, 		/* 2nd slice  */
				{.offset=54, .length=7},		/* 3rd slice  */
				{.offset=81, .length=7},		/* 4th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page MLC with BCH-12 ECC, uses 20 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_4k = {
	.eccbytes	= 20*8,  /* 20*8 = 160 bytes */
	.eccpos		= { 
		 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,
		196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=6}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=7}, 		/* 2nd slice  */
				{.offset=54, .length=7},		/* 3rd slice  */
				{.offset=81, .length=7},		/* 4th slice */
				{.offset=108, .length=7},		/* 5th slice */
				{.offset=135, .length=7},		/* 6th slice */
				{.offset=162, .length=7},		/* 7th slice */
				{.offset=189, .length=7},		/* 8th slice */
	            		//{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-12 ECC, uses 20 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_8k = {
	.eccbytes	= 20*16,  /* 20*8 = 320 bytes */
	.eccpos		= { 
		 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,
		196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,

		223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
		250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
		277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,
		304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,
		331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,
		358,359,360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,
		385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,
		412,413,414,415,416,417,418,419,420,421,422,423,424,425,426,427,428,429,430,431
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=6}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=7}, 		/* 2nd slice  */
				{.offset=54, .length=7},		/* 3rd slice  */
				{.offset=81, .length=7},		/* 4th slice */
				{.offset=108, .length=7},		/* 5th slice */
				{.offset=135, .length=7},		/* 6th slice */
				{.offset=162, .length=7},		/* 7th slice */
				{.offset=189, .length=7},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=216, .length=7},		/* 5th slice */
				{.offset=243, .length=7},		/* 6th slice */
				{.offset=270, .length=7},		/* 7th slice */
				{.offset=297, .length=7},		/* 8th slice */
				{.offset=324, .length=7},		/* 5th slice */
				{.offset=351, .length=7},		/* 6th slice */
				{.offset=378, .length=7},		/* 7th slice */
				{.offset=405, .length=7},		/* 8th slice */
#endif
			}
};
#else //CONFIG_MTD_BRCMNAND_VERSION = CONFIG_MTD_BRCMNAND_VERS_7_0
/*
 * 2K page SLC with BCH-8 ECC, uses 14 ECC bytes per 512B ECC step, 27B+ OOB size 
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_2k = {
	.eccbytes	= 14*4,  /* 14*4 = 56 bytes */
	.eccpos		= { 
		13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		94,95,96,97,98,99,100,101,102,103,104,105,106,107,
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=12}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=13}, 		/* 2nd slice  */
				{.offset=54, .length=13},		/* 3rd slice  */
				{.offset=81, .length=13},		/* 4th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page SLC/MLC with BCH-8 ECC, uses 14 ECC bytes per 512B ECC step, and only have 16B OOB
 * Rely on the fact that the UBI/UBIFS layer does not store anything in the OOB
 */
static struct nand_ecclayout brcmnand_oob_bch8_16_4k = {
	.eccbytes	= 14*8,  /* 14*8 = 112 bytes */
	.eccpos		= { 
		2,3,4,5,6,7,8,9,10,11,12,13,14,15,
		18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		34,35,36,37,38,39,40,41,42,43,44,45,46,47,
		50,51,52,53,54,55,56,57,58,59,60,61,62,63,
#if ! defined(UNDERSIZED_ECCPOS_API)
		66,67,68,69,70,71,72,73,74,75,76,77,78,79,
		82,83,84,85,86,87,88,89,90,91,92,93,94,95,
		98,99,100,101,102,103,104,105,106,107,108,109,110,111,
		114,115,116,117,118,119,120,121,122,123,124,125,126,127,
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=1}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=2}, 		/* 2nd slice  */
				{.offset=32, .length=2},		/* 3rd slice  */
				{.offset=48, .length=2},		/* 4th slice */
				{.offset=64, .length=2},		/* 5th slice */
				{.offset=80, .length=2},		/* 6th slice */
				{.offset=96, .length=2},		/* 7th slice */
				{.offset=112, .length=2},		/* 8th slice */
				{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-8 ECC, uses 14 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_4k = {
	.eccbytes	= 14*8,  /* 14*8 = 112 bytes */
	.eccpos		= { 
		13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		175,176,177,178,179,180,181,182,183,184,185,186,187,188,
		202,203,204,205,206,207,208,209,210,211,212,213,214,215
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=12}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=13}, 		/* 2nd slice  */
				{.offset=54, .length=13},		/* 3rd slice  */
				{.offset=81, .length=13},		/* 4th slice */
				{.offset=108, .length=13},		/* 5th slice */
				{.offset=135, .length=13},		/* 6th slice */
				{.offset=162, .length=13},		/* 7th slice */
				{.offset=189, .length=13},		/* 8th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page SLC/MLC with BCH-8 ECC, uses 14 ECC bytes per 512B ECC step, and only have 16B OOB
 * Rely on the fact that the UBI/UBIFS layer does not store anything in the OOB
 */
static struct nand_ecclayout brcmnand_oob_bch8_16_8k = {
	.eccbytes	= 14*16,  /* 14*16 = 224 bytes */
	.eccpos		= { 
		2,3,4,5,6,7,8,9,10,11,12,13,14,15,
		18,19,20,21,22,23,24,25,26,27,28,29,30,31,
		34,35,36,37,38,39,40,41,42,43,44,45,46,47,
		50,51,52,53,54,55,56,57,58,59,60,61,62,63,
#if ! defined(UNDERSIZED_ECCPOS_API)
		66,67,68,69,70,71,72,73,74,75,76,77,78,79,
		82,83,84,85,86,87,88,89,90,91,92,93,94,95,
		98,99,100,101,102,103,104,105,106,107,108,109,110,111,
		114,115,116,117,118,119,120,121,122,123,124,125,126,127,

		130,131,132,133,134,135,136,137,138,139,140,141,142,143,
		146,147,148,149,150,151,152,153,154,155,156,157,158,159,
		162,163,164,165,166,167,168,169,170,171,172,173,174,175,
		178,179,180,181,182,183,184,185,186,187,188,189,190,191,
		194,195,196,197,198,199,200,201,202,203,204,205,206,207,
		210,211,212,213,214,215,216,217,218,219,220,221,222,223,
		226,227,228,229,230,231,232,233,234,235,236,237,238,239,
		242,243,244,245,246,247,248,249,250,251,252,253,254,255
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=1}, 		/* 1st slice loses byte 0 */
				{.offset=16,.length=2}, 		/* 2nd slice  */
				{.offset=32, .length=2},		/* 3rd slice  */
				{.offset=48, .length=2},		/* 4th slice */
				{.offset=64, .length=2},		/* 5th slice */
				{.offset=80, .length=2},		/* 6th slice */
				{.offset=96, .length=2},		/* 7th slice */
				{.offset=112, .length=2},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=128, .length=2},		/* 9th slice */
				{.offset=144, .length=2},		/* 10th slice */
				{.offset=128, .length=2},		/* 11th slice */
				{.offset=144, .length=2},		/* 12th slice */
				{.offset=160, .length=2},		/* 13th slice */
				{.offset=176, .length=2},		/* 14th slice */
				{.offset=192, .length=2},		/* 15th slice */
				{.offset=208, .length=2},		/* 16th slice */	
#endif
	            		{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-8 ECC, uses 14 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch8_27_8k = {
	.eccbytes	= 14*16,  /* 14*16 = 224 bytes */
	.eccpos		= { 
		13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		175,176,177,178,179,180,181,182,183,184,185,186,187,188,
		202,203,204,205,206,207,208,209,210,211,212,213,214,215,

		229,230,231,232,233,234,235,236,237,238,239,240,241,242,
		256,257,258,259,260,261,262,263,264,265,266,267,268,269,
		283,284,285,286,287,288,289,290,291,292,293,294,295,296,
		310,311,312,313,314,315,316,317,318,319,320,321,322,323,
		337,338,339,340,341,342,343,344,345,346,347,348,349,350,
		364,365,366,367,368,369,370,371,372,373,374,375,376,377,
		391,392,393,394,395,396,397,398,399,400,401,402,403,404,
		418,419,420,421,422,423,424,425,426,427,428,429,430,431
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=12}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=13}, 		/* 2nd slice  */
				{.offset=54, .length=13},		/* 3rd slice  */
				{.offset=81, .length=13},		/* 4th slice */
				{.offset=108, .length=13},		/* 5th slice */
				{.offset=135, .length=13},		/* 6th slice */
				{.offset=162, .length=13},		/* 7th slice */
				{.offset=189, .length=13},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=216, .length=13},		/* 9th slice */
				{.offset=243, .length=13},		/* 10th slice */
				{.offset=270, .length=13},		/* 11th slice */
				{.offset=297, .length=13},		/* 12th slice */
				{.offset=324, .length=13},		/* 13th slice */
				{.offset=351, .length=13},		/* 14th slice */
				{.offset=378, .length=13},		/* 15th slice */
				{.offset=405, .length=13},		/* 16th slice */
#endif
			}
};

/*
 * 2K page with BCH-12 ECC, uses 21 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_2k = {
	.eccbytes	= 21*4,  /* 21*4 = 84 bytes */
	.eccpos		= { 
		 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=5}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=6}, 		/* 2nd slice  */
				{.offset=54, .length=6},		/* 3rd slice  */
				{.offset=81, .length=6},		/* 4th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};

/*
 * 4K page MLC with BCH-12 ECC, uses 21 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_4k = {
	.eccbytes	= 21*8,  /* 21*8 = 168 bytes */
	.eccpos		= { 
		 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
  		87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,
		195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=5}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=6}, 		/* 2nd slice  */
				{.offset=54, .length=6},		/* 3rd slice  */
				{.offset=81, .length=6},		/* 4th slice */
				{.offset=108, .length=6},		/* 5th slice */
				{.offset=135, .length=6},		/* 6th slice */
				{.offset=162, .length=6},		/* 7th slice */
				{.offset=189, .length=6},		/* 8th slice */
	            		{.offset=0, .length=0}			/* End marker */
			}
};


/*
 * 4K page MLC with BCH-12 ECC, uses 21 ECC bytes per 512B ECC step, and requires OOB size of 27B+
 */
static struct nand_ecclayout brcmnand_oob_bch12_27_8k = {
	.eccbytes	= 21*16,  /* 21*16 = 336 bytes */
	.eccpos		= { 
		 6, 7, 8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
		33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,
		60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
		87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,
#if ! defined(UNDERSIZED_ECCPOS_API)
		114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,
		141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
		168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,
                195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
		222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,
		249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,
		276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,
		303,304,305,306,307,308,309,310,311,312,313,314,315,316,317,318,319,320,321,322,323,
		330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,
		357,358,359,360,361,362,363,364,365,366,367,368,369,370,371,372,373,374,375,376,377,
		384,385,386,387,388,389,390,391,392,393,394,395,396,397,398,399,400,401,402,403,404,
		411,412,413,414,415,416,417,418,419,420,421,422,423,424,425,426,427,428,429,430,431
#endif
		},
	.oobfree	= { /* 0  used for BBT and/or manufacturer bad block marker, 
	                    * first slice loses 1 byte for BBT */
				{.offset=1, .length=5}, 		/* 1st slice loses byte 0 */
				{.offset=27,.length=6}, 		/* 2nd slice  */
				{.offset=54, .length=6},		/* 3rd slice  */
				{.offset=81, .length=6},		/* 4th slice */
				{.offset=108, .length=6},		/* 5th slice */
				{.offset=135, .length=6},		/* 6th slice */
				{.offset=162, .length=6},		/* 7th slice */
				{.offset=189, .length=6},		/* 8th slice */
#if ! defined(UNDERSIZED_ECCPOS_API)
				{.offset=216, .length=6},		/* 5th slice */
				{.offset=243, .length=6},		/* 6th slice */
				{.offset=270, .length=6},		/* 7th slice */
				{.offset=297, .length=6},		/* 8th slice */
				{.offset=324, .length=6},		/* 5th slice */
				{.offset=351, .length=6},		/* 6th slice */
				{.offset=378, .length=6},		/* 7th slice */
				{.offset=405, .length=6},		/* 8th slice */
#endif
			}
};
#endif

#endif

#endif
