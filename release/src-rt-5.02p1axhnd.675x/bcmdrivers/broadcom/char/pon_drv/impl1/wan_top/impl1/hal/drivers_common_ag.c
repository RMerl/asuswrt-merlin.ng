/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "drivers_common_ag.h"


/* get_test_method_value */
uint32_t gtmv(bdmf_test_method method, uint8_t bits)
{
    switch (method)
    {
    case bdmf_test_method_mid :
        return 1 << (bits - 1);
    case bdmf_test_method_high :
        /* todo need to fix this, the problem was that ((1<<32)-1)=0 ???, it should be 0xFFFFFFFF */
        return bits == 32 ? UINT32_MAX : (1 << bits) - 1;
    case bdmf_test_method_low :
    default :
        return 0;
    }
    return 0;
}

