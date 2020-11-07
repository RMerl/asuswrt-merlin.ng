/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

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
 ------------------------------------------------------------------------- */
#ifndef DEV_ID_H__
#define DEV_ID_H__

/**
 * m = memory, c = core, r = register, f = field, d = data.
 */
#if !defined(GET_FIELD) && !defined(SET_FIELD)
#define BRCM_ALIGN(c,r,f)   c##_##r##_##f##_ALIGN
#define BRCM_BITS(c,r,f)    c##_##r##_##f##_BITS
#define BRCM_MASK(c,r,f)    c##_##r##_##f##_MASK
#define BRCM_SHIFT(c,r,f)   c##_##r##_##f##_SHIFT

#define GET_FIELD(m,c,r,f) \
	((((m) & BRCM_MASK(c,r,f)) >> BRCM_SHIFT(c,r,f)) << BRCM_ALIGN(c,r,f))

#define SET_FIELD(m,c,r,f,d) \
	((m) = (((m) & ~BRCM_MASK(c,r,f)) | ((((d) >> BRCM_ALIGN(c,r,f)) << \
	 BRCM_SHIFT(c,r,f)) & BRCM_MASK(c,r,f))) \
	)

#define SET_TYPE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##d)
#define SET_NAME_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,c##_##r##_##f##_##d)
#define SET_VALUE_FIELD(m,c,r,f,d) SET_FIELD(m,c,r,f,d)

#endif /* GET & SET */

/***************************************************************************
 *registers
 ***************************************************************************/
#define DEV_ID_DEV_ID                            0x00000000 /* Device ID field */
#define DEV_ID_REV_ID                            0x00000004 /* Revision ID field */
#define DEV_ID_FAM_ID                            0x00000008 /* Family ID field */

/***************************************************************************
 *DEV_ID - Device ID field
 ***************************************************************************/
/* DEV_ID :: DEV_ID :: DEV_ID [31:00] */
#define DEV_ID_DEV_ID_DEV_ID_MASK                                  0xffffffff
#define DEV_ID_DEV_ID_DEV_ID_ALIGN                                 0
#define DEV_ID_DEV_ID_DEV_ID_BITS                                  32
#define DEV_ID_DEV_ID_DEV_ID_SHIFT                                 0
#define DEV_ID_DEV_ID_DEV_ID_DEFAULT                               1754267664

/***************************************************************************
 *REV_ID - Revision ID field
 ***************************************************************************/
/* DEV_ID :: REV_ID :: reserved0 [31:08] */
#define DEV_ID_REV_ID_reserved0_MASK                               0xffffff00
#define DEV_ID_REV_ID_reserved0_ALIGN                              0
#define DEV_ID_REV_ID_reserved0_BITS                               24
#define DEV_ID_REV_ID_reserved0_SHIFT                              8

/* DEV_ID :: REV_ID :: REV_ID [07:00] */
#define DEV_ID_REV_ID_REV_ID_MASK                                  0x000000ff
#define DEV_ID_REV_ID_REV_ID_ALIGN                                 0
#define DEV_ID_REV_ID_REV_ID_BITS                                  8
#define DEV_ID_REV_ID_REV_ID_SHIFT                                 0
#define DEV_ID_REV_ID_REV_ID_DEFAULT                               0

/***************************************************************************
 *FAM_ID - Family ID field
 ***************************************************************************/
/* DEV_ID :: FAM_ID :: FAMILY_ID [31:12] */
#define DEV_ID_FAM_ID_FAMILY_ID_MASK                               0xfffff000
#define DEV_ID_FAM_ID_FAMILY_ID_ALIGN                              0
#define DEV_ID_FAM_ID_FAMILY_ID_BITS                               20
#define DEV_ID_FAM_ID_FAMILY_ID_SHIFT                              12
#define DEV_ID_FAM_ID_FAMILY_ID_DEFAULT                            428289

/* DEV_ID :: FAM_ID :: RSV_ID [11:08] */
#define DEV_ID_FAM_ID_RSV_ID_MASK                                  0x00000f00
#define DEV_ID_FAM_ID_RSV_ID_ALIGN                                 0
#define DEV_ID_FAM_ID_RSV_ID_BITS                                  4
#define DEV_ID_FAM_ID_RSV_ID_SHIFT                                 8
#define DEV_ID_FAM_ID_RSV_ID_DEFAULT                               0

/* DEV_ID :: FAM_ID :: MAJ_REV_ID [07:04] */
#define DEV_ID_FAM_ID_MAJ_REV_ID_MASK                              0x000000f0
#define DEV_ID_FAM_ID_MAJ_REV_ID_ALIGN                             0
#define DEV_ID_FAM_ID_MAJ_REV_ID_BITS                              4
#define DEV_ID_FAM_ID_MAJ_REV_ID_SHIFT                             4
#define DEV_ID_FAM_ID_MAJ_REV_ID_DEFAULT                           1

/* DEV_ID :: FAM_ID :: MIN_REV_ID [03:00] */
#define DEV_ID_FAM_ID_MIN_REV_ID_MASK                              0x0000000f
#define DEV_ID_FAM_ID_MIN_REV_ID_ALIGN                             0
#define DEV_ID_FAM_ID_MIN_REV_ID_BITS                              4
#define DEV_ID_FAM_ID_MIN_REV_ID_SHIFT                             0
#define DEV_ID_FAM_ID_MIN_REV_ID_DEFAULT                           0

#endif /* #ifndef DEV_ID_H__ */

/* End of File */
