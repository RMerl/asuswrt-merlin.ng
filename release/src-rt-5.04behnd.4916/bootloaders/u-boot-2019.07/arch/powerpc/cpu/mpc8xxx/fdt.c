// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2014 Freescale Semiconductor, Inc.
 *
 * This file is derived from arch/powerpc/cpu/mpc85xx/cpu.c and
 * arch/powerpc/cpu/mpc86xx/cpu.c. Basically this file contains
 * cpu specific common code for 85xx/86xx processors.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <asm/mp.h>
#include <asm/fsl_serdes.h>
#include <phy.h>
#include <hwconfig.h>

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1
#endif

#if defined(CONFIG_MP) && (defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx))
static int ft_del_cpuhandle(void *blob, int cpuhandle)
{
	int off, ret = -FDT_ERR_NOTFOUND;

	/* if we find a match, we'll delete at it which point the offsets are
	 * invalid so we start over from the beginning
	 */
	off = fdt_node_offset_by_prop_value(blob, -1, "cpu-handle",
						&cpuhandle, 4);
	while (off != -FDT_ERR_NOTFOUND) {
		fdt_delprop(blob, off, "cpu-handle");
		ret = 1;
		off = fdt_node_offset_by_prop_value(blob, -1, "cpu-handle",
				&cpuhandle, 4);
	}

	return ret;
}

void ft_fixup_num_cores(void *blob) {
	int off, num_cores, del_cores;

	del_cores = 0;
	num_cores = cpu_numcores();

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		u32 *reg = (u32 *)fdt_getprop(blob, off, "reg", 0);
		u32 phys_cpu_id = thread_to_core(*reg);

		if (!is_core_valid(phys_cpu_id) || is_core_disabled(phys_cpu_id)) {
			int ph = fdt_get_phandle(blob, off);

			/* Delete the cpu node once there are no cpu handles */
			if (-FDT_ERR_NOTFOUND == ft_del_cpuhandle(blob, ph)) {
				fdt_del_node(blob, off);
				del_cores++;
			}
			/* either we deleted some cpu handles or the cpu node
			 * so we reset the offset back to the start since we
			 * can't trust the offsets anymore
			 */
			off = -1;
		}
		off = fdt_node_offset_by_prop_value(blob, off,
				"device_type", "cpu", 4);
	}
	debug ("%x core system found\n", num_cores);
	debug ("deleted %d extra core entry entries from device tree\n",
								del_cores);
}
#endif /* defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx) */

int fdt_fixup_phy_connection(void *blob, int offset, phy_interface_t phyc)
{
	return fdt_setprop_string(blob, offset, "phy-connection-type",
					 phy_string_for_interface(phyc));
}

#ifdef CONFIG_SYS_SRIO
static inline void ft_disable_srio_port(void *blob, int srio_off, int port)
{
	int off = fdt_node_offset_by_prop_value(blob, srio_off,
			"cell-index", &port, 4);
	if (off >= 0) {
		off = fdt_setprop_string(blob, off, "status", "disabled");
		if (off > 0)
			printf("WARNING unable to set status for fsl,srio "
				"port %d: %s\n", port, fdt_strerror(off));
	}
}

static inline void ft_disable_rman(void *blob)
{
	int off = fdt_node_offset_by_compatible(blob, -1, "fsl,rman");
	if (off >= 0) {
		off = fdt_setprop_string(blob, off, "status", "disabled");
		if (off > 0)
			printf("WARNING unable to set status for fsl,rman %s\n",
				fdt_strerror(off));
	}
}

static inline void ft_disable_rmu(void *blob)
{
	int off = fdt_node_offset_by_compatible(blob, -1, "fsl,srio-rmu");
	if (off >= 0) {
		off = fdt_setprop_string(blob, off, "status", "disabled");
		if (off > 0)
			printf("WARNING unable to set status for "
				"fsl,srio-rmu %s\n", fdt_strerror(off));
	}
}

void ft_srio_setup(void *blob)
{
	int srio1_used = 0, srio2_used = 0;
	int srio_off;

	/* search for srio node, if doesn't exist just return - nothing todo */
	srio_off = fdt_node_offset_by_compatible(blob, -1, "fsl,srio");
	if (srio_off < 0)
		return ;

#ifdef CONFIG_SRIO1
	if (is_serdes_configured(SRIO1))
		srio1_used = 1;
#endif
#ifdef CONFIG_SRIO2
	if (is_serdes_configured(SRIO2))
		srio2_used = 1;
#endif

	/* mark port1 disabled */
	if (!srio1_used)
		ft_disable_srio_port(blob, srio_off, 1);

	/* mark port2 disabled */
	if (!srio2_used)
		ft_disable_srio_port(blob, srio_off, 2);

	/* if both ports not used, disable controller, rmu and rman */
	if (!srio1_used && !srio2_used) {
		fdt_setprop_string(blob, srio_off, "status", "disabled");

		ft_disable_rman(blob);
		ft_disable_rmu(blob);
	}
}
#endif
