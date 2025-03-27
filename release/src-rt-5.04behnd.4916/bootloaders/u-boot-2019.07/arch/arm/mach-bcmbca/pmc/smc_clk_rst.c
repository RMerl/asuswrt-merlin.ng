/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/

#include "clk_rst.h"
#include "clk_rpc_svc.h"

int get_xrdp_freq(unsigned int *freq)
{
    return bcm_rpc_clk_get_rate("xrdp_pll", CLK_DOMAIN_NAME_MAX_LEN, freq);
}

unsigned long get_rdp_freq(unsigned int *freq)
{
    return bcm_rpc_clk_get_rate("rdp_pll", CLK_DOMAIN_NAME_MAX_LEN, freq);
}

int biu_ch_freq_get(unsigned int ch, unsigned int *freq)
{
    int ret = 0;
    char *ch_names[] = {"armcore0"}; 
    if (ch > 0 )
        return -1;

    ret = bcm_rpc_clk_get_rate(ch_names[ch], CLK_DOMAIN_NAME_MAX_LEN, freq);
    return ret;
}

