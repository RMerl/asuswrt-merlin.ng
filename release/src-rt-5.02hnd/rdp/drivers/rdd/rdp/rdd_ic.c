/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

void rdd_ic_debug_mode_enable(bdmf_boolean enable)
{
    /* TODO */
}

#ifdef CM3390
int rdd_ic_context_cfg(rdpa_traffic_dir direction, uint32_t context_id, const rdd_ic_context_t *context)
{
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS *ds_ingress_classification_context_table_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS *ds_ingress_classification_context_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_TABLE_DTS *vlan_cmd_idx_table_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS *vlan_cmd_idx_entry_ptr;

    if (direction == rdpa_dir_ds)
    {
        if (context_id >= RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE)
            return BDMF_ERR_RANGE;

        ds_ingress_classification_context_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();
        ds_ingress_classification_context_entry_ptr = &(ds_ingress_classification_context_table_ptr->entry[context_id]);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_WRITE(((VLAN_COMMAND_INDEX_TABLE_ADDRESS +
            (context_id * sizeof(RDD_VLAN_COMMAND_INDEX_ENTRY_DTS))) >> 3), ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE(context->opbit_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE(context->ipbit_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_WRITE(context->wifi_ssid, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_WRITE(context->subnet_id, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_WRITE(context->forw_mode, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_WRITE(context->egress_port, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(context->qos_method, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(context->priority, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE((context->action == rdpa_forward_action_drop) ? 1 : 0, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE((context->action == rdpa_forward_action_host) ? 1 : 0, ds_ingress_classification_context_entry_ptr);

        if (context->policer < 0)
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE(0, ds_ingress_classification_context_entry_ptr);
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE(0, ds_ingress_classification_context_entry_ptr);
        }
        else
        {
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE(1, ds_ingress_classification_context_entry_ptr);
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE(context->policer, ds_ingress_classification_context_entry_ptr);
        }

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_WRITE(context->service_queue_mode, ds_ingress_classification_context_entry_ptr);

        if (context->service_queue_mode)
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(context->service_queue, ds_ingress_classification_context_entry_ptr);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE(context->opbit_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE(context->ipbit_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE(context->dscp_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE(context->dscp_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE(context->ecn_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE((context->dei_command == RDD_DEI_CMD_TRANSPARENT) ? 0 : 1,
            ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE((context->dei_command == RDD_DEI_CMD_CLEAR) ? 0 : 1,
            ds_ingress_classification_context_entry_ptr);

        vlan_cmd_idx_table_ptr = RDD_VLAN_COMMAND_INDEX_TABLE_PTR();

        vlan_cmd_idx_entry_ptr = &(vlan_cmd_idx_table_ptr->entry[context_id]);

        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth0_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth1_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth2_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth3_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth4_vlan_command, vlan_cmd_idx_entry_ptr);
#ifndef G9991
#ifdef CM3390
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr);
#endif
        RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.pci_vlan_command, vlan_cmd_idx_entry_ptr);
#else
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth6_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth7_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth8_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth9_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth10_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth11_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth12_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth13_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth14_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth15_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth16_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth17_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth18_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth19_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth20_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth21_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth22_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_WRITE(context->vlan_command_id.ds_vlan_command.eth23_vlan_command, vlan_cmd_idx_entry_ptr);
#endif
    }
    else
        return BDMF_ERR_PARM;

    return BDMF_ERR_OK;
}

int rdd_ic_context_get(rdpa_traffic_dir direction, uint32_t context_id, rdd_ic_context_t *context)
{
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS  *ds_ingress_classification_context_table_ptr;
    RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS  *ds_ingress_classification_context_entry_ptr;
    RDD_VLAN_COMMAND_INDEX_TABLE_DTS                 *vlan_cmd_idx_table_ptr;
    RDD_VLAN_COMMAND_INDEX_ENTRY_DTS                 *vlan_cmd_idx_entry_ptr;
    uint32_t  drop_flow, cpu_flow, policer_enable, dei_remark_enable, dei_value;

    if (direction == rdpa_dir_ds)
    {
        if (context_id >= RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE)
            return BDMF_ERR_RANGE;

        ds_ingress_classification_context_table_ptr = RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR();
        ds_ingress_classification_context_entry_ptr = &(ds_ingress_classification_context_table_ptr->entry[context_id]);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(context->priority, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_READ(context->forw_mode, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_READ(context->egress_port, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(context->qos_method, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_READ(context->subnet_id, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_READ(context->wifi_ssid, ds_ingress_classification_context_entry_ptr);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ(drop_flow, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ(cpu_flow, ds_ingress_classification_context_entry_ptr);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ(policer_enable, ds_ingress_classification_context_entry_ptr);

        if (policer_enable)
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ(context->policer, ds_ingress_classification_context_entry_ptr);
        else
            context->policer = -1;

        context->rate_shaper = -1;

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_READ(context->service_queue_mode, ds_ingress_classification_context_entry_ptr);

        if (context->service_queue_mode)
            RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_READ(context->service_queue, ds_ingress_classification_context_entry_ptr);

        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ(context->opbit_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ(context->ipbit_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ(context->opbit_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ(context->ipbit_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ(context->dscp_remark, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ(context->dscp_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ(context->ecn_val, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ(dei_remark_enable, ds_ingress_classification_context_entry_ptr);
        RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ(dei_value, ds_ingress_classification_context_entry_ptr);

        vlan_cmd_idx_table_ptr = RDD_VLAN_COMMAND_INDEX_TABLE_PTR();

        vlan_cmd_idx_entry_ptr = &(vlan_cmd_idx_table_ptr->entry[context_id]);

        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth0_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth1_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth2_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth3_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth4_vlan_command, vlan_cmd_idx_entry_ptr);
#ifndef G9991
#ifdef CM3390
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr);
#endif
        RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.pci_vlan_command, vlan_cmd_idx_entry_ptr);
#else
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth5_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth6_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth7_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth8_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth9_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth10_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth11_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth12_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth13_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth14_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth15_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth16_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth17_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth18_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth19_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth20_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth21_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth22_vlan_command, vlan_cmd_idx_entry_ptr);
        RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_READ(context->vlan_command_id.ds_vlan_command.eth23_vlan_command, vlan_cmd_idx_entry_ptr);
#endif
    }
    else
        return BDMF_ERR_PARM;

    if (dei_remark_enable)
    {
        if (dei_value)
            context->dei_command = RDD_DEI_CMD_SET;
        else
            context->dei_command = RDD_DEI_CMD_CLEAR;
    }
    else
        context->dei_command = RDD_DEI_CMD_TRANSPARENT;

    if (drop_flow)
        context->action = rdpa_forward_action_drop;
    else if (cpu_flow)
        context->action = rdpa_forward_action_host;
    else
        context->action = rdpa_forward_action_forward;

    return BDMF_ERR_OK;
}
#endif
