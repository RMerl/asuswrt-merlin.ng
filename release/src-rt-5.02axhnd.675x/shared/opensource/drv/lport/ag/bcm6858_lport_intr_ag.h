/*
   Copyright (c) 2015 Broadcom Corporation
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

#ifndef _BCM6858_LPORT_INTR_AG_H_
#define _BCM6858_LPORT_INTR_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t qgphy_energy_off_intr;
    uint8_t qgphy_energy_on_intr;
    uint8_t mdio_err_intr;
    uint8_t mdio_done_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} lport_intr_status_0;

typedef struct
{
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t qgphy_energy_off_intr;
    uint8_t qgphy_energy_on_intr;
    uint8_t mdio_err_intr;
    uint8_t mdio_done_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} lport_intr_set_0;

typedef struct
{
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t qgphy_energy_off_intr;
    uint8_t qgphy_energy_on_intr;
    uint8_t mdio_err_intr;
    uint8_t mdio_done_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} lport_intr_clear_0;

typedef struct
{
    uint8_t mac_rx_cdc_single_bit_err_intr_mask;
    uint8_t mac_rx_cdc_double_bit_err_intr_mask;
    uint8_t mac_tx_cdc_single_bit_err_intr_mask;
    uint8_t mac_tx_cdc_double_bit_err_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t qgphy_energy_off_intr_mask;
    uint8_t qgphy_energy_on_intr_mask;
    uint8_t mdio_err_intr_mask;
    uint8_t mdio_done_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} lport_intr_mask_status_0;

typedef struct
{
    uint8_t mac_rx_cdc_single_bit_err_intr_mask;
    uint8_t mac_rx_cdc_double_bit_err_intr_mask;
    uint8_t mac_tx_cdc_single_bit_err_intr_mask;
    uint8_t mac_tx_cdc_double_bit_err_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t qgphy_energy_off_intr_mask;
    uint8_t qgphy_energy_on_intr_mask;
    uint8_t mdio_err_intr_mask;
    uint8_t mdio_done_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} lport_intr_mask_set_0;

typedef struct
{
    uint8_t mac_rx_cdc_single_bit_err_intr_mask;
    uint8_t mac_rx_cdc_double_bit_err_intr_mask;
    uint8_t mac_tx_cdc_single_bit_err_intr_mask;
    uint8_t mac_tx_cdc_double_bit_err_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t qgphy_energy_off_intr_mask;
    uint8_t qgphy_energy_on_intr_mask;
    uint8_t mdio_err_intr_mask;
    uint8_t mdio_done_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} lport_intr_mask_clear_0;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
} lport_intr_status_1;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
} lport_intr_set_1;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
} lport_intr_clear_1;

typedef struct
{
    uint8_t rx_remote_fault_intr_mask;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
} lport_intr_mask_status_1;

typedef struct
{
    uint8_t rx_remote_fault_intr_mask;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
} lport_intr_mask_set_1;

typedef struct
{
    uint8_t rx_remote_fault_intr_mask;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
} lport_intr_mask_clear_1;

int ag_drv_lport_intr_status_0_set(const lport_intr_status_0 *status_0);
int ag_drv_lport_intr_status_0_get(lport_intr_status_0 *status_0);
int ag_drv_lport_intr_set_0_set(const lport_intr_set_0 *set_0);
int ag_drv_lport_intr_set_0_get(lport_intr_set_0 *set_0);
int ag_drv_lport_intr_clear_0_set(const lport_intr_clear_0 *clear_0);
int ag_drv_lport_intr_clear_0_get(lport_intr_clear_0 *clear_0);
int ag_drv_lport_intr_mask_status_0_set(const lport_intr_mask_status_0 *mask_status_0);
int ag_drv_lport_intr_mask_status_0_get(lport_intr_mask_status_0 *mask_status_0);
int ag_drv_lport_intr_mask_set_0_set(const lport_intr_mask_set_0 *mask_set_0);
int ag_drv_lport_intr_mask_set_0_get(lport_intr_mask_set_0 *mask_set_0);
int ag_drv_lport_intr_mask_clear_0_set(const lport_intr_mask_clear_0 *mask_clear_0);
int ag_drv_lport_intr_mask_clear_0_get(lport_intr_mask_clear_0 *mask_clear_0);
int ag_drv_lport_intr_status_1_set(const lport_intr_status_1 *status_1);
int ag_drv_lport_intr_status_1_get(lport_intr_status_1 *status_1);
int ag_drv_lport_intr_set_1_set(const lport_intr_set_1 *set_1);
int ag_drv_lport_intr_set_1_get(lport_intr_set_1 *set_1);
int ag_drv_lport_intr_clear_1_set(const lport_intr_clear_1 *clear_1);
int ag_drv_lport_intr_clear_1_get(lport_intr_clear_1 *clear_1);
int ag_drv_lport_intr_mask_status_1_set(const lport_intr_mask_status_1 *mask_status_1);
int ag_drv_lport_intr_mask_status_1_get(lport_intr_mask_status_1 *mask_status_1);
int ag_drv_lport_intr_mask_set_1_set(const lport_intr_mask_set_1 *mask_set_1);
int ag_drv_lport_intr_mask_set_1_get(lport_intr_mask_set_1 *mask_set_1);
int ag_drv_lport_intr_mask_clear_1_set(const lport_intr_mask_clear_1 *mask_clear_1);
int ag_drv_lport_intr_mask_clear_1_get(lport_intr_mask_clear_1 *mask_clear_1);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_intr_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

