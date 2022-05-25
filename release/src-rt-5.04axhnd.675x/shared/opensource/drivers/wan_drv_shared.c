/*
    Copyright 2016 Broadcom Corporation

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

#include <wan_drv.h>
#include "bcm_ext_timer.h"
#include "bcmtypes.h"
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

static pon_serdes_lof_fixup_fifo_t fifo_cb;
static pon_serdes_lof_fixup_reset_cdr_t reset_cdr;
static void (*pmd_rx_restart)(void);
static void (*reset_rx_fifo)(void);
static void (*reset_tx_fifo)(void);
static void (*reset_gb)(void);
static int state_tracking, is_init = TRUE, sd_timer_id = -1;
DEFINE_SPINLOCK(state_lock);
#define STATE_NO_SYNC   0
#define STATE_SYNC      1
#define STATE_JUST_SYNC 2

static void restart_sd_timer(int now)
{
    ext_timer_set_period(sd_timer_id, now ? 1 : 1000*1000);
    ext_timer_start(sd_timer_id);
}

static void fixup_func(unsigned long data)
{
    int current_state_tracking;
    static uint64_t prev_stamp; /* time of previous sync */
    const uint64_t  now_stamp = jiffies;
    const uint64_t  gap_stamp = now_stamp - prev_stamp; /* time since last sync */

    spin_lock(&state_lock);
    current_state_tracking = state_tracking;
    if (state_tracking == STATE_JUST_SYNC)
        state_tracking = STATE_SYNC;
    spin_unlock(&state_lock);

    if (fifo_cb.reset_fifo && is_init && current_state_tracking)
    {
        is_init = FALSE;
        fifo_cb.reset_fifo();
    }
    else if (current_state_tracking)
    {
        if (fifo_cb.gearbox_drift_test && fifo_cb.gearbox_drift_test(current_state_tracking == STATE_JUST_SYNC))
            fifo_cb.reset_fifo();
        if (current_state_tracking == STATE_JUST_SYNC)
        {
            prev_stamp = now_stamp;
            if (gap_stamp > msecs_to_jiffies(100))
            {
                wd_log_debug("\nJUST_SYNC");
                /* if (pmd_rx_restart)
                {
                    (*pmd_rx_restart)();
                    wd_log_debug("\nJUST_SYNC");
                    mdelay(10);
                }
                if (reset_rx_fifo)
                {
                    (*reset_rx_fifo)();
                    mdelay(50); // wait to recover from reset_rx_fifo before reset_tx_fifo
                }
                if (reset_tx_fifo)
                    (*reset_tx_fifo)(); */
                if (reset_gb)
                {
                    mdelay(20); /* wait for sync to be processed before reset_gb */
                    (*reset_gb)();
                }
            }
            else
                wd_log_debug("\nIGNORING JUST_SYNC");
        }
    }
    else
    {
        if (gap_stamp > msecs_to_jiffies(100))
        {
            wd_log_debug("\nDISC");
               /* if (pmd_rx_restart)
                (*pmd_rx_restart)(); */
            if (reset_cdr)
                (*reset_cdr)();
        }
        else
            wd_log_debug("\nIGNORING DISC");
    }

    restart_sd_timer(0);
}

DECLARE_TASKLET(fixup_tasklet, fixup_func, 0);

static void sd_timer_callback(unsigned long xi_user_params_ptr)
{
    tasklet_hi_schedule(&fixup_tasklet);
}

void pon_serdes_lof_fixup_irq(int lof)
{
    if (sd_timer_id == -1)
        return;

    spin_lock(&state_lock);
    ext_timer_stop(sd_timer_id);
    state_tracking = lof ? STATE_NO_SYNC : STATE_JUST_SYNC;

    wd_log_debug("LOF interrupt clear: %d\n", state_tracking);

    if (fifo_cb.reset_fifo && state_tracking)
        tasklet_hi_schedule(&fixup_tasklet);
    else
        restart_sd_timer(1);
    spin_unlock(&state_lock);
}
EXPORT_SYMBOL(pon_serdes_lof_fixup_irq);

int pon_serdes_lof_fixup_cfg(pon_serdes_lof_fixup_fifo_t *_fifo_cb, pon_serdes_lof_fixup_reset_cdr_t reset_cb, void (*pmd_rx_restart_cb)(void), void (*reset_rx_fifo_cb)(void), void (*reset_tx_fifo_cb)(void), void (*reset_gb_cb)(void))
{
    if (_fifo_cb)
        fifo_cb = *_fifo_cb;
    reset_cdr = reset_cb;
    pmd_rx_restart = pmd_rx_restart_cb;
    reset_rx_fifo = reset_rx_fifo_cb;
    reset_tx_fifo = reset_tx_fifo_cb;
    reset_gb = reset_gb_cb;

    if (sd_timer_id != -1)
        return 0;

    sd_timer_id = ext_timer_alloc_only(-1, &sd_timer_callback, 0);
    if (sd_timer_id < 0)
    {
        wd_log("wan_drv: failed to allocate timer\n");
        return -1;
    }

    restart_sd_timer(0);

    return 0;
}
EXPORT_SYMBOL(pon_serdes_lof_fixup_cfg);

