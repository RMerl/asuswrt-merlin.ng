/*
   <:copyright-BRCM:2018:DUAL/GPL:standard

      Copyright (c) 2018 Broadcom
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
 *  Created on: June/2019
 *      Author: fulin.chang@broadcom.com
 */

#ifndef _SW_COMMON_H_
#define _SW_COMMON_H_

#include "port.h"
#include "enet.h"
#include <bcm/bcmswapitypes.h>

#define DATA_TYPE_HOST_ENDIAN   (0x00<<24)
#define DATA_TYPE_BYTE_STRING   (0x01<<24)
#define DATA_TYPE_VID_MAC       (0x02<<24)


extern enetx_port_t *sw_p;   /* switch */

int ioctl_extsw_regaccess(struct ethswctl_data *e, enetx_port_t *port);
int ioctl_extsw_port_mirror_ops(struct ethswctl_data *e);
int ioctl_extsw_port_trunk_ops(struct ethswctl_data *e);
int ioctl_extsw_control(struct ethswctl_data *e);
int ioctl_extsw_pbvlan(struct ethswctl_data *e);
int ioctl_extsw_vlan(struct ethswctl_data *e);
int ioctl_extsw_arl_access(struct ethswctl_data *e);
int ioctl_extsw_arl_dump(struct ethswctl_data *e);  // add by Andrew
int ioctl_extsw_port_jumbo_control(struct ethswctl_data *e);
int ioctl_extsw_port_irc_get(struct ethswctl_data *e);
int ioctl_extsw_port_irc_set(struct ethswctl_data *e);
int ioctl_extsw_dos_ctrl(struct ethswctl_data *e);
int ioctl_extsw_port_storm_ctrl(struct ethswctl_data *e);

void extsw_rreg_wrap(int unit, int page, int reg, void *vptr, int len);
void extsw_wreg_wrap(int unit, int page, int reg, void *vptr, int len);

int extsw_arl_write_ext(int unit, uint8_t *mac, uint16_t vid, uint32_t v32);
int remove_arl_entry_wrapper(void *ptr);

int port_sw_port_stp_set(enetx_port_t *self, int mode, int state);
void port_sw_port_fast_age(enetx_port_t *port);
int port_sw_port_role_set(enetx_port_t *self, port_netdev_role_t role);
int port_sw_mib_dump(enetx_port_t *self, int all);

void port_sw_fast_age(enetx_port_t *sw);

#if defined(CONFIG_NET_SWITCHDEV)
#endif

#define IS_PHY_ADDR_FLAG 0x80000000
#define IS_SPI_ACCESS    0x40000000

#define PORT_ID_M 0xF
#define PORT_ID_S 0
#define PHY_REG_M 0x1F
#define PHY_REG_S 4

#endif
