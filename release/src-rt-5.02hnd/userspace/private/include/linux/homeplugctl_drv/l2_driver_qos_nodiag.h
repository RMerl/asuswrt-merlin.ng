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

/** @defgroup utils L2Driver API: QoS management for no_diag firmware
 *  @{
 */

/**
   \file l2_driver_qos_nodiag.h
   \brief QoS rule management for no_diag firmware
*/


#ifndef L2_DRIVER_QOS_NODIAG_H
#define L2_DRIVER_QOS_NODIAG_H


#include "base_types.h"
#include "l2_driver_classifier.h"
#include "transactions/definitions.h"


/** @ingroup N3
   \brief Channel Access Priority.
*/
typedef enum
{
   CAP_0 = 0,
   CAP_1,
   CAP_2,
   CAP_3
} tE_CAP;

/** @ingroup N3
   \brief Classifier rule for no_diag.
*/
typedef struct
{
   tE_ClassifierRule id; //!< rule id
   TU8 value_size; //!< value size
   TU8 value[16]; //!< value
} tS_ClassifierRuleNoDiag;

/** @ingroup N3
   \brief Classifier rule set for no_diag.
*/
typedef struct
{
   TU8 no_av_rules; //!< Number of chaining rules (a maximum of 4)
   tE_ClassifierRule id[4]; //!< rule's ids

   TU8 no_bytes_used; //!< Number of bytes used in the rule value buffer
   TU8 value[16]; /*!< rules value buffer
                       (all the rules shared 16 bytes for their value) */
} tS_ClassifierRuleSetNoDiag;


/** @ingroup N3
   \brief QoS rule.
*/
typedef struct st_QoSRuleNoDiag tS_QoSRuleNoDiag;
struct st_QoSRuleNoDiag
{
   tE_CAP CAP; //!< QoS rule channel access priority.
   tS_ClassifierRuleSetNoDiag classifier_rule_set; //!< classifier rules assoc.
};


/** @ingroup N3
   \brief Exit code once the classifier rules and its associated CSPEC data
          are changed in the NV-RAM
*/
typedef enum
{
  L2API_ECODE_SETQOS_NORULES = 0, //!< Number of QoS rules to set is 0
  L2API_ECODE_SETQOS_RULESNULL, //!< QoS rules to set are NULL
  L2API_ECODE_SETQOS_MAXRULES, //!< Number of QoS rules to set greater than MAX
  L2API_ECODE_SETQOS_GETPARAM1, //!< Error get val for param CLASSIFIER_RULES_SRC_DST_MAC
  L2API_ECODE_SETQOS_GETPARAM2, //!< Error get val for param CLASSIFIER_RULES_VLAN
  L2API_ECODE_SETQOS_GETPARAM3, //!< Error get val for param CLASSIFIER_RULES_IP_TOS_PROT
  L2API_ECODE_SETQOS_GETPARAM4, //!< Error get val for param CLASSIFIER_RULES_IP_DST_SRC_ADDR
  L2API_ECODE_SETQOS_GETPARAM5, //!< Error get val for param CLASSIFIER_RULES_UDP_TCP_PORT
  L2API_ECODE_SETQOS_MAXTYPERULES1, /*!< Number of RULES_SRC_DST_MAC greater than
                                         val for param CLASSIFIER_RULES_SRC_DST_MAC */
  L2API_ECODE_SETQOS_MAXTYPERULES2, /*!< Number of VLAN greater than
                                         val for param CLASSIFIER_RULES_VLAN */
  L2API_ECODE_SETQOS_MAXTYPERULES3, /*!< Number of IP_TOS_PROT greater than
                                         val for param CLASSIFIER_RULES_IP_TOS_PROT */
  L2API_ECODE_SETQOS_MAXTYPERULES4, /*!< Number of IP_DST_SRC_ADDR greater than
                                         val for param CLASSIFIER_RULES_IP_DST_SRC_ADDR */
  L2API_ECODE_SETQOS_MAXTYPERULES5, /*!< Number of UDP_TCP_PORT greater than
                                         val for param CLASSIFIER_RULES_UDP_TCP_PORT */
  L2API_ECODE_SETQOS_SUCCESS, //!< Data set successfully
  L2API_ECODE_SETQOS_FAIL //!< Data didn't set successfully

} tE_SetQoSRulesResult;

#ifdef __cplusplus
extern "C" {
#endif


/**
   \brief   Change the Classifier rules and its associated CSPEC data in the
            NV-RAM.
   \note    You must reboot the device for the changes to take effect.
   \param   no_rules - Number of QoS rules to set.
   \param   qos_rules - QoS rules to set. A maximum of 12 are allowed.
   \return  tE_SetQoSRulesResult with exit code.
*/

tE_SetQoSRulesResult l2driver_SetQoSRules(
   const TU8 no_rules,
   const tS_QoSRuleNoDiag *qos_rules);


/**
   \brief   Remove all the Classifier rules and its associated CSPEC data in the
            NV-RAM.
   \note    You must reboot the device for the changes to take effect.
   \return  TRUE if the data was cleared successfully otherwise return FALSE.
*/

TBool l2driver_ClearQoSRules();

/**
   \brief   Obtain the QoS rules saved within the NV-RAM.
   \param   no_rules - number of rules obtained.
   \param   qos_rules - QoS rules obtained.
   \return  TRUE if the rules were obtained successfully otherwise return FALSE.
*/

TBool l2driver_GetQoSRules(
   TU8* no_rules,
   tS_QoSRuleNoDiag *qos_rules);

/**
   \brief   Extract from a Classifier Rule Set the single Classifier Rules
   \param   rule_set - Rule set
   \param   rules - array with the individual rules extracted from the rule set
*/
void l2driver_ExtractClassifRule(
   const tS_ClassifierRuleSetNoDiag rule_set,
   tS_ClassifierRuleNoDiag* rules);

#ifdef __cplusplus
}
#endif


#endif // L2_DRIVER_QOS_NODIAG_H

/** @} */ // end of group utils
