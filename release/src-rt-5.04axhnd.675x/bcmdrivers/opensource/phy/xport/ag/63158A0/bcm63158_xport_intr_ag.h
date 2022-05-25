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

#ifndef _BCM63158_XPORT_INTR_AG_H_
#define _BCM63158_XPORT_INTR_AG_H_

//#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_cpu_status;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_cpu_set;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_cpu_clear;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_cpu_mask_status;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_cpu_mask_set;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_cpu_mask_clear;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_pci_status;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_pci_set;

typedef struct
{
    uint8_t mab_status_intr;
    uint8_t rx_remote_fault_intr;
    uint8_t serdes_mod_def0_event_intr;
    uint8_t dserdes_sd_off_intr;
    uint8_t dserdes_sd_on_intr;
    uint8_t link_down_intr;
    uint8_t link_up_intr;
    uint8_t xlmac_intr;
    uint8_t tx_timesync_fifo_entry_valid_intr;
    uint8_t sgphy_energy_off_intr;
    uint8_t sgphy_energy_on_intr;
    uint8_t mib_reg_err_intr;
    uint8_t mac_reg_err_intr;
    uint8_t ubus_err_intr;
} xport_intr_pci_clear;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_pci_mask_status;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_pci_mask_set;

typedef struct
{
    uint8_t mab_status_intr_mask;
    uint8_t rx_remote_fault_intr_mask;
    uint8_t serdes_mod_def0_event_intr_mask;
    uint8_t dserdes_sd_off_intr_mask;
    uint8_t dserdes_sd_on_intr_mask;
    uint8_t link_down_intr_mask;
    uint8_t link_up_intr_mask;
    uint8_t xlmac_intr_mask;
    uint8_t tx_timesync_fifo_entry_valid_intr_mask;
    uint8_t sgphy_energy_off_intr_mask;
    uint8_t sgphy_energy_on_intr_mask;
    uint8_t mib_reg_err_intr_mask;
    uint8_t mac_reg_err_intr_mask;
    uint8_t ubus_err_intr_mask;
} xport_intr_pci_mask_clear;

int ag_drv_xport_intr_cpu_status_set(const xport_intr_cpu_status *cpu_status);
int ag_drv_xport_intr_cpu_status_get(xport_intr_cpu_status *cpu_status);
int ag_drv_xport_intr_cpu_set_set(const xport_intr_cpu_set *cpu_set);
int ag_drv_xport_intr_cpu_set_get(xport_intr_cpu_set *cpu_set);
int ag_drv_xport_intr_cpu_clear_set(const xport_intr_cpu_clear *cpu_clear);
int ag_drv_xport_intr_cpu_clear_get(xport_intr_cpu_clear *cpu_clear);
int ag_drv_xport_intr_cpu_mask_status_set(const xport_intr_cpu_mask_status *cpu_mask_status);
int ag_drv_xport_intr_cpu_mask_status_get(xport_intr_cpu_mask_status *cpu_mask_status);
int ag_drv_xport_intr_cpu_mask_set_set(const xport_intr_cpu_mask_set *cpu_mask_set);
int ag_drv_xport_intr_cpu_mask_set_get(xport_intr_cpu_mask_set *cpu_mask_set);
int ag_drv_xport_intr_cpu_mask_clear_set(const xport_intr_cpu_mask_clear *cpu_mask_clear);
int ag_drv_xport_intr_cpu_mask_clear_get(xport_intr_cpu_mask_clear *cpu_mask_clear);
int ag_drv_xport_intr_pci_status_set(const xport_intr_pci_status *pci_status);
int ag_drv_xport_intr_pci_status_get(xport_intr_pci_status *pci_status);
int ag_drv_xport_intr_pci_set_set(const xport_intr_pci_set *pci_set);
int ag_drv_xport_intr_pci_set_get(xport_intr_pci_set *pci_set);
int ag_drv_xport_intr_pci_clear_set(const xport_intr_pci_clear *pci_clear);
int ag_drv_xport_intr_pci_clear_get(xport_intr_pci_clear *pci_clear);
int ag_drv_xport_intr_pci_mask_status_set(const xport_intr_pci_mask_status *pci_mask_status);
int ag_drv_xport_intr_pci_mask_status_get(xport_intr_pci_mask_status *pci_mask_status);
int ag_drv_xport_intr_pci_mask_set_set(const xport_intr_pci_mask_set *pci_mask_set);
int ag_drv_xport_intr_pci_mask_set_get(xport_intr_pci_mask_set *pci_mask_set);
int ag_drv_xport_intr_pci_mask_clear_set(const xport_intr_pci_mask_clear *pci_mask_clear);
int ag_drv_xport_intr_pci_mask_clear_get(xport_intr_pci_mask_clear *pci_mask_clear);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xport_intr_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

