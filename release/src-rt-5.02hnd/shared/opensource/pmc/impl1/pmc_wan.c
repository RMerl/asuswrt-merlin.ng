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
 * pmc_wan.c
 *
 *  Created on: Nov 30 2015
 *      Author: yonatani
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "pmc_drv.h"
#include "pmc_wan.h"
#include "BPCM.h"

int pmc_wan_init(void)
{
    int status;
    WAN_TOP_BPCM_SRESET_REG sreset;

    status =  PowerOnDevice(PMB_ADDR_WAN);

    //take pins out of reset
    // can be modified later for smaller granularity

    sreset.Bits.wan_main_rst_n = 0;
    sreset.Bits.wan_top_bb_rst_n = 0;
    sreset.Bits.epon_core_rst_n = 0;
    sreset.Bits.epon_rx_rclk16_sw_reset_n = 0;
    sreset.Bits.epon_rx_rbc125_sw_reset_n = 0;
    sreset.Bits.epon_tx_tclk16_sw_reset_n = 0;
    sreset.Bits.epon_tx_clk125_sw_reset_n = 0;
    sreset.Bits.gpon_main_rst_n = 0;
    sreset.Bits.gpon_rx_rst_n = 0;
    sreset.Bits.gpon_tx_rst_n = 0;
    sreset.Bits.gpon_8khz_rst_n = 0;
    sreset.Bits.ngpon_main_rst_n = 0;
    sreset.Bits.ngpon_rx_rst_n = 0;
    sreset.Bits.ngpon_tx_rst_n = 0;
    sreset.Bits.ngpon_8khz_rst_n = 0;
    sreset.Bits.gpon_nco_rst_n = 0;
    sreset.Bits.apm_rst_n = 0;

    status = WriteBPCMRegister(PMB_ADDR_WAN, BPCMRegOffset(sr_control), sreset.Reg32);

    return status;
}
EXPORT_SYMBOL(pmc_wan_init);
postcore_initcall(pmc_wan_init);

