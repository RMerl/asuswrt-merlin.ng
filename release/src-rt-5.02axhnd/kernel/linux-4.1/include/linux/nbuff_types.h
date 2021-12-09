#if defined(CONFIG_BCM_KF_NBUFF)
#ifndef __NBUFF_TYPES_H_INCLUDED__
#define __NBUFF_TYPES_H_INCLUDED__

/*
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
*/

/*
 *******************************************************************************
 *
 * File Name  : nbuff_types.h
 * Description: Simple nbuff type defines.
 *
 ******************************************************************************* */

#define MUST_BE_ZERO                0

/* virtual network buffer pointer to SKB|FPB|TGB|FKB  */
typedef void * pNBuff_t;
#define PNBUFF_NULL                 ((pNBuff_t)NULL)

typedef enum NBuffPtrType
{
    SKBUFF_PTR = MUST_BE_ZERO,      /* Default Linux networking socket buffer */
    FPBUFF_PTR,                     /* Experimental BRCM IuDMA freepool buffer*/
    TGBUFF_PTR,                     /* LAB Traffic generated network buffer   */
    FKBUFF_PTR,                     /* Lightweight fast kernel network buffer */
    /* Do not add new ptr types */
} NBuffPtrType_t;

                                    /* 2lsbits in pointer encode NbuffType_t  */
#define NBUFF_TYPE_MASK             0x3ul
#define NBUFF_PTR_MASK              (~NBUFF_TYPE_MASK)
#define NBUFF_PTR_TYPE(pNBuff)      ((uintptr_t)(pNBuff) & NBUFF_TYPE_MASK)


#define IS_SKBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == SKBUFF_PTR )
#define IS_FPBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == FPBUFF_PTR )
#define IS_TGBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == TGBUFF_PTR )
#define IS_FKBUFF_PTR(pNBuff)       ( NBUFF_PTR_TYPE(pNBuff) == FKBUFF_PTR )


#endif  /* defined(__NBUFF_TYPES_H_INCLUDED__) */
#endif
