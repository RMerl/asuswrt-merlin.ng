/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
