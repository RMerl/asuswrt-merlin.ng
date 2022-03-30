// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "xrdp_drv_drivers_common_ag.h"


/* get_test_method_value */
uint32_t gtmv(bdmf_test_method method, uint8_t bits)
{
    switch (method)
    {
    case bdmf_test_method_mid :
        return 1 << (bits - 1);
    case bdmf_test_method_high :
        /* todo need to fix this, the problem was that ((1<<32)-1)=0 ???, it should be 0xFFFFFFFF */
        return bits == 32 ? _32BITS_MAX_VAL_ : (1 << bits) - 1;
    case bdmf_test_method_low :
    default :
        return 0;
    }
    return 0;
}

