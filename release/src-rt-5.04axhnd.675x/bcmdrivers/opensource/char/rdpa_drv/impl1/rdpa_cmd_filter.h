#ifndef __RDPA_CMD_FILTER_H_INCLUDED__
#define __RDPA_CMD_FILTER_H_INCLUDED__

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
 * File Name  : rdpa_cmd_filter.h
 *
 * Description: This file contains the filter API.
 *
 *******************************************************************************
 */

#if defined(CONFIG_BCM_INGQOS) || defined(CONFIG_BCM_INGQOS_MODULE)

#define RDPA_IQ_INGRESS_FILTER_SUPPORT_FIELD (IQ_KEY_MASK_IP_PROTO | \
            IQ_KEY_MASK_DST_PORT | IQ_KEY_MASK_ETHER_TYPE)

int rdpa_iq_filter_add(void *iq_param);
int rdpa_iq_filter_rem(void *iq_param);
#endif

int rdpa_cmd_filter_ioctl(unsigned long arg);

#endif /* __RDPA_CMD_FILTER_H_INCLUDED__ */


