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
 *  Created on: Jan/2016
 *      Author: ido@broadcom.com
 */

#ifndef _ENET_TYPES_H_
#define _ENET_TYPES_H_

#include <linux/types.h>

/* RX info context, can be different for different platforms */
typedef struct
{
    uint32_t src_port; /* Shared fields, must be included in all platforms */
    uint32_t ptp_index;
    uint32_t flow_id;
    uint32_t data_offset;
    uint32_t reason;
    uint32_t extra_skb_flags;
    uint32_t is_exception;
} enetx_rx_info_t;

#endif

