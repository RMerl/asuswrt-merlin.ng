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
 * lport_mdio.c
 *
 *  Created on: 24 Sep 2015
 *      Author: yonatani
 */
#ifndef _CFE_
#include <asm/delay.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include "lport_intr.h"
#define UDELAY udelay
#else
#include "lib_types.h"
#include "lib_printf.h"
#include "cfe_timer.h"
#define UDELAY(_a) cfe_usleep(_a)
#endif
#include "bcm6858_drivers_lport_ag.h"
#include "lport_defs.h"
#include "bcm6858_lport_mdio_ag.h"
#include "lport_mdio.h"

typedef enum
{
    MDIO_CL45 = 0,
    MDIO_CL22,
} E_MDIO_CL_TYPE;

typedef enum
{
    MDIO_CL45_ADDRESS =0,
    MDIO_WRITE,
    MDIO_CL45_RDINC,
    MDIO_CL22_RD = MDIO_CL45_RDINC,
    MDIO_CL45_RD,
} E_MDIO_OPCODE;

#define LPORT_ACCESS_MDIO_TRANSACTION_RETRY 1000
#define LPORT_ACCESS_MDIO_TRANSACTION_TIMEOUT_MS 1000

#if LPORT_MDIO_TRANS_LOCKED
#include <linux/mutex.h>
#define MDIO_CHECK_CNTX_LOCK do{ \
    if(in_interrupt()) BUG(); \
    mutex_lock(&mdio_mtx); \
}while(0);

#define MDIO_CNTX_UNLOCK mutex_unlock(&mdio_mtx)
DEFINE_MUTEX(mdio_mtx);
#else
#define MDIO_CHECK_CNTX_LOCK
#define MDIO_CNTX_UNLOCK
#endif

#if LPORT_MDIO_TRANS_INT
DECLARE_COMPLETION(mdio_done);
static uint32_t mdio_timeout;
#endif

#if LPORT_MDIO_TRANS_INT
static void mdio_trans_done_isr(const lport_intr_info_s *info, void *priv)
{
    struct completion *mdio_done_isr = (struct completion *)priv;
    lport_mdio_intr_enable(LPORT_MDIO_DONE,0);
    lport_mdio_intr_clear(LPORT_MDIO_DONE);
    complete(mdio_done_isr);
}

static uint32_t mdio_set_and_wait(lport_mdio_control *control)
{
    int rc = LPORT_ERR_OK;

    //First Arm the mdio interrupt
    lport_mdio_intr_clear(LPORT_MDIO_DONE);
    lport_mdio_intr_enable(LPORT_MDIO_DONE,1);

    ag_drv_lport_mdio_control_set(control);

    if (!wait_for_completion_interruptible_timeout(&mdio_done,mdio_timeout))
    {
        rc =  LPORT_ERR_IO;
        goto done;
    }

    rc = ag_drv_lport_mdio_control_get(control);
    if (rc || control->fail || control->start_busy)
    {
        pr_err("%s(%d):LPORT MDIO interface is not ready rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, rc, control->fail, control->start_busy);
        rc =  LPORT_ERR_IO;
    }

done:
    return rc;
}

#else
static uint32_t mdio_set_and_wait(lport_mdio_control *control)
{
    int rc = LPORT_ERR_OK;
    uint32_t retries = LPORT_ACCESS_MDIO_TRANSACTION_RETRY;

    rc = ag_drv_lport_mdio_control_set(control);
    if (rc)
    {
        pr_err("%s(%d):LPORT MDIO interface is not ready, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, control->fail, control->start_busy);
        return LPORT_ERR_IO;

    }

    /* Poll for transaction end & not error */
    do
    {
        UDELAY(10);
        rc = ag_drv_lport_mdio_control_get(control);
    }while ((!rc) && retries-- && control->start_busy && (!control->fail));

    if (!retries)
    {
        pr_err(
            "%s(%d):LPORT MDIO write transaction passed too many retries rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control->fail, control->start_busy);
        return LPORT_ERR_IO;
    }
    else if (control->fail || control->start_busy || rc)
    {
        pr_err(
            "%s(%d):LPORT MDIO write transaction failed or busy rc=%d,fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control->fail, control->start_busy);
        return LPORT_ERR_IO;
    }
    return rc;
}
#endif

int lport_mdio_bus_init(void)
{
    int rc = LPORT_ERR_OK;
    uint8_t free_run_clk_enable;
    uint8_t supress_preamble;
    uint8_t mdio_clk_divider;
    uint8_t mdio_clause;

#if LPORT_MDIO_TRANS_LOCKED
    mutex_init(&mdio_mtx);
#endif
#if LPORT_MDIO_TRANS_INT
    init_completion(&mdio_done);
    lport_mdio_intr_register(LPORT_MDIO_DONE,mdio_trans_done_isr, &mdio_done);
    mdio_timeout = msecs_to_jiffies(LPORT_ACCESS_MDIO_TRANSACTION_TIMEOUT_MS);
    pr_debug ("LPORT MDIO on Interrupt mode\n");
#endif

    rc = ag_drv_lport_mdio_cfg_get(&free_run_clk_enable,&supress_preamble,&mdio_clk_divider,&mdio_clause);
    /*
     * Reference clock (typically 250 MHz) is divided by 2x(mdio_clk_divider+1) to generate MDIO clock(MDC),
     * For example: 0 = 125MHz, 9 = 12.5MHz,  12 = 9.6MHz, 63 = 1.95MHz, 127 = 0.98MHz, 255 = 0.49MHz
     */
    mdio_clk_divider = 12;
    free_run_clk_enable = 1;
    rc = rc ? rc: ag_drv_lport_mdio_cfg_set(free_run_clk_enable,supress_preamble,mdio_clk_divider,mdio_clause);

    return rc;
}

int lport_mdio22_wr(uint16_t phyid,uint16_t addr, uint16_t data)
{
    uint8_t free_run_clk_enable;
    uint8_t supress_preamble;
    uint8_t mdio_clk_divider;
    uint8_t mdio_clause;
    lport_mdio_control control;
    int rc = LPORT_ERR_OK;

    MDIO_CHECK_CNTX_LOCK;

    rc = ag_drv_lport_mdio_control_get(&control);
    if (rc || control.start_busy)
    {
        pr_err("%s(%d):LPORT MDIO interface is not ready rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, rc, control.fail, control.start_busy);
        rc =  LPORT_ERR_IO;
        goto done;
    }
    rc = ag_drv_lport_mdio_cfg_get(&free_run_clk_enable,&supress_preamble,&mdio_clk_divider,&mdio_clause);

    mdio_clause = MDIO_CL22;

    rc = rc ? rc: ag_drv_lport_mdio_cfg_set(free_run_clk_enable,supress_preamble,mdio_clk_divider,mdio_clause);

    control.phy_prt_addr = phyid;
    control.reg_dev_addr = addr;
    control.data_addr = data;
    control.start_busy = 1;
    control.op_code =MDIO_WRITE;
    control.fail = 0;

    if ( mdio_set_and_wait(&control) )
    {
        pr_err("%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, rc, control.fail, control.start_busy);
        rc =  LPORT_ERR_IO;
        goto done;
    }

done:
    MDIO_CNTX_UNLOCK;
    return rc;
}
#ifndef _CFE_
EXPORT_SYMBOL(lport_mdio22_wr);
#endif

int lport_mdio22_rd(uint16_t phyid,uint16_t addr, uint16_t *data)
{
    uint8_t free_run_clk_enable;
    uint8_t supress_preamble;
    uint8_t mdio_clk_divider;
    uint8_t mdio_clause;
    lport_mdio_control control;
    int rc = LPORT_ERR_OK;

    MDIO_CHECK_CNTX_LOCK;
    rc = ag_drv_lport_mdio_control_get(&control);
    if(rc /* || control.fail */ || control.start_busy)
    {
        pr_err("%s(%d):LPORT MDIO interface is not ready rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, rc, control.fail, control.start_busy);
        rc =  LPORT_ERR_IO;
        goto done;
    }
    rc = ag_drv_lport_mdio_cfg_get(&free_run_clk_enable,&supress_preamble,&mdio_clk_divider,&mdio_clause);

    mdio_clause = MDIO_CL22;

    rc = rc ? rc: ag_drv_lport_mdio_cfg_set(free_run_clk_enable,supress_preamble,mdio_clk_divider,mdio_clause);

    control.phy_prt_addr = phyid;
    control.reg_dev_addr = addr;
    control.start_busy = 1;
    control.op_code =MDIO_CL22_RD;
    control.fail = 0;

    if ( mdio_set_and_wait(&control) )
    {
        pr_err("%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__,__LINE__, rc, control.fail, control.start_busy);
        rc =  LPORT_ERR_IO;
        goto done;
    }

    //read the result,mdio_set_and_wait also fetch the result
    *data = control.data_addr;

done:
    MDIO_CNTX_UNLOCK;
    return rc;
}
#ifndef _CFE_
EXPORT_SYMBOL(lport_mdio22_rd);
#endif

int lport_mdio45_rd(uint16_t phyid, uint16_t devid, uint16_t addr, uint16_t *data)
{
    uint8_t free_run_clk_enable;
    uint8_t supress_preamble;
    uint8_t mdio_clk_divider;
    uint8_t mdio_clause;
    lport_mdio_control control;
    int rc = LPORT_ERR_OK;

    MDIO_CHECK_CNTX_LOCK;
    rc = ag_drv_lport_mdio_control_get(&control);
    if (rc || control.start_busy)
    {
        pr_err(
            "%s(%d):LPORT MDIO interface is not ready rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }
    rc = ag_drv_lport_mdio_cfg_get(&free_run_clk_enable,&supress_preamble, &mdio_clk_divider,
        &mdio_clause);

    mdio_clause = MDIO_CL45;

    rc = rc ?
        rc :
        ag_drv_lport_mdio_cfg_set(free_run_clk_enable,supress_preamble, mdio_clk_divider,
            mdio_clause);

    //write the CL45 address
    control.phy_prt_addr = phyid;
    control.reg_dev_addr = devid;
    control.data_addr = addr;
    control.start_busy = 1;
    control.op_code = MDIO_CL45_ADDRESS;
    control.fail = 0;

    if (mdio_set_and_wait(&control))
    {
        pr_err(
            "%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }

    //write the read request
    control.start_busy = 1;
    control.op_code = MDIO_CL45_RD;

    if (mdio_set_and_wait(&control))
    {
        pr_err(
            "%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }

    //read the result,mdio_set_and_wait also fetch the result
    *data = control.data_addr;
done:
    MDIO_CNTX_UNLOCK;
    return rc;
}
#ifndef _CFE_
EXPORT_SYMBOL(lport_mdio45_rd);
#endif

int lport_mdio45_wr(uint16_t phyid, uint16_t devid, uint16_t addr,
    uint16_t data)
{
    uint8_t free_run_clk_enable;
    uint8_t supress_preamble;
    uint8_t mdio_clk_divider;
    uint8_t mdio_clause;
    lport_mdio_control control;
    int rc = LPORT_ERR_OK;

    MDIO_CHECK_CNTX_LOCK;
    rc = ag_drv_lport_mdio_control_get(&control);
    if (rc || control.start_busy)
    {
        pr_err( "%s(%d):LPORT MDIO interface is not ready rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }
    rc = ag_drv_lport_mdio_cfg_get(&free_run_clk_enable,&supress_preamble, &mdio_clk_divider,
        &mdio_clause);

    mdio_clause = MDIO_CL45;

    rc = rc ?
        rc :
        ag_drv_lport_mdio_cfg_set(free_run_clk_enable,supress_preamble, mdio_clk_divider,
            mdio_clause);

    //write the CL45 address
    control.phy_prt_addr = phyid;
    control.reg_dev_addr = devid;
    control.data_addr = addr;
    control.start_busy = 1;
    control.op_code = MDIO_CL45_ADDRESS;
    control.fail = 0;

    if (mdio_set_and_wait(&control))
    {
        pr_err(
            "%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }

    //write the read request
    control.start_busy = 1;
    control.op_code = MDIO_WRITE;
    control.data_addr = data;

    if (mdio_set_and_wait(&control))
    {
        pr_err(
            "%s(%d):LPORT MDIO transaction failed rc=%d, fail=%d, busy=%d\n",
            __FUNCTION__, __LINE__, rc, control.fail, control.start_busy);
        rc = LPORT_ERR_IO;
        goto done;
    }

done:
    MDIO_CNTX_UNLOCK;
    return rc;
}
#ifndef _CFE_
EXPORT_SYMBOL(lport_mdio45_wr);
#endif
