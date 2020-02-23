/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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
 :>
*/
/*
 * rdpa_vlan_action_int.h
 * vlan action rdp/xrdp interface abstraction
 */

#ifndef RDPA_VLAN_ACTION_EX_H_
#define RDPA_VLAN_ACTION_EX_H_

/* vlan_action rdd interface parameters */
typedef struct rdpa_rdd_vlan_action_params
{
    bdmf_index                  index;
    rdpa_traffic_dir            dir;
    rdpa_vlan_action_cfg_t      cfg;
    bdmf_boolean                outer_tpid_overwrite_enable;
    bdmf_boolean                inner_tpid_overwrite_enable;
    uint8_t                     outer_tpid_id;
    uint8_t                     inner_tpid_id;
} rdpa_rdd_vlan_action_params_t;

#ifndef XRDP

/* vlan action cookie.
 * For XRDP it contains command list
 * For RDP it is currently unused
 */

typedef struct rdd_vlan_action_cl
{
    int dummy;
} rdd_vlan_action_cl_t;

#else

/* Max number of commands in command list */
#define RDPA_RDD_MAX_VLAN_ACTION_CL_LEN         26

/* vlan action cookie.
 */
typedef struct rdd_vlan_action_cl
{
    uint16_t num_commands;
    uint8_t tag_state_cl_offset[4]; /* Per tag state: untagged, PTAG, QTAG, dual tag */
    uint16_t commands[RDPA_RDD_MAX_VLAN_ACTION_CL_LEN];
} rdd_vlan_action_cl_t;

/* Get DROP action command list  */
bdmf_error_t rdpa_vlan_action_drop_cl_get(rdd_vlan_action_cl_t *cl);

#endif

/* Validate VLAN command */
bdmf_error_t rdpa_rdd_vlan_action_validate(struct bdmf_object *mo, const rdpa_vlan_action_cfg_t *cfg);

/* Apply TPID map */
void rdpa_rdd_tpid_set(uint16_t tpid_array[], uint8_t size);

/* Apply VLAN action */
bdmf_error_t rdpa_rdd_vlan_action_set(struct bdmf_object *mo, const rdpa_rdd_vlan_action_params_t *params,
    rdd_vlan_action_cl_t *cl);

/* Get command list associated with VLAN action */
bdmf_error_t rdpa_vlan_action_cl_get(struct bdmf_object *mo, rdd_vlan_action_cl_t *cl);

#endif /* RDPA_VLAN_ACTION_EX_H_ */
