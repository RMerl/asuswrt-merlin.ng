/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#define DWMCI_CLKSEL		0x09C
#define DWMCI_SET_SAMPLE_CLK(x)	(x)
#define DWMCI_SET_DRV_CLK(x)	((x) << 16)
#define DWMCI_SET_DIV_RATIO(x)	((x) << 24)

#define EMMCP_MPSBEGIN0		0x1200
#define EMMCP_SEND0		0x1204
#define EMMCP_CTRL0		0x120C

#define MPSCTRL_SECURE_READ_BIT		(0x1<<7)
#define MPSCTRL_SECURE_WRITE_BIT	(0x1<<6)
#define MPSCTRL_NON_SECURE_READ_BIT	(0x1<<5)
#define MPSCTRL_NON_SECURE_WRITE_BIT	(0x1<<4)
#define MPSCTRL_USE_FUSE_KEY		(0x1<<3)
#define MPSCTRL_ECB_MODE		(0x1<<2)
#define MPSCTRL_ENCRYPTION		(0x1<<1)
#define MPSCTRL_VALID			(0x1<<0)

/* CLKSEL Register */
#define DWMCI_DIVRATIO_BIT		24
#define DWMCI_DIVRATIO_MASK		0x7
