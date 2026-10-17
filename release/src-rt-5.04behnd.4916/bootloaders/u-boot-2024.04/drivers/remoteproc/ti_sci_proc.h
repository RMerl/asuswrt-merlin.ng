/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Texas Instruments TI-SCI Processor Controller Helper Functions
 *
 * Copyright (C) 2018-2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 *	Suman Anna <s-anna@ti.com>
 */

#ifndef REMOTEPROC_TI_SCI_PROC_H
#define REMOTEPROC_TI_SCI_PROC_H

#include <linux/printk.h>
#define TISCI_INVALID_HOST 0xff

/**
 * struct ti_sci_proc - structure representing a processor control client
 * @sci: cached TI-SCI protocol handle
 * @ops: cached TI-SCI proc ops
 * @proc_id: processor id for the consumer remoteproc device
 * @host_id: host id to pass the control over for this consumer remoteproc
 *	     device
 * @dev_id: Device ID as identified by system controller.
 */
struct ti_sci_proc {
	const struct ti_sci_handle *sci;
	const struct ti_sci_proc_ops *ops;
	u8 proc_id;
	u8 host_id;
	u16 dev_id;
};

static inline int ti_sci_proc_request(struct ti_sci_proc *tsp)
{
	int ret;

	debug("%s: proc_id = %d\n", __func__, tsp->proc_id);

	ret = tsp->ops->proc_request(tsp->sci, tsp->proc_id);
	if (ret)
		pr_err("ti-sci processor request failed: %d\n", ret);
	return ret;
}

static inline int ti_sci_proc_release(struct ti_sci_proc *tsp)
{
	int ret;

	debug("%s: proc_id = %d\n", __func__, tsp->proc_id);

	if (tsp->host_id != TISCI_INVALID_HOST)
		ret = tsp->ops->proc_handover(tsp->sci, tsp->proc_id,
					      tsp->host_id);
	else
		ret = tsp->ops->proc_release(tsp->sci, tsp->proc_id);

	if (ret)
		pr_err("ti-sci processor release failed: %d\n", ret);
	return ret;
}

static inline int ti_sci_proc_handover(struct ti_sci_proc *tsp)
{
	int ret;

	debug("%s: proc_id = %d\n", __func__, tsp->proc_id);

	ret = tsp->ops->proc_handover(tsp->sci, tsp->proc_id, tsp->host_id);
	if (ret)
		pr_err("ti-sci processor handover of %d to %d failed: %d\n",
		       tsp->proc_id, tsp->host_id, ret);
	return ret;
}

static inline int ti_sci_proc_get_status(struct ti_sci_proc *tsp,
					 u64 *boot_vector, u32 *cfg_flags,
					 u32 *ctrl_flags, u32 *status_flags)
{
	int ret;

	ret = tsp->ops->get_proc_boot_status(tsp->sci, tsp->proc_id,
					     boot_vector, cfg_flags, ctrl_flags,
					     status_flags);
	if (ret)
		pr_err("ti-sci processor get_status failed: %d\n", ret);

	debug("%s: proc_id = %d, boot_vector = 0x%llx, cfg_flags = 0x%x, ctrl_flags = 0x%x, sts = 0x%x\n",
	      __func__, tsp->proc_id, *boot_vector, *cfg_flags, *ctrl_flags,
	      *status_flags);
	return ret;
}

static inline int ti_sci_proc_set_config(struct ti_sci_proc *tsp,
					 u64 boot_vector,
					 u32 cfg_set, u32 cfg_clr)
{
	int ret;

	debug("%s: proc_id = %d, boot_vector = 0x%llx, cfg_set = 0x%x, cfg_clr = 0x%x\n",
	      __func__, tsp->proc_id, boot_vector, cfg_set, cfg_clr);

	ret = tsp->ops->set_proc_boot_cfg(tsp->sci, tsp->proc_id, boot_vector,
					  cfg_set, cfg_clr);
	if (ret)
		pr_err("ti-sci processor set_config failed: %d\n", ret);
	return ret;
}

static inline int ti_sci_proc_set_control(struct ti_sci_proc *tsp,
					  u32 ctrl_set, u32 ctrl_clr)
{
	int ret;

	debug("%s: proc_id = %d, ctrl_set = 0x%x, ctrl_clr = 0x%x\n", __func__,
	      tsp->proc_id, ctrl_set, ctrl_clr);

	ret = tsp->ops->set_proc_boot_ctrl(tsp->sci, tsp->proc_id, ctrl_set,
					   ctrl_clr);
	if (ret)
		pr_err("ti-sci processor set_control failed: %d\n", ret);
	return ret;
}

static inline int ti_sci_proc_power_domain_on(struct ti_sci_proc *tsp)
{
	int ret;

	debug("%s: dev_id = %d\n", __func__, tsp->dev_id);

	ret = tsp->sci->ops.dev_ops.get_device_exclusive(tsp->sci, tsp->dev_id);
	if (ret)
		pr_err("Power-domain on failed for dev = %d\n", tsp->dev_id);

	return ret;
}

static inline int ti_sci_proc_power_domain_off(struct ti_sci_proc *tsp)
{
	int ret;

	debug("%s: dev_id = %d\n", __func__, tsp->dev_id);

	ret = tsp->sci->ops.dev_ops.put_device(tsp->sci, tsp->dev_id);
	if (ret)
		pr_err("Power-domain off failed for dev = %d\n", tsp->dev_id);

	return ret;
}
#endif /* REMOTEPROC_TI_SCI_PROC_H */
