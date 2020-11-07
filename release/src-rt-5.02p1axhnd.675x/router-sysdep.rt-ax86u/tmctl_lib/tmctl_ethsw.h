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


#ifndef _TMCTL_ETHSW_H_
#define _TMCTL_ETHSW_H_

/*!\file tmctl_ethsw.h
 * \brief This file contains declarations for tmctl ethsw related functions.
 *
 */

#include "tmctl_api.h"
#include "ethswctl_api.h"

/* List the modes of EthSw cosqsched */
typedef enum
{
   TMCTL_COSQ_SCHED_INVALID = 0,
   TMCTL_COSQ_SCHED_SP,
   TMCTL_COSQ_SCHED_SPWRR,
   TMCTL_COSQ_SCHED_SPWDRR,
   TMCTL_COSQ_SCHED_WRR,
   TMCTL_COSQ_SCHED_WDRR
} tmctl_cosq_sched_e;


/* ----------------------------------------------------------------------------
 * This function check if an interface is WAN or not.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    isWan (OUT) Is WAN interface or not
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_isWanIntf(const char* ifname, BOOL* isWan);


/* ----------------------------------------------------------------------------
 * This function determine new port queue configurations,
 * based on current port queue configurations and new queue config.
 * When new queue config is applied, EthSw cosqsched state will
 * be switched between below 11 states:
 *
 * --------------------------------------------
 *          EthSw cosqsched state list
 * --------------------------------------------
 * St.  Q0  Q1  Q2  Q3  Q4  Q5  Q6  Q7 =>  Mode
 * 01)  SP  SP  SP  SP  SP  SP  SP  SP =>    SP
 * 02) WRR WRR WRR WRR  SP  SP  SP  SP => SPWRR
 * 03) WRR WRR WRR WRR WRR  SP  SP  SP => SPWRR
 * 04) WRR WRR WRR WRR WRR WRR  SP  SP => SPWRR
 * 05) WRR WRR WRR WRR WRR WRR WRR  SP => SPWRR
 * 06) WRR WRR WRR WRR WRR WRR WRR WRR =>   WRR
 * 07) WDR WDR WDR WDR  SP  SP  SP  SP => SPWDR
 * 08) WDR WDR WDR WDR WDR  SP  SP  SP => SPWDR
 * 09) WDR WDR WDR WDR WDR WDR  SP  SP => SPWDR
 * 10) WDR WDR WDR WDR WDR WDR WDR  SP => SPWDR
 * 11) WDR WDR WDR WDR WDR WDR WDR WDR =>   WDR
 *
 * Parameters:
 *    ifname (IN)         Linux interface name.
 *    newQcfg_p (IN)      Structure containing the new queue config parameters.
 *    currPortQcfg_p (IN) The current port queue configurations.
 *    newPortQcfg_p (OUT) The new port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_determineNewPortQcfg(tmctl_portTmParms_t* tmParms_p,
                                            tmctl_queueCfg_t*    newQcfg_p,
                                            tmctl_portQcfg_t*    currPortQcfg_p,
                                            tmctl_portQcfg_t*    newPortQcfg_p);


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch port scheduler.
 *
 * Parameters:
 *    ifname (IN)      Linux interface name.
 *    portQcfg_p (OUT) The port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortSched(const char*       ifname,
                                    tmctl_portQcfg_t* portQcfg_p);


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch port scheduler.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    portQcfg_p (IN) The port queue configurations.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setPortSched(const char*       ifname,
                                    tmctl_portQcfg_t* portQcfg_p);


/* ----------------------------------------------------------------------------
 * This function resets the Ethernet switch port scheduler to Strict Priority.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_resetPortSched(const char* ifname);


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch port shaper configuration.
 *
 * Parameters:
 *    ifname (IN) Linux interface name.
 *    qid (IN) Queue ID.
 *    shaper_p (OUT) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortShaper(const char*     ifname,
                                     tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch port shaper configuration.
 *
 * Parameters:
 *    ifname (IN)   Linux interface name.
 *    shaper_p (IN) The port shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setPortShaper(const char*     ifname,
                                     tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function gets the Ethernet switch queue shaper configuration.
 *
 * Parameters:
 *    ifname (IN)    Linux interface name.
 *    qid (IN)       Queue ID.
 *    shaper_p (OUT) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getQueueShaper(const char*     ifname,
                                      int             qid,
                                      tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function sets the Ethernet switch queue shaper configuration.
 *
 * Parameters:
 *    ifname (IN)   Linux interface name.
 *    qid (IN)      Queue ID.
 *    shaper_p (IN) The queue shaper configuration.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_setQueueShaper(const char*     ifname,
                                      int             qid,
                                      tmctl_shaper_t* shaper_p);


/* ----------------------------------------------------------------------------
 * This function gets port TM parameters (capabilities) from Ethernet Switch
 * driver.
 *
 * Parameters:
 *    ifname (IN)     Linux interface name.
 *    tmParms_p (OUT) Structure to return port TM parameters.
 *
 * Return:
 *    tmctl_ret_e enum value
 * ----------------------------------------------------------------------------
 */
tmctl_ret_e tmctlEthSw_getPortTmParms(const char*          ifname,
                                      tmctl_portTmParms_t* tmParms_p);


#endif /* _TMCTL_ETHSW_H_ */

