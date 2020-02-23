/*
* <:copyright-BRCM:2014-2015:proprietary:standard
* 
*    Copyright (c) 2014-2015 Broadcom 
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
 *    rdpa_gem_ex.h
 *    Created on: Nov 18, 2016
 */

#ifndef _RDPA_GEM_EX_H_
#define _RDPA_GEM_EX_H_

#if defined(BCM63158)
#define RDPA_GEM_HIGH_PRIO_GEM_MAX    47
#else
#define RDPA_GEM_HIGH_PRIO_GEM_MAX    31
#endif

/*
 *  Private GEM DBs
 */
/* gem object private data */
typedef struct 
{
    bdmf_index index; /**< Gem index */
    uint16_t port; /**< GEM port (external number) */
    rdpa_gem_flow_type type; /**< Flow type: Ethernet, OMCI, etc. */
    bdmf_index ds_def_flow; /**< Gem default flow configuration index */
    rdpa_gem_flow_ds_cfg_t ds_cfg; /**< GEM DS configuration */
    rdpa_gem_flow_us_cfg_t us_cfg; /**< GEM US configuration */
} gem_drv_priv_t;

void remove_def_flow(gem_drv_priv_t *gem);
int _cfg_us_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, bdmf_object_handle tcont, uint16_t gem_port,
    bdmf_boolean calc_crc, bdmf_boolean encrpt);
int _cfg_ds_gem_flow_hw(bdmf_boolean cfg_gem, bdmf_index gem_flow, uint16_t gem_port,
    rdpa_flow_destination destination, rdpa_discard_prty discard_prty, bdmf_index ds_def_flow);
int gem_pre_init_ex(void);
int gem_attr_port_action_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, 
    const void *val, uint32_t size);
int gem_attr_port_action_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, 
    uint32_t size);
int gem_attr_ds_def_flow_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val, 
    uint32_t size);
int gem_attr_ds_def_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);
int gem_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size);
int gem_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);

#endif /* _RDPA_GEM_EX_H_ */
