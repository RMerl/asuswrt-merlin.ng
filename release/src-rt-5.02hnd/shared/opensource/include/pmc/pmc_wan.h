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
 * pmc_wan.h
 *
 *  Created on: Nov 30 2015
 *      Author: yonatani
 */


#ifndef _PMC_WAN_H_
#define _PMC_WAN_H_

#include <bcmtypes.h>


typedef union
{
    struct
    {
#ifdef PMC_LITTLE_ENDIAN
    uint32_t wan_main_rst_n:1;
    uint32_t wan_top_bb_rst_n:1;
    uint32_t epon_core_rst_n:1;
    uint32_t epon_rx_rclk16_sw_reset_n:1;
    uint32_t epon_rx_rbc125_sw_reset_n:1;
    uint32_t epon_tx_tclk16_sw_reset_n:1;
    uint32_t epon_tx_clk125_sw_reset_n:1;
    uint32_t gpon_main_rst_n:1;
    uint32_t gpon_rx_rst_n:1;
    uint32_t gpon_tx_rst_n:1;
    uint32_t gpon_8khz_rst_n:1;
    uint32_t ngpon_main_rst_n:1;
    uint32_t ngpon_rx_rst_n:1;
    uint32_t ngpon_tx_rst_n:1;
    uint32_t ngpon_8khz_rst_n:1;
    uint32_t gpon_nco_rst_n:1;
    uint32_t apm_rst_n:1;
    uint32_t reserved:15;
#else
    uint32_t reserved:15;
    uint32_t apm_rst_n:1;
    uint32_t gpon_nco_rst_n:1;
    uint32_t ngpon_8khz_rst_n:1;
    uint32_t ngpon_tx_rst_n:1;
    uint32_t ngpon_rx_rst_n:1;
    uint32_t ngpon_main_rst_n:1;
    uint32_t gpon_8khz_rst_n:1;
    uint32_t gpon_tx_rst_n:1;
    uint32_t gpon_rx_rst_n:1;
    uint32_t gpon_main_rst_n:1;
    uint32_t epon_tx_clk125_sw_reset_n:1;
    uint32_t epon_tx_tclk16_sw_reset_n:1;
    uint32_t epon_rx_rbc125_sw_reset_n:1;
    uint32_t epon_rx_rclk16_sw_reset_n:1;
    uint32_t epon_core_rst_n:1;
    uint32_t wan_top_bb_rst_n:1;
    uint32_t wan_main_rst_n:1;
#endif
    }Bits;
    uint32_t Reg32;
}WAN_TOP_BPCM_SRESET_REG;

int pmc_wan_init(void);
#endif /* _PMC_WAN_H_ */
