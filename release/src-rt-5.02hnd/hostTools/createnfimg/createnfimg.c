/*
******************************************************************************
* Copyright 2009 Broadcom Inc.
******************************************************************************
<:label-BRCM:2013:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*
* Description:
*   This program takes in a binary file and breaks it into 512 byte or 2048
*   byte pages.  For each 512 byte partial page, it creates the NAND OOB area
*   contents with a Hamming or BCH ECC and optional JFFS2 cleanmarker. Each
*   page is written out, followed by the OOB area wich is 16 bytes for a 512
*   byte page or 64 bytes for a 2048 byte page. The output file name is the 
*   input file name with a ".out" extension.
*
*   The BCH finite field order, 'm' parameter, is 13 for the BCM6368, BCM6816,
*   BCM6362, BCM6328 and BCM63268.  This field is 14 for chips that are newer
*   than the BCM63268.
*
* Build:
*   gcc -o createnfimg createnfimg.c
*
* Usage examples:
*   BCM63268
*   Small page Hamming: ./createnfimg -b 1 -P  512 -i bcm963268GW_nand_cferom_fs_image_16.w
*   Large page Hamming: ./createnfimg -b 1 -P 2048 -i bcm963268GW_nand_cferom_fs_image_128.w
*   Small page BCH-4  : ./createnfimg -b 4 -P  512 -m 13 -i bcm963268GW_nand_cferom_fs_image_16.w
*   Large page BCH-8  : ./createnfimg -b 8 -P 2048 -m 13 -i bcm963268GW_nand_cferom_fs_image_128.w
*
*   BCM6828
*   Small page BCH-4  : ./createnfimg -b 4 -P  512 -m 14 -i bcm96828GW_nand_cferom_fs_image_16.w
*
******************************************************************************
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>


/********************************************************************************/
/* Begin support for JFFS2 cleanmarker in spare area and 1 bit Hamming ECC     */
/********************************************************************************/

#define HAMMING_SIZE                3
#define HAMMING_SPARE_B0            6

#define NAC_ECC_LVL_HAMMING         1
#define NAC_ECC_LVL_BCH_4           4
#define NAC_ECC_LVL_BCH_8           8
#define NAC_ECC_LVL_BCH_12          12

#define CLEANMARKER_SIZE            8

#define OOB_SIZE (p_oob_size * partial_pages_per_page)

/* Condition to determine the spare layout. */
#define LAYOUT_PARMS(L,S,P)     \
    (((unsigned long)(L)<<28) | ((unsigned long)(S)<<16) | (P))

/* Each bit in the ECCMSK array represents a spare area byte. Bits that are
 * set correspond to spare area bytes that are reserved for the ECC or bad
 * block indicator. Bits that are not set can be used for data such as the
 * JFFS2 cleanmarker. This macro returns 0 if the spare area byte at offset,
 * OFS, is available and non-0 if it is being used for the ECC or BI.
 */
#define ECC_MASK_BIT(ECCMSK, OFS)   (ECCMSK[OFS / 8] & (1 << (OFS % 8)))

typedef struct SpareLayout
{
    unsigned char sl_bi_ofs[2];
    unsigned char sl_spare_mask[];
} SPARE_LAYOUT, *PSPARE_LAYOUT;

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_64 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

/* 0,0,0,0,0,B,E,E-E,0,0,0,0,0,0,0 */
SPARE_LAYOUT brcmnand_oob_16 =
    {{5, 5}, {0xe0, 0x01}};

/* 0,0,0,0,0,B,0,0-0,E,E,E,E,E,E,E */
SPARE_LAYOUT brcmnand_oob_bch4_512 =
    {{5, 5}, {0x20, 0xfe}};

/* B,B,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 * 0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,0,0,0,0,0,0,0
 */
SPARE_LAYOUT brcmnand_oob_128 =
    {{0, 1}, {0xc3, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01,
              0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01}};

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
 */
SPARE_LAYOUT brcmnand_oob_bch4_2k =
    {{0, 0}, {0x01, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_2k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 * 0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-0,0,0,E,E,E,E,E,-E,E,E,E,E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_16_4k =
    {{0, 0}, {0xf9, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
              0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff}};

/* B,0,0,0,0,0,0,0-0,0,0,0,0,E,E,E-E,E,E,E,E,E,E,E-E,E,E
 * 0,0,0,0,0-0,0,0,0,0,0,0,0-E,E,E,E,E,E,E,E-E,E,E,E,E,E
 * 0,0-0,0,0,0,0,0,0,0-0,0,0,E,E,E,E,E-E,E,E,E,E,E,E,E-E
 * 0,0,0,0,0,0,0-0,0,0,0,0,0,E,E-E,E,E,E,E,E,E,E-E,E,E,E
 */
SPARE_LAYOUT brcmnand_oob_bch8_27_2k =
   {{0, 0}, {0x01, 0xe0, 0xff, 0x07, 0x00, 0xff, 0x3f, 0x00, 0xf8, 0xff,
             0x01, 0xc0, 0xff, 0x0f}};

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

int p_spare_jffs2_cleanmarker = 1;
uint8_t *spare_cleanmarker;
const uint8_t cleanmarker_be[] = {0x19, 0x85, 0x20, 0x03, 0x00, 0x00, 0x00, 0x08};
const uint8_t cleanmarker_le[] = {0x85, 0x19, 0x03, 0x20, 0x08, 0x00, 0x00, 0x00};
const uint8_t * cleanmarker;

static void calc_spare_layout(int level, int spare_size, int page_size, uint8_t *pcm);
static void nand_calculate_ecc_BRCM_Hamming(const uint8_t *data, uint8_t *ecc_code);

/********************************************************************************/
/* End support for JFFS2 cleanmarker in spare area and 1 bit Hamming ECC       */
/********************************************************************************/


#define P "  -> "
#define SWAP_DATA_BEFORE_ECC_COMPUTATION         0  /* Change as needed. */
#define SWAP_DATA_AFTER_ECC_COMPUTATION          0  /* Change as needed. */
#define MIN_FIELD_ORDER                         13
#define MAX_FIELD_ORDER                         14
#define MIN_T                                    1
#define MAX_T                                   40


/*******************************************************************************
** Function declarations
********************************************************************************/
static void nand_calculate_ecc_BRCM_bch(uint8_t *data,uint8_t *ecc_code,int m, int n,int k,int t,int u,int v);
static void swap32(uint8_t *data);
static void swap(uint8_t *, uint8_t * , int);

/* Variables that start with 'p_' are program parameters. */
int  p_seed=1;                   /* Random seed specification. */
int  p_bch_level=4;              /* BCH code, either 4, 8 or 12. */
int  p_oob_size=16;              /* OOB size in bytes per 512 byte subpage, must be 16 or 27. */
int  p_page_size=2048;           /* Page size in bytes. */
int  p_pages_per_block=64;       /* Pages per erase block */
char *p_outfile_out_name = 0;    /* The data and the oob. */
const char *p_infile_name = 0;
int  p_partial_page_size = 512;  
int  p_field_order = MIN_FIELD_ORDER;  /* Order of the finite field for BCH encoding (between MIN_FIELD_ORDER and MAX_FIELD_ORDER, inclusive) */
char *pgm_name;
int  p_trim=1;                   /* Trim input file to page size. */
int  p_pad=0;                    /* amount to pad OOB after ECC has been generated */


void usage()
{
    printf("\nUSAGE:\n");
    printf("  %s <options>\n", pgm_name);
    printf("\nDESCRIPTION:\n");
    printf("  %s takes an input file, generates the ecc bytes for it and creates an output\n", pgm_name);
    printf("  file with both the data and the OOB bytes.\n");
    printf("\nOPTIONS:\n");
    printf("    -b <bch_level>     -- Level of BCH correction; default=%d.\n", p_bch_level);
    printf("    -c <partial_page_size>   -- Partial page size in bytes; default=%d.\n", p_partial_page_size);
    printf("    -d <pad_amount>    -- OOB padding amount to fill after ECC is finished,\n");
    printf("                          for example with newer NAND devices that have 128 bytes OOB per page,\n");
    printf("                          treat part like OOB=64 (-r 16) and set this value to 64\n");
    printf("                          to pad 64 bytes after the OOB 64 bytes; default=%d.\n", p_pad);
    printf("    -h                 -- Prints help text.\n");
    printf("    -i <infile>        -- Name of the input file.  Output files will be 'infile.out'.\n");
    printf("    -j <0|1>           -- 1=Add JFFS2 cleanmarker to spare area: default=%d.\n", p_spare_jffs2_cleanmarker);
    printf("    -l <0|1>           -- 1=Select little endian clean marker (default is big endian)\n");
    printf("    -m <field_order>   -- Order of the finite field for BCH; default=%d.\n", p_field_order);
    printf("    -n <pages_per_block> -- Pages per erase block; default=%d.\n", p_pages_per_block);
    printf("    -p <page_size>     -- Page size in bytes; default=%d.\n", p_page_size);
    printf("    -r <oob_size>      -- OOB size in bytes per 512 byte subpage; default=%d.\n", p_oob_size);
    printf("    -s <seed>          -- Random seed value; default=%d.\n", p_seed);
    printf("    -t <0|1>           -- 1=Trim input file to a page boundary (eliminate extraneous signature bytes); default=%d.\n", p_trim);
    printf("\n");
    exit(1);
}




int main(int argc, char **argv)
{


    uint8_t   *data;                    /* Points to whole page of data (512 or 2048) + OOB (16/27 bytes oob x subpages) */
    uint8_t   ecc_code[70];             /* 7 bytes of ECC bits for BCH4, upto 70 bytes of ECC for t = 40, m = 14, BCH code */
    int       partial_pages_per_page;
    int       page = 0;   
    int       s,n,k,p,t,u,v,i,m,subpage;
    int       optchar;

    FILE      *infile;
    FILE      *outfile_out;
    char      *pch;
    int       bytes_read;
    int       ii, bit2flip, byte2flip;
    const char ubifs_split_marker[] = "BcmFs-ubifs";

    pgm_name = strrchr(argv[0], '/');
    pgm_name = pgm_name ? pgm_name + 1 : argv[0];

    cleanmarker = cleanmarker_be;

    if (argc < 2) usage();

    /* Parse options */
    opterr = 0;
    while (-1 != (optchar = getopt(argc, argv, "b:B:c:C:i:I:j:J:l:L:m:M:n:N:p:P:r:R:s:S:t:T:d:D"))) {
        switch (optchar) {
        case 'b':
        case 'B':
            p_bch_level = atoi(optarg);
            if (p_bch_level<MIN_T || p_bch_level>MAX_T) {
                fprintf(stderr, "%s: BCH value '%s' illegal; must be between %d and %d, inclusive.\n", pgm_name, optarg,MIN_T,MAX_T);
                exit(1);
            }
            break;

        case 'c':
        case 'C':
            p_partial_page_size = atoi(optarg);
            if (p_partial_page_size!=512 && p_partial_page_size != 1024) {
                fprintf(stderr, "%s: Illegal partial page size '%s'.\n", pgm_name, optarg);
                exit(1);
            }
            break;

        case 'i':
        case 'I':
            p_infile_name = optarg;
            break;

        case 'j':
        case 'J':
            p_spare_jffs2_cleanmarker = atoi(optarg);
            break;

        case 'l':
        case 'L':
            if (atoi(optarg))
                cleanmarker = cleanmarker_le;
            else
                cleanmarker = cleanmarker_be;
            break;

        case 'm':
        case 'M':
            p_field_order = atoi(optarg);
            if (p_field_order<MIN_FIELD_ORDER || p_field_order>MAX_FIELD_ORDER ) {
                fprintf(stderr, "%s: Only Finite fields from GF(2^%d) to GF(2^%d) are supported.\n",pgm_name,MIN_FIELD_ORDER,MAX_FIELD_ORDER);
                exit(1);
            }
            break;

        case 'n':
        case 'N':
            p_pages_per_block = atoi(optarg);
            break;

        case 'p':
        case 'P':
            p_page_size = atoi(optarg);
            break;

        case 'r':
        case 'R':
            p_oob_size = atoi(optarg);
            break;

        case 's':
        case 'S':
            p_seed = atoi(optarg);
            break;

        case 't':
        case 'T':
            p_trim = atoi(optarg);
            break;

        case 'd':
        case 'D':
            p_pad = atoi(optarg);
            break;

        default:
            usage();
        }

    }

    if (p_page_size==0 || p_page_size%p_partial_page_size) {
        fprintf(stderr, "%s: Illegal page size '%s'.\n", pgm_name, optarg);
        exit(1);
    }


    if (! p_infile_name) {
        fprintf(stderr, "%s: no infile name given!\n", pgm_name);
        exit(1);
    }


    partial_pages_per_page = p_page_size / p_partial_page_size;
    data = malloc(p_page_size + OOB_SIZE + p_pad);
    spare_cleanmarker = malloc(OOB_SIZE);

    /*
    **   Note 1: Number of ecc_code bytes v <= s
    **   Note 2: Zeros are padded at beginning of ecc_code when p does not divide by 8.
    */

    /* Finite field order */
    m = p_field_order;  

    /* Error correction capability (in bits). */
    t = p_bch_level;  

    /* Number of spare bytes per partial page. */
    s = p_oob_size;
    
    /* Codeword length (in bits) */
    n = (p_partial_page_size + s) * 8;

    /* Parity length (in bits). */
    p = m * t;

    /* Message length (in bits). */
    k = n - p;

    /* Number of data bytes = floor(k/8) */
    u = k / 8;

    /* Number of ecc_code bytes, v = ceil(p/8) = ceil(m*t/8) = p_partial_page_size+s-u */
    v = p_partial_page_size + s - u;

    if (v > p_oob_size) {
      fprintf(stderr, "%s: Number of spare bytes must be at least %d\n", pgm_name, v); 
      exit(1);
    }

    infile  = fopen(p_infile_name, "rb");
    if (infile == NULL) 
    {
        fprintf(stderr, "%s: Could not open input file '%s'.\n", pgm_name, p_infile_name);
        exit(1);
    }

    

    /* Open output out file (name = <input_file>.out) */
    i= strlen(p_infile_name);
    p_outfile_out_name = (char *)malloc(i + 16);
    strcpy(p_outfile_out_name, p_infile_name);
    if (pch = strchr(p_outfile_out_name, '.')) {
        *pch = 0;
    }
    strcat(p_outfile_out_name, ".out");

    outfile_out = fopen(p_outfile_out_name, "wb");
    if (outfile_out == NULL) 
    {
        fprintf(stderr, "%s: Could not open output file '%s' for writing.\n", pgm_name, p_outfile_out_name);
        fclose (infile);
        exit(1);
    }

    if (p_spare_jffs2_cleanmarker == 1)
        calc_spare_layout(p_bch_level, OOB_SIZE, p_page_size, spare_cleanmarker);

    if (t == 1)
    {
        printf(P "Use Hamming Code:\n");
        printf(P "error correction capability         t = %4d (bits)\n", t);
        printf(P "number of ecc_code bytes            v = %4d\n", HAMMING_SIZE);
        printf(P "number of spare bytes               s = %4d\n", s);
    }
    else
    {
        printf(P "Use BCH Code:\n");
        printf(P "error correction capability         t = %4d (bits)\n", t);
        printf(P "codeword length                     n = %4d (bits)\n", n);
        printf(P "message length                      k = %4d (bits)\n", k);
        printf(P "parity length                       p = %4d (bits)\n", p);
        printf(P "number of ecc_code bytes            v = %4d\n", v);
        printf(P "number of spare bytes               s = %4d\n", s);
        printf(P "number of data bytes used for ecc   u = %4d\n", u);
    }
    
    /* Generate oob data. */
    while (1)
    {
        /* fill the buffer with 0xFF each time through */
        for (i = 0; i < (p_page_size + OOB_SIZE + p_pad); i++)
        {
            data[i] = 0xff;
        }

        bytes_read = fread(data, 1, p_page_size, infile);

        /* If EOF & entire page is written -- exit! */
        if ( bytes_read == 0 ) break;
        /* snap generation of outfile to input file page size, this will trim any extraneaous bytes from the end of the input file */
        if (p_trim && (bytes_read != p_page_size)) break;
        
        if (p_spare_jffs2_cleanmarker && !page)
        { 
            for (subpage = 0; subpage < partial_pages_per_page; subpage++)
                for(i = 0; i < p_oob_size; i++)
                    data[p_page_size + (subpage * p_oob_size) + i] = spare_cleanmarker[(subpage * p_oob_size) + i];
        }

        for (i = 0; i < p_page_size; i += 4)
        { // calculate ECC if not all page data is FF's or there is a clean marker in OOB
            if (*(uint32_t *)(data + i) != 0xffffffff)
            {

        /********************************************************************************/
        /*                             GENERATE ECC_CODE                                */
        /********************************************************************************/

                if (SWAP_DATA_BEFORE_ECC_COMPUTATION)
                {
                    for (i = 0; i < p_partial_page_size; i+=4)
                    {
                        swap32(&data[i]);
                    }
                }

                for (subpage = 0; subpage < partial_pages_per_page; subpage++)
                {
                    swap(&data[(subpage + 1) * p_partial_page_size], &data[p_page_size + (subpage * p_oob_size)], p_oob_size); // temporarily swap OOB area with tail of data

                    if (t == 1)
                        nand_calculate_ecc_BRCM_Hamming(&data[subpage * p_partial_page_size], ecc_code);
                    else
                        nand_calculate_ecc_BRCM_bch(&data[subpage * p_partial_page_size], ecc_code, m, n, k, t, u, v);

                    swap(&data[(subpage + 1) * p_partial_page_size], &data[p_page_size + (subpage * p_oob_size)], p_oob_size); // swap back OOB area and tail of data

                    if (t == 1)
                        for(i = 0; i < HAMMING_SIZE; i++)
                        { /* Fill spare_area_out buffer with ECC data */
                            data[p_page_size + (subpage * p_oob_size) + i + HAMMING_SPARE_B0] = ecc_code[i];
                        }
                    else
                        for(i = 0; i < v; i++)
                        {
                            data[p_page_size + ((subpage + 1) * p_oob_size) - v + i] = ecc_code[i];
                        }
                }

                if (SWAP_DATA_AFTER_ECC_COMPUTATION)
                {
                    for (i = 0; i < p_page_size; i+=4)
                    {
                        swap32(&data[i]);
                    }
                }

                break;
            }
        }

        /* Write out the data buffer. */
        fwrite(data, p_page_size + OOB_SIZE + p_pad, 1, outfile_out);
        
        if (0 == strncmp(data + p_page_size - 0x100, ubifs_split_marker, sizeof(ubifs_split_marker)))
        {
            printf("found UBIFS\n");
            p_spare_jffs2_cleanmarker = 0;
        }

        page++;
        if (page >= p_pages_per_block)
            page = 0;
    } 

    fclose(infile);
    fclose(outfile_out);

} /* end of main */


/*--------------------------------------------------------------------------------- */
/* BRCM NAND Flesh Memory ECC Encoder (BCH Code) */
/*--------------------------------------------------------------------------------- */
/*  Description:    BCH (N, K, 1<=t<=12) Encoder                                    */
/*                                                                                  */
/*    1. generator polynomial                                                       */
/*      t = 1:     g(x) = 11011000000001                                            */
/*      t = 2:     g(x) = 110100101010100010101011001                               */
/*      t = 3:     g(x) = 1011011110111101010011011010111101011101                  */
/*      t = 4:     g(x) = 11010101011000011101010111000010000011000100101000101     */
/*      t = 5:     g(x) = 1101000111101011000000110101011010100000001111010010100   */
/*                        10110101111                                               */
/*      t = 6:     g(x) = 1011111010001101100111010011101100001111001001110000110   */
/*                        010010011001111001111111                                  */
/*      t = 7:     g(x) = 1010010110110100101100010110011111010000000111001011001   */
/*                        0110101100001000000010000000000000001                     */
/*      t = 8:     g(x) = 1100010011011111001000111010001110000010111000011100100   */
/*                        00011000011011110000001110010100010011111101010001        */
/*      t = 9:     g(x) = 1000011101100100011011010110011111001101100011010101001   */
/*                        1001110011101011110001010011111110111000010000101010100   */
/*                        01101101                                                  */
/*      t = 10:    g(x) = 1001001111101100110100000111111010101000000011010101000   */
/*                        0011110110111001001000101100100111110000101001011000011   */
/*                        110111001001011010011                                     */
/*      t = 11:    g(x) = 1101000011000000001011110100011011100100000110011100111   */
/*                        0111001011001011100011110001001010001001110000001111111   */
/*                        1010011001011011111000110100011001                        */
/*      t = 12:    g(x) = 1000101000000111101000000100100001111110101101100111011   */
/*                        0001100100101000000101001011001010010000111100110101001   */
/*                        01101010001000011010100100110011100001001001111           */
/*                                                                                  */
/*    2. primitive polynomial                                                       */
/*      p(x) = 11011000000001                                                       */
/*      p(x) = x^13 + x^4 + x^3 + x + 1                                             */
/*--------------------------------------------------------------------------------- */
static void nand_calculate_ecc_BRCM_bch(uint8_t *data,uint8_t *ecc_code,int m,int n,int k,int t,int u,int v)
{
    int           i;
    int           j;
    int           r;
    int           ff;
    int           msg[1 << MAX_FIELD_ORDER]; /* LSB first */
    int           cw[1 << MAX_FIELD_ORDER];  /* LSB first */
    int           *gx;       /* LSB first */

      /* contains generating polynomial gxM_T[] for field order = M and
       * correction capability = T */
#include "bch_gen_poly.h"

    /*------------------------------------------------------------------------------- */
    /* Construct msg in bit (LSB first) */
    /*------------------------------------------------------------------------------- */
    for (i=k-1;i>=0;i--)
        msg[i] = 0;

    r = 0;
    for (i=0;i<u;i++)
    {
        for (j=7;j>=0;j--)
        {
            msg[k-1-r] = (data[i]>>j)&0x01;
            r++;
        }
    }


    /*------------------------------------------------------------------------------- */
    /* Initialize generator polynomial */
    /*------------------------------------------------------------------------------- */
    if (m == 13) {
      switch (t)
      {
        case 1:  gx = gx13_1;  break;
        case 2:  gx = gx13_2;  break;
        case 3:  gx = gx13_3;  break;
        case 4:  gx = gx13_4;  break;
        case 5:  gx = gx13_5;  break;
        case 6:  gx = gx13_6;  break;
        case 7:  gx = gx13_7;  break;
        case 8:  gx = gx13_8;  break;
        case 9:  gx = gx13_9;  break;
        case 10:  gx = gx13_10;  break;
        case 11:  gx = gx13_11;  break;
        case 12:  gx = gx13_12;  break;
        case 13:  gx = gx13_13;  break;
        case 14:  gx = gx13_14;  break;
        case 15:  gx = gx13_15;  break;
        case 16:  gx = gx13_16;  break;
        case 17:  gx = gx13_17;  break;
        case 18:  gx = gx13_18;  break;
        case 19:  gx = gx13_19;  break;
        case 20:  gx = gx13_20;  break;
        case 21:  gx = gx13_21;  break;
        case 22:  gx = gx13_22;  break;
        case 23:  gx = gx13_23;  break;
        case 24:  gx = gx13_24;  break;
        case 25:  gx = gx13_25;  break;
        case 26:  gx = gx13_26;  break;
        case 27:  gx = gx13_27;  break;
        case 28:  gx = gx13_28;  break;
        case 29:  gx = gx13_29;  break;
        case 30:  gx = gx13_30;  break;
        case 31:  gx = gx13_31;  break;
        case 32:  gx = gx13_32;  break;
        case 33:  gx = gx13_33;  break;
        case 34:  gx = gx13_34;  break;
        case 35:  gx = gx13_35;  break;
        case 36:  gx = gx13_36;  break;
        case 37:  gx = gx13_37;  break;
        case 38:  gx = gx13_38;  break;
        case 39:  gx = gx13_39;  break;
        case 40:  gx = gx13_40;  break;
        default: fprintf(stderr,"Field order = %d, t = %d is not supported\n",m,t); exit(1); break;
      }
    } else if (m == 14) {
      switch (t)
      {
        case 1:  gx = gx14_1;  break;
        case 2:  gx = gx14_2;  break;
        case 3:  gx = gx14_3;  break;
        case 4:  gx = gx14_4;  break;
        case 5:  gx = gx14_5;  break;
        case 6:  gx = gx14_6;  break;
        case 7:  gx = gx14_7;  break;
        case 8:  gx = gx14_8;  break;
        case 9:  gx = gx14_9;  break;
        case 10:  gx = gx14_10;  break;
        case 11:  gx = gx14_11;  break;
        case 12:  gx = gx14_12;  break;
        case 13:  gx = gx14_13;  break;
        case 14:  gx = gx14_14;  break;
        case 15:  gx = gx14_15;  break;
        case 16:  gx = gx14_16;  break;
        case 17:  gx = gx14_17;  break;
        case 18:  gx = gx14_18;  break;
        case 19:  gx = gx14_19;  break;
        case 20:  gx = gx14_20;  break;
        case 21:  gx = gx14_21;  break;
        case 22:  gx = gx14_22;  break;
        case 23:  gx = gx14_23;  break;
        case 24:  gx = gx14_24;  break;
        case 25:  gx = gx14_25;  break;
        case 26:  gx = gx14_26;  break;
        case 27:  gx = gx14_27;  break;
        case 28:  gx = gx14_28;  break;
        case 29:  gx = gx14_29;  break;
        case 30:  gx = gx14_30;  break;
        case 31:  gx = gx14_31;  break;
        case 32:  gx = gx14_32;  break;
        case 33:  gx = gx14_33;  break;
        case 34:  gx = gx14_34;  break;
        case 35:  gx = gx14_35;  break;
        case 36:  gx = gx14_36;  break;
        case 37:  gx = gx14_37;  break;
        case 38:  gx = gx14_38;  break;
        case 39:  gx = gx14_39;  break;
        case 40:  gx = gx14_40;  break;
        default: fprintf(stderr,"Field order = %d, t = %d is not supported\n",m,t); exit(1); break;
      }
    } else {
      fprintf(stderr,"Field order = %d, t = %d is not supported\n",m,t); 
      exit(1); 
    }


    /*------------------------------------------------------------------------------- */
    /* Encoding */
    /*------------------------------------------------------------------------------- */
    for (i=0;i<n;i++)
    {
        if (i<n-k)
            cw[i] = 0;
        else
            cw[i] = msg[i-(n-k)];
    }

    for (i=k-1;i>=0;i--)
    {
        ff = cw[n-k-1]^msg[i];
        for (j=n-k-1;j>=1;j--)
            cw[j] = cw[j-1]^(ff&gx[j]);
        cw[0] = ff&gx[0];
    }

    for (i=0;i<v;i++)
    {
        ecc_code[i] = 128*cw[8*(v-i)-1] + 64*cw[8*(v-i)-2] + 32*cw[8*(v-i)-3] + 16*cw[8*(v-i)-4]+
            8*cw[8*(v-i)-5] +  4*cw[8*(v-i)-6] +  2*cw[8*(v-i)-7] +  1*cw[8*(v-i)-8];
    }

}

static void swap32(uint8_t *data)
{
    uint8_t tmp;

    tmp=data[3];
    data[3]=data[0];
    data[0]=tmp;
    tmp=data[2];
    data[2]=data[1];
    data[1]=tmp;    
}

/* swap buffer for another */
static void swap(uint8_t *first, uint8_t *second, int amount)
{
    uint8_t temp;

    while(amount--)
    {
        temp = *first;
        *first++ = *second;
        *second++ = temp;
    }
}

/********************************************************************************/
/* Begin support for JFFS2 cleanmarker in spare area and 1 bit Hamming ECC     */
/********************************************************************************/

static void calc_spare_layout(int level, int spare_size, int page_size, uint8_t *pcm)
{

//    const uint8_t cleanmarker[] = {0x19, 0x85, 0x20, 0x03, 0x00, 0x00, 0x00, 0x08};
//    const uint8_t cleanmarker[] = {0x85, 0x19, 0x03, 0x20, 0x08, 0x00, 0x00, 0x00};
    int i, j;
    PSPARE_LAYOUT sl;
    uint32_t layout_parms = LAYOUT_PARMS(level, spare_size, page_size);

    switch(layout_parms)
    {
    case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,16,512)):
        sl = &brcmnand_oob_16;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,16,512)):
        sl = &brcmnand_oob_bch4_512;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,64,2048)):
        sl = &brcmnand_oob_bch4_2k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,64,2048)):
        sl = &brcmnand_oob_bch8_16_2k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,64,2048)):
        sl = &brcmnand_oob_64;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,128,4096)):
        sl = &brcmnand_oob_bch4_4k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,128,4096)):
        sl = &brcmnand_oob_bch8_16_4k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,108,2048)):
        sl = &brcmnand_oob_bch8_27_2k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,216,4096)):
        sl = &brcmnand_oob_bch8_27_4k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,216,4096)):
        sl = &brcmnand_oob_bch12_27_4k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_HAMMING,128,4096)):
        sl = &brcmnand_oob_128;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_4,256,8192)):
        sl = &brcmnand_oob_bch4_8k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,256,8192)):
        sl = &brcmnand_oob_bch8_16_8k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_8,432,8192)):
        sl = &brcmnand_oob_bch8_27_8k;
        break;

    case (LAYOUT_PARMS(NAC_ECC_LVL_BCH_12,432,8192)):
        sl = &brcmnand_oob_bch12_27_8k;
        break;

    default:
        printf("No valid spare layout for level=%lu, spare size=%lu,"
            " page size=%lu\n", level, spare_size, page_size);

        /* Change to a disabled spare layout. */
        sl = &brcmnand_oob_bch12_27_8k;
        for( i = 0; i < spare_size / 8; i++ )
            sl->sl_spare_mask[i] = 0;
        break;
    }

    /* Skip spare area offsets reserved for ECC and BI bytes. */
    for( i = 0, j = 0; i < spare_size; i++ )
    {
        if( ECC_MASK_BIT(sl->sl_spare_mask, i) == 0 && j < CLEANMARKER_SIZE)
            pcm[i] = cleanmarker[j++];
        else
            pcm[i] = 0xff;
    }
}

static void  nand_calculate_ecc_BRCM_Hamming(const uint8_t *data, uint8_t *ecc_code)
{
    int i,j;
    static uint8_t o_ecc[24], temp[10];
    static uint32_t b_din[512/4];
    uint32_t* i_din = &b_din[0];
    unsigned long pre_ecc;

    memcpy((uint8_t*) i_din, data, 512);
    memset(o_ecc, 0, sizeof(o_ecc));
    for(i=0;i<512/4;i++)
    {
        i_din[i] = htonl(i_din[i]);

        memset(temp, 0, sizeof(temp));
        for(j=0;j<32;j++)
        {
            temp[0]^=((i_din[i]& 0x55555555)>>j)&0x1;
            temp[1]^=((i_din[i]& 0xAAAAAAAA)>>j)&0x1;
            temp[2]^=((i_din[i]& 0x33333333)>>j)&0x1;
            temp[3]^=((i_din[i]& 0xCCCCCCCC)>>j)&0x1;
            temp[4]^=((i_din[i]& 0x0F0F0F0F)>>j)&0x1;
            temp[5]^=((i_din[i]& 0xF0F0F0F0)>>j)&0x1;
            temp[6]^=((i_din[i]& 0x00FF00FF)>>j)&0x1;
            temp[7]^=((i_din[i]& 0xFF00FF00)>>j)&0x1;
            temp[8]^=((i_din[i]& 0x0000FFFF)>>j)&0x1;
            temp[9]^=((i_din[i]& 0xFFFF0000)>>j)&0x1;
        }
        for(j=0;j<10;j++)
            o_ecc[j]^=temp[j];
        if(i%2)
        {
            for(j=0;j<32;j++)
                o_ecc[11]^=(i_din[i]>>j)&0x1;//P32
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[10]^=(i_din[i]>>j)&0x1;//P32'
        }
        if((i&0x3)<2)
        {
            for(j=0;j<32;j++)
                o_ecc[12]^=(i_din[i]>>j)&0x1;//P64'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[13]^=(i_din[i]>>j)&0x1;//P64
        }
        if((i&0x7)<4)
        {
            for(j=0;j<32;j++)
                o_ecc[14]^=(i_din[i]>>j)&0x1;//P128'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[15]^=(i_din[i]>>j)&0x1;//P128
        }
        if((i&0xF)<8)
        {
            for(j=0;j<32;j++)
                o_ecc[16]^=(i_din[i]>>j)&0x1;//P256'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[17]^=(i_din[i]>>j)&0x1;//P256
        }
        if((i&0x1F)<16)
        {
            for(j=0;j<32;j++)
                o_ecc[18]^=(i_din[i]>>j)&0x1;//P512'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[19]^=(i_din[i]>>j)&0x1;//P512
        }
        if((i&0x3F)<32)
        {
            for(j=0;j<32;j++)
                o_ecc[20]^=(i_din[i]>>j)&0x1;//P1024'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[21]^=(i_din[i]>>j)&0x1;//P1024
        }
        if((i&0x7F)<64)
        {
            for(j=0;j<32;j++)
                o_ecc[22]^=(i_din[i]>>j)&0x1;//P2048'
        }
        else
        {
            for(j=0;j<32;j++)
                o_ecc[23]^=(i_din[i]>>j)&0x1;//P2048
        }
        // print intermediate value
        pre_ecc = 0;
        for(j=23;j>=0;j--)
        {
            pre_ecc = (pre_ecc << 1) | (o_ecc[j] ? 1 : 0 ); 
        }
        //        printf( "pre_ecc[%d] = 0x%06.6x\n", i, pre_ecc );
    }

    ecc_code[0] = (o_ecc[ 7]<<7 | o_ecc[ 6]<<6 | o_ecc[ 5]<<5 | o_ecc[ 4]<<4
        | o_ecc[ 3]<<3 | o_ecc[ 2]<<2 | o_ecc[ 1]<<1 | o_ecc[ 0]);
    ecc_code[1] = (o_ecc[15]<<7 | o_ecc[14]<<6 | o_ecc[13]<<5 | o_ecc[12]<<4
        | o_ecc[11]<<3 | o_ecc[10]<<2 | o_ecc[ 9]<<1 | o_ecc[ 8]);
    ecc_code[2] = (o_ecc[23]<<7 | o_ecc[22]<<6 | o_ecc[21]<<5 | o_ecc[20]<<4
        | o_ecc[19]<<3 | o_ecc[18]<<2 | o_ecc[17]<<1 | o_ecc[16]);
}

/********************************************************************************/
/* End support for JFFS2 cleanmarker in spare area and 1 bit Hamming ECC       */
/********************************************************************************/

