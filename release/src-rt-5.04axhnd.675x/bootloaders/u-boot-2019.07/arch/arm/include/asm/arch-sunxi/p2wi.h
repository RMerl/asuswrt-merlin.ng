/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi platform Push-Push i2c register definition.
 *
 * (c) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 * http://linux-sunxi.org
 *
 * (c)Copyright 2006-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#ifndef _SUNXI_P2WI_H
#define _SUNXI_P2WI_H

#include <linux/types.h>

#define P2WI_CTRL_RESET (0x1 << 0)
#define P2WI_CTRL_IRQ_EN (0x1 << 1)
#define P2WI_CTRL_TRANS_ABORT (0x1 << 6)
#define P2WI_CTRL_TRANS_START (0x1 << 7)

#define __P2WI_CC_CLK(n) (((n) & 0xff) << 0)
#define P2WI_CC_CLK_MASK __P2WI_CC_CLK_DIV(0xff)
#define __P2WI_CC_CLK_DIV(n) (((n) >> 1) - 1)
#define P2WI_CC_CLK_DIV(n) \
	__P2WI_CC_CLK(__P2WI_CC_CLK_DIV(n))
#define P2WI_CC_SDA_OUT_DELAY(n) (((n) & 0x7) << 8)
#define P2WI_CC_SDA_OUT_DELAY_MASK P2WI_CC_SDA_OUT_DELAY(0x7)

#define P2WI_IRQ_TRANS_DONE (0x1 << 0)
#define P2WI_IRQ_TRANS_ERR (0x1 << 1)
#define P2WI_IRQ_LOAD_BUSY (0x1 << 2)

#define P2WI_STAT_TRANS_DONE (0x1 << 0)
#define P2WI_STAT_TRANS_ERR (0x1 << 1)
#define P2WI_STAT_LOAD_BUSY (0x1 << 2)
#define __P2WI_STAT_TRANS_ERR(n) (((n) & 0xff) << 8)
#define P2WI_STAT_TRANS_ERR_MASK __P2WI_STAT_TRANS_ERR_ID(0xff)
#define __P2WI_STAT_TRANS_ERR_BYTE_1 0x01
#define __P2WI_STAT_TRANS_ERR_BYTE_2 0x02
#define __P2WI_STAT_TRANS_ERR_BYTE_3 0x04
#define __P2WI_STAT_TRANS_ERR_BYTE_4 0x08
#define __P2WI_STAT_TRANS_ERR_BYTE_5 0x10
#define __P2WI_STAT_TRANS_ERR_BYTE_6 0x20
#define __P2WI_STAT_TRANS_ERR_BYTE_7 0x40
#define __P2WI_STAT_TRANS_ERR_BYTE_8 0x80
#define P2WI_STAT_TRANS_ERR_BYTE_1 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_1)
#define P2WI_STAT_TRANS_ERR_BYTE_2 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_2)
#define P2WI_STAT_TRANS_ERR_BYTE_3 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_3)
#define P2WI_STAT_TRANS_ERR_BYTE_4 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_4)
#define P2WI_STAT_TRANS_ERR_BYTE_5 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_5)
#define P2WI_STAT_TRANS_ERR_BYTE_6 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_6)
#define P2WI_STAT_TRANS_ERR_BYTE_7 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_7)
#define P2WI_STAT_TRANS_ERR_BYTE_8 \
	__P2WI_STAT_TRANS_ERR(__P2WI_STAT_TRANS_ERR_BYTE_8)

#define P2WI_DATADDR_BYTE_1(n) (((n) & 0xff) << 0)
#define P2WI_DATADDR_BYTE_1_MASK P2WI_DATADDR_BYTE_1(0xff)
#define P2WI_DATADDR_BYTE_2(n) (((n) & 0xff) << 8)
#define P2WI_DATADDR_BYTE_2_MASK P2WI_DATADDR_BYTE_2(0xff)
#define P2WI_DATADDR_BYTE_3(n) (((n) & 0xff) << 16)
#define P2WI_DATADDR_BYTE_3_MASK P2WI_DATADDR_BYTE_3(0xff)
#define P2WI_DATADDR_BYTE_4(n) (((n) & 0xff) << 24)
#define P2WI_DATADDR_BYTE_4_MASK P2WI_DATADDR_BYTE_4(0xff)
#define P2WI_DATADDR_BYTE_5(n) (((n) & 0xff) << 0)
#define P2WI_DATADDR_BYTE_5_MASK P2WI_DATADDR_BYTE_5(0xff)
#define P2WI_DATADDR_BYTE_6(n) (((n) & 0xff) << 8)
#define P2WI_DATADDR_BYTE_6_MASK P2WI_DATADDR_BYTE_6(0xff)
#define P2WI_DATADDR_BYTE_7(n) (((n) & 0xff) << 16)
#define P2WI_DATADDR_BYTE_7_MASK P2WI_DATADDR_BYTE_7(0xff)
#define P2WI_DATADDR_BYTE_8(n) (((n) & 0xff) << 24)
#define P2WI_DATADDR_BYTE_8_MASK P2WI_DATADDR_BYTE_8(0xff)

#define __P2WI_DATA_NUM_BYTES(n) (((n) & 0x7) << 0)
#define P2WI_DATA_NUM_BYTES_MASK __P2WI_DATA_NUM_BYTES(0x7)
#define P2WI_DATA_NUM_BYTES(n) __P2WI_DATA_NUM_BYTES((n) - 1)
#define P2WI_DATA_NUM_BYTES_READ (0x1 << 4)

#define P2WI_DATA_BYTE_1(n) (((n) & 0xff) << 0)
#define P2WI_DATA_BYTE_1_MASK P2WI_DATA_BYTE_1(0xff)
#define P2WI_DATA_BYTE_2(n) (((n) & 0xff) << 8)
#define P2WI_DATA_BYTE_2_MASK P2WI_DATA_BYTE_2(0xff)
#define P2WI_DATA_BYTE_3(n) (((n) & 0xff) << 16)
#define P2WI_DATA_BYTE_3_MASK P2WI_DATA_BYTE_3(0xff)
#define P2WI_DATA_BYTE_4(n) (((n) & 0xff) << 24)
#define P2WI_DATA_BYTE_4_MASK P2WI_DATA_BYTE_4(0xff)
#define P2WI_DATA_BYTE_5(n) (((n) & 0xff) << 0)
#define P2WI_DATA_BYTE_5_MASK P2WI_DATA_BYTE_5(0xff)
#define P2WI_DATA_BYTE_6(n) (((n) & 0xff) << 8)
#define P2WI_DATA_BYTE_6_MASK P2WI_DATA_BYTE_6(0xff)
#define P2WI_DATA_BYTE_7(n) (((n) & 0xff) << 16)
#define P2WI_DATA_BYTE_7_MASK P2WI_DATA_BYTE_7(0xff)
#define P2WI_DATA_BYTE_8(n) (((n) & 0xff) << 24)
#define P2WI_DATA_BYTE_8_MASK P2WI_DATA_BYTE_8(0xff)

#define P2WI_LINECTRL_SDA_CTRL_EN (0x1 << 0)
#define P2WI_LINECTRL_SDA_OUT_HIGH (0x1 << 1)
#define P2WI_LINECTRL_SCL_CTRL_EN (0x1 << 2)
#define P2WI_LINECTRL_SCL_OUT_HIGH (0x1 << 3)
#define P2WI_LINECTRL_SDA_STATE_HIGH (0x1 << 4)
#define P2WI_LINECTRL_SCL_STATE_HIGH (0x1 << 5)

#define P2WI_PM_DEV_ADDR(n) (((n) & 0xff) << 0)
#define P2WI_PM_DEV_ADDR_MASK P2WI_PM_DEV_ADDR(0xff)
#define P2WI_PM_CTRL_ADDR(n) (((n) & 0xff) << 8)
#define P2WI_PM_CTRL_ADDR_MASK P2WI_PM_CTRL_ADDR(0xff)
#define P2WI_PM_INIT_DATA(n) (((n) & 0xff) << 16)
#define P2WI_PM_INIT_DATA_MASK P2WI_PM_INIT_DATA(0xff)
#define P2WI_PM_INIT_SEND (0x1 << 31)

struct sunxi_p2wi_reg {
	u32 ctrl;	/* 0x00 control */
	u32 cc;		/* 0x04 clock control */
	u32 irq;	/* 0x08 interrupt */
	u32 status;	/* 0x0c status */
	u32 dataddr0;	/* 0x10 data address 0 */
	u32 dataddr1;	/* 0x14 data address 1 */
	u32 numbytes;	/* 0x18 num bytes */
	u32 data0;	/* 0x1c data buffer 0 */
	u32 data1;	/* 0x20 data buffer 1 */
	u32 linectrl;	/* 0x24 line control */
	u32 pm;		/* 0x28 power management */
};

void p2wi_init(void);
int p2wi_change_to_p2wi_mode(u8 slave_addr, u8 ctrl_reg, u8 init_data);
int p2wi_read(const u8 addr, u8 *data);
int p2wi_write(const u8 addr, u8 data);

#endif /* _SUNXI_P2WI_H */
