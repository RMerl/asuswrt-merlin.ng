#ifndef __RDPA_CMD_IC_H_INCLUDED__
#define __RDPA_CMD_IC_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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
