// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/armada100.h>

#define UARTCLK14745KHZ	(APBC_APBCLK | APBC_FNCLK | APBC_FNCLKSEL(1))
#define SET_MRVL_ID	(1<<8)
#define L2C_RAM_SEL	(1<<4)

int arch_cpu_init(void)
{
	u32 val;
	struct armd1cpu_registers *cpuregs =
		(struct armd1cpu_registers *) ARMD1_CPU_BASE;

	struct armd1apb1_registers *apb1clkres =
		(struct armd1apb1_registers *) ARMD1_APBC1_BASE;

	struct armd1mpmu_registers *mpmu =
		(struct armd1mpmu_registers *) ARMD1_MPMU_BASE;

	/* set SEL_MRVL_ID bit in ARMADA100_CPU_CONF register */
	val = readl(&cpuregs->cpu_conf);
	val = val | SET_MRVL_ID;
	writel(val, &cpuregs->cpu_conf);

	/* Enable Clocks for all hardware units */
	writel(0xFFFFFFFF, &mpmu->acgr);

	/* Turn on AIB and AIB-APB Functional clock */
	writel(APBC_APBCLK | APBC_FNCLK, &apb1clkres->aib);

	/* ensure L2 cache is not mapped as SRAM */
	val = readl(&cpuregs->cpu_conf);
	val = val & ~(L2C_RAM_SEL);
	writel(val, &cpuregs->cpu_conf);

	/* Enable GPIO clock */
	writel(APBC_APBCLK, &apb1clkres->gpio);

#ifdef CONFIG_I2C_MV
	/* Enable general I2C clock */
	writel(APBC_RST | APBC_FNCLK | APBC_APBCLK, &apb1clkres->twsi0);
	writel(APBC_FNCLK | APBC_APBCLK, &apb1clkres->twsi0);

	/* Enable power I2C clock */
	writel(APBC_RST | APBC_FNCLK | APBC_APBCLK, &apb1clkres->twsi1);
	writel(APBC_FNCLK | APBC_APBCLK, &apb1clkres->twsi1);
#endif

	/*
	 * Enable Functional and APB clock at 14.7456MHz
	 * for configured UART console
	 */
#if (CONFIG_SYS_NS16550_COM1 == ARMD1_UART3_BASE)
	writel(UARTCLK14745KHZ, &apb1clkres->uart3);
#elif (CONFIG_SYS_NS16550_COM1 == ARMD1_UART2_BASE)
	writel(UARTCLK14745KHZ, &apb1clkres->uart2);
#else
	writel(UARTCLK14745KHZ, &apb1clkres->uart1);
#endif
	icache_enable();

	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	u32 id;
	struct armd1cpu_registers *cpuregs =
		(struct armd1cpu_registers *) ARMD1_CPU_BASE;

	id = readl(&cpuregs->chip_id);
	printf("SoC:   Armada 88AP%X-%X\n", (id & 0xFFF), (id >> 0x10));
	return 0;
}
#endif

#ifdef CONFIG_I2C_MV
void i2c_clk_enable(void)
{
}
#endif
