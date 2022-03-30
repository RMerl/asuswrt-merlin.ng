// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_qe.h>

#ifdef CONFIG_QE
DECLARE_GLOBAL_DATA_PTR;

/*
 * If a QE firmware has been uploaded, then add the 'firmware' node under
 * the 'qe' node.
 */
void fdt_fixup_qe_firmware(void *blob)
{
	struct qe_firmware_info *qe_fw_info;
	int node, ret;

	qe_fw_info = qe_get_firmware_info();
	if (!qe_fw_info)
		return;

	node = fdt_path_offset(blob, "/qe");
	if (node < 0)
		return;

	/* We assume the node doesn't exist yet */
	node = fdt_add_subnode(blob, node, "firmware");
	if (node < 0)
		return;

	ret = fdt_setprop(blob, node, "extended-modes",
		&qe_fw_info->extended_modes, sizeof(u64));
	if (ret < 0)
		goto error;

	ret = fdt_setprop_string(blob, node, "id", qe_fw_info->id);
	if (ret < 0)
		goto error;

	ret = fdt_setprop(blob, node, "virtual-traps", qe_fw_info->vtraps,
		sizeof(qe_fw_info->vtraps));
	if (ret < 0)
		goto error;

	return;

error:
	fdt_del_node(blob, node);
}

void ft_qe_setup(void *blob)
{
	do_fixup_by_prop_u32(blob, "device_type", "qe", 4,
		"bus-frequency", gd->arch.qe_clk, 1);
	do_fixup_by_prop_u32(blob, "device_type", "qe", 4,
		"brg-frequency", gd->arch.brg_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"clock-frequency", gd->arch.qe_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"bus-frequency", gd->arch.qe_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe",
		"brg-frequency", gd->arch.brg_clk, 1);
	do_fixup_by_compat_u32(blob, "fsl,qe-gtm",
		"clock-frequency", gd->arch.qe_clk / 2, 1);
	fdt_fixup_qe_firmware(blob);
}
#endif
