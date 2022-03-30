/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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

