#ifndef __RDPA_CMD_DS_WAN_UDP_FILTER_H_INCLUDED__
#define __RDPA_CMD_DS_WAN_UDP_FILTER_H_INCLUDED__

/*
 <:copyright-BRCM:2014:DUAL/GPL:standard
 
    Copyright (c) 2014 Broadcom 
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
 * File Name  : rdpa_cmd_ds_wan_udp_filter.h
 *
 * Description: This file contains the Runner DS WAN UDP Filter IOCTL API.
 *
 *******************************************************************************
 */
int rdpa_cmd_ds_wan_udp_filter_ioctl(unsigned long arg);

#endif /* __RDPA_CMD_DS_WAN_UDP_FILTER_H_INCLUDED__ */
