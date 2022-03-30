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


#ifndef _RDPA_MLLID_H_
#define _RDPA_MLLID_H_

/**
 * \defgroup mllid MLLID Management
 * \ingroup eponmanagement
 * Objects and functions in this group are used for MLLID configuration
 * @{
 */

#if defined(XRDP) && !defined(BCM6846) && !defined(BCM6878)
#define RDPA_EPON_MLLID_NUM      16
#else
#define RDPA_EPON_MLLID_NUM      8
#endif

/* mllid object private data */
typedef struct {
    bdmf_index index;                   /* MLLID index */
    bdmf_index flow_id;                 /* hw flow id*/
    bdmf_boolean enable;                /* enable mllid service*/
} mllid_drv_priv_t;

/** MLLID flow statistics 
 * Underlying type for mllid_stat aggregate
 */
typedef struct 
{
    uint32_t rx_packets;                /**< Rx Packets */
    uint32_t rx_bytes;                  /**< Rx Bytes */
    uint32_t rx_packets_discard;        /**< Rx Packet discard */  
} rdpa_mllid_stat_t;

/** @} end of mllid Doxygen group */

#endif /* _RDPA_MLLID_H_ */
