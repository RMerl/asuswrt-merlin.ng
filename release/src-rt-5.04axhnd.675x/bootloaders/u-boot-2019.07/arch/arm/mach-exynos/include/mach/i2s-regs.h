/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
 */

#ifndef __I2S_REGS_H__
#define __I2S_REGS_H__

#define CON_RESET		(1 << 31)
#define CON_TXFIFO_FULL		(1 << 8)
#define CON_TXCH_PAUSE		(1 << 4)
#define CON_ACTIVE		(1 << 0)

#define MOD_OP_CLK		(3 << 30)
#define MOD_BLCP_SHIFT		24
#define MOD_BLCP_16BIT		(0 << MOD_BLCP_SHIFT)
#define MOD_BLCP_8BIT		(1 << MOD_BLCP_SHIFT)
#define MOD_BLCP_24BIT		(2 << MOD_BLCP_SHIFT)
#define MOD_BLCP_MASK		(3 << MOD_BLCP_SHIFT)

#define MOD_BLC_16BIT		(0 << 13)
#define MOD_BLC_8BIT		(1 << 13)
#define MOD_BLC_24BIT		(2 << 13)
#define MOD_BLC_MASK		(3 << 13)

#define MOD_SLAVE		(1 << 11)
#define MOD_RCLKSRC		(0 << 10)
#define MOD_MASK		(3 << 8)
#define MOD_LR_LLOW		(0 << 7)
#define MOD_LR_RLOW		(1 << 7)
#define MOD_SDF_IIS		(0 << 5)
#define MOD_SDF_MSB		(1 << 5)
#define MOD_SDF_LSB		(2 << 5)
#define MOD_SDF_MASK		(3 << 5)
#define MOD_RCLK_256FS		(0 << 3)
#define MOD_RCLK_512FS		(1 << 3)
#define MOD_RCLK_384FS		(2 << 3)
#define MOD_RCLK_768FS		(3 << 3)
#define MOD_RCLK_MASK		(3 << 3)
#define MOD_BCLK_32FS		(0 << 1)
#define MOD_BCLK_48FS		(1 << 1)
#define MOD_BCLK_16FS		(2 << 1)
#define MOD_BCLK_24FS		(3 << 1)
#define MOD_BCLK_MASK		(3 << 1)

#define MOD_CDCLKCON		(1 << 12)

#define FIC_TXFLUSH		(1 << 15)
#define FIC_RXFLUSH		(1 << 7)

#define PSREN			(1 << 15)
#define PSVAL			(3 << 8)

#endif /* __I2S_REGS_H__ */
