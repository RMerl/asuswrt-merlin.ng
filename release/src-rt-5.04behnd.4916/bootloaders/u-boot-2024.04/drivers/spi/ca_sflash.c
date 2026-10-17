// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Cortina SPI-FLASH Controller
 *
 * Copyright (C) 2020 Cortina Access Inc. All Rights Reserved.
 *
 * Author: PengPeng Chen <pengpeng.chen@cortina-access.com>
 */

#include <common.h>
#include <malloc.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>
#include <spi.h>
#include <spi-mem.h>
#include <reset.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

struct ca_sflash_regs {
	u32 idr;		/* 0x00:Flash word ID Register */
	u32 tc;			/* 0x04:Flash Timeout Counter Register */
	u32 sr;			/* 0x08:Flash Status Register */
	u32 tr;			/* 0x0C:Flash Type Register */
	u32 asr;		/* 0x10:Flash ACCESS START/BUSY Register */
	u32 isr;		/* 0x14:Flash Interrupt Status Register */
	u32 imr;		/* 0x18:Flash Interrupt Mask Register */
	u32 fcr;		/* 0x1C:NAND Flash FIFO Control Register */
	u32 ffsr;		/* 0x20:Flash FIFO Status Register */
	u32 ffar;		/* 0x24:Flash FIFO ADDRESS Register */
	u32 ffmar;		/* 0x28:Flash FIFO MATCHING ADDRESS Register */
	u32 ffdr;		/* 0x2C:Flash FIFO Data Register */
	u32 ar;			/* 0x30:Serial Flash Access Register */
	u32 ear;		/* 0x34:Serial Flash Extend Access Register */
	u32 adr;		/* 0x38:Serial Flash ADdress Register */
	u32 dr;			/* 0x3C:Serial Flash Data Register */
	u32 tmr;		/* 0x40:Serial Flash Timing Register */
};

/*
 * FLASH_TYPE
 */
#define CA_FLASH_TR_PIN			BIT(15)
#define CA_FLASH_TR_TYPE_MSK		GENMASK(14, 12)
#define CA_FLASH_TR_TYPE(tp)		(((tp) << 12) & CA_FLASH_TR_TYPE_MSK)
#define CA_FLASH_TR_WIDTH			BIT(11)
#define CA_FLASH_TR_SIZE_MSK		GENMASK(10, 9)
#define CA_FLASH_TR_SIZE(sz)		(((sz) << 9) & CA_FLASH_TR_SIZE_MSK)

/*
 * FLASH_FLASH_ACCESS_START
 */
#define CA_FLASH_ASR_IND_START_EN	BIT(1)
#define CA_FLASH_ASR_DMA_START_EN	BIT(3)
#define CA_FLASH_ASR_WR_ACCESS_EN	BIT(9)

/*
 * FLASH_FLASH_INTERRUPT
 */
#define CA_FLASH_ISR_REG_IRQ		BIT(1)
#define CA_FLASH_ISR_FIFO_IRQ		BIT(2)

/*
 * FLASH_SF_ACCESS
 */
#define CA_SF_AR_OP_MSK		GENMASK(7, 0)
#define CA_SF_AR_OP(op)		((op) << 0 & CA_SF_AR_OP_MSK)
#define CA_SF_AR_ACCODE_MSK		GENMASK(11, 8)
#define CA_SF_AR_ACCODE(ac)		(((ac) << 8) & CA_SF_AR_ACCODE_MSK)
#define CA_SF_AR_FORCE_TERM		BIT(12)
#define CA_SF_AR_FORCE_BURST		BIT(13)
#define CA_SF_AR_AUTO_MODE_EN		BIT(15)
#define CA_SF_AR_CHIP_EN_ALT		BIT(16)
#define CA_SF_AR_HI_SPEED_RD		BIT(17)
#define CA_SF_AR_MIO_INF_DC		BIT(24)
#define CA_SF_AR_MIO_INF_AC		BIT(25)
#define CA_SF_AR_MIO_INF_CC		BIT(26)
#define CA_SF_AR_DDR_MSK		GENMASK(29, 28)
#define CA_SF_AR_DDR(ddr)		(((ddr) << 28) & CA_SF_AR_DDR_MSK)
#define CA_SF_AR_MIO_INF_MSK		GENMASK(31, 30)
#define CA_SF_AR_MIO_INF(io)		(((io) << 30) & CA_SF_AR_MIO_INF_MSK)

/*
 * FLASH_SF_EXT_ACCESS
 */
#define CA_SF_EAR_OP_MSK		GENMASK(7, 0)
#define CA_SF_EAR_OP(op)		(((op) << 0) & CA_SF_EAR_OP_MSK)
#define CA_SF_EAR_DATA_CNT_MSK		GENMASK(20, 8)
#define CA_SF_EAR_DATA_CNT(cnt)		(((cnt) << 8) & CA_SF_EAR_DATA_CNT_MSK)
#define CA_SF_EAR_DATA_CNT_MAX		(4096)
#define CA_SF_EAR_ADDR_CNT_MSK		GENMASK(23, 21)
#define CA_SF_EAR_ADDR_CNT(cnt)		(((cnt) << 21) & CA_SF_EAR_ADDR_CNT_MSK)
#define CA_SF_EAR_ADDR_CNT_MAX		(5)
#define CA_SF_EAR_DUMY_CNT_MSK		GENMASK(29, 24)
#define CA_SF_EAR_DUMY_CNT(cnt)		(((cnt) << 24) & CA_SF_EAR_DUMY_CNT_MSK)
#define CA_SF_EAR_DUMY_CNT_MAX		(32)
#define CA_SF_EAR_DRD_CMD_EN		BIT(31)

/*
 * FLASH_SF_ADDRESS
 */
#define CA_SF_ADR_REG_MSK		GENMASK(31, 0)
#define CA_SF_ADR_REG(addr)		(((addr) << 0) & CA_SF_ADR_REG_MSK)

/*
 * FLASH_SF_DATA
 */
#define CA_SF_DR_REG_MSK		GENMASK(31, 0)
#define CA_SF_DR_REG(addr)		(((addr) << 0) & CA_SF_DR_REG_MSK)

/*
 * FLASH_SF_TIMING
 */
#define CA_SF_TMR_IDLE_MSK		GENMASK(7, 0)
#define CA_SF_TMR_IDLE(idle)		(((idle) << 0) & CA_SF_TMR_IDLE_MSK)
#define CA_SF_TMR_HOLD_MSK		GENMASK(15, 8)
#define CA_SF_TMR_HOLD(hold)		(((hold) << 8) & CA_SF_TMR_HOLD_MSK)
#define CA_SF_TMR_SETUP_MSK		GENMASK(23, 16)
#define CA_SF_TMR_SETUP(setup)		(((setup) << 16) & CA_SF_TMR_SETUP_MSK)
#define CA_SF_TMR_CLK_MSK		GENMASK(26, 24)
#define CA_SF_TMR_CLK(clk)		(((clk) << 24) & CA_SF_TMR_CLK_MSK)

#define CA_SFLASH_IND_WRITE		0
#define CA_SFLASH_IND_READ		1
#define CA_SFLASH_MEM_MAP		3
#define CA_SFLASH_FIFO_TIMEOUT_US	30000
#define CA_SFLASH_BUSY_TIMEOUT_US	40000

#define CA_SF_AC_OP			0x00
#define CA_SF_AC_OP_1_DATA		0x01
#define CA_SF_AC_OP_2_DATA		0x02
#define CA_SF_AC_OP_3_DATA		0x03
#define CA_SF_AC_OP_4_DATA		0x04
#define CA_SF_AC_OP_3_ADDR		0x05
#define CA_SF_AC_OP_4_ADDR		(CA_SF_AC_OP_3_ADDR)
#define CA_SF_AC_OP_3_ADDR_1_DATA	0x06
#define CA_SF_AC_OP_4_ADDR_1_DATA	(CA_SF_AC_OP_3_ADDR_1_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_2_DATA	0x07
#define CA_SF_AC_OP_4_ADDR_2_DATA	(CA_SF_AC_OP_3_ADDR_2_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_3_DATA	0x08
#define CA_SF_AC_OP_4_ADDR_3_DATA	(CA_SF_AC_OP_3_ADDR_3_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_4_DATA	0x09
#define CA_SF_AC_OP_4_ADDR_4_DATA	(CA_SF_AC_OP_3_ADDR_4_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_X_1_DATA	0x0A
#define CA_SF_AC_OP_4_ADDR_X_1_DATA	(CA_SF_AC_OP_3_ADDR_X_1_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_X_2_DATA	0x0B
#define CA_SF_AC_OP_4_ADDR_X_2_DATA	(CA_SF_AC_OP_3_ADDR_X_2_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_X_3_DATA	0x0C
#define CA_SF_AC_OP_4_ADDR_X_3_DATA	(CA_SF_AC_OP_3_ADDR_X_3_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_X_4_DATA	0x0D
#define CA_SF_AC_OP_4_ADDR_X_4_DATA	(CA_SF_AC_OP_3_ADDR_X_4_DATA << 2)
#define CA_SF_AC_OP_3_ADDR_4X_1_DATA	0x0E
#define CA_SF_AC_OP_4_ADDR_4X_1_DATA	(CA_SF_AC_OP_3_ADDR_4X_1_DATA << 2)
#define CA_SF_AC_OP_EXTEND		0x0F

#define CA_SF_ACCESS_MIO_SINGLE		0
#define CA_SF_ACCESS_MIO_DUAL		1
#define CA_SF_ACCESS_MIO_QUARD		2

enum access_type {
	RD_ACCESS,
	WR_ACCESS,
};

struct ca_sflash_priv {
	struct ca_sflash_regs *regs;
	u8 rx_width;
	u8 tx_width;
};

/*
 * This function doesn't do anything except help with debugging
 */
static int ca_sflash_claim_bus(struct udevice *dev)
{
	debug("%s:\n", __func__);
	return 0;
}

static int ca_sflash_release_bus(struct udevice *dev)
{
	debug("%s:\n", __func__);
	return 0;
}

static int ca_sflash_set_speed(struct udevice *dev, uint speed)
{
	debug("%s:\n", __func__);
	return 0;
}

static int ca_sflash_set_mode(struct udevice *dev, uint mode)
{
	struct ca_sflash_priv *priv = dev_get_priv(dev);

	if (mode & SPI_RX_QUAD)
		priv->rx_width = 4;
	else if (mode & SPI_RX_DUAL)
		priv->rx_width = 2;
	else
		priv->rx_width = 1;

	if (mode & SPI_TX_QUAD)
		priv->tx_width = 4;
	else if (mode & SPI_TX_DUAL)
		priv->tx_width = 2;
	else
		priv->tx_width = 1;

	debug("%s: mode=%d, rx_width=%d, tx_width=%d\n",
	      __func__, mode, priv->rx_width, priv->tx_width);

	return 0;
}

static int _ca_sflash_wait_for_not_busy(struct ca_sflash_priv *priv)
{
	u32 asr;

	if (readl_poll_timeout(&priv->regs->asr, asr,
			       !(asr & CA_FLASH_ASR_IND_START_EN),
			       CA_SFLASH_BUSY_TIMEOUT_US)) {
		pr_err("busy timeout (stat:%#x)\n", asr);
		return -1;
	}

	return 0;
}

static int _ca_sflash_wait_cmd(struct ca_sflash_priv *priv,
			       enum access_type type)
{
	if (type == WR_ACCESS) {
		/* Enable write access and start the sflash indirect access */
		clrsetbits_le32(&priv->regs->asr, GENMASK(31, 0),
				CA_FLASH_ASR_WR_ACCESS_EN
				| CA_FLASH_ASR_IND_START_EN);
	} else if (type == RD_ACCESS) {
		/* Start the sflash indirect access */
		clrsetbits_le32(&priv->regs->asr, GENMASK(31, 0),
				CA_FLASH_ASR_IND_START_EN);
	} else {
		printf("%s: !error access type.\n", __func__);
		return -1;
	}

	/* Wait til the action(rd/wr) completed */
	return _ca_sflash_wait_for_not_busy(priv);
}

static int _ca_sflash_read(struct ca_sflash_priv *priv,
			   u8 *buf, unsigned int data_len)
{
	u32 reg_data;
	int len;

	len = data_len;
	while (len >= 4) {
		if (_ca_sflash_wait_cmd(priv, RD_ACCESS))
			return -1;
		reg_data = readl(&priv->regs->dr);
		*buf++ = reg_data & 0xFF;
		*buf++ = (reg_data >> 8) & 0xFF;
		*buf++ = (reg_data >> 16) & 0xFF;
		*buf++ = (reg_data >> 24) & 0xFF;
		len -= 4;
		debug("%s: reg_data=%#08x\n",
		      __func__, reg_data);
	}

	if (len > 0) {
		if (_ca_sflash_wait_cmd(priv, RD_ACCESS))
			return -1;
		reg_data = readl(&priv->regs->dr);
		debug("%s: reg_data=%#08x\n",
		      __func__, reg_data);
	}

	switch (len) {
	case 3:
		*buf++ = reg_data & 0xFF;
		*buf++ = (reg_data >> 8) & 0xFF;
		*buf++ = (reg_data >> 16) & 0xFF;
		break;
	case 2:
		*buf++ = reg_data & 0xFF;
		*buf++ = (reg_data >> 8) & 0xFF;
		break;
	case 1:
		*buf++ = reg_data & 0xFF;
		break;
	case 0:
		break;
	default:
		printf("%s: error data_length %d!\n", __func__, len);
	}

	return 0;
}

static int _ca_sflash_mio_set(struct ca_sflash_priv *priv,
			      u8 width)
{
	if (width == 4) {
		setbits_le32(&priv->regs->ar,
			     CA_SF_AR_MIO_INF_DC
			     | CA_SF_AR_MIO_INF(CA_SF_ACCESS_MIO_QUARD)
			     | CA_SF_AR_FORCE_BURST);
	} else if (width == 2) {
		setbits_le32(&priv->regs->ar,
			     CA_SF_AR_MIO_INF_DC
			     | CA_SF_AR_MIO_INF(CA_SF_ACCESS_MIO_DUAL)
			     | CA_SF_AR_FORCE_BURST);
	} else if (width == 1) {
		setbits_le32(&priv->regs->ar,
			     CA_SF_AR_MIO_INF(CA_SF_ACCESS_MIO_SINGLE)
			     | CA_SF_AR_FORCE_BURST);
	} else {
		printf("%s: error rx/tx width  %d!\n", __func__, width);
		return -1;
	}

	return 0;
}

static int _ca_sflash_write(struct ca_sflash_priv *priv,
			    u8 *buf, unsigned int data_len)
{
	u32 reg_data;
	int len;

	len = data_len;
	while (len > 0) {
		reg_data = buf[0]
			| (buf[1] << 8)
			| (buf[2] << 16)
			| (buf[3] << 24);

		debug("%s: reg_data=%#08x\n",
		      __func__, reg_data);
		/* Fill data */
		clrsetbits_le32(&priv->regs->dr, GENMASK(31, 0), reg_data);

		if (_ca_sflash_wait_cmd(priv, WR_ACCESS))
			return -1;

		len -= 4;
		buf += 4;
	}

	return 0;
}

static int _ca_sflash_access_data(struct ca_sflash_priv *priv,
				  struct spi_mem_op *op)
{
	int total_cnt;
	unsigned int len;
	unsigned int data_cnt = op->data.nbytes;
	u64 addr_offset = op->addr.val;
	u8 addr_cnt = op->addr.nbytes;
	u8 *data_buf = NULL;
	u8 *buf = NULL;

	if (op->data.dir == SPI_MEM_DATA_IN)
		data_buf = (u8 *)op->data.buf.in;
	else
		data_buf = (u8 *)op->data.buf.out;

	if (data_cnt > CA_SF_EAR_DATA_CNT_MAX)
		buf = malloc(CA_SF_EAR_DATA_CNT_MAX);
	else
		buf = malloc(data_cnt);

	total_cnt = data_cnt;
	while (total_cnt > 0) {
		/* Fill address */
		if (addr_cnt > 0)
			clrsetbits_le32(&priv->regs->adr,
					GENMASK(31, 0), (u32)addr_offset);

		if (total_cnt > CA_SF_EAR_DATA_CNT_MAX) {
			len = CA_SF_EAR_DATA_CNT_MAX;
			addr_offset += CA_SF_EAR_DATA_CNT_MAX;
			/* Clear start bit before next bulk read */
			clrbits_le32(&priv->regs->asr, GENMASK(31, 0));
		} else {
			len = total_cnt;
		}

		memset(buf, 0, len);
		if (op->data.dir == SPI_MEM_DATA_IN) {
			if (_ca_sflash_read(priv, buf, len))
				break;
			memcpy(data_buf, buf, len);
		} else {
			memcpy(buf, data_buf, len);
			if (_ca_sflash_write(priv, buf, len))
				break;
		}

		total_cnt -= len;
		data_buf += len;
	}
	if (buf)
		free(buf);

	return total_cnt > 0 ? -1 : 0;
}

static int _ca_sflash_issue_cmd(struct ca_sflash_priv *priv,
				struct spi_mem_op *op, u8 opcode)
{
	u8 dummy_cnt = op->dummy.nbytes;
	u8 addr_cnt = op->addr.nbytes;
	u8 mio_width;
	unsigned int data_cnt = op->data.nbytes;
	u64 addr_offset = op->addr.val;

	/* Set the access register */
	clrsetbits_le32(&priv->regs->ar,
			GENMASK(31, 0), CA_SF_AR_ACCODE(opcode));

	if (opcode == CA_SF_AC_OP_EXTEND) { /* read_data, write_data */
		if (data_cnt > 6) {
			if (op->data.dir == SPI_MEM_DATA_IN)
				mio_width = priv->rx_width;
			else
				mio_width = priv->tx_width;
			if (_ca_sflash_mio_set(priv, mio_width))
				return -1;
		}
		debug("%s: FLASH ACCESS reg=%#08x\n",
		      __func__, readl(&priv->regs->ar));

		/* Use command in extend_access register */
		clrsetbits_le32(&priv->regs->ear,
				GENMASK(31, 0), CA_SF_EAR_OP(op->cmd.opcode)
				| CA_SF_EAR_DUMY_CNT(dummy_cnt * 8 - 1)
				| CA_SF_EAR_ADDR_CNT(addr_cnt - 1)
				| CA_SF_EAR_DATA_CNT(4 - 1)
				| CA_SF_EAR_DRD_CMD_EN);
		debug("%s: FLASH EXT ACCESS reg=%#08x\n",
		      __func__, readl(&priv->regs->ear));

		if (_ca_sflash_access_data(priv, op))
			return -1;
	} else { /* reset_op, wr_enable, wr_disable */
		setbits_le32(&priv->regs->ar,
			     CA_SF_AR_OP(op->cmd.opcode));
		debug("%s: FLASH ACCESS reg=%#08x\n",
		      __func__, readl(&priv->regs->ar));

		if (opcode == CA_SF_AC_OP_4_ADDR) { /* erase_op */
			/* Configure address length */
			if (addr_cnt > 3)	/* 4 Bytes address */
				setbits_le32(&priv->regs->tr,
					     CA_FLASH_TR_SIZE(2));
			else				/* 3 Bytes address */
				clrbits_le32(&priv->regs->tr,
					     CA_FLASH_TR_SIZE_MSK);

			/* Fill address */
			if (addr_cnt > 0)
				clrsetbits_le32(&priv->regs->adr,
						GENMASK(31, 0),
						(u32)addr_offset);
		}

		if (_ca_sflash_wait_cmd(priv, RD_ACCESS))
			return -1;
	}
	/* elapse 10us before issuing any other command */
	udelay(10);

	return 0;
}

static int ca_sflash_exec_op(struct spi_slave *slave,
			     const struct spi_mem_op *op)
{
	struct ca_sflash_priv *priv = dev_get_priv(slave->dev->parent);
	u8 opcode;

	debug("%s: cmd:%#02x addr.val:%#llx addr.len:%#x data.len:%#x data.dir:%#x\n",
	      __func__, op->cmd.opcode, op->addr.val,
	      op->addr.nbytes, op->data.nbytes, op->data.dir);

	if (op->data.nbytes == 0 && op->addr.nbytes == 0) {
		opcode = CA_SF_AC_OP;
	} else if (op->data.nbytes == 0 && op->addr.nbytes > 0) {
		opcode = CA_SF_AC_OP_4_ADDR;
	} else if (op->data.nbytes > 0) {
		opcode = CA_SF_AC_OP_EXTEND;
	} else {
		printf("%s: can't support cmd.opcode:(%#02x) type currently!\n",
		       __func__, op->cmd.opcode);
		return -1;
	}

	return _ca_sflash_issue_cmd(priv, (struct spi_mem_op *)op, opcode);
}

static void ca_sflash_init(struct ca_sflash_priv *priv)
{
	/* Set FLASH_TYPE as serial flash, value: 0x0400*/
	clrsetbits_le32(&priv->regs->tr,
			GENMASK(31, 0), CA_FLASH_TR_SIZE(2));
	debug("%s: FLASH_TYPE reg=%#x\n",
	      __func__, readl(&priv->regs->tr));

	/* Minimize flash timing, value: 0x07010101 */
	clrsetbits_le32(&priv->regs->tmr,
			GENMASK(31, 0),
			CA_SF_TMR_CLK(0x07)
			| CA_SF_TMR_SETUP(0x01)
			| CA_SF_TMR_HOLD(0x01)
			| CA_SF_TMR_IDLE(0x01));
	debug("%s: FLASH_TIMING reg=%#x\n",
	      __func__, readl(&priv->regs->tmr));
}

static int ca_sflash_probe(struct udevice *dev)
{
	struct ca_sflash_priv *priv = dev_get_priv(dev);
	struct resource res;
	int ret;

	/* Map the registers */
	ret = dev_read_resource_byname(dev, "sflash-regs", &res);
	if (ret) {
		dev_err(dev, "can't get regs base addresses(ret = %d)!\n", ret);
		return ret;
	}
	priv->regs = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	ca_sflash_init(priv);

	printf("SFLASH: Controller probed ready\n");
	return 0;
}

static const struct spi_controller_mem_ops ca_sflash_mem_ops = {
	.exec_op = ca_sflash_exec_op,
};

static const struct dm_spi_ops ca_sflash_ops = {
	.claim_bus = ca_sflash_claim_bus,
	.release_bus = ca_sflash_release_bus,
	.set_speed = ca_sflash_set_speed,
	.set_mode = ca_sflash_set_mode,
	.mem_ops = &ca_sflash_mem_ops,
};

static const struct udevice_id ca_sflash_ids[] = {
	{.compatible = "cortina,ca-sflash"},
	{}
};

U_BOOT_DRIVER(ca_sflash) = {
	.name = "ca_sflash",
	.id = UCLASS_SPI,
	.of_match = ca_sflash_ids,
	.ops = &ca_sflash_ops,
	.priv_auto = sizeof(struct ca_sflash_priv),
	.probe = ca_sflash_probe,
};
