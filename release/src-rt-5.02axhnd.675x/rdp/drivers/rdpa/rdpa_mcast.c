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
 * rdpa_mcast.c
 *
 *  Created on: April 30, 2013
 *      Author: mdemaria
 */

#include <bdmf_dev.h>
#include <rdd.h>
#ifdef XRDP
#include "rdpa_rdd_map.h"
#include "rdd_ucast.h"
#else
#include <rdd_ih_defs.h>
#endif
#include <rdpa_api.h>
#include "rdpa_int.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_mcast_ex.h"
#include "rdp_mm.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_flow_idx_pool.h"

#if defined(BDMF_SYSTEM_SIM)
uint32_t *sim_port_header_buffer_g = NULL;
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif

/***************************************************************************
 * mcast object type
 **************************************************************************/

#ifdef XRDP
/* 256 or 1024 possible valures as per RDPA system object */
#define RDPA_MCAST_MAX_FLOWS 256
#else
#define RDPA_MCAST_MAX_FLOWS 128
#endif

typedef struct {
    void *virt_p;
    bdmf_phys_addr_t phys_addr;
} mcast_drv_mem_info_t;

/* mcast object private data */
typedef struct {
    uint32_t num_flows;     /**< Number of configured Multicast flows */
    mcast_drv_mem_info_t mem_info[RDPA_MCAST_MAX_FLOWS];
    rdpa_flow_idx_pool_t *flow_idx_pool_p;
    flow_display_info_t  *flow_disp_info_p;
} mcast_drv_priv_t;

static struct bdmf_object *mcast_object;
static DEFINE_BDMF_FASTLOCK(mcast_lock);
static int is_idx_pool_local = 1;

/* Forward declaration */
static void __remove_all_mcast_flows(void);

/*
 * mcast memory management funtions
 */

#define RDPA_MCAST_MEM_INFO_DEBUG

static mcast_drv_mem_info_t *__alloc_mem_info(uint32_t rdpa_flow_idx)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    mcast_drv_mem_info_t *mem_info_p = &mcast->mem_info[rdpa_flow_idx]; /* OK to assign even if invalid idx */

    BUG_ON(rdpa_flow_idx >= ARRAYSIZE(mcast->mem_info));
    BUG_ON(mcast->mem_info[rdpa_flow_idx].virt_p != NULL);

    mem_info_p->virt_p = (void *)(~0);

    BDMF_TRACE_DBG_OBJ(mcast_object, "rdpa flow index %u -> mem_info assigned\n", rdpa_flow_idx);

    return mem_info_p;
}

static void __free_mem_info(uint32_t rdpa_flow_idx)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);

    BUG_ON(rdpa_flow_idx >= ARRAYSIZE(mcast->mem_info));
    BUG_ON(mcast->mem_info[rdpa_flow_idx].virt_p == NULL);

    BDMF_TRACE_DBG_OBJ(mcast_object, "rdpa flow index %u -> mem_info released\n", rdpa_flow_idx);

    mcast->mem_info[rdpa_flow_idx].virt_p = NULL;
}

static mcast_drv_mem_info_t *__get_mem_info(uint32_t rdpa_flow_idx)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    mcast_drv_mem_info_t *mem_info_p;

    BUG_ON(rdpa_flow_idx >= ARRAYSIZE(mcast->mem_info));
    BUG_ON(mcast->mem_info[rdpa_flow_idx].virt_p == NULL);

    mem_info_p = &mcast->mem_info[rdpa_flow_idx];

    BDMF_TRACE_DBG_OBJ(mcast_object, "rdpa_flow_idx %u : virt_p 0x%p, phys_addr 0x%llX\n",
                       rdpa_flow_idx, mem_info_p->virt_p, (long long unsigned int)mem_info_p->phys_addr);

    return mem_info_p;
}

/*
 * mcast object callback funtions
 */

static int mcast_pre_init(struct bdmf_object *mo)
{
    return rdpa_mcast_pre_init_ex();
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int mcast_post_init(struct bdmf_object *mo)
{
    mcast_drv_priv_t *mcast;
    int i;

    /* save pointer to the mcast object */
    mcast_object = mo;

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "mcast");

    /* Initialize Flow memory management structures */
    mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    for (i = 0; i < ARRAYSIZE(mcast->mem_info); ++i)
    {
        mcast->mem_info[i].virt_p = NULL;
    }

    if ((mcast->flow_disp_info_p && !mcast->flow_idx_pool_p) ||
        (!mcast->flow_disp_info_p && mcast->flow_idx_pool_p))
    {
        BDMF_TRACE_ERR("Index pool (%p) and Display pool (%p)\n", mcast->flow_disp_info_p, mcast->flow_idx_pool_p);
        return BDMF_ERR_INTERNAL;
    }

    /* Make sure index pool is initialized */
    if (!mcast->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        mcast->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!mcast->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");

            return BDMF_ERR_NOMEM;
        }

        err = rdpa_flow_idx_pool_init(mcast->flow_idx_pool_p, RDPA_MCAST_MAX_FLOWS, "mcast");
        if (err)
        {
            bdmf_free(mcast->flow_idx_pool_p);
            mcast->flow_idx_pool_p = NULL;
            return err;
        }

        /* initialize the Display_pool */
        mcast->flow_disp_info_p = (flow_display_info_t *)bdmf_alloc(sizeof(flow_display_info_t)*RDPA_MCAST_MAX_FLOWS);
        if (!mcast->flow_disp_info_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for flow_disp_info_p\n");
            rdpa_flow_idx_pool_exit(mcast->flow_idx_pool_p);
            bdmf_free(mcast->flow_idx_pool_p);
            return BDMF_ERR_NOMEM;
        }
        memset(mcast->flow_disp_info_p, 0, sizeof(flow_display_info_t)*RDPA_MCAST_MAX_FLOWS);

        is_idx_pool_local = 1;
    }
    else
    {
        /* Index pool already created; Must make sure it is created with enough indexes */
        if (RDPA_MCAST_MAX_FLOWS != rdpa_flow_idx_pool_get_pool_size(mcast->flow_idx_pool_p))
        {
            BDMF_TRACE_ERR("Index pool does not have enough indexes %u > %u\n", 
                           RDPA_MCAST_MAX_FLOWS, rdpa_flow_idx_pool_get_pool_size(mcast->flow_idx_pool_p));

            return BDMF_ERR_INTERNAL;
        }
        /* ASSUMPTION - Display pool is created with correct size */

        is_idx_pool_local = 0;
    }
    return rdpa_mcast_post_init_ex();
}

static void mcast_destroy(struct bdmf_object *mo)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    __remove_all_mcast_flows();

    rdpa_mcast_destroy_ex();

    if (is_idx_pool_local)
    {
        rdpa_flow_idx_pool_exit(mcast->flow_idx_pool_p);
        bdmf_free(mcast->flow_idx_pool_p);
        bdmf_free(mcast->flow_disp_info_p);
    }

    mcast_object = NULL;
}

/** find mcast object */
static int mcast_get(struct bdmf_type *drv, struct bdmf_object *owner,
                     const char *discr, struct bdmf_object **pmo)
{
    if (mcast_object == NULL)
    {
        return BDMF_ERR_NOENT;
    }

    *pmo = mcast_object;

    return 0;
}

static uint32_t __rdpa_if_mask_to_rdd_vport_mask(uint32_t rdpa_port_mask)
{
    rdpa_ports ports = (rdpa_ports)rdpa_port_mask;
    uint32_t rdd_egress_port_vector = rdpa_ports_to_rdd_egress_port_vector(ports, 0);

    rdd_egress_port_vector >>= 1;

    return (uint32_t)rdd_egress_port_vector;
}

static uint32_t __rdd_vport_mask_to_rdpa_if_mask(uint32_t rdd_port_mask)
{
    uint32_t rdd_egress_port_vector = rdd_port_mask << 1;
    rdpa_ports ports = rdpa_rdd_egress_port_vector_to_ports(rdd_egress_port_vector, 0);

    return (uint32_t)ports;
}


/*
 * mcast attribute access
 */

static void __rdd_context_create(rdd_vport_id_t rdd_vport,
                                 rdpa_mcast_flow_t *rdpa_mcast_flow,
                                 rdd_mcast_flow_t *rdd_mcast_flow,
                                 rdd_mcast_port_header_buffer_t *rdd_port_header_buffer,
                                 bdmf_boolean is_new_flow,
                                 bdmf_boolean update_port_header)
{
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(rdd_mcast_flow);
    rdpa_mcast_port_context_t *rdpa_port_context;
    rdd_mcast_port_context_t *rdd_port_context = (rdd_mcast_port_context_t *)
        &rdd_mcast_context->port_context[rdd_vport];
    rdpa_if rdpa_port;
    int rc_id, priority = 0, tc;
#if defined(XRDP)
    int rc;
#endif

    if (rdd_vport >= RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES)
    {
        BDMF_TRACE_ERR("Invalid rdd_vport %u\n", rdd_vport);

        return;
    }

    /* Data fill the Multicast RDD Context structure */

    if (is_new_flow)
    {
        memset(rdd_mcast_context, 0xFF, sizeof(rdd_mcast_flow_context_t));

        rdd_mcast_context->multicast_flag = 1;

        rdd_mcast_context->mcast_port_header_buffer_ptr = rdpa_mcast_flow->result.port_header_buffer_addr;

        rdd_mcast_context->command_list_length_64 = (rdpa_mcast_flow->result.l3_cmd_list_length + 7) >> 3;
        memcpy(rdd_mcast_context->l3_command_list,
               rdpa_mcast_flow->result.l3_cmd_list,
               RDPA_CMD_LIST_MCAST_L3_LIST_SIZE);
    }

    rdd_mcast_context->is_routed = (rdpa_mcast_flow->result.is_routed) ? 1 : 0;
    rdd_mcast_context->number_of_ports = rdpa_mcast_flow->result.number_of_ports;
    rdd_mcast_context->port_mask = __rdpa_if_mask_to_rdd_vport_mask(rdpa_mcast_flow->result.port_mask);
    rdd_mcast_context->mtu = rdpa_mcast_flow->result.mtu;
    rdd_mcast_context->is_tos_mangle = (rdpa_mcast_flow->result.is_tos_mangle) ? 1 : 0;
    rdd_mcast_context->tos = rdpa_mcast_flow->result.tos;
    rdd_mcast_context->wlan_mcast_clients = rdpa_mcast_flow->result.wlan_mcast_clients;
    rdd_mcast_context->wlan_mcast_index = rdpa_mcast_flow->result.wlan_mcast_fwd_table_index;

    rdpa_mcast_rdd_vport_to_rdpa_if_ex(rdd_vport, &rdpa_port);
    if (update_port_header)
    {
        rdpa_port_context = &rdpa_mcast_flow->result.port_context[RDPA_PORT_TO_CTX_IDX(rdpa_port)];
        rdd_port_context->state = rdpa_mcast_port_state_cmd_list;
        tc = rdpa_port_context->tc;
        rdd_port_context->queue = rdpa_port_context->queue;
#if defined(XRDP)
        /* code ignores WLAN as egress_phy, since it's not really needed.
         * TODO, maybe make it more clear and generic for all interface */
        rdd_port_context->egress_phy = rdd_egress_phy_eth_lan;
        if (rdpa_port == rdpa_if_lan6)
        {
            rdpa_wan_type egress_wan_type = rdpa_wan_if_to_wan_type(rdpa_port_context->egress_if);
            int egress_phy = rdpa_wan_type2rdd_egress_phy(egress_wan_type);
            int channel = 0;

            rdd_port_context->egress_phy = egress_phy & 0x3;
            rdd_port_context->egress_phy_ext = egress_phy >> 2;
            /* this rdd_port_context->egress_mode for DSL bonding, but unsure
             * how we can support this */
            rdd_port_context->egress_mode = rdpa_port_context->wan_flow_mode; 
            rdd_port_context->egress_port = rdpa_port_context->wan_flow;
            rc = _rdpa_egress_tm_wan_flow_queue_to_rdd(rdpa_port_context->egress_if,
                                                       rdpa_port_context->wan_flow,
                                                       rdpa_port_context->queue, &channel,
                                                       &rc_id, &priority, &tc);
            if (rc)
                BDMF_TRACE_ERR("%s: fail to _rdpa_egress_tm_wan_flow_queue_to_rdd: rc = %d\n", __func__, rc);
            rdd_port_context->queue = priority;
        }
        else
#endif
        {
            rdd_port_context->lag_port = rdpa_port_context->lag_port;
            if (_rdpa_egress_tm_lan_port_queue_to_rdd_tc_check(rdpa_port, rdpa_port_context->queue, &rc_id, &priority, &tc) == 0)
            {        
                rdd_port_context->queue = priority;
            }
        }
        rdd_port_context->is_wred_high_prio = tc;
        rdd_port_context->l2_command_list_length = rdpa_port_context->l2_command_list_length;
        rdd_port_context->l2_header_length = rdpa_port_context->l2_header_length;
        rdd_port_context->l2_push = rdpa_port_context->l2_push;
        rdd_port_context->l2_offset = rdpa_port_context->l2_offset;

        rdpa_mcast_rdd_port_header_buffer_set_ex(rdd_vport,
                                                 rdpa_port_context,
                                                 rdd_port_header_buffer,
                                                 rdd_port_context);
    }
}

/* "flow" attribute "read" callback */
static int mcast_attr_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                bdmf_index index, void *val, uint32_t size)
{
    rdpa_mcast_flow_t *rdpa_mcast_flow = (rdpa_mcast_flow_t *)val;
    rdd_mcast_flow_t rdd_mcast_flow;
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(&rdd_mcast_flow);
    mcast_drv_mem_info_t *mem_info_p;
    int rc;
    rdd_vport_id_t rdd_vport;
    rdpa_if rdpa_port;
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    uint32_t rdd_flow_id;
    uint32_t rdpa_flow_idx = index;
    if (rdpa_flow_idx_pool_get_id(mcast->flow_idx_pool_p, rdpa_flow_idx, &rdd_flow_id))
    {
        return BDMF_ERR_NOENT;
    }

    if (size != sizeof(rdpa_mcast_flow_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d,%d,%d>\n", __FUNCTION__,
            (int)size, (int)sizeof(rdpa_mcast_flow_t),
            (int)sizeof(rdpa_mcast_flow_key_t), (int)sizeof(rdpa_mcast_flow_result_t));
        return BDMF_ERR_NOENT;
    }

    /* Read the RDD flow context from RDD */
    rc = rdpa_mcast_rdd_context_get_ex(rdd_flow_id, &rdd_mcast_flow);
    if (rc)
    {
        return rc;
    }

    /* Data fill the RDPA structure */
    memset(rdpa_mcast_flow, 0, sizeof(rdpa_mcast_flow_t));

    mem_info_p = __get_mem_info(rdpa_flow_idx);

    rdpa_mcast_flow->result.port_mask = __rdd_vport_mask_to_rdpa_if_mask(rdd_mcast_context->port_mask);
    rdpa_mcast_flow->result.number_of_ports = rdd_mcast_context->number_of_ports;
    rdpa_mcast_flow->result.is_routed = rdd_mcast_context->is_routed;
    rdpa_mcast_flow->result.mtu = rdd_mcast_context->mtu;
    rdpa_mcast_flow->result.is_tos_mangle = rdd_mcast_context->is_tos_mangle;
    rdpa_mcast_flow->result.tos = rdd_mcast_context->tos;
    rdpa_mcast_flow->result.wlan_mcast_clients = rdd_mcast_context->wlan_mcast_clients;
    rdpa_mcast_flow->result.wlan_mcast_fwd_table_index = rdd_mcast_context->wlan_mcast_index;
    rdpa_mcast_flow->result.port_header_buffer_addr = rdd_mcast_context->mcast_port_header_buffer_ptr;
    rdpa_mcast_flow->result.port_header_buffer_virt = mem_info_p->virt_p;
#if defined(BDMF_SYSTEM_SIM)
    rdpa_mcast_flow->result.port_header_buffer_addr = (uint64_t)mem_info_p->phys_addr;
#endif

/* FIXME! */
#ifndef XRDP
#if defined(RDPA_MCAST_MEM_INFO_DEBUG)
    BUG_ON(rdpa_mcast_flow->result.port_header_buffer_addr != (uint64_t)mem_info_p->phys_addr);
#endif
#endif

    rdpa_mcast_flow->result.l3_cmd_list_length = rdd_mcast_context->command_list_length_64 << 3;
    memcpy(rdpa_mcast_flow->result.l3_cmd_list,
           rdd_mcast_context->l3_command_list,
           RDPA_CMD_LIST_MCAST_L3_LIST_SIZE);

    /* Only loop through the number of EMACs supported by the RDD 
     * Currently only LAN ports are supported but this should be modified when CPU/WLAN support is added.
     * Only 8 LAN ports supported 0-6 (ETH) + 7 (WLAN) */
    for (rdd_vport = 0; rdd_vport < RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES; ++rdd_vport)
    {
        rdpa_mcast_port_context_t *rdpa_port_context;
        rdd_mcast_port_context_t *rdd_port_context = (rdd_mcast_port_context_t *)
            &rdd_mcast_context->port_context[rdd_vport];
        rdd_mcast_port_header_buffer_t *rdd_port_header_buffer = mem_info_p->virt_p;

        rdpa_mcast_rdd_vport_to_rdpa_if_ex(rdd_vport, &rdpa_port);
        if (rdpa_port == rdpa_if_none)
        {
            continue;
        }
        rdpa_port_context = &rdpa_mcast_flow->result.port_context[RDPA_PORT_TO_CTX_IDX(rdpa_port)];

        rc = rdpa_mcast_rdd_port_header_buffer_get_ex(rdd_vport,
                                                      rdd_port_header_buffer,
                                                      rdpa_port_context,
                                                      rdd_port_context);
        if (rc == BDMF_ERR_NOENT)
        {
            continue;
        }

        if (rc)
        {
            return rc;
        }

        rdpa_port_context->state = rdd_port_context->state;
        rdpa_port_context->l2_command_list_length = rdd_port_context->l2_command_list_length;
        rdpa_port_context->l2_header_length = rdd_port_context->l2_header_length;
        rdpa_port_context->l2_push = rdd_port_context->l2_push;
        rdpa_port_context->l2_offset = rdd_port_context->l2_offset;
        rdpa_port_context->queue = rdd_port_context->queue;
        rdpa_port_context->is_wred_high_prio = rdd_port_context->is_wred_high_prio;
#if defined(XRDP)
        if (rdpa_port == rdpa_if_lan6)
        {
            int egress_phy = (rdd_port_context->egress_phy & 0x1) << 2;

            egress_phy += rdd_port_context->egress_phy & 0x3;
            rdpa_port_context->egress_if = rdpa_wan_type_to_if(rdd_egress_phy2rdpa_wan_type(egress_phy));
            rdpa_port_context->wan_flow = rdd_port_context->egress_port;
            /* this rdd_port_context->egress_mode for DSL bonding, but unsure
             * how we can support this */
            rdpa_port_context->wan_flow_mode = rdd_port_context->egress_mode;
        }
        else
#endif
        {
            rdpa_port_context->lag_port = rdd_port_context->lag_port;
        }
    }

    /* Read the RDD flow key from RDD */
    rc = rdpa_mcast_rdd_key_get_ex(rdd_flow_id, rdpa_mcast_flow, &rdd_mcast_flow);

    /* Copy the SIP/DIP */
    memcpy(&rdpa_mcast_flow->key.src_ip, &mcast->flow_disp_info_p[rdpa_flow_idx].l3.sip, sizeof(bdmf_ip_t));
    memcpy(&rdpa_mcast_flow->key.dst_ip, &mcast->flow_disp_info_p[rdpa_flow_idx].l3.dip, sizeof(bdmf_ip_t));

    rdpa_mcast_flow->hw_flow_id = rdd_flow_id;

    return rc;
}

/* "flow" attribute write callback */
static int mcast_attr_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad,
                                 bdmf_index index, const void *val, uint32_t size)
{
    rdpa_mcast_flow_t *rdpa_mcast_flow = (rdpa_mcast_flow_t *)val;
    rdd_mcast_flow_t rdd_mcast_flow;
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(&rdd_mcast_flow);
    bdmf_boolean update_port_header = 1;
    uint32_t port_mask_mask = (uint32_t) ~0;
    /* Though lan0 EMAC starts with enumeration value 1 but we take it from 0 */
    rdd_vport_id_t rdd_vport;
    /* This variable holds the RDPA port mask in RDD way. The name may be little weird */
    uint32_t rdpa_port_mask_as_rdd = 0; 
    int rc;
    uint32_t rdpa_flow_idx = index;
    uint32_t rdd_flow_id;
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);

    if (mo->state != bdmf_state_active)
    {
        return BDMF_ERR_INVALID_OP;
    }
    if (size != sizeof(rdpa_mcast_flow_t))
    {
        BDMF_TRACE_ERR("%s() : Size difference <%d:%d,%d,%d>\n",
            __FUNCTION__, (int)size, (int)sizeof(rdpa_mcast_flow_t),
            (int)sizeof(rdpa_mcast_flow_key_t),
            (int)sizeof(rdpa_mcast_flow_result_t));
        return BDMF_ERR_NOENT;
    }

    if (rdpa_flow_idx_pool_get_id(mcast->flow_idx_pool_p, rdpa_flow_idx, &rdd_flow_id))
    {
        return BDMF_ERR_NOENT;
    }
    /* Read the flow context from RDD */
    rc = rdpa_mcast_rdd_context_get_ex(rdd_flow_id, &rdd_mcast_flow);
    if (rc)
    {
        return rc;
    }

#if !defined(XRDP)
    if (rdpa_mcast_flow->result.wlan_mcast_clients > rdd_mcast_context->wlan_mcast_clients)
    {
        /* Add WLAN Client */

        rdd_vport = RDD_WLAN0_VPORT - 1;  /* Notice the -1 here */
    }
    else if (rdpa_mcast_flow->result.wlan_mcast_clients < rdd_mcast_context->wlan_mcast_clients)
    {
        /* Remove WLAN Client */

        rdd_vport = RDD_WLAN0_VPORT - 1;  /* Notice the -1 here */

        update_port_header = 0;
    }
    else
#endif
    {
        /* RDP: Ethernet Clients, XRDP: Ethernet and WLAN Clients */

        /* Convert the rdpa_port_mask to rdd for easier comparision later in the loop. */
        rdpa_port_mask_as_rdd = __rdpa_if_mask_to_rdd_vport_mask(rdpa_mcast_flow->result.port_mask);

        for (rdd_vport = 0; rdd_vport < RDD_MCAST_MAX_PORT_CONTEXT_ENTRIES-1; ++rdd_vport)
        {
            uint32_t rdd_port_mask = rdd_mcast_context->port_mask & (1 << rdd_vport); 
            uint32_t rdpa_port_mask = rdpa_port_mask_as_rdd & (1 << rdd_vport);

            port_mask_mask &= ~(1 << rdd_vport);

            if (rdd_port_mask ^ rdpa_port_mask)
            {
                if ((rdd_mcast_context->port_mask & port_mask_mask) !=
                    (rdpa_port_mask_as_rdd & port_mask_mask))
                {
                    BDMF_TRACE_ERR("Multiple port changes detected: "
                                   "flow <%u>, old port_mask 0x%02X, new port_mask 0x%02X\n",
                                   rdpa_flow_idx, rdd_mcast_context->port_mask, rdpa_port_mask_as_rdd);

                    return BDMF_ERR_INVALID_OP;
                }

                if (rdd_port_mask)
                {
                    /* Remove port from Multicast flow */
                    BDMF_TRACE_DBG_OBJ(mo, "Remove port <%u> from Multicast Flow <%u>\n", rdd_vport, rdpa_flow_idx);

                    /* The port header should not be updated on port removal to avoid race
                       conditions with Runner, which can also update it when the port is a member */
                    update_port_header = 0;
                }
                else
                {
                    /* Add port to Multicast flow */
                    BDMF_TRACE_DBG_OBJ(mo, "Add port <%u> to Multicast Flow <%u>\n", rdd_vport, rdpa_flow_idx);
                }

                break;
            }
        }
    }

    /* Prepare the connection context to configure in RDD */

    {
        mcast_drv_mem_info_t *mem_info_p = __get_mem_info(rdpa_flow_idx);

        __rdd_context_create(rdd_vport, rdpa_mcast_flow, &rdd_mcast_flow, mem_info_p->virt_p, 0, update_port_header);
    }

    /* Modify the flow in RDD */
    rc = rdpa_mcast_rdd_context_modify_ex(rdd_flow_id, &rdd_mcast_flow);

    return rc;
}

/* "flow" attribute add callback */
static int mcast_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad,
                               bdmf_index *index, const void *val, uint32_t size)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    rdpa_mcast_flow_t *rdpa_mcast_flow = (rdpa_mcast_flow_t *)val;
    rdd_mcast_flow_t rdd_mcast_flow = {};
    void *port_header_buffer = NULL;
    rdpa_if rdpa_if_port;
    rdd_vport_id_t rdd_vport;
    bdmf_phys_addr_t phys_addr;
    int rc;
    uint32_t rdd_flow_id;
    uint32_t rdpa_flow_idx;
    /* Allocate Port Header buffer entry */

    rdpa_if_port = ffs(rdpa_mcast_flow->result.port_mask) - 1;

    if (RDPA_PORT_TO_CTX_IDX(rdpa_if_port) >= rdpa_if_max_mcast_port)
    {
        BDMF_TRACE_ERR("Port is out of range, port = %u\n", rdpa_if_port);

        return BDMF_ERR_RANGE;
    }

    rdpa_mcast_rdpa_if_to_rdd_vport_ex(rdpa_if_port, &rdd_vport);

    /* Allocate memory for the flow Port Header buffer */

    port_header_buffer = rdp_mm_aligned_alloc_atomic(sizeof(rdd_mcast_port_header_buffer_t), &phys_addr);
    if (port_header_buffer == NULL)
    {
        BDMF_TRACE_ERR("Could not allocate Port Header Buffer\n");

        return BDMF_ERR_NOMEM;
    }
    rdpa_mcast_flow->result.port_header_buffer_addr = (uint64_t)phys_addr;

#if defined(BDMF_SYSTEM_SIM)
    sim_port_header_buffer_g = port_header_buffer;
#endif

    BDMF_TRACE_DBG_OBJ(mo, "port_header_buffer %p, rdpa_mcast_flow->result.port_header_buffer_addr 0x%llx\n",
                       port_header_buffer, (long long unsigned int)rdpa_mcast_flow->result.port_header_buffer_addr);

    /* Initialize the Port Header buffer */

    memset(port_header_buffer, 0xFF, sizeof(rdd_mcast_port_header_buffer_t));

    /* Prepare the connection context to configure in RDD */
    __rdd_context_create(rdd_vport, rdpa_mcast_flow,
                         &rdd_mcast_flow, port_header_buffer, 1, 1);

    /* Prepare the connection key to configure in RDD */
    rdpa_mcast_rdd_key_create_ex(rdpa_mcast_flow, &rdd_mcast_flow);

    /* Add the Multicast flow to RDD */
    rc = rdpa_mcast_rdd_flow_add_ex(index, &rdd_mcast_flow);
    if (rc)
    {
        rdp_mm_aligned_free(port_header_buffer, sizeof(rdd_mcast_port_header_buffer_t));

        if (rc == BDMF_ERR_IGNORE) 
        {
            /* Hash collisions are ignored. Note that index will stay FHW_TUPLE_INVALID */
            return 0;
        }
        return rc;
    }

    rdd_flow_id = *index;

    if (rdpa_flow_idx_pool_get_index(mcast->flow_idx_pool_p, &rdpa_flow_idx))
    {
        rdpa_mcast_rdd_flow_delete_ex(rdd_flow_id);
        rdp_mm_aligned_free(port_header_buffer, sizeof(rdd_mcast_port_header_buffer_t));
        return BDMF_ERR_NORES;
    }
    if (rdpa_flow_idx_pool_set_id(mcast->flow_idx_pool_p, rdpa_flow_idx, rdd_flow_id) != 0)
    {
        rdpa_mcast_rdd_flow_delete_ex(rdd_flow_id);
        rdpa_flow_idx_pool_return_index(mcast->flow_idx_pool_p, rdpa_flow_idx);
        rdp_mm_aligned_free(port_header_buffer, sizeof(rdd_mcast_port_header_buffer_t));
        return BDMF_ERR_NORES;
    }

    *index = rdpa_flow_idx;

    /* Save flow memory allocation info */
    {
        mcast_drv_mem_info_t *mem_info_p = __alloc_mem_info(rdpa_flow_idx);

        mem_info_p->virt_p = port_header_buffer;
        mem_info_p->phys_addr = phys_addr;
    }
    /* Store the SIP/DIP */
    memcpy(&mcast->flow_disp_info_p[rdpa_flow_idx].l3.sip, &rdpa_mcast_flow->key.src_ip, sizeof(bdmf_ip_t));
    memcpy(&mcast->flow_disp_info_p[rdpa_flow_idx].l3.dip, &rdpa_mcast_flow->key.dst_ip, sizeof(bdmf_ip_t));

    bdmf_fastlock_lock(&mcast_lock);
    mcast->num_flows++;
    bdmf_fastlock_unlock(&mcast_lock);

    return 0;
}

/* "flow" attribute delete callback */
static int mcast_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    rdd_mcast_flow_t rdd_mcast_flow;
    mcast_drv_mem_info_t *mem_info_p;
    int rc;
    uint32_t rdd_flow_id;
    uint32_t rdpa_flow_idx = index;

    if (rdpa_flow_idx_pool_get_id(mcast->flow_idx_pool_p, rdpa_flow_idx, &rdd_flow_id))
    {
        return BDMF_ERR_NOENT;
    }
    /* Read the RDD flow context from RDD */
    rc = rdpa_mcast_rdd_context_get_ex(rdd_flow_id, &rdd_mcast_flow);
    if (rc)
    {
        return rc;
    }

    /* Free the flow Port header buffer */

    mem_info_p = __get_mem_info(rdpa_flow_idx);

    rdp_mm_aligned_free(mem_info_p->virt_p, sizeof(rdd_mcast_port_header_buffer_t));

    __free_mem_info(rdpa_flow_idx);

    /* Delete the flow from RDD */

    rc = rdpa_mcast_rdd_flow_delete_ex(rdd_flow_id);
    if (rc)
    {
        return rc;
    }

    bdmf_fastlock_lock(&mcast_lock);
    mcast->num_flows--;
    bdmf_fastlock_unlock(&mcast_lock);

    if (rdpa_flow_idx_pool_return_index(mcast->flow_idx_pool_p, rdpa_flow_idx))
    {
        /* Should NEVER happen */
    }
    memset(&mcast->flow_disp_info_p[rdpa_flow_idx], 0, sizeof(flow_display_info_t));
    return 0;
}

/* "flow" attribute find callback.
 * Updates *index, can update *val as well
 */
static int mcast_attr_flow_find(struct bdmf_object *mo, struct bdmf_attr *ad,
                                bdmf_index *index, void *val, uint32_t size)
{
    rdpa_mcast_flow_t *rdpa_mcast_flow = (rdpa_mcast_flow_t *)val;
    rdd_mcast_flow_t rdd_mcast_flow = {};
    int rc;
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    uint32_t rdpa_flow_idx = 0;
    uint32_t rdd_flow_id;

    rdpa_mcast_rdd_key_create_ex(rdpa_mcast_flow, &rdd_mcast_flow);

    rc = rdpa_mcast_rdd_flow_find_ex(index, &rdd_mcast_flow);
    rdd_flow_id = *index;

    if (!rc && rdpa_flow_idx_pool_reverse_get_index(mcast->flow_idx_pool_p, &rdpa_flow_idx, rdd_flow_id))
    {
        return BDMF_ERR_NOENT;
    }
    *index = rdpa_flow_idx;
    return rc;
}

/* "flow_stats" attribute "read" callback */
static int mcast_attr_flow_stats_read(struct bdmf_object *mo, struct bdmf_attr *ad,
                                      bdmf_index index, void *val, uint32_t size)
{
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    rdd_mcast_flow_t rdd_mcast_flow;
    rdd_mcast_flow_context_t *rdd_mcast_context =
        RDD_MCAST_FLOW_CONTEXT_GET_EX(&rdd_mcast_flow);
    int rc;
    mcast_drv_priv_t *mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);
    uint32_t rdpa_flow_idx = index;
    uint32_t rdd_flow_id;

    if (rdpa_flow_idx_pool_get_id(mcast->flow_idx_pool_p, rdpa_flow_idx, &rdd_flow_id))
    {
        return BDMF_ERR_NOENT;
    }

    /* Read the flow stats from the RDD */
    rc = rdpa_mcast_rdd_flow_stats_get_ex(rdd_flow_id, &rdd_mcast_flow);
    if (rc)
    {
        return rc;
    }

    stat->packets = rdd_mcast_context->flow_hits;
    stat->bytes = rdd_mcast_context->flow_bytes;

    return rc;
}

/*  ip_flow_key aggregate type */
struct bdmf_aggr_type mcast_flow_key_type = {
    .name = "mcast_flow_key", .struct_name = "rdpa_mcast_flow_key_t",
    .help = "Multicast Flow Key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "src_ip", .help = "Source IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mcast_flow_key_t, src_ip)
        },
        { .name = "dst_ip", .help = "Destination IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_mcast_flow_key_t, dst_ip)
        },
#ifndef XRDP
        { .name = "protocol", .help = "IP protocol", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_key_t, protocol),
            .flags = BDMF_ATTR_UNSIGNED
        },
#endif
        { .name = "num_vlan_tags", .help = "Number of VLAN Tags", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_key_t, num_vlan_tags),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "outer_vlan_id", .help = "Outer VLAN ID (0xFFFF == *)", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_key_t, outer_vlan_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "inner_vlan_id", .help = "Inner VLAN ID (0xFFFF == *)", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_key_t, inner_vlan_id),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "rx_if", .help = "Received interface", .size = sizeof(rdpa_if),
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .offset = offsetof(rdpa_mcast_flow_key_t, rx_if)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_flow_key_type);

const bdmf_attr_enum_table_t rdpa_mcast_port_state_enum_table =
{
    .type_name = "rdpa_mcast_port_state", .help = "Port State",
    .values = {
        {"CMD", rdpa_mcast_port_state_cmd_list},
        {"HDR", rdpa_mcast_port_state_header},
        {NULL, 0}
    }
};

/*  ip_flow_key aggregate type */
struct bdmf_aggr_type mcast_port_context_type = {
    .name = "mcast_port_context", .struct_name = "rdpa_mcast_port_context_t",
    .help = "Multicast Port Context",
    .fields = (struct bdmf_attr[])
    {
        { .name = "state", .help = "Port State", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_mcast_port_state_enum_table,
            .size = sizeof(rdpa_mcast_port_state_t), .offset = offsetof(rdpa_mcast_port_context_t, state)
        },
        { .name = "port_header", .help = "L2 Command List / L2 Header", .size = sizeof(rdpa_mcast_port_header_t),
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_mcast_port_context_t, port_header)
        },
        { .name = "l2_command_list_length", .help = "L2 Command List length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, l2_command_list_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l2_header_length", .help = "Tx L2 Header length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, l2_header_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l2_push", .help = "Tx L2 Header Push/Pull flag", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, l2_push),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l2_offset", .help = "Tx L2 Header Offset", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, l2_offset),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "queue", .help = "Egress Queue", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, queue),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "lag_port", .help = "LAG Port", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, lag_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_wred_high_prio", .help = "1: High Priority for WRED; 0: Low Priority", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, is_wred_high_prio),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "egress_if", .help = "Egress Interface",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_mcast_port_context_t, egress_if)
        },
        { .name = "wan_flow", .help = "DSL ATM/PTM US channel", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, wan_flow),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wan_flow_mode", .help = "xDSL PTM bonded or single", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_port_context_t, wan_flow_mode),
            .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_port_context_type);

/*  ip_flow_result aggregate type */
struct bdmf_aggr_type mcast_flow_result_type = 
{
    .name = "mcast_flow_result", .struct_name = "rdpa_mcast_flow_result_t",
    .help = "Multicast Flow Result",
    .size = sizeof(rdpa_mcast_flow_result_t),
    .fields = (struct bdmf_attr[])
    {
        {.name = "rdpa_ifs", .help = "Bit mask of ports that joined this flow", .size = sizeof(uint32_t),
            .type = bdmf_attr_enum_mask, .ts.enum_table = &rdpa_if_enum_table, 
            .offset = offsetof(rdpa_mcast_flow_result_t, port_mask)
        },
        { .name = "number_of_ports", .help = "Number of ports that joined this flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, number_of_ports),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_routed", .help = "1: Routed Flow; 0: Bridged Flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, is_routed),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "mtu", .help = "Egress Port MTU", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, mtu),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "is_tos_mangle", .help = "1: Mangle ToS; 0: No Mangle ToS", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, is_tos_mangle),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "tos", .help = "Rx ToS", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, tos),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "port_header_buffer_addr", .help = "Port Header Buffer Physical Address", .size = sizeof(uint64_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, port_header_buffer_addr),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "port_header_buffer_virt", .help = "Port Header Buffer Virtual Address", .size = sizeof(void *),
            .type = bdmf_attr_pointer, .offset = offsetof(rdpa_mcast_flow_result_t, port_header_buffer_virt),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT
        },
        { .name = "wlan_mcast_clients", .help = "WLAN Multicast Forwarding Table Clients", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, wlan_mcast_clients),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "wlan_mcast_fwd_table_index", .help = "WLAN Multicast Forwarding Table Index", .size = sizeof(bdmf_index),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, wlan_mcast_fwd_table_index),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l3_cmd_list_length", .help = "L3 Command List Length", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_mcast_flow_result_t, l3_cmd_list_length),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "l3_cmd_list", .help = "L3 Command List", .size = RDPA_CMD_LIST_MCAST_L3_LIST_SIZE,
            .type = bdmf_attr_buffer, .offset = offsetof(rdpa_mcast_flow_result_t, l3_cmd_list)
        },
        { .name = "lan0_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan0])
        },
        { .name = "lan1_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan1])
        },
        { .name = "lan2_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan2])
        },
        { .name = "lan3_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan3])
        },
        { .name = "lan4_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan4])
        },
        { .name = "lan5_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan5])
        },
        { .name = "lan6_context", .help = "Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan6])
        },
        { .name = "lan7_context", .help = "WLAN Port Context", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_port_context",
            .offset = offsetof(rdpa_mcast_flow_result_t, port_context[rdpa_if_lan7])
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_flow_result_type);

/*  ip_flow_info aggregate type */
struct bdmf_aggr_type mcast_flow_type = {
    .name = "mcast_flow", .struct_name = "rdpa_mcast_flow_t",
    .help = "Multicast Flow (key+result)",
    .size = sizeof(rdpa_mcast_flow_t),
    .fields = (struct bdmf_attr[]) {
        { .name = "hw_id", .help = "HW Flow ID",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_mcast_flow_t, hw_flow_id)
        },
        { .name = "key", .help = "Multicast flow key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_flow_key", .offset = offsetof(rdpa_mcast_flow_t, key)
        },
        { .name = "result", .help = "Multicast flow result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "mcast_flow_result", .offset = offsetof(rdpa_mcast_flow_t, result)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mcast_flow_type);

/* Object attribute descriptors */
static struct bdmf_attr mcast_attrs[] = {
    { .name = "nflows", .help = "Number of configured Multicast flows",
      .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
      .size = sizeof(uint32_t), .offset = offsetof(mcast_drv_priv_t, num_flows)
    },
    { .name = "flow_idx_pool_ptr", .help = "Flow ID Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(mcast_drv_priv_t, flow_idx_pool_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow_disp_pool_ptr", .help = "Flow Display Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(mcast_drv_priv_t, flow_disp_info_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow", .help = "Multicast flow entry",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "mcast_flow", .array_size = RDPA_MCAST_MAX_FLOWS,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
      .read = mcast_attr_flow_read, .write = mcast_attr_flow_write,
      .add = mcast_attr_flow_add, .del = mcast_attr_flow_delete,
      .find = mcast_attr_flow_find
    },
    { .name = "flow_stat", .help = "Multicast flow entry statistics",
      .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_MCAST_MAX_FLOWS,
      .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
      .read = mcast_attr_flow_stats_read
    },
    BDMF_ATTR_LAST
};


static int mcast_drv_init(struct bdmf_type *drv);
static void mcast_drv_exit(struct bdmf_type *drv);

struct bdmf_type mcast_drv = {
    .name = "mcast",
    .parent = "system",
    .description = "Multicast Flow Manager",
    .drv_init = mcast_drv_init,
    .drv_exit = mcast_drv_exit,
    .pre_init = mcast_pre_init,
    .post_init = mcast_post_init,
    .destroy = mcast_destroy,
    .get = mcast_get,
    .extra_size = sizeof(mcast_drv_priv_t),
    .aattr = mcast_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_mcast, mcast_drv);

/* Init module. Cater for GPL layer */
static int mcast_drv_init(struct bdmf_type *drv)
{
    rdpa_mcast_flow_result_t dummy;

/* FIXME! */
#ifndef XRDP
    uint32_t rdd_mcast_l3_list_offset = offsetof(rdd_mcast_flow_context_t,
                                                 l3_command_list);

    if (RDPA_CMD_LIST_MCAST_L3_LIST_OFFSET != rdd_mcast_l3_list_offset)
    {
        BDMF_TRACE_ERR("MCAST_L3_LIST_OFFSET mismatch: RDPA %u, RDD %u",
                       RDPA_CMD_LIST_MCAST_L3_LIST_OFFSET, rdd_mcast_l3_list_offset);

        return BDMF_ERR_INTERNAL;
    }
#endif

#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mcast_drv = rdpa_mcast_drv;
    f_rdpa_mcast_get = rdpa_mcast_get;
#endif
    /* Multicast flow has port_mask for rdpa_if that could include all lan, wlan and cpu port */
    BUG_ON((sizeof(dummy.port_mask)*8) < rdpa_if_max_mcast_port);

    return 0;
}

/* Exit module. Cater for GPL layer */
static void mcast_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mcast_drv = NULL;
    f_rdpa_mcast_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get mcast object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_mcast_get(bdmf_object_handle *_obj_)
{
    if (!mcast_object || mcast_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;

    bdmf_get(mcast_object);

    *_obj_ = mcast_object;

    return 0;
}

static void __remove_all_mcast_flows(void)
{
    uint32_t rdpa_flow_idx, rdpa_flow_idx_max;
    mcast_drv_priv_t *mcast = NULL;

    if (mcast_object == NULL)
    {
        return;
    }

    mcast = (mcast_drv_priv_t *)bdmf_obj_data(mcast_object);

    rdpa_flow_idx_max = rdpa_flow_idx_pool_get_pool_size(mcast->flow_idx_pool_p);

    for (rdpa_flow_idx = 0; rdpa_flow_idx < rdpa_flow_idx_max; ++rdpa_flow_idx)
    {
        mcast_attr_flow_delete(mcast_object, NULL, rdpa_flow_idx);
    }
}

