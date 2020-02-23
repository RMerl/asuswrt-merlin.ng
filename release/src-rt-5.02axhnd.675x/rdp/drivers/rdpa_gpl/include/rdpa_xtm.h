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
