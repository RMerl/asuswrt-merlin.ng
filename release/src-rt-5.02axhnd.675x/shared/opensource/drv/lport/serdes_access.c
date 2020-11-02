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
/*
 * serdes_access.c
 *
 *  Created on: April 2015
 *      Author: yonatani
 */

#include "serdes_access.h"
#include "bcm6858_lport_srds_ag.h"
#include "bcm6858_drivers_lport_ag.h"
#include "lport_mdio.h"
#include "lport_defs.h"
#include "boardparms.h"
#include "bcm_gpio.h"
#ifndef _CFE_
#include <asm/delay.h>
#define UDELAY(_a) udelay(_a)
#include "lport_intr.h"
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/delay.h>
#else
extern void cfe_usleep(int usec);
#define UDELAY(_a) cfe_usleep(_a)
#endif

merlin_sdk_cb_s merlin_callbacks = {};
#ifndef _CFE_
EXPORT_SYMBOL(merlin_callbacks);
#endif
#if !defined(_CFE_) && !defined(G9991)
#define REMOTE_FAULT_HANDLER
#endif

#define MERLIN_AER_ADDRESS 0xFFDE
#define SERDES_PLL_LOCK_RETRIES 100
#define PMI_DELAY UDELAY(10)

static uint16_t g_merlin_mdio_address[2] = {0x8, 0x9};

static int write_serdes_pmi(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask, uint16_t value)
{
    int rc;
    uint8_t err = 0;
    uint8_t start_busy = 1;
    uint8_t r_w = 0; /* Means write operation */
    uint16_t reg_data = value;

    if (!merlin_id)
    {
        rc = ag_drv_lport_srds_serdes_0_indir_acc_addr_0_set(addr);
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_mask_0_set(~mask);
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(err, start_busy, r_w, reg_data);

        PMI_DELAY;

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(&err, &start_busy, &r_w, &reg_data);
    }
    else
    {
        rc = ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(addr);
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_mask_0_set(~mask);
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(err, start_busy, r_w, reg_data);

        PMI_DELAY;

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(&err, &start_busy, &r_w, &reg_data);
    }

    if (rc | err | start_busy)
    {
        pr_err("Error while validating write to Serdes %d: Busy=%d,Err=%d,Rc=%d\n",
            merlin_id, start_busy, err, rc);
        return LPORT_ERR_IO;
    }
    return LPORT_ERR_OK;
}

static int read_serdes_pmi(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask, uint16_t *value)
{
    int rc;
    uint8_t err = 0;
    uint8_t start_busy = 1;
    uint8_t r_w = 1; /* Means read operation */
    uint16_t reg_data = 0;

    if (!merlin_id)
    {
        rc = ag_drv_lport_srds_serdes_0_indir_acc_addr_0_set(addr);
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(err, start_busy, r_w, reg_data);

        PMI_DELAY;

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(&err, &start_busy, &r_w, &reg_data);
    }
    else
    {
        rc = ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(addr);
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(err, start_busy, r_w, reg_data);

        PMI_DELAY;

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(&err, &start_busy, &r_w, &reg_data);
    }
    if (rc | err | start_busy)
    {
        pr_err("Error while validating write to Serdes %d: Busy=%d,Err=%d,Rc=%d\n",
            merlin_id, start_busy, err, rc);
        return LPORT_ERR_IO;
    }
    *value = reg_data & mask;
    return LPORT_ERR_OK;
}

int write_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr,uint16_t mask,uint16_t value)
{
    return write_serdes_pmi(merlin_id,addr,mask,value);
}

int read_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask, uint16_t *value)
{
    return read_serdes_pmi(merlin_id,addr,mask,value);
}

int serdes_get_pll_lock(E_MERLIN_ID core_id)
{
    lport_srds_dual_serdes_0_status serdes0_status;
    lport_srds_dual_serdes_1_status serdes1_status;
    uint32_t pll_lock_retry = SERDES_PLL_LOCK_RETRIES;

    if (core_id == MERLIN_ID_0)
    {
        do
        {
            UDELAY(10000);
            ag_drv_lport_srds_dual_serdes_0_status_get(&serdes0_status);
        }while(!serdes0_status.pll_lock && --pll_lock_retry);

        if(!serdes0_status.pll_lock)
        {
            pr_err("%s(%d):Failed to LOCK PLL of Serdes 0\n",__FUNCTION__,__LINE__);
            return LPORT_ERR_IO;
        }
    }
    else
    {
        /* Poll PLL */
        do
        {
            UDELAY(10000);
            ag_drv_lport_srds_dual_serdes_1_status_get(&serdes1_status);
        }while(!serdes1_status.pll_lock && --pll_lock_retry);

        if(!serdes1_status.pll_lock)
        {
            pr_err("%s(%d):Failed to LOCK PLL of Serdes 1\n",__FUNCTION__,__LINE__);
            return LPORT_ERR_IO;
        }
    }

    return LPORT_ERR_OK;
}

static E_MERLIN_VCO core_vco[2] = {};

static int merlin_core_init(E_MERLIN_ID core_id, E_MERLIN_VCO vco)
{
    int disable;
    lport_srds_dual_serdes_0_cntrl serdes0_cntrl;
    lport_srds_dual_serdes_1_cntrl serdes1_cntrl;

    if (core_vco[core_id] == vco)
        return LPORT_ERR_OK;

    disable = vco == MERLIN_VCO_UNKNOWN;

#ifndef LPORT_SERDES_POWER_SAVING
    if (disable)
        return LPORT_ERR_OK;
#endif

    /* check if we need to reset core */
    if (core_id == MERLIN_ID_0)
    {
        ag_drv_lport_srds_dual_serdes_0_cntrl_get(&serdes0_cntrl);

        serdes0_cntrl.serdes_reset = 1;
        serdes0_cntrl.refclk_reset = 1;
        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
        UDELAY(10000);

        serdes0_cntrl.iddq = disable;
        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
        UDELAY(10000);

        serdes0_cntrl.serdes_reset = disable;
        serdes0_cntrl.refclk_reset = disable;
        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
        UDELAY(10000);

        serdes0_cntrl.serdes_prtad = g_merlin_mdio_address[MERLIN_ID_0];
        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
        UDELAY(10000);
    }
    else
    {
        ag_drv_lport_srds_dual_serdes_1_cntrl_get(&serdes1_cntrl);

        serdes1_cntrl.serdes_reset = 1;
        serdes1_cntrl.refclk_reset = 1;
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
        UDELAY(10000);

        serdes1_cntrl.iddq = disable;
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
        UDELAY(10000);

        serdes1_cntrl.serdes_reset = disable;
        serdes1_cntrl.refclk_reset = disable;
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
        UDELAY(10000);

        serdes1_cntrl.serdes_prtad = g_merlin_mdio_address[MERLIN_ID_1];
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
        UDELAY(10000);
    } 

    if (disable)
    {
        core_vco[core_id] = MERLIN_VCO_UNKNOWN;
        return LPORT_ERR_OK;
    }

    /* Check Merlin Driver registration */
    if (!merlin_callbacks.merlin_core_init)
    {
        pr_err("Merlin driver not registered, Failed to init SERDES \n");
        return LPORT_ERR_STATE;
    }

    if (merlin_callbacks.merlin_core_init(core_id, vco))
    {
        pr_err("%s(%d):merlin_core_init merlin_id=%d failed\n", __FUNCTION__,
            __LINE__, core_id);
        return LPORT_ERR_IO;
    }

    if (serdes_get_pll_lock(core_id))
        return LPORT_ERR_IO;

    core_vco[core_id] = vco;

    return LPORT_ERR_OK;
}

static unsigned short tx_dis_gpio[4];

static void tx_gpio_enable(E_MERLIN_LANE lane_id, uint32_t enable)
{
    unsigned int gpio = tx_dis_gpio[lane_id];

    if (gpio == BP_NOT_DEFINED)
        return;

    bcm_gpio_set_data(gpio, enable ? 0 : 1);
}

#ifdef REMOTE_FAULT_HANDLER
typedef struct
{
    struct work_struct base_work;
    const lport_intr_info_s *info;
} serdes_work_t;

static void serdes_work_cb(struct work_struct *work)
{
    serdes_work_t *serdes_work = container_of(work, serdes_work_t, base_work);
    uint32_t lane_id = serdes_work->info->entity_id;
    lport_srds_dual_serdes_0_cntrl serdes0_cntrl;
    merlin_status_t status = {};

    kfree(serdes_work);
    msleep(1000);

    ag_drv_lport_srds_dual_serdes_0_cntrl_get(&serdes0_cntrl);

    if (serdes0_cntrl.serdes_reset == 1)
        return;

    if (merlin_callbacks.merlin_ioctl(lane_id, MERLIN_CMD_STATUS_GET, (merlin_control_t *)&status))
        return;

    if (status.rx_REMOTE_FAULT)
    {
        tx_gpio_enable(lane_id, 0);
        tx_gpio_enable(lane_id, 1);
        msleep(1000);
    }

    lport_intr_clear(LPORT_REMOTE_FAULT, lane_id);
    lport_intr_enable(LPORT_REMOTE_FAULT, lane_id, 1);
}

static int serdes_queue_work(const lport_intr_info_s *info)
{
    serdes_work_t *serdes_work = kmalloc(sizeof(serdes_work_t), GFP_ATOMIC);
    if (!serdes_work)
    {
        pr_err("serdes_queue_work: kmalloc failed to allocate work struct\n");
        return -1;
    }

    INIT_WORK(&serdes_work->base_work, serdes_work_cb);
    serdes_work->info = info;

    queue_work(system_unbound_wq, &serdes_work->base_work);

    return 0;
}

static void remote_fault_handler(const lport_intr_info_s *info, void *priv)
{
    uint32_t lane_id = info->entity_id;

    lport_intr_enable(LPORT_REMOTE_FAULT, lane_id, 0);
    lport_intr_clear(LPORT_REMOTE_FAULT, lane_id);

    /* Schedule a work queue to restart the TX laser */
    if (serdes_queue_work(info))
    {
        /* Enable interuupt back in case of failure */
        lport_intr_enable(LPORT_REMOTE_FAULT, lane_id, 1);
    }
}

static void remote_fault_enable(E_MERLIN_LANE lane_id, int enable)
{
    if (!(lane_id == MERLIN_LANE_0 || lane_id == MERLIN_LANE_1))
        return;

    if (enable)
    {
        lport_intr_register(LPORT_REMOTE_FAULT, lane_id, remote_fault_handler, NULL);
        lport_intr_clear(LPORT_REMOTE_FAULT, lane_id);
        lport_intr_enable(LPORT_REMOTE_FAULT, lane_id, 1);
    }
    else
    {
        lport_intr_enable(LPORT_REMOTE_FAULT, lane_id, 0);
        lport_intr_clear(LPORT_REMOTE_FAULT, lane_id);
        lport_intr_unregister(LPORT_REMOTE_FAULT, lane_id);
    }
}
#endif

static void lane_tx_enable(E_MERLIN_LANE lane_id, int enable)
{
    if (enable)
    {
        tx_gpio_enable(lane_id, 1);
#ifdef REMOTE_FAULT_HANDLER
        remote_fault_enable(lane_id, 1);
#endif
    }
    else
    {
#ifdef REMOTE_FAULT_HANDLER
        remote_fault_enable(lane_id, 0);
#endif
        tx_gpio_enable(lane_id, 0);
    }
}

/* Ports 0,7 maps to Merlin Core 0 Lane 0
 * Ports 1,4 maps to Merlin Core 0 Lane 1
 * Ports 2,5 maps to Merlin Core 1 Lane 0
 * Ports 3,6 maps to Merlin Core 1 Lane 1
 *
 */

static E_MERLIN_LANE port_to_lane[] = {
    MERLIN_LANE_0,
    MERLIN_LANE_1,
    MERLIN_LANE_2,
    MERLIN_LANE_3,
    MERLIN_LANE_1,
    MERLIN_LANE_2,
    MERLIN_LANE_3,
    MERLIN_LANE_0
};

static E_MERLIN_ID port_to_core[] = {
    MERLIN_ID_0,
    MERLIN_ID_0,
    MERLIN_ID_1,
    MERLIN_ID_1,
    MERLIN_ID_0,
    MERLIN_ID_1,
    MERLIN_ID_1,
    MERLIN_ID_0,
};

static E_MERLIN_ID lane_to_core[] = {
    MERLIN_ID_0,
    MERLIN_ID_0,
    MERLIN_ID_1,
    MERLIN_ID_1,
};

static LPORT_PORT_MUX_SELECT lane_mux[4] = {};

static int merlin_lane_init(E_MERLIN_LANE lane_id, LPORT_PORT_MUX_SELECT prt_mux_sel)
{
    E_MERLIN_ID core = lane_to_core[lane_id];

    if (lane_mux[lane_id] == prt_mux_sel)
        return LPORT_ERR_OK;

    if (core_vco[core] == MERLIN_VCO_UNKNOWN)
    {
        lane_mux[lane_id] = PORT_UNAVAIL;
        return LPORT_ERR_OK;
    }

    /* Check Merlin Driver registration */
    if (!merlin_callbacks.merlin_lane_init)
    {
        pr_err("Merlin driver not registered, Failed to init SERDES \n");
        return LPORT_ERR_STATE;
    }

    if (merlin_callbacks.merlin_lane_init(lane_id, prt_mux_sel))
    {
        pr_err("%s(%d):merlin_lane_init lane_id=%d failed\n", __FUNCTION__,
            __LINE__, lane_id);
        return LPORT_ERR_IO;
    }

    lane_mux[lane_id] = prt_mux_sel;

    lane_tx_enable(lane_id, prt_mux_sel != PORT_UNAVAIL);

    return LPORT_ERR_OK;
}

int port_write_tx_dis_state(uint32_t port, uint32_t state)
{
    E_MERLIN_LANE lane_id = port_to_lane[port];

    lane_tx_enable(lane_id, state == 0);

    return 0;
}

static void bp_init(void)
{
    static int bp_initialized = 0;

    if (bp_initialized)
        return;

    BpGetTxDisGpio(MERLIN_LANE_0, &tx_dis_gpio[0]);
    BpGetTxDisGpio(MERLIN_LANE_1, &tx_dis_gpio[1]);
    BpGetTxDisGpio(MERLIN_LANE_2, &tx_dis_gpio[2]);
    BpGetTxDisGpio(MERLIN_LANE_3, &tx_dis_gpio[3]);

    bp_initialized = 1;
}

int lport_serdes_init(lport_init_s *init_params)
{
    uint32_t i;
    E_MERLIN_VCO vco[2] = {};
    LPORT_PORT_MUX_SELECT mux[4] = {};

    bp_init();

    for (i = 0; i < LPORT_NUM_OF_PORTS; i++)
    {
        LPORT_PORT_MUX_SELECT prt_mux_sel = init_params->prt_mux_sel[i];

        if (LPORT_IS_SERDES_PORT(prt_mux_sel))
        {
            E_MERLIN_LANE lane = port_to_lane[i];
            E_MERLIN_ID core = port_to_core[i];

            mux[lane] = prt_mux_sel;
            vco[core] = MERLIN_VCO_103125_MHZ;
        }
    }

    for (i = 0; i < 2; i++) /* All cores: 0, 1 */ 
    {
        merlin_core_init(i, vco[i]);
    }

    for (i = 0; i < 4; i++) /* All lanes: 0, 1, 2, 3 */
    {
        merlin_lane_init(i, mux[i]);
    }

    return LPORT_ERR_OK;
}

int lport_serdes_get_status(uint32_t port, lport_port_status_s *port_status)
{
    return merlin_callbacks.merlin_get_status(port_to_lane[port], port_status);
}

int lport_serdes_speed_set(uint32_t port, LPORT_PORT_RATE rate)
{
    return merlin_callbacks.merlin_speed_set(port_to_lane[port], rate);
}

int lport_serdes_set_loopback(uint32_t port, uint32_t loopback_mode, uint32_t enable)
{
    merlin_control_t control = {};
    control.loopback.mode = loopback_mode;
    control.loopback.enable = enable;
    return merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_LOOPBACK_SET, &control);
}

int lport_serdes_set_tx_cfg(uint32_t port, merlin_tx_cfg_t *tx_cfg)
{
    int rc;
    merlin_control_t control = {};

    control.tx_cfg = *tx_cfg;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_TXCFG_SET, &control);
    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

exit:    
    return rc;
}

int lport_serdes_get_tx_cfg(uint32_t port, merlin_tx_cfg_t *tx_cfg)
{
    int rc;
    merlin_control_t control = {};

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_TXCFG_GET, &control);
    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

    *tx_cfg = control.tx_cfg;

exit:    
    return rc;
}

int lport_serdes_diag(uint32_t port, uint32_t cmd)
{
    int rc;
    merlin_control_t control = {};
    merlin_command_t merlin_cmd = cmd == 1 ? MERLIN_CMD_STATUS_GET : cmd == 2 ? MERLIN_CMD_STATS_GET : 0;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], merlin_cmd, &control);

    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

    switch(merlin_cmd)
    {
    case MERLIN_CMD_STATUS_GET:
        pr_info("tx_LOCAL_FAULT: %d\n", control.status.tx_LOCAL_FAULT);
        pr_info("tx_REMOTE_FAULT: %d\n", control.status.tx_REMOTE_FAULT);
        pr_info("PMD_LOCK: %d\n", control.status.PMD_LOCK);
        pr_info("signal_ok: %d\n", control.status.signal_ok);
        pr_info("rx_LOCAL_FAULT: %d\n", control.status.rx_LOCAL_FAULT);
        pr_info("rx_REMOTE_FAULT: %d\n", control.status.rx_REMOTE_FAULT);
        pr_info("rx_LINK_STATUS: %d\n", control.status.rx_LINK_STATUS);
        pr_info("rx_SYNC_STATUS: %d\n", control.status.rx_SYNC_STATUS);
        pr_info("pll_lock: %d\n", control.status.pll_lock);
        break;
    case MERLIN_CMD_STATS_GET:
        pr_info("\nkcode66ErrCount: %d\n", control.stats.kcode66ErrCount);
        pr_info("sync66ErrCount: %d\n", control.stats.sync66ErrCount);
        pr_info("cl49ieee_errored_blocks: %d\n", control.stats.cl49ieee_errored_blocks);
        pr_info("BER_count_per_ln: %d\n", control.stats.BER_count_per_ln);
        pr_info("cl49_valid_sh_cnt: %d\n", control.stats.cl49_valid_sh_cnt);
        pr_info("cl49_invalid_sh_cnt: %d\n", control.stats.cl49_invalid_sh_cnt);
        break;
    default:
        pr_info("Not implemented yet\n");
        break;
    } 
exit:
    return rc;
}


int lport_serdes_prbs_generation(uint32_t port, E_SERDES_PRBS_PATTERN_TYPE pattern_type)
{
    int rc = 0;
    merlin_control_t control = {};

    control.prbs_type = (uint16_t)pattern_type;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_PRBS_GENRATE, &control);
    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

exit:
    return rc;
}

int lport_serdes_prbs_monitor(uint32_t port, E_SERDES_PRBS_PATTERN_TYPE pattern_type)
{
    int rc = 0;
    merlin_control_t control = {};

    control.prbs_type = (uint16_t)pattern_type;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_PRBS_MONITOR, &control);
    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

exit:
    return rc;
}

int lport_serdes_prbs_stats_get(uint32_t port)
{
    int rc;
    merlin_control_t control = {};
    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_PRBS_STATS_GET, &control);
    if (rc)
    {
        pr_err("Failed to get merlin command %d \n", rc);
        goto exit;
    }

    pr_info("prbs_lock_status:0x%x\n", control.prbs_stats.prbs_lock_status);
    pr_info("prbs_lock_lost:0x%x\n", control.prbs_stats.prbs_lock_lost);
    pr_info("prbs_error:    0x%x\n", control.prbs_stats.prbs_error);

exit:
    return rc;
}

int lport_serdes_read_reg(uint32_t port, uint16_t device_type, uint16_t reg_address)
{
    int rc;
    merlin_control_t control = {};

    control.reg_data.device_type = device_type;
    control.reg_data.reg_addr = reg_address;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_REG_READ, &control);

    if (!rc)
        pr_info("\n 0x%04X: 0x%04X\n", reg_address, control.reg_data.reg_value);

    return rc;
}

int lport_serdes_write_reg(uint32_t port, uint16_t device_type, uint16_t reg_address, uint16_t value)
{
    int rc;
    merlin_control_t control = {};

    control.reg_data.device_type = device_type;
    control.reg_data.reg_addr = reg_address;
    control.reg_data.reg_value = value;

    rc = merlin_callbacks.merlin_ioctl(port_to_lane[port], MERLIN_CMD_REG_WRITE, &control);

    return rc;
}
