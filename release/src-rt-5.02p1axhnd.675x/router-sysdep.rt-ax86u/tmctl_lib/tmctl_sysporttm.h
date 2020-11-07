/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2019:proprietary:standard

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


#ifndef _TMCTL_SYSPORT_H_
#define _TMCTL_SYSPORT_H_

/*!\file tmctl_sysport.h
 * \brief This file contains declarations for tmctl sysport related functions.
 *
 */

#include "tmctl_api.h"
#include "archer_api.h"

#define TMCTL_SYSPORT_TM_MAX_QUEUES 8
#define TMCTL_SYSPORT_TM_DEFAULT_SHAPER_KBPS 1000*1000 /* 1Gbps */
#define TMCTL_SYSPORT_TM_DEFAULT_SHAPER_MBS 2000 /* 2000Bytes */

/*TODO: Get queue size using archerctl api.
  Before that, qsize is hardcoded to SYSPORT_TM_QUEUE_SIZE(512)
  which is defined in sysport driver. */
#define SYSPORT_TM_QUEUE_SIZE 512


/* ----------------------------------------------------------------------------
 * This function gets Sysport TM capabilities from Sysport driver.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    tmParms_p (OUT) structure to return Sysport TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getPortTmParms(const char* ifname,
                                          tmctl_portTmParms_t* tmParms_p);

/* ----------------------------------------------------------------------------
 * This function initializes the Sysport TM configuration for a port.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_portTmInit(const char* ifname);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    qid (IN) Queue ID.
 *    qcfg_p (OUT) structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getQueueCfg(const char* ifname,
                                       int qid,
                                       tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name. 
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_setQueueCfg(const char* ifname,
                                       tmctl_portTmParms_t* tmParms_p,
                                       tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the Sysport TM port shaping rate.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    shaper_p (OUT) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getPortShaper(const char* ifname,
                                         tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function sets the port shaping rate. If the specified shaping rate
 * is greater than 0, Sysport TM mode will be switched from auto to manual.
 * And the shaper rate will be set to this value. Otherwise, the shaper rate
 * of manual mode will be set according to 1Gbps and change back to auto mode.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_setPortShaper(const char* ifname,
                                         tmctl_shaper_t* shaper_p);
                                          

/* ----------------------------------------------------------------------------
 * This function gets the queue statistics of a Sysport TM queue.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    qid (IN) Queue ID.
 *    stats_p (OUT) structure to receive the queue statistics.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlSysportTm_getQueueStats(const char* ifname,
                                         int qid,
                                         tmctl_queueStats_t* stats_p);
#endif /* _TMCTL_SYSPORT_H_ */

