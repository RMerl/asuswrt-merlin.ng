/***********************************************************************
 * <:copyright-BRCM:2008-2013:proprietary:standard
 * 
 *    Copyright (c) 2008-2013 Broadcom 
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :> *
 * $Change: 121810 $
 ***********************************************************************/

/** @defgroup utils L2Driver API: Classifier rules
 *  @{
 */

/**
   \file l2_driver_classifier.h
   \brief L2 Driver classifier rules related functions.
*/

#ifndef L2_DRIVER_CLASSIFIER_H
#define L2_DRIVER_CLASSIFIER_H

#include "base_types.h"

/**
   \brief Classifier rules currently defined.
*/
typedef enum
{
   CLASSIF_RULE_ETH_DA = 0, //!< Destination Address of the Ethernet frame.
   CLASSIF_RULE_ETH_SA, //!< Source Address of the Ethernet frame.
   CLASSIF_RULE_VLAN_PRIORITY, //!< VLAN User Priority
   CLASSIF_RULE_VLAN_ID, //!< VLAN ID
   CLASSIF_RULE_IPV4_TOS, //!< IPv4 Type of Service
   CLASSIF_RULE_IPV4_PROT, //!< IPv4 Protocol
   CLASSIF_RULE_IPV4_SA, //!< IPv4 Source Address
   CLASSIF_RULE_IPV4_DA, //!< IPv4 Destination Address
   CLASSIF_RULE_IPV6_TRAF_CLASS, //!< IPv6 Traffic Class
   CLASSIF_RULE_IPV6_FLOW_LABEL, //!< IPv6 Flow Label
   CLASSIF_RULE_IPV6_SA, //!< IPv6 Source Address
   CLASSIF_RULE_IPV6_DA, //!< IPv6 Destination Address
   CLASSIF_RULE_TCP_SPORT, //!< TCP Source Port
   CLASSIF_RULE_TCP_DPORT, //!< TCP Destination Port
   CLASSIF_RULE_UDP_SPORT, //!< UDP Source Port
   CLASSIF_RULE_UDP_DPORT, //!< UDP Destination Port
   CLASSIF_RULE_MCAST_ADDR_IPv4, //!< Multicast address IPv4
   CLASSIF_RULE_MCAST_ADDR_IPv6,  //!< Multicast address IPv4
   CLASSIF_RULE_INVALID = 0XFF   //!< Invalid / uninitialized
}
tE_ClassifierRule;

/**
   \brief Classifier rules types
*/
typedef enum
{
   NUMBER,
   ETH_ADD,
   IPv4_ADDR,
   IPv6_ADDR,
   MCAST_ADDR_IPV4,
   MCAST_ADDR_IPV6
} tE_ClassifierRuleType;


#ifdef __cplusplus
extern "C" {
#endif

/**
   \brief Returns size in octets for a classifier rule
   \param   classif_rule   rule
   \return  TU8            classifier rule size
*/
TU8 l2driver_classifier_size(const tE_ClassifierRule classif_rule);

/**
   \brief Get rule type
   \param   rule                    rule
   \return  tE_ClassifierRuleType   rule type
*/
tE_ClassifierRuleType l2driver_classifier_type(const tE_ClassifierRule rule);

#ifdef __cplusplus
}
#endif


#endif // L2_DRIVER_CLASSIFIER_H

/** @} */ // end of group utils

