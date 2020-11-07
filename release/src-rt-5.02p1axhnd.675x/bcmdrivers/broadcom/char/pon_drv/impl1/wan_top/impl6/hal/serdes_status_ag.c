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

#include "drivers_common_ag.h"
#include "serdes_status_ag.h"
bdmf_error_t ag_drv_serdes_status_serdes_status_status_get(serdes_status_serdes_status_status *serdes_status_status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!serdes_status_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, STATUS, reg_status);

    serdes_status_status->pmd_pll0_lock = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_PLL0_LOCK, reg_status);
    serdes_status_status->pmd_rx_lock_0_invert = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_LOCK_0_INVERT, reg_status);
    serdes_status_status->pmd_rx_clk_vld_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_CLK_VLD_0, reg_status);
    serdes_status_status->pmd_tx_clk_vld = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_TX_CLK_VLD, reg_status);
    serdes_status_status->pmd_rx_lock_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_LOCK_0, reg_status);
    serdes_status_status->pmd_energy_detect_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_ENERGY_DETECT_0, reg_status);
    serdes_status_status->pmd_signal_detect_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_SIGNAL_DETECT_0, reg_status);
    serdes_status_status->pmi_lp_acknowledge = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMI_LP_ACKNOWLEDGE, reg_status);
    serdes_status_status->pmi_lp_error = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMI_LP_ERROR, reg_status);
    serdes_status_status->o_laser_burst_en = RU_FIELD_GET(0, SERDES_STATUS, STATUS, O_LASER_BURST_EN, reg_status);
    serdes_status_status->pmd_pll1_lock = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_PLL1_LOCK, reg_status);

    return BDMF_ERR_OK;
}

