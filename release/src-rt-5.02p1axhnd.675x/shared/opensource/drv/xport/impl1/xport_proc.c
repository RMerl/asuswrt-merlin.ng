/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_proc.c
 *
 */

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>

#include "proc_cmd.h"
#include "xport_drv.h"
#include "bcm63158_xport_xlmac_core_ag.h"
#include "bcm63158_xport_xlmac_reg_ag.h"
#include "bcm63158_xport_reg_ag.h"
#include "bcm63158_xport_mib_reg_ag.h"
#include "bcm63158_xport_mab_ag.h"
#include "bcm63158_drivers_xport_ag.h"
#include "xport_xlmac_indirect_access.h"

#include "bcm_map_part.h"

#define PROC_DIR           "driver/xport"
#define CMD_PROC_FILE      "cmd"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *cmd_proc_file;

static int xport_port_validate(int argc, char *argv[], int port_pos, int min, int max)
{
    int port;

    if (argc < port_pos+1)
        goto error;

    if (kstrtos32(argv[port_pos], 10, &port))
        goto error;

    if (port < min || port > max)
        goto error;

    return port;

error:
    return -1;
}
static char *xport_duplex_to_str(XPORT_PORT_DUPLEX duplex)
{
    return duplex == XPORT_HALF_DUPLEX ? "Half duplex" : "Full duplex";
}

static int xport_proc_cmd_status(int argc, char *argv[])
{
    int port;
    xport_port_status_s port_status;
    char *xport_status_usage = "Usage: status <port id (0-1)>";

    if ((port = xport_port_validate(argc, argv, 1, 0, 1)) == -1)
    {
        pr_info("%s\n",xport_status_usage);
        return -1;
    }

    xport_get_port_status(port, &port_status);
    pr_info("Port %d status:\n", port);
    pr_info("\tautoneg_en: %d\n", port_status.autoneg_en);
    pr_info("\tport_up: %d\n", port_status.port_up);
    pr_info("\trate: %s\n", xport_rate_to_str(port_status.rate));
    pr_info("\tduplex: %s\n", xport_duplex_to_str(port_status.duplex));
    pr_info("\trx_pause_en: %d\n", port_status.rx_pause_en);
    pr_info("\ttx_pause_en: %d\n", port_status.tx_pause_en);
    pr_info("\tmac_rx_en: %d\n", port_status.mac_rx_en);
    pr_info("\tmac_tx_en: %d\n", port_status.mac_tx_en);
    pr_info("\tmac_lpbk: %d\n", port_status.mac_lpbk);

    return 0;
}

void xport_proc_xlmac_core_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_ctrl ctrl;
    ag_drv_xport_xlmac_core_ctrl_get(port_id, &ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_XLMAC_CTRL               :\n",port_id);
    pr_info("extended_hig2_en                                   = 0x%02x\n",ctrl.extended_hig2_en);
    pr_info("link_status_select                                 = 0x%02x\n",ctrl.link_status_select);
    pr_info("sw_link_status                                     = 0x%02x\n",ctrl.sw_link_status);
    pr_info("xgmii_ipg_check_disable                            = 0x%02x\n",ctrl.xgmii_ipg_check_disable);
    pr_info("rs_soft_reset                                      = 0x%02x\n",ctrl.rs_soft_reset);
    pr_info("local_lpbk_leak_enb                                = 0x%02x\n",ctrl.local_lpbk_leak_enb);
    pr_info("soft_reset                                         = 0x%02x\n",ctrl.soft_reset);
    pr_info("lag_failover_en                                    = 0x%02x\n",ctrl.lag_failover_en);
    pr_info("remove_failover_lpbk                               = 0x%02x\n",ctrl.remove_failover_lpbk);
    pr_info("local_lpbk                                         = 0x%02x\n",ctrl.local_lpbk);
    pr_info("rx_en                                              = 0x%02x\n",ctrl.rx_en);
    pr_info("tx_en                                              = 0x%02x\n",ctrl.tx_en);

}
void xport_proc_xlmac_core_mode_dump(uint8_t port_id)
{
    uint8_t speed_mode, no_sop_for_crc_hg, hdr_mode;
    ag_drv_xport_xlmac_core_mode_get(port_id,&speed_mode,&no_sop_for_crc_hg,&hdr_mode);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_XLMAC_MODE:\n",port_id);
    pr_info("speed_mode                                         = 0x%02x\n",speed_mode);
    pr_info("no_sop_for_crc_hg                                  = 0x%02x\n",no_sop_for_crc_hg);
    pr_info("hdr_mode                                           = 0x%02x\n",hdr_mode);

}
void xport_proc_xlmac_core_spare0_dump(uint8_t port_id)
{
}
void xport_proc_xlmac_core_spare1_dump(uint8_t port_id)
{
}
void xport_proc_xlmac_core_tx_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_tx_ctrl ctrl;
    ag_drv_xport_xlmac_core_tx_ctrl_get(port_id,&ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_CTRL                  :\n",port_id);
    pr_info("tx_threshold                                       = 0x%02x\n",ctrl.tx_threshold);
    pr_info("ep_discard                                         = 0x%02x\n",ctrl.ep_discard);
    pr_info("tx_preamble_length                                 = 0x%02x\n",ctrl.tx_preamble_length);
    pr_info("throt_denom                                        = 0x%02x\n",ctrl.throt_denom);
    pr_info("throt_num                                          = 0x%02x\n",ctrl.throt_num);
    pr_info("average_ipg                                        = 0x%02x\n",ctrl.average_ipg);
    pr_info("pad_threshold                                      = 0x%02x\n",ctrl.pad_threshold);
    pr_info("pad_en                                             = 0x%02x\n",ctrl.pad_en);
    pr_info("tx_any_start                                       = 0x%02x\n",ctrl.tx_any_start);
    pr_info("discard                                            = 0x%02x\n",ctrl.discard);
    pr_info("crc_mode                                           = 0x%02x\n",ctrl.crc_mode);
}
void xport_proc_xlmac_core_tx_mac_sa_dump(uint8_t port_id)
{
    uint64_t ctrl_sa;
    ag_drv_xport_xlmac_core_tx_mac_sa_get(port_id,&ctrl_sa);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_MAC_SA                = <0x%016llu>\n",port_id,*(uint64_t*)&ctrl_sa);
}
void xport_proc_xlmac_core_rx_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_rx_ctrl rx_ctrl;
    ag_drv_xport_xlmac_core_rx_ctrl_get(port_id,&rx_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_CTRL                  :\n",port_id);
    pr_info("rx_pass_pfc                                        = 0x%02x\n",rx_ctrl.rx_pass_pfc);
    pr_info("rx_pass_pause                                      = 0x%02x\n",rx_ctrl.rx_pass_pause);
    pr_info("rx_pass_ctrl                                       = 0x%02x\n",rx_ctrl.rx_pass_ctrl);
    pr_info("rsvd_3                                             = 0x%02x\n",rx_ctrl.rsvd_3);
    pr_info("rsvd_2                                             = 0x%02x\n",rx_ctrl.rsvd_2);
    pr_info("runt_threshold                                     = 0x%02x\n",rx_ctrl.runt_threshold);
    pr_info("strict_preamble                                    = 0x%02x\n",rx_ctrl.strict_preamble);
    pr_info("strip_crc                                          = 0x%02x\n",rx_ctrl.strip_crc);
    pr_info("rx_any_start                                       = 0x%02x\n",rx_ctrl.rx_any_start);
    pr_info("rsvd_1                                             = 0x%02x\n",rx_ctrl.rsvd_1);

}
void xport_proc_xlmac_core_rx_mac_sa_dump(uint8_t port_id)
{
    uint64_t rx_sa;
    ag_drv_xport_xlmac_core_rx_mac_sa_get(port_id,&rx_sa);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_MAC_SA                = <0x%016llu>\n",port_id,*(uint64_t*)&rx_sa);
}
void xport_proc_xlmac_core_rx_max_size_dump(uint8_t port_id)
{
    uint16_t rx_max_size;
    ag_drv_xport_xlmac_core_rx_max_size_get(port_id,&rx_max_size);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_MAX_SIZE              = <%d>\n",port_id,rx_max_size);

}
void xport_proc_xlmac_core_rx_vlan_tag_dump(uint8_t port_id)
{
    uint8_t outer_vlan_tag_enable; uint8_t inner_vlan_tag_enable; uint16_t outer_vlan_tag; uint16_t inner_vlan_tag;
    ag_drv_xport_xlmac_core_rx_vlan_tag_get(port_id,&outer_vlan_tag_enable,&inner_vlan_tag_enable,&outer_vlan_tag,&inner_vlan_tag);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_VLAN_TAG              = \n",port_id);
    pr_info("outer_vlan_tag_enable                              = 0x%02x\n",outer_vlan_tag_enable);
    pr_info("inner_vlan_tag_enable                              = 0x%02x\n",inner_vlan_tag_enable);
    pr_info("outer_vlan_tag                                     = 0x%02x\n",outer_vlan_tag);
    pr_info("inner_vlan_tag                                     = 0x%02x\n",inner_vlan_tag);

}
void xport_proc_xlmac_core_rx_lss_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_rx_lss_ctrl rx_lss_ctrl;
    ag_drv_xport_xlmac_core_rx_lss_ctrl_get(port_id,&rx_lss_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_LSS_CTRL              :\n",port_id);
    pr_info("reset_flow_control_timers_on_link_down             = 0x%02x\n",rx_lss_ctrl.reset_flow_control_timers_on_link_down);
    pr_info("drop_tx_data_on_link_interrupt                     = 0x%02x\n",rx_lss_ctrl.drop_tx_data_on_link_interrupt);
    pr_info("drop_tx_data_on_remote_fault                       = 0x%02x\n",rx_lss_ctrl.drop_tx_data_on_remote_fault);
    pr_info("drop_tx_data_on_local_fault                        = 0x%02x\n",rx_lss_ctrl.drop_tx_data_on_local_fault);
    pr_info("link_interruption_disable                          = 0x%02x\n",rx_lss_ctrl.link_interruption_disable);
    pr_info("use_external_faults_for_tx                         = 0x%02x\n",rx_lss_ctrl.use_external_faults_for_tx);
    pr_info("remote_fault_disable                               = 0x%02x\n",rx_lss_ctrl.remote_fault_disable);
    pr_info("local_fault_disable                                = 0x%02x\n",rx_lss_ctrl.local_fault_disable);

}
void xport_proc_xlmac_core_rx_lss_status_dump(uint8_t port_id)
{
    uint8_t link_interruption_status; uint8_t remote_fault_status; uint8_t local_fault_status;
    ag_drv_xport_xlmac_core_rx_lss_status_get(port_id,&link_interruption_status,&remote_fault_status,&local_fault_status);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_LSS_STATUS:\n",port_id);
    pr_info("link_interruption_status                           = 0x%02x\n",link_interruption_status);
    pr_info("remote_fault_status                                = 0x%02x\n",remote_fault_status);
    pr_info("local_fault_status                                 = 0x%02x\n",local_fault_status);
}
void xport_proc_xlmac_core_clear_rx_lss_status_dump(uint8_t port_id)
{
    uint8_t clear_link_interruption_status; uint8_t clear_remote_fault_status; uint8_t clear_local_fault_status;
    ag_drv_xport_xlmac_core_clear_rx_lss_status_get(port_id,&clear_link_interruption_status,&clear_remote_fault_status,&clear_local_fault_status);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_CLEAR_RX_LSS_STATUS:\n",port_id);
    pr_info("clear_link_interruption_status                     = 0x%02x\n",clear_link_interruption_status);
    pr_info("clear_remote_fault_status                          = 0x%02x\n",clear_remote_fault_status);
    pr_info("clear_local_fault_status                           = 0x%02x\n",clear_local_fault_status);

}
void xport_proc_xlmac_core_pause_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_pause_ctrl pause_ctrl;
    ag_drv_xport_xlmac_core_pause_ctrl_get(port_id,&pause_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_PAUSE_CTRL               :\n",port_id);
    pr_info("pause_xoff_timer                                   = 0x%02x\n",pause_ctrl.pause_xoff_timer);
    pr_info("rsvd_2                                             = 0x%02x\n",pause_ctrl.rsvd_2);
    pr_info("rsvd_1                                             = 0x%02x\n",pause_ctrl.rsvd_1);
    pr_info("rx_pause_en                                        = 0x%02x\n",pause_ctrl.rx_pause_en);
    pr_info("tx_pause_en                                        = 0x%02x\n",pause_ctrl.tx_pause_en);
    pr_info("pause_refresh_en                                   = 0x%02x\n",pause_ctrl.pause_refresh_en);
    pr_info("pause_refresh_timer                                = 0x%02x\n",pause_ctrl.pause_refresh_timer);

}
void xport_proc_xlmac_core_pfc_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_pfc_ctrl pfc_ctrl;
    ag_drv_xport_xlmac_core_pfc_ctrl_get(port_id,&pfc_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_PFC_CTRL                 :\n",port_id);
    pr_info("tx_pfc_en                                          = 0x%02x\n",pfc_ctrl.tx_pfc_en);		
    pr_info("rx_pfc_en                                          = 0x%02x\n",pfc_ctrl.rx_pfc_en);		
    pr_info("pfc_stats_en                                       = 0x%02x\n",pfc_ctrl.pfc_stats_en);		
    pr_info("rsvd                                               = 0x%02x\n",pfc_ctrl.rsvd);			
    pr_info("force_pfc_xon                                      = 0x%02x\n",pfc_ctrl.force_pfc_xon);	
    pr_info("pfc_refresh_en                                     = 0x%02x\n",pfc_ctrl.pfc_refresh_en);
    pr_info("pfc_xoff_timer                                     = 0x%02x\n",pfc_ctrl.pfc_xoff_timer);
    pr_info("pfc_refresh_timer                                  = 0x%02x\n",pfc_ctrl.pfc_refresh_timer);

}
void xport_proc_xlmac_core_pfc_type_dump(uint8_t port_id)
{
    uint16_t pfc_eth_type;
    ag_drv_xport_xlmac_core_pfc_type_get(port_id,&pfc_eth_type);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_PFC_TYPE                 = <0x%04u>\n",port_id,pfc_eth_type);
}
void xport_proc_xlmac_core_pfc_opcode_dump(uint8_t port_id)
{
    uint16_t pfc_opcode;
    ag_drv_xport_xlmac_core_pfc_opcode_get(port_id,&pfc_opcode);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_PFC_OPCODE               = <0x%04u>\n",port_id,pfc_opcode);
}
void xport_proc_xlmac_core_pfc_da_dump(uint8_t port_id)
{
    uint64_t pfc_macda;
    ag_drv_xport_xlmac_core_pfc_da_get(port_id,&pfc_macda);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_PFC_DA                   = <0x%016llu>\n",port_id,*(uint64_t*)&pfc_macda);
}
void xport_proc_xlmac_core_llfc_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_llfc_ctrl llfc_ctrl;
    ag_drv_xport_xlmac_core_llfc_ctrl_get(port_id,&llfc_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_LLFC_CTRL                :\n",port_id);
    pr_info("llfc_img                                           = 0x%02x\n",llfc_ctrl.llfc_img);			
    pr_info("no_som_for_crc_llfc	                            = 0x%02x\n",llfc_ctrl.no_som_for_crc_llfc);	
    pr_info("llfc_crc_ignore		                            = 0x%02x\n",llfc_ctrl.llfc_crc_ignore);		
    pr_info("llfc_cut_through_mode	                            = 0x%02x\n",llfc_ctrl.llfc_cut_through_mode);	
    pr_info("llfc_in_ipg_only                                   = 0x%02x\n",llfc_ctrl.llfc_in_ipg_only);		
    pr_info("rx_llfc_en                                         = 0x%02x\n",llfc_ctrl.rx_llfc_en);		
    pr_info("tx_llfc_en                                         = 0x%02x\n",llfc_ctrl.tx_llfc_en);		

}
void xport_proc_xlmac_core_tx_llfc_msg_fields_dump(uint8_t port_id)
{
    uint16_t llfc_xoff_time; uint8_t tx_llfc_fc_obj_logical; uint8_t tx_llfc_msg_type_logical;
    ag_drv_xport_xlmac_core_tx_llfc_msg_fields_get(port_id,&llfc_xoff_time,&tx_llfc_fc_obj_logical,&tx_llfc_msg_type_logical);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_LLFC_MSG_FIELDS:\n",port_id);
    pr_info("llfc_xoff_time                                     = 0x%02x\n",llfc_xoff_time);
    pr_info("tx_llfc_fc_obj_logical                             = 0x%02x\n",tx_llfc_fc_obj_logical);
    pr_info("tx_llfc_msg_type_logical                           = 0x%02x\n",tx_llfc_msg_type_logical);
}
void xport_proc_xlmac_core_rx_llfc_msg_fields_dump(uint8_t port_id)
{
    uint8_t rx_llfc_fc_obj_physical; uint8_t rx_llfc_msg_type_physical; uint8_t rx_llfc_fc_obj_logical; uint8_t rx_llfc_msg_type_logical;
    ag_drv_xport_xlmac_core_rx_llfc_msg_fields_get(port_id,&rx_llfc_fc_obj_physical,&rx_llfc_msg_type_physical,&rx_llfc_fc_obj_logical,&rx_llfc_msg_type_logical);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_LLFC_MSG_FIELDS:\n",port_id);
    pr_info("rx_llfc_fc_obj_physical                            = 0x%02x\n",rx_llfc_fc_obj_physical);
    pr_info("rx_llfc_msg_type_physical                          = 0x%02x\n",rx_llfc_msg_type_physical);
    pr_info("rx_llfc_fc_obj_logical                             = 0x%02x\n",rx_llfc_fc_obj_logical);
    pr_info("rx_llfc_msg_type_logical                           = 0x%02x\n",rx_llfc_msg_type_logical);
}
void xport_xlmac_core_tx_timestamp_fifo_data_dump(uint8_t port_id)
{
    uint8_t ts_entry_valid; uint16_t sequence_id; uint32_t time_stamp;
    ag_drv_xport_xlmac_core_tx_timestamp_fifo_data_get(port_id,&ts_entry_valid,&sequence_id,&time_stamp);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_TIMESTAMP_FIFO_DATA:\n",port_id);
    pr_info("ts_entry_valid                                     = 0x%02x\n",ts_entry_valid);
    pr_info("sequence_id                                        = 0x%04x\n",sequence_id);
    pr_info("time_stamp                                         = 0x%08x\n",time_stamp);
}
void xport_proc_xlmac_core_tx_timestamp_fifo_status_dump(uint8_t port_id)
{
    uint8_t entry_count;
    ag_drv_xport_xlmac_core_tx_timestamp_fifo_status_get(port_id,&entry_count);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_TIMESTAMP_FIFO_STATUS:\n",port_id);
    pr_info("entry_count                                        = 0x%02x\n",entry_count);
}
void xport_proc_xlmac_core_fifo_status_dump(uint8_t port_id)
{
    xport_xlmac_core_fifo_status fifo_status;
    ag_drv_xport_xlmac_core_fifo_status_get(port_id,&fifo_status);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_FIFO_STATUS              :\n",port_id);
    pr_info("link_status                                        = 0x%02x\n",fifo_status.link_status);		
    pr_info("rx_pkt_overflow                                    = 0x%02x\n",fifo_status.rx_pkt_overflow);	
    pr_info("tx_ts_fifo_overflow                                = 0x%02x\n",fifo_status.tx_ts_fifo_overflow);	
    pr_info("tx_llfc_msg_overflow                               = 0x%02x\n",fifo_status.tx_llfc_msg_overflow);	
    pr_info("rsvd_2                                             = 0x%02x\n",fifo_status.rsvd_2);		
    pr_info("tx_pkt_overflow                                    = 0x%02x\n",fifo_status.tx_pkt_overflow);	
    pr_info("tx_pkt_underflow                                   = 0x%02x\n",fifo_status.tx_pkt_underflow);	
    pr_info("rx_msg_overflow                                    = 0x%02x\n",fifo_status.rx_msg_overflow);	
    pr_info("rsvd_1                                             = 0x%02x\n",fifo_status.rsvd_1);		

}
void xport_proc_xlmac_core_clear_fifo_status_dump(uint8_t port_id)
{
    xport_xlmac_core_clear_fifo_status clear_fifo_status;
    ag_drv_xport_xlmac_core_clear_fifo_status_get(port_id,&clear_fifo_status);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_CLEAR_FIFO_STATUS        :\n",port_id);
    pr_info("clear_rx_pkt_overflow                              = 0x%02x\n",clear_fifo_status.clear_rx_pkt_overflow);
    pr_info("clear_tx_ts_fifo_overflow                          = 0x%02x\n",clear_fifo_status.clear_tx_ts_fifo_overflow);
    pr_info("clear_tx_llfc_msg_overflow                         = 0x%02x\n",clear_fifo_status.clear_tx_llfc_msg_overflow);
    pr_info("rsvd_2                                             = 0x%02x\n",clear_fifo_status.rsvd_2);
    pr_info("clear_tx_pkt_overflow                              = 0x%02x\n",clear_fifo_status.clear_tx_pkt_overflow);
    pr_info("clear_tx_pkt_underflow                             = 0x%02x\n",clear_fifo_status.clear_tx_pkt_underflow);
    pr_info("clear_rx_msg_overflow                              = 0x%02x\n",clear_fifo_status.clear_rx_msg_overflow);
    pr_info("rsvd_1                                             = 0x%02x\n",clear_fifo_status.rsvd_1);

}
void xport_proc_xlmac_core_lag_failover_status_dump(uint8_t port_id)
{
    uint8_t rsvd; uint8_t lag_failover_loopback;
    ag_drv_xport_xlmac_core_lag_failover_status_get(port_id,&rsvd,&lag_failover_loopback);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_LAG_FAILOVER_STATUS:\n",port_id);
    pr_info("rsvd                                               = 0x%02x\n",rsvd);
    pr_info("lag_failover_loopback                              = 0x%02x\n",lag_failover_loopback);
}
void xport_proc_xlmac_core_eee_ctrl_dump(uint8_t port_id)
{
    uint8_t rsvd; uint8_t eee_en;
    ag_drv_xport_xlmac_core_eee_ctrl_get(port_id,&rsvd,&eee_en);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_EEE_CTRL:\n",port_id);
    pr_info("rsvd                                               = 0x%02x\n",rsvd);
    pr_info("eee_en                                             = 0x%02x\n",eee_en);
}
void xport_proc_xlmac_core_eee_timers_dump(uint8_t port_id)
{
    uint16_t eee_ref_count; uint16_t eee_wake_timer; uint32_t eee_delay_entry_timer;
    ag_drv_xport_xlmac_core_eee_timers_get(port_id,&eee_ref_count,&eee_wake_timer,&eee_delay_entry_timer);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_EEE_TIMERS:\n",port_id);
    pr_info("eee_ref_count                                      = 0x%04x\n",eee_ref_count);
    pr_info("eee_wake_timer                                     = 0x%04x\n",eee_wake_timer);
    pr_info("eee_delay_entry_timer                              = 0x%08x\n",eee_delay_entry_timer);
}
void xport_proc_xlmac_core_eee_1_sec_link_status_timer_dump(uint8_t port_id)
{
    uint32_t one_second_timer;
    ag_drv_xport_xlmac_core_eee_1_sec_link_status_timer_get(port_id,&one_second_timer);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_EEE_1_SEC_LINK_STATUS_TIMER:\n",port_id);
    pr_info("one_second_timer                                   = 0x%04x\n",one_second_timer);

}
void xport_proc_xlmac_core_higig_hdr_0_dump(uint8_t port_id)
{
    uint64_t higig_hdr_0;
    ag_drv_xport_xlmac_core_higig_hdr_0_get(port_id,&higig_hdr_0);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_HIGIG_HDR_0              = <0x%016llu>\n",port_id,*(uint64_t*)&higig_hdr_0);
}
void xport_proc_xlmac_core_higig_hdr_1_dump(uint8_t port_id)
{
    uint64_t higig_hdr_1;
    ag_drv_xport_xlmac_core_higig_hdr_1_get(port_id,&higig_hdr_1);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_HIGIG_HDR_1              = <0x%016llu>\n",port_id,*(uint64_t*)&higig_hdr_1);
}
void xport_proc_xlmac_core_gmii_eee_ctrl_dump(uint8_t port_id)
{
    uint8_t gmii_lpi_predict_mode_en; uint16_t gmii_lpi_predict_threshold;
    ag_drv_xport_xlmac_core_gmii_eee_ctrl_get(port_id,&gmii_lpi_predict_mode_en,&gmii_lpi_predict_threshold);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_GMII_EEE_CTRL:\n",port_id);
    pr_info("gmii_lpi_predict_mode_en                           = 0x%02x\n",gmii_lpi_predict_mode_en);
    pr_info("gmii_lpi_predict_threshold                         = 0x%04x\n",gmii_lpi_predict_threshold);
}
void xport_proc_xlmac_core_timestamp_adjust_dump(uint8_t port_id)
{
    uint8_t ts_use_cs_offset; uint8_t ts_tsts_adjust; uint16_t ts_osts_adjust;
    ag_drv_xport_xlmac_core_timestamp_adjust_get(port_id,&ts_use_cs_offset,&ts_tsts_adjust,&ts_osts_adjust);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TIMESTAMP_ADJUST:\n",port_id);
    pr_info("ts_use_cs_offset                                   = 0x%02x\n",ts_use_cs_offset);
    pr_info("ts_tsts_adjust                                     = 0x%02x\n",ts_tsts_adjust);
    pr_info("ts_osts_adjust                                     = 0x%04x\n",ts_osts_adjust);
}
void xport_proc_xlmac_core_timestamp_byte_adjust_dump(uint8_t port_id)
{
    uint8_t rx_timer_byte_adjust_en; uint16_t rx_timer_byte_adjust; uint8_t tx_timer_byte_adjust_en; uint16_t tx_timer_byte_adjust;
    ag_drv_xport_xlmac_core_timestamp_byte_adjust_get(port_id,&rx_timer_byte_adjust_en,&rx_timer_byte_adjust,&tx_timer_byte_adjust_en,&tx_timer_byte_adjust);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TIMESTAMP_BYTE_ADJUST:\n",port_id);
    pr_info("rx_timer_byte_adjust_en                            = 0x%02x\n",rx_timer_byte_adjust_en);
    pr_info("rx_timer_byte_adjust                               = 0x%02x\n",rx_timer_byte_adjust);
    pr_info("tx_timer_byte_adjust_en                            = 0x%04x\n",tx_timer_byte_adjust_en);
    pr_info("tx_timer_byte_adjust                               = 0x%04x\n",tx_timer_byte_adjust);
}
void xport_proc_xlmac_core_tx_crc_corrupt_ctrl_dump(uint8_t port_id)
{
    uint32_t prog_tx_crc; uint8_t tx_crc_corruption_mode; uint8_t tx_crc_corrupt_en; uint8_t tx_err_corrupts_crc;
    ag_drv_xport_xlmac_core_tx_crc_corrupt_ctrl_get(port_id,&prog_tx_crc,&tx_crc_corruption_mode,&tx_crc_corrupt_en,&tx_err_corrupts_crc);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_TX_CRC_CORRUPT_CTRL:\n",port_id);
    pr_info("prog_tx_crc                                        = 0x%08x\n",prog_tx_crc);
    pr_info("tx_crc_corruption_mode                             = 0x%02x\n",tx_crc_corruption_mode);
    pr_info("tx_crc_corrupt_en                                  = 0x%04x\n",tx_crc_corrupt_en);
    pr_info("tx_err_corrupts_crc                                = 0x%04x\n",tx_err_corrupts_crc);
}
void xport_proc_xlmac_core_e2e_ctrl_dump(uint8_t port_id)
{
    xport_xlmac_core_e2e_ctrl e2e_ctrl;
    ag_drv_xport_xlmac_core_e2e_ctrl_get(port_id,&e2e_ctrl);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_E2E_CTRL                 :\n",port_id);
    pr_info("e2efc_dual_modid_en                                = 0x%02x\n",e2e_ctrl.e2efc_dual_modid_en);
    pr_info("e2ecc_legacy_imp_en                                = 0x%02x\n",e2e_ctrl.e2ecc_legacy_imp_en);
    pr_info("e2ecc_dual_modid_en                                = 0x%02x\n",e2e_ctrl.e2ecc_dual_modid_en);
    pr_info("honor_pause_for_e2e                                = 0x%02x\n",e2e_ctrl.honor_pause_for_e2e);
    pr_info("e2e_enable                                         = 0x%02x\n",e2e_ctrl.e2e_enable);

}
void xport_proc_xlmac_core_e2ecc_module_hdr_0_dump(uint8_t port_id)
{
    uint64_t e2ecc_module_hdr_0;
    ag_drv_xport_xlmac_core_e2ecc_module_hdr_0_get(port_id,&e2ecc_module_hdr_0);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_E2ECC_MODULE_HDR_0       = <0x%016llu>\n",port_id,*(uint64_t*)&e2ecc_module_hdr_0);
}
void xport_proc_xlmac_core_e2ecc_module_hdr_1_dump(uint8_t port_id)
{
    uint64_t e2ecc_module_hdr_1;
    ag_drv_xport_xlmac_core_e2ecc_module_hdr_1_get(port_id,&e2ecc_module_hdr_1);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_E2ECC_MODULE_HDR_1       = <0x%016llu>\n",port_id,*(uint64_t*)&e2ecc_module_hdr_1);
}
void xport_proc_xlmac_core_e2ecc_data_hdr_0_dump(uint8_t port_id)
{
    uint64_t e2ecc_data_hdr_0;
    ag_drv_xport_xlmac_core_e2ecc_data_hdr_0_get(port_id,&e2ecc_data_hdr_0);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_E2ECC_DATA_HDR_0         = <0x%016llu>\n",port_id,*(uint64_t*)&e2ecc_data_hdr_0);
}
void xport_proc_xlmac_core_e2ecc_data_hdr_1_dump(uint8_t port_id)
{
    uint64_t e2ecc_data_hdr_1;
    ag_drv_xport_xlmac_core_e2ecc_data_hdr_1_get(port_id,&e2ecc_data_hdr_1);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_E2ECC_DATA_HDR_1         = <0x%016llu>\n",port_id,*(uint64_t*)&e2ecc_data_hdr_1);
}
void xport_proc_xlmac_core_e2efc_module_hdr_0_dump(uint8_t port_id)
{
    uint64_t e2efc_module_hdr_0;
    ag_drv_xport_xlmac_core_e2efc_module_hdr_0_get(port_id,&e2efc_module_hdr_0);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_e2efc_MODULE_HDR_0       = <0x%016llu>\n",port_id,*(uint64_t*)&e2efc_module_hdr_0);
}
void xport_proc_xlmac_core_e2efc_module_hdr_1_dump(uint8_t port_id)
{
    uint64_t e2efc_module_hdr_1;
    ag_drv_xport_xlmac_core_e2efc_module_hdr_1_get(port_id,&e2efc_module_hdr_1);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_e2efc_MODULE_HDR_1       = <0x%016llu>\n",port_id,*(uint64_t*)&e2efc_module_hdr_1);
}
void xport_proc_xlmac_core_e2efc_data_hdr_0_dump(uint8_t port_id)
{
    uint64_t e2efc_data_hdr_0;
    ag_drv_xport_xlmac_core_e2efc_data_hdr_0_get(port_id,&e2efc_data_hdr_0);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_e2efc_DATA_HDR_0         = <0x%016llu>\n",port_id,*(uint64_t*)&e2efc_data_hdr_0);
}
void xport_proc_xlmac_core_e2efc_data_hdr_1_dump(uint8_t port_id)
{
    uint64_t e2efc_data_hdr_1;
    ag_drv_xport_xlmac_core_e2efc_data_hdr_1_get(port_id,&e2efc_data_hdr_1);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_e2efc_DATA_HDR_1         = <0x%016llu>\n",port_id,*(uint64_t*)&e2efc_data_hdr_1);
}
void xport_proc_xlmac_core_txfifo_cell_cnt_dump(uint8_t port_id)
{
    uint8_t cell_cnt;
    ag_drv_xport_xlmac_core_txfifo_cell_cnt_get(port_id,&cell_cnt);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_TXFIFO_CELL_CNT          = <0x%02u>\n",port_id,cell_cnt);
}
void xport_proc_xlmac_core_txfifo_cell_req_cnt_dump(uint8_t port_id)
{
    uint8_t cell_req_cnt;
    ag_drv_xport_xlmac_core_txfifo_cell_req_cnt_get(port_id,&cell_req_cnt);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_1_PORT%d_TXFIFO_CELL_REQ_CNT      = <0x%02u>\n",port_id,cell_req_cnt);
}
void xport_proc_xlmac_core_mem_ctrl_dump(uint8_t port_id)
{
    uint16_t tx_cdc_mem_ctrl_tm; uint16_t rx_cdc_mem_ctrl_tm;
    ag_drv_xport_xlmac_core_mem_ctrl_get(port_id,&tx_cdc_mem_ctrl_tm,&rx_cdc_mem_ctrl_tm);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_MEM_CTRL:\n",port_id);
    pr_info("tx_cdc_mem_ctrl_tm                                 = 0x%04x\n",tx_cdc_mem_ctrl_tm);
    pr_info("rx_cdc_mem_ctrl_tm                                 = 0x%04x\n",rx_cdc_mem_ctrl_tm);
}
void xport_proc_xlmac_core_ecc_ctrl_dump(uint8_t port_id)
{
    uint8_t tx_cdc_ecc_ctrl_en; uint8_t rx_cdc_ecc_ctrl_en;
    ag_drv_xport_xlmac_core_ecc_ctrl_get(port_id,&tx_cdc_ecc_ctrl_en,&rx_cdc_ecc_ctrl_en);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_ECC_CTRL:\n",port_id);
    pr_info("tx_cdc_ecc_ctrl_en                                 = 0x%04x\n",tx_cdc_ecc_ctrl_en);
    pr_info("rx_cdc_ecc_ctrl_en                                 = 0x%04x\n",rx_cdc_ecc_ctrl_en);
}
void xport_proc_xlmac_core_ecc_force_double_bit_err_dump(uint8_t port_id)
{
    uint8_t tx_cdc_force_double_bit_err; uint8_t rx_cdc_force_double_bit_err;
    ag_drv_xport_xlmac_core_ecc_force_double_bit_err_get(port_id,&tx_cdc_force_double_bit_err,&rx_cdc_force_double_bit_err);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_ECC_FORCE_DOUBLE_BIT_ERR:\n",port_id);
    pr_info("tx_cdc_force_double_bit_err                        = 0x%02x\n",tx_cdc_force_double_bit_err);
    pr_info("rx_cdc_force_double_bit_err                        = 0x%02x\n",rx_cdc_force_double_bit_err);
}
void xport_proc_xlmac_core_ecc_force_single_bit_err_dump(uint8_t port_id)
{
    uint8_t tx_cdc_force_single_bit_err; uint8_t rx_cdc_force_single_bit_err;
    ag_drv_xport_xlmac_core_ecc_force_single_bit_err_get(port_id,&tx_cdc_force_single_bit_err,&rx_cdc_force_single_bit_err);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_ECC_FORCE_single_BIT_ERR:\n",port_id);
    pr_info("tx_cdc_force_single_bit_err                        = 0x%02x\n",tx_cdc_force_single_bit_err);
    pr_info("rx_cdc_force_single_bit_err                        = 0x%02x\n",rx_cdc_force_single_bit_err);
}
void xport_proc_xlmac_core_rx_cdc_ecc_status_dump(uint8_t port_id)
{
    uint8_t rx_cdc_double_bit_err; uint8_t rx_cdc_single_bit_err;
    ag_drv_xport_xlmac_core_rx_cdc_ecc_status_get(port_id,&rx_cdc_double_bit_err,&rx_cdc_single_bit_err);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_RX_CDC_ECC_STATUS:\n",port_id);
    pr_info("rx_cdc_double_bit_err                              = 0x%02x\n",rx_cdc_double_bit_err);
    pr_info("rx_cdc_single_bit_err                              = 0x%02x\n",rx_cdc_single_bit_err);
}
void xport_proc_xlmac_core_tx_cdc_ecc_status_dump(uint8_t port_id)
{
    uint8_t tx_cdc_double_bit_err; uint8_t tx_cdc_single_bit_err;
    ag_drv_xport_xlmac_core_tx_cdc_ecc_status_get(port_id,&tx_cdc_double_bit_err,&tx_cdc_single_bit_err);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_tx_CDC_ECC_STATUS:\n",port_id);
    pr_info("tx_cdc_double_bit_err                              = 0x%02x\n",tx_cdc_double_bit_err);
    pr_info("tx_cdc_single_bit_err                              = 0x%02x\n",tx_cdc_single_bit_err);
}
void xport_proc_xlmac_core_clear_ecc_status_dump(uint8_t port_id)
{
    uint8_t clear_tx_cdc_double_bit_err; uint8_t clear_tx_cdc_single_bit_err; uint8_t clear_rx_cdc_double_bit_err; uint8_t clear_rx_cdc_single_bit_err;
    ag_drv_xport_xlmac_core_clear_ecc_status_get(port_id,&clear_tx_cdc_double_bit_err,&clear_tx_cdc_single_bit_err,&clear_rx_cdc_double_bit_err,&clear_rx_cdc_single_bit_err);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_CLEAR_ECC_STATUS:\n",port_id);
    pr_info("clear_tx_cdc_double_bit_err                        = 0x%02x\n",clear_tx_cdc_double_bit_err);
    pr_info("clear_tx_cdc_single_bit_err                        = 0x%02x\n",clear_tx_cdc_single_bit_err);
    pr_info("clear_rx_cdc_double_bit_err                        = 0x%02x\n",clear_rx_cdc_double_bit_err);
    pr_info("clear_rx_cdc_single_bit_err                        = 0x%02x\n",clear_rx_cdc_single_bit_err);
}
void xport_proc_xlmac_core_intr_status_dump(uint8_t port_id)
{
    xport_xlmac_core_intr_status intr_status;
    ag_drv_xport_xlmac_core_intr_status_get(port_id,&intr_status);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_INTR_STATUS              :\n",port_id);
    pr_info("sum_ts_entry_valid                                 = 0x%02x\n",intr_status.sum_ts_entry_valid);
    pr_info("sum_link_interruption_status                       = 0x%02x\n",intr_status.sum_link_interruption_status);
    pr_info("sum_remote_fault_status                            = 0x%02x\n",intr_status.sum_remote_fault_status);
    pr_info("sum_local_fault_status                             = 0x%02x\n",intr_status.sum_local_fault_status);
    pr_info("sum_rx_cdc_double_bit_err                          = 0x%02x\n",intr_status.sum_rx_cdc_double_bit_err);
    pr_info("sum_rx_cdc_single_bit_err                          = 0x%02x\n",intr_status.sum_rx_cdc_single_bit_err);
    pr_info("sum_tx_cdc_double_bit_err                          = 0x%02x\n",intr_status.sum_tx_cdc_double_bit_err);
    pr_info("sum_tx_cdc_single_bit_err                          = 0x%02x\n",intr_status.sum_tx_cdc_single_bit_err);
    pr_info("sum_rx_msg_overflow                                = 0x%02x\n",intr_status.sum_rx_msg_overflow);
    pr_info("sum_rx_pkt_overflow                                = 0x%02x\n",intr_status.sum_rx_pkt_overflow);
    pr_info("sum_tx_ts_fifo_overflow                            = 0x%02x\n",intr_status.sum_tx_ts_fifo_overflow);
    pr_info("sum_tx_llfc_msg_overflow                           = 0x%02x\n",intr_status.sum_tx_llfc_msg_overflow);
    pr_info("sum_tx_pkt_overflow                                = 0x%02x\n",intr_status.sum_tx_pkt_overflow);
    pr_info("sum_tx_pkt_underflow                               = 0x%02x\n",intr_status.sum_tx_pkt_underflow);

}
void xport_proc_xlmac_core_intr_enable_dump(uint8_t port_id)
{
    xport_xlmac_core_intr_enable intr_enable;
    ag_drv_xport_xlmac_core_intr_enable_get(port_id,&intr_enable);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_INTR_ENABLE              :\n",port_id);
    pr_info("en_ts_entry_valid                                  = 0x%02x\n",intr_enable.en_ts_entry_valid);
    pr_info("en_link_interruption_status                        = 0x%02x\n",intr_enable.en_link_interruption_status);
    pr_info("en_remote_fault_status                             = 0x%02x\n",intr_enable.en_remote_fault_status);
    pr_info("en_local_fault_status                              = 0x%02x\n",intr_enable.en_local_fault_status);
    pr_info("en_rx_cdc_double_bit_err                           = 0x%02x\n",intr_enable.en_rx_cdc_double_bit_err);
    pr_info("en_rx_cdc_single_bit_err                           = 0x%02x\n",intr_enable.en_rx_cdc_single_bit_err);
    pr_info("en_tx_cdc_double_bit_err                           = 0x%02x\n",intr_enable.en_tx_cdc_double_bit_err);
    pr_info("en_tx_cdc_single_bit_err                           = 0x%02x\n",intr_enable.en_tx_cdc_single_bit_err);
    pr_info("en_rx_msg_overflow                                 = 0x%02x\n",intr_enable.en_rx_msg_overflow);
    pr_info("en_rx_pkt_overflow                                 = 0x%02x\n",intr_enable.en_rx_pkt_overflow);
    pr_info("en_tx_ts_fifo_overflow                             = 0x%02x\n",intr_enable.en_tx_ts_fifo_overflow);
    pr_info("en_tx_llfc_msg_overflow                            = 0x%02x\n",intr_enable.en_tx_llfc_msg_overflow);
    pr_info("en_tx_pkt_overflow                                 = 0x%02x\n",intr_enable.en_tx_pkt_overflow);
    pr_info("en_tx_pkt_underflow                                = 0x%02x\n",intr_enable.en_tx_pkt_underflow);
}
void xport_proc_xlmac_core_version_id_dump(uint8_t port_id)
{
    uint16_t xlmac_version;
    ag_drv_xport_xlmac_core_version_id_get(port_id,&xlmac_version);
    pr_info("\n");
    pr_info("XPORT_XLMAC_CORE_0_PORT%d_VERSION_ID               = <0x%04u>\n",port_id,xlmac_version);
}

typedef struct reg_name_func0
{
    char name[64];
    void (*func0)(void);
} reg_name_func0;
typedef struct reg_name_func1
{
    char name[64];
    void (*func1)(uint8_t port_id);
} reg_name_func1;

reg_name_func1 xlma_core_reg_name_func_array[] = {
    { "CTRL", xport_proc_xlmac_core_ctrl_dump },
    { "MODE", xport_proc_xlmac_core_mode_dump},
    { "SPARE0", xport_proc_xlmac_core_spare0_dump},
    { "SPARE1", xport_proc_xlmac_core_spare1_dump},
    { "TX_CTRL", xport_proc_xlmac_core_tx_ctrl_dump},
    { "TX_MAC_SA", xport_proc_xlmac_core_tx_mac_sa_dump},
    { "RX_CTRL", xport_proc_xlmac_core_rx_ctrl_dump},
    { "RX_MAC_SA", xport_proc_xlmac_core_rx_mac_sa_dump},
    { "RX_MAX_SIZE", xport_proc_xlmac_core_rx_max_size_dump},
    { "RX_VLAN_TAG", xport_proc_xlmac_core_rx_vlan_tag_dump},
    { "RX_LSS_CTRL", xport_proc_xlmac_core_rx_lss_ctrl_dump},
    { "RX_LSS_STATUS", xport_proc_xlmac_core_rx_lss_status_dump},
    { "CLEAR_RX_LSS_STATUS", xport_proc_xlmac_core_clear_rx_lss_status_dump},
    { "PAUSE_CTRL", xport_proc_xlmac_core_pause_ctrl_dump},
    { "PFC_CTRL", xport_proc_xlmac_core_pfc_ctrl_dump},
    { "PFC_TYPE", xport_proc_xlmac_core_pfc_type_dump},
    { "PFC_OPCODE", xport_proc_xlmac_core_pfc_opcode_dump},
    { "PFC_DA",xport_proc_xlmac_core_pfc_da_dump},
    { "LLFC_CTRL",xport_proc_xlmac_core_llfc_ctrl_dump},
    { "TX_LLFC_MSG_FIELDS",xport_proc_xlmac_core_tx_llfc_msg_fields_dump},
    { "RX_LLFC_MSG_FIELDS",xport_proc_xlmac_core_rx_llfc_msg_fields_dump},
    { "TX_TIMESTAMP_FIFO_DATA",xport_xlmac_core_tx_timestamp_fifo_data_dump},
    { "TX_TIMESTAMP_FIFO_STATUS",xport_proc_xlmac_core_tx_timestamp_fifo_status_dump},
    { "FIFO_STATUS",xport_proc_xlmac_core_fifo_status_dump},
    { "CLEAR_FIFO_STATUS",xport_proc_xlmac_core_clear_fifo_status_dump},
    { "LAG_FAILOVER_STATUS",xport_proc_xlmac_core_lag_failover_status_dump},
    { "EEE_CTRL",xport_proc_xlmac_core_eee_ctrl_dump},
    { "EEE_TIMERS",xport_proc_xlmac_core_eee_timers_dump},
    { "EEE_1_SEC_LINK_STATUS_TIMER",xport_proc_xlmac_core_eee_1_sec_link_status_timer_dump},
    { "HIGIG_HDR_0",xport_proc_xlmac_core_higig_hdr_0_dump},
    { "HIGIG_HDR_1",xport_proc_xlmac_core_higig_hdr_1_dump},
    { "GMII_EEE_CTRL",xport_proc_xlmac_core_gmii_eee_ctrl_dump},
    { "TIMESTAMP_ADJUST",xport_proc_xlmac_core_timestamp_adjust_dump},
    { "TIMESTAMP_BYTE_ADJUST",xport_proc_xlmac_core_timestamp_byte_adjust_dump},
    { "TX_CRC_CORRUPT_CTRL",xport_proc_xlmac_core_tx_crc_corrupt_ctrl_dump},
    { "E2E_CTRL",xport_proc_xlmac_core_e2e_ctrl_dump},
    { "E2ECC_MODULE_HDR_0",xport_proc_xlmac_core_e2ecc_module_hdr_0_dump},
    { "E2ECC_MODULE_HDR_1",xport_proc_xlmac_core_e2ecc_module_hdr_1_dump},
    { "E2ECC_DATA_HDR_0",xport_proc_xlmac_core_e2ecc_data_hdr_0_dump},
    { "E2ECC_DATA_HDR_1",xport_proc_xlmac_core_e2ecc_data_hdr_1_dump},
    { "E2EFC_MODULE_HDR_0",xport_proc_xlmac_core_e2efc_module_hdr_0_dump},
    { "E2EFC_MODULE_HDR_1",xport_proc_xlmac_core_e2efc_module_hdr_1_dump},
    { "E2EFC_DATA_HDR_0",xport_proc_xlmac_core_e2efc_data_hdr_0_dump},
    { "E2EFC_DATA_HDR_1",xport_proc_xlmac_core_e2efc_data_hdr_1_dump},
    { "TXFIFO_CELL_CNT",xport_proc_xlmac_core_txfifo_cell_cnt_dump},
    { "TXFIFO_CELL_REQ_CNT",xport_proc_xlmac_core_txfifo_cell_req_cnt_dump},
    { "MEM_CTRL",xport_proc_xlmac_core_mem_ctrl_dump},
    { "ECC_CTRL",xport_proc_xlmac_core_ecc_ctrl_dump},
    { "ECC_FORCE_DOUBLE_BIT_ERR",xport_proc_xlmac_core_ecc_force_double_bit_err_dump},
    { "ECC_FORCE_SINGLE_BIT_ERR",xport_proc_xlmac_core_ecc_force_single_bit_err_dump},
    { "RX_CDC_ECC_STATUS",xport_proc_xlmac_core_rx_cdc_ecc_status_dump},
    { "TX_CDC_ECC_STATUS",xport_proc_xlmac_core_tx_cdc_ecc_status_dump},
    { "CLEAR_ECC_STATUS",xport_proc_xlmac_core_clear_ecc_status_dump},
    { "INTR_STATUS",xport_proc_xlmac_core_intr_status_dump},
    { "INTR_ENABLE",xport_proc_xlmac_core_intr_enable_dump},
    { "VERSION_ID",xport_proc_xlmac_core_version_id_dump},
    { "", NULL},
};

static void xport_proc_cmd_xlmac_core_reg_dump(uint8_t port_id, char *reg_str)
{
    reg_name_func1 *p_array = &xlma_core_reg_name_func_array[0];
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func1(port_id);
        p_array++;
    }
}

void xport_reg_xport_revision_dump(void)
{
    uint32_t xport_rev;
    ag_drv_xport_reg_xport_revision_get(&xport_rev);
    pr_info("\n");
    pr_info("XPORT_REG_XPORT_REVISION                           = <0x%08u>\n",xport_rev);
}
void xport_reg_led_0_cntrl_dump(void)
{
    xport_reg_led_0_cntrl led_0_cntrl;
    ag_drv_xport_reg_led_0_cntrl_get(&led_0_cntrl);
    pr_info("\n");
    pr_info("XPORT_REG_LED_0_CTRL                               = \n");
    pr_info("lnk_ovrd_en                                        = 0x%02x\n",led_0_cntrl.lnk_ovrd_en);
    pr_info("spd_ovrd_en                                        = 0x%02x\n",led_0_cntrl.spd_ovrd_en);
    pr_info("lnk_status_ovrd                                    = 0x%02x\n",led_0_cntrl.lnk_status_ovrd);
    pr_info("led_spd_ovrd                                       = 0x%02x\n",led_0_cntrl.led_spd_ovrd);
    pr_info("act_led_pol_sel                                    = 0x%02x\n",led_0_cntrl.act_led_pol_sel);
    pr_info("spdlnk_led2_act_pol_sel                            = 0x%02x\n",led_0_cntrl.spdlnk_led2_act_pol_sel);
    pr_info("spdlnk_led1_act_pol_sel                            = 0x%02x\n",led_0_cntrl.spdlnk_led1_act_pol_sel);
    pr_info("spdlnk_led0_act_pol_sel                            = 0x%02x\n",led_0_cntrl.spdlnk_led0_act_pol_sel);
    pr_info("act_led_act_sel                                    = 0x%02x\n",led_0_cntrl.act_led_act_sel);
    pr_info("spdlnk_led2_act_sel                                = 0x%02x\n",led_0_cntrl.spdlnk_led2_act_sel);
    pr_info("spdlnk_led1_act_sel                                = 0x%02x\n",led_0_cntrl.spdlnk_led1_act_sel);
    pr_info("spdlnk_led0_act_sel                                = 0x%02x\n",led_0_cntrl.spdlnk_led0_act_sel);
    pr_info("tx_act_en                                          = 0x%02x\n",led_0_cntrl.tx_act_en);
    pr_info("rx_act_en                                          = 0x%02x\n",led_0_cntrl.rx_act_en);

}
void xport_reg_led_0_link_and_speed_encoding_sel_dump(void)
{
    xport_reg_led_0_link_and_speed_encoding_sel led_0_link_and_speed_encoding_sel;
    ag_drv_xport_reg_led_0_link_and_speed_encoding_sel_get(&led_0_link_and_speed_encoding_sel);
    pr_info("\n");
    pr_info("XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING_SEL        = \n");
    pr_info("rsvd_sel_spd_encode_2                              = 0x%02x\n",led_0_link_and_speed_encoding_sel.rsvd_sel_spd_encode_2);
    pr_info("rsvd_sel_spd_encode_1                              = 0x%02x\n",led_0_link_and_speed_encoding_sel.rsvd_sel_spd_encode_1);
    pr_info("sel_10g_encode                                     = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_10g_encode);
    pr_info("sel_2500m_encode                                   = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_2500m_encode);
    pr_info("sel_1000m_encode                                   = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_1000m_encode);
    pr_info("sel_100m_encode                                    = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_100m_encode);
    pr_info("sel_10m_encode                                     = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_10m_encode);
    pr_info("sel_no_link_encode                                 = 0x%02x\n",led_0_link_and_speed_encoding_sel.sel_no_link_encode);

}
void xport_reg_led_0_link_and_speed_encoding_dump(void)
{
    xport_reg_led_0_link_and_speed_encoding led_0_link_and_speed_encoding;
    ag_drv_xport_reg_led_0_link_and_speed_encoding_get(&led_0_link_and_speed_encoding);
    pr_info("\n");
    pr_info("XPORT_REG_LED_0_LINK_AND_SPEED_ENCODING            = \n");
    pr_info("rsvd_spd_encode_2                                  = 0x%02x\n",led_0_link_and_speed_encoding.rsvd_spd_encode_2);
    pr_info("rsvd_spd_encode_1                                  = 0x%02x\n",led_0_link_and_speed_encoding.rsvd_spd_encode_1);
    pr_info("m10g_encode                                        = 0x%02x\n",led_0_link_and_speed_encoding.m10g_encode);
    pr_info("m2500_encode                                       = 0x%02x\n",led_0_link_and_speed_encoding.m2500_encode);
    pr_info("m1000_encode                                       = 0x%02x\n",led_0_link_and_speed_encoding.m1000_encode);
    pr_info("m100_encode                                        = 0x%02x\n",led_0_link_and_speed_encoding.m100_encode);
    pr_info("m10_encode                                         = 0x%02x\n",led_0_link_and_speed_encoding.m10_encode);
    pr_info("no_link_encode                                     = 0x%02x\n",led_0_link_and_speed_encoding.no_link_encode);
}
void xport_reg_led_1_cntrl_dump(void)
{
    xport_reg_led_1_cntrl led_1_cntrl;
    ag_drv_xport_reg_led_1_cntrl_get(&led_1_cntrl);
    pr_info("\n");
    pr_info("XPORT_REG_LED_1_CTRL                               = \n");
    pr_info("lnk_ovrd_en                                        = 0x%02x\n",led_1_cntrl.lnk_ovrd_en);
    pr_info("spd_ovrd_en                                        = 0x%02x\n",led_1_cntrl.spd_ovrd_en);
    pr_info("lnk_status_ovrd                                    = 0x%02x\n",led_1_cntrl.lnk_status_ovrd);
    pr_info("led_spd_ovrd                                       = 0x%02x\n",led_1_cntrl.led_spd_ovrd);
    pr_info("act_led_pol_sel                                    = 0x%02x\n",led_1_cntrl.act_led_pol_sel);
    pr_info("spdlnk_led2_act_pol_sel                            = 0x%02x\n",led_1_cntrl.spdlnk_led2_act_pol_sel);
    pr_info("spdlnk_led1_act_pol_sel                            = 0x%02x\n",led_1_cntrl.spdlnk_led1_act_pol_sel);
    pr_info("spdlnk_led0_act_pol_sel                            = 0x%02x\n",led_1_cntrl.spdlnk_led0_act_pol_sel);
    pr_info("act_led_act_sel                                    = 0x%02x\n",led_1_cntrl.act_led_act_sel);
    pr_info("spdlnk_led2_act_sel                                = 0x%02x\n",led_1_cntrl.spdlnk_led2_act_sel);
    pr_info("spdlnk_led1_act_sel                                = 0x%02x\n",led_1_cntrl.spdlnk_led1_act_sel);
    pr_info("spdlnk_led0_act_sel                                = 0x%02x\n",led_1_cntrl.spdlnk_led0_act_sel);
    pr_info("tx_act_en                                          = 0x%02x\n",led_1_cntrl.tx_act_en);
    pr_info("rx_act_en                                          = 0x%02x\n",led_1_cntrl.rx_act_en);

}
void xport_reg_led_1_link_and_speed_encoding_sel_dump(void)
{
    xport_reg_led_1_link_and_speed_encoding_sel led_1_link_and_speed_encoding_sel;
    ag_drv_xport_reg_led_1_link_and_speed_encoding_sel_get(&led_1_link_and_speed_encoding_sel);
    pr_info("\n");
    pr_info("XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING_SEL        = \n");
    pr_info("rsvd_sel_spd_encode_2                              = 0x%02x\n",led_1_link_and_speed_encoding_sel.rsvd_sel_spd_encode_2);
    pr_info("rsvd_sel_spd_encode_1                              = 0x%02x\n",led_1_link_and_speed_encoding_sel.rsvd_sel_spd_encode_1);
    pr_info("sel_10g_encode                                     = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_10g_encode);
    pr_info("sel_2500m_encode                                   = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_2500m_encode);
    pr_info("sel_1000m_encode                                   = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_1000m_encode);
    pr_info("sel_100m_encode                                    = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_100m_encode);
    pr_info("sel_10m_encode                                     = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_10m_encode);
    pr_info("sel_no_link_encode                                 = 0x%02x\n",led_1_link_and_speed_encoding_sel.sel_no_link_encode);

}
void xport_reg_led_1_link_and_speed_encoding_dump(void)
{
    xport_reg_led_1_link_and_speed_encoding led_1_link_and_speed_encoding;
    ag_drv_xport_reg_led_1_link_and_speed_encoding_get(&led_1_link_and_speed_encoding);
    pr_info("\n");
    pr_info("XPORT_REG_LED_1_LINK_AND_SPEED_ENCODING            = \n");
    pr_info("rsvd_spd_encode_2                                  = 0x%02x\n",led_1_link_and_speed_encoding.rsvd_spd_encode_2);
    pr_info("rsvd_spd_encode_1                                  = 0x%02x\n",led_1_link_and_speed_encoding.rsvd_spd_encode_1);
    pr_info("m10g_encode                                        = 0x%02x\n",led_1_link_and_speed_encoding.m10g_encode);
    pr_info("m2500_encode                                       = 0x%02x\n",led_1_link_and_speed_encoding.m2500_encode);
    pr_info("m1000_encode                                       = 0x%02x\n",led_1_link_and_speed_encoding.m1000_encode);
    pr_info("m100_encode                                        = 0x%02x\n",led_1_link_and_speed_encoding.m100_encode);
    pr_info("m10_encode                                         = 0x%02x\n",led_1_link_and_speed_encoding.m10_encode);
    pr_info("no_link_encode                                     = 0x%02x\n",led_1_link_and_speed_encoding.no_link_encode);
}
void xport_reg_led_blink_rate_cntrl_dump(void)
{
    uint16_t led_on_time; uint16_t led_off_time;
    ag_drv_xport_reg_led_blink_rate_cntrl_get(&led_on_time,&led_off_time);
    pr_info("\n");
    pr_info("XPORT_REG_LED_BLINK_RATE_CNTRL                     = \n");
    pr_info("led_on_time                                        = 0x%02x\n",led_on_time);
    pr_info("led_off_time                                       = 0x%02x\n",led_off_time);
}
void xport_reg_led_serial_ctrl_dump(void)
{
    xport_reg_led_serial_ctrl led_serial_ctrl;
    ag_drv_xport_reg_led_serial_ctrl_get(&led_serial_ctrl);
    pr_info("\n");
    pr_info("XPORT_REG_LED_SERIAL_CTRL                          = \n");
    pr_info("smode                                              = 0x%02x\n",led_serial_ctrl.smode);
    pr_info("sled_clk_frequency                                 = 0x%02x\n",led_serial_ctrl.sled_clk_frequency);
    pr_info("sled_clk_pol                                       = 0x%02x\n",led_serial_ctrl.sled_clk_pol);
    pr_info("refresh_period                                     = 0x%02x\n",led_serial_ctrl.refresh_period);
    pr_info("port_en                                            = 0x%02x\n",led_serial_ctrl.port_en);

}
void xport_reg_refresh_period_cntrl_dump(void)
{
    uint32_t refresh_period_cnt;
    ag_drv_xport_reg_refresh_period_cntrl_get(&refresh_period_cnt);
    pr_info("\n");
    pr_info("XPORT_REG_REFRESH_PERIOD_CNTRL                     = \n");
    pr_info("refresh_period_cnt                                 = 0x%08x\n",refresh_period_cnt);
}
void xport_reg_aggregate_led_cntrl_dump(void)
{
    uint8_t lnk_pol_sel; uint8_t act_pol_sel; uint8_t act_sel; uint16_t port_en;
    ag_drv_xport_reg_aggregate_led_cntrl_get(&lnk_pol_sel,&act_pol_sel,&act_sel,&port_en);
    pr_info("\n");
    pr_info("XPORT_REG_aggregate_led_cntrl                      = \n");
    pr_info("lnk_pol_sel                                        = 0x%04x\n",lnk_pol_sel);
    pr_info("act_pol_sel                                        = 0x%04x\n",act_pol_sel);
    pr_info("act_sel                                            = 0x%04x\n",act_sel);
    pr_info("port_en                                            = 0x%04x\n",port_en);
}
void xport_reg_aggregate_led_blink_rate_cntrl_dump(void)
{
    uint16_t led_on_time; uint16_t led_off_time;
    ag_drv_xport_reg_aggregate_led_blink_rate_cntrl_get(&led_on_time,&led_off_time);
    pr_info("\n");
    pr_info("XPORT_REG_aggregate_led_blink_rate_cntrl           = \n");
    pr_info("led_on_time                                        = 0x%04x\n",led_on_time);
    pr_info("led_off_time                                       = 0x%04x\n",led_off_time);
}
void xport_reg_spare_cntrl_dump(void)
{
    uint32_t spare_reg;
    ag_drv_xport_reg_spare_cntrl_get(&spare_reg);
    pr_info("\n");
    pr_info("XPORT_REG_spare_cntrl                              = \n");
    pr_info("spare_reg                                          = 0x%04x\n",spare_reg);

}
void xport_reg_xport_cntrl_1_dump(void)
{
    uint8_t msbus_clk_sel; uint8_t wan_led0_sel; uint8_t timeout_rst_disable; uint8_t p0_mode;
    ag_drv_xport_reg_xport_cntrl_1_get(&msbus_clk_sel, &wan_led0_sel, &timeout_rst_disable, &p0_mode);
    pr_info("\n");
    pr_info("XPORT_REG_XPORT_CNTRL_1                            = \n");
    pr_info("msbus_clk_sel                                      = 0x%04x\n",msbus_clk_sel);
    pr_info("wan_led0_sel                                       = 0x%04x\n",wan_led0_sel);
    pr_info("timeout_rst_disable                                = 0x%04x\n",timeout_rst_disable);
    pr_info("p0_mode                                            = 0x%04x\n",p0_mode);
}
void xport_reg_crossbar_status_dump(void)
{
    xport_reg_crossbar_status crossbar_status;
    ag_drv_xport_reg_crossbar_status_get(&crossbar_status);
    pr_info("\n");
    pr_info("XPORT_REG_CROSSBAR_STATUS                          = \n");
    pr_info("full_duplex                                        = 0x%02x\n",crossbar_status.full_duplex);
    pr_info("pause_tx                                           = 0x%02x\n",crossbar_status.pause_tx);
    pr_info("pause_rx                                           = 0x%02x\n",crossbar_status.pause_rx);
    pr_info("speed_2500                                         = 0x%02x\n",crossbar_status.speed_2500);
    pr_info("speed_1000                                         = 0x%02x\n",crossbar_status.speed_1000);
    pr_info("speed_100                                          = 0x%02x\n",crossbar_status.speed_100);
    pr_info("speed_10                                           = 0x%02x\n",crossbar_status.speed_10);
    pr_info("link_status                                        = 0x%02x\n",crossbar_status.link_status);

}
void xport_reg_pon_ae_serdes_status_dump(void)
{
    xport_reg_pon_ae_serdes_status pon_ae_serdes_status;
    ag_drv_xport_reg_pon_ae_serdes_status_get(&pon_ae_serdes_status);
    pr_info("\n");
    pr_info("XPORT_REG_PON_AE_SERDES_STATUS                     = \n");
    pr_info("mod_def0                                           = 0x%02x\n",pon_ae_serdes_status.mod_def0);
    pr_info("ext_sig_det                                        = 0x%02x\n",pon_ae_serdes_status.ext_sig_det);
    pr_info("pll1_lock                                          = 0x%02x\n",pon_ae_serdes_status.pll1_lock);
    pr_info("pll0_lock                                          = 0x%02x\n",pon_ae_serdes_status.pll0_lock);
    pr_info("link_status                                        = 0x%02x\n",pon_ae_serdes_status.link_status);
    pr_info("cdr_lock                                           = 0x%02x\n",pon_ae_serdes_status.cdr_lock);
    pr_info("rx_sigdet                                          = 0x%02x\n",pon_ae_serdes_status.rx_sigdet);

}
reg_name_func0 xport_reg_name_func_array[] = {
    {"XPORT_REVISION", xport_reg_xport_revision_dump },
    {"LED_0_CNTRL", xport_reg_led_0_cntrl_dump},
    {"LED_0_LINK_AND_SPEED_ENCODING_SEL", xport_reg_led_0_link_and_speed_encoding_sel_dump},
    {"LED_0_LINK_AND_SPEED_ENCODING", xport_reg_led_0_link_and_speed_encoding_dump},    
    {"LED_1_CNTRL", xport_reg_led_1_cntrl_dump},
    {"LED_1_LINK_AND_SPEED_ENCODING_SEL", xport_reg_led_1_link_and_speed_encoding_sel_dump},
    {"LED_1_LINK_AND_SPEED_ENCODING", xport_reg_led_1_link_and_speed_encoding_dump},    
    {"LED_BLINK_RATE_CNTRL", xport_reg_led_blink_rate_cntrl_dump},
    {"LED_SERIAL_CNTRL", xport_reg_led_serial_ctrl_dump},
    {"REFRESH_PERIOD_CNTRL", xport_reg_refresh_period_cntrl_dump},
    {"AGGREGATE_LED_CNTRL", xport_reg_aggregate_led_cntrl_dump},
    {"AGGREGATE_LED_BLINK_RATE_CNTRL", xport_reg_aggregate_led_blink_rate_cntrl_dump},
    {"SPARE_CNTRL", xport_reg_spare_cntrl_dump},
    {"XPORT_CNTRL_1", xport_reg_xport_cntrl_1_dump},
    {"CROSSBAR_STATUS", xport_reg_crossbar_status_dump},
    {"PON_AE_SERDES_STATUS", xport_reg_pon_ae_serdes_status_dump},
    { "", NULL},
};


static void xport_proc_cmd_xport_reg_reg_dump(char *reg_str)
{
    reg_name_func0 *p_array = &xport_reg_name_func_array[0];
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func0();
        p_array++;
    }
}

void xport_mab_ctrl_dump(void)
{
    xport_mab_ctrl ctrl;
    ag_drv_xport_mab_ctrl_get(&ctrl);
    pr_info("\n");
    pr_info("XPORT_MAB_CNTRL                                    = \n");
    pr_info("link_down_rst_en                                   = 0x%02x\n",ctrl.link_down_rst_en);
    pr_info("xgmii_tx_rst                                       = 0x%02x\n",ctrl.xgmii_tx_rst);
    pr_info("gmii_tx_rst                                        = 0x%02x\n",ctrl.gmii_tx_rst);
    pr_info("xgmii_rx_rst                                       = 0x%02x\n",ctrl.xgmii_rx_rst);
    pr_info("gmii_rx_rst                                        = 0x%02x\n",ctrl.gmii_rx_rst);
}
void xport_mab_tx_wrr_ctrl_dump(void)
{
    xport_mab_tx_wrr_ctrl tx_wrr_ctrl;
    ag_drv_xport_mab_tx_wrr_ctrl_get(&tx_wrr_ctrl);
    pr_info("\n");
    pr_info("XPORT_MAB_TX_WRR_CTRL                              = \n");
    pr_info("arb_mode                                           = 0x%02x\n",tx_wrr_ctrl.arb_mode);
    pr_info("p3_weight                                          = 0x%02x\n",tx_wrr_ctrl.p3_weight);
    pr_info("p2_weight                                          = 0x%02x\n",tx_wrr_ctrl.p2_weight);
    pr_info("p1_weight                                          = 0x%02x\n",tx_wrr_ctrl.p1_weight);
    pr_info("p0_weight                                          = 0x%02x\n",tx_wrr_ctrl.p0_weight);

}
void xport_mab_tx_threshold_dump(void)
{
    xport_mab_tx_threshold tx_threshold;
    ag_drv_xport_mab_tx_threshold_get(&tx_threshold);
    pr_info("\n");
    pr_info("XPORT_MAB_TX_THRESHOLD                             = \n");
    pr_info("xgmii0_tx_threshold                                = 0x%02x\n",tx_threshold.xgmii0_tx_threshold);
    pr_info("gmii3_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii3_tx_threshold);
    pr_info("gmii2_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii2_tx_threshold);
    pr_info("gmii1_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii1_tx_threshold);
    pr_info("gmii0_tx_threshold                                 = 0x%02x\n",tx_threshold.gmii0_tx_threshold);

}
void xport_mab_link_down_tx_data_dump(void)
{
    uint8_t txctl; uint8_t txd;
    ag_drv_xport_mab_link_down_tx_data_get(&txctl,&txd);
    pr_info("\n");
    pr_info("XPORT_MAB_LINK_DOWN_TX_DATA                        = \n");
    pr_info("txctl                                              = 0x%02x\n",txctl);
    pr_info("txd                                                = 0x%02x\n",txd);
}
void xport_mab_status_dump(void)
{
    xport_mab_status status;
    ag_drv_xport_mab_status_get(&status);
    pr_info("\n");
    pr_info("XPORT_MAB_STATUS                                   = \n");
    pr_info("xgmii_rx_afifo_overrun                             = 0x%02x\n",status.xgmii_rx_afifo_overrun);
    pr_info("gmii_rx_afifo_overrun_vect                         = 0x%02x\n",status.gmii_rx_afifo_overrun_vect);
    pr_info("xgmii_tx_frm_underrun                              = 0x%02x\n",status.xgmii_tx_frm_underrun);
    pr_info("xgmii_outstanding_credits_cnt_underrun             = 0x%02x\n",status.xgmii_outstanding_credits_cnt_underrun);
    pr_info("gmii_outstanding_credits_cnt_underrun_vect         = 0x%02x\n",status.gmii_outstanding_credits_cnt_underrun_vect);
    pr_info("xgmii_tx_afifo_overrun                             = 0x%02x\n",status.xgmii_tx_afifo_overrun);
    pr_info("gmii_tx_afifo_overrun_vect                         = 0x%02x\n",status.gmii_tx_afifo_overrun_vect);
}

reg_name_func0 mab_reg_name_func_array[] = {
    {"CNTRL", xport_mab_ctrl_dump },        
    {"TX_WRR_CTRL", xport_mab_tx_wrr_ctrl_dump},
    {"TX_THRESHOLD", xport_mab_tx_threshold_dump}, 
    {"LINK_DOWN_TX_DATA", xport_mab_link_down_tx_data_dump},
    {"STATUS", xport_mab_status_dump},
    { "", NULL},
};

static void xport_proc_cmd_mab_reg_dump(char *reg_str)
{
    reg_name_func0 *p_array = &mab_reg_name_func_array[0];
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func0();
        p_array++;
    }
}

void xlmac_reg_dir_acc_data_write_dump(void)
{
}
void xlmac_reg_dir_acc_data_read_dump(void)
{
}
void xlmac_reg_indir_acc_addr_0_dump(void)
{
}
void xlmac_reg_indir_acc_data_low_0_dump(void)
{
}
void xlmac_reg_indir_acc_data_high_0_dump(void)
{
}
void xlmac_reg_indir_acc_addr_1_dump(void)
{
}
void xlmac_reg_indir_acc_data_low_1_dump(void)
{
}
void xlmac_reg_indir_acc_data_high_1_dump(void)
{
}
void xlmac_reg_config_dump(void)
{
    xport_xlmac_reg_config config;
    ag_drv_xport_xlmac_reg_config_get(&config);
    pr_info("\n");
    pr_info("XLMAC_REG_CONFIG                                   = \n");
    pr_info("xlmac_reset                                        = 0x%02x\n",config.xlmac_reset);
    pr_info("rx_dual_cycle_tdm_en                               = 0x%02x\n",config.rx_dual_cycle_tdm_en);
    pr_info("rx_non_linear_quad_tdm_en                          = 0x%02x\n",config.rx_non_linear_quad_tdm_en);
    pr_info("rx_flex_tdm_enable                                 = 0x%02x\n",config.rx_flex_tdm_enable);
    pr_info("mac_mode                                           = 0x%02x\n",config.mac_mode);
    pr_info("osts_timer_disable                                 = 0x%02x\n",config.osts_timer_disable);
    pr_info("bypass_osts                                        = 0x%02x\n",config.bypass_osts);
    pr_info("egr_1588_timestamping_mode                         = 0x%02x\n",config.egr_1588_timestamping_mode);
}
void xlmac_reg_interrupt_check_dump(void)
{
    uint8_t xlmac_intr_check;
    ag_drv_xport_xlmac_reg_interrupt_check_get(&xlmac_intr_check);
    pr_info("\n");
    pr_info("XLMAC_REG_INTERRUPT_CHECK                          = \n");
    pr_info("xlmac_intr_check                                   = 0x%02x\n",xlmac_intr_check);
}
void xlmac_reg_port_0_rxerr_mask_dump(void)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_0_rxerr_mask_get(&rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_PORT_0_RXERR_MASK                        = \n");
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_1_rxerr_mask_dump(void)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_1_rxerr_mask_get(&rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_PORT_1_RXERR_MASK                        = \n");
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_2_rxerr_mask_dump(void)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_2_rxerr_mask_get(&rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_PORT_2_RXERR_MASK                        = \n");
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_port_3_rxerr_mask_dump(void)
{
    uint32_t rsv_err_mask;
    ag_drv_xport_xlmac_reg_port_3_rxerr_mask_get(&rsv_err_mask);
    pr_info("\n");
    pr_info("XLMAC_REG_PORT_3_RXERR_MASK                        = \n");
    pr_info("rsv_err_mask                                       = 0x%02x\n",rsv_err_mask);
}
void xlmac_reg_rmt_lpbk_cntrl_dump(void)
{
    xport_xlmac_reg_rmt_lpbk_cntrl rmt_lpbk_cntrl;
    ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_get(&rmt_lpbk_cntrl);
    pr_info("\n");
    pr_info("XLMAC_REG_RMT_LPBK_CNTRL                           = \n");
    pr_info("read_threshold                                     = 0x%02x\n",rmt_lpbk_cntrl.read_threshold);
    pr_info("tx_port_id                                         = 0x%02x\n",rmt_lpbk_cntrl.tx_port_id);
    pr_info("tx_port_sel                                        = 0x%02x\n",rmt_lpbk_cntrl.tx_port_sel);
    pr_info("rxerr_en                                           = 0x%02x\n",rmt_lpbk_cntrl.rxerr_en);
    pr_info("tx_crc_err                                         = 0x%02x\n",rmt_lpbk_cntrl.tx_crc_err);
    pr_info("tx_crc_mode                                        = 0x%02x\n",rmt_lpbk_cntrl.tx_crc_mode);
    pr_info("rmt_loopback_en                                    = 0x%02x\n",rmt_lpbk_cntrl.rmt_loopback_en);

}

reg_name_func0 xlmac_reg_name_func_array[] = {
    { "DIR_ACC_DATA_WRITE", xlmac_reg_dir_acc_data_write_dump },
    { "DIR_ACC_DATA_READ", xlmac_reg_dir_acc_data_read_dump },
    { "INDIR_ACC_ADDR_0", xlmac_reg_indir_acc_addr_0_dump },
    { "INDIR_ACC_DATA_LOW_0", xlmac_reg_indir_acc_data_low_0_dump },
    { "INDIR_ACC_DATA_HIGH_0", xlmac_reg_indir_acc_data_high_0_dump },
    { "INDIR_ACC_ADDR_1", xlmac_reg_indir_acc_addr_1_dump },
    { "INDIR_ACC_DATA_LOW_1", xlmac_reg_indir_acc_data_low_1_dump },
    { "INDIR_ACC_DATA_HIGH_1", xlmac_reg_indir_acc_data_high_1_dump },
    { "CONFIG", xlmac_reg_config_dump },
    { "INTERRUPT_CHECK", xlmac_reg_interrupt_check_dump },
    { "PORT_0_RXERR_MASK", xlmac_reg_port_0_rxerr_mask_dump },
    { "PORT_1_RXERR_MASK", xlmac_reg_port_1_rxerr_mask_dump },
    { "PORT_2_RXERR_MASK", xlmac_reg_port_2_rxerr_mask_dump },
    { "PORT_3_RXERR_MASK", xlmac_reg_port_3_rxerr_mask_dump },
    { "RMT_LPBK_CNTRL", xlmac_reg_rmt_lpbk_cntrl_dump },
    { "", NULL},
};

static void xport_proc_cmd_xlmac_reg_dump(char *reg_str)
{
    reg_name_func0 *p_array = &xlmac_reg_name_func_array[0];
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func0();
        p_array++;
    }
}

void mib_reg_cntrl_dump(void)
{
    uint8_t eee_cnt_mode; uint8_t saturate_en; uint8_t cor_en; uint8_t cnt_rst;
    ag_drv_xport_mib_reg_cntrl_get(&eee_cnt_mode,&saturate_en,&cor_en,&cnt_rst);
    pr_info("\n");
    pr_info("MIB_REG_CNTRL          L                           = \n");
    pr_info("eee_cnt_mode                                       = 0x%02x\n",eee_cnt_mode);
    pr_info("saturate_en                                        = 0x%02x\n",saturate_en);
    pr_info("cor_en                                             = 0x%02x\n",cor_en);
    pr_info("cnt_rst                                            = 0x%02x\n",cnt_rst);
}
void mib_reg_eee_pulse_duration_cntrl_dump(void)
{
    uint8_t cnt;
    ag_drv_xport_mib_reg_eee_pulse_duration_cntrl_get(&cnt);
    pr_info("\n");
    pr_info("MIB_REG_EEE_PULSE_DURATION_CNTRL                   = \n");
    pr_info("cnt                                                = 0x%02x\n",cnt);
}
void mib_reg_gport0_max_pkt_size_dump(void)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport0_max_pkt_size_get(&max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_GPORT0_MAX_PKT_SIZE                        = \n");
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport1_max_pkt_size_dump(void)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport1_max_pkt_size_get(&max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_GPORT1_MAX_PKT_SIZE                        = \n");
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport2_max_pkt_size_dump(void)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport2_max_pkt_size_get(&max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_GPORT2_MAX_PKT_SIZE                        = \n");
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_gport3_max_pkt_size_dump(void)
{
    uint16_t max_pkt_size;
    ag_drv_xport_mib_reg_gport3_max_pkt_size_get(&max_pkt_size);
    pr_info("\n");
    pr_info("MIB_REG_GPORT3_MAX_PKT_SIZE                        = \n");
    pr_info("max_pkt_size                                       = 0x%04x\n",max_pkt_size);
}
void mib_reg_ecc_cntrl_dump(void)
{
    uint8_t tx_mib_ecc_en; uint8_t rx_mib_ecc_en;
    ag_drv_xport_mib_reg_ecc_cntrl_get(&tx_mib_ecc_en,&rx_mib_ecc_en);
    pr_info("\n");
    pr_info("MIB_REG_ECC_CNTRL                                  = \n");
    pr_info("tx_mib_ecc_en                                      = 0x%04x\n",tx_mib_ecc_en);
    pr_info("rx_mib_ecc_en                                      = 0x%04x\n",rx_mib_ecc_en);

}
void mib_reg_force_sb_ecc_err_dump(void)
{
    xport_mib_reg_force_sb_ecc_err force_sb_ecc_err;
    ag_drv_xport_mib_reg_force_sb_ecc_err_get(&force_sb_ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_FORCE_SB_ECC_ERR                           = \n");
    pr_info("force_tx_mem3_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem3_serr);
    pr_info("force_tx_mem2_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem2_serr);
    pr_info("force_tx_mem1_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem1_serr);
    pr_info("force_tx_mem0_serr                                 = 0x%02x\n",force_sb_ecc_err.force_tx_mem0_serr);
    pr_info("force_rx_mem4_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem4_serr);
    pr_info("force_rx_mem3_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem3_serr);
    pr_info("force_rx_mem2_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem2_serr);
    pr_info("force_rx_mem1_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem1_serr);
    pr_info("force_rx_mem0_serr                                 = 0x%02x\n",force_sb_ecc_err.force_rx_mem0_serr);

}
void mib_reg_force_db_ecc_err_dump(void)
{
    xport_mib_reg_force_db_ecc_err force_db_ecc_err;
    ag_drv_xport_mib_reg_force_db_ecc_err_get(&force_db_ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_FORCE_DB_ECC_ERR                           = \n");
    pr_info("force_tx_mem3_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem3_derr);
    pr_info("force_tx_mem2_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem2_derr);
    pr_info("force_tx_mem1_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem1_derr);
    pr_info("force_tx_mem0_derr                                 = 0x%02x\n",force_db_ecc_err.force_tx_mem0_derr);
    pr_info("force_rx_mem4_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem4_derr);
    pr_info("force_rx_mem3_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem3_derr);
    pr_info("force_rx_mem2_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem2_derr);
    pr_info("force_rx_mem1_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem1_derr);
    pr_info("force_rx_mem0_derr                                 = 0x%02x\n",force_db_ecc_err.force_rx_mem0_derr);

}
void mib_reg_rx_mem0_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem0_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_RX_MEM0_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem1_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem1_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_RX_MEM1_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem2_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem2_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_RX_MEM2_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem3_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem3_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_RX_MEM3_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_rx_mem4_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_rx_mem4_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_RX_MEM4_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem0_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem0_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_TX_MEM0_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem1_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem1_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_TX_MEM1_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem2_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem2_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_TX_MEM2_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}
void mib_reg_tx_mem3_ecc_status_dump(void)
{
    uint8_t mem_addr; uint8_t double_bit_ecc_err; uint8_t multi_ecc_err; uint8_t ecc_err;
    ag_drv_xport_mib_reg_tx_mem3_ecc_status_get(&mem_addr,&double_bit_ecc_err,&multi_ecc_err,&ecc_err);
    pr_info("\n");
    pr_info("MIB_REG_TX_MEM3_ECC_STATUS                         = \n");
    pr_info("mem_addr                                           = 0x%02x\n",mem_addr);
    pr_info("double_bit_ecc_err                                 = 0x%02x\n",double_bit_ecc_err);
    pr_info("multi_ecc_err                                      = 0x%02x\n",multi_ecc_err);
    pr_info("ecc_err                                            = 0x%02x\n",ecc_err);
}

reg_name_func0 mib_reg_name_func_array[] = {
    { "MIB_CNTRL", mib_reg_cntrl_dump },	 		
    { "MIB_EEE_PULSE_DURATION_CNTRL", mib_reg_eee_pulse_duration_cntrl_dump },
    { "MIB_GPORT0_MAX_PKT_SIZE", mib_reg_gport0_max_pkt_size_dump },	
    { "MIB_GPORT1_MAX_PKT_SIZE", mib_reg_gport1_max_pkt_size_dump },	
    { "MIB_GPORT2_MAX_PKT_SIZE", mib_reg_gport2_max_pkt_size_dump },	
    { "MIB_GPORT3_MAX_PKT_SIZE", mib_reg_gport3_max_pkt_size_dump },	
    { "MIB_ECC_CNTRL", mib_reg_ecc_cntrl_dump },	 		
    { "MIB_FORCE_SB_ECC_ERR", mib_reg_force_sb_ecc_err_dump },	 	
    { "MIB_FORCE_DB_ECC_ERR", mib_reg_force_db_ecc_err_dump },	 	
    { "MIB_RX_MEM0_ECC_STATUS", mib_reg_rx_mem0_ecc_status_dump },	 	
    { "MIB_RX_MEM1_ECC_STATUS", mib_reg_rx_mem1_ecc_status_dump },	 	
    { "MIB_RX_MEM2_ECC_STATUS", mib_reg_rx_mem2_ecc_status_dump },	 	
    { "MIB_RX_MEM3_ECC_STATUS", mib_reg_rx_mem3_ecc_status_dump },	 	
    { "MIB_RX_MEM4_ECC_STATUS", mib_reg_rx_mem4_ecc_status_dump },	 	
    { "MIB_TX_MEM0_ECC_STATUS", mib_reg_tx_mem0_ecc_status_dump },	 	
    { "MIB_TX_MEM1_ECC_STATUS", mib_reg_tx_mem1_ecc_status_dump },	 	
    { "MIB_TX_MEM2_ECC_STATUS", mib_reg_tx_mem2_ecc_status_dump },	 	
    { "MIB_TX_MEM3_ECC_STATUS", mib_reg_tx_mem3_ecc_status_dump },
    { "", NULL},
};

static void xport_proc_cmd_mib_reg_dump(char *reg_str)
{
    reg_name_func0 *p_array = &mib_reg_name_func_array[0];
    while(p_array->name[0] != '\0')
    {
        if (!reg_str || strstr(p_array->name, reg_str)) p_array->func0();
        p_array++;
    }
}


typedef enum {
    eFIRST_BLK,
    eXLMAC_CORE = eFIRST_BLK,
    eXLMAC_REG,
    eXPORT_REG,
    eMIB_REG,
    eMAB,
    eLAST_BLK = eMAB,
    eADDR,
    eINVALID
}enum_blk_type;

typedef struct {
    enum_blk_type blk_type;
    uint32_t start_addr;
    uint32_t size;
}blk_addr_range;

blk_addr_range blk_addr[] = {
    {eXLMAC_CORE,   (XPORT_PHYS_BASE+XPORT_XLMAC_CORE_OFFSET), XPORT_XLMAC_CORE_SIZE },
    {eXLMAC_REG,    (XPORT_PHYS_BASE+XPORT_XLMAC_REG_OFFSET), XPORT_XLMAC_REG_SIZE },
    {eXPORT_REG,    (XPORT_PHYS_BASE+XPORT_REG_OFFSET), XPORT_REG_SIZE },
    {eMIB_REG,      (XPORT_PHYS_BASE+XPORT_MIB_REG_OFFSET), XPORT_MIB_REG_SIZE },
    {eMAB,          (XPORT_PHYS_BASE+XPORT_MAB_REG_OFFSET), XPORT_MAB_REG_SIZE },
    {eINVALID,0,0}
};


static int xport_proc_cmd_reg_dump(int argc, char *argv[])
{
    int port = 0;
    char *xport_reg_dump_usage = "Usage: reg_dump [XLMAC_CORE <port id (0-1)> | XLMAC_REG | XPORT_REG | MIB_REG | MAB | addr] <Qualifying string>";
    int blk_idx = 1;
    int port_needed = 0;
    char *blk_p = argv[1];
    char *q_str_p = NULL;
    enum_blk_type blk_type = eINVALID;
    uint32_t reg_phy_addr = 0;
    uintptr_t reg_addr;

    if (argc == 1)
    {
        port = 0;
        blk_type = eFIRST_BLK;
        goto DUMP_REGS;
    }
    if (argc < 2)
    {
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }

    if (!strcmp(blk_p, "XLMAC_CORE")) { blk_type = eXLMAC_CORE; port_needed = 1;} 
    else if (!strcmp(blk_p, "XLMAC_REG")) blk_type = eXLMAC_REG;
    else if (!strcmp(blk_p, "XPORT_REG")) blk_type = eXPORT_REG;
    else if (!strcmp(blk_p, "MIB_REG")) blk_type = eMIB_REG;
    else if (!strcmp(blk_p, "MAB")) blk_type = eMAB;
    else if (!strcmp(blk_p, "addr")) blk_type = eADDR;

    if (blk_type == eADDR && argc == 3) {
        int i;
        uint32_t v32;
        uint64_t v64;
        kstrtou32(argv[2], 16, &reg_phy_addr);
        for (i=0; blk_addr[i].blk_type != eINVALID;i++) {
            if (reg_phy_addr >= (blk_addr[i].start_addr) && reg_phy_addr <= (blk_addr[i].start_addr+blk_addr[i].size)) {
                blk_type = blk_addr[i].blk_type;
                break;
            }
        }
        if (blk_addr[i].blk_type == eINVALID) {
            pr_info("\n");
            pr_info("Reg Address <0x%08x> Outside of XPORT block range\n", reg_phy_addr);
            return 0;
        }
        reg_addr = (uintptr_t)XPORT_BASE + ( reg_phy_addr - XPORT_PHYS_BASE+XPORT_OFFSET );

        switch ( blk_type )
        {
        case eXLMAC_CORE:
            xport_xlmac_indirect_read(0, reg_addr, &v64);
            printk(" 0x%08x = 0x%016llx \n",reg_phy_addr,v64);
            break;
        case eXLMAC_REG:
        case eXPORT_REG:
        case eMIB_REG:
        case eMAB:
            READ_32(reg_addr, v32);
            printk(" 0x%08x = 0x%08x \n",reg_phy_addr,v32);
            break;
        default:
            break;
        }
        return 0;
    }

    if (blk_type == eINVALID || (port_needed && argc <= blk_idx+port_needed))
    {
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }
    if (port_needed && ((port = xport_port_validate(argc, argv, blk_idx+1, 0, 1)) == -1))
    {
        pr_info("Command validation failed : port <%d> argc <%d> ", port, argc);
        pr_info("\n");
        pr_info("%s\n", xport_reg_dump_usage);
        return 0;
    }


    if (argc > blk_idx+port_needed+1)
    {
        q_str_p = argv[blk_idx+port_needed+1];
    }

DUMP_REGS:
    do
    {
        switch ( blk_type )
        {
        case eXLMAC_CORE:
            xport_proc_cmd_xlmac_core_reg_dump(port, q_str_p);
            break;
        case eXLMAC_REG:
            xport_proc_cmd_xlmac_reg_dump(q_str_p);
            break;
        case eXPORT_REG:
            xport_proc_cmd_xport_reg_reg_dump(q_str_p);
            break;
        case eMIB_REG:
            xport_proc_cmd_mib_reg_dump(q_str_p);
            break;
        case eMAB:
            xport_proc_cmd_mab_reg_dump(q_str_p);
            break;
        default:
            break;
        }
        if (blk_type == eXLMAC_CORE && port == 0)
        {
            port = 1;
        }
        else blk_type++;

    } while (argc == 1 && blk_type != eLAST_BLK+1);
    return 0;
}
static int xport_proc_cmd_reg_set(int argc, char *argv[])
{
    char *xport_reg_set_usage = "Usage: reg_set <Reg addr (hex)> <Value (hex)> <field Position> <Width>";
    uint32_t reg_phy_addr = 0;
    uint32_t v32_Lo = 0;
    uint32_t v32 = 0;
    uint64_t v64, field_mask = 0;
    uint32_t field_pos, width, width_tmp;
    enum_blk_type blk_type = eINVALID;
    uintptr_t reg_addr;
    int i;

    if (argc < 5)
    {
        pr_info("\n");
        pr_info("%s\n", xport_reg_set_usage);
        return 0;
    }

    if (kstrtou32(argv[1], 16, &reg_phy_addr))
    {
        pr_info("\n%s\nInvalid <Reg addr (hex)>\n", xport_reg_set_usage);
        return 0;
    }
    if (kstrtou32(argv[2], 16, &v32_Lo))
    {
        pr_info("\n%s\nInvalid <Value (hex)>\n", xport_reg_set_usage);
        return 0;
    }
    if (kstrtou32(argv[3], 10, &field_pos))
    {
        pr_info("\n%s\nInvalid <field Position>\n", xport_reg_set_usage);
        return 0;
    }
    if (kstrtou32(argv[4], 10, &width))
    {
        pr_info("\n%s\nInvalid <Width>\n", xport_reg_set_usage);
        return 0;
    }
    width_tmp = width;

    for (i=0; blk_addr[i].blk_type != eINVALID;i++) {
        if (reg_phy_addr >= (blk_addr[i].start_addr) && reg_phy_addr <= (blk_addr[i].start_addr+blk_addr[i].size)) {
            blk_type = blk_addr[i].blk_type;
            break;
        }
    }
    if (blk_addr[i].blk_type == eINVALID) {
        pr_info("\n");
        pr_info("Reg Address <0x%08x> Outside of XPORT block range\n", reg_phy_addr);
        return 0;
    }

    while(width) {
        field_mask = (field_mask<<1) + 1;
        width--;
    }
    field_mask <<= field_pos;


    reg_addr = (uintptr_t)XPORT_BASE + ( reg_phy_addr - XPORT_PHYS_BASE+XPORT_OFFSET );

    pr_info ("Address <0x%08x : 0x%p> Val <0x%08x> Pos <%u> width <%u> mask <0x%016llx>\n",reg_phy_addr,(void*)reg_addr, v32_Lo, field_pos, width_tmp, field_mask);

    switch ( blk_type )
    {
    case eXLMAC_CORE:
        xport_xlmac_indirect_read(0, reg_addr, &v64);
        v64 &= ~field_mask ;
        v64 |= ((((uint64_t)v32_Lo) << field_pos) & field_mask);
        xport_xlmac_indirect_write(0, reg_addr, v64);
        break;
    case eXLMAC_REG:
    case eXPORT_REG:
    case eMIB_REG:
    case eMAB:
        READ_32(reg_addr, v32);
        v32 &= ~field_mask ;
        v32 |= ((((uint64_t)v32_Lo) << field_pos) & field_mask);
        WRITE_32(reg_addr, v32);
        break;
    default:
        break;
    }

    return 0;
}

static struct proc_cmd_ops command_entries[] = {
    { .name = "status", .do_command	= xport_proc_cmd_status},
    { .name = "reg_dump", .do_command	= xport_proc_cmd_reg_dump},
    { .name = "reg_set", .do_command	= xport_proc_cmd_reg_set},
};

static struct proc_cmd_table xport_command_table = {
    .module_name = "XPORT",
    .size = ARRAY_SIZE(command_entries),
    .ops = command_entries
};



static void xport_proc_exit(void)
{
    if (cmd_proc_file) 
    {
        remove_proc_entry(CMD_PROC_FILE, proc_dir);
        cmd_proc_file = NULL;
    }	
    if (proc_dir)
    {
        remove_proc_entry(PROC_DIR, NULL);
        proc_dir = NULL;
    }	
}

int __init xport_proc_init(void)
{
    int status = 0;

    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) 
    {
        pr_err("Failed to create PROC directory %s.\n",
            PROC_DIR);
        goto error;
    }
    cmd_proc_file = proc_create_cmd(CMD_PROC_FILE, proc_dir,
        &xport_command_table);
    if (!cmd_proc_file) 
    {
        pr_err("Failed to create %s\n", CMD_PROC_FILE);
        goto error;
    }

    return status;

error:
    if (proc_dir)
        xport_proc_exit();

    status = -EIO;
    return status;
}
postcore_initcall(xport_proc_init);
