// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015, Siemens AG
 * Author: Jan Kiszka <jan.kiszka@siemens.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/arch/flow.h>
#include <asm/arch/powergate.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/pmc.h>

static void park_cpu(void)
{
	while (1)
		asm volatile("wfi");
}

/**
 * Initialize power management for application processors
 */
void psci_board_init(void)
{
	struct flow_ctlr *flow = (struct flow_ctlr *)NV_PA_FLOW_BASE;

	writel((u32)park_cpu, EXCEP_VECTOR_CPU_RESET_VECTOR);

	/*
	 * The naturally expected order of putting these CPUs under Flow
	 * Controller regime would be
	 *  - configure the Flow Controller
	 *  - power up the CPUs
	 *  - wait for the CPUs to hit wfi and be powered down again
	 *
	 * However, this doesn't work in practice. We rather need to power them
	 * up first and park them in wfi. While they are waiting there, we can
	 * indeed program the Flow Controller to powergate them on wfi, which
	 * will then happen immediately as they are already in that state.
	 */
	tegra_powergate_power_on(TEGRA_POWERGATE_CPU1);
	tegra_powergate_power_on(TEGRA_POWERGATE_CPU2);
	tegra_powergate_power_on(TEGRA_POWERGATE_CPU3);

	writel((2 << CSR_WAIT_WFI_SHIFT) | CSR_ENABLE, &flow->cpu1_csr);
	writel((4 << CSR_WAIT_WFI_SHIFT) | CSR_ENABLE, &flow->cpu2_csr);
	writel((8 << CSR_WAIT_WFI_SHIFT) | CSR_ENABLE, &flow->cpu3_csr);

	writel(EVENT_MODE_STOP, &flow->halt_cpu1_events);
	writel(EVENT_MODE_STOP, &flow->halt_cpu2_events);
	writel(EVENT_MODE_STOP, &flow->halt_cpu3_events);

	while (!(readl(&flow->cpu1_csr) & CSR_PWR_OFF_STS) ||
		!(readl(&flow->cpu2_csr) & CSR_PWR_OFF_STS) ||
		!(readl(&flow->cpu3_csr) & CSR_PWR_OFF_STS))
		/* wait */;
}
