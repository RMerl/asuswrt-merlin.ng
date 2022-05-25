/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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

extern rdd_vlan_actions_matrix_t *g_vlan_actions_matrix_ptr;
extern uint32_t g_vlan_mapping_command_to_action[RDD_MAX_VLAN_CMD][RDD_MAX_PBITS_CMD];

void rdd_egress_ethertype_config(uint16_t ether_type_1, uint16_t ether_type_2)
{
    RDD_SYSTEM_CONFIGURATION_DTS *sytem_cfg_register;

    sytem_cfg_register = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);
    RDD_SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_1_WRITE(ether_type_1, sytem_cfg_register);
    RDD_SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_2_WRITE(ether_type_2, sytem_cfg_register);
}

void rdd_vlan_command_always_egress_ether_type_config(uint16_t ether_type_3)
{
    RDD_SYSTEM_CONFIGURATION_DTS *sytem_cfg_register;

    sytem_cfg_register = (RDD_SYSTEM_CONFIGURATION_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SYSTEM_CONFIGURATION_ADDRESS);
    RDD_SYSTEM_CONFIGURATION_EGRESS_ETHER_TYPE_3_WRITE(ether_type_3, sytem_cfg_register);
}

int rdd_ds_pbits_to_qos_entry_cfg(rdd_rdd_vport virtual_port, uint32_t pbits, rdd_tx_queue_id_t qos)
{
    RDD_DS_PBITS_TO_QOS_TABLE_DTS *pbits_to_qos_table_ptr;
    RDD_PBITS_TO_QOS_ENTRY_DTS *pbits_to_qos_entry_ptr;

    pbits_to_qos_table_ptr = RDD_DS_PBITS_TO_QOS_TABLE_PTR();
    pbits_to_qos_entry_ptr = &(pbits_to_qos_table_ptr->entry[virtual_port][pbits]);
    RDD_PBITS_TO_QOS_ENTRY_QOS_WRITE(qos, pbits_to_qos_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_tpid_overwrite_table_cfg(uint16_t *tpid_overwrite_array, rdpa_traffic_dir direction)
{
    RDD_DS_TPID_OVERWRITE_TABLE_DTS *tpid_overwrite_table_ptr;
    RDD_TPID_OVERWRITE_ENTRY_DTS *tpid_overwrite_entry_ptr;
    uint32_t i;

    if (direction == rdpa_dir_ds)
        tpid_overwrite_table_ptr = RDD_DS_TPID_OVERWRITE_TABLE_PTR();
    else
        return BDMF_ERR_PARM;

    for (i = 0; i < RDD_DS_TPID_OVERWRITE_TABLE_SIZE; i++)
    {
        tpid_overwrite_entry_ptr = &(tpid_overwrite_table_ptr->entry[i]);
        RDD_TPID_OVERWRITE_ENTRY_TPID_WRITE(tpid_overwrite_array[i], tpid_overwrite_entry_ptr);
    }

    return BDMF_ERR_OK;
}

int rdd_ds_tc_to_queue_entry_cfg(rdd_rdd_vport virtual_port, uint8_t tc, rdd_tx_queue_id_t queue)
{
    RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS *traffic_class_to_queue_table_ptr;
    RDD_QUEUE_ENTRY_DTS *queue_entry_ptr;

    traffic_class_to_queue_table_ptr = RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR();
    queue_entry_ptr = &(traffic_class_to_queue_table_ptr->entry[virtual_port][tc]);
    RDD_QUEUE_ENTRY_QUEUE_WRITE(queue, queue_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_vlan_cmd_cfg(rdpa_traffic_dir direction, rdd_vlan_cmd_param_t *vlan_command_params)
{
    RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS *ds_vlan_optimization_table_ptr;
    RDD_VLAN_OPTIMIZATION_ENTRY_DTS *vlan_optimization_entry_ptr;
    RDD_DS_VLAN_PARAMETER_TABLE_DTS *ds_vlan_parameters_table_ptr;
    RDD_VLAN_PARAMETER_ENTRY_DTS *vlan_parameters_entry_ptr;
    RDD_DS_PBITS_PARAMETER_TABLE_DTS *ds_pbits_parameters_table_ptr;
    RDD_PBITS_PARAMETER_ENTRY_DTS *pbits_parameters_entry_ptr;
    uint32_t vlan_optimize;

    if (g_vlan_mapping_command_to_action[vlan_command_params->vlan_command][vlan_command_params->pbits_command] == 0)
        vlan_optimize = (1 << 7);
    else
        vlan_optimize = (0 << 7);

    if (vlan_command_params->vlan_command == RDD_VLAN_CMD_ADD_TAG_ALWAYS || vlan_command_params->vlan_command == RDD_VLAN_CMD_REMOVE_TAG_ALWAYS)
        vlan_optimize |= (1 << 6);

    vlan_optimize |= g_vlan_mapping_command_to_action[vlan_command_params->vlan_command][vlan_command_params->pbits_command];

    /* write vlan and P-bits parameters */
    ds_vlan_parameters_table_ptr = RDD_DS_VLAN_PARAMETER_TABLE_PTR();
    vlan_parameters_entry_ptr = &(ds_vlan_parameters_table_ptr->entry[vlan_command_params->vlan_command_id]);

    ds_pbits_parameters_table_ptr = RDD_DS_PBITS_PARAMETER_TABLE_PTR();
    pbits_parameters_entry_ptr = &(ds_pbits_parameters_table_ptr->entry[vlan_command_params->vlan_command_id]);

    ds_vlan_optimization_table_ptr = RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR();
    vlan_optimization_entry_ptr = &(ds_vlan_optimization_table_ptr->entry[vlan_command_params->vlan_command_id]);

    RDD_VLAN_PARAMETER_ENTRY_OUTER_VID_WRITE(vlan_command_params->outer_vid, vlan_parameters_entry_ptr);
    RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_ID_WRITE(vlan_command_params->outer_tpid_id, vlan_parameters_entry_ptr);
    RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_OVERWRITE_ENABLE_WRITE(vlan_command_params->outer_tpid_overwrite_enable, vlan_parameters_entry_ptr);

    RDD_VLAN_PARAMETER_ENTRY_INNER_VID_WRITE(vlan_command_params->inner_vid, vlan_parameters_entry_ptr);
    RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_ID_WRITE(vlan_command_params->inner_tpid_id, vlan_parameters_entry_ptr);
    RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_OVERWRITE_ENABLE_WRITE(vlan_command_params->inner_tpid_overwrite_enable, vlan_parameters_entry_ptr);

    RDD_PBITS_PARAMETER_ENTRY_OUTER_PBITS_WRITE(vlan_command_params->outer_pbits, pbits_parameters_entry_ptr);
    RDD_PBITS_PARAMETER_ENTRY_INNER_PBITS_WRITE(vlan_command_params->inner_pbits, pbits_parameters_entry_ptr);

    RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE(vlan_optimize, vlan_optimization_entry_ptr);

    return BDMF_ERR_OK;
} 

int rdd_dscp_to_pbits_global_cfg(uint32_t dscp, uint32_t pbits)
{
    RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS *global_dscp_to_pbits_table_ptr;
    RDD_DSCP_TO_PBITS_ENTRY_DTS *dscp_to_pbits_entry_ptr;

    global_dscp_to_pbits_table_ptr = RDD_GLOBAL_DSCP_TO_PBITS_TABLE_PTR();
    dscp_to_pbits_entry_ptr = &(global_dscp_to_pbits_table_ptr->entry[dscp]);
    RDD_DSCP_TO_PBITS_ENTRY_PBITS_WRITE(pbits, dscp_to_pbits_entry_ptr);

    return BDMF_ERR_OK;
}

int rdd_pbits_to_pbits_config(uint32_t table_number, uint32_t input_pbits, uint32_t output_pbits)
{
    RDD_DS_PBITS_TO_PBITS_TABLE_DTS *ds_pbits_to_pbits_table_ptr;
    RDD_PBITS_TO_PBITS_ENTRY_DTS *pbits_to_pbits_entry_ptr;

    ds_pbits_to_pbits_table_ptr = RDD_DS_PBITS_TO_PBITS_TABLE_PTR();
    pbits_to_pbits_entry_ptr = &(ds_pbits_to_pbits_table_ptr->entry[table_number][input_pbits]);
    RDD_PBITS_TO_PBITS_ENTRY_PBITS_WRITE(output_pbits, pbits_to_pbits_entry_ptr);

    return BDMF_ERR_OK;
}

void _rdd_vlan_matrix_initialize(void)
{
    RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS *ds_vlan_optimization_table_ptr;
    RDD_VLAN_OPTIMIZATION_ENTRY_DTS *vlan_optimization_entry_ptr;
    RDD_DS_VLAN_COMMANDS_TABLE_DTS *ds_vlan_commands_table_ptr;
    RDD_VLAN_COMMAND_ENRTY_DTS *vlan_command_entry_ptr;
    rdd_vlan_action_entry_t *vlan_action_entry_ptr;
    uint32_t vlan_action_counter;
    uint32_t vlan_command_id;
    uint32_t pbits_command_id;
    uint32_t tag_state;
    uint32_t action_exist;

    /* initialize vlan optimization table */
    ds_vlan_optimization_table_ptr = RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR();

    for (vlan_command_id = 0; vlan_command_id < RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE; vlan_command_id++)
    {
        vlan_optimization_entry_ptr = &(ds_vlan_optimization_table_ptr->entry[vlan_command_id]);

        RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE((1 << 7), vlan_optimization_entry_ptr);
    }

    /* initialize vlan action matrix */
    g_vlan_actions_matrix_ptr = (rdd_vlan_actions_matrix_t *)bdmf_alloc(sizeof(rdd_vlan_actions_matrix_t));

    if (g_vlan_actions_matrix_ptr == NULL)
        return;

    memset(g_vlan_actions_matrix_ptr, 0, sizeof(rdd_vlan_actions_matrix_t));

    /* Untagged Matrix */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TWO_TAGS][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 5;
    vlan_action_entry_ptr->pbits_action = 7;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TWO_TAGS][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 5;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_TAG_ALWAYS][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 11;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_ADD_3RD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][RDD_VLAN_CMD_REMOVE_TAG_ALWAYS][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Priority Tagged Matrix */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_TWO_TAGS][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_TWO_TAGS][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REMOVE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_REMOVE_TWO_TAGS][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 4;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_ADD_3RD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;

    /* use PBITS_REMAP entry in table, but action is PBITS transparent */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Single Tagged Matrix */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 3;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 1;
    vlan_action_entry_ptr->pbits_action = 9;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 1;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REMOVE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_REMOVE_TWO_TAGS][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 14;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 4;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_ADD_3RD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;

    /* use PBITS_REMAP entry in table, but action is PBITS transparent */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;


    /* Double Tagged Matrix */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 5;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 3;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 2;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 4;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_TWO_TAGS][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 8;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 0;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 10;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 9;
    vlan_action_entry_ptr->pbits_action = 2;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 18;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REPLACE_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 18;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_REMOVE_OUTER_TAG_COPY][RDD_PBITS_CMD_TRANSPARENT]);
    vlan_action_entry_ptr->vlan_action = 20;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_ADD_3RD_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 15;
    vlan_action_entry_ptr->pbits_action = 3;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_COPY]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 10;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_ADD_OUTER_TAG_REPLACE_INNER_TAG][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 6;
    vlan_action_entry_ptr->pbits_action = 11;

    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_CONFIGURED]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 5;

    /* use PBITS_REMAP entry in table, but action is PBITS transparent */
    vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][RDD_VLAN_CMD_TRANSPARENT][RDD_PBITS_CMD_REMAP]);
    vlan_action_entry_ptr->vlan_action = 3;
    vlan_action_entry_ptr->pbits_action = 6;

    vlan_action_counter = 1;

    for (vlan_command_id = RDD_VLAN_CMD_TRANSPARENT; vlan_command_id < RDD_MAX_VLAN_CMD; vlan_command_id++)
    {
        for (pbits_command_id = RDD_PBITS_CMD_TRANSPARENT; pbits_command_id < RDD_MAX_PBITS_CMD; pbits_command_id++)
        {
            action_exist = 0;

            for (tag_state = RDD_VLAN_TYPE_UNTAGGED; tag_state <= RDD_VLAN_TYPES; tag_state++)
            {
                if (g_vlan_actions_matrix_ptr->entry[tag_state][vlan_command_id][pbits_command_id].vlan_action)
                {
                    action_exist = 1;

                    break;
                }
            }

            if (action_exist == 1)
            {
                g_vlan_mapping_command_to_action[vlan_command_id][pbits_command_id] = vlan_action_counter;

                ds_vlan_commands_table_ptr = RDD_DS_VLAN_COMMANDS_TABLE_PTR();
                vlan_command_entry_ptr = &(ds_vlan_commands_table_ptr->entry[vlan_action_counter]);

                vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_UNTAGGED][vlan_command_id][pbits_command_id]);

                RDD_VLAN_COMMAND_ENRTY_VLAN_UNTAGGED_COMMAND_WRITE(vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr);
                RDD_VLAN_COMMAND_ENRTY_PBITS_UNTAGGED_COMMAND_WRITE(vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr);

                vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_SINGLE][vlan_command_id][pbits_command_id]);

                RDD_VLAN_COMMAND_ENRTY_VLAN_SINGLE_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr);
                RDD_VLAN_COMMAND_ENRTY_PBITS_SINGLE_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr);

                vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_DOUBLE][vlan_command_id][pbits_command_id]);

                RDD_VLAN_COMMAND_ENRTY_VLAN_DOUBLE_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr);
                RDD_VLAN_COMMAND_ENRTY_PBITS_DOUBLE_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr);

                vlan_action_entry_ptr = &(g_vlan_actions_matrix_ptr->entry[RDD_VLAN_TYPE_PRIORITY][vlan_command_id][pbits_command_id]);

                RDD_VLAN_COMMAND_ENRTY_VLAN_PRIORITY_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->vlan_action, vlan_command_entry_ptr);
                RDD_VLAN_COMMAND_ENRTY_PBITS_PRIORITY_TAGGED_COMMAND_WRITE(vlan_action_entry_ptr->pbits_action, vlan_command_entry_ptr);

                vlan_action_counter++;
            }
            else
                g_vlan_mapping_command_to_action[vlan_command_id][pbits_command_id] = 0;
        }
    }
}
