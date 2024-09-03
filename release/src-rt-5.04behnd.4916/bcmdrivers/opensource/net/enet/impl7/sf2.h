/*
   <:copyright-BRCM:2015:DUAL/GPL:standard
   
      Copyright (c) 2015 Broadcom 
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

/*
 *  Created on: May/2017
 *      Author: steven.hsieh@broadcom.com
 */

#ifndef _SF2_H_
#define _SF2_H_

#include "port.h"
#include "enet.h"
#include "sw_common.h"
#include <bcm/bcmswapitypes.h>

void sf2_pseudo_mdio_switch_read(int page, int reg, void *data_out, int len);
void sf2_pseudo_mdio_switch_write(int page, int reg, void *data_in, int len);

extern enetx_port_t *sf2_sw;       /* 1st SF2 switch */
extern enetx_port_t *sf2_sw_ext;   /* 2nd SF2 switch */
extern enetx_port_t *nb_sw_ext;    /* external non-brcm switch */
extern uint32_t sf2_unit_bmap;

extern sw_ops_t port_sf2_sw;
extern port_ops_t port_sf2_port;
extern port_ops_t port_sf2_port_mac;
extern port_ops_t port_sf2_port_imp;
 
void port_sf2_deep_green_mode_handler(void);

void ioctl_extsw_dump_page0(void);
int ioctl_pwrmngt_get_deepgreenmode(int mode);
int ioctl_pwrmngt_set_deepgreenmode(int enable);
int ioctl_extsw_pmdioaccess(struct ethswctl_data *e);
int ioctl_extsw_info(struct ethswctl_data *e);
int ioctl_extsw_cfg_acb(struct ethswctl_data *e);
int ioctl_extsw_que_map(struct ethswctl_data *e);
int ioctl_extsw_cosq_port_mapping(struct ethswctl_data *e);
int ioctl_extsw_pcp_to_priority_mapping(struct ethswctl_data *e);
int ioctl_extsw_pid_to_priority_mapping(struct ethswctl_data *e);
int ioctl_extsw_set_multiport_address(int unit, uint8_t *addr);
int ioctl_extsw_que_mon(struct ethswctl_data *e);
int ioctl_extsw_maclmt(struct ethswctl_data *e);
int ioctl_extsw_prio_control(struct ethswctl_data *e);
int ioctl_extsw_dscp_to_priority_mapping(struct ethswctl_data *e);
int ioctl_extsw_cos_priority_method_cfg(struct ethswctl_data *e);
int ioctl_extsw_cosq_sched(enetx_port_t *self, struct ethswctl_data *e);
int ioctl_extsw_port_erc_config(struct ethswctl_data *e);
int ioctl_extsw_port_shaper_config(struct ethswctl_data *e);
int ioctl_extsw_cfp(struct ethswctl_data *e);    /* in sf2_cfp.c */
void extsw_set_mac_address(enetx_port_t *port);

#define IS_UNIT_SF2(u)  (((unsigned int)(u)<MAX_SWITCHES)&&(sf2_unit_bmap & (1 << (u))))

#endif

