// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
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
