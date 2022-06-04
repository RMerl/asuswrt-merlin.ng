#if defined(CONFIG_BCM_KF_RUNNER)
/*
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; 
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef BR_FP_H
#define BR_FP_H

#include <linux/device.h>
#include <linux/module.h>

#define BR_FP_FDB_ADD 1
#define BR_FP_FDB_REMOVE 2
#define BR_FP_FDB_MODIFY 3
#define BR_FP_FDB_CHECK_AGE 4
#define BR_FP_PORT_ADD 5
#define BR_FP_PORT_REMOVE 6
#define BR_FP_LOCAL_SWITCHING_DISABLE 7
#define BR_FP_BRIDGE_TYPE 8

struct br_fp_data
{
    int (*rdpa_hook)(int cmd, void *in, void *out);
    void *rdpa_priv;
};

/* interface routine */
void br_fp_set_callbacks(struct br_fp_data *fpdata);
void br_fp_clear_callbacks(void);
 
#endif /* BR_FP_H */
#endif
