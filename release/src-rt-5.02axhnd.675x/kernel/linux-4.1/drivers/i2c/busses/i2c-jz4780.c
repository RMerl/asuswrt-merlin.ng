/*
 * Ingenic JZ4780 I2C bus driver
 *
 * Copyright (C) 2006 - 2009 Ingenic Semiconductor Inc.
 * Copyright (C) 2015 Imagination Technologies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/time.h>

#define JZ4780_I2C_CTRL		0x00
#define JZ4780_I2C_TAR		0x04
#define JZ4780_I2C_SAR		0x08
#define JZ4780_I2C_DC		0x10
#define JZ4780_I2C_SHCNT	0x14
#define JZ4780_I2C_SLCNT	0x18
#define JZ4780_I2C_FHCNT	0x1C
#define JZ4780_I2C_FLCNT	0x20
#define JZ4780_I2C_INTST	0x2C
#define JZ4780_I2C_INTM		0x30
#define JZ4780_I2C_RXTL		0x38
#define JZ4780_I2C_TXTL		0x3C
#define JZ4780_I2C_CINTR	0x40
#define JZ4780_I2C_CRXUF	0x44
#define JZ4780_I2C_CRXOF	0x48
#define JZ4780_I2C_CTXOF	0x4C
#define JZ4780_I2C_CRXREQ	0x50
#define JZ4780_I2C_CTXABRT	0x54
#define JZ4780_I2C_CRXDONE	0x58
#define JZ4780_I2C_CACT		0x5C
#define JZ4780_I2C_CSTP		0x60
#define JZ4780_I2C_CSTT		0x64
#define JZ4780_I2C_CGC		0x68
#define JZ4780_I2C_ENB		0x6C
#define JZ4780_I2C_STA		0x70
#define JZ4780_I2C_TXABRT	0x80
#define JZ4780_I2C_DMACR	0x88
#define JZ4780_I2C_DMATDLR	0x8C
#define JZ4780_I2C_DMARDLR	0x90
#define JZ4780_I2C_SDASU	0x94
#define JZ4780_I2C_ACKGC	0x98
#define JZ4780_I2C_ENSTA	0x9C
#define JZ4780_I2C_SDAHD	0xD0

#define JZ4780_I2C_CTRL_STPHLD		BIT(7)
#define JZ4780_I2C_CTRL_SLVDIS		BIT(6)
#define JZ4780_I2C_CTRL_REST		BIT(5)
#define JZ4780_I2C_CTRL_MATP		BIT(4)
#define JZ4780_I2C_CTRL_SATP		BIT(3)
#define JZ4780_I2C_CTRL_SPDF		BIT(2)
#define JZ4780_I2C_CTRL_SPDS		BIT(1)
#define JZ4780_I2C_CTRL_MD		BIT(0)

#define JZ4780_I2C_STA_SLVACT		BIT(6)
#define JZ4780_I2C_STA_MSTACT		BIT(5)
#define JZ4780_I2C_STA_RFF		BIT(4)
#define JZ4780_I2C_STA_RFNE		BIT(3)
#define JZ4780_I2C_STA_TFE		BIT(2)
#define JZ4780_I2C_STA_TFNF		BIT(1)
#define JZ4780_I2C_STA_ACT		BIT(0)

static const char * const jz4780_i2c_abrt_src[] = {
	"ABRT_7B_ADDR_NOACK",
	"ABRT_10ADDR1_NOACK",
	"ABRT_10ADDR2_NOACK",
	"ABRT_XDATA_NOACK",
	"ABRT_GCALL_NOACK",
	"ABRT_GCALL_READ",
	"ABRT_HS_ACKD",
	"SBYTE_ACKDET",
	"ABRT_HS_NORSTRT",
	"SBYTE_NORSTRT",
	"ABRT_10B_RD_NORSTRT",
	"ABRT_MASTER_DIS",
	"ARB_LOST",
	"SLVFLUSH_TXFIFO",
	"SLV_ARBLOST",
	"SLVRD_INTX",
};

#define JZ4780_I2C_INTST_IGC		BIT(11)
#define JZ4780_I2C_INTST_ISTT		BIT(10)
#define JZ4780_I2C_INTST_ISTP		BIT(9)
#define JZ4780_I2C_INTST_IACT		BIT(8)
#define JZ4780_I2C_INTST_RXDN		BIT(7)
#define JZ4780_I2C_INTST_TXABT		BIT(6)
#define JZ4780_I2C_INTST_RDREQ		BIT(5)
#define JZ4780_I2C_INTST_TXEMP		BIT(4)
#define JZ4780_I2C_INTST_TXOF		BIT(3)
#define JZ4780_I2C_INTST_RXFL		BIT(2)
#define JZ4780_I2C_INTST_RXOF		BIT(1)
#define JZ4780_I2C_INTST_RXUF		BIT(0)

#define JZ4780_I2C_INTM_MIGC		BIT(11)
#define JZ4780_I2C_INTM_MISTT		BIT(10)
#define JZ4780_I2C_INTM_MISTP		BIT(9)
#define JZ4780_I2C_INTM_MIACT		BIT(8)
#define JZ4780_I2C_INTM_MRXDN		BIT(7)
#define JZ4780_I2C_INTM_MTXABT		BIT(6)
#define JZ4780_I2C_INTM_MRDREQ		BIT(5)
#define JZ4780_I2C_INTM_MTXEMP		BIT(4)
#define JZ4780_I2C_INTM_MTXOF		BIT(3)
#define JZ4780_I2C_INTM_MRXFL		BIT(2)
#define JZ4780_I2C_INTM_MRXOF		BIT(1)
#define JZ4780_I2C_INTM_MRXUF		BIT(0)

#define JZ4780_I2C_DC_READ		BIT(8)

#define JZ4780_I2C_SDAHD_HDENB		BIT(8)

#define JZ4780_I2C_ENB_I2C		BIT(0)

#define JZ4780_I2CSHCNT_ADJUST(n)	(((n) - 8) < 6 ? 6 : ((n) - 8))
#define JZ4780_I2CSLCNT_ADJUST(n)	(((n) - 1) < 8 ? 8 : ((n) - 1))
#define JZ4780_I2CFHCNT_ADJUST(n)	(((n) - 8) < 6 ? 6 : ((n) - 8))
#define JZ4780_I2CFLCNT_ADJUST(n)	(((n) - 1) < 8 ? 8 : ((n) - 1))

#define JZ4780_I2C_FIFO_LEN	16
#define TX_LEVEL		3
#define RX_LEVEL		(JZ4780_I2C_FIFO_LEN - TX_LEVEL - 1)

#define JZ4780_I2C_TIMEOUT	300

#define BUFSIZE 200

struct jz4780_i2c {
	void __iomem		*iomem;
	int			 irq;
	struct clk		*clk;
	struct i2c_adapter	 adap;

	/* lock to protect rbuf and wbuf between xfer_rd/wr and irq handler */
	spinlock_t		lock;

	/* beginning of lock scope */
	unsigned char		*rbuf;
	int			rd_total_len;
	int			rd_data_xfered;
	int			rd_cmd_xfered;

	unsigned char		*wbuf;
	int			wt_len;

	int			is_write;
	int			stop_hold;
	int			speed;

	int			data_buf[BUFSIZE];
	int			cmd_buf[BUFSIZE];
	int			cmd;

	/* end of lock scope */
	struct completion	trans_waitq;
};

static inline unsigned short jz4780_i2c_readw(struct jz4780_i2c *i2c,
					      unsigned long offset)
{
	return readw(i2c->iomem + offset);
}

static inline void jz4780_i2c_writew(struct jz4780_i2c *i2c,
				     unsigned long offset, unsigned short val)
{
	writew(val, i2c->iomem + offset);
}

static int jz4780_i2c_disable(struct jz4780_i2c *i2c)
{
	unsigned short regval;
	unsigned long loops = 5;

	jz4780_i2c_writew(i2c, JZ4780_I2C_ENB, 0);

	do {
		regval = jz4780_i2c_readw(i2c, JZ4780_I2C_ENSTA);
		if (!(regval & JZ4780_I2C_ENB_I2C))
			return 0;

		usleep_range(5000, 15000);
	} while (--loops);

	dev_err(&i2c->adap.dev, "disable failed: ENSTA=0x%04x\n", regval);
	return -ETIMEDOUT;
}

static int jz4780_i2c_enable(struct jz4780_i2c *i2c)
{
	unsigned short regval;
	unsigned long loops = 5;

	jz4780_i2c_writew(i2c, JZ4780_I2C_ENB, 1);

	do {
		regval = jz4780_i2c_readw(i2c, JZ4780_I2C_ENSTA);
		if (regval & JZ4780_I2C_ENB_I2C)
			return 0;

		usleep_range(5000, 15000);
	} while (--loops);

	dev_err(&i2c->adap.dev, "enable failed: ENSTA=0x%04x\n", regval);
	return -ETIMEDOUT;
}

static int jz4780_i2c_set_target(struct jz4780_i2c *i2c, unsigned char address)
{
	unsigned short regval;
	unsigned long loops = 5;

	do {
		regval = jz4780_i2c_readw(i2c, JZ4780_I2C_STA);
		if ((regval & JZ4780_I2C_STA_TFE) &&
		    !(regval & JZ4780_I2C_STA_MSTACT))
			break;

		usleep_range(5000, 15000);
	} while (--loops);

	if (loops) {
		jz4780_i2c_writew(i2c, JZ4780_I2C_TAR, address);
		return 0;
	}

	dev_err(&i2c->adap.dev,
		"set device to address 0x%02x failed, STA=0x%04x\n",
		address, regval);

	return -ENXIO;
}

static int jz4780_i2c_set_speed(struct jz4780_i2c *i2c)
{
	int dev_clk_khz = clk_get_rate(i2c->clk) / 1000;
	int cnt_high = 0;	/* HIGH period count of the SCL clock */
	int cnt_low = 0;	/* LOW period count of the SCL clock */
	int cnt_period = 0;	/* period count of the SCL clock */
	int setup_time = 0;
	int hold_time = 0;
	unsigned short tmp = 0;
	int i2c_clk = i2c->speed;

	if (jz4780_i2c_disable(i2c))
		dev_dbg(&i2c->adap.dev, "i2c not disabled\n");

	/*
	 * 1 JZ4780_I2C cycle equals to cnt_period PCLK(i2c_clk)
	 * standard mode, min LOW and HIGH period are 4700 ns and 4000 ns
	 * fast mode, min LOW and HIGH period are 1300 ns and 600 ns
	 */
	cnt_period = dev_clk_khz / i2c_clk;

	if (i2c_clk <= 100)
		cnt_high = (cnt_period * 4000) / (4700 + 4000);
	else
		cnt_high = (cnt_period * 600) / (1300 + 600);

	cnt_low = cnt_period - cnt_high;

	/*
	 * NOTE: JZ4780_I2C_CTRL_REST can't set when i2c enabled, because
	 * normal read are 2 messages, we cannot disable i2c controller
	 * between these two messages, this means that we must always set
	 * JZ4780_I2C_CTRL_REST when init JZ4780_I2C_CTRL
	 *
	 */
	if (i2c_clk <= 100) {
		tmp = JZ4780_I2C_CTRL_SPDS | JZ4780_I2C_CTRL_REST
		      | JZ4780_I2C_CTRL_SLVDIS | JZ4780_I2C_CTRL_MD;
		jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

		jz4780_i2c_writew(i2c, JZ4780_I2C_SHCNT,
				  JZ4780_I2CSHCNT_ADJUST(cnt_high));
		jz4780_i2c_writew(i2c, JZ4780_I2C_SLCNT,
				  JZ4780_I2CSLCNT_ADJUST(cnt_low));
	} else {
		tmp = JZ4780_I2C_CTRL_SPDF | JZ4780_I2C_CTRL_REST
		      | JZ4780_I2C_CTRL_SLVDIS | JZ4780_I2C_CTRL_MD;
		jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

		jz4780_i2c_writew(i2c, JZ4780_I2C_FHCNT,
				  JZ4780_I2CFHCNT_ADJUST(cnt_high));
		jz4780_i2c_writew(i2c, JZ4780_I2C_FLCNT,
				  JZ4780_I2CFLCNT_ADJUST(cnt_low));
	}

	/*
	 * a i2c device must internally provide a hold time at least 300ns
	 * tHD:DAT
	 *	Standard Mode: min=300ns, max=3450ns
	 *	Fast Mode: min=0ns, max=900ns
	 * tSU:DAT
	 *	Standard Mode: min=250ns, max=infinite
	 *	Fast Mode: min=100(250ns is recommended), max=infinite
	 *
	 * 1i2c_clk = 10^6 / dev_clk_khz
	 * on FPGA, dev_clk_khz = 12000, so 1i2c_clk = 1000/12 = 83ns
	 * on Pisces(1008M), dev_clk_khz=126000, so 1i2c_clk = 1000 / 126 = 8ns
	 *
	 * The actual hold time is (SDAHD + 1) * (i2c_clk period).
	 *
	 * Length of setup time calculated using (SDASU - 1) * (ic_clk_period)
	 *
	 */
	if (i2c_clk <= 100) { /* standard mode */
		setup_time = 300;
		hold_time = 400;
	} else {
		setup_time = 450;
		hold_time = 450;
	}

	hold_time = ((hold_time * dev_clk_khz) / 1000000) - 1;
	setup_time = ((setup_time * dev_clk_khz) / 1000000)  + 1;

	if (setup_time > 255)
		setup_time = 255;

	if (setup_time <= 0)
		setup_time = 1;

	jz4780_i2c_writew(i2c, JZ4780_I2C_SDASU, setup_time);

	if (hold_time > 255)
		hold_time = 255;

	if (hold_time >= 0) {
		/*i2c hold time enable */
		hold_time |= JZ4780_I2C_SDAHD_HDENB;
		jz4780_i2c_writew(i2c, JZ4780_I2C_SDAHD, hold_time);
	} else {
		/* disable hold time */
		jz4780_i2c_writew(i2c, JZ4780_I2C_SDAHD, 0);
	}

	return 0;
}

static int jz4780_i2c_cleanup(struct jz4780_i2c *i2c)
{
	int ret;
	unsigned long flags;
	unsigned short tmp;

	spin_lock_irqsave(&i2c->lock, flags);

	/* can send stop now if need */
	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
	tmp &= ~JZ4780_I2C_CTRL_STPHLD;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

	/* disable all interrupts first */
	jz4780_i2c_writew(i2c, JZ4780_I2C_INTM, 0);

	/* then clear all interrupts */
	jz4780_i2c_readw(i2c, JZ4780_I2C_CTXABRT);
	jz4780_i2c_readw(i2c, JZ4780_I2C_CINTR);

	/* then disable the controller */
	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
	tmp &= ~JZ4780_I2C_ENB_I2C;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);
	udelay(10);
	tmp |= JZ4780_I2C_ENB_I2C;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

	spin_unlock_irqrestore(&i2c->lock, flags);

	ret = jz4780_i2c_disable(i2c);
	if (ret)
		dev_err(&i2c->adap.dev,
			"unable to disable device during cleanup!\n");

	if (unlikely(jz4780_i2c_readw(i2c, JZ4780_I2C_INTM)
		     & jz4780_i2c_readw(i2c, JZ4780_I2C_INTST)))
		dev_err(&i2c->adap.dev,
			"device has interrupts after a complete cleanup!\n");

	return ret;
}

static int jz4780_i2c_prepare(struct jz4780_i2c *i2c)
{
	jz4780_i2c_set_speed(i2c);
	return jz4780_i2c_enable(i2c);
}

static void jz4780_i2c_send_rcmd(struct jz4780_i2c *i2c, int cmd_count)
{
	int i;

	for (i = 0; i < cmd_count; i++)
		jz4780_i2c_writew(i2c, JZ4780_I2C_DC, JZ4780_I2C_DC_READ);
}

static void jz4780_i2c_trans_done(struct jz4780_i2c *i2c)
{
	jz4780_i2c_writew(i2c, JZ4780_I2C_INTM, 0);
	complete(&i2c->trans_waitq);
}

static irqreturn_t jz4780_i2c_irq(int irqno, void *dev_id)
{
	unsigned short tmp;
	unsigned short intst;
	unsigned short intmsk;
	struct jz4780_i2c *i2c = dev_id;
	unsigned long flags;

	spin_lock_irqsave(&i2c->lock, flags);
	intmsk = jz4780_i2c_readw(i2c, JZ4780_I2C_INTM);
	intst = jz4780_i2c_readw(i2c, JZ4780_I2C_INTST);

	intst &= intmsk;

	if (intst & JZ4780_I2C_INTST_TXABT) {
		jz4780_i2c_trans_done(i2c);
		goto done;
	}

	if (intst & JZ4780_I2C_INTST_RXOF) {
		dev_dbg(&i2c->adap.dev, "received fifo overflow!\n");
		jz4780_i2c_trans_done(i2c);
		goto done;
	}

	/*
	 * When reading, always drain RX FIFO before we send more Read
	 * Commands to avoid fifo overrun
	 */
	if (i2c->is_write == 0) {
		int rd_left;

		while ((jz4780_i2c_readw(i2c, JZ4780_I2C_STA)
				  & JZ4780_I2C_STA_RFNE)) {
			*(i2c->rbuf++) = jz4780_i2c_readw(i2c, JZ4780_I2C_DC)
					 & 0xff;
			i2c->rd_data_xfered++;
			if (i2c->rd_data_xfered == i2c->rd_total_len) {
				jz4780_i2c_trans_done(i2c);
				goto done;
			}
		}

		rd_left = i2c->rd_total_len - i2c->rd_data_xfered;

		if (rd_left <= JZ4780_I2C_FIFO_LEN)
			jz4780_i2c_writew(i2c, JZ4780_I2C_RXTL, rd_left - 1);
	}

	if (intst & JZ4780_I2C_INTST_TXEMP) {
		if (i2c->is_write == 0) {
			int cmd_left = i2c->rd_total_len - i2c->rd_cmd_xfered;
			int max_send = (JZ4780_I2C_FIFO_LEN - 1)
					 - (i2c->rd_cmd_xfered
					 - i2c->rd_data_xfered);
			int cmd_to_send = min(cmd_left, max_send);

			if (i2c->rd_cmd_xfered != 0)
				cmd_to_send = min(cmd_to_send,
						  JZ4780_I2C_FIFO_LEN
						  - TX_LEVEL - 1);

			if (cmd_to_send) {
				jz4780_i2c_send_rcmd(i2c, cmd_to_send);
				i2c->rd_cmd_xfered += cmd_to_send;
			}

			cmd_left = i2c->rd_total_len - i2c->rd_cmd_xfered;
			if (cmd_left == 0) {
				intmsk = jz4780_i2c_readw(i2c, JZ4780_I2C_INTM);
				intmsk &= ~JZ4780_I2C_INTM_MTXEMP;
				jz4780_i2c_writew(i2c, JZ4780_I2C_INTM, intmsk);

				tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
				tmp &= ~JZ4780_I2C_CTRL_STPHLD;
				jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);
			}
		} else {
			unsigned short data;
			unsigned short i2c_sta;

			i2c_sta = jz4780_i2c_readw(i2c, JZ4780_I2C_STA);

			while ((i2c_sta & JZ4780_I2C_STA_TFNF) &&
			       (i2c->wt_len > 0)) {
				i2c_sta = jz4780_i2c_readw(i2c, JZ4780_I2C_STA);
				data = *i2c->wbuf;
				data &= ~JZ4780_I2C_DC_READ;
				jz4780_i2c_writew(i2c, JZ4780_I2C_DC,
						  data);
				i2c->wbuf++;
				i2c->wt_len--;
			}

			if (i2c->wt_len == 0) {
				if (!i2c->stop_hold) {
					tmp = jz4780_i2c_readw(i2c,
							       JZ4780_I2C_CTRL);
					tmp &= ~JZ4780_I2C_CTRL_STPHLD;
					jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL,
							  tmp);
				}

				jz4780_i2c_trans_done(i2c);
				goto done;
			}
		}
	}

done:
	spin_unlock_irqrestore(&i2c->lock, flags);
	return IRQ_HANDLED;
}

static void jz4780_i2c_txabrt(struct jz4780_i2c *i2c, int src)
{
	int i;

	dev_err(&i2c->adap.dev, "txabrt: 0x%08x\n", src);
	dev_err(&i2c->adap.dev, "device addr=%x\n",
		jz4780_i2c_readw(i2c, JZ4780_I2C_TAR));
	dev_err(&i2c->adap.dev, "send cmd count:%d  %d\n",
		i2c->cmd, i2c->cmd_buf[i2c->cmd]);
	dev_err(&i2c->adap.dev, "receive data count:%d  %d\n",
		i2c->cmd, i2c->data_buf[i2c->cmd]);

	for (i = 0; i < 16; i++) {
		if (src & BIT(i))
			dev_dbg(&i2c->adap.dev, "I2C TXABRT[%d]=%s\n",
				i, jz4780_i2c_abrt_src[i]);
	}
}

static inline int jz4780_i2c_xfer_read(struct jz4780_i2c *i2c,
				       unsigned char *buf, int len, int cnt,
				       int idx)
{
	int ret = 0;
	long timeout;
	int wait_time = JZ4780_I2C_TIMEOUT * (len + 5);
	unsigned short tmp;
	unsigned long flags;

	memset(buf, 0, len);

	spin_lock_irqsave(&i2c->lock, flags);

	i2c->stop_hold = 0;
	i2c->is_write = 0;
	i2c->rbuf = buf;
	i2c->rd_total_len = len;
	i2c->rd_data_xfered = 0;
	i2c->rd_cmd_xfered = 0;

	if (len <= JZ4780_I2C_FIFO_LEN)
		jz4780_i2c_writew(i2c, JZ4780_I2C_RXTL, len - 1);
	else
		jz4780_i2c_writew(i2c, JZ4780_I2C_RXTL, RX_LEVEL);

	jz4780_i2c_writew(i2c, JZ4780_I2C_TXTL, TX_LEVEL);

	jz4780_i2c_writew(i2c, JZ4780_I2C_INTM,
			  JZ4780_I2C_INTM_MRXFL | JZ4780_I2C_INTM_MTXEMP
			  | JZ4780_I2C_INTM_MTXABT | JZ4780_I2C_INTM_MRXOF);

	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
	tmp |= JZ4780_I2C_CTRL_STPHLD;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

	spin_unlock_irqrestore(&i2c->lock, flags);

	timeout = wait_for_completion_timeout(&i2c->trans_waitq,
					      msecs_to_jiffies(wait_time));

	if (!timeout) {
		dev_err(&i2c->adap.dev, "irq read timeout\n");
		dev_dbg(&i2c->adap.dev, "send cmd count:%d  %d\n",
			i2c->cmd, i2c->cmd_buf[i2c->cmd]);
		dev_dbg(&i2c->adap.dev, "receive data count:%d  %d\n",
			i2c->cmd, i2c->data_buf[i2c->cmd]);
		ret = -EIO;
	}

	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_TXABRT);
	if (tmp) {
		jz4780_i2c_txabrt(i2c, tmp);
		ret = -EIO;
	}

	return ret;
}

static inline int jz4780_i2c_xfer_write(struct jz4780_i2c *i2c,
					unsigned char *buf, int len,
					int cnt, int idx)
{
	int ret = 0;
	int wait_time = JZ4780_I2C_TIMEOUT * (len + 5);
	long timeout;
	unsigned short tmp;
	unsigned long flags;

	spin_lock_irqsave(&i2c->lock, flags);

	if (idx < (cnt - 1))
		i2c->stop_hold = 1;
	else
		i2c->stop_hold = 0;

	i2c->is_write = 1;
	i2c->wbuf = buf;
	i2c->wt_len = len;

	jz4780_i2c_writew(i2c, JZ4780_I2C_TXTL, TX_LEVEL);

	jz4780_i2c_writew(i2c, JZ4780_I2C_INTM, JZ4780_I2C_INTM_MTXEMP
					| JZ4780_I2C_INTM_MTXABT);

	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
	tmp |= JZ4780_I2C_CTRL_STPHLD;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

	spin_unlock_irqrestore(&i2c->lock, flags);

	timeout = wait_for_completion_timeout(&i2c->trans_waitq,
					      msecs_to_jiffies(wait_time));
	if (timeout && !i2c->stop_hold) {
		unsigned short i2c_sta;
		int write_in_process;

		timeout = JZ4780_I2C_TIMEOUT * 100;
		for (; timeout > 0; timeout--) {
			i2c_sta = jz4780_i2c_readw(i2c, JZ4780_I2C_STA);

			write_in_process = (i2c_sta & JZ4780_I2C_STA_MSTACT) ||
				!(i2c_sta & JZ4780_I2C_STA_TFE);
			if (!write_in_process)
				break;
			udelay(10);
		}
	}

	if (!timeout) {
		dev_err(&i2c->adap.dev, "write wait timeout\n");
		ret = -EIO;
	}

	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_TXABRT);
	if (tmp) {
		jz4780_i2c_txabrt(i2c, tmp);
		ret = -EIO;
	}

	return ret;
}

static int jz4780_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msg,
			   int count)
{
	int i = -EIO;
	int ret = 0;
	struct jz4780_i2c *i2c = adap->algo_data;

	ret = jz4780_i2c_prepare(i2c);
	if (ret) {
		dev_err(&i2c->adap.dev, "I2C prepare failed\n");
		goto out;
	}

	if (msg->addr != jz4780_i2c_readw(i2c, JZ4780_I2C_TAR)) {
		ret = jz4780_i2c_set_target(i2c, msg->addr);
		if (ret)
			goto out;
	}
	for (i = 0; i < count; i++, msg++) {
		if (msg->flags & I2C_M_RD)
			ret = jz4780_i2c_xfer_read(i2c, msg->buf, msg->len,
						   count, i);
		else
			ret = jz4780_i2c_xfer_write(i2c, msg->buf, msg->len,
						    count, i);

		if (ret)
			goto out;
	}

	ret = i;

out:
	jz4780_i2c_cleanup(i2c);
	return ret;
}

static u32 jz4780_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm jz4780_i2c_algorithm = {
	.master_xfer	= jz4780_i2c_xfer,
	.functionality	= jz4780_i2c_functionality,
};

static const struct of_device_id jz4780_i2c_of_matches[] = {
	{ .compatible = "ingenic,jz4780-i2c", },
	{ /* sentinel */ }
};

static int jz4780_i2c_probe(struct platform_device *pdev)
{
	int ret = 0;
	unsigned int clk_freq = 0;
	unsigned short tmp;
	struct resource *r;
	struct jz4780_i2c *i2c;

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct jz4780_i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->adap.owner		= THIS_MODULE;
	i2c->adap.algo		= &jz4780_i2c_algorithm;
	i2c->adap.algo_data	= i2c;
	i2c->adap.retries	= 5;
	i2c->adap.dev.parent	= &pdev->dev;
	i2c->adap.dev.of_node	= pdev->dev.of_node;
	sprintf(i2c->adap.name, "%s", pdev->name);

	init_completion(&i2c->trans_waitq);
	spin_lock_init(&i2c->lock);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c->iomem = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(i2c->iomem))
		return PTR_ERR(i2c->iomem);

	platform_set_drvdata(pdev, i2c);

	i2c->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(i2c->clk))
		return PTR_ERR(i2c->clk);

	clk_prepare_enable(i2c->clk);

	if (of_property_read_u32(pdev->dev.of_node, "clock-frequency",
				 &clk_freq)) {
		dev_err(&pdev->dev, "clock-frequency not specified in DT");
		return clk_freq;
	}

	i2c->speed = clk_freq / 1000;
	jz4780_i2c_set_speed(i2c);

	dev_info(&pdev->dev, "Bus frequency is %d KHz\n", i2c->speed);

	tmp = jz4780_i2c_readw(i2c, JZ4780_I2C_CTRL);
	tmp &= ~JZ4780_I2C_CTRL_STPHLD;
	jz4780_i2c_writew(i2c, JZ4780_I2C_CTRL, tmp);

	jz4780_i2c_writew(i2c, JZ4780_I2C_INTM, 0x0);

	i2c->irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(&pdev->dev, i2c->irq, jz4780_i2c_irq, 0,
			       dev_name(&pdev->dev), i2c);
	if (ret) {
		ret = -ENODEV;
		goto err;
	}

	ret = i2c_add_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to add bus\n");
		goto err;
	}

	return 0;

err:
	clk_disable_unprepare(i2c->clk);
	return ret;
}

static int jz4780_i2c_remove(struct platform_device *pdev)
{
	struct jz4780_i2c *i2c = platform_get_drvdata(pdev);

	clk_disable_unprepare(i2c->clk);
	i2c_del_adapter(&i2c->adap);
	return 0;
}

static struct platform_driver jz4780_i2c_driver = {
	.probe		= jz4780_i2c_probe,
	.remove		= jz4780_i2c_remove,
	.driver		= {
		.name	= "jz4780-i2c",
		.of_match_table = of_match_ptr(jz4780_i2c_of_matches),
	},
};

module_platform_driver(jz4780_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ztyan<ztyan@ingenic.cn>");
MODULE_DESCRIPTION("i2c driver for JZ4780 SoCs");
