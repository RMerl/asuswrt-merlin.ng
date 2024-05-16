/*
    Copyright 2021 Broadcom Corporation

    <:label-BRCM:2021:DUAL/GPL:standard

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
#include <linux/version.h>

void onu_sgb_gbox_tx_fifo_ptr_wr_rd__tx_looptime_reset(void);
void onu_cdr_relock__sgb_gbox_rx_fifo_ptr_wr_rd(void);

#define STATE_NO_SYNC   0
#define STATE_SYNC      1
#define STATE_JUST_SYNC 2
static int state_tracking = STATE_NO_SYNC, sd_timer_id = -1;
DEFINE_SPINLOCK(state_lock);
static int enabled;

static void restart_sd_timer(int now)
{
    ext_timer_set_period(sd_timer_id, now ? 1 : 1000*1000);
    ext_timer_start(sd_timer_id);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0))
static void fixup_func(struct tasklet_struct *unused)
#else
static void fixup_func(unsigned long data)
#endif
{
    int current_state_tracking;
    static uint64_t prev_stamp; /* time of previous sync */
    const uint64_t  now_stamp = jiffies;
    const uint64_t  gap_stamp = now_stamp - prev_stamp; /* time since last sync */

    spin_lock(&state_lock);
    if (!enabled)
    {
        spin_unlock(&state_lock);
        goto restart_timer;
    }

    current_state_tracking = state_tracking;
    if (state_tracking == STATE_JUST_SYNC)
        state_tracking = STATE_SYNC;
    spin_unlock(&state_lock);

    if (current_state_tracking)
    {
        if (current_state_tracking == STATE_JUST_SYNC)
        {
            prev_stamp = now_stamp;
            if (gap_stamp > msecs_to_jiffies(100))
            {
                wd_log_debug("JUST_SYNC\n");
                mdelay(20); /* wait for sync to be processed */
                onu_sgb_gbox_tx_fifo_ptr_wr_rd__tx_looptime_reset();
            }
            else
                wd_log_debug("IGNORING JUST_SYNC\n");
        }
    }
    else
    {
        if (gap_stamp > msecs_to_jiffies(100))
        {
            wd_log_debug("DISC\n");
            onu_cdr_relock__sgb_gbox_rx_fifo_ptr_wr_rd();
        }
        else
            wd_log_debug("IGNORING DISC\n");
    }

restart_timer:
    restart_sd_timer(0);
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0))
DECLARE_TASKLET(fixup_tasklet, fixup_func);
#else
DECLARE_TASKLET(fixup_tasklet, fixup_func, 0);
#endif

static void sd_timer_callback(unsigned long xi_user_params_ptr)
{
    ext_timer_stop(sd_timer_id);
    tasklet_hi_schedule(&fixup_tasklet);
}

void pon_serdes_lof_fixup_irq(int lof)
{
    if (sd_timer_id == -1)
        return;

    spin_lock(&state_lock);
    ext_timer_stop(sd_timer_id);
    state_tracking = lof ? STATE_NO_SYNC : STATE_JUST_SYNC;
    restart_sd_timer(1);
    spin_unlock(&state_lock);
    wd_log_debug("LOF interrupt lof==%d, state_tracking==%d\n", lof, state_tracking);
}
EXPORT_SYMBOL(pon_serdes_lof_fixup_irq);

int pon_serdes_lof_fixup_cfg(int enable)
{
    spin_lock(&state_lock);
    state_tracking = STATE_NO_SYNC;
    enabled = enable;
    spin_unlock(&state_lock);

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

