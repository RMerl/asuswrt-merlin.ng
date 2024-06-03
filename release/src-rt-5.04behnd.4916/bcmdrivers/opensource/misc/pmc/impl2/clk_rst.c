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
#include <linux/kernel.h>
#include <linux/module.h>
#include "clk_rpc_svc.h"

int rdppll_vco_freq_get(unsigned int pll_addr, unsigned int *fvco)
{
    int ret = 0;

    ret = bcm_rpc_clk_get_rate("pllvco", CLK_DOMAIN_NAME_MAX_LEN, fvco);
    return ret;
}

#define PLL_NUM_MAX  (7)

int rdppll_ch_freq_get(unsigned int pll_addr, unsigned int ch, unsigned int *freq)
{
    int ret = 0;
    char pll_domain_name[PLL_NUM_MAX][CLK_DOMAIN_NAME_MAX_LEN] = 
               { "pllsys0", "pllsys1", "pllsys2", "pllsys3", "pllsys4", "pllsys5", "pllsys6"};

   if (ch >= PLL_NUM_MAX)
   {
        printk("%s: ERROR: ch[%d] > PLL_NUM_MAX[4]\n",__FUNCTION__, ch);
        return -1;
   }

   ret = bcm_rpc_clk_get_rate(pll_domain_name[ch], CLK_DOMAIN_NAME_MAX_LEN, freq);

   return ret;
}

unsigned long get_rdp_freq(unsigned int *rdp_freq)
{
    int ret = 0;

   ret = bcm_rpc_clk_get_rate("rdp_pll", CLK_DOMAIN_NAME_MAX_LEN, rdp_freq);
   return ret;
}
EXPORT_SYMBOL(get_rdp_freq);


int pll_ch_freq_set_offs(unsigned int pll_addr, unsigned int ch, unsigned int mdiv, unsigned int offset)
{
   return 0;
}

void set_vreg_clk(void)
{
    return;
}

void bcm_set_vreg_sync(void)
{
    return;
}
EXPORT_SYMBOL(bcm_set_vreg_sync);

void set_spike_mitigation(unsigned int spike_us)
{
    return;
}
