/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/


#ifndef _TMCTL_FAP_H_
#define _TMCTL_FAP_H_

/*!\file tmctl_fap.h
 * \brief This file contains declarations for tmctl fap related functions.
 *
 */

#include "tmctl_api.h"
#include "fapctl_api.h"

/* ----------------------------------------------------------------------------
 * This function initializes the FAP TM configuration for a port.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    tmParms_p (IN) Port tm parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_portTmInit(const char* ifname,
                                tmctl_portTmParms_t* tmParms_p);

/* ----------------------------------------------------------------------------
 * This function un-initializes the FAP TM configuration of a port.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_portTmUninit(const char* ifname);

/* ----------------------------------------------------------------------------
 * This function gets the configuration of a FAP TM queue.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    qid (IN) Queue ID.
 *    qcfg_p (OUT) structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getQueueCfg(const char* ifname,
                                 int qid,
                                 tmctl_queueCfg_t* qcfg_p);

/* ----------------------------------------------------------------------------
 * This function sets the configuration of a queue.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name. 
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setQueueCfg(const char* ifname,
                                 tmctl_portTmParms_t* tmParms_p,
                                 tmctl_queueCfg_t* qcfg_p);

/* ----------------------------------------------------------------------------
 * This function dislocates the queue from FAP TM driver.
 * Note that the FAP TM queue is not deleted. Un-configured bit will be set and
 * qsize will returned zero when getting queue configuration.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    qid (IN) Queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_delQueueCfg(const char* ifname,
                                 int qid);

/* ----------------------------------------------------------------------------
 * This function gets the port shaping rate.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    shaper_p (OUT) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getPortShaper(const char* ifname,
                                   tmctl_shaper_t* shaper_p);

/* ----------------------------------------------------------------------------
 * This function sets the port shaping rate. If the specified shaping rate
 * is greater than 0, FAP TM mode will be switched from auto to manual.
 * And the shaper rate will be set to this value. Otherwise, the shaper
 * rate of manual mode will be set according to auto mode, or even change
 * back to auto mode if no queues configured.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setPortShaper(const char* ifname,
                                   tmctl_shaper_t* shaper_p);

/* ----------------------------------------------------------------------------
 * This function allocates a free FAP TM queue profile index.
 *
 * Parameters:
 *    queueProfileId_p (OUT) pointer for returned queue profile index.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_allocQueueProfileId(int* queueProfileId_p);

/* ----------------------------------------------------------------------------
 * This function frees a FAP TM queue profile index.
 *
 * Parameters:
 *    queueProfileId (IN) queue profile index to be freed.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_freeQueueProfileId(int queueProfileId);

/* ----------------------------------------------------------------------------
 * This function gets the queue profile of a FAP TM queue profile.
 *
 * Parameters:
 *    queueProfileId (IN) Queue Profile ID.
 *    queueProfile_p (OUT) structure to receive the queue profile parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getQueueProfile(int queueProfileId,
                                     tmctl_queueProfile_t* queueProfile_p);

/* ----------------------------------------------------------------------------
 * This function sets the queue profile of a FAP TM queue profile.
 *
 * Parameters:
 *    queueProfileId (IN) Queue Profile ID.
 *    queueProfile_p (IN) structure containing the queue profile parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setQueueProfile(int queueProfileId,
                                     tmctl_queueProfile_t* queueProfile_p);

/* ----------------------------------------------------------------------------
 * This function gets the drop algorithm of a FAP TM queue.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    qid (IN) Queue ID.
 *    dropAlg_p (OUT) structure to receive the drop algorithm parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getQueueDropAlg(const char* ifname,
                                     int qid,
                                     tmctl_queueDropAlg_t* dropAlg_p);

/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a FAP TM queue.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name. 
 *    qid (IN) Queue ID.
 *    dropAlg_p (IN) structure containing the drop algorithm parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setQueueDropAlg(const char* ifname,
                                     int qid,
                                     tmctl_queueDropAlg_t* dropAlg_p);

/* ----------------------------------------------------------------------------
 * This function gets the drop algorithm of a FAP TM XTM Channel.
 *
 * Parameters:
 *    chid (IN) XTM Channel ID.
 *    dropAlg_p (OUT) structure to receive the drop algorithm parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getXtmChannelDropAlg(int chid,
                                          tmctl_queueDropAlg_t* dropAlg_p);

/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a FAP TM XTM Channel.
 *
 * Parameters:
 *    chid (IN) XTM Channel ID.
 *    dropAlg_p (IN) structure containing the drop algorithm parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setXtmChannelDropAlg(int chid,
                                          tmctl_queueDropAlg_t* dropAlg_p);

/* ----------------------------------------------------------------------------
 * This function sets the drop algorithm of a FAP TM XTM Channel in another way.
 *
 * Parameters:
 *    chid (IN) XTM Channel ID.
 *    dropAlgorithm (IN) drop algorithm.
 *    dropAlgLo_p (IN) pointer to drop algorithm structure.
 *    dropAlgHi_p (IN) pointer to drop algorithm structure.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_setXtmChannelDropAlgExt(int chid,
                                             tmctl_dropAlg_e dropAlgorithm,
                                             tmctl_queueDropAlgExt_t* dropAlgLo_p,
                                             tmctl_queueDropAlgExt_t* dropAlgHi_p);
                                          
/* ----------------------------------------------------------------------------
 * This function gets the queue statistics of a FAP TM queue.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    qid (IN) Queue ID.
 *    stats_p (OUT) structure to receive the queue statistics.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getQueueStats(const char* ifname,
                                   int qid,
                                   tmctl_queueStats_t* stats_p);

/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities) from fapctl driver.
 *
 * Parameters:
 *    ifname (IN) fapctl interface name.
 *    tmParms_p (OUT) structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlFap_getPortTmParms(const char* ifname,
                                    tmctl_portTmParms_t* tmParms_p);

#endif /* _TMCTL_FAP_H_ */

