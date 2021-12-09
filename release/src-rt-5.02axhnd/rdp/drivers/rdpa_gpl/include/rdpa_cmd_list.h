/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
