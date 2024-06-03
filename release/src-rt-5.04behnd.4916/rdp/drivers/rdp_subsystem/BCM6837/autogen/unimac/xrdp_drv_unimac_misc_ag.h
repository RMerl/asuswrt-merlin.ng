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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint16_t rxfifo_pause_threshold;
    bdmf_boolean backpressure_enable_int;
    bdmf_boolean backpressure_enable_ext;
    bdmf_boolean fifo_overrun_ctl_en;
    bdmf_boolean remote_loopback_en;
} unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2;

typedef struct
{
    uint8_t port_rate;
    bdmf_boolean lpi_tx_detect;
    bdmf_boolean lpi_rx_detect;
    uint8_t pp_stats;
    bdmf_boolean pp_stats_valid;
} unimac_misc_unimac_top_unimac_misc_unimac_stat;


/**********************************************************************************************************************
 * mac_crc_fwd: 
 *     Forward CRC
 * mac_crc_owrt: 
 *     Overwrite CRC
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(uint8_t umac_misc_id, bdmf_boolean mac_crc_fwd, bdmf_boolean mac_crc_owrt);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(uint8_t umac_misc_id, bdmf_boolean *mac_crc_fwd, bdmf_boolean *mac_crc_owrt);

/**********************************************************************************************************************
 * max_pkt_size: 
 *     Max Packet Size - Used fro statistics
 * rxfifo_congestion_threshold: 
 *     RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached,
 *     rx_fifo_congestion output is set.
 *     This configuration is in 16-byte resolution (the number of bytes in a FIFO line).
 *     Max value is: 0x100.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(uint8_t umac_misc_id, uint16_t max_pkt_size, uint16_t rxfifo_congestion_threshold);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(uint8_t umac_misc_id, uint16_t *max_pkt_size, uint16_t *rxfifo_congestion_threshold);

/**********************************************************************************************************************
 * rxfifo_pause_threshold: 
 *     RX FIFO Threshold - This is the fifo located between the UNIMAC IP and BBH. Once the threshold is reached,
 *     pause is sent. This configuration is in 16-byte resolution (the number of bytes in a FIFO line).
 *     Max Value is 0x100.
 * backpressure_enable_int: 
 *     Backpressure enable for internal unimac
 * backpressure_enable_ext: 
 *     Backpressure enable for external switch
 * fifo_overrun_ctl_en: 
 *     Enable the mechanism for always receiving data from UNIMAC IP and dropping overrun words in the unimac_glue
 *     FIFO.
 * remote_loopback_en: 
 *     When setting this bit, RX recovered clock will also input the clk25/pll_ref_clk in the UNIMAC.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(uint8_t umac_misc_id, const unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2);

/**********************************************************************************************************************
 * port_rate: 
 *     Port data rate, to be used in LED block.                                 encoded as:
 *     3b000:   10Mb/s
 *     3b001:  100Mb/s
 *     3b010: 1000Mb/s
 *     3b011: 2500Mb/s
 *     3b100:   10Gb/s or higher
 *     3b101:    5Gb/s
 *     3b110: reserved
 *     3b111: reserved
 *     
 *     
 * lpi_tx_detect: 
 *     Transmit LPI State - Set to 1 whenever LPI_IDLES are being Sent out on the TX Line
 * lpi_rx_detect: 
 *     Receive LPI State - Set to 1 whenever LPI_IDLES are being received on the RX Line
 * pp_stats: 
 *     The XOFF/XON status of the receiving PPP control frame
 *     
 * pp_stats_valid: 
 *     MAC to indicate a PPP control frame is received
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_stat *unimac_top_unimac_misc_unimac_stat);

/**********************************************************************************************************************
 * debug_sel: 
 *     Select which debug bus to output.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(uint8_t umac_misc_id, uint8_t debug_sel);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(uint8_t umac_misc_id, uint8_t *debug_sel);

/**********************************************************************************************************************
 * unimac_rst: 
 *     Synchronous UNIMAC reset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(uint8_t umac_misc_id, bdmf_boolean unimac_rst);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(uint8_t umac_misc_id, bdmf_boolean *unimac_rst);

/**********************************************************************************************************************
 * overrun_counter: 
 *     This registers holds the number of 128-bit lines that were received from the UNIMAC IP, but were thrown due to
 *     the fact that the unimac_glue FIFO was full. This field stops counting when it reaches 0xff and will be cleared
 *     upon read.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(uint8_t umac_misc_id, uint32_t *overrun_counter);

/**********************************************************************************************************************
 * tsi_sign_ext: 
 *     The MAC logic will need to evaluate this bit in order to determine how to sign extend the Timestamp (TSe).
 *     If the bit is a zero, the TSe MUST be sign extended with zeros.
 *     If the bit is a one, the TSe MUST be sign extended based on the Msbit of the TSe. That is, if the Msbit of the
 *     TSe is zero, the TSe is signed extended with zeros. If the Msbit of the TSe is one, the TSe is sign extended
 *     with one. (from unimac spec)
 *     
 * osts_timer_dis: 
 *     This active high signal is used to disable the OSTS time-stamping function in the MAC when asserted during the
 *     time the CPU Is initializing the local counters.(from unimac spec)
 * egr_1588_ts_mode: 
 *     When set to 0, enables legacy sign-extension of 32-bit timestamp, when 1, enables 48-bit time-stamping. (from
 *     unimac spec)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(uint8_t umac_misc_id, bdmf_boolean tsi_sign_ext, bdmf_boolean osts_timer_dis, bdmf_boolean egr_1588_ts_mode);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(uint8_t umac_misc_id, bdmf_boolean *tsi_sign_ext, bdmf_boolean *osts_timer_dis, bdmf_boolean *egr_1588_ts_mode);

/**********************************************************************************************************************
 * gen_int: 
 *     general interrupt
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(uint8_t umac_misc_id, bdmf_boolean gen_int);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(uint8_t umac_misc_id, bdmf_boolean *gen_int);

/**********************************************************************************************************************
 * value: 
 *     The bit description is the same like for the ISR.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(uint8_t umac_misc_id, bdmf_boolean value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(uint8_t umac_misc_id, bdmf_boolean *value);

/**********************************************************************************************************************
 * value: 
 *     The bit description is the same like for the ISR.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(uint8_t umac_misc_id, bdmf_boolean value);
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(uint8_t umac_misc_id, bdmf_boolean *value);

/**********************************************************************************************************************
 * value: 
 *     The bit description is the same like for the ISR.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(uint8_t umac_misc_id, bdmf_boolean *value);

#ifdef USE_BDMF_SHELL
enum
{
    cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_stat,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_debug,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_rst,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter,
    cli_unimac_misc_unimac_top_unimac_misc_unimac_1588,
    cli_unimac_misc_unimac_top_unimac_ints_isr,
    cli_unimac_misc_unimac_top_unimac_ints_ier,
    cli_unimac_misc_unimac_top_unimac_ints_itr,
    cli_unimac_misc_unimac_top_unimac_ints_ism,
};

int bcm_unimac_misc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_unimac_misc_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
