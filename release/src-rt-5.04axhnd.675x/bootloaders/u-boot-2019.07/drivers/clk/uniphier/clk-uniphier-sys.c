// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include "clk-uniphier.h"

#define UNIPHIER_LD4_SYS_CLK_NAND(_id)					\
	UNIPHIER_CLK_RATE(128, 50000000),				\
	UNIPHIER_CLK_GATE((_id), 128, 0x2104, 2)

#define UNIPHIER_LD11_SYS_CLK_NAND(_id)					\
	UNIPHIER_CLK_RATE(128, 50000000),				\
	UNIPHIER_CLK_GATE((_id), 128, 0x210c, 0)

const struct uniphier_clk_data uniphier_pxs2_sys_clk_data[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_SLD8) ||\
    defined(CONFIG_ARCH_UNIPHIER_PRO4) || defined(CONFIG_ARCH_UNIPHIER_PRO5) ||\
    defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	UNIPHIER_LD4_SYS_CLK_NAND(2),
	UNIPHIER_CLK_RATE(3, 200000000),
	UNIPHIER_CLK_GATE_SIMPLE(6, 0x2104, 12),	/* ether (Pro4, PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(7, 0x2104, 5),		/* ether-gb (Pro4) */
	UNIPHIER_CLK_GATE_SIMPLE(8, 0x2104, 10),	/* stdmac */
	UNIPHIER_CLK_GATE_SIMPLE(10, 0x2260, 0),	/* ether-phy (Pro4) */
	UNIPHIER_CLK_GATE_SIMPLE(12, 0x2104, 6),	/* gio (Pro4, Pro5) */
	UNIPHIER_CLK_GATE_SIMPLE(14, 0x2104, 16),	/* usb30 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(15, 0x2104, 17),	/* usb31 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(16, 0x2104, 19),	/* usb30-phy (PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(20, 0x2104, 20),	/* usb31-phy (PXs2) */
	{ /* sentinel */ }
#endif
};

const struct uniphier_clk_data uniphier_ld20_sys_clk_data[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	UNIPHIER_LD11_SYS_CLK_NAND(2),
	UNIPHIER_CLK_RATE(3, 200000000),
	UNIPHIER_CLK_GATE_SIMPLE(6, 0x210c, 6),		/* ether */
	UNIPHIER_CLK_GATE_SIMPLE(8, 0x210c, 8),		/* stdmac */
	UNIPHIER_CLK_GATE_SIMPLE(14, 0x210c, 14),	/* usb30 (LD20) */
	UNIPHIER_CLK_GATE_SIMPLE(16, 0x210c, 12),	/* usb30-phy0 (LD20) */
	UNIPHIER_CLK_GATE_SIMPLE(17, 0x210c, 13),	/* usb30-phy1 (LD20) */
	{ /* sentinel */ }
#endif
};

const struct uniphier_clk_data uniphier_pxs3_sys_clk_data[] = {
#if defined(CONFIG_ARCH_UNIPHIER_PXS3)
	UNIPHIER_LD11_SYS_CLK_NAND(2),
	UNIPHIER_CLK_RATE(3, 200000000),
	UNIPHIER_CLK_GATE_SIMPLE(6, 0x210c, 9),		/* ether0 */
	UNIPHIER_CLK_GATE_SIMPLE(7, 0x210c, 10),	/* ether1 */
	UNIPHIER_CLK_GATE_SIMPLE(12, 0x210c, 4),	/* usb30 (gio0) */
	UNIPHIER_CLK_GATE_SIMPLE(13, 0x210c, 5),	/* usb31-0 (gio1) */
	UNIPHIER_CLK_GATE_SIMPLE(14, 0x210c, 6),	/* usb31-1 (gio1-1) */
	UNIPHIER_CLK_GATE_SIMPLE(16, 0x210c, 16),	/* usb30-phy0 */
	UNIPHIER_CLK_GATE_SIMPLE(17, 0x210c, 18),	/* usb30-phy1 */
	UNIPHIER_CLK_GATE_SIMPLE(18, 0x210c, 20),	/* usb30-phy2 */
	UNIPHIER_CLK_GATE_SIMPLE(20, 0x210c, 17),	/* usb31-phy0 */
	UNIPHIER_CLK_GATE_SIMPLE(21, 0x210c, 19),	/* usb31-phy1 */
	{ /* sentinel */ }
#endif
};
