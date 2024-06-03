/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef _SC_SCI_H
#define _SC_SCI_H

#include <asm/arch/sci/types.h>
#include <asm/arch/sci/svc/misc/api.h>
#include <asm/arch/sci/svc/pad/api.h>
#include <asm/arch/sci/svc/pm/api.h>
#include <asm/arch/sci/svc/rm/api.h>
#include <asm/arch/sci/rpc.h>
#include <dt-bindings/soc/imx_rsrc.h>
#include <linux/errno.h>

static inline int sc_err_to_linux(sc_err_t err)
{
	int ret;

	switch (err) {
	case SC_ERR_NONE:
		return 0;
	case SC_ERR_VERSION:
	case SC_ERR_CONFIG:
	case SC_ERR_PARM:
		ret = -EINVAL;
		break;
	case SC_ERR_NOACCESS:
	case SC_ERR_LOCKED:
	case SC_ERR_UNAVAILABLE:
		ret = -EACCES;
		break;
	case SC_ERR_NOTFOUND:
	case SC_ERR_NOPOWER:
		ret = -ENODEV;
		break;
	case SC_ERR_IPC:
		ret = -EIO;
		break;
	case SC_ERR_BUSY:
		ret = -EBUSY;
		break;
	case SC_ERR_FAIL:
		ret = -EIO;
		break;
	default:
		ret = 0;
		break;
	}

	debug("%s %d %d\n", __func__, err, ret);

	return ret;
}

/* PM API*/
int sc_pm_set_resource_power_mode(sc_ipc_t ipc, sc_rsrc_t resource,
				  sc_pm_power_mode_t mode);
int sc_pm_set_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
			 sc_pm_clock_rate_t *rate);
int sc_pm_get_clock_rate(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
			 sc_pm_clock_rate_t *rate);
int sc_pm_clock_enable(sc_ipc_t ipc, sc_rsrc_t resource, sc_pm_clk_t clk,
		       sc_bool_t enable, sc_bool_t autog);

/* MISC API */
int sc_misc_get_control(sc_ipc_t ipc, sc_rsrc_t resource, sc_ctrl_t ctrl,
			u32 *val);
void sc_misc_get_boot_dev(sc_ipc_t ipc, sc_rsrc_t *boot_dev);
void sc_misc_boot_status(sc_ipc_t ipc, sc_misc_boot_status_t status);
void sc_misc_build_info(sc_ipc_t ipc, u32 *build, u32 *commit);
int sc_misc_otp_fuse_read(sc_ipc_t ipc, u32 word, u32 *val);

/* RM API */
sc_bool_t sc_rm_is_memreg_owned(sc_ipc_t ipc, sc_rm_mr_t mr);
int sc_rm_get_memreg_info(sc_ipc_t ipc, sc_rm_mr_t mr, sc_faddr_t *addr_start,
			  sc_faddr_t *addr_end);
sc_bool_t sc_rm_is_resource_owned(sc_ipc_t ipc, sc_rsrc_t resource);

/* PAD API */
int sc_pad_set(sc_ipc_t ipc, sc_pad_t pad, u32 val);
#endif
