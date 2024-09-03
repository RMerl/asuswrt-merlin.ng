/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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


#include "xrdp_drv_drivers_common_ag.h"


/* get_test_method_value */
uint32_t gtmv(bdmf_test_method method, uint8_t bits)
{
    switch (method)
    {
    case bdmf_test_method_mid:
        return bits == 0 ? 0 : 1 << (bits - 1);
    case bdmf_test_method_high:
        /* todo need to fix this, the problem was that ((1<<32)-1) = 0 ???, it should be 0xFFFFFFFF */
        return bits >= 32 ? UINT32_MAX : (1 << bits) - 1;
    case bdmf_test_method_low:
    default :
        return 0;
    }
}

