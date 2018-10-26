/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _XRDP_DRV_UNIMAC_MISC_AG_H_
#define _XRDP_DRV_UNIMAC_MISC_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* direct_gmii: direct_gmii - Direct GMII mode. If direct set to 0, the only 25MHz clock needs to */
/*               be fed at ref25, for both 10/100Mbps speeds                                      */
/* gport_mode: gport_mode - GPORT mode to support legacy ENTSW gport/gxport RX FIFO protocol      */
/* ss_mode_mii: ss_mode_mii - Indicates whether the unimac should operate in source synchronous m */
/*              ode                                                                               */
/* txcrcer: txcrcer - This when asserted along with MAC_TXEOP indicates a corrupt CRC of the pack */
/*          et                                                                                    */
/* mac_crc_fwd: mac_crc_fwd - Forward CRC                                                         */
/* mac_crc_owrt: mac_crc_owrt - Overwrite CRC                                                     */
/* ext_tx_flow_control: ext_tx_flow_control - When this active high signal is asserted and config */
/*                       bit OOB_EFC_EN is set, then data frame transmission is stopped           */
/* launch_enable: launch_enable - If enabled through register programming, the launch enable risi */
/*                ng edge enables the packet transmit on per packet basis                         */
/* pp_pse_en: pp_pse_en - The corresponding class enable vector of PPP control frame              */
/* pp_gen: pp_gen - Instructs MAC to generate a PPP control frame                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean direct_gmii;
    bdmf_boolean gport_mode;
    bdmf_boolean ss_mode_mii;
    bdmf_boolean txcrcer;
    bdmf_boolean mac_crc_fwd;
    bdmf_boolean mac_crc_owrt;
    bdmf_boolean ext_tx_flow_control;
    bdmf_boolean launch_enable;
    uint8_t pp_pse_en;
    bdmf_boolean pp_gen;
} unimac_misc_unimac_top_unimac_misc_unimac_cfg;


/**************************************************************************************************/
/* rxfifo_pause_threshold: rxfifo_pause_threshold - RX FIFO Threshold - This is the fifo located  */
/*                         between the UNIMAC IP and BBH. Once the threshold is reached, pause is */
/*                          sent. This configuration is in 16-byte resolution (the number of byte */
/*                         s in a FIFO line).Max Value is 0x100.                                  */
/* backpressure_enable_int: backpressure_enable_int - Backpressure enable for internal unimac     */
/* backpressure_enable_ext: backpressure_enable_ext - Backpressure enable for external switch     */
/* fifo_overrun_ctl_en: fifo_overrun_ctl_en - Enable the mechanism for always receiving data from */
/*                       UNIMAC IP and dropping overrun words in the unimac_glue FIFO.            */
/* remote_loopback_en: remote_loopback_en - When setting this bit, RX recovered clock will also i */
/*                     nput the clk25/pll_ref_clk in the UNIMAC.                                  */
/**************************************************************************************************/
typedef struct
{
    uint16_t rxfifo_pause_threshold;
    bdmf_boolean backpressure_enable_int;
    bdmf_boolean backpressure_enable_ext;
    bdmf_boolean fifo_overrun_ctl_en;
    bdmf_boolean remote_loopback_en;
} unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2;


/**************************************************************************************************/
/* auto_config_en: auto_config_en - Indicates auto config mode is enabled                         */
/* eth_duplex: eth_duplex - Indicates current Duplex mode (0 - full duplex, 1 - half duplex)      */
/* eth_speed: eth_speed - Indicates current speed (0 - 10Mbps, 1 - 100Mbps, 2 - 1Gbps, 3 - 2.5Gbp */
/*            s)                                                                                  */
/* rx_fifo_dat_avl: rx_fifo_dat_avl - Indicates when onde or more entries in UNIMAC FIFO are full */
/*                  . Used for power management purposes                                          */
/* mac_in_pause: mac_in_pause - Asserted when MAC_tx is paused                                    */
/* mac_tx_empty: mac_tx_empty - Indicates that MAC Transmit Path is Empty                         */
/* launch_ack: launch_ack - Acknowledges launch enable after the packet starts transmission. Goes */
/*              low after launch_ena goes low                                                     */
/* lpi_tx_detect: lpi_tx_detect - Transmit LPI State - Set to 1 whenever LPI_IDLES are being Sent */
/*                 out on the TX Line                                                             */
/* lpi_rx_detect: lpi_rx_detect - Receive LPI State - Set to 1 whenever LPI_IDLES are being recei */
/*                ved on the RX Line                                                              */
/* rx_sop_out: rx_sop_out - SOP detected for the received packet                                  */
/* rx_sop_delete: rx_sop_delete - The received packet is dropped                                  */
/* mac_stats_update: mac_stats_update - MAC STATS need to be updated                              */
/* pp_stats: pp_stats - The XOFF/XON status of the receiving PPP control frame                    */
/* pp_stats_valid: pp_stats_valid - MAC to indicate a PPP control frame is received               */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean auto_config_en;
    bdmf_boolean eth_duplex;
    uint8_t eth_speed;
    bdmf_boolean rx_fifo_dat_avl;
    bdmf_boolean mac_in_pause;
    bdmf_boolean mac_tx_empty;
    bdmf_boolean launch_ack;
    bdmf_boolean lpi_tx_detect;
    bdmf_boolean lpi_rx_detect;
    bdmf_boolean rx_sop_out;
    bdmf_boolean rx_sop_delete;
    bdmf_boolean mac_stats_update;
    uint8_t pp_stats;
    bdmf_boolean pp_stats_valid;
} unimac_misc_unimac_top_unimac_misc_unimac_stat;

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(uint8_t umac_misc_id, const unimac_misc_unimac_top_unimac_misc_unimac_cfg *unimac_top_unimac_misc_unimac_cfg);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_cfg *unimac_top_unimac_misc_unimac_cfg);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(uint8_t umac_misc_id, uint16_t max_pkt_size, uint16_t rxfifo_congestion_threshold);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(uint8_t umac_misc_id, uint16_t *max_pkt_size, uint16_t *rxfifo_congestion_threshold);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(uint8_t umac_misc_id, const unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_set(uint8_t umac_misc_id, uint32_t gport_stat_update_mask);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_get(uint8_t umac_misc_id, uint32_t *gport_stat_update_mask);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_stat *unimac_top_unimac_misc_unimac_stat);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(uint8_t umac_misc_id, uint8_t debug_sel);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(uint8_t umac_misc_id, uint8_t *debug_sel);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(uint8_t umac_misc_id, bdmf_boolean unimac_rst);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(uint8_t umac_misc_id, bdmf_boolean *unimac_rst);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(uint8_t umac_misc_id, uint32_t gport_rsv_mask);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_get(uint8_t umac_misc_id, uint32_t *gport_rsv_mask);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(uint8_t umac_misc_id, uint32_t *overrun_counter);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(uint8_t umac_misc_id, bdmf_boolean tsi_sign_ext, bdmf_boolean osts_timer_dis, bdmf_boolean egr_1588_ts_mode);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(uint8_t umac_misc_id, bdmf_boolean *tsi_sign_ext, bdmf_boolean *osts_timer_dis, bdmf_boolean *egr_1588_ts_mode);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(uint8_t umac_misc_id, bdmf_boolean gen_int);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(uint8_t umac_misc_id, bdmf_boolean *gen_int);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(uint8_t umac_misc_id, bdmf_boolean value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(uint8_t umac_misc_id, bdmf_boolean *value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(uint8_t umac_misc_id, bdmf_boolean value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(uint8_t umac_misc_id, bdmf_boolean *value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(uint8_t umac_misc_id, bdmf_boolean *value);

#ifdef USE_BDMF_SHELL
enum
{
    cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_stat,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_debug,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_rst,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_1588,
    cli_unimac_misc_unimac_top_unimac_ints_isr,
    cli_unimac_misc_unimac_top_unimac_ints_ier,
    cli_unimac_misc_unimac_top_unimac_ints_itr,
    cli_unimac_misc_unimac_top_unimac_ints_ism,
};

int bcm_unimac_misc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_unimac_misc_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

