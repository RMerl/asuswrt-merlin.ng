/*
<:copyright-BRCM:2016:DUAL/GPL:standard

   Copyright (c) 2016 Broadcom 
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


#ifndef WAN_TYPES_H_INCLUDED
#define WAN_TYPES_H_INCLUDED

#include <linux/types.h>


typedef enum
{
    SUPPORTED_WAN_TYPES_AUTO_SENSE_UNAVAILABLE = (    0),
    SUPPORTED_WAN_TYPES_BIT_GPON               = (1<< 0),
    SUPPORTED_WAN_TYPES_BIT_EPON_1_1           = (1<< 1),
    SUPPORTED_WAN_TYPES_BIT_TURBO_EPON_2_1     = (1<< 2),
    SUPPORTED_WAN_TYPES_BIT_EPON_10_1          = (1<< 3),
    SUPPORTED_WAN_TYPES_BIT_EPON_10_10         = (1<< 4),
    SUPPORTED_WAN_TYPES_BIT_AE_1_1             = (1<< 5),
    SUPPORTED_WAN_TYPES_BIT_AE_10_10           = (1<< 6),
    SUPPORTED_WAN_TYPES_BIT_XGPON              = (1<< 7),
    SUPPORTED_WAN_TYPES_BIT_NGPON2_10_25       = (1<< 8),
    SUPPORTED_WAN_TYPES_BIT_NGPON2_10_10       = (1<< 9),
    SUPPORTED_WAN_TYPES_BIT_XGSPON             = (1<<10),
} SUPPORTED_WAN_TYPES;

typedef uint32_t SUPPORTED_WAN_TYPES_BITMAP;


typedef enum
{
    PMD_EPON_1_1_WAN,
    PMD_EPON_2_1_WAN,
    PMD_GPON_2_1_WAN,
    PMD_GBE_1_1_WAN,
    PMD_XGPON1_10_2_WAN,
    PMD_EPON_10_1_WAN,
    PMD_AUTO_DETECT_WAN, /* gpon, epon auto detect */
} PMD_WAN_TYPES;


typedef void (*get_non_brcm_pmd_supported_wan_type_bm)(SUPPORTED_WAN_TYPES_BITMAP *wan_type_bm);
typedef void (*configure_non_brcm_pmd_wan_type)(PMD_WAN_TYPES new_pmd_wan_type);

#endif /* WAN_TYPES_H_INCLUDED */
