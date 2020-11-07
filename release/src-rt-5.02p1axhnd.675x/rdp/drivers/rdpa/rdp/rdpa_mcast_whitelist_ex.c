/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
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
 * :> 
 */

#include <bdmf_dev.h>
#include <rdd.h>
#include <rdpa_api.h>
#include "rdpa_mcast_whitelist_ex.h"

int rdpa_mcast_whitelist_pre_init_ex(void)
{
    return BDMF_ERR_OK;
}

int rdpa_mcast_whitelist_post_init_ex(void)
{
    return BDMF_ERR_OK;
}

void rdpa_mcast_whitelist_destroy_ex(void)
{
    return;
}

/* "whitelist" attribute "read" callback */
int mcast_attr_whitelist_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                 bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist" attribute "add" callback */
int mcast_attr_whitelist_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                bdmf_index *index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist" attribute "del" callback */
int mcast_attr_whitelist_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                   bdmf_index index)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist" attribute "find" callback */
int mcast_attr_whitelist_find_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                 bdmf_index *index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist enable_port" attribute "read" callback */
int mcast_attr_whitelist_enable_port_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist enable_port" attribute "write" callback */
int mcast_attr_whitelist_enable_port_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                              bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist stat" attribute "read" callback */
int mcast_attr_whitelist_stats_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                       bdmf_index index, void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

/* "whitelist global_stat" attribute read callback */
int mcast_attr_whitelist_global_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index index, void *val, uint32_t size);
{
    return BDMF_ERR_NOT_SUPPORTED;
}

