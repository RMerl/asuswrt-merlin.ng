/*
   <:copyright-BRCM:2014:DUAL/GPL:standard
   
      Copyright (c) 2014 Broadcom 
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
/* This file contains the definitions for Broadcom's 6838 Data path		  	  */
/* initialization sequence											          */
/*                                                                            */
/******************************************************************************/

#ifndef __OREN_DATA_PATH_INIT_H
#define __OREN_DATA_PATH_INIT_H

#include <rdp_drv_bbh.h>

typedef enum
{
	DPI_RC_OK,
	DPI_RC_ERROR
} E_DPI_RC;

typedef struct
{
	uint32_t wan_bbh;
	uint32_t mtu_size;
	uint32_t headroom_size;
	uint32_t car_mode;
	uint32_t ip_class_method;
	uint32_t bridge_fc_mode;
	uint32_t runner_freq;
	uint32_t enabled_port_map;
	uint32_t g9991_debug_port;
	uint32_t us_ddr_queue_enable;
	uint32_t runner_tm_base_addr;
	uint32_t runner_tm_base_addr_phys;
	uint32_t runner_mc_base_addr;
	uint32_t runner_mc_base_addr_phys;
	uint32_t bpm_buffer_size;
	uint32_t bpm_buffers_number;
	uint32_t wan_phy_port_type;
} S_DPI_CFG;

uint32_t data_path_init(S_DPI_CFG *pCfg);
uint32_t data_path_init_fiber(DRV_BBH_PORT_INDEX wan_emac);
uint32_t data_path_go(void);
void bridge_port_sa_da_cfg(void);

#endif
