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
 *  Created on: 6 בספט׳ 2015
 *      Author: yonatani
 */
#include "serdes_access.h"
#include "bcm6858_lport_srds_ag.h"
#include "bcm6858_drivers_lport_ag.h"
#include "lport_mdio.h"
#include "lport_defs.h"
#ifndef _CFE_
#include <asm/delay.h>
#define UDELAY(_a) udelay(_a)
#include "lport_intr.h"
#include <linux/workqueue.h>
#include <linux/slab.h>

#else
extern void cfe_usleep(int usec);
#define UDELAY(_a) cfe_usleep(_a)
#endif
extern void bcm_gpio_set_data(unsigned int, unsigned int);
merlin_sdk_cb_s merlin_callbacks = {};
#ifndef _CFE_
EXPORT_SYMBOL(merlin_callbacks);
#endif

#define MERLIN_AER_ADDRESS 0xFFDE
#define SERDES_PLL_LOCK_RETRIES 100

#define GPIO_52 			(52)
#define SFP_10G_TX			(GPIO_52)
 
typedef enum
{
    SERDES_ACCESS_MDIO22,
    SERDES_ACCESS_MDIO45,
    SERDES_ACCESS_PMI
} E_SERDES_ACCESS_TYPE;


#pragma pack(push,1)
typedef struct
{
    uint32_t reg_addr:4;
    uint32_t block:12;
    uint32_t lane:11;
    uint32_t devid:5;
}s_bcm_aer;
#pragma pack(pop)

static E_SERDES_ACCESS_TYPE g_serdes_access_type = SERDES_ACCESS_PMI;

//maintain this MDIO addresses via register SERDES_X_CNTRL serdes_prtad field
static uint16_t g_merlin_mdio_address[2] = {6,0xa};

static int write_serdes_pmi(E_MERLIN_ID merlin_id,uint32_t addr,uint16_t mask,uint16_t value)
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

        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(err,
                start_busy, r_w, reg_data);

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(&err, &start_busy,
            &r_w, &reg_data);
    }
    else
    {
        rc = ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(addr);
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_mask_0_set(~mask);

        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(err,
                start_busy, r_w, reg_data);

        //validate no error after writing
        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(&err, &start_busy,
            &r_w, &reg_data);
    }

    if (rc | err | start_busy)
    {
        pr_err("Error while validating write to Serdes %d: Busy=%d,Err=%d,Rc=%d\n",
            merlin_id, start_busy, err, rc);
        return LPORT_ERR_IO;
    }
    return LPORT_ERR_OK;
}

static int read_serdes_pmi(E_MERLIN_ID merlin_id,uint32_t addr,uint16_t mask,uint16_t *value)
{
    int rc;
    uint8_t err = 0;
    uint8_t start_busy = 1;
    uint8_t r_w = 1; /* Means read operation */
    uint16_t reg_data = 0;

    if (!merlin_id)
    {
        rc = ag_drv_lport_srds_serdes_0_indir_acc_addr_0_set(addr);

        rc = rc ? rc : ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_set(err,
                start_busy, r_w, reg_data);

        //validate no error after writing
        rc = rc ? rc:ag_drv_lport_srds_serdes_0_indir_acc_cntrl_0_get(&err, &start_busy,
            &r_w, &reg_data);
    }
    else
    {
        rc = ag_drv_lport_srds_serdes_1_indir_acc_addr_0_set(addr);

        rc = rc ? rc : ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_set(err,
                start_busy, r_w, reg_data);

        //validate no error after writing
        rc = rc ? rc:ag_drv_lport_srds_serdes_1_indir_acc_cntrl_0_get(&err, &start_busy,
            &r_w, &reg_data);
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

static int read_mdio22(E_MERLIN_ID merlin_id,uint16_t addr,uint16_t *value)
{
    s_bcm_aer *aer = (s_bcm_aer *)&addr;
    uint16_t reg_addr;
    uint16_t reg_val;
    int rc;

    //write AER Address
    reg_val = MERLIN_AER_ADDRESS & 0xFFF0;
    reg_addr = 0x1f;
    rc = lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    reg_addr = 0xE;
    reg_val = (aer->devid << 11) | aer->lane;
    rc =rc ? rc : lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    // set up Block Address
    reg_val = aer->reg_addr & 0xfff0;
    reg_addr = 0x1f;
    rc =rc ? rc : lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    //write actual data reg
    reg_addr = 0xE;
    rc =rc ? rc : lport_mdio22_rd(g_merlin_mdio_address[merlin_id], reg_addr, &reg_val);
    *value = reg_val;

    return rc;
}

static uint32_t write_mdio22(E_MERLIN_ID merlin_id,uint32_t addr,uint16_t value)
{
    s_bcm_aer *aer = (s_bcm_aer *)&addr;
    uint16_t reg_addr;
    uint16_t reg_val;
    int rc;

    //write AER Address
    reg_val = MERLIN_AER_ADDRESS & 0xFFF0;
    reg_addr = 0x1f;
    rc = lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    reg_addr = 0xE;
    reg_val = (aer->devid << 11) | aer->lane;
    rc =rc ? rc : lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    // set up Block Address
    reg_val = aer->reg_addr & 0xfff0;
    reg_addr = 0x1f;
    rc =rc ? rc : lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    //write actual data reg
    reg_addr = 0xE;
    rc =rc ? rc : lport_mdio22_wr(g_merlin_mdio_address[merlin_id], reg_addr, reg_val);

    return rc;
}

static int write_serdes_mdio22(E_MERLIN_ID merlin_id,uint16_t addr,uint16_t mask,uint16_t value)
{
    uint16_t reg_val;
    int rc;

    rc = read_mdio22(merlin_id,addr,&reg_val);

    reg_val = (reg_val & ~mask) | (value & mask);

    rc = rc ? rc : write_mdio22(merlin_id,addr,reg_val);

    return rc;
}

static uint32_t read_serdes_mdio22(E_MERLIN_ID merlin_id,uint16_t addr,uint16_t mask,uint16_t *value)
{
    uint16_t reg_val;
    int rc;

    rc = read_mdio22(merlin_id,addr,&reg_val);

    *value = (reg_val & ~mask);

    return rc;
}


/*
   static int read_serdes_mdio45(E_MERLIN_ID merlin_id,uint16_t addr,uint16_t mask,uint16_t *value)
   {
   return LPORT_ERR_OK;
   }

   static int write_serdes_mdio45(E_MERLIN_ID merlin_id,uint16_t addr,uint16_t mask,uint16_t value)
   {
   return LPORT_ERR_OK;
   }

   static int write_mdio45(E_MERLIN_ID merlin_id,uint32_t addr,uint16_t mask,uint16_t value)
   {
   return write_serdes_mdio45;
   }

   static int read_mdio45(E_MERLIN_ID merlin_id,uint32_t addr,uint16_t mask,uint16_t *value)
   {
   return LPORT_ERR_OK;
   }
   */

int write_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr,uint16_t mask,uint16_t value)
{
    switch(g_serdes_access_type)
    {
    case SERDES_ACCESS_PMI:
        return write_serdes_pmi(merlin_id,addr,mask,value);
    case SERDES_ACCESS_MDIO22:
        return write_serdes_mdio22(merlin_id,addr,mask,value);
    case SERDES_ACCESS_MDIO45:
        //        return write_serdes_mdio45(merlin_id,addr,mask,value);
        return LPORT_ERR_RANGE;
    }

    return LPORT_ERR_RANGE;
}

int read_serdes_reg(E_MERLIN_ID merlin_id, uint32_t addr, uint16_t mask, uint16_t *value)
{
    switch(g_serdes_access_type)
    {
    case SERDES_ACCESS_PMI:
        return read_serdes_pmi(merlin_id,addr,mask,value);
    case SERDES_ACCESS_MDIO22:
        return read_serdes_mdio22(merlin_id,addr,mask,value);
    case SERDES_ACCESS_MDIO45:
        //        return read_serdes_mdio45(merlin_id,addr,mask,value);
        return LPORT_ERR_RANGE;
    }

    return LPORT_ERR_RANGE;
}

int write_gpio_52_state(uint32_t state)
{
    bcm_gpio_set_data(SFP_10G_TX, state);

    return 0;
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

int merlin_init_core(E_MERLIN_ID core_id, E_MERLIN_VCO vco)
{
    lport_srds_dual_serdes_0_cntrl serdes0_cntrl;
    lport_srds_dual_serdes_1_cntrl serdes1_cntrl;

    /* Check Merlin Driver registration */
    if (!merlin_callbacks.merlin_init)
    {
        pr_err("Merlin driver not registered, Failed to init SERDES \n");
        return LPORT_ERR_STATE;
    }

    /* check if we need to reset core */
    if (core_id == MERLIN_ID_0)
    {
        ag_drv_lport_srds_dual_serdes_0_cntrl_get(&serdes0_cntrl);
        serdes0_cntrl.iddq = 0;
        /* wait at least 150ns */
        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
        UDELAY(10000);

        serdes0_cntrl.serdes_reset = 0;
        serdes0_cntrl.refclk_reset = 0;

        ag_drv_lport_srds_dual_serdes_0_cntrl_set(&serdes0_cntrl);
    }
    else
    {
        ag_drv_lport_srds_dual_serdes_1_cntrl_get(&serdes1_cntrl);
        serdes1_cntrl.iddq = 0;
        /* wait at least 150ns */
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
        UDELAY(10000);

        serdes1_cntrl.serdes_reset = 0;
        serdes1_cntrl.refclk_reset = 0;
        ag_drv_lport_srds_dual_serdes_1_cntrl_set(&serdes1_cntrl);
    } 
    pr_info("SERDES_%d IDQ Deasserted!\n", core_id);

    if (merlin_callbacks.merlin_init(core_id, vco))
    {
        pr_err("%s(%d):merlin_init core = %d failed\n", __FUNCTION__,
            __LINE__, core_id);
        return LPORT_ERR_IO;
    }

    return LPORT_ERR_OK;
}

#ifndef _CFE_

typedef struct
{
    struct work_struct base_work;
    lport_intr_info_s *info;
} serdes_work_t;

static void serdes_work_cb(struct work_struct *work)
{
    serdes_work_t *serdes_work = container_of(work, serdes_work_t, base_work);
    lport_intr_info_s *info = serdes_work->info;
    uint32_t port = info->entity_id;

    bcm_gpio_set_data(52, 0);
    lport_intr_enable(LPORT_REMOTE_FAULT, port, 1);

    kfree(serdes_work);
}

static int serdes_queue_work(lport_intr_info_s *info)
{
    serdes_work_t *serdes_work = kmalloc(sizeof(serdes_work_t), GFP_ATOMIC);
    if (!serdes_work)
    {
        printk("serdes_queue_work: kmalloc failed to allocate work struct\n");
        return -1;
    }

    INIT_WORK(&serdes_work->base_work, serdes_work_cb);
    serdes_work->info = info;

    queue_work(system_unbound_wq, &serdes_work->base_work);

    return 0;
}

static void remote_fault_handler(const lport_intr_info_s *info, void *priv)
{
    uint32_t port = info->entity_id;

    lport_intr_enable(LPORT_REMOTE_FAULT, port, 0);
    lport_intr_clear(LPORT_REMOTE_FAULT, port);

    /* Toggle TX disable GPIO in order to restart TX lazer */
    bcm_gpio_set_data(52, 1);

    /* Schedule the TX disable GPIO Set to 0 and enable interrupt back */
    if (serdes_queue_work(info))
    {
        /* Enable interuupt back in case of failure */
    
        lport_intr_enable(LPORT_REMOTE_FAULT, port, 1);
    }
}
#endif

/* lport_serdes_init is the function that initialized Serdes on both
 * LPORT block and calls Merlin SDK initialization, we call the init per
 * Serdes CORE, thus we have to know which port can be muxed to which Serdes.
 * Ports 0,7 maps to Merlin Core 0 Lane 0
 * Ports 1,4 maps to Merlin Core 0 Lane 1
 * Ports 2,5 maps to Merlin Core 1 Lane 0
 * Ports 3,6 maps to Merlin Core 1 Lane 1
 *
 */

static E_MERLIN_LANE port_2_lane_idx[] = {
    MERLIN_LANE_0,
    MERLIN_LANE_1,
    MERLIN_LANE_2,
    MERLIN_LANE_3,
    MERLIN_LANE_1,
    MERLIN_LANE_2,
    MERLIN_LANE_3,
    MERLIN_LANE_0
};

int lport_serdes_init(lport_init_s *init_params)
{
    uint32_t i;
    int rc;
    int     core_used[2] = {};
    lport_serdes_cfg_s serdes_cfg;
    E_MERLIN_VCO vco;
#ifndef _CFE_
    int register_int = 0;
#endif

    for (i = 0 ; i < LPORT_NUM_OF_PORTS; i++ )
    {
        if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[i]))
        {
            if(!core_used[SERDES_LANE_TO_CORE(port_2_lane_idx[i])])
            {
                if (init_params->prt_mux_sel[i] == PORT_HSGMII)
                    vco = MERLIN_VCO_9375_MHZ;
                else
                    vco = MERLIN_VCO_103125_MHZ;

                rc = merlin_init_core(SERDES_LANE_TO_CORE(port_2_lane_idx[i]), vco);
                if (rc)
                {
                    pr_err("Init of merline core for port %d failed\n", i);
                    return LPORT_ERR_STATE;
                }

                core_used[SERDES_LANE_TO_CORE(port_2_lane_idx[i])] = 1;
            }
            serdes_cfg.prt_mux_sel = init_params->prt_mux_sel[i];
            serdes_cfg.autoneg_mode = MERLIN_AN_NONE;
            switch(serdes_cfg.prt_mux_sel)
            {
            case PORT_SGMII_1000BASE_X:
                serdes_cfg.autoneg_mode = MERLIN_AN_IEEE_CL37;
                /*no break*/
            case PORT_SGMII_SLAVE:
                serdes_cfg.speed = LPORT_RATE_1000MB;
                break;
            case PORT_HSGMII:
                serdes_cfg.speed = LPORT_RATE_2500MB;
                break;
            case PORT_XFI:
                serdes_cfg.speed = LPORT_RATE_10G;
#ifndef _CFE_
                register_int = 1;
#endif
                break;
            default:
                pr_err("%s(%d)Wrong port type %d for port %d\n",__FUNCTION__, __LINE__, serdes_cfg.prt_mux_sel, i);
                return LPORT_ERR_PARAM;
            }
            merlin_callbacks.merlin_set_cfg(port_2_lane_idx[i], &serdes_cfg);
        }
    }

    for (i = 0; i < 2; i++)  
    {
        if (core_used[i])
        {
            merlin_callbacks.merlin_post_init(i);
            UDELAY(10000);
            if (serdes_get_pll_lock(i))
            {
                return LPORT_ERR_IO;
            }
            pr_info("SERDES_%d Merlin PLL Locked!\n", i);

#ifndef _CFE_
            if (register_int)
            {
                lport_intr_register(LPORT_REMOTE_FAULT, i, remote_fault_handler, NULL);
                lport_intr_enable(LPORT_REMOTE_FAULT, i, 1);
            }
#endif
        }
    }

    for (i = 0 ; i < LPORT_NUM_OF_PORTS; i++ )
    {
        if (LPORT_IS_SERDES_PORT(init_params->prt_mux_sel[i]))
        {
            merlin_callbacks.merlin_datapath_reset(port_2_lane_idx[i]);
        }
    }

    return LPORT_ERR_OK;
}

int lport_serdes_get_status(uint32_t port, lport_port_status_s *port_status)
{
    return merlin_callbacks.merlin_get_status(port_2_lane_idx[port], port_status);
}

int lport_serdes_change_speed(uint32_t port, LPORT_PORT_RATE rate)
{
    return merlin_callbacks.merlin_change_speed(port_2_lane_idx[port], rate);
}

int lport_serdes_set_loopback(uint32_t port, uint32_t loopback_mode, uint32_t enable)
{
    merlin_control_t control = {};
    control.loopback.mode = loopback_mode;
    control.loopback.enable = enable;
    return merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_LOOPBACK_SET, &control);
}

int lport_serdes_diag(uint32_t port, uint32_t cmd)
{
    int rc;
    merlin_control_t control = {};
    merlin_command_t merlin_cmd = cmd == 1 ? MERLIN_CMD_STATUS_GET : cmd == 2 ? MERLIN_CMD_STATS_GET : 0;

    rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], merlin_cmd, &control);

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

     rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_PRBS_GENRATE, &control);
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

    rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_PRBS_MONITOR, &control);
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
    rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_PRBS_STATS_GET, &control);
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

    rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_REG_READ, &control);

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

    rc = merlin_callbacks.merlin_ioctl(port_2_lane_idx[port], MERLIN_CMD_REG_WRITE, &control);

    return rc;
}
