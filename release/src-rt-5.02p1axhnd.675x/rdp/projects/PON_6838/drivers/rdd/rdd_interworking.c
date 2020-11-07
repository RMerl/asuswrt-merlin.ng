/*
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
:>
*/

#include "rdd.h"


/******************************************************************************/
/*                                                                            */
/*                            Global Variables                                */
/*                                                                            */
/******************************************************************************/

extern uint8_t    *g_runner_ddr_base_addr;
extern uint32_t    g_ddr_headroom_size;
extern uint8_t     g_broadcom_switch_mode;
extern uint8_t     g_G9991_mode;
extern RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTE  g_ingress_classification_rule_cfg_table[ 2 ];
extern uint32_t                                       g_ih_lookup_mode_occupied[ 2 ];
extern BL_LILAC_RDD_WAN_PHYSICAL_PORT_DTE             g_wan_physical_port;
extern RDD_WAN_TX_POINTERS_TABLE_DTS                  *wan_tx_pointers_table_ptr;
extern RDD_64_BIT_TABLE_CFG                           g_hash_table_cfg[ BL_LILAC_RDD_MAX_HASH_TABLE ];
extern BL_LILAC_RDD_LOCK_CRITICAL_SECTION_FP_IRQ_DTE    f_rdd_lock_irq;
extern BL_LILAC_RDD_UNLOCK_CRITICAL_SECTION_FP_IRQ_DTE  f_rdd_unlock_irq;
extern bdmf_fastlock                                    int_lock_irq;

static BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_long_lookup_entry_add ( rdpa_traffic_dir, uint8_t *, uint32_t );
static BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_long_lookup_entry_delete ( rdpa_traffic_dir, uint8_t * );


uint32_t f_rdd_get_ingress_classification_rule_cfg_table_size ( rdpa_traffic_dir xi_direction )
{
    if ( xi_direction == rdpa_dir_ds )
    {
        return RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }
    else
    {
        return RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }
}

void rdd_ingress_classification_generic_rule_cfg(rdpa_traffic_dir dir,
    int gen_rule_cfg_idx, rdpa_ic_gen_rule_cfg_t *gen_rule_cfg)
{
    RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS *gen_rule_cfg_entry;

    if (dir == rdpa_dir_ds)
    {
        RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS *gen_rule_cfg_table;

        gen_rule_cfg_table = (RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS *)
            (DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS);
        gen_rule_cfg_entry = &(gen_rule_cfg_table->entry[gen_rule_cfg_idx]);
    }
    else
    {
        RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS *gen_rule_cfg_table;

        gen_rule_cfg_table = (RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS *)
            (DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS);
        gen_rule_cfg_entry = &(gen_rule_cfg_table->entry[gen_rule_cfg_idx]);

    }
    RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_TYPE_WRITE(gen_rule_cfg->type, gen_rule_cfg_entry);
    RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_OFFSET_WRITE(gen_rule_cfg->offset,
        gen_rule_cfg_entry);
    RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_MASK_WRITE(gen_rule_cfg->mask, gen_rule_cfg_entry);
}

void dump_g_ic_rule_tbl(rdpa_traffic_dir dir)
{
    int i;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("=============================================\n");
    for (i = 0; i < 16; i++)
    {
        if (!g_ingress_classification_rule_cfg_table[dir].rule_cfg[i].valid)
            continue;
        bdmf_trace("Idx=%d, {first_rule_cfg_id=%d, first_gen_filter_rule_cfg_id=%d, priority=%d, rule_type=%d, next_rule_cfg_id=%d, next_group_id=%d\n",
            i, g_ingress_classification_rule_cfg_table[dir].first_rule_cfg_id,
            g_ingress_classification_rule_cfg_table[dir].first_gen_filter_rule_cfg_id,
            g_ingress_classification_rule_cfg_table[dir].rule_cfg[i].priority,
            g_ingress_classification_rule_cfg_table[dir].rule_cfg[i].rule_type,
            g_ingress_classification_rule_cfg_table[dir].rule_cfg[i].next_rule_cfg_id,
            g_ingress_classification_rule_cfg_table[dir].rule_cfg[i].next_group_id);
    }
    bdmf_trace("=============================================\n");
}


BL_LILAC_RDD_ERROR_DTE _rdd_ingress_classification_non_ip_flow_rule_cfg_add( uint32_t new_rule_cfg_id, rdpa_traffic_dir xi_direction, int32_t xi_rule_cfg_priority,
    uint32_t ingress_classification_rule_cfg_table_size)
{
    uint32_t next_rule_cfg_id, previous_rule_cfg_id, rule_cfg_id;
    uint8_t *rule_cfg_descriptor_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS *rule_cfg_entry_ptr;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );
    }

    if ( g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id == ingress_classification_rule_cfg_table_size )
    {
        /* the list is empty, its the 1st item */
        g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_rule_cfg_id = ingress_classification_rule_cfg_table_size;
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( ingress_classification_rule_cfg_table_size, rule_cfg_entry_ptr );

        g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id = new_rule_cfg_id;
        MWRITE_8( rule_cfg_descriptor_ptr, new_rule_cfg_id );

        return BL_LILAC_RDD_OK;
    }
    next_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;
    previous_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

    while ( 1 )
    {
        if ( ( next_rule_cfg_id == ingress_classification_rule_cfg_table_size )
            || ( xi_rule_cfg_priority > g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].priority ) )
        {
            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_rule_cfg_id = next_rule_cfg_id;

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( next_rule_cfg_id, rule_cfg_entry_ptr );

            if ( next_rule_cfg_id == g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id )
            {
                /* the entry is with the highest priority in the list */

                /* group sorting */
                if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type == g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].rule_type )
                {
                    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_group_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].next_group_id;

                    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].next_group_id, rule_cfg_entry_ptr );
                }
                else if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type < g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].rule_type )
                {
                    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_group_id = next_rule_cfg_id;

                    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( next_rule_cfg_id, rule_cfg_entry_ptr );
                }
                else
                {
                    return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_ILEGGAL_GROUP_SORT );
                }

                g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id = new_rule_cfg_id;

                MWRITE_8( rule_cfg_descriptor_ptr, new_rule_cfg_id );
            }
            else
            {
                /* the entry is with priority somewhere in the middle/last in the list */
                if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type < g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                {
                    return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_ILEGGAL_GROUP_SORT );
                }

                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].next_rule_cfg_id = new_rule_cfg_id;

                if ( xi_direction == rdpa_dir_ds )
                {
                    rule_cfg_entry_ptr = &(ds_rule_cfg_table_ptr->entry[previous_rule_cfg_id]); 
                }
                else
                {
                    rule_cfg_entry_ptr = &(us_rule_cfg_table_ptr->entry[previous_rule_cfg_id]);                     
                }

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( new_rule_cfg_id, rule_cfg_entry_ptr );

                /* group sorting */
                if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type == g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                {
                    /* the new rule has the same rule type as the previous rule, the next group pointer is the same */
                    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_group_id =
                        g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].next_group_id;
                    if ( xi_direction == rdpa_dir_ds )
                    {
                        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                    }
                    else
                    {
                        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                    }

                    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].next_group_id, rule_cfg_entry_ptr );
                }
                else
                {
                    /* the new rule is the highest priority in its group, pass all on memebers of the previous group and set the group pointer to the new rule */
                    rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

                    while ( 1 )
                    {
                        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].rule_type ==
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                        {
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_group_id = new_rule_cfg_id;
                            if ( xi_direction == rdpa_dir_ds )
                            {
                                rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
                            }
                            else
                            {
                                rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
                            }
                            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( new_rule_cfg_id, rule_cfg_entry_ptr );
                        }

                        rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id;

                        if ( rule_cfg_id == ingress_classification_rule_cfg_table_size )
                        {
                            break;
                        }
                    }

                    if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].valid )
                    {
                        /* the new rule is not the last in the list, therefore update its next group pointer */
                        if ( xi_direction == rdpa_dir_ds )
                        {
                            rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                        }
                        else
                        {
                            rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                        }
                        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type ==
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].rule_type )
                        {
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_group_id =
                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].next_group_id;

                            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].next_group_id, rule_cfg_entry_ptr );
                        }
                        else
                        {
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].next_group_id = next_rule_cfg_id;

                            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( next_rule_cfg_id, rule_cfg_entry_ptr );
                        }
                    }
                }
            }

            break;
        }

        previous_rule_cfg_id = next_rule_cfg_id;
        next_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ next_rule_cfg_id ].next_rule_cfg_id;
    }

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE _rdd_ingress_classification_gen_filter_rule_cfg_add( uint32_t new_rule_cfg_id, rdpa_traffic_dir dir, int32_t prio,
    uint32_t ic_rule_cfg_table_size )
{
    uint32_t next_rule_cfg_id, previous_rule_cfg_id;
    uint8_t *rule_cfg_descriptor_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS *rule_cfg_entry_ptr;

    if ( dir == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );
    }

    if ( g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id == ic_rule_cfg_table_size )
    {
        /* the list is empty, its the 1st item */
        g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id = new_rule_cfg_id;
        g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ new_rule_cfg_id ].next_rule_cfg_id = ic_rule_cfg_table_size;
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( ic_rule_cfg_table_size, rule_cfg_entry_ptr );
        MWRITE_8( rule_cfg_descriptor_ptr, new_rule_cfg_id );
        return BL_LILAC_RDD_OK;
    }

    next_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id;
    previous_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id;

    while ( 1 )
    {
        if ( ( next_rule_cfg_id == ic_rule_cfg_table_size ) || ( prio > g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ next_rule_cfg_id ].priority ) )
        {
            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ new_rule_cfg_id ].next_rule_cfg_id = next_rule_cfg_id;

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( next_rule_cfg_id, rule_cfg_entry_ptr );

            if ( next_rule_cfg_id == g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id )
            {
                /* the entry is with the highest priority in the list */
                g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ new_rule_cfg_id ].next_group_id = ic_rule_cfg_table_size; /* No next group */
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( ic_rule_cfg_table_size, rule_cfg_entry_ptr );

                g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id = new_rule_cfg_id;
                MWRITE_8( rule_cfg_descriptor_ptr, new_rule_cfg_id );
            }
            else
            {
                /* the entry is with priority somewhere in the middle/last in the list */
                g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ previous_rule_cfg_id ].next_rule_cfg_id = new_rule_cfg_id;

                if ( dir == rdpa_dir_ds )
                    rule_cfg_entry_ptr = &(ds_rule_cfg_table_ptr->entry[previous_rule_cfg_id]); 
                else
                    rule_cfg_entry_ptr = &(us_rule_cfg_table_ptr->entry[previous_rule_cfg_id]);                     

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( new_rule_cfg_id, rule_cfg_entry_ptr );

                /* group sorting */
                if ( g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ new_rule_cfg_id ].rule_type == g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                {
                    /* the new rule has the same rule type as the previous rule, the next group pointer is the same */
                    g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ new_rule_cfg_id ].next_group_id =
                        g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ previous_rule_cfg_id ].next_group_id;
                    if ( dir == rdpa_dir_ds )
                    {
                        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                    }
                    else
                    {
                        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
                    }

                    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE (
                        g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ previous_rule_cfg_id ].next_group_id, rule_cfg_entry_ptr );
                }
            }

            break;
        }

        previous_rule_cfg_id = next_rule_cfg_id;
        next_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ next_rule_cfg_id ].next_rule_cfg_id;
    }

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_add ( rdpa_traffic_dir                        xi_direction,
                                                                 int32_t                                 xi_rule_cfg_priority,
                                                                 rdpa_ic_type                            xi_rule_cfg_type,
                                                                 rdpa_ic_fields                          xi_rule_cfg_key_mask,
                                                                 rdpa_forward_action                     xi_rule_hit_action,
                                                                 rdpa_forward_action                     xi_rule_miss_action,
                                                                 rdd_ingress_classification_lookup_mode *xo_rule_cfg_lookup_mode,
                                                                 int generic_rule_cfg_idx1,
                                                                 int generic_rule_cfg_idx2
                                                                 )

{
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS       *rule_cfg_entry_ptr;
    uint32_t                                            key_length;
    uint32_t                                            new_rule_cfg_id;
    rdd_ingress_classification_lookup_mode              rule_cfg_lookup_mode;
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *us_rule_cfg_table_ptr;
    unsigned long                                       flags;
    uint32_t                                            ingress_classification_rule_cfg_table_size;
    int i, rc;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    ingress_classification_rule_cfg_table_size = f_rdd_get_ingress_classification_rule_cfg_table_size(xi_direction);

    /* set the lookup mode (long/short/optimized/IH) according to the key fields length and type */
    key_length = 0;
    for (i = RDPA_IC_FIRST_4_BYTE_KEY; i <  RDPA_IC_LAST_KEY; i++)
    {
        if (!(xi_rule_cfg_key_mask & (1 << i)))
            continue;
        if (i < RDPA_IC_FIRST_2_BYTE_KEY)
            key_length += 4;
        else if (i < RDPA_IC_FIRST_1_BYTE_KEY)
            key_length += 2;
        else
            key_length += 1;
    }

    if ( key_length > 14 )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_KEY_TOO_LONG );
    }
    else if ( key_length > 6 )
    {
        rule_cfg_lookup_mode = rdd_ingress_classification_lookup_mode_long;
    }
    else if ( xi_rule_cfg_key_mask & ( RDPA_IC_MASK_SRC_IP | RDPA_IC_MASK_DST_IP | RDPA_IC_MASK_SRC_PORT | RDPA_IC_MASK_DST_PORT |
                                       RDPA_IC_MASK_DST_MAC | RDPA_IC_MASK_SRC_MAC | RDPA_IC_MASK_ETHER_TYPE |
                                       RDPA_IC_MASK_IP_PROTOCOL | RDPA_IC_MASK_DSCP | RDPA_IC_MASK_NUM_OF_VLANS |
                                       RDPA_IC_MASK_OUTER_TPID | RDPA_IC_MASK_INNER_TPID | RDPA_IC_MASK_L3_PROTOCOL |
                                       RDPA_IC_MASK_IPV6_FLOW_LABEL | RDPA_IC_MASK_GENERIC_1 | RDPA_IC_MASK_GENERIC_2) )
    {
        rule_cfg_lookup_mode = rdd_ingress_classification_lookup_mode_short;
    }
    else if ( ( xi_rule_cfg_key_mask & ( RDPA_IC_MASK_SSID | RDPA_IC_MASK_INNER_VID | RDPA_IC_MASK_INNER_PBIT ) )
              || g_ih_lookup_mode_occupied[ xi_direction ]
              || ( ( g_G9991_mode | g_broadcom_switch_mode ) && xi_direction == rdpa_dir_us &&  xi_rule_cfg_key_mask & RDPA_IC_MASK_INGRESS_PORT )
              || ( xi_rule_cfg_key_mask == RDPA_IC_MASK_INGRESS_PORT ) )
    {
        rule_cfg_lookup_mode = rdd_ingress_classification_lookup_mode_optimized;
    }
    else
    {
        rule_cfg_lookup_mode = rdd_ingress_classification_lookup_mode_ih;
        g_ih_lookup_mode_occupied[ xi_direction ] = 1;
    }

    *xo_rule_cfg_lookup_mode = rule_cfg_lookup_mode;

    /* find an empty entry in the table for the new rule */
    if ( rule_cfg_lookup_mode == rdd_ingress_classification_lookup_mode_ih )
    {
        /* IH rule should be located in entry 0 */
        new_rule_cfg_id = 0;
    }
    else
    {
        for ( new_rule_cfg_id = 1; new_rule_cfg_id < ingress_classification_rule_cfg_table_size; new_rule_cfg_id++ )
        {
            if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].valid == 0 )
            {
                break;
            }
        }
    }

    /* 16 rules are already configured */
    if ( new_rule_cfg_id == ingress_classification_rule_cfg_table_size )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_FULL );
    }

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ new_rule_cfg_id ] );
    }

    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE ( xi_rule_cfg_type, rule_cfg_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE ( xi_rule_cfg_key_mask, rule_cfg_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE ( rule_cfg_lookup_mode, rule_cfg_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_HIT_ACTION_WRITE ( ( xi_rule_hit_action & 1 ), rule_cfg_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_MISS_ACTION_WRITE ( ( xi_rule_miss_action & 1 ), rule_cfg_entry_ptr );

    if (xi_rule_cfg_key_mask & RDPA_IC_MASK_GENERIC_1)
    {
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_1_WRITE(generic_rule_cfg_idx1, rule_cfg_entry_ptr);
    }
    if (xi_rule_cfg_key_mask & RDPA_IC_MASK_GENERIC_2)
    {
        RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_2_WRITE(generic_rule_cfg_idx2, rule_cfg_entry_ptr);
    }

    /* sort the role in descending priority */
    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].valid = 1;
    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].priority = xi_rule_cfg_priority;
    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ new_rule_cfg_id ].rule_type = xi_rule_cfg_type;

    if ( xi_rule_cfg_type == RDPA_IC_TYPE_GENERIC_FILTER )
    {
        rc = _rdd_ingress_classification_gen_filter_rule_cfg_add(new_rule_cfg_id, xi_direction, xi_rule_cfg_priority, ingress_classification_rule_cfg_table_size);
    }
    else
    {
        rc = _rdd_ingress_classification_non_ip_flow_rule_cfg_add(new_rule_cfg_id, xi_direction, xi_rule_cfg_priority, ingress_classification_rule_cfg_table_size);
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );

    dump_g_ic_rule_tbl(xi_direction);

    return rc;
}


BL_LILAC_RDD_ERROR_DTE _rdd_ingress_classification_non_ip_flow_rule_cfg_delete ( rdpa_traffic_dir xi_direction, int32_t xi_rule_cfg_priority,
    uint32_t ingress_classification_rule_cfg_table_size )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS  *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS  *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS     *rule_cfg_entry_ptr;
    uint8_t                                           *rule_cfg_descriptor_ptr;
    uint32_t                                          deleted_rule_cfg_id;
    uint32_t                                          previous_rule_cfg_id;
    uint32_t                                          rule_cfg_id;
#ifdef UNDEF
    uint32_t                                          optimized_rule_cfg_id;
    uint32_t                                          rule_cfg_entry_lookup_mode;
    uint32_t                                          rule_cfg_entry_next_rule_cfg_id;
    uint32_t                                          rule_cfg_entry_next_group_id;
    uint32_t                                          rule_cfg_entry_rule_type;
#endif

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_DESCRIPTOR_ADDRESS );
    }

    deleted_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;
    previous_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

    while ( 1 )
    {
        if ( deleted_rule_cfg_id == ingress_classification_rule_cfg_table_size )
        {
            return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_NOT_EXIST );
        }

        /* search the entry in the table according to priority field */
        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].priority == xi_rule_cfg_priority )
        {
            if ( deleted_rule_cfg_id == g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id )
            {
                g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;

                MWRITE_8( rule_cfg_descriptor_ptr, g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id );
            }
            else
            {
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].next_rule_cfg_id =
                    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
                if ( xi_direction == rdpa_dir_ds )
                {
                    rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ previous_rule_cfg_id ] );
                }
                else
                {
                    rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ previous_rule_cfg_id ] );
                }

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id, rule_cfg_entry_ptr );

                /* group sorting */
                if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].rule_type != 
                     g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                {
                    /* the deleted rule has the highest priority in its group, pass all on memebers of the previous group and set the group pointer to the group pointer of the deleted rule */
                    rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

                    while ( 1 )
                    {
                        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].rule_type ==
                             g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                        {
                            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_group_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
                            if ( xi_direction == rdpa_dir_ds )
                            {
                                rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
                            }
                            else
                            {
                                rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
                            }

                            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id, rule_cfg_entry_ptr );
                        }

                        rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id;

                        if ( rule_cfg_id == ingress_classification_rule_cfg_table_size )
                        {
                            break;
                        }
                    }
                }
            }
            if ( xi_direction == rdpa_dir_ds )
            {
                rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ deleted_rule_cfg_id ] );
            }
            else
            {
                rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ deleted_rule_cfg_id ] );
            }

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( ingress_classification_rule_cfg_table_size, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( ingress_classification_rule_cfg_table_size, rule_cfg_entry_ptr );

            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].valid = 0;
            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].priority = -1;
            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].rule_type = 0;
            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id = ingress_classification_rule_cfg_table_size;
            g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_group_id = ingress_classification_rule_cfg_table_size;

            break;
        }

        previous_rule_cfg_id = deleted_rule_cfg_id;
        deleted_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
    }

    if ( deleted_rule_cfg_id == 0 )
    {
        g_ih_lookup_mode_occupied[ xi_direction ] = 0;
    }
	
#ifdef UNDEF
    /* check for a new candidate rule in the IH */
    if ( deleted_rule_cfg_id == 0 )
    {
        g_ih_lookup_mode_occupied[ xi_direction ] = 0;

        optimized_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;
        previous_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

        while ( 1 )
        {
            if ( optimized_rule_cfg_id == RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE )
            {
                break;
            }

            rule_cfg_entry_ptr = &( rule_cfg_table_ptr->entry[ optimized_rule_cfg_id ] );

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_READ ( rule_cfg_entry_lookup_mode, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_READ ( rule_cfg_entry_key_mask, rule_cfg_entry_ptr );

            if ( ( rule_cfg_entry_lookup_mode == rdpa_ingress_classification_lookup_mode_optimized ) && !( rule_cfg_entry_key_mask & rdpa_ingress_classification_rule_mask_ssid ) )
            {
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_READ ( rule_cfg_entry_next_group_id, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( rule_cfg_entry_next_rule_cfg_id, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_READ ( rule_cfg_entry_rule_type, rule_cfg_entry_ptr );

                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ 0 ].valid = 1;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ 0 ].priority = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].priority;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ 0 ].rule_type = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].rule_type;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ 0 ].next_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_rule_cfg_id;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ 0 ].next_group_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_group_id;

                rule_cfg_entry_ptr = &( rule_cfg_table_ptr->entry[ 0 ] );

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE ( rdpa_ingress_classification_lookup_mode_ih, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( rule_cfg_entry_next_rule_cfg_id, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( rule_cfg_entry_next_group_id, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE ( rule_cfg_entry_rule_type, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE ( rule_cfg_entry_key_mask, rule_cfg_entry_ptr );

                if ( optimized_rule_cfg_id != g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id )
                {
                    g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].next_rule_cfg_id = 0;

                    rule_cfg_entry_ptr = &( rule_cfg_table_ptr->entry[ previous_rule_cfg_id ] );

                    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( 0, rule_cfg_entry_ptr );

                    /* group sorting */
                    if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].rule_type !=
                         g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                    {
                        rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

                        while ( 1 )
                        {
                            if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].rule_type ==
                                 g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ previous_rule_cfg_id ].rule_type )
                            {
                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_group_id = 0;

                                rule_cfg_entry_ptr = &( rule_cfg_table_ptr->entry[ rule_cfg_id ] );

                                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_group_id, rule_cfg_entry_ptr );
                            }

                            rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id;

                            if ( rule_cfg_id == RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE )
                            {
                                break;
                            }
                        }
                    }
                }
                else
                {
                    g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id = 0;
                }

                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].valid = 0;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].priority = -1;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].rule_type = 0;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_rule_cfg_id = RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_group_id = RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;

                rule_cfg_entry_ptr = &( rule_cfg_table_ptr->entry[ optimized_rule_cfg_id ] );

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE ( 0, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE ( 0, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE ( 0, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE, rule_cfg_entry_ptr );
                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( RDD_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE, rule_cfg_entry_ptr );

                g_ih_lookup_mode_occupied[ xi_direction ] = 1;

                break;
            }

            previous_rule_cfg_id = optimized_rule_cfg_id;
            optimized_rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ optimized_rule_cfg_id ].next_rule_cfg_id;
        }
    }
#endif

    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE _rdd_ingress_classification_ip_flow_rule_cfg_delete ( rdpa_traffic_dir dir, int32_t prio, uint32_t ic_rule_cfg_table_size )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS *rule_cfg_entry_ptr;
    uint8_t *rule_cfg_descriptor_ptr;
    uint32_t deleted_rule_cfg_id, previous_rule_cfg_id;

    if ( dir == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        rule_cfg_descriptor_ptr = ( uint8_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_IP_FLOW_RULE_CFG_DESCRIPTOR_ADDRESS );
    }

    deleted_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id;
    previous_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id;

    while ( 1 )
    {
        if ( deleted_rule_cfg_id == ic_rule_cfg_table_size )
        {
            return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_NOT_EXIST );
        }

        /* search the entry in the table according to priority field */
        if ( g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].priority == prio )
        {
            if ( deleted_rule_cfg_id == g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id )
            {
                g_ingress_classification_rule_cfg_table[ dir ].first_gen_filter_rule_cfg_id =
                    g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
                MWRITE_8( rule_cfg_descriptor_ptr, g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id );
            }
            else
            {
                g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ previous_rule_cfg_id ].next_rule_cfg_id =
                    g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
                if ( dir == rdpa_dir_ds )
                    rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ previous_rule_cfg_id ] );
                else
                    rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ previous_rule_cfg_id ] );

                RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE (
                    g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id, rule_cfg_entry_ptr );
            }
            if ( dir == rdpa_dir_ds )
                rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ deleted_rule_cfg_id ] );
            else
                rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ deleted_rule_cfg_id ] );

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE ( 0, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE ( ic_rule_cfg_table_size, rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE ( ic_rule_cfg_table_size, rule_cfg_entry_ptr );

            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].valid = 0;
            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].priority = -1;
            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].rule_type = 0;
            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id = ic_rule_cfg_table_size;
            g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_group_id = ic_rule_cfg_table_size;

            break;
        }

        previous_rule_cfg_id = deleted_rule_cfg_id;
        deleted_rule_cfg_id = g_ingress_classification_rule_cfg_table[ dir ].rule_cfg[ deleted_rule_cfg_id ].next_rule_cfg_id;
    }

    if ( deleted_rule_cfg_id == 0 )
    {
        g_ih_lookup_mode_occupied[ dir ] = 0;
    }
	
    return BL_LILAC_RDD_OK;
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_delete ( rdpa_traffic_dir  xi_direction,
                                                                    int32_t           xi_rule_cfg_priority )
{
    unsigned long                                     flags;
    int rc;
    uint32_t                                          ingress_classification_rule_cfg_table_size;

    if ( xi_direction == rdpa_dir_ds )
        ingress_classification_rule_cfg_table_size = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    else
        ingress_classification_rule_cfg_table_size = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    if ( xi_rule_cfg_priority < RDPA_IP_FLOW_PRTY_OFFSET )
    {
        rc = _rdd_ingress_classification_non_ip_flow_rule_cfg_delete( xi_direction, xi_rule_cfg_priority, ingress_classification_rule_cfg_table_size );
    }
    else
    {
        rc = _rdd_ingress_classification_ip_flow_rule_cfg_delete( xi_direction, xi_rule_cfg_priority, ingress_classification_rule_cfg_table_size );
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );

    dump_g_ic_rule_tbl(xi_direction);

    return rc;
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_cfg_modify ( rdpa_traffic_dir     xi_direction,
                                                                    int32_t              xi_rule_cfg_priority,
                                                                    rdpa_forward_action  xi_rule_hit_action,
                                                                    rdpa_forward_action  xi_rule_miss_action )
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS       *rule_cfg_entry_ptr;
    uint32_t                                            rule_cfg_id;
    uint32_t                                            ingress_classification_rule_cfg_table_size;
    unsigned long                                       flags;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        ingress_classification_rule_cfg_table_size = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        ingress_classification_rule_cfg_table_size = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }

    rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id;

    while ( 1 )
    {
        if ( rule_cfg_id == ingress_classification_rule_cfg_table_size )
        {
            f_rdd_unlock_irq ( &int_lock_irq, flags );
            return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_NOT_EXIST );
        }

        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].priority == xi_rule_cfg_priority )
        {
             if ( xi_direction == rdpa_dir_ds )
             {
                 rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
             }
             else
             {
                 rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
             }

            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_HIT_ACTION_WRITE ( ( xi_rule_hit_action & 1 ), rule_cfg_entry_ptr );
            RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_MISS_ACTION_WRITE ( ( xi_rule_miss_action & 1 ), rule_cfg_entry_ptr );

            break;
        }

        rule_cfg_id = g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id;
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_table_print ( rdpa_traffic_dir  xi_direction )
{
    uint32_t  rule_cfg_id;
    uint32_t  ingress_classification_rule_cfg_table_size = f_rdd_get_ingress_classification_rule_cfg_table_size( xi_direction );
    
    for ( rule_cfg_id = 0; rule_cfg_id < ingress_classification_rule_cfg_table_size; rule_cfg_id++ )
    {
#if defined(FIRMWARE_INIT)
        printf("rule: %2d  valid: %d  next: %2d  next group: %2d   priority: %3d   type: %d\n", rule_cfg_id, g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].valid,
                                                                                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_rule_cfg_id,
                                                                                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].next_group_id,
                                                                                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].priority,
                                                                                                g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].rule_type );
#endif
    }

#if defined(FIRMWARE_INIT)
    printf("Head: %d\n\n", g_ingress_classification_rule_cfg_table[ xi_direction ].first_rule_cfg_id );
#endif

    return ( BL_LILAC_RDD_OK );
}

static BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_update ( rdpa_traffic_dir  xi_direction,
                                                             uint32_t          xi_rule_cfg_priority,
                                                             rdpa_ic_key_t     *xi_rule_key,
                                                             uint32_t          xi_context_id,
                                                             int is_add)
{
    RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *ds_rule_cfg_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS    *us_rule_cfg_table_ptr;
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS       *rule_cfg_entry_ptr;
    RDD_MAC_PARAMS                                      mac_params;
    BL_LILAC_RDD_ERROR_DTE                              rdd_error;
    uint32_t                                            rule_cfg_id;
    uint32_t                                            rule_cfg_entry_lookup_mode;
    uint32_t                                            rule_cfg_entry_key_mask;
#if !defined(FIRMWARE_INIT)
    uint32_t                                            *ipv6_buffer_ptr;
#endif
    uint32_t                                            ipv6_ip_crc;
    uint8_t                                             hash_entry[ 16 ];
    uint32_t                                            table_id;
    uint32_t                                            key_mask_high;
    uint32_t                                            key_mask_low;
    uint32_t                                            entry_index;
    uint32_t                                            i;
    unsigned long                                       flags;
    uint32_t                                            ingress_classification_rule_cfg_table_size;

    f_rdd_lock_irq ( &int_lock_irq, &flags );

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_rule_cfg_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        ingress_classification_rule_cfg_table_size = RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }
    else
    {
        us_rule_cfg_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS );
        ingress_classification_rule_cfg_table_size = RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE;
    }

    for ( rule_cfg_id = 0; rule_cfg_id < ingress_classification_rule_cfg_table_size; rule_cfg_id++ )
    {
        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].valid == 0 )
        {
            continue;
        }

        if ( g_ingress_classification_rule_cfg_table[ xi_direction ].rule_cfg[ rule_cfg_id ].priority == xi_rule_cfg_priority )
        {
            break;
        }
    }

    if ( rule_cfg_id == ingress_classification_rule_cfg_table_size )
    {
        f_rdd_unlock_irq ( &int_lock_irq, flags );
        return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_NOT_EXIST );
    }

    if ( xi_direction == rdpa_dir_ds )
    {
        rule_cfg_entry_ptr = &( ds_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
    }
    else
    {
        rule_cfg_entry_ptr = &( us_rule_cfg_table_ptr->entry[ rule_cfg_id ] );
    }
    
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_READ ( rule_cfg_entry_lookup_mode, rule_cfg_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_READ ( rule_cfg_entry_key_mask, rule_cfg_entry_ptr );

    if ( xi_direction == rdpa_dir_ds )
    {
        table_id = BL_LILAC_RDD_DS_INGRESS_CLASSIFICATION_SHORT_TABLE;
    }
    else
    {
        table_id = BL_LILAC_RDD_US_INGRESS_CLASSIFICATION_SHORT_TABLE;
    }

    if ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_ih )
    {
        /* IH rule */
        memset ( hash_entry, 0, 8 );

        if (is_add)
        {
            hash_entry[ 0 ] = ( ( xi_context_id & 0xF0 ) >> 4 );
            hash_entry[ 1 ] = ( ( xi_context_id & 0x0F ) << 4 );
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_VID )
        {
            hash_entry[ 5 ] |= ( ( xi_rule_key->outer_vid & 0xF00 ) >> 8 );
            hash_entry[ 6 ] = ( xi_rule_key->outer_vid & 0x0FF );
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_PBIT )
        {
            hash_entry[ 5 ] |= ( xi_rule_key->outer_pbits << 5 );
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INGRESS_PORT )
        {
            hash_entry[ 7 ] = xi_rule_key->ingress_port;
        }

        key_mask_high = INGRESS_CLASSIFICATION_IH_ENTRY_KEY_MASK_HIGH;
        key_mask_low = INGRESS_CLASSIFICATION_IH_ENTRY_KEY_MASK_LOW;
    }
    else if ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_optimized )
    {
        /* optimized rule */
        memset ( hash_entry, 0, 8 );

        if (is_add)
        {
            hash_entry[ 0 ] = ( ( xi_context_id & 0xF0 ) >> 4 );
            hash_entry[ 1 ] = ( ( xi_context_id & 0x0F ) << 4 ) | rule_cfg_id;
        }
        else
        {
            hash_entry[ 1 ] = rule_cfg_id;

        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_VID )
        {
            hash_entry[ 6 ] |= ( ( xi_rule_key->outer_vid & 0xF00 ) >> 8 );
            hash_entry[ 7 ] = xi_rule_key->outer_vid & 0x0FF;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INNER_VID )
        {
            hash_entry[ 4 ] |= ( ( xi_rule_key->inner_vid & 0xF00 ) >> 8 );
            hash_entry[ 5 ] = xi_rule_key->inner_vid & 0x0FF;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_PBIT )
        {
            hash_entry[ 6 ] |= ( xi_rule_key->outer_pbits << 5 );
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INNER_PBIT )
        {
            hash_entry[ 4 ] |= ( xi_rule_key->inner_pbits << 5 );
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INGRESS_PORT )
        {
            hash_entry[ 3 ] = xi_rule_key->ingress_port;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_SSID )
        {
            hash_entry[ 2 ] = xi_rule_key->ssid;
        }

        key_mask_high = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_HIGH;
        key_mask_low = INGRESS_CLASSIFICATION_OPTIMIZED_ENTRY_KEY_MASK_LOW;
    }
    else if ( ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_short ) || ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_long ) )
    {
        if ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_long )
            i = 16;
        else
            i = 8;

        memset ( hash_entry, 0, i );
        if (is_add)
        {
            if (rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_long)
                hash_entry[ 1 ] = rule_cfg_id;
            else
            {
                hash_entry[ 0 ] = ( ( xi_context_id & 0xF0 ) >> 4 );
                hash_entry[ 1 ] = ( ( xi_context_id & 0x0F ) << 4 ) | rule_cfg_id;
            }
        }
        else
        {
            hash_entry[ 1 ] = rule_cfg_id;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_SRC_IP )
        {
            if ( xi_rule_key->src_ip.family == bdmf_ip_family_ipv4 )
            {
                hash_entry[ i - 1 ] = ( xi_rule_key->src_ip.addr.ipv4 & 0x000000FF ) >> 0;
                hash_entry[ i - 2 ] = ( xi_rule_key->src_ip.addr.ipv4 & 0x0000FF00 ) >> 8;
                hash_entry[ i - 3 ] = ( xi_rule_key->src_ip.addr.ipv4 & 0x00FF0000 ) >> 16;
                hash_entry[ i - 4 ] = ( xi_rule_key->src_ip.addr.ipv4 & 0xFF000000 ) >> 24;
            }
            else if ( xi_rule_key->src_ip.family == bdmf_ip_family_ipv6 )
            {
#if !defined(FIRMWARE_INIT)

                ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

                MWRITE_BLK_8( ipv6_buffer_ptr, xi_rule_key->src_ip.addr.ipv6.data, 16 );

                rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

                if ( rdd_error != BL_LILAC_RDD_OK )
                {
                    f_rdd_unlock_irq ( &int_lock_irq, flags );
                    return ( rdd_error );
                }

                ipv6_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
                ipv6_ip_crc = xi_rule_key->src_ip.addr.ipv4;
#endif
                hash_entry[ i - 1 ] = ( ipv6_ip_crc & 0x000000FF ) >> 0;
                hash_entry[ i - 2 ] = ( ipv6_ip_crc & 0x0000FF00 ) >> 8;
                hash_entry[ i - 3 ] = ( ipv6_ip_crc & 0x00FF0000 ) >> 16;
                hash_entry[ i - 4 ] = ( ipv6_ip_crc & 0xFF000000 ) >> 24;
            }

            i -= 4;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_DST_IP )
        {
            if ( xi_rule_key->dst_ip.family == bdmf_ip_family_ipv4 )
            {
                hash_entry[ i - 1 ] = ( xi_rule_key->dst_ip.addr.ipv4 & 0x000000FF ) >> 0;
                hash_entry[ i - 2 ] = ( xi_rule_key->dst_ip.addr.ipv4 & 0x0000FF00 ) >> 8;
                hash_entry[ i - 3 ] = ( xi_rule_key->dst_ip.addr.ipv4 & 0x00FF0000 ) >> 16;
                hash_entry[ i - 4 ] = ( xi_rule_key->dst_ip.addr.ipv4 & 0xFF000000 ) >> 24;
            }
            else if ( xi_rule_key->dst_ip.family == bdmf_ip_family_ipv6 )
            {
#if !defined(FIRMWARE_INIT)

                ipv6_buffer_ptr = ( uint32_t * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BUFFER_ADDRESS );

                MWRITE_BLK_8( ipv6_buffer_ptr, xi_rule_key->dst_ip.addr.ipv6.data, 16 );

                rdd_error = f_rdd_cpu_tx_send_message ( LILAC_RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, BL_LILAC_RDD_WAIT );

                if ( rdd_error != BL_LILAC_RDD_OK )
                {
                    f_rdd_unlock_irq ( &int_lock_irq, flags );
                    return ( rdd_error );
                }

                ipv6_ip_crc = *( volatile uint32_t * )ipv6_buffer_ptr;
#else
                ipv6_ip_crc = xi_rule_key->dst_ip.addr.ipv4;
#endif
                hash_entry[ i - 1 ] = ( ipv6_ip_crc & 0x000000FF ) >> 0;
                hash_entry[ i - 2 ] = ( ipv6_ip_crc & 0x0000FF00 ) >> 8;
                hash_entry[ i - 3 ] = ( ipv6_ip_crc & 0x00FF0000 ) >> 16;
                hash_entry[ i - 4 ] = ( ipv6_ip_crc & 0xFF000000 ) >> 24;
            }

            i -= 4;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_IPV6_FLOW_LABEL )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->ipv6_flow_label & 0x000000FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->ipv6_flow_label & 0x0000FF00 ) >> 8;
            hash_entry[ i - 3 ] = ( xi_rule_key->ipv6_flow_label & 0x000F0000 ) >> 16;
            hash_entry[ i - 4 ] = 0;
            i -= 4;
        }

        if (rule_cfg_entry_key_mask & RDPA_IC_MASK_GENERIC_1)
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->generic_key_1 & 0x000000FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->generic_key_1 & 0x0000FF00 ) >> 8;
            hash_entry[ i - 3 ] = ( xi_rule_key->generic_key_1 & 0x00FF0000 ) >> 16;
            hash_entry[ i - 4 ] = ( xi_rule_key->generic_key_1 & 0xFF000000 ) >> 24;
            i -= 4;
        }

        if (rule_cfg_entry_key_mask & RDPA_IC_MASK_GENERIC_2)
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->generic_key_2 & 0x000000FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->generic_key_2 & 0x0000FF00 ) >> 8;
            hash_entry[ i - 3 ] = ( xi_rule_key->generic_key_2 & 0x00FF0000 ) >> 16;
            hash_entry[ i - 4 ] = ( xi_rule_key->generic_key_2 & 0xFF000000 ) >> 24;
            i -= 4;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_TPID )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->outer_tpid & 0x000000FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->outer_tpid & 0x0000FF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INNER_TPID )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->inner_tpid & 0x000000FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->inner_tpid & 0x0000FF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_SRC_PORT )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->src_port & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->src_port & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_DST_PORT )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->dst_port & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->dst_port & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_VID )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->outer_vid & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->outer_vid & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INNER_VID )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->inner_vid & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->inner_vid & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_DST_MAC )
        {
            memset ( &mac_params, 0, sizeof ( RDD_MAC_PARAMS ) );
            memcpy ( mac_params.mac_addr.b, &( xi_rule_key->dst_mac.b ), 6 );

            rdd_error = rdd_mac_entry_search ( &mac_params, &entry_index );

            if ( rdd_error )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( rdd_error );
            }

            hash_entry[ i - 1 ] = ( entry_index & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( entry_index & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_SRC_MAC )
        {
            memset ( &mac_params, 0, sizeof ( RDD_MAC_PARAMS ) );
            memcpy ( mac_params.mac_addr.b, &( xi_rule_key->src_mac.b ), 6 );

            rdd_error = rdd_mac_entry_search ( &mac_params, &entry_index );

            if ( rdd_error )
            {
                f_rdd_unlock_irq ( &int_lock_irq, flags );
                return ( rdd_error );
            }

            hash_entry[ i - 1 ] = ( entry_index & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( entry_index & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_ETHER_TYPE )
        {
            hash_entry[ i - 1 ] = ( xi_rule_key->etype & 0x00FF ) >> 0;
            hash_entry[ i - 2 ] = ( xi_rule_key->etype & 0xFF00 ) >> 8;
            i -= 2;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_IP_PROTOCOL )
        {
            hash_entry[ i - 1 ] = xi_rule_key->protocol;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_DSCP )
        {
            hash_entry[ i - 1 ] = xi_rule_key->dscp;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_SSID )
        {
            hash_entry[ i - 1 ] = xi_rule_key->ssid;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INGRESS_PORT )
        {
            hash_entry[ i - 1 ] = xi_rule_key->ingress_port;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_OUTER_PBIT )
        {
            hash_entry[ i - 1 ] = xi_rule_key->outer_pbits;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_INNER_PBIT )
        {
            hash_entry[ i - 1 ] = xi_rule_key->inner_pbits;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_NUM_OF_VLANS )
        {
            hash_entry[ i - 1 ] = xi_rule_key->number_of_vlans;
            i -= 1;
        }

        if ( rule_cfg_entry_key_mask & RDPA_IC_MASK_L3_PROTOCOL )
        {
            hash_entry[ i - 1 ] = xi_rule_key->l3_protocol;
            i -= 1;
        }

        key_mask_high = INGRESS_CLASSIFICATION_SHORT_ENTRY_KEY_MASK_HIGH;
        key_mask_low = INGRESS_CLASSIFICATION_SHORT_ENTRY_KEY_MASK_LOW;
    }

    if ( ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_ih ) ||
         ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_optimized ) ||
         ( rule_cfg_entry_lookup_mode == rdd_ingress_classification_lookup_mode_short ) )
    {
        if (is_add)
        {
            rdd_error = rdd_add_hash_entry_64_bit ( &g_hash_table_cfg[ table_id ],
                hash_entry,
                NULL,
                key_mask_high,
                key_mask_low,
                0,
                &entry_index );
        }
        else
        {
            rdd_error = rdd_remove_hash_entry_64_bit ( &g_hash_table_cfg[ table_id ],
                hash_entry,
                key_mask_high,
                key_mask_low,
                0,
                BL_LILAC_RDD_CAM_OPTIMIZATION_DISABLE,
                &entry_index );
        }
    }
    else
    {
        if (is_add)
            rdd_error = rdd_ingress_classification_long_lookup_entry_add ( xi_direction, hash_entry, xi_context_id );
        else
            rdd_error = rdd_ingress_classification_long_lookup_entry_delete ( xi_direction, hash_entry );
    }

    f_rdd_unlock_irq ( &int_lock_irq, flags );
    return ( rdd_error );
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_add ( rdpa_traffic_dir  xi_direction,
                                                             uint32_t          xi_rule_cfg_priority,
                                                             rdpa_ic_key_t     *xi_rule_key,
                                                             uint32_t          xi_context_id)
{
    return rdd_ingress_classification_rule_update(xi_direction, xi_rule_cfg_priority, xi_rule_key, xi_context_id, 1); 
}

BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_rule_delete ( rdpa_traffic_dir  xi_direction,
                                                                uint32_t          xi_rule_cfg_priority,
                                                                rdpa_ic_key_t     *xi_rule_key )
{
    return rdd_ingress_classification_rule_update(xi_direction, xi_rule_cfg_priority, xi_rule_key, 0, 0);
}

BL_LILAC_RDD_ERROR_DTE rdd_us_ingress_classification_default_flows_config ( BL_LILAC_RDD_EMAC_ID_DTE      xi_emac_id,
                                                                            uint32_t                      xi_context_id )
{
    RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS  *default_flows_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_DTS  *default_flows_entry_ptr;

    if ( xi_emac_id >= BL_LILAC_RDD_EMAC_ID_COUNT )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_EMAC_ID );
    }

    default_flows_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_ADDRESS );
    default_flows_entry_ptr = &( default_flows_table_ptr->entry[ xi_emac_id ]);

    RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_FLOW_ID_WRITE ( xi_context_id, default_flows_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_config ( rdpa_traffic_dir                             xi_direction,
                                                                   uint32_t                                     xi_context_id,
                                                                   const rdd_ingress_classification_context_t  *xi_context )
{
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *ds_ingress_classification_context_table_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *ds_ingress_classification_context_entry_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *us_ingress_classification_context_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *us_ingress_classification_context_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_TABLE_DTS                 *vlan_cmd_idx_table_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS                 *vlan_cmd_idx_entry_ptr;

    if ( xi_direction == rdpa_dir_ds )
    {
        if ( xi_context_id >= RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE )
        {
            return ( -1 );
        }

        ds_ingress_classification_context_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        ds_ingress_classification_context_entry_ptr = &( ds_ingress_classification_context_table_ptr->entry[ xi_context_id ] );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_WRITE ( ( ( VLAN_COMMAND_INDEX_TABLE_ADDRESS + ( xi_context_id * sizeof ( RDD_VLAN_COMMAND_INDEX_ENTRY_DTS ) ) ) >> 3 ),
                                                                                 ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE ( xi_context->opbit_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE ( xi_context->ipbit_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_WRITE ( xi_context->wifi_ssid, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_WRITE ( xi_context->subnet_id, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_WRITE ( xi_context->forw_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_WRITE ( xi_context->egress_port, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE ( xi_context->qos_method, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE ( xi_context->priority, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE ( ( xi_context->action == rdpa_forward_action_drop ) ? 1 : 0, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE ( ( xi_context->action == rdpa_forward_action_host ) ? 1 : 0, ds_ingress_classification_context_entry_ptr );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_WRITE ( xi_context->ic_ip_flow, ds_ingress_classification_context_entry_ptr );

        if ( xi_context->policer < 0 )
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE ( 0, ds_ingress_classification_context_entry_ptr );
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE ( 0, ds_ingress_classification_context_entry_ptr );
        }
        else
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE ( 1, ds_ingress_classification_context_entry_ptr );
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE ( xi_context->policer, ds_ingress_classification_context_entry_ptr );
        }

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_WRITE ( xi_context->service_queue_mode, ds_ingress_classification_context_entry_ptr );

        if ( xi_context->service_queue_mode )
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE ( xi_context->service_queue, ds_ingress_classification_context_entry_ptr );
        }

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE ( xi_context->opbit_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE ( xi_context->ipbit_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE ( xi_context->dscp_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE ( xi_context->dscp_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE ( xi_context->ecn_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE( ( xi_context->dei_command == rdd_dei_command_transparent ) ? 0 : 1 , ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE( ( xi_context->dei_command == rdd_dei_command_clear ) ? 0 : 1 , ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_WRITE ( xi_context->cpu_mirroring, ds_ingress_classification_context_entry_ptr );

        vlan_cmd_idx_table_ptr = RDD_VLAN_COMMAND_INDEX_TABLE_PTR();

        vlan_cmd_idx_entry_ptr = &( vlan_cmd_idx_table_ptr->entry[ xi_context_id ] );

        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth0_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth1_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth2_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth3_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth4_vlan_command, vlan_cmd_idx_entry_ptr );
#ifndef G9991
        RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.pci_vlan_command, vlan_cmd_idx_entry_ptr );
#else
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth6_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth7_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth8_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth9_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth10_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth11_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth12_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth13_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth14_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth15_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth16_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth17_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth18_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth19_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth20_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth21_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth22_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_WRITE ( xi_context->vlan_command_id.ds_vlan_command.eth23_vlan_command, vlan_cmd_idx_entry_ptr );
#endif
    }
    else
    {
        if ( xi_context_id >= RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE )
        {
            return ( -1 );
        }

        us_ingress_classification_context_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        us_ingress_classification_context_entry_ptr = &( us_ingress_classification_context_table_ptr->entry[ xi_context_id ] );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_CMD_INDEX_WRITE ( xi_context->vlan_command_id.us_vlan_command, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE ( ( xi_context->action == rdpa_forward_action_drop ) ? 1 : 0, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE ( ( xi_context->action == rdpa_forward_action_host ) ? 1 : 0, us_ingress_classification_context_entry_ptr );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_WRITE ( xi_context->ic_ip_flow, us_ingress_classification_context_entry_ptr );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE ( xi_context->opbit_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE ( xi_context->ipbit_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_WRITE ( xi_context->wan_flow, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_CONTROLLER_WRITE ( xi_context->rate_controller_id, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE ( xi_context->qos_method, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE ( xi_context->priority, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE ( xi_context->opbit_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE ( xi_context->ipbit_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE ( xi_context->dscp_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE ( xi_context->dscp_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE ( xi_context->ecn_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_MODE_WRITE ( xi_context->wan_flow_mapping_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_TABLE_WRITE ( xi_context->wan_flow_mapping_table, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_OVERRUN_WAN_FLOW_MODE_WRITE ( xi_context->qos_rule_wan_flow_overrun, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE( ( xi_context->dei_command == rdd_dei_command_transparent ) ? 0 : 1 , us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE( ( xi_context->dei_command == rdd_dei_command_clear ) ? 0 : 1 , us_ingress_classification_context_entry_ptr );

        if ( xi_context->policer < 0 )
        {
            RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE ( 0, us_ingress_classification_context_entry_ptr );
            RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE ( 0, us_ingress_classification_context_entry_ptr );
        }
        else
        {
            RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE ( 1, us_ingress_classification_context_entry_ptr );
            RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE ( xi_context->policer, us_ingress_classification_context_entry_ptr );
        }
    }

    return ( BL_LILAC_RDD_OK );
}



BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_get ( rdpa_traffic_dir                      xi_direction,
                                                                uint32_t                              xi_context_id,
                                                                rdd_ingress_classification_context_t  *xo_context )
{
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *ds_ingress_classification_context_table_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *ds_ingress_classification_context_entry_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *us_ingress_classification_context_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *us_ingress_classification_context_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_TABLE_DTS                 *vlan_cmd_idx_table_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS                 *vlan_cmd_idx_entry_ptr;
    uint32_t                                         drop_flow;
    uint32_t                                         cpu_flow;
    uint32_t                                         policer_enable;
    uint32_t                                         dei_remark_enable;
    uint32_t                                         dei_value;

    if ( xi_direction == rdpa_dir_ds )
    {
        if ( xi_context_id >= RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE )
        {
            return ( -1 );
        }

        ds_ingress_classification_context_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        ds_ingress_classification_context_entry_ptr = &( ds_ingress_classification_context_table_ptr->entry[ xi_context_id ] );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ ( xo_context->priority, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_READ ( xo_context->forw_mode, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_READ ( xo_context->egress_port, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ ( xo_context->qos_method, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_READ ( xo_context->subnet_id, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_READ ( xo_context->wifi_ssid, ds_ingress_classification_context_entry_ptr );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ ( drop_flow, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ ( cpu_flow, ds_ingress_classification_context_entry_ptr );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ ( policer_enable, ds_ingress_classification_context_entry_ptr );

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_READ ( xo_context->ic_ip_flow, ds_ingress_classification_context_entry_ptr );

        if ( policer_enable )
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ ( xo_context->policer, ds_ingress_classification_context_entry_ptr );
        }
        else
        {
            xo_context->policer = -1;
        }

        xo_context->rate_shaper = -1;

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_READ ( xo_context->service_queue_mode, ds_ingress_classification_context_entry_ptr );

        if ( xo_context->service_queue_mode )
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_READ ( xo_context->service_queue, ds_ingress_classification_context_entry_ptr );
        }

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ ( xo_context->opbit_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ ( xo_context->ipbit_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ ( xo_context->opbit_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ ( xo_context->ipbit_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ ( xo_context->dscp_remark, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ ( xo_context->dscp_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ ( xo_context->ecn_val, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ( dei_remark_enable, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ( dei_value, ds_ingress_classification_context_entry_ptr );
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_READ( xo_context->cpu_mirroring, ds_ingress_classification_context_entry_ptr );

        vlan_cmd_idx_table_ptr = RDD_VLAN_COMMAND_INDEX_TABLE_PTR();

        vlan_cmd_idx_entry_ptr = &( vlan_cmd_idx_table_ptr->entry[ xi_context_id ] );

        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth0_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth1_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth2_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth3_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth4_vlan_command, vlan_cmd_idx_entry_ptr );
#ifndef G9991
        RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.pci_vlan_command, vlan_cmd_idx_entry_ptr );
#else
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth6_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth7_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth8_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth9_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth10_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth11_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth12_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth13_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth14_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth15_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth16_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth17_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth18_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth19_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth20_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth21_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth22_vlan_command, vlan_cmd_idx_entry_ptr );
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_READ ( xo_context->vlan_command_id.ds_vlan_command.eth23_vlan_command, vlan_cmd_idx_entry_ptr );
#endif
    }
    else
    {
        if ( xi_context_id >= RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE )
        {
            return ( -1 );
        }

        us_ingress_classification_context_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS );
        us_ingress_classification_context_entry_ptr = &( us_ingress_classification_context_table_ptr->entry[ xi_context_id ] );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ ( xo_context->priority, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_CONTROLLER_READ ( xo_context->rate_controller_id, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_CMD_INDEX_READ ( xo_context->vlan_command_id.us_vlan_command, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_READ ( xo_context->wan_flow, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ ( xo_context->qos_method, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ ( drop_flow, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ ( cpu_flow, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ ( policer_enable, us_ingress_classification_context_entry_ptr );

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_READ ( xo_context->ic_ip_flow, us_ingress_classification_context_entry_ptr );

        if ( policer_enable )
        {
            RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ ( xo_context->policer, us_ingress_classification_context_entry_ptr );
        }
        else
        {
            xo_context->policer = -1;
        }

        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ ( xo_context->opbit_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ ( xo_context->ipbit_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ ( xo_context->opbit_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ ( xo_context->ipbit_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ ( xo_context->dscp_remark, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ ( xo_context->dscp_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ ( xo_context->ecn_val, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_MODE_READ ( xo_context->wan_flow_mapping_mode, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_TABLE_READ ( xo_context->wan_flow_mapping_table, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_OVERRUN_WAN_FLOW_MODE_READ ( xo_context->qos_rule_wan_flow_overrun, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ( dei_remark_enable, us_ingress_classification_context_entry_ptr );
        RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ( dei_value, us_ingress_classification_context_entry_ptr );
    }

    if ( dei_remark_enable )
    {   
        if ( dei_value )
        {
            xo_context->dei_command = rdd_dei_command_set;
        }
        else
        {
            xo_context->dei_command = rdd_dei_command_clear;
        }
    }
    else
    {
        xo_context->dei_command = rdd_dei_command_transparent;
    }

    if ( drop_flow )
    {
        xo_context->action = rdpa_forward_action_drop;
    }
    else if ( cpu_flow )
    {
        xo_context->action = rdpa_forward_action_host;
    }
    else
    {
        xo_context->action = rdpa_forward_action_forward;
    }

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_long_lookup_entry_add ( rdpa_traffic_dir  xi_direction,
                                                                                 uint8_t           *xi_hash_entry,
                                                                                 uint32_t          xi_context_id )
{
    RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS  *ds_ingress_classification_lookup_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS  *us_ingress_classification_lookup_table_ptr;
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS     *ingress_classification_lookup_entry_ptr;
    uint32_t                                             crc_init_value, crc_result, hash_index, tries;
    uint32_t                                             entry_index;
    uint32_t                                             entry_valid;
    uint32_t                                             entry_skipped;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_ingress_classification_lookup_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS );
    }
    else
    {
        us_ingress_classification_lookup_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                               US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );
    }

    crc_init_value =  rdd_crc_init_value_get( RDD_CRC_TYPE_16 );

    crc_init_value = rdd_crc_bit_by_bit ( & ( xi_hash_entry[ 1 ] ), 1, 4, crc_init_value, RDD_CRC_TYPE_16 );

    crc_result = rdd_crc_bit_by_bit ( & ( xi_hash_entry[ 3 ] ), 6, 0, crc_init_value, RDD_CRC_TYPE_16 );

    hash_index = ( ( ( crc_result * 2 ) & ( ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE * 2  ) - 1 ) ) / 2 );

    for ( tries = 0; tries < RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH; tries++ )
    {
        entry_index = ( hash_index + tries ) & ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE - 1 );

        if ( xi_direction == rdpa_dir_ds )
        {
            ingress_classification_lookup_entry_ptr = &( ds_ingress_classification_lookup_table_ptr->entry[ entry_index ] );
        }
        else
        {
            ingress_classification_lookup_entry_ptr = &( us_ingress_classification_lookup_table_ptr->entry[ entry_index ] );
        }

        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_VALID_READ ( entry_valid, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_READ ( entry_skipped, ingress_classification_lookup_entry_ptr );

        if ( !( entry_valid ) || ( entry_skipped ) )
        {
            break;
        }
    }

    if ( tries == RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH )
    {
        return ( BL_LILAC_RDD_ERROR_INGRESS_CLASSIFICATION_CFG_LONG_TABLE_FULL );
    }

    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_INDEX_WRITE ( xi_hash_entry[ 1 ], ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_0_WRITE ( ( xi_hash_entry[ 2 ] << 16 ) | ( xi_hash_entry[ 3 ] << 8 ) | ( xi_hash_entry[ 4 ] << 0 ), ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_1_WRITE ( ( ( xi_hash_entry[ 5 ] << 24 ) | ( xi_hash_entry[ 6 ] << 16 ) | ( xi_hash_entry[ 7 ] << 8 ) | ( xi_hash_entry[ 8 ] << 0 ) ), ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_CONTEXT_WRITE ( xi_context_id, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_2_WRITE ( ( xi_hash_entry[ 9 ] << 16 ) | ( xi_hash_entry[ 10 ] << 8 ) | ( xi_hash_entry[ 11 ] << 0 ), ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_3_WRITE ( ( xi_hash_entry[ 12 ] << 24 ) | ( xi_hash_entry[ 13 ] << 16 ) | ( xi_hash_entry[ 14 ] << 8 ) | ( xi_hash_entry[ 15 ] << 0 ), ingress_classification_lookup_entry_ptr );

    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_WRITE ( LILAC_RDD_OFF, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_VALID_WRITE ( LILAC_RDD_ON, ingress_classification_lookup_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


static BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_long_lookup_entry_delete ( rdpa_traffic_dir  xi_direction,
                                                                                    uint8_t           *xi_hash_entry )
{
    RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS  *ds_ingress_classification_lookup_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS  *us_ingress_classification_lookup_table_ptr;
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS     *ingress_classification_lookup_entry_ptr;
    uint32_t                                             ingress_classification_entry_key_0;
    uint32_t                                             ingress_classification_entry_key_1;
    uint32_t                                             ingress_classification_entry_key_2;
    uint32_t                                             ingress_classification_entry_key_3;
    uint8_t                                              ingress_classification_entry_key_index;
    uint32_t                                             crc_init_value, crc_result, hash_index, tries;
    uint32_t                                             entry_index;
    uint32_t                                             entry_valid;
    uint32_t                                             entry_skipped;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_ingress_classification_lookup_table_ptr = ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS );
    }
    else
    {
        us_ingress_classification_lookup_table_ptr = ( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) +
                                                                                                               US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );
    }

    crc_init_value =  rdd_crc_init_value_get( RDD_CRC_TYPE_16 );

    crc_init_value = rdd_crc_bit_by_bit ( & ( xi_hash_entry[ 1 ] ), 1, 4, crc_init_value, RDD_CRC_TYPE_16 );

    crc_result = rdd_crc_bit_by_bit ( & ( xi_hash_entry[ 3 ] ), 6, 0, crc_init_value, RDD_CRC_TYPE_16 );

    hash_index = ( ( ( crc_result * 2 ) & ( ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE * 2  ) - 1 ) ) / 2 );

    for ( tries = 0; tries < RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH; tries++ )
    {
        entry_index = ( hash_index + tries ) & ( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE - 1 );

        if ( xi_direction == rdpa_dir_ds )
        {
            ingress_classification_lookup_entry_ptr = &( ds_ingress_classification_lookup_table_ptr->entry[ entry_index ] );
        }
        else
        {
            ingress_classification_lookup_entry_ptr = &( us_ingress_classification_lookup_table_ptr->entry[ entry_index ] );
        }

        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_VALID_READ ( entry_valid, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_READ ( entry_skipped, ingress_classification_lookup_entry_ptr );

        if ( !( entry_valid ) && !( entry_skipped ) )
        {
            return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY );
        }

        if ( entry_skipped )
        {
            continue;
        }

        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_INDEX_READ ( ingress_classification_entry_key_index, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_0_READ ( ingress_classification_entry_key_0, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_1_READ ( ingress_classification_entry_key_1, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_2_READ ( ingress_classification_entry_key_2, ingress_classification_lookup_entry_ptr );
        RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_3_READ ( ingress_classification_entry_key_3, ingress_classification_lookup_entry_ptr );

        if ( ( ingress_classification_entry_key_index == xi_hash_entry[ 1 ] ) &&
             ( ingress_classification_entry_key_0 == ( ( xi_hash_entry[ 2 ] << 16 ) | ( xi_hash_entry[ 3 ] << 8 ) | ( xi_hash_entry[ 4 ] << 0 ) ) ) &&
             ( ingress_classification_entry_key_1 == ( ( xi_hash_entry[ 5 ] << 24 ) | ( xi_hash_entry[ 6 ] << 16 ) | ( xi_hash_entry[ 7 ] << 8 ) | ( xi_hash_entry[ 8 ] << 0 ) ) ) &&
             ( ingress_classification_entry_key_2 == ( ( xi_hash_entry[ 9 ] << 16 ) | ( xi_hash_entry[ 10 ] << 8 ) | ( xi_hash_entry[ 11 ] << 0 ) ) ) &&
             ( ingress_classification_entry_key_3 == ( ( xi_hash_entry[ 12 ] << 24 ) | ( xi_hash_entry[ 13 ] << 16 ) | ( xi_hash_entry[ 14 ] << 8 ) | ( xi_hash_entry[ 15 ] << 0 ) ) ) )
        {
            break;
        }
    }

    if ( tries == RDD_INGRESS_CLASSIFICATION_SEARCH_DEPTH )
    {
        return ( BL_LILAC_RDD_ERROR_HASH_TABLE_NO_MATCHING_KEY );
    }

    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_INDEX_WRITE ( 0, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_0_WRITE ( 0, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_1_WRITE ( 0, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_2_WRITE ( 0, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_3_WRITE ( 0, ingress_classification_lookup_entry_ptr );
    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_CONTEXT_WRITE ( 0, ingress_classification_lookup_entry_ptr );

    RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_WRITE ( LILAC_RDD_ON, ingress_classification_lookup_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ingress_classification_context_counter_read ( rdpa_traffic_dir   xi_direction,
                                                                         uint8_t            xi_context_id,
                                                                         uint16_t           *xo_counter )
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS  *ds_ingress_classification_counters_table_ptr;
    RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS  *us_ingress_classification_counters_table_ptr;
    uint16_t                                          *counter_entry_ptr;

    if ( xi_direction == rdpa_dir_ds )
    {
        ds_ingress_classification_counters_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();
        counter_entry_ptr = ( uint16_t * )&( ds_ingress_classification_counters_table_ptr->entry[ xi_context_id ] );
    }
    else
    {
        us_ingress_classification_counters_table_ptr = RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR();
        counter_entry_ptr = ( uint16_t * )&( us_ingress_classification_counters_table_ptr->entry[ xi_context_id ] );
    }

    MREAD_16( counter_entry_ptr, *xo_counter );
    MWRITE_16( counter_entry_ptr, 0 );
#endif
    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ds_wan_flow_config ( uint32_t                                        xi_wan_flow,
                                                rdpa_cpu_reason                                 xi_cpu_reason,
                                                BL_LILAC_RDD_DOWNSTREAM_FLOW_CLASSIFY_MODE_DTE  xi_flow_classify_mode,
                                                uint8_t                                         xi_ingress_flow )
{
    RDD_DS_WAN_FLOW_TABLE_DTS  *wan_flow_table_ptr;
    RDD_DS_WAN_FLOW_ENTRY_DTS  *wan_flow_entry_ptr;

    wan_flow_table_ptr = ( RDD_DS_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_FLOW_TABLE_ADDRESS );

    wan_flow_entry_ptr = &( wan_flow_table_ptr->entry[ xi_wan_flow ] );

    RDD_DS_WAN_FLOW_ENTRY_INGRESS_FLOW_WRITE ( xi_ingress_flow, wan_flow_entry_ptr );
    RDD_DS_WAN_FLOW_ENTRY_INGRESS_CLASSIFY_MODE_WRITE ( xi_flow_classify_mode, wan_flow_entry_ptr );
    RDD_DS_WAN_FLOW_ENTRY_CPU_REASON_WRITE ( xi_cpu_reason, wan_flow_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_us_wan_dsl_bonding_config ( int xi_ptm_bonding)
{
    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_us_wan_flow_config ( uint32_t                      xi_wan_flow,
                                                RDD_WAN_CHANNEL_ID            xi_wan_channel,
                                                uint32_t                      xi_hdr_type,
                                                uint32_t                      xi_wan_port,
                                                BL_LILAC_RDD_TX_CRC_CALC_DTE  xi_crc_calc,
                                                int                           xi_ptm_bonding,
                                                uint8_t                       xi_pbits_to_queue_table_index,
                                                uint8_t                       xi_traffic_class_to_queue_table_index )
{
    RDD_US_WAN_FLOW_TABLE_DTS  *wan_flow_table_ptr;
    RDD_US_WAN_FLOW_ENTRY_DTS  *wan_flow_entry_ptr;

    wan_flow_table_ptr = ( RDD_US_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_WAN_FLOW_TABLE_ADDRESS );

    wan_flow_entry_ptr = &( wan_flow_table_ptr->entry[ xi_wan_flow ] );

    RDD_US_WAN_FLOW_ENTRY_HDR_TYPE_WRITE(xi_hdr_type, wan_flow_entry_ptr);
    RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_WRITE ( xi_wan_port , wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_CRC_CALC_WRITE ( xi_crc_calc, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_WRITE ( xi_wan_channel, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_WRITE ( xi_pbits_to_queue_table_index, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_WRITE ( xi_traffic_class_to_queue_table_index, wan_flow_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_wan_flow_get ( uint32_t                      xi_wan_flow,
                                             RDD_WAN_CHANNEL_ID            *xo_wan_channel,
                                             uint32_t                      *xo_wan_port,
                                             BL_LILAC_RDD_TX_CRC_CALC_DTE  *xo_crc_calc,
                                             uint8_t                       *xo_pbits_to_queue_table_index,
                                             uint8_t                       *xo_traffic_class_to_queue_table_index )
{
    RDD_US_WAN_FLOW_TABLE_DTS  *wan_flow_table_ptr;
    RDD_US_WAN_FLOW_ENTRY_DTS  *wan_flow_entry_ptr;

    wan_flow_table_ptr = RDD_US_WAN_FLOW_TABLE_PTR();
    wan_flow_entry_ptr = &( wan_flow_table_ptr->entry[ xi_wan_flow ] );

    RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_READ ( *xo_wan_port , wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_CRC_CALC_READ ( *xo_crc_calc, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_READ ( *xo_wan_channel, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_READ ( *xo_pbits_to_queue_table_index, wan_flow_entry_ptr );
    RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_READ ( *xo_traffic_class_to_queue_table_index, wan_flow_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ds_pbits_to_qos_entry_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                          uint32_t                      xi_pbits,
                                                          BL_LILAC_RDD_QUEUE_ID_DTE     xi_qos )
{
    RDD_DS_PBITS_TO_QOS_TABLE_DTS  *pbits_to_qos_table_ptr;
    RDD_PBITS_TO_QOS_ENTRY_DTS     *pbits_to_qos_entry_ptr;
    int32_t                        bridge_port_index;

#ifndef G9991
    /* check the validity of the input parameters - bridge port */
    if ( ( xi_bridge_port < BL_LILAC_RDD_LAN0_BRIDGE_PORT ) || ( ( xi_bridge_port > BL_LILAC_RDD_LAN4_BRIDGE_PORT ) && ( xi_bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT ) ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 );
#else
    /* support only the first entry in the table */
    bridge_port_index = 0;
#endif

    /* check the validity of the input parameters - P-bits */
    if ( xi_pbits > LILAC_RDD_MAX_PBITS )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_PBITS );
    }

    pbits_to_qos_table_ptr = ( RDD_DS_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_PBITS_TO_QOS_TABLE_ADDRESS );

    pbits_to_qos_entry_ptr = &( pbits_to_qos_table_ptr->entry[ bridge_port_index ][ xi_pbits ] );

    RDD_PBITS_TO_QOS_ENTRY_QOS_WRITE ( xi_qos, pbits_to_qos_entry_ptr );

	/* dupilcate table for local switching (prevent stalls)*/
	pbits_to_qos_table_ptr = ( RDD_DS_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_ADDRESS - sizeof ( RUNNER_COMMON ) );

	pbits_to_qos_entry_ptr = &( pbits_to_qos_table_ptr->entry[ bridge_port_index ][ xi_pbits ] );

    RDD_PBITS_TO_QOS_ENTRY_QOS_WRITE ( xi_qos, pbits_to_qos_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_pbits_to_qos_entry_config ( uint8_t                    xi_wan_mapping_table_index,
                                                          uint32_t                   xi_pbits,
                                                          BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue,
                                                          uint8_t                    xi_rate_controller )
{
    RDD_US_PBITS_TO_QOS_TABLE_DTS  *pbits_to_qos_table_ptr;
    RDD_US_QUEUE_ENTRY_DTS         *us_queue_entry_ptr;

    /* check the validity of the input parameters - P-bits */
    if ( xi_pbits > LILAC_RDD_MAX_PBITS || xi_rate_controller > BL_LILAC_RDD_RATE_CONTROLLER_31 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_PBITS );
    }

    /* check the validity of the input parameters - xi_wan_mapping_table_index */
    if ( xi_wan_mapping_table_index >= RDD_US_PBITS_TO_QOS_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_WAN_MAPPING_TABLE_INDEX );
    }

    pbits_to_qos_table_ptr = RDD_US_PBITS_TO_QOS_TABLE_PTR();

    us_queue_entry_ptr = &( pbits_to_qos_table_ptr->entry[ xi_wan_mapping_table_index ][ xi_pbits ] );

    RDD_US_QUEUE_ENTRY_QUEUE_WRITE ( xi_queue, us_queue_entry_ptr );
    RDD_US_QUEUE_ENTRY_RATE_CONTROLLER_WRITE ( xi_rate_controller, us_queue_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_global_config ( uint32_t  xi_dscp,
                                                         uint32_t  xi_pbits )
{
    RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS  *global_dscp_to_pbits_table_ptr;
    RDD_DSCP_TO_PBITS_ENTRY_DTS         *dscp_to_pbits_entry_ptr;

    global_dscp_to_pbits_table_ptr = ( RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_TABLE_ADDRESS );

    dscp_to_pbits_entry_ptr = &( global_dscp_to_pbits_table_ptr->entry[ xi_dscp ] );

    RDD_DSCP_TO_PBITS_ENTRY_PBITS_WRITE ( xi_pbits, dscp_to_pbits_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_dscp_to_pbits_dei_global_config ( uint32_t  xi_dscp,
                                                             uint32_t  xi_pbits,
                                                             uint32_t  xi_dei )
{
    RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS  *global_dscp_to_pbits_dei_table_ptr;
    RDD_DSCP_TO_PBITS_DEI_ENTRY_DTS         *dscp_to_pbits_dei_entry_ptr;

    global_dscp_to_pbits_dei_table_ptr = ( RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_DEI_TABLE_ADDRESS );

    dscp_to_pbits_dei_entry_ptr = &( global_dscp_to_pbits_dei_table_ptr->entry[ xi_dscp ] );

    RDD_DSCP_TO_PBITS_DEI_ENTRY_PBITS_WRITE ( xi_pbits, dscp_to_pbits_dei_entry_ptr );
    RDD_DSCP_TO_PBITS_DEI_ENTRY_DEI_WRITE ( xi_dei, dscp_to_pbits_dei_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}

void rdd_force_dscp_to_pbit_config(rdpa_traffic_dir dir, bdmf_boolean enable)
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *bridge_cfg_register;

    if (dir == rdpa_dir_ds)
    {
        bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET)
            + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);
    }  
    else
    {
        bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET)
            + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);
    }

    RDD_BRIDGE_CONFIGURATION_REGISTER_FORCE_DSCP_TO_PBIT_WRITE(enable, bridge_cfg_register);
}

void rdd_rate_limit_overhead_cfg(uint8_t  xi_rate_limit_overhead)
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *bridge_cfg_register;

    bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET)
            + DS_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);

    RDD_BRIDGE_CONFIGURATION_REGISTER_RATE_LIMIT_OVERHEAD_WRITE(0, bridge_cfg_register);
    
    bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET)
            + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);

    RDD_BRIDGE_CONFIGURATION_REGISTER_RATE_LIMIT_OVERHEAD_WRITE(xi_rate_limit_overhead, bridge_cfg_register);
}

BL_LILAC_RDD_ERROR_DTE rdd_ack_prioritization_config(bdmf_boolean enable)
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *bridge_cfg_register;

    bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);
    RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PRIORITIZATION_WRITE(enable, bridge_cfg_register);

    return ( BL_LILAC_RDD_OK );
}

BL_LILAC_RDD_ERROR_DTE rdd_ack_packet_size_threshold_config(uint8_t threshold)
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *bridge_cfg_register;

    bridge_cfg_register = (RDD_BRIDGE_CONFIGURATION_REGISTER_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS);
    RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PACKET_SIZE_THRESHOLD_WRITE(threshold, bridge_cfg_register);

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_wan_to_wan_us_ingress_flow_config ( uint32_t  xi_ingress_flow )
{
    RDD_BRIDGE_CONFIGURATION_REGISTER_DTS  *bridge_cfg_register;

    bridge_cfg_register = ( RDD_BRIDGE_CONFIGURATION_REGISTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_BRIDGE_CONFIGURATION_REGISTER_ADDRESS );

    RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_TO_WAN_INGRESS_FLOW_WRITE ( xi_ingress_flow, bridge_cfg_register );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_ds_traffic_class_to_queue_entry_config ( BL_LILAC_RDD_BRIDGE_PORT_DTE  xi_bridge_port,
                                                                    uint8_t                       xi_traffic_class,
                                                                    BL_LILAC_RDD_QUEUE_ID_DTE     xi_queue )
{
    RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS  *traffic_class_to_queue_table_ptr;
    RDD_QUEUE_ENTRY_DTS                      *queue_entry_ptr;
    int32_t                                  bridge_port_index;

    /* check the validity of the input parameters - bridge port */
#ifndef G9991
    if ( ( xi_bridge_port < BL_LILAC_RDD_LAN0_BRIDGE_PORT ) || ( ( xi_bridge_port > BL_LILAC_RDD_LAN4_BRIDGE_PORT ) && ( xi_bridge_port != BL_LILAC_RDD_PCI_BRIDGE_PORT ) ) )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_BRIDGE_PORT_ID );
    }

    bridge_port_index = rdd_bridge_port_to_port_index ( xi_bridge_port, 0 );
#else
    /* support only the first entry in the table */
    bridge_port_index = 0;
#endif

    /* check the validity of the input parameters - traffic class */
    if ( xi_traffic_class > RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_TRAFFIC_CLASS );
    }

    traffic_class_to_queue_table_ptr = ( RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_ADDRESS );

    queue_entry_ptr = &( traffic_class_to_queue_table_ptr->entry[ bridge_port_index ][ xi_traffic_class ] );

    RDD_QUEUE_ENTRY_QUEUE_WRITE ( xi_queue, queue_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_traffic_class_to_queue_entry_config ( uint8_t                    xi_wan_mapping_table_index,
                                                                    uint8_t                    xi_traffic_class,
                                                                    BL_LILAC_RDD_QUEUE_ID_DTE  xi_queue,
                                                                    uint8_t                    xi_rate_controller )
{
    RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS  *us_traffic_class_to_queue_table_ptr;
    RDD_US_QUEUE_ENTRY_DTS                   *us_queue_entry_ptr;

    /* check the validity of the input parameters - xi_wan_mapping_table_index */
    if ( xi_wan_mapping_table_index >= RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_WAN_MAPPING_TABLE_INDEX );
    }

    /* check the validity of the input parameters - traffic class */
    if ( xi_traffic_class >= RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_TRAFFIC_CLASS );
    }

    us_traffic_class_to_queue_table_ptr = RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR();

    us_queue_entry_ptr = &( us_traffic_class_to_queue_table_ptr->entry[ xi_wan_mapping_table_index ][ xi_traffic_class ] );

    RDD_US_QUEUE_ENTRY_QUEUE_WRITE ( xi_queue, us_queue_entry_ptr );
    RDD_US_QUEUE_ENTRY_RATE_CONTROLLER_WRITE ( xi_rate_controller, us_queue_entry_ptr );

    return ( BL_LILAC_RDD_OK );
}


BL_LILAC_RDD_ERROR_DTE rdd_us_pbits_to_wan_flow_entry_config ( uint8_t  xi_wan_mapping_table,
                                                               uint8_t  xi_pbits,
                                                               uint8_t  xi_wan_flow )
{
    RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS  *pbits_to_wan_flow_table_ptr;
    uint8_t                             *wan_flow_entry_ptr;

    /* check the validity of the input parameters - xi_wan_mapping_index */
    if ( xi_wan_mapping_table >= RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_PBITS_TO_WAN_FLOW_MAPPING_TABLE );
    }

    /* check the validity of the input parameters - P-bits */
    if ( xi_pbits > LILAC_RDD_MAX_PBITS )
    {
        return ( BL_LILAC_RDD_ERROR_ILLEGAL_PBITS );
    }

    pbits_to_wan_flow_table_ptr = RDD_US_PBITS_TO_WAN_FLOW_TABLE_PTR();
    wan_flow_entry_ptr = ( uint8_t * )&( pbits_to_wan_flow_table_ptr->entry[ xi_wan_mapping_table ][ xi_pbits ] );

    MWRITE_8 ( wan_flow_entry_ptr, xi_wan_flow );

    return ( BL_LILAC_RDD_OK );
}

