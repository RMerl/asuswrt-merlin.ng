/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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


#include "rdd.h"
#include "rdd_tcam_ic.h"
#include "rdd_data_structures_auto.h"
#include "rdp_drv_proj_cntr.h"
#include "rdd_ic_common.h"

/* RDD Actions - Different from RDPA actions (IPV6 masked are RDD only, ANY is RDPA only)*/
typedef enum {
    /* 8 bytes fields */
    RDD_IC_FIRST_8_BYTE_KEY,
    RDD_IC_SRC_IPV6_MASKED = RDD_IC_FIRST_8_BYTE_KEY,   /* RDD only */
    RDD_IC_DST_IPV6_MASKED,                             /* RDD only */
    /* 4 bytes fields */
    RDD_IC_FIRST_4_BYTE_KEY,
    RDD_IC_SRC_IP = RDD_IC_FIRST_4_BYTE_KEY,
    RDD_IC_DST_IP,
    RDD_IC_IPV6_FLOW_LABEL,
    /* Generic ic values used also to field id */
    RDD_IC_GENERIC_0,
    RDD_IC_GENERIC_L2 = RDD_IC_GENERIC_0,
    RDD_IC_GENERIC_1,
    RDD_IC_GENERIC_L3 = RDD_IC_GENERIC_1,
    RDD_IC_GENERIC_2,
    RDD_IC_GENERIC_L4 = RDD_IC_GENERIC_2,
    RDD_IC_GENERIC_3,
    /* 2 bytes fields */
    RDD_IC_FIRST_2_BYTE_KEY,
    RDD_IC_OUTER_TPID = RDD_IC_FIRST_2_BYTE_KEY,
    RDD_IC_INNER_TPID,
    RDD_IC_SRC_PORT,
    RDD_IC_DST_PORT,
    RDD_IC_OUTER_VID,
    RDD_IC_INNER_VID,
    RDD_IC_DST_MAC,
    RDD_IC_SRC_MAC,
    RDD_IC_ETHER_TYPE,
    /*1 byte fields*/
    RDD_IC_FIRST_1_BYTE_KEY,
    RDD_IC_IP_PROTOCOL = RDD_IC_FIRST_1_BYTE_KEY,
    RDD_IC_TOS,
    RDD_IC_DSCP,
    RDD_IC_SSID,
    RDD_IC_INGRESS_PORT,
    RDD_IC_OUTER_PBIT,
    RDD_IC_INNER_PBIT,
    RDD_IC_NUM_OF_VLANS,
    RDD_IC_L3_PROTOCOL,
    RDD_IC_GEM_FLOW,
    RDD_IC_NETWORK_LAYER,
    RDD_IC_LAST_KEY,
} rdd_ic_value;

/** Ingress classification RDD rule mask fields bitmask */
typedef enum
{
    RDD_IC_MASK_SRC_IPV6_MASKED = (1 << RDD_IC_SRC_IPV6_MASKED),
    RDD_IC_MASK_DST_IPV6_MASKED = (1 << RDD_IC_DST_IPV6_MASKED),
    RDD_IC_MASK_SRC_IP          = (1 << RDD_IC_SRC_IP),
    RDD_IC_MASK_DST_IP          = (1 << RDD_IC_DST_IP),
    RDD_IC_MASK_IPV6_FLOW_LABEL = (1 << RDD_IC_IPV6_FLOW_LABEL),
    RDD_IC_MASK_GENERIC_0       = (1 << RDD_IC_GENERIC_0),
    RDD_IC_MASK_GENERIC_1       = (1 << RDD_IC_GENERIC_1),
    RDD_IC_MASK_GENERIC_2       = (1 << RDD_IC_GENERIC_2),
    RDD_IC_MASK_GENERIC_3       = (1 << RDD_IC_GENERIC_3),
    RDD_IC_MASK_OUTER_TPID      = (1 << RDD_IC_OUTER_TPID),
    RDD_IC_MASK_INNER_TPID      = (1 << RDD_IC_INNER_TPID),
    RDD_IC_MASK_SRC_PORT        = (1 << RDD_IC_SRC_PORT),
    RDD_IC_MASK_DST_PORT        = (1 << RDD_IC_DST_PORT),
    RDD_IC_MASK_OUTER_VID       = (1 << RDD_IC_OUTER_VID),
    RDD_IC_MASK_INNER_VID       = (1 << RDD_IC_INNER_VID),
    RDD_IC_MASK_DST_MAC         = (1 << RDD_IC_DST_MAC),
    RDD_IC_MASK_SRC_MAC         = (1 << RDD_IC_SRC_MAC),
    RDD_IC_MASK_ETHER_TYPE      = (1 << RDD_IC_ETHER_TYPE),
    RDD_IC_MASK_IP_PROTOCOL     = (1 << RDD_IC_IP_PROTOCOL),
    RDD_IC_MASK_TOS             = (1 << RDD_IC_TOS),
    RDD_IC_MASK_DSCP            = (1 << RDD_IC_DSCP),
    RDD_IC_MASK_SSID            = (1 << RDD_IC_SSID),
    RDD_IC_MASK_INGRESS_PORT    = (1 << RDD_IC_INGRESS_PORT),
    RDD_IC_MASK_OUTER_PBIT      = (1 << RDD_IC_OUTER_PBIT),
    RDD_IC_MASK_INNER_PBIT      = (1 << RDD_IC_INNER_PBIT),
    RDD_IC_MASK_NUM_OF_VLANS    = (1 << RDD_IC_NUM_OF_VLANS),
    RDD_IC_MASK_L3_PROTOCOL     = (1 << RDD_IC_L3_PROTOCOL),
    RDD_IC_MASK_GEM_FLOW        = (1 << RDD_IC_GEM_FLOW),
    RDD_IC_MASK_NETWORK_LAYER   = (1 << RDD_IC_NETWORK_LAYER),
} rdd_ic_fields;

/* Max number of FW commands in a single table, including terminator command */
#define RDD_TCAM_MAX_CMD_SET_SIZE         RDD_TCAM_IC_CMD_TABLE_SIZE2

/* Max command transfer size (bytes) */
#define RDD_TCAM_MAX_CMD_SIZE             8

extern uint32_t table_id_to_ref_cnt_index[TCAM_IC_MODULE_LAST + 1];

/*
 * Static (constant) IC field info
 */
typedef struct rdd_tcam_field_info
{
    uint16_t offset;                    /* Byte offset from the source */
    uint16_t size;                      /* Field size */
    uint32_t mask;                      /* Field mask - only for fields <= 4 bytes */
    rdd_ic_value field_id;              /* Field id */
} rdd_tcam_field_info_t;

/*
 * Dynamic IC field info
 */
typedef struct rdd_tcam_field
{
    uint16_t offset;            /* Byte offset in TCAM record */
    uint16_t use_count;         /* Field use count */
} rdd_tcam_field_t;

/* F/W command (host side representation) */
typedef struct rdd_tcam_fw_cmd
{
    uint16_t src_offset;                /* Byte offset from the source */
    uint8_t tcam_offset;                /* Byte offset in TCAM record */
    rdd_ic_value field_id;              /* RDD Field id */
} rdd_tcam_fw_cmd_t;

/* IC table control block - All fields related to tcam table are in RDD level */
typedef struct rdd_tcam_table
{
    rdd_tcam_table_id id;                                       /* table id */
    rdd_ic_fields all_fields_mask;                              /* All used fields mask */
    rdd_tcam_field_t fields[RDD_IC_LAST_KEY];                   /* Field data */
    rdd_tcam_field_info_t gen_fields[RDD_TCAM_MAX_GEN_FIELDS];  /* Generic fields info */
    rdp_tcam_key_area_t used_mask;                              /* Allocated field mask */
    rdd_tcam_fw_cmd_t cmds[RDD_TCAM_MAX_CMD_SET_SIZE - 1];      /* command set, excluding terminator command */
    uint32_t cmd_table_size;                                    /* command table size (in entries) including terminating entry */
    uint32_t active_cmd_table;                                  /* active table index in command table array */
    uint32_t key_size;                                          /* Key size: 32 / 64 bytes */
    uint16_t num_cmds;                                          /* Number of commands in cmds[] array */
    uint16_t max_cmds;                                          /* Max number of commands in cmds[] array */
    const rdd_module_t *module;                                 /* F/W module structure */
    uint32_t classification_result;                             /* invert_match bit action, reason, counter offset */
    rdd_tcam_ic_module module_id;                               /* FW module id */
} rdd_tcam_table_t;

/* Convert layer to fields id for generic filters */
static const rdd_ic_value rdd_tcam_layer_to_field_id[] =
{
    [RDPA_OFFSET_L2] = RDD_IC_GENERIC_L2,
    [RDPA_OFFSET_L3] = RDD_IC_GENERIC_L3,
    [RDPA_OFFSET_L4] = RDD_IC_GENERIC_L4,
};

/* Field-info table.
 */
static const rdd_tcam_field_info_t rdd_tcam_fields[] =
{
    [RDD_IC_L3_PROTOCOL] = { .size = 1, .mask = 0x30, .field_id = RDD_IC_L3_PROTOCOL },
    [RDD_IC_IP_PROTOCOL] = { .size = 1, .mask = 0xff, .field_id = RDD_IC_IP_PROTOCOL },
    [RDD_IC_TOS] = { .size = 1, .mask = 0xff, .field_id = RDD_IC_TOS },
    [RDD_IC_DSCP] = { .size = 1, .mask = 0xfc, .field_id = RDD_IC_DSCP },
    [RDD_IC_SRC_IP] = { .size = 4, .mask = 0xffffffff, .field_id = RDD_IC_SRC_IP },
    [RDD_IC_SRC_IPV6_MASKED] = { .size = 8, .field_id = RDD_IC_SRC_IPV6_MASKED },
    [RDD_IC_DST_IP] = { .size = 4, .mask = 0xffffffff, .field_id = RDD_IC_DST_IP },
    [RDD_IC_DST_IPV6_MASKED] = { .size = 8, .field_id = RDD_IC_DST_IPV6_MASKED },
    [RDD_IC_SRC_PORT] = { .size = 2, .mask = 0xffff, .field_id = RDD_IC_SRC_PORT },
    [RDD_IC_DST_PORT] = { .size = 2, .mask = 0xffff, .field_id = RDD_IC_DST_PORT },
    [RDD_IC_INGRESS_PORT] = { .size = 1, .mask = 0xff, .field_id = RDD_IC_INGRESS_PORT },
    [RDD_IC_OUTER_VID] = { .size = 2, .mask = 0x0fff, .field_id = RDD_IC_OUTER_VID },
    [RDD_IC_INNER_VID] = { .size = 2, .mask = 0x0fff, .field_id = RDD_IC_INNER_VID },
    [RDD_IC_OUTER_PBIT] = { .size = 1, .mask = 0xe0, .field_id = RDD_IC_OUTER_PBIT },
    [RDD_IC_INNER_PBIT] = { .size = 1, .mask = 0xe0, .field_id = RDD_IC_INNER_PBIT },
    [RDD_IC_NUM_OF_VLANS] = { .size = 1, .mask = 0x03, .field_id = RDD_IC_NUM_OF_VLANS },
    [RDD_IC_OUTER_TPID] =  { .size = 2, .mask = 0xffff, .field_id = RDD_IC_OUTER_TPID },
    [RDD_IC_INNER_TPID] =  { .size = 2, .mask = 0xffff, .field_id = RDD_IC_INNER_TPID },
    [RDD_IC_DST_MAC] = { .size = 6, .field_id = RDD_IC_DST_MAC },
    [RDD_IC_SRC_MAC] = { .size = 6, .field_id = RDD_IC_SRC_MAC },
    [RDD_IC_IPV6_FLOW_LABEL] = { .size = 4, .mask = 0x000fffff, .field_id = RDD_IC_IPV6_FLOW_LABEL},
    [RDD_IC_ETHER_TYPE] = { .size = 2, .mask = 0xffff, .field_id = RDD_IC_ETHER_TYPE},
    [RDD_IC_SSID] = { },      /* ??? */
    [RDD_IC_GEM_FLOW] = { .size = 1, .mask = 0xff, .field_id = RDD_IC_GEM_FLOW },
    [RDD_IC_NETWORK_LAYER] = { .size = 1, .mask = 0xff, .field_id = RDD_IC_NETWORK_LAYER },
};

/* Addresses of command handlers per image */
static const uint32_t rdd_image_cmd_handler_addresses[][runner_image_last+1] =
{
    [RDD_IC_L3_PROTOCOL] = TCAM_CMD_L3_PROTOCOL_ADDR_ARR,
    [RDD_IC_IP_PROTOCOL] = TCAM_CMD_IP_PROTOCOL_ADDR_ARR,
    [RDD_IC_NETWORK_LAYER] = TCAM_CMD_NETWORK_LAYER_ADDR_ARR,
    [RDD_IC_TOS] = TCAM_CMD_TOS_ADDR_ARR,
    [RDD_IC_DSCP] = TCAM_CMD_TOS_ADDR_ARR,
    [RDD_IC_SRC_IP] = TCAM_CMD_SRC_IP_ADDR_ARR,
    [RDD_IC_SRC_IPV6_MASKED] = TCAM_CMD_SRC_IPV6_MASKED_ADDR_ARR,
    [RDD_IC_DST_IP] = TCAM_CMD_DST_IP_ADDR_ARR,
    [RDD_IC_DST_IPV6_MASKED] = TCAM_CMD_DST_IPV6_MASKED_ADDR_ARR,
    [RDD_IC_SRC_PORT] = TCAM_CMD_SRC_PORT_ADDR_ARR,
    [RDD_IC_DST_PORT] = TCAM_CMD_DST_PORT_ADDR_ARR,
    [RDD_IC_INGRESS_PORT] = TCAM_CMD_INGRESS_PORT_ADDR_ARR,
    [RDD_IC_OUTER_VID] = TCAM_CMD_OUTER_VID_ADDR_ARR,
    [RDD_IC_INNER_VID] = TCAM_CMD_INNER_VID_ADDR_ARR,
    [RDD_IC_OUTER_PBIT] = TCAM_CMD_OUTER_PBIT_ADDR_ARR,
    [RDD_IC_INNER_PBIT] = TCAM_CMD_INNER_PBIT_ADDR_ARR,
    [RDD_IC_NUM_OF_VLANS] = TCAM_CMD_VLAN_NUM_ADDR_ARR,
    [RDD_IC_OUTER_TPID] = TCAM_CMD_OUTER_TPID_ADDR_ARR,
    [RDD_IC_INNER_TPID] = TCAM_CMD_INNER_TPID_ADDR_ARR,
    [RDD_IC_DST_MAC] = TCAM_CMD_DST_MAC_ADDR_ARR,
    [RDD_IC_SRC_MAC] = TCAM_CMD_SRC_MAC_ADDR_ARR,
    [RDD_IC_ETHER_TYPE] = TCAM_CMD_ETHERTYPE_ADDR_ARR,
    [RDD_IC_IPV6_FLOW_LABEL] = TCAM_CMD_IPV6_LABEL_ADDR_ARR,
    [RDD_IC_GENERIC_L2] = TCAM_CMD_GENERIC_L2_ADDR_ARR,
    [RDD_IC_GENERIC_L3] = TCAM_CMD_GENERIC_L3_ADDR_ARR,
    [RDD_IC_GENERIC_L4] = TCAM_CMD_GENERIC_L4_ADDR_ARR,
    [RDD_IC_GEM_FLOW] = TCAM_CMD_GEM_FLOW_ADDR_ARR,
    [RDD_IC_LAST_KEY] = TCAM_CMD_IC_SUBMIT_ADDR_ARR,
};

/* IC tables */
static rdd_tcam_table_t rdd_tcam_tables[RDD_TCAM_MAX_TABLES];

static uint32_t shadow_cmd_table;        /* shadow table index in command table array */
static bdmf_boolean fw_initialized;


/******************************************************************************/
/*                                                                            */
/*                            F/W tables configuration helpers                */
/*                                                                            */
/******************************************************************************/

/* Update command table */
static void _rdd_tcam_fw_cmd_table_update(rdd_tcam_table_t *table)
{
    uint32_t cmd_table_address;
    uint32_t new_shadow_table;
    uint32_t cmd_entry;
    uint32_t *addr;
    uint8_t tcam_offset, src_offset;
    rdd_ic_value handler_index;
    int i, core_index;

    /* Write command table to the shadow */
    cmd_entry = shadow_cmd_table * RDD_TCAM_MAX_CMD_SET_SIZE;
    for (i = 0; i <= table->num_cmds; i++, ++cmd_entry)
    {
        /* First command include only the first handler address pointr */
        tcam_offset = (i == 0) ? 0 : table->cmds[i-1].tcam_offset;
        src_offset = (i == 0) ? 0 : table->cmds[i-1].src_offset;

        RDD_TCAM_IC_CMD_DST_OFFSET_WRITE_G(tcam_offset, RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR, cmd_entry);
        RDD_TCAM_IC_CMD_SRC_OFFSET_WRITE_G(src_offset, RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR, cmd_entry); /* Used only in generic fields */

        /* Write the ptr of next command handler address to the command */
        for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
        {
            if (rdd_image_cmd_handler_addresses[table->cmds[i].field_id][rdp_core_to_image_map[core_index]] != INVALID_LABEL_ADDRESS)
            {
                addr = (uint32_t *)(DEVICE_ADDRESS((rdp_runner_core_addr[core_index] + RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR[core_index]) + cmd_entry * sizeof(RDD_TCAM_IC_CMD_DTS)));
                handler_index = (i == table->num_cmds) ? RDD_IC_LAST_KEY : table->cmds[i].field_id;
                MWRITE_I_16(addr, 0,
                            rdd_image_cmd_handler_addresses[handler_index][rdp_core_to_image_map[core_index]] );
            }
        }
    }

    /* The table id should be written to the byte in tcam key */
    RDD_TCAM_IC_CMD_DST_OFFSET_WRITE_G(table->key_size - 1, RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR, cmd_entry);

    /* Exchange active and shadow tables */
    new_shadow_table = table->active_cmd_table;
    table->active_cmd_table = shadow_cmd_table;
    shadow_cmd_table = new_shadow_table;
    for (i = 0; i < GROUPED_EN_SEGMENTS_NUM; i++)
    {
        if (RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR[i] != INVALID_TABLE_ADDRESS)
        {
            cmd_table_address = RDD_TCAM_IC_CMD_TABLE_ADDRESS_ARR[i] + RDD_TCAM_MAX_CMD_SET_SIZE * sizeof(RDD_TCAM_IC_CMD_DTS) * table->active_cmd_table;
            RDD_TCAM_IC_CFG_CMD_TABLE_WRITE(cmd_table_address, ((uint8_t *)RDD_TCAM_IC_CFG_TABLE_PTR(i) +
		        (sizeof(RDD_TCAM_IC_CFG_DTS) * table->module_id)));
        }
    }
}

/* Update classification result */
static void _rdd_tcam_fw_classification_result_update(rdd_tcam_table_t *table)
{
    RDD_TCAM_IC_CFG_CLASSIFICATION_RESULT_WRITE_G(table->classification_result,
        table->module->cfg_ptr, table->module_id);
}

/* module init */
static void _rdd_tcam_fw_module_init(rdd_tcam_table_t *table)
{
    const rdd_tcam_table_parm_t *params = table->module->params;

    if (!fw_initialized)
    {
        shadow_cmd_table = TCAM_IC_MODULE_LAST + 1;
        fw_initialized = 1;
    }

    /* Set-up (empty) command table  */
    _rdd_tcam_fw_cmd_table_update(table);

    /* Make sure that scratch offset is 64-byte aligned */
    BUG_ON(params->scratch_offset & 63);

    /* Module init. Set up module configuration */
    RDD_TCAM_IC_CFG_RES_OFFSET_WRITE_G(table->module->res_offset, table->module->cfg_ptr, table->module_id);
    RDD_TCAM_IC_CFG_CONTEXT_OFFSET_WRITE_G(table->module->context_offset, table->module->cfg_ptr, table->module_id);
    RDD_TCAM_IC_CFG_CLASSIFICATION_RESULT_WRITE_G(table->classification_result, table->module->cfg_ptr, table->module_id);
    RDD_TCAM_IC_CFG_KEY_SIZE_WRITE_G(table->key_size, table->module->cfg_ptr, table->module_id);
    RDD_TCAM_IC_CFG_SCRATCH_OFFSET_WRITE_G(params->scratch_offset, table->module->cfg_ptr, table->module_id);
    RDD_TCAM_IC_CFG_TABLE_ID_WRITE_G(table->id, table->module->cfg_ptr, table->module_id);
}


/******************************************************************************/
/*                                                                            */
/*                            Internal helpers                                */
/*                                                                            */
/******************************************************************************/

rdd_ic_value tcam_rdpa_ic_to_rdd_ic(rdpa_ic_value rdpa_ic_field, const uint8_t gen_indexes[], const rdpa_ic_key_t *rule_key)
{
    if((rdpa_ic_field >= RDPA_IC_GENERIC_1) && (rdpa_ic_field <= RDPA_IC_GENERIC_2))
        return RDD_IC_GENERIC_0 + gen_indexes[rdpa_ic_field - RDPA_IC_GENERIC_1];
    else if ( ((rdpa_ic_field == RDPA_IC_SRC_IP) && (rule_key->src_ip_mask.family == bdmf_ip_family_ipv6)) ||
              ((rdpa_ic_field == RDPA_IC_DST_IP) && (rule_key->dst_ip_mask.family == bdmf_ip_family_ipv6)) )
        return rdpa_ic_field;     /* Masked IPv6 tcam fields are 0 or 1 */
    else
        return (rdpa_ic_field + RDD_IC_SRC_IP); /* If more RDD only fields will be added, this index will have to be changed */
}

/* Prepare TCAM key and mask areas */
static bdmf_error_t _rdd_tcam_prepare_tcam_areas(rdd_tcam_table_id table_id, rdp_tcam_key_area_t *tcam_key,
    rdp_tcam_key_area_t *tcam_mask)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];

    memset(tcam_key, 0, sizeof(rdp_tcam_key_area_t));
    memset(tcam_mask, 0, sizeof(rdp_tcam_key_area_t));

    /* key size in bytes */
    tcam_key->b[table->key_size-1]     = (uint8_t)table_id;
    tcam_mask->b[table->key_size-1]    = 0xff;

    return BDMF_ERR_OK;
}

/* Get IC field info given the IC table and field index */
static const rdd_tcam_field_info_t *_rdd_tcam_field_info(rdd_tcam_table_id table_id, rdd_ic_value tcam_fld)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    const rdd_tcam_field_info_t *fld_info;

    /* Generic fields must be configured before use */
    if ((tcam_fld >= RDD_IC_GENERIC_0) && (tcam_fld <= RDD_IC_GENERIC_3))
    {
        fld_info = &table->gen_fields[tcam_fld - RDD_IC_GENERIC_0];
    }
    else
    {
        fld_info = &rdd_tcam_fields[tcam_fld];
    }
    if (!fld_info->size)
    {
        BDMF_TRACE_ERR("Field %d is unconfigured\n", tcam_fld);
        return NULL;
    }
    return fld_info;
}

/* Allocate room for new IC field in TCAM key */
static bdmf_error_t _rdd_tcam_field_allocate(rdd_tcam_table_id table_id, rdd_ic_value tcam_fld)
{
    static uint8_t zero[8] = {};
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    const rdd_tcam_field_info_t *fld_info = _rdd_tcam_field_info(table_id, tcam_fld);
    int i;

    /* Got unconfigured field? RDPA should've caught it */
    if (!fld_info)
        return BDMF_ERR_INTERNAL;

    /* Find unused room in TCAM key.
     * 1 byte fields are added from the right
     * other sizes are added from the left
     */
    if (fld_info->size == 1)
    {
        for (i = table->key_size-2; i >= 0; i--)
        {
            if (!table->used_mask.b[i])
                break;
        }
    }
    else
    {
        int align = (fld_info->size == 2 || fld_info->size == 6) ? 2 : 4;
        for (i = 0; i < table->key_size-1; i += align)
        {
            if (!memcmp(&table->used_mask.b[i], zero, fld_info->size))
                break;
        }
    }

    /* Key area overflow ? */
    if (i < 0 || i >= table->key_size - 1)
    {
        BDMF_TRACE_ERR("Can't add classification flow. TCAM key record is full\n");
        return BDMF_ERR_OVERFLOW;
    }

    table->fields[tcam_fld].offset = i;

    /* mark room allocated for the field in TCAM area as reserved */
    for (; i < table->fields[tcam_fld].offset + fld_info->size; i++)
        table->used_mask.b[i] = 0xff;

    return BDMF_ERR_OK;
}

/* Decrement field reference count. Return number of fields that are no longer referenced */
static int _rdd_tcam_unreference_fields(rdd_tcam_table_id table_id, rdpa_ic_fields fields, const rdpa_ic_key_t *rule_key, const uint8_t gen_indexes[])
{
    static uint8_t zero[8] = {};
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    int num_deleted = 0;
    rdd_tcam_field_t *fld_data;
    rdpa_ic_value rdpa_fld;
    rdd_ic_value tcam_fld;

    /* Go over fields in key mask and un-reference them.
     * If field becomes unused - command table should be rebuilt
     */
    while ((rdpa_fld = ffs(fields)) != 0)
    {
        --rdpa_fld; /* 1-based --> 0-based */

        /* Map RDD_IC_ field index to TCAM_IC field index */
        tcam_fld = tcam_rdpa_ic_to_rdd_ic(rdpa_fld, gen_indexes, rule_key);

        fld_data = &table->fields[tcam_fld];

        BUG_ON(!fld_data->use_count);

        /* Reduce usecount. If field becomes unreferenced - update TCAM mask */
        --fld_data->use_count;
        if (!fld_data->use_count)
        {
            const rdd_tcam_field_info_t *fld_info = _rdd_tcam_field_info(table_id, tcam_fld);
            /* Clear field from TCAM mask */
            memcpy(&table->used_mask.b[fld_data->offset], zero, fld_info->size);
            fld_data->offset = 0;
            table->all_fields_mask &= ~(1 << tcam_fld);
            ++num_deleted;
        }
        fields &= ~(1 << rdpa_fld);
    }
    return num_deleted;
}

/* Copy key field to tcam record and update mask */
static void _rdd_tcam_copy_field_to_tcam(rdd_tcam_table_id table_id, rdpa_ic_value fld,
    rdd_ic_value tcam_fld, const rdpa_ic_key_t *rule_key,
    rdp_tcam_key_area_t *tcam_key, rdp_tcam_key_area_t *tcam_mask)
{
    static bdmf_mac_t mac_mask = { .b = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }  };
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    rdd_tcam_field_t *fld_data = &table->fields[tcam_fld];
    const rdd_tcam_field_info_t *fld_info = _rdd_tcam_field_info(table_id, tcam_fld);
    uint8_t *key = &tcam_key->b[fld_data->offset];
    uint8_t *mask = &tcam_mask->b[fld_data->offset];
    uint32_t ip_addr;

    /* Set mask for sizes 1, 2 and 4 */
    switch (fld_info->size)
    {
    case 1:
        *mask = fld_info->mask;
        break;
    case 2:
        *(uint16_t *)mask = cpu_to_be16(fld_info->mask);
        break;
    case 4:
        *(uint32_t *)mask = cpu_to_be32(fld_info->mask);
        break;
    default:
        break;
    }

    /* Set field value */
    switch (fld)
    {
    case RDPA_IC_L3_PROTOCOL:
        *key = rule_key->l3_protocol << PARSER_RESULT_LAYER3_PROTOCOL_F_OFFSET;
        break;

    case RDPA_IC_IP_PROTOCOL:
        *key = rule_key->protocol;
        break;

    case RDPA_IC_TOS:
        *key = rule_key->tos;
        if (rule_key->tos_mask != 0)
            *mask = rule_key->tos_mask;
        break;

    case RDPA_IC_DSCP:
        /* shifted for FW (loads 1 byte) */
        *key = (rule_key->dscp)<<2;
        break;

    case RDPA_IC_SRC_IP:
        if (tcam_fld == RDD_IC_SRC_IPV6_MASKED)
        {
            memcpy(key, &rule_key->src_ip.addr.ipv6, 8);
            memcpy(mask, &rule_key->src_ip_mask.addr.ipv6, 8);
        }
        else
        {
            if (rule_key->src_ip.family == bdmf_ip_family_ipv6)
                rdd_crc_ipv6_addr_calc(&rule_key->src_ip, &ip_addr);
            else
                ip_addr = rule_key->src_ip.addr.ipv4;
            *(uint32_t *)key = cpu_to_be32(ip_addr);
            if (!bdmf_ip_is_zero(&rule_key->src_ip_mask))
                *(uint32_t *)mask = cpu_to_be32(rule_key->src_ip_mask.addr.ipv4);
        }
        break;

    case RDPA_IC_DST_IP:
        if (tcam_fld == RDD_IC_DST_IPV6_MASKED)
        {
            memcpy(key, &rule_key->dst_ip.addr.ipv6, 8);
            memcpy(mask, &rule_key->dst_ip_mask.addr.ipv6, 8);
        }
        else
        {
            if (rule_key->dst_ip.family == bdmf_ip_family_ipv6)
                rdd_crc_ipv6_addr_calc(&rule_key->dst_ip, &ip_addr);
            else
                ip_addr = rule_key->dst_ip.addr.ipv4;
            *(uint32_t *)key = cpu_to_be32(ip_addr);
            if (!bdmf_ip_is_zero(&rule_key->dst_ip_mask))
                *(uint32_t *)mask = cpu_to_be32(rule_key->dst_ip_mask.addr.ipv4);
        }
        break;

    case RDPA_IC_SRC_PORT:
        *(uint16_t *)key = cpu_to_be16(rule_key->src_port);
        if (rule_key->src_port_mask != 0)
            *(uint16_t *)mask = cpu_to_be16(rule_key->src_port_mask);
        break;

    case RDPA_IC_DST_PORT:
        *(uint16_t *)key = cpu_to_be16(rule_key->dst_port);
        if (rule_key->dst_port_mask != 0)
            *(uint16_t *)mask = cpu_to_be16(rule_key->dst_port_mask);
        break;

    case RDPA_IC_INGRESS_PORT:
        *key = rule_key->ingress_port;
        break;

    case RDPA_IC_GEM_FLOW:
        *key = rule_key->gem_flow;
        break;

    case RDPA_IC_OUTER_VID:
        *(uint16_t *)key = cpu_to_be16(rule_key->outer_vid);
        break;

    case RDPA_IC_INNER_VID:
        *(uint16_t *)key = cpu_to_be16(rule_key->inner_vid);
        break;

    case RDPA_IC_OUTER_PBIT:
        /* shifted for FW (loads 1 byte) */
        *key = (rule_key->outer_pbits)<<5;
        if(rule_key->outer_pbits_mask != 0)
            *mask = (rule_key->outer_pbits_mask)<<5;
        break;

    case RDPA_IC_INNER_PBIT:
        /* shifted for FW (loads 1 byte) */
        *key = (rule_key->inner_pbits)<<5;
        if(rule_key->inner_pbits_mask != 0)
            *mask = (rule_key->inner_pbits_mask)<<5;
        break;

    case RDPA_IC_NUM_OF_VLANS:
        *key = rule_key->number_of_vlans;
        break;

    case RDPA_IC_OUTER_TPID:
        *(uint16_t *)key = cpu_to_be16(rule_key->outer_tpid);
        break;

    case RDPA_IC_INNER_TPID:
        *(uint16_t *)key = cpu_to_be16(rule_key->inner_tpid);
        break;

    case RDPA_IC_DST_MAC:
        *(bdmf_mac_t *)key = rule_key->dst_mac;
        if(bdmf_mac_is_zero(&rule_key->dst_mac_mask))
            *(bdmf_mac_t *)mask = mac_mask;
        else
            *(bdmf_mac_t *)mask = rule_key->dst_mac_mask;
        break;

    case RDPA_IC_SRC_MAC:
        *(bdmf_mac_t *)key = rule_key->src_mac;
        if(bdmf_mac_is_zero(&rule_key->src_mac_mask))
            *(bdmf_mac_t *)mask = mac_mask;
        else
            *(bdmf_mac_t *)mask = rule_key->src_mac_mask;
        break;

    case RDPA_IC_GENERIC_1:
        *(uint32_t *)key = cpu_to_be32(rule_key->generic_key_1);
        if (rule_key->generic_mask != 0)
        {
            *(uint32_t *)mask = cpu_to_be32(rule_key->generic_mask);
        }
        break;

    case RDPA_IC_GENERIC_2:
        *(uint32_t *)key = cpu_to_be32(rule_key->generic_key_2);
        if (rule_key->generic_mask_2 != 0)
        {
            *(uint32_t *)mask = cpu_to_be32(rule_key->generic_mask_2);
        }
        break;

    case RDPA_IC_IPV6_FLOW_LABEL:
        *(uint32_t *)key = cpu_to_be32(rule_key->ipv6_flow_label);
        break;

    case RDPA_IC_ETHER_TYPE:
        *(uint16_t *)key = cpu_to_be16(rule_key->etype);
        break;

    case RDPA_IC_SSID:
        *key = rule_key->ssid;
        break;

    case RDPA_IC_NETWORK_LAYER:
        *key = rule_key->network_layer;
        break;


    default:
        BDMF_TRACE_ERR("Unexpected field %d\n", fld);
        break;
    }
}

/* (Re)build F/W command table */
static bdmf_error_t _rdd_tcam_build_cmd_table(rdd_tcam_table_id table_id)
{
    /* This function builds S/W mirror of F/W command table.
     * Multiple IC fields are merged into a single command where possible
     * Commands are sorted in order of increasing offset in tcam key
     */
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    rdd_ic_fields key_mask = table->all_fields_mask;
    const rdd_tcam_field_info_t *fld_info;
    rdd_tcam_field_t *fld_data;
    rdd_tcam_fw_cmd_t *cmd = NULL;
    int num_cmds = 0;
    bdmf_error_t err = BDMF_ERR_OK;
    rdd_ic_value tcam_fld;

    /* Go over fields in all-key mask */
    while (key_mask)
    {
        /* Find field with min offset */
        uint32_t tmp_mask = key_mask;
        uint32_t min_offset = 64; /* big number */
        rdd_ic_value tmp_fld = 0;

        tcam_fld = 0;
        while ((tmp_fld = ffs(tmp_mask)) != 0)
        {
            --tmp_fld; /* 1-based --> 0-based */
            if (table->fields[tmp_fld].offset < min_offset)
            {
                min_offset = table->fields[tmp_fld].offset;
                tcam_fld = tmp_fld;
            }
            tmp_mask &= ~(1 << tmp_fld);
        }

        fld_data = &table->fields[tcam_fld];
        fld_info = _rdd_tcam_field_info(table_id, tcam_fld);

        /* Unable to merge. Create new command */
        if (num_cmds == table->max_cmds)
        {
            err = BDMF_ERR_TOO_MANY;
            break;
        }

        cmd = &table->cmds[num_cmds++];
        cmd->src_offset = fld_info->offset; /* Used for generic filters where src_offset is needed */
        cmd->tcam_offset = fld_data->offset;
        cmd->field_id = fld_info->field_id;

        /* to the next field */
        key_mask &= ~(1 << tcam_fld);
    }
    table->num_cmds = num_cmds;

    return err;
}

/* Copy command table to SRAM and activate it */
static void _rdd_tcam_activate_cmd_table(rdd_tcam_table_id table_id)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    _rdd_tcam_fw_cmd_table_update(table);
}

/******************************************************************************/
/*                                                                            */
/*                            External interface                              */
/*                                                                            */
/******************************************************************************/

/****************************************************************************************
 * module->init callback
 *****************************************************************************************/
int rdd_tcam_module_init(const rdd_module_t *module)
{
    const rdd_tcam_table_parm_t *table_parms = (const rdd_tcam_table_parm_t  *)module->params;
    rdd_tcam_table_id table_id = table_parms->module_id;
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    bdmf_error_t rc;

    BUG_ON((unsigned)table_id >= RDD_TCAM_MAX_TABLES);
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES)
    {
        return (int)BDMF_ERR_PARM;
    }

    /* If 1st call - init TCAM driver in 256 bit mode (default) */
    if (!fw_initialized)
        drv_tcam_mode_set(RDP_TCAM_KEY_256);

    rc = rdd_tcam_table_create(table_id, table_parms);
    BUG_ON(rc != BDMF_ERR_OK);

    table->module = module;
    table->module_id = table_parms->module_id;

    /* Init classification module */
    _rdd_tcam_fw_module_init(table);

    return rc;
}

/* Create IC table */
bdmf_error_t rdd_tcam_table_create(
    rdd_tcam_table_id             table_id,
    const rdd_tcam_table_parm_t  *table_parms)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    bdmf_error_t err;

    RDD_BTRACE("table_id = %d, table_parms = %p\n", table_id, table_parms);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES || !table_parms ||
        table_parms->max_cmds >= RDD_TCAM_MAX_CMD_SET_SIZE)
    {
        return BDMF_ERR_PARM;
    }
    else
    {
        RDD_TRACE("table_parms = { max_cmds = %d, invert_match = %d, module_id = %d, "
            "scratch_offset = %d }\n",
            table_parms->max_cmds, table_parms->invert_match, table_parms->module_id, table_parms->scratch_offset);
    }

    /* Check if table has already been created */
    if (table->max_cmds)
        return BDMF_ERR_ALREADY;

    err = drv_tcam_keysize_get(&table->key_size);
    if (err)
    {
        BDMF_TRACE_RET(err, "TCAM driver must be initialized first\n");
    }

    table->max_cmds = table_parms->max_cmds ? table_parms->max_cmds : RDD_TCAM_MAX_CMD_SET_SIZE - 1;
    table->cmd_table_size = table->max_cmds + 1;
    table->active_cmd_table = table_id;
    table->id = table_id;

    return BDMF_ERR_OK;
}


/* Destroy IC table */
bdmf_error_t rdd_tcam_table_destroy(
    rdd_tcam_table_id             table_id)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];

    RDD_BTRACE("table_id = %d\n", table_id);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES)
        return BDMF_ERR_PARM;

    /* Check if table has already been created */
    if (!table->max_cmds)
        return BDMF_ERR_NOENT;

    /* Make sure that the table is not in use */
    if (table->all_fields_mask)
        return BDMF_ERR_STATE;

    memset(table, 0, sizeof(*table));

    return BDMF_ERR_OK;
}


/* Set "invert_match" property */
bdmf_error_t rdd_tcam_table_invert_match_set(
    rdd_tcam_table_id             table_id,
    bdmf_boolean                  invert_match)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];

    RDD_BTRACE("table_id = %d, invert_match = %d\n", table_id, invert_match);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES)
        return BDMF_ERR_PARM;

    /* Check if table has already been created */
    if (!table->max_cmds)
        return BDMF_ERR_NOENT;

    /* Make sure that the table is not in use */
    if (table->all_fields_mask)
        return BDMF_ERR_STATE;

    /* Set "INVERT_MATCH" bit in classification_result */
    if (invert_match)
        table->classification_result |= TCAM_HIT_MISS_INVERT_INVERT;
    else
        table->classification_result &= ~TCAM_HIT_MISS_INVERT_INVERT;

    /* Update FW tables */
    _rdd_tcam_fw_classification_result_update(table);

    return BDMF_ERR_OK;
}


/* Get "invert_match" property */
bdmf_error_t rdd_tcam_table_invert_match_get(
    rdd_tcam_table_id             table_id,
    bdmf_boolean                 *invert_match)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];

    RDD_BTRACE("table_id = %d, invert_match = %p\n", table_id, invert_match);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES)
        return BDMF_ERR_PARM;

    /* Check if table has already been created */
    if (!table->max_cmds)
        return BDMF_ERR_NOENT;

    *invert_match = (table->classification_result & TCAM_HIT_MISS_INVERT_INVERT) != 0;

    return BDMF_ERR_OK;
}

/* Add ingress classification rule */
bdmf_error_t rdd_tcam_rule_add(
    rdd_tcam_table_id table_id,
    rdpa_ic_fields rule_key_mask,
    const uint8_t gen_indexes[],
    const rdpa_ic_key_t *rule_key,
    rdp_tcam_key_area_t *tcam_key,
    rdp_tcam_key_area_t *tcam_mask)
{
    bdmf_error_t err;
    rdpa_ic_fields key_mask;
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    int new_commands = 0;
    rdpa_ic_value rdpa_fld;
    rdd_ic_value tcam_fld;

    RDD_BTRACE("table_id = %d, rule_key_mask = %x, rule_key = %p, tcam_key = %p, tcam_mask = %p\n",
        table_id, rule_key_mask, rule_key, tcam_key, tcam_mask);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES    ||
        !rule_key_mask                               ||
        !gen_indexes                                 ||
        !rule_key                                    ||
        !tcam_key                                    ||
        !tcam_mask)
    {
        return BDMF_ERR_PARM;
    }
    /* IP mask address validation */
    if ( ((!bdmf_ip_is_zero(&rule_key->src_ip_mask)) && (rule_key->src_ip.family != rule_key->src_ip_mask.family)) ||
         ((!bdmf_ip_is_zero(&rule_key->dst_ip_mask)) && (rule_key->dst_ip.family != rule_key->dst_ip_mask.family)) )
    {
        BDMF_TRACE_ERR("IP mask is invalid, address and mask must be with the same type!\n");
        return BDMF_ERR_PARM;
    }

    /* Make sure that table exists */
    if (!table->max_cmds)
    {
        return BDMF_ERR_NOENT;
    }

    err = _rdd_tcam_prepare_tcam_areas(table_id, tcam_key, tcam_mask);
    if (err)
        return err;

    /* For any field only table_id should be in tcam key and mask */
    if(rule_key_mask == RDPA_IC_MASK_ANY)
        return BDMF_ERR_OK;

    /* Go over active fields. If the fields are already used - there is no need
     * to rebuild command table.
     */
    key_mask = rule_key_mask;
    while ((rdpa_fld = ffs(key_mask)) != 0)
    {
        --rdpa_fld; /* 1-based --> 0-based */

        /* Map RDD_IC_ field index to TCAM_IC field index */
        tcam_fld = tcam_rdpa_ic_to_rdd_ic(rdpa_fld, gen_indexes, rule_key);

        /* sanity value, check shift won't be done with value greater than 32 */
        if ((tcam_fld >= RDD_IC_LAST_KEY) || (tcam_fld >= 32))
        {
            BDMF_TRACE_ERR("rdpa_fld %u is insane. tcam_fld %u >= %u\n", rdpa_fld, tcam_fld, RDPA_IC_LAST_KEY);
            err = BDMF_ERR_PARM;
            break;
        }

        /* Allocate room in TCAM record if field is not already referenced */
        if (!table->fields[tcam_fld].use_count)
        {
            ++new_commands;
            err = _rdd_tcam_field_allocate(table_id, tcam_fld);
            if (err)
                break;

            table->all_fields_mask |= (1 << tcam_fld);
        }

        /* Copy field data into TCAM record */
        _rdd_tcam_copy_field_to_tcam(table_id, rdpa_fld, tcam_fld, rule_key, tcam_key, tcam_mask);

        ++table->fields[tcam_fld].use_count;

        key_mask &= ~(1 << rdpa_fld);
    }

    /* Re-build command table if necessary */
    if (!err && new_commands)
    {
        err = _rdd_tcam_build_cmd_table(table_id);
    }

    /* Roll back if error */
    if (err)
    {
        _rdd_tcam_unreference_fields(table_id, key_mask ^ rule_key_mask, rule_key, gen_indexes);
        return err;
    }

    /* Finally activate the new command table in the F/W - if command set was changed */
    if (new_commands)
    {
        _rdd_tcam_activate_cmd_table(table_id);
    }

    return BDMF_ERR_OK;
}


/* Get low-level TCAM key and mask for existing rule. */
bdmf_error_t rdd_tcam_rule_key_get(
    rdd_tcam_table_id        table_id,
    rdpa_ic_fields           rule_key_mask,
    const uint8_t            gen_indexes[],
    const rdpa_ic_key_t     *rule_key,
    rdp_tcam_key_area_t     *tcam_key,
    rdp_tcam_key_area_t     *tcam_mask)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    bdmf_error_t err;
    rdpa_ic_value rdpa_fld;
    rdd_ic_value tcam_fld;

    RDD_BTRACE("table_id = %d, rule_key_mask = %x, rule_key = %p, tcam_key = %p, tcam_mask = %p\n",
        table_id, rule_key_mask, rule_key, tcam_key, tcam_mask);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES    ||
        !rule_key_mask                               ||
        !gen_indexes                                 ||
        !rule_key                                    ||
        !tcam_key                                    ||
        !tcam_mask)
    {
        return BDMF_ERR_PARM;
    }

    err = _rdd_tcam_prepare_tcam_areas(table_id, tcam_key, tcam_mask);
    if (err)
        return err;

    /* For any field only table_id should be in tcam key and mask */
    if(rule_key_mask == RDPA_IC_MASK_ANY)
        return BDMF_ERR_OK;


    /* Go over fields in key mask and un-reference them.
     * If field becomes unused - command table should be rebuilt
     */
    while ((rdpa_fld = ffs(rule_key_mask)) != 0)
    {
        --rdpa_fld; /* 1-based --> 0-based */

        /* Map RDD_IC_ field index to TCAM_IC field index */
        tcam_fld = tcam_rdpa_ic_to_rdd_ic(rdpa_fld, gen_indexes, rule_key);

        BUG_ON(!table->fields[tcam_fld].use_count);

        /* Copy field to TCAM key */
        _rdd_tcam_copy_field_to_tcam(table_id, rdpa_fld, tcam_fld, rule_key, tcam_key, tcam_mask);

        rule_key_mask &= ~(1 << rdpa_fld);
    }

    return BDMF_ERR_OK;
}


/* Delete ingress classification rule */
bdmf_error_t rdd_tcam_rule_delete(
    rdd_tcam_table_id        table_id,
    rdpa_ic_fields           rule_key_mask,
    const rdpa_ic_key_t      *rule_key,
    const uint8_t            gen_indexes[])
{
    int delete_commands;

    RDD_BTRACE("table_id = %d, rule_key_mask = %x\n", table_id, rule_key_mask);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES ||
        !rule_key_mask                            ||
        !gen_indexes)
    {
        return BDMF_ERR_PARM;
    }

    /* If deleted rule is any - no need to change the commands */
    if (rule_key_mask == RDPA_IC_MASK_ANY)
        return BDMF_ERR_OK;

    /* Reduce field reference count. Fields that are no longer referenced are removed from TCAM mask */
    delete_commands = _rdd_tcam_unreference_fields(table_id, rule_key_mask, rule_key, gen_indexes);

    /* Rebuild and apply command table if needed */
    if (delete_commands)
    {
        /* Rebuild command table */
        _rdd_tcam_build_cmd_table(table_id);

        /* Activate command table */
        _rdd_tcam_activate_cmd_table(table_id);
    }

    return BDMF_ERR_OK;
}


/* Set generic key configuration */
bdmf_error_t rdd_tcam_generic_key_set(
    rdd_tcam_table_id                   table_id,
    uint8_t                             gen_index,
    const rdpa_ic_gen_rule_cfg_t       *cfg)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    rdd_tcam_field_info_t *fld_info = &table->gen_fields[gen_index];
    rdd_ic_value tcam_fld;

    RDD_BTRACE("table_id = %d, gen_index = %d, cfg = %p\n", table_id, gen_index, cfg);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES ||
        gen_index >= RDD_TCAM_MAX_GEN_FIELDS      ||
        !cfg)
    {
        return BDMF_ERR_PARM;
    }

    RDD_TRACE("cfg = { type = %d, offset = %d, mask = 0x%x}\n", cfg->type, cfg->offset, cfg->mask);

    /* Check if table has already been created */
    if (!table->max_cmds)
    {
        return BDMF_ERR_NOENT;
    }

    /* Can't change configuration of generic field in use */
    tcam_fld = RDD_IC_GENERIC_0 + gen_index;
    if (table->all_fields_mask & (1 << tcam_fld))
    {
        if (fld_info->field_id != rdd_tcam_layer_to_field_id[cfg->type] ||
            fld_info->offset != cfg->offset                                          ||
            fld_info->mask   != cfg->mask                                            ||
            fld_info->size   != sizeof(uint32_t))
        {
            return BDMF_ERR_STATE;
        }
    }

    fld_info->offset = cfg->offset;
    fld_info->mask = cfg->mask;
    fld_info->size = sizeof(uint32_t);  /* Generic fields currently are only 4 bytes */
    fld_info->field_id = rdd_tcam_layer_to_field_id[cfg->type];

    return BDMF_ERR_OK;
}


/* Get generic key configuration */
bdmf_error_t rdd_tcam_generic_key_get(
    rdd_tcam_table_id                   table_id,
    uint8_t                             gen_index,
    rdpa_ic_gen_rule_cfg_t             *cfg)
{
    rdd_tcam_table_t *table = &rdd_tcam_tables[table_id];
    rdd_tcam_field_info_t *fld_info = &table->gen_fields[gen_index];

    RDD_BTRACE("table_id = %d, gen_index = %d, cfg = %p\n", table_id, gen_index, cfg);

    /* Parameter validation */
    if ((unsigned)table_id >= RDD_TCAM_MAX_TABLES ||
        gen_index >= RDD_TCAM_MAX_GEN_FIELDS      ||
        !cfg)
    {
        return BDMF_ERR_PARM;
    }

    RDD_TRACE("cfg = { type = %d, offset = %d, mask = 0x%x}\n", cfg->type, cfg->offset, cfg->mask);

    if (!fld_info->size)
        return BDMF_ERR_NOENT;

    cfg->offset = fld_info->offset;
    cfg->mask = fld_info->mask;
    cfg->type = rdd_tcam_layer_to_field_id[cfg->type];

    return BDMF_ERR_OK;
}

void rdd_tcam_ic_result_entry_compose(uint16_t index, const rdd_ic_context_t *ctx, uint8_t *entry)
{
    RDD_BTRACE("index = %d, ctx = %p, entry = %p\n", index, ctx, entry);
    RDD_TRACE("ctx values to be used = { priority = %d, tx_flow = %d, egress_port = %d, include_mcast = %d policer_id = %x, loopback = %d }\n",
        ctx->priority, ctx->tx_flow, ctx->egress_port, ctx->include_mcast, (int)ctx->policer, ctx->loopback);
    rdd_ic_result_entry_compose(index, ctx, entry);
}

void rdd_ic_mcast_enable(bdmf_boolean enable)
{
    RDD_BTRACE("enable = %d\n", enable);

#if !defined(BCM63158)
    GROUP_MWRITE_8(RDD_IC_MCAST_ENABLE_ADDRESS_ARR, 0, enable);
#endif
}

void rdd_ic_module_enable(uint8_t module_id, uint8_t enable)
{
    int i;
    uint32_t ref_cnt_index = table_id_to_ref_cnt_index[module_id];

    for (i=TCAM_IC_MODULE_FIRST; i<TCAM_IC_MODULE_LAST; i++)
    {
        /* enable DS/US module */ 
        if (table_id_to_ref_cnt_index[i] == ref_cnt_index)
        {
            RDD_TCAM_IC_CFG_ENABLE_WRITE_G(enable, RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR, i);
        }
    }
}

