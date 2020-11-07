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


#ifndef _TMCTL_RDPA_H_
#define _TMCTL_RDPA_H_

/*!\file tmctl_rdpa.h
 * \brief This file contains declarations for tmctl rdpa related functions.
 *
 */

#include "tmctl_api.h"
#include "rdpactl_api.h"

#define TMCTL_MAX_PORTS_IN_LAG_GRP  (4) /* This number comes from the switch
                                         * which supports upto 4 ports in group */
/* ----------------------------------------------------------------------------
 * This function gets rdpa interface (port) ID by interface name.
 *
 * Parameters:
 *    ifname (IN) Interface name
 *    rdpaIf_p (OUT) rdpa interface ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getRdpaIfByIfname(const char* ifname, rdpa_if* rdpaIf_p);


/* ----------------------------------------------------------------------------
 * This function initializes the Runner TM configuration for a port, TCONT,
 * or LLID.
 *
 * Parameters:
 *    devType (IN)   rdpactl device type.
 *    rdpaIf (IN)    rdpa interface ID.
 *    tmParms_p (IN) Port tm parameters.
 *    cfgFlags (IN)  Configuration flags.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_TmInit(int devType, int rdpaIf,
                             tmctl_portTmParms_t* tmParms_p,
                             uint32_t             cfgFlags);


/* ----------------------------------------------------------------------------
 * This function un-initializes the Runner TM configuration of a port, TCONT,
*  or LLID.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_TmUninit(int devType, int rdpaIf);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of a runner queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    qid (IN) Queue ID.
 *    tmId_p (OUT) tmId of the queue.
 *    qcfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getQueueCfg(int devType, int rdpaIf,
                                  int qid, int* tmId_p, tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of a queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    tmParms_p (IN) port tm parameters.
 *    qcfg_p (IN) structure containing the queue config parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setQueueCfg(int devType, int rdpaIf,
                                  tmctl_portTmParms_t* tmParms_p,
                                  tmctl_queueCfg_t* qcfg_p);


/* ----------------------------------------------------------------------------
 * This function dislocates the Runner queue from the rdpactl config driver.
 * Note that the Runner queue is not deleted.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    qid (IN) Queue ID.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_delQueueCfg(int devType, int rdpaIf, int qid);


/* ----------------------------------------------------------------------------
 * This function gets the Runner queue configuration of the device.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    maxQueues (IN) max number of queues supported by the port.
 *    portQcfg_p (OUT) Structure to receive the port queue configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getDevQueueCfg(int devType, int rdpaIf,
                                     int maxQueues,
                                     tmctl_portQcfg_t* portQcfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the port shaping rate.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (OUT) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getPortShaper(int devType, int rdpaIf,
                                    tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function sets the port shaping rate. If the specified shaping rate
 * is greater than 0, the pre-configured Runner overall rate limiter will
 * be set with the shaping rate and linked to the port. Otherwise, the
 * overall rate limiter will be un-linked from the port.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setPortShaper(int devType, int rdpaIf,
                                    tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function gets the statistics of a queue.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    qid (IN) Queue ID.
 *    queueStats_p (OUT) Structure to return the queue stats.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getQueueStats(int devType, int rdpaIf, int qid,
                                    tmctl_queueStats_t* queueStats_p);


/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities) from rdpactl driver.
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID / TCONT ID / LLID
 *    tmParms_p (OUT) Structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getTmParms(int devType, int rdpaIf,
                                 tmctl_portTmParms_t* tmParms_p);

/* ----------------------------------------------------------------------------
 * This function gets TM memory information
 *
 * Parameters:
 *    fpmPoolMemorySize (OUT) fpm pool memory size in MB
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */

tmctl_ret_e tmctlRdpa_getMemoryInfo(int * fpmPoolMemorySize);
/* ----------------------------------------------------------------------------
 * This function sets the port rate limiting in the Runner thus enabling the per-port
 * shaping. Shaping rate less than or equal to 0 disables the per-port shaping
 * enables
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (IN) Shaper parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setTmRlCfg(int devType, int rdpaIf,
                                 tmctl_shaper_t* shaper_p);

/* ----------------------------------------------------------------------------
 * This function gets the port rate limiting parameters
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    shaper_p (OUT) Shaper (rate limiting) parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getTmRlCfg(int devType, int rdpaIf,
                                 tmctl_shaper_t* shaper_p);


tmctl_ret_e tmctlRdpa_setQueueDropAlg(int devType, int rdpaIf, int qid,
                                      tmctl_queueDropAlg_t* dropAlg_p);


tmctl_ret_e tmctlRdpa_getQueueDropAlg(int devType, int rdpaIf, int qid,
                                      tmctl_queueDropAlg_t* dropAlg_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of pbit to q table. If the
 * configuration is not found, ....
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    cfg_p (OUT) Structure to receive configuration parameters.
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getPbitToQ(int devType, 
                                 int rdpaIf, 
                                 tmctl_pbitToQCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of pbit to q table. 
 *
 * Parameters:
 *    devType (IN) rdpactl device type.
 *    rdpaIf (IN) rdpa interface ID.
 *    cfg_p (IN) config parameters.
 *               
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setPbitToQ(int devType, 
                                 int rdpaIf, 
                                 tmctl_pbitToQCfg_t* cfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of dscp to pbit feature.
 *
 * Parameters:
 *    dir (IN) direction.
 *    enable_p (OUT) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of dscp to pbit feature.
 *
 * Parameters:
 *    dir (IN) direction.
 *    enable_p (IN) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setForceDscpToPbit(tmctl_dir_e dir, BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function gets the configuration of packet based qos.
 *
 * Parameters:
 *    dir (IN) direction.
 *    type (IN) qos type
 *    enable_p (OUT) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_getPktBasedQos(tmctl_dir_e dir, 
                                  tmctl_qosType_e type, 
                                  BOOL* enable_p);


/* ----------------------------------------------------------------------------
 * This function sets the configuration of packet based qos.
 *
 * Parameters:
 *    dir (IN) direction.
 *    type (IN) qos type
 *    enable_p (IN) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setPktBasedQos(tmctl_dir_e dir, 
                                  tmctl_qosType_e type, 
                                  BOOL* enable_p);

/* ----------------------------------------------------------------------------
 * This function sets the service queues status in RDPA and assigns service
 * queues to the best-effort WAN TX queues.
 *
 * Parameters:
 *    enable (IN) enable or disable
 *
 * Return:
 *    tmctl_return_e enum value.
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlRdpa_setSvcQueuesEnable(BOOL enable);

#endif /* _TMCTL_RDPA_H_ */
