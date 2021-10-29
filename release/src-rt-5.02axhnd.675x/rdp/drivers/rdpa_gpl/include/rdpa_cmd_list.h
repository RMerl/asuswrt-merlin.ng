/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _RDPA_CMD_LIST_H_
#define _RDPA_CMD_LIST_H_

#if defined(BCM63158)
#define RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH   98
#define RDPA_CMD_LIST_PACKET_BUFFER_OFFSET       0
#define RDPA_CMD_LIST_PACKET_HEADER_OFFSET       62
#define RDPA_CMD_LIST_UCAST_LIST_OFFSET          20
#define RDPA_CMD_LIST_HEADROOM                   RDPA_CMD_LIST_PACKET_HEADER_OFFSET
#else
#define RDPA_CMD_LIST_PACKET_HEADER_MAX_LENGTH   110
#define RDPA_CMD_LIST_PACKET_BUFFER_OFFSET       0
#define RDPA_CMD_LIST_PACKET_HEADER_OFFSET       18
#define RDPA_CMD_LIST_UCAST_LIST_OFFSET          24
#define RDPA_CMD_LIST_HEADROOM                   58
#endif

#define RDPA_CMD_LIST_UCAST_LIST_SIZE            104
#define RDPA_CMD_LIST_UCAST_LIST_SIZE_32         (RDPA_CMD_LIST_UCAST_LIST_SIZE / 4)

#define RDPA_CMD_LIST_MCAST_L2_LIST_OFFSET       0
#define RDPA_CMD_LIST_MCAST_L2_LIST_SIZE         64
#define RDPA_CMD_LIST_MCAST_L2_LIST_SIZE_32      (RDPA_CMD_LIST_MCAST_L2_LIST_SIZE / 4)

#define RDPA_CMD_LIST_MCAST_L3_LIST_OFFSET       52
#define RDPA_CMD_LIST_MCAST_L3_LIST_SIZE         20
#define RDPA_CMD_LIST_MCAST_L3_LIST_SIZE_32      (RDPA_CMD_LIST_MCAST_L3_LIST_SIZE / 4)

#endif /* _RDPA_CMD_LIST_H_ */
