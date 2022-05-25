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

#ifndef _BCM4912_XPORT_XLMAC_REG_AG_H_
#define _BCM4912_XPORT_XLMAC_REG_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"

/**************************************************************************************************/
/* err:  - Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1)  */
/*      and this bit is set it indicates that register transaction completed with error.          */
/* start_busy:  - START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indire */
/*             ct register read/write transaction. When transaction completes hardware clears thi */
/*             s bit.                                                                             */
/* r_w:  - Register transaction:
0 : Register Write.
'1 : Register Read.
                         */
/* reg_port_id:  - Register Port ID.                                                              */
/* reg_offset:  - Register offset.
Note: Bit 7 is ignored by HW. Write it as 0.                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_xlmac_reg_indir_acc_addr_0;


/**************************************************************************************************/
/* err:  - Transaction Status. When transaction completes (START_BUSY = 0 after it was set to 1)  */
/*      and this bit is set it indicates that register transaction completed with error.          */
/* start_busy:  - START_BUSY, Self-clearing. CPU writes this bit to 1 in order to initiate indire */
/*             ct register read/write transaction. When transaction completes hardware clears thi */
/*             s bit.                                                                             */
/* r_w:  - Register transaction:
0 : Register Write.
'1 : Register Read.
                         */
/* reg_port_id:  - Register Port ID.                                                              */
/* reg_offset:  - Register offset.
Note: Bit 7 is ignored by HW. Write it as 0.                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t err;
    uint8_t start_busy;
    uint8_t r_w;
    uint8_t reg_port_id;
    uint8_t reg_offset;
} xport_xlmac_reg_indir_acc_addr_1;


/**************************************************************************************************/
/* xlmac_reset:  - Active high XLMAC hard reset.                                                  */
/* rx_dual_cycle_tdm_en:  - When set, Rx CDC FIFO read TDM order has same port for 2 consecutive  */
/*                       cycles.
This is a strap input for the MAC core and should be changed onl */
/*                       y while hard reset is asserted.                                          */
/* rx_non_linear_quad_tdm_en:  - When set, RX CDC FIFO read TDM generation order for quad mode is */
/*                             0,2,1,3. Otherwise, it is 0,1,2,3.
This is a strap input for the M */
/*                            AC core and should be changed only while hard reset is asserted.    */
/* rx_flex_tdm_enable:  - Enables non-linear TDM generation on the receive system interface,  bas */
/*                     ed on data availability in Rx FIFOs.
0 : Flex TDM Enabled.
1 : Flex TDM Di */
/*                     sabled.
                                                                   */
/* mac_mode:  - Number of ports supported by XLMAC.
000 : Quad Port.All ports are used.
001 : Tri */
/*           -Port. Ports 0, 1 and 2 are used.
010 : Tri-Port. Ports 0, 2 and 3 are used.
011 : D */
/*           ual Port. Port 0 and 2 are used.
1xx : Single Port. Port 0 is used.
Note: Valid comb */
/*           inations for 63158 are single Port (P0 active) or Quad Port (P0 and/or P1 active).   */
/* osts_timer_disable:  - OSTS time-stamping disable.
0 : OSTS Enabled.
1 : OSTS Disabled.
       */
/* bypass_osts:  - Bypasses transmit OSTS functionality. When set, reduces Tx path latency.
0 : D */
/*              o not bypass transmit OSTS function.
1 : Bypass transmit OSTS function.
XLMAC mus */
/*              t be reset for this bit to take effect.                                           */
/* egr_1588_timestamping_mode:  - 1588 Egress Time-stamping mode.
0 : Legacy, sign extended 32-bi */
/*                             t timestamp mode.
1 : 48-bit timestamp mode.
XLMAC must be reset f */
/*                             or this bit to take effect.                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t xlmac_reset;
    uint8_t rx_dual_cycle_tdm_en;
    uint8_t rx_non_linear_quad_tdm_en;
    uint8_t rx_flex_tdm_enable;
    uint8_t mac_mode;
    uint8_t osts_timer_disable;
    uint8_t bypass_osts;
    uint8_t egr_1588_timestamping_mode;
} xport_xlmac_reg_config;


/**************************************************************************************************/
/* read_threshold:  - Remote loopback logic starts reading packet data from the loopback FIFO onl */
/*                 y when at least READ_THRESHOLD entries are available in the FIFO. Used to prev */
/*                 ent XLMAC TX underflow.                                                        */
/* tx_port_id:  - TX PORT_ID[1:0]. Valid only when TX_PORT_SEL = 1.                               */
/* tx_port_sel:  - When set TX PORT_ID[1:0] comes from this registers. When cleared TX PORT_ID[1: */
/*              0] equals RX PORT_ID[1:0]. TX PORT_ID[1:0] is used by remote loopback logic to mo */
/*              nitor EP credits and to indicate outgoing XLMAC port.                             */
/* rxerr_en:  - When set RXERR is propagated to TXERR. When cleared TXERR = 0.                    */
/* tx_crc_err:  - When set CRC is corrupted for the outgoing packet.                              */
/* tx_crc_mode:  - TX CRC Mode. Encoded as:
00 : CRC Append.
01 : CRC Forward.
10 : CRC Replace.
 */
/*              11 : Reserved.
CRC Append mode should be enabled only if XLMAC is programmed to s */
/*              trip off CRC.                                                                     */
/* rmt_loopback_en:  - When set enables XLMAC Remote (RX to TX) loopback. XLMAC must be kept in r */
/*                  eset while remote loopback is being enabled and released from the reset there */
/*                  after.                                                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t read_threshold;
    uint8_t tx_port_id;
    uint8_t tx_port_sel;
    uint8_t rxerr_en;
    uint8_t tx_crc_err;
    uint8_t tx_crc_mode;
    uint8_t rmt_loopback_en;
} xport_xlmac_reg_rmt_lpbk_cntrl;

int ag_drv_xport_xlmac_reg_dir_acc_data_write_set(uint8_t xlmac_id, uint32_t write_data);
int ag_drv_xport_xlmac_reg_dir_acc_data_write_get(uint8_t xlmac_id, uint32_t *write_data);
int ag_drv_xport_xlmac_reg_dir_acc_data_read_get(uint8_t xlmac_id, uint32_t *read_data);
int ag_drv_xport_xlmac_reg_indir_acc_addr_0_set(uint8_t xlmac_id, const xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_xlmac_reg_indir_acc_addr_0_get(uint8_t xlmac_id, xport_xlmac_reg_indir_acc_addr_0 *indir_acc_addr_0);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_0_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_0_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xport_xlmac_reg_indir_acc_addr_1_set(uint8_t xlmac_id, const xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_xlmac_reg_indir_acc_addr_1_get(uint8_t xlmac_id, xport_xlmac_reg_indir_acc_addr_1 *indir_acc_addr_1);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_set(uint8_t xlmac_id, uint32_t data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_low_1_get(uint8_t xlmac_id, uint32_t *data_low);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_set(uint8_t xlmac_id, uint32_t data_high);
int ag_drv_xport_xlmac_reg_indir_acc_data_high_1_get(uint8_t xlmac_id, uint32_t *data_high);
int ag_drv_xport_xlmac_reg_config_set(uint8_t xlmac_id, const xport_xlmac_reg_config *config);
int ag_drv_xport_xlmac_reg_config_get(uint8_t xlmac_id, xport_xlmac_reg_config *config);
int ag_drv_xport_xlmac_reg_interrupt_check_set(uint8_t xlmac_id, uint8_t xlmac_intr_check);
int ag_drv_xport_xlmac_reg_interrupt_check_get(uint8_t xlmac_id, uint8_t *xlmac_intr_check);
int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_0_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_1_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_2_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_set(uint8_t xlmac_id, uint32_t rsv_err_mask);
int ag_drv_xport_xlmac_reg_port_3_rxerr_mask_get(uint8_t xlmac_id, uint32_t *rsv_err_mask);
int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_set(uint8_t xlmac_id, const xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl);
int ag_drv_xport_xlmac_reg_rmt_lpbk_cntrl_get(uint8_t xlmac_id, xport_xlmac_reg_rmt_lpbk_cntrl *rmt_lpbk_cntrl);
int ag_drv_xport_xlmac_reg_port_0_mib_rsv_mask_set(uint8_t xlmac_id, uint32_t mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_0_mib_rsv_mask_get(uint8_t xlmac_id, uint32_t *mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_1_mib_rsv_mask_set(uint8_t xlmac_id, uint32_t mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_1_mib_rsv_mask_get(uint8_t xlmac_id, uint32_t *mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_2_mib_rsv_mask_set(uint8_t xlmac_id, uint32_t mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_2_mib_rsv_mask_get(uint8_t xlmac_id, uint32_t *mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_3_mib_rsv_mask_set(uint8_t xlmac_id, uint32_t mib_rsv_mask);
int ag_drv_xport_xlmac_reg_port_3_mib_rsv_mask_get(uint8_t xlmac_id, uint32_t *mib_rsv_mask);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_xlmac_reg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

