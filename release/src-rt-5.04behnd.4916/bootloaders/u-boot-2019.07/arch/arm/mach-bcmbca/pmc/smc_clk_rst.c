/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
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

