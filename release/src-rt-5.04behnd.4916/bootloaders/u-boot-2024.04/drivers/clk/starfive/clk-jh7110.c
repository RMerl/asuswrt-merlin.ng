// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022-23 StarFive Technology Co., Ltd.
 *
 * Author:	Yanhong Wang <yanhong.wang@starfivetech.com>
 *		Xingyu Wu <xingyu.wu@starfivetech.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dt-bindings/clock/starfive,jh7110-crg.h>
#include <log.h>
#include <linux/clk-provider.h>

#include "clk.h"

#define STARFIVE_CLK_ENABLE_SHIFT	31 /* [31] */
#define STARFIVE_CLK_INVERT_SHIFT	30 /* [30] */
#define STARFIVE_CLK_MUX_SHIFT		24 /* [29:24] */
#define STARFIVE_CLK_DIV_SHIFT		0  /* [23:0] */

#define OFFSET(id) ((id) * 4)

#define JH7110_SYS_ID_TRANS(id) ((id) + JH7110_PLLCLK_END + JH7110_EXTCLK_END)
#define JH7110_AON_ID_TRANS(id) ((id) + JH7110_SYS_ID_TRANS(JH7110_SYSCLK_END))
#define JH7110_STG_ID_TRANS(id) ((id) + JH7110_AON_ID_TRANS(JH7110_AONCLK_END))

typedef int (*jh1710_init_fn)(struct udevice *dev);

struct jh7110_clk_priv {
	void __iomem *reg;
	jh1710_init_fn init;
};

static const char *cpu_root_sels[2] = {
	[0] = "oscillator",
	[1] = "pll0_out",
};

static const char *perh_root_sels[2] = {
	[0] = "pll0_out",
	[1] = "pll2_out",
};

static const char *bus_root_sels[2] = {
	[0] = "oscillator",
	[1] = "pll2_out",
};

static const char *qspi_ref_sels[2] = {
	[0] = "oscillator",
	[1] = "qspi_ref_src",
};

static const char *gmac1_tx_sels[2] = {
	[0] = "gmac1_gtxclk",
	[1] = "gmac1_rmii_rtx",
};

static const char *gmac0_tx_sels[2] = {
	[0] = "gmac0_gtxclk",
	[1] = "gmac0_rmii_rtx",
};

static const char *apb_func_sels[2] = {
	[0] = "osc_div4",
	[1] = "oscillator",
};

static const char *gmac1_rx_sels[2] = {
	[0] = "gmac1-rgmii-rxin-clock",
	[1] = "gmac1_rmii_rtx",
};

static struct clk *starfive_clk_mux(void __iomem *reg,
				    const char *name,
				    unsigned int offset,
				    u8 width,
				    const char * const *parent_names,
				    u8 num_parents)
{
	return  clk_register_mux(NULL, name, parent_names, num_parents, 0,
				reg + offset, STARFIVE_CLK_MUX_SHIFT,
				width, 0);
}

static struct clk *starfive_clk_gate(void __iomem *reg,
				     const char *name,
				     const char *parent_name,
				     unsigned int offset)
{
	return clk_register_gate(NULL, name, parent_name, 0, reg + offset,
				STARFIVE_CLK_ENABLE_SHIFT, 0, NULL);
}

static struct clk *starfive_clk_inv(void __iomem *reg,
				    const char *name,
				    const char *parent_name,
				    unsigned int offset)
{
	return clk_register_gate(NULL, name, parent_name, 0, reg + offset,
				STARFIVE_CLK_INVERT_SHIFT, 0, NULL);
}

static struct clk *starfive_clk_divider(void __iomem *reg,
					const char *name,
					const char *parent_name,
					unsigned int offset,
					u8 width)
{
	return clk_register_divider(NULL, name, parent_name, 0, reg + offset,
				0, width, CLK_DIVIDER_ONE_BASED);
}

static struct clk *starfive_clk_composite(void __iomem *reg,
					  const char *name,
					  const char * const *parent_names,
					  unsigned int num_parents,
					  unsigned int offset,
					  unsigned int mux_width,
					  unsigned int gate_width,
					  unsigned int div_width)
{
	struct clk *clk = ERR_PTR(-ENOMEM);
	struct clk_divider *div = NULL;
	struct clk_gate *gate = NULL;
	struct clk_mux *mux = NULL;
	int mask_arry[4] = {0x1, 0x3, 0x7, 0xF};
	int mask;

	if (mux_width) {
		if (mux_width > 4)
			goto fail;
		else
			mask = mask_arry[mux_width - 1];

		mux = kzalloc(sizeof(*mux), GFP_KERNEL);
		if (!mux)
			goto fail;

		mux->reg = reg + offset;
		mux->mask = mask;
		mux->shift = STARFIVE_CLK_MUX_SHIFT;
		mux->num_parents = num_parents;
		mux->flags = 0;
		mux->parent_names = parent_names;
	}

	if (gate_width) {
		gate = kzalloc(sizeof(*gate), GFP_KERNEL);

		if (!gate)
			goto fail;

		gate->reg = reg + offset;
		gate->bit_idx = STARFIVE_CLK_ENABLE_SHIFT;
		gate->flags = 0;
	}

	if (div_width) {
		div = kzalloc(sizeof(*div), GFP_KERNEL);
		if (!div)
			goto fail;

		div->reg = reg + offset;

		if (offset == OFFSET(JH7110_SYSCLK_UART3_CORE) ||
		    offset == OFFSET(JH7110_SYSCLK_UART4_CORE) ||
		    offset == OFFSET(JH7110_SYSCLK_UART5_CORE)) {
			div->shift = 8;
			div->width = 8;
		} else {
			div->shift = STARFIVE_CLK_DIV_SHIFT;
			div->width = div_width;
		}
		div->flags = CLK_DIVIDER_ONE_BASED;
		div->table = NULL;
	}

	clk = clk_register_composite(NULL, name,
				     parent_names, num_parents,
				     &mux->clk, &clk_mux_ops,
				     &div->clk, &clk_divider_ops,
				     &gate->clk, &clk_gate_ops, 0);

	if (IS_ERR(clk))
		goto fail;

	return clk;

fail:
	kfree(gate);
	kfree(div);
	kfree(mux);
	return ERR_CAST(clk);
}

static struct clk *starfive_clk_fix_parent_composite(void __iomem *reg,
						     const char *name,
						     const char *parent_names,
						     unsigned int offset,
						     unsigned int mux_width,
						     unsigned int gate_width,
						     unsigned int div_width)
{
	const char * const *parents;

	parents  = &parent_names;

	return starfive_clk_composite(reg, name, parents, 1, offset,
			mux_width, gate_width, div_width);
}

static struct clk *starfive_clk_gate_divider(void __iomem *reg,
					     const char *name,
					     const char *parent,
					     unsigned int offset,
					     unsigned int width)
{
	const char * const *parent_names;

	parent_names  = &parent;

	return starfive_clk_composite(reg, name, parent_names, 1,
				offset, 0, 1, width);
}

static int jh7110_syscrg_init(struct udevice *dev)
{
	struct jh7110_clk_priv *priv = dev_get_priv(dev);
	struct clk *pclk;

	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_CPU_ROOT),
	       starfive_clk_mux(priv->reg, "cpu_root",
				OFFSET(JH7110_SYSCLK_CPU_ROOT), 1,
				cpu_root_sels, ARRAY_SIZE(cpu_root_sels)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_CPU_CORE),
	       starfive_clk_divider(priv->reg,
				    "cpu_core", "cpu_root",
				    OFFSET(JH7110_SYSCLK_CPU_CORE), 3));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_CPU_BUS),
	       starfive_clk_divider(priv->reg,
				    "cpu_bus", "cpu_core",
				    OFFSET(JH7110_SYSCLK_CPU_BUS), 2));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_PERH_ROOT),
	       starfive_clk_composite(priv->reg,
				      "perh_root",
				      perh_root_sels, ARRAY_SIZE(perh_root_sels),
				      OFFSET(JH7110_SYSCLK_PERH_ROOT), 1, 0, 2));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_BUS_ROOT),
	       starfive_clk_mux(priv->reg, "bus_root",
				OFFSET(JH7110_SYSCLK_BUS_ROOT), 1,
				bus_root_sels,	ARRAY_SIZE(bus_root_sels)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_NOCSTG_BUS),
	       starfive_clk_divider(priv->reg,
				    "nocstg_bus", "bus_root",
				    OFFSET(JH7110_SYSCLK_NOCSTG_BUS), 3));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_AXI_CFG0),
	       starfive_clk_divider(priv->reg,
				    "axi_cfg0", "bus_root",
				    OFFSET(JH7110_SYSCLK_AXI_CFG0), 2));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_STG_AXIAHB),
	       starfive_clk_divider(priv->reg,
				    "stg_axiahb", "axi_cfg0",
				    OFFSET(JH7110_SYSCLK_STG_AXIAHB), 2));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_AHB0),
	       starfive_clk_gate(priv->reg,
				 "ahb0", "stg_axiahb",
				 OFFSET(JH7110_SYSCLK_AHB0)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_AHB1),
	       starfive_clk_gate(priv->reg,
				 "ahb1", "stg_axiahb",
				 OFFSET(JH7110_SYSCLK_AHB1)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_APB_BUS),
	       starfive_clk_divider(priv->reg,
				    "apb_bus", "stg_axiahb",
				    OFFSET(JH7110_SYSCLK_APB_BUS), 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_APB0),
	       starfive_clk_gate(priv->reg,
				 "apb0", "apb_bus",
				 OFFSET(JH7110_SYSCLK_APB0)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_QSPI_AHB),
	       starfive_clk_gate(priv->reg,
				 "qspi_ahb", "ahb1",
				 OFFSET(JH7110_SYSCLK_QSPI_AHB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_QSPI_APB),
	       starfive_clk_gate(priv->reg,
				 "qspi_apb", "apb_bus",
				 OFFSET(JH7110_SYSCLK_QSPI_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_QSPI_REF_SRC),
	       starfive_clk_divider(priv->reg,
				    "qspi_ref_src", "pll0_out",
				    OFFSET(JH7110_SYSCLK_QSPI_REF_SRC), 5));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_QSPI_REF),
	       starfive_clk_composite(priv->reg,
				      "qspi_ref",
				      qspi_ref_sels, ARRAY_SIZE(qspi_ref_sels),
				      OFFSET(JH7110_SYSCLK_QSPI_REF), 1, 1, 0));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_SDIO0_AHB),
	       starfive_clk_gate(priv->reg,
				 "sdio0_ahb", "ahb0",
				 OFFSET(JH7110_SYSCLK_SDIO0_AHB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_SDIO1_AHB),
	       starfive_clk_gate(priv->reg,
				 "sdio1_ahb", "ahb0",
				 OFFSET(JH7110_SYSCLK_SDIO1_AHB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_SDIO0_SDCARD),
	       starfive_clk_fix_parent_composite(priv->reg,
						 "sdio0_sdcard", "axi_cfg0",
						 OFFSET(JH7110_SYSCLK_SDIO0_SDCARD), 0, 1, 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_SDIO1_SDCARD),
	       starfive_clk_fix_parent_composite(priv->reg,
						 "sdio1_sdcard", "axi_cfg0",
						 OFFSET(JH7110_SYSCLK_SDIO1_SDCARD), 0, 1, 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_USB_125M),
	       starfive_clk_divider(priv->reg,
				    "usb_125m", "pll0_out",
				    OFFSET(JH7110_SYSCLK_USB_125M), 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_NOC_BUS_STG_AXI),
	       starfive_clk_gate(priv->reg,
				 "noc_bus_stg_axi", "nocstg_bus",
				 OFFSET(JH7110_SYSCLK_NOC_BUS_STG_AXI)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_AHB),
	       starfive_clk_gate(priv->reg,
				 "gmac1_ahb", "ahb0",
				 OFFSET(JH7110_SYSCLK_GMAC1_AHB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_AXI),
	       starfive_clk_gate(priv->reg,
				 "gmac1_axi", "stg_axiahb",
				 OFFSET(JH7110_SYSCLK_GMAC1_AXI)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC_SRC),
	       starfive_clk_divider(priv->reg,
				    "gmac_src", "pll0_out",
				    OFFSET(JH7110_SYSCLK_GMAC_SRC), 3));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_GTXCLK),
	       starfive_clk_divider(priv->reg,
				    "gmac1_gtxclk", "pll0_out",
				    OFFSET(JH7110_SYSCLK_GMAC1_GTXCLK), 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_GTXC),
	       starfive_clk_gate(priv->reg,
				 "gmac1_gtxc", "gmac1_gtxclk",
				 OFFSET(JH7110_SYSCLK_GMAC1_GTXC)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_RMII_RTX),
	       starfive_clk_divider(priv->reg,
				    "gmac1_rmii_rtx", "gmac1-rmii-refin-clock",
				    OFFSET(JH7110_SYSCLK_GMAC1_RMII_RTX), 5));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_PTP),
	       starfive_clk_gate_divider(priv->reg,
					 "gmac1_ptp", "gmac_src",
					 OFFSET(JH7110_SYSCLK_GMAC1_PTP), 5));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_RX),
	       starfive_clk_mux(priv->reg, "gmac1_rx",
				OFFSET(JH7110_SYSCLK_GMAC1_RX), 1,
				gmac1_rx_sels,	ARRAY_SIZE(gmac1_rx_sels)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_TX),
	       starfive_clk_composite(priv->reg,
				      "gmac1_tx",
				      gmac1_tx_sels, ARRAY_SIZE(gmac1_tx_sels),
				      OFFSET(JH7110_SYSCLK_GMAC1_TX), 1, 1, 0));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC1_TX_INV),
	       starfive_clk_inv(priv->reg,
				"gmac1_tx_inv", "gmac1_tx",
				OFFSET(JH7110_SYSCLK_GMAC1_TX_INV)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC0_GTXCLK),
	       starfive_clk_gate_divider(priv->reg,
					 "gmac0_gtxclk", "pll0_out",
					 OFFSET(JH7110_SYSCLK_GMAC0_GTXCLK), 4));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC0_PTP),
	       starfive_clk_gate_divider(priv->reg,
					 "gmac0_ptp", "gmac_src",
					 OFFSET(JH7110_SYSCLK_GMAC0_PTP), 5));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_GMAC0_GTXC),
	       starfive_clk_gate(priv->reg,
				 "gmac0_gtxc", "gmac0_gtxclk",
				 OFFSET(JH7110_SYSCLK_GMAC0_GTXC)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART0_APB),
	       starfive_clk_gate(priv->reg,
				 "uart0_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART0_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART0_CORE),
	       starfive_clk_gate(priv->reg,
				 "uart0_core", "oscillator",
				 OFFSET(JH7110_SYSCLK_UART0_CORE)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART1_APB),
	       starfive_clk_gate(priv->reg,
				 "uart1_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART1_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART1_CORE),
	       starfive_clk_gate(priv->reg,
				 "uart1_core", "oscillator",
				 OFFSET(JH7110_SYSCLK_UART1_CORE)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART2_APB),
	       starfive_clk_gate(priv->reg,
				 "uart2_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART2_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART2_CORE),
	       starfive_clk_gate(priv->reg,
				 "uart2_core", "oscillator",
				 OFFSET(JH7110_SYSCLK_UART2_CORE)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART3_APB),
	       starfive_clk_gate(priv->reg,
				 "uart3_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART3_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART3_CORE),
	       starfive_clk_gate_divider(priv->reg,
					 "uart3_core", "perh_root",
					 OFFSET(JH7110_SYSCLK_UART3_CORE), 8));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART4_APB),
	       starfive_clk_gate(priv->reg,
				 "uart4_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART4_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART4_CORE),
	       starfive_clk_gate_divider(priv->reg,
					 "uart4_core", "perh_root",
					 OFFSET(JH7110_SYSCLK_UART4_CORE), 8));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART5_APB),
	       starfive_clk_gate(priv->reg,
				 "uart5_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_UART5_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_UART5_CORE),
	       starfive_clk_gate_divider(priv->reg,
					 "uart5_core", "perh_root",
					 OFFSET(JH7110_SYSCLK_UART5_CORE), 8));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_I2C2_APB),
	       starfive_clk_gate(priv->reg,
				 "i2c2_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_I2C2_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_I2C5_APB),
	       starfive_clk_gate(priv->reg,
				 "i2c5_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_I2C5_APB)));
	/* Watchdog clocks */
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_WDT_APB),
	       starfive_clk_gate(priv->reg,
				 "wdt_apb", "apb0",
				 OFFSET(JH7110_SYSCLK_WDT_APB)));
	clk_dm(JH7110_SYS_ID_TRANS(JH7110_SYSCLK_WDT_CORE),
	       starfive_clk_gate(priv->reg,
				 "wdt_core", "oscillator",
				 OFFSET(JH7110_SYSCLK_WDT_CORE)));

	/* enable noc_bus_stg_axi clock */
	if (!clk_get_by_id(JH7110_SYSCLK_NOC_BUS_STG_AXI, &pclk))
		clk_enable(pclk);

	return 0;
}

static int jh7110_aoncrg_init(struct udevice *dev)
{
	struct jh7110_clk_priv *priv = dev_get_priv(dev);

	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_OSC_DIV4),
	       starfive_clk_divider(priv->reg,
				    "osc_div4", "oscillator",
				    OFFSET(JH7110_AONCLK_OSC_DIV4), 5));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_APB_FUNC),
	       starfive_clk_mux(priv->reg, "apb_func",
				OFFSET(JH7110_AONCLK_APB_FUNC), 1,
				apb_func_sels,	ARRAY_SIZE(apb_func_sels)));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_GMAC0_AHB),
	       starfive_clk_gate(priv->reg,
				 "gmac0_ahb", "stg_axiahb",
				 OFFSET(JH7110_AONCLK_GMAC0_AHB)));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_GMAC0_AXI),
	       starfive_clk_gate(priv->reg,
				 "gmac0_axi", "stg_axiahb",
				 OFFSET(JH7110_AONCLK_GMAC0_AXI)));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_GMAC0_RMII_RTX),
	       starfive_clk_divider(priv->reg,
				    "gmac0_rmii_rtx", "gmac0-rmii-refin-clock",
				    OFFSET(JH7110_AONCLK_GMAC0_RMII_RTX), 5));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_GMAC0_TX),
	       starfive_clk_composite(priv->reg,
				      "gmac0_tx", gmac0_tx_sels,
				      ARRAY_SIZE(gmac0_tx_sels),
				      OFFSET(JH7110_AONCLK_GMAC0_TX), 1, 1, 0));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_GMAC0_TX_INV),
	       starfive_clk_inv(priv->reg,
				"gmac0_tx_inv", "gmac0_tx",
				OFFSET(JH7110_AONCLK_GMAC0_TX_INV)));
	clk_dm(JH7110_AON_ID_TRANS(JH7110_AONCLK_OTPC_APB),
	       starfive_clk_gate(priv->reg,
				 "otpc_apb", "apb_bus",
				 OFFSET(JH7110_AONCLK_OTPC_APB)));

	return 0;
}

static int jh7110_stgcrg_init(struct udevice *dev)
{
	struct jh7110_clk_priv *priv = dev_get_priv(dev);

	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_APB),
	       starfive_clk_gate(priv->reg,
				 "usb_apb", "apb_bus",
				 OFFSET(JH7110_STGCLK_USB_APB)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_UTMI_APB),
	       starfive_clk_gate(priv->reg,
				 "usb_utmi_apb", "apb_bus",
				 OFFSET(JH7110_STGCLK_USB_UTMI_APB)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_AXI),
	       starfive_clk_gate(priv->reg,
				 "usb_axi", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_USB_AXI)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_LPM),
	       starfive_clk_gate_divider(priv->reg,
					 "usb_lpm", "oscillator",
					 OFFSET(JH7110_STGCLK_USB_LPM), 2));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_STB),
	       starfive_clk_gate_divider(priv->reg,
					 "usb_stb", "oscillator",
					 OFFSET(JH7110_STGCLK_USB_STB), 3));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_APP_125),
	       starfive_clk_gate(priv->reg,
				 "usb_app_125", "usb_125m",
				 OFFSET(JH7110_STGCLK_USB_APP_125)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_USB_REFCLK),
	       starfive_clk_divider(priv->reg, "usb_refclk", "oscillator",
				    OFFSET(JH7110_STGCLK_USB_REFCLK), 2));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE0_AXI),
	       starfive_clk_gate(priv->reg,
				 "pcie0_axi", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_PCIE0_AXI)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE0_APB),
	       starfive_clk_gate(priv->reg,
				 "pcie0_apb", "apb_bus",
				 OFFSET(JH7110_STGCLK_PCIE0_APB)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE0_TL),
	       starfive_clk_gate(priv->reg,
				 "pcie0_tl", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_PCIE0_TL)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE1_AXI),
	       starfive_clk_gate(priv->reg,
				 "pcie1_axi", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_PCIE1_AXI)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE1_APB),
	       starfive_clk_gate(priv->reg,
				 "pcie1_apb", "apb_bus",
				 OFFSET(JH7110_STGCLK_PCIE1_APB)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_PCIE1_TL),
	       starfive_clk_gate(priv->reg,
				 "pcie1_tl", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_PCIE1_TL)));

	/* Security clocks */
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_SEC_HCLK),
	       starfive_clk_gate(priv->reg,
				 "sec_ahb", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_SEC_HCLK)));
	clk_dm(JH7110_STG_ID_TRANS(JH7110_STGCLK_SEC_MISCAHB),
	       starfive_clk_gate(priv->reg,
				 "sec_misc_ahb", "stg_axiahb",
				 OFFSET(JH7110_STGCLK_SEC_MISCAHB)));

	return 0;
}

static int jh7110_clk_probe(struct udevice *dev)
{
	struct jh7110_clk_priv *priv = dev_get_priv(dev);

	priv->init = (jh1710_init_fn)dev_get_driver_data(dev);
	priv->reg =  (void __iomem *)dev_read_addr_ptr(dev);

	if (priv->init)
		return priv->init(dev);

	return 0;
}

static int jh7110_clk_bind(struct udevice *dev)
{
	/* The reset driver does not have a device node, so bind it here */
	return device_bind_driver_to_node(dev, "jh7110_reset", dev->name,
							dev_ofnode(dev), NULL);
}

static int jh7110_sys_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = JH7110_SYS_ID_TRANS(args->args[0]);
	else
		clk->id = 0;

	return 0;
}

static int jh7110_aon_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = JH7110_AON_ID_TRANS(args->args[0]);
	else
		clk->id = 0;

	return 0;
}

static int jh7110_stg_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	if (args->args_count > 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		clk->id = JH7110_STG_ID_TRANS(args->args[0]);
	else
		clk->id = 0;

	return 0;
}

static const struct udevice_id jh7110_sys_clk_of_match[] = {
	{ .compatible = "starfive,jh7110-syscrg",
	  .data = (ulong)&jh7110_syscrg_init
	},
	{ }
};

JH7110_CLK_OPS(sys);

U_BOOT_DRIVER(jh7110_sys_clk) = {
	.name = "jh7110_sys_clk",
	.id = UCLASS_CLK,
	.of_match = jh7110_sys_clk_of_match,
	.probe = jh7110_clk_probe,
	.ops = &jh7110_sys_clk_ops,
	.priv_auto = sizeof(struct jh7110_clk_priv),
	.bind = jh7110_clk_bind,
};

static const struct udevice_id jh7110_aon_clk_of_match[] = {
	{ .compatible = "starfive,jh7110-aoncrg",
	  .data = (ulong)&jh7110_aoncrg_init
	},
	{ }
};

JH7110_CLK_OPS(aon);

U_BOOT_DRIVER(jh7110_aon_clk) = {
	.name = "jh7110_aon_clk",
	.id = UCLASS_CLK,
	.of_match = jh7110_aon_clk_of_match,
	.probe = jh7110_clk_probe,
	.ops = &jh7110_aon_clk_ops,
	.priv_auto = sizeof(struct jh7110_clk_priv),
	.bind = jh7110_clk_bind,
};

static const struct udevice_id jh7110_stg_clk_of_match[] = {
	{ .compatible = "starfive,jh7110-stgcrg",
	  .data = (ulong)&jh7110_stgcrg_init
	},
	{ }
};

JH7110_CLK_OPS(stg);

U_BOOT_DRIVER(jh7110_stg_clk) = {
	.name = "jh7110_stg_clk",
	.id = UCLASS_CLK,
	.of_match = jh7110_stg_clk_of_match,
	.probe = jh7110_clk_probe,
	.ops = &jh7110_stg_clk_ops,
	.priv_auto = sizeof(struct jh7110_clk_priv),
	.bind = jh7110_clk_bind,
};
