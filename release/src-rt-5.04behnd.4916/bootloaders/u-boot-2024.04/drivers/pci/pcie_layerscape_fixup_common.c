// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2021 NXP
 *
 * PCIe DT fixup for NXP Layerscape SoCs
 * Author: Wasim Khan <wasim.khan@nxp.com>
 *
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include "pcie_layerscape_fixup_common.h"

extern int next_stream_id;

void ft_pci_setup(void *blob, struct bd_info *bd)
{
#if defined(CONFIG_PCIE_LAYERSCAPE_GEN4)
	uint svr;

	svr = SVR_SOC_VER(get_svr());

	if (svr == SVR_LX2160A && IS_SVR_REV(get_svr(), 1, 0))
		ft_pci_setup_ls_gen4(blob, bd);
	else
#endif /* CONFIG_PCIE_LAYERSCAPE_GEN4 */
		ft_pci_setup_ls(blob, bd);
}

#if defined(CONFIG_FSL_LAYERSCAPE)
static int lx2_board_fix_fdt(void *fdt)
{
	char *reg_name, *old_str, *new_str;
	const char *reg_names;
	int names_len, old_str_len, new_str_len, remaining_str_len;
	struct str_map {
		char *old_str;
		char *new_str;
	} reg_names_map[] = {
		{ "csr_axi_slave", "regs" },
		{ "config_axi_slave", "config" }
	};
	int off = -1, i;
	const fdt32_t *prop;
	u32 ob_wins, ib_wins;

	fdt_for_each_node_by_compatible(off, fdt, -1, "fsl,lx2160a-pcie") {
		fdt_setprop(fdt, off, "compatible", "fsl,ls2088a-pcie",
			    strlen("fsl,ls2088a-pcie") + 1);

		reg_names = fdt_getprop(fdt, off, "reg-names", &names_len);
		if (!reg_names)
			continue;
		reg_name = (char *)reg_names;
		remaining_str_len = names_len - (reg_name - reg_names);
		i = 0;
		while ((i < ARRAY_SIZE(reg_names_map)) && remaining_str_len) {
			old_str = reg_names_map[i].old_str;
			new_str = reg_names_map[i].new_str;
			old_str_len = strlen(old_str);
			new_str_len = strlen(new_str);
			if (memcmp(reg_name, old_str, old_str_len) == 0) {
				/* first only leave required bytes for new_str
				 * and copy rest of the string after it
				 */
				memcpy(reg_name + new_str_len,
				       reg_name + old_str_len,
				       remaining_str_len - old_str_len);

				/* Now copy new_str */
				memcpy(reg_name, new_str, new_str_len);
				names_len -= old_str_len;
				names_len += new_str_len;
				i++;
			}

			reg_name = memchr(reg_name, '\0', remaining_str_len);
			if (!reg_name)
				break;
			reg_name += 1;

			remaining_str_len = names_len - (reg_name - reg_names);
		}
		fdt_setprop(fdt, off, "reg-names", reg_names, names_len);
		fdt_delprop(fdt, off, "apio-wins");
		fdt_delprop(fdt, off, "ppio-wins");
	}

	/* Fixup PCIe EP nodes */
	fdt_for_each_node_by_compatible(off, fdt, -1, "fsl,lx2160a-pcie-ep") {
		fdt_setprop_string(fdt, off, "compatible",
				   "fsl,lx2160ar2-pcie-ep");
		prop = fdt_getprop(fdt, off, "apio-wins", NULL);
		if (!prop) {
			printf("%s: Failed to fixup PCIe EP node @0x%x\n",
			       __func__, off);
			off = fdt_node_offset_by_compatible(fdt, off,
							    "fsl,lx2160a-pcie-ep");
			continue;
		}

		ob_wins = fdt32_to_cpu(*prop);
		ib_wins = (ob_wins == 256) ? 24 : 8;
		fdt_setprop_u32(fdt, off, "num-ib-windows", ib_wins);
		fdt_setprop_u32(fdt, off, "num-ob-windows", ob_wins);
		fdt_delprop(fdt, off, "apio-wins");
	}

	return 0;
}

int pcie_board_fix_fdt(void *fdt)
{
	uint svr;

	svr = SVR_SOC_VER(get_svr());

	if ((svr == SVR_LX2160A || svr == SVR_LX2162A ||
	     svr == SVR_LX2120A || svr == SVR_LX2080A ||
	     svr == SVR_LX2122A || svr == SVR_LX2082A) &&
	     IS_SVR_REV(get_svr(), 2, 0))
		return lx2_board_fix_fdt(fdt);

	return 0;
}

#if defined(CONFIG_ARCH_LX2160A) || defined(CONFIG_ARCH_LX2162A)
/* returns the next available streamid for pcie, -errno if failed */
int pcie_next_streamid(int currentid, int idx)
{
	if (currentid > FSL_PEX_STREAM_ID_END)
		return -EINVAL;

	return currentid | ((idx + 1) << 11);
}
#else
/* returns the next available streamid for pcie, -errno if failed */
int pcie_next_streamid(int currentid, int idx)
{
	if (next_stream_id > FSL_PEX_STREAM_ID_END)
		return -EINVAL;

	return next_stream_id++;
}
#endif
#endif /* CONFIG_FSL_LAYERSCAPE */
