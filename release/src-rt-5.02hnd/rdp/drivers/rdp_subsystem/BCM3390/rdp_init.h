/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
#ifndef _RDP_INIT_H_
#define _RDP_INIT_H_

typedef struct
{
    uint32_t rdp_ddr0_pkt_base;           /* fpm pool DDR0 base address for unicast packets */
    uint32_t rdp_ddr1_pkt_base;           /* fpm pool DDR1 base address for unicast packets */
    uint32_t rdp_ddr0_mc_base;            /* fpm pool DDR0 base address for multicast packets */
    uint32_t rdp_ddr1_mc_base;            /* fpm pool DDR1 base address for multicast packets */
    uint32_t rdp_runner_tables_ddr_ptr;   /* runner tables DDR base address*/
    uint32_t runner_freq;                 /* rdp block clock */
    uint8_t rdp_byoi_enabled;             /* is BYOI enabled */
    uint8_t cmim_pass_through_enabled;    /* is CMIM pass through enabled */
} rdp_init_params;

void rdp_get_init_params(rdp_init_params *init_params);
int rdpa_post_init(void);

#endif