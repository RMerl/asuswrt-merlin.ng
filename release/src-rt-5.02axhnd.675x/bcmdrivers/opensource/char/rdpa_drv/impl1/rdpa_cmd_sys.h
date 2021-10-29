#ifndef __RDPA_CMD_SYS_H_INCLUDED__
#define __RDPA_CMD_SYS_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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
 *******************************************************************************
 * File Name  : rdpa_cmd_sys.h
 *
 * Description: This file contains the Ingress classifier API.
 *
 *******************************************************************************
 */
#if (defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE))
#if defined(CONFIG_BCM_DSL_RDP)
#define RDPA_IQ_IH_CONG_THRESHOLD_NUM 4
#endif
int rdpa_iq_sys_set_cong_ctrl(void *iq_param);
int rdpa_iq_sys_get_cong_ctrl(void *iq_param);
#endif
int rdpa_cmd_sys_ioctl(unsigned long arg);
void rdpa_cmd_sys_init(void);

#endif /* __RDPA_CMD_SYS_H_INCLUDED__ */

