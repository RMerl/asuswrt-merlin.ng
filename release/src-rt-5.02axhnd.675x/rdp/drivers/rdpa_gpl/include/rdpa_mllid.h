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
