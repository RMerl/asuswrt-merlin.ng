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


#ifndef _RDPA_XTM_H_
#define _RDPA_XTM_H_

/** \defgroup xtm XTM Management
 * Objects in this group control XTM-related configuration
 */

/**
 * \defgroup xtmchannel CHANNEL Management
 * \ingroup xtm
 * @{
 */

#define RDPA_MAX_XTMCHANNEL  16      /**< Max number of XTMCHANNELs */


/** @} end of xtmchannel Doxygen group */

/**
 * \defgroup xtmflow FLOW Management
 * \ingroup xtm
 * @{
 */

#define RDPA_MAX_XTMFLOW       255 /**< Max number of XTMFLOWs. */

/** XTMFLOW US configuration.
 * Underlying type for xtmflow_us_cfg aggregate
 */
typedef struct {
    bdmf_object_handle xtmchannel;       /**< XTMCHANNEL id */
} rdpa_xtmflow_us_cfg_t;


/** XTMFLOW statistics */
typedef struct {
    uint32_t tx_packets;            /**< Tx Packets */
    uint32_t tx_bytes;              /**< Tx bytes */
    uint32_t tx_packets_discard;    /**< Tx Packet discard */ 
} rdpa_xtmflow_stat_t;

/** @} end of xtmflow Doxygen group */

#endif /* _RDPA_XTM_H_ */
