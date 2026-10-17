// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/stv0991_cgu.h>
#include<asm/arch/stv0991_periph.h>

static struct stv0991_cgu_regs *const stv0991_cgu_regs = \
				(struct stv0991_cgu_regs *) (CGU_BASE_ADDR);

void enable_pll1(void)
{
	/* pll1 already configured for 1000Mhz, just need to enable it */
	writel(readl(&stv0991_cgu_regs->pll1_ctrl) & ~(0x01),
			&stv0991_cgu_regs->pll1_ctrl);
}

void clock_setup(int peripheral)
{
	switch (peripheral) {
	case UART_CLOCK_CFG:
		writel(UART_CLK_CFG, &stv0991_cgu_regs->uart_freq);
		break;
	case ETH_CLOCK_CFG:
		enable_pll1();
		writel(ETH_CLK_CFG, &stv0991_cgu_regs->eth_freq);

		/* Clock selection for ethernet tx_clk & rx_clk*/
		writel((readl(&stv0991_cgu_regs->eth_ctrl) & ETH_CLK_MASK)
				| ETH_CLK_CTRL, &stv0991_cgu_regs->eth_ctrl);
		break;
	case QSPI_CLOCK_CFG:
		writel(QSPI_CLK_CTRL, &stv0991_cgu_regs->qspi_freq);
		break;
	default:
		break;
	}
}
