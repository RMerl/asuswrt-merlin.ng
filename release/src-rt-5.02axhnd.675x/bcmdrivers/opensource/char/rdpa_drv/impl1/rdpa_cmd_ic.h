#ifndef __RDPA_CMD_IC_H_INCLUDED__
#define __RDPA_CMD_IC_H_INCLUDED__

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
 * File Name  : rdpa_cmd_ic.h
 *
 * Description: This file contains the Ingress classifier API.
 *
 *******************************************************************************
 */
 
#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)

#define RDPA_IQ_INGRESS_CLASS_SUPPORT_FIELD (IQ_KEY_MASK_INGRESS_DEVICE | \
            IQ_KEY_MASK_SRC_MAC | IQ_KEY_MASK_DST_MAC | \
            IQ_KEY_MASK_ETHER_TYPE | IQ_KEY_MASK_OUTER_VID | \
            IQ_KEY_MASK_OUTER_PBIT | IQ_KEY_MASK_INNER_VID | \
            IQ_KEY_MASK_INNER_PBIT | IQ_KEY_MASK_L2_PROTO | \
            IQ_KEY_MASK_L3_PROTO | IQ_KEY_MASK_IP_PROTO | \
            IQ_KEY_MASK_SRC_IP | IQ_KEY_MASK_DST_IP | IQ_KEY_MASK_DSCP | \
            IQ_KEY_MASK_IPV6_FLOW_LABEL | IQ_KEY_MASK_SRC_PORT | \
            IQ_KEY_MASK_DST_PORT | IQ_KEY_MASK_OFFSET_0 | IQ_KEY_MASK_OFFSET_1)

int rdpa_iq_ingress_class_add(void *iq_param);
int rdpa_iq_ingress_class_rem(void *iq_param);
int rdpa_iq_ingress_class_get(void *iq_param);
int rdpa_iq_ingress_class_find(void *iq_param);
#endif

int rdpa_cmd_ic_ioctl(unsigned long arg);
void rdpa_cmd_ic_init(void);

#endif /* __RDPA_CMD_IC_H_INCLUDED__ */
