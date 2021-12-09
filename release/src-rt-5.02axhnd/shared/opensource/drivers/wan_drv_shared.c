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
#include "bl_os_wraper.h"
#include <linux/spinlock.h>

static pon_serdes_lof_fixup_fifo_t fifo_cb;
static pon_serdes_lof_fixup_reset_cdr_t reset_cdr;
static int state_tracking, is_init = TRUE, sd_timer_id = -1;
DEFINE_SPINLOCK(state_lock);
#define STATE_NO_SYNC   0
#define STATE_SYNC      1
#define STATE_JUST_SYNC 2

static void restart_sd_timer(void)
{
    ext_timer_set_period(sd_timer_id, 1000*1000);
    ext_timer_start(sd_timer_id);
}

static void fixup_func(unsigned long data)
{
    int current_state_tracking;

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
    }
    else
    {
        if (reset_cdr)
            (*reset_cdr)();
    }

    restart_sd_timer();
}

DECLARE_TASKLET(fixup_tasklet, fixup_func, (unsigned long)0);

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
        restart_sd_timer();
    spin_unlock(&state_lock);
}
EXPORT_SYMBOL(pon_serdes_lof_fixup_irq);

int pon_serdes_lof_fixup_cfg(pon_serdes_lof_fixup_fifo_t *_fifo_cb, pon_serdes_lof_fixup_reset_cdr_t reset_cb)
{
    if (_fifo_cb)
        fifo_cb = *_fifo_cb;
    if (reset_cb)
        reset_cdr = reset_cb;

    if (sd_timer_id != -1)
        return 0;

    sd_timer_id = ext_timer_alloc_only(-1, &sd_timer_callback, 0);
    if (sd_timer_id < 0)
    {
        wd_log("wan_drv: failed to allocate timer\n");
        return -1;
    }

    restart_sd_timer();

    return 0;
}
EXPORT_SYMBOL(pon_serdes_lof_fixup_cfg);

