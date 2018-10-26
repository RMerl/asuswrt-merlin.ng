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
/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the definitions for Broadcom's 6838 Data path           */
/* initialization sequence                                                    */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifndef __BCM63138_DATA_PATH_INIT_H
#define __BCM63138_DATA_PATH_INIT_H

typedef enum
{
    WAN_TYPE_NONE = 0,
    WAN_TYPE_RGMII = 1,
    WAN_TYPE_DSL = 5
}E_WAN_TYPE;


typedef enum
{
    DPI_RC_OK,
    DPI_RC_ERROR
}E_DPI_RC;


typedef struct
{
    uint32_t mtu_size;
    uint32_t headroom_size;
    uint32_t runner_freq;
    uint32_t runner_tm_base_addr;
    uint32_t runner_tm_base_addr_phys;
    uint32_t runner_tm_size;
    uint32_t runner_mc_base_addr;
    uint32_t runner_mc_base_addr_phys;
    uint32_t runner_mc_size;
    uint32_t runner_lp;
} S_DPI_CFG;

uint32_t data_path_init(S_DPI_CFG *pCfg);
uint32_t data_path_go(void);
uint32_t data_path_shutdown(void);
void f_configure_bridge_port_sa_da(void);
void reset_unreset_rdp_block(void);


#endif //#define __BCM63138_DATA_PATH_INIT_H
