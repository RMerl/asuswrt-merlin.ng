#ifndef __RDPA_CMD_FILTER_H_INCLUDED__
#define __RDPA_CMD_FILTER_H_INCLUDED__

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
 * File Name  : rdpa_cmd_filter.h
 *
 * Description: This file contains the filter API.
 *
 *******************************************************************************
 */

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)

#define RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD (IQ_KEY_MASK_IP_PROTO | \
            IQ_KEY_MASK_SRC_PORT | IQ_KEY_MASK_DST_PORT | IQ_KEY_MASK_ETHER_TYPE)

int rdpa_iq_filter_add(void *iq_param);
int rdpa_iq_filter_rem(void *iq_param);
#endif

#endif /* __RDPA_CMD_FILTER_H_INCLUDED__ */


