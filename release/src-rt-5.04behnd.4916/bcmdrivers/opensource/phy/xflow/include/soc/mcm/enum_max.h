/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2012:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 *
 ************************************************************************/

#ifndef _SOC_ENUM_MAX_H
#define _SOC_ENUM_MAX_H

#include "macsec_macros.h"

/****************************************************************
 *
 * BCM Chip Support options.
 *
 * If NO_BCM_<chip> is defined, do not define BCM_<chip>.  This
 * avoids adding support for the chip.  NO_BCM_<chip> can be added
 * to Make.local.
 ****************************************************************/

/* Some basic definitions */
#define SOC_BLOCK_MSB_BP                30
#define SOC_BLOCK_BP                    20
#define SOC_MEMOFS_BP                   16
#define SOC_REGIDX_BP                   12
#define SOC_RT_BP                       25

/* Maximum values across all chips */
#define SOC_NUM_SUPPORTED_CHIPS         79
#define SOC_MAX_NUM_COS                 48

/* defaults until maximized below */
#define SOC_MAX_NUM_PIPES               0       /* max 8 */
#define SOC_MAX_NUM_BLKS                0       /* max 326 */
#define SOC_MAX_NUM_PORTS               0       /* max 1280 */
#define SOC_MAX_NUM_PP_PORTS            0       /* max 170 */
#define SOC_MAX_MEM_BYTES               0       /* max 653 */

#if     1 > SOC_MAX_NUM_PIPES
#undef  SOC_MAX_NUM_PIPES
#define SOC_MAX_NUM_PIPES               1
#endif

#if     30 > SOC_MAX_NUM_BLKS
#undef  SOC_MAX_NUM_BLKS
#define SOC_MAX_NUM_BLKS                30
#endif

#if     78 > SOC_MAX_NUM_PORTS
#undef  SOC_MAX_NUM_PORTS
#define SOC_MAX_NUM_PORTS               78
#endif

#if     -1 > SOC_MAX_NUM_PP_PORTS
#undef  SOC_MAX_NUM_PP_PORTS
#define SOC_MAX_NUM_PP_PORTS            -1
#endif

#if     65 > SOC_MAX_MEM_BYTES
#undef  SOC_MAX_MEM_BYTES
#define SOC_MAX_MEM_BYTES               65
#endif

#define SOC_MAX_MEM_FIELD_NUM           317
#define SOC_MAX_REG_FIELD_BITS          640
#define SOC_MAX_MEM_FIELD_BITS          4250
#define SOC_MAX_FORMAT_FIELD_BITS       16272
#define SOC_MAX_FORMAT_FIELD_NUM        225

#if     SOC_MAX_REG_FIELD_BITS > SOC_MAX_MEM_FIELD_BITS
#define SOC_MAX_FIELD_BITS              SOC_MAX_REG_FIELD_BITS
#else
#define SOC_MAX_FIELD_BITS              SOC_MAX_MEM_FIELD_BITS
#endif
#define SOC_MAX_MEM_WORDS               BYTES2WORDS(SOC_MAX_MEM_BYTES)
#define SOC_MAX_FORMAT_WORDS            BITS2WORDS(SOC_MAX_FORMAT_FIELD_BITS)
#define SOC_MAX_REG_FIELD_WORDS         BITS2WORDS(SOC_MAX_REG_FIELD_BITS)
#define SOC_MAX_MEM_FIELD_WORDS         BITS2WORDS(SOC_MAX_MEM_FIELD_BITS)
#define SOC_MAX_FIELD_WORDS             BITS2WORDS(SOC_MAX_FIELD_BITS)
#define SOC_MAX_FORMAT_FIELD_WORDS      BITS2WORDS(SOC_MAX_FORMAT_FIELD_BITS)

#endif  /* !_SOC_ENUM_MAX_H */
