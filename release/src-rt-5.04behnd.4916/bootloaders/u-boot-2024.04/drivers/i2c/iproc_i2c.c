// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Broadcom
 *
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <config.h>
#include <dm.h>
#include <linux/printk.h>
#include "errno.h"
#include <i2c.h>
#include "iproc_i2c.h"

DECLARE_GLOBAL_DATA_PTR;

struct iproc_i2c_regs {
	u32 cfg_reg;
	u32 timg_cfg;
	u32 addr_reg;
	u32 mstr_fifo_ctrl;
	u32 slv_fifo_ctrl;
	u32 bitbng_ctrl;
	u32 blnks[6]; /* Not to be used */
	u32 mstr_cmd;
	u32 slv_cmd;
	u32 evt_en;
	u32 evt_sts;
	u32 mstr_datawr;
	u32 mstr_datard;
	u32 slv_datawr;
	u32 slv_datard;
};

struct iproc_i2c {
	struct iproc_i2c_regs __iomem *base; /* register base */
	int bus_speed;
	int i2c_init_done;
};

/* Function to read a value from specified register. */
static unsigned int iproc_i2c_reg_read(u32 *reg_addr)
{
	unsigned int val;

	val = readl((void *)(reg_addr));
	return cpu_to_le32(val);
}

/* Function to write a value ('val') in to a specified register. */
static int iproc_i2c_reg_write(u32 *reg_addr, unsigned int val)
{
	val = cpu_to_le32(val);
	writel(val, (void *)(reg_addr));
	return  0;
}

#if defined(DEBUG)
static int iproc_dump_i2c_regs(struct iproc_i2c *bus_prvdata)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;

	debug("\n----------------------------------------------\n");
	debug("%s: Dumping SMBus registers...\n", __func__);

	regval = iproc_i2c_reg_read(&base->cfg_reg);
	debug("CCB_SMB_CFG_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->timg_cfg);
	debug("CCB_SMB_TIMGCFG_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->addr_reg);
	debug("CCB_SMB_ADDR_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->mstr_fifo_ctrl);
	debug("CCB_SMB_MSTRFIFOCTL_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->slv_fifo_ctrl);
	debug("CCB_SMB_SLVFIFOCTL_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->bitbng_ctrl);
	debug("CCB_SMB_BITBANGCTL_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->mstr_cmd);
	debug("CCB_SMB_MSTRCMD_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->slv_cmd);
	debug("CCB_SMB_SLVCMD_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->evt_en);
	debug("CCB_SMB_EVTEN_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->evt_sts);
	debug("CCB_SMB_EVTSTS_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->mstr_datawr);
	debug("CCB_SMB_MSTRDATAWR_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->mstr_datard);
	debug("CCB_SMB_MSTRDATARD_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->slv_datawr);
	debug("CCB_SMB_SLVDATAWR_REG=0x%08X\n", regval);

	regval = iproc_i2c_reg_read(&base->slv_datard);
	debug("CCB_SMB_SLVDATARD_REG=0x%08X\n", regval);

	debug("----------------------------------------------\n\n");
	return 0;
}
#else
static int iproc_dump_i2c_regs(struct iproc_i2c *bus_prvdata)
{
	return 0;
}
#endif

/*
 * Function to ensure that the previous transaction was completed before
 * initiating a new transaction. It can also be used in polling mode to
 * check status of completion of a command
 */
static int iproc_i2c_startbusy_wait(struct iproc_i2c *bus_prvdata)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;

	regval = iproc_i2c_reg_read(&base->mstr_cmd);

	/* Check if an operation is in progress. During probe it won't be.
	 * But when shutdown/remove was called we want to make sure that
	 * the transaction in progress completed
	 */
	if (regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK) {
		unsigned int i = 0;

		do {
			mdelay(10);
			i++;
			regval = iproc_i2c_reg_read(&base->mstr_cmd);

			/* If start-busy bit cleared, exit the loop */
		} while ((regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK) &&
			 (i < IPROC_SMB_MAX_RETRIES));

		if (i >= IPROC_SMB_MAX_RETRIES) {
			pr_err("%s: START_BUSY bit didn't clear, exiting\n",
			       __func__);
			return -ETIMEDOUT;
		}
	}
	return 0;
}

/*
 * This function set clock frequency for SMBus block. As per hardware
 * engineering, the clock frequency can be changed dynamically.
 */
static int iproc_i2c_set_clk_freq(struct iproc_i2c *bus_prvdata)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;

	regval = iproc_i2c_reg_read(&base->timg_cfg);

	switch (bus_prvdata->bus_speed) {
	case I2C_SPEED_STANDARD_RATE:
		regval &= ~CCB_SMB_TIMGCFG_MODE400_MASK;
		break;

	case I2C_SPEED_FAST_RATE:
		regval |= CCB_SMB_TIMGCFG_MODE400_MASK;
		break;

	default:
		return -EINVAL;
	}

	iproc_i2c_reg_write(&base->timg_cfg, regval);
	return 0;
}

static int iproc_i2c_init(struct udevice *bus)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;

	debug("\nEntering %s\n", __func__);

	/* Put controller in reset */
	regval = iproc_i2c_reg_read(&base->cfg_reg);
	regval |= CCB_SMB_CFG_RST_MASK;
	regval &= ~CCB_SMB_CFG_SMBEN_MASK;
	iproc_i2c_reg_write(&base->cfg_reg, regval);

	/* Wait 100 usec as per spec */
	udelay(100);

	/* bring controller out of reset */
	regval &= ~CCB_SMB_CFG_RST_MASK;
	iproc_i2c_reg_write(&base->cfg_reg, regval);

	/* Flush Tx, Rx FIFOs. Note we are setting the Rx FIFO threshold to 0.
	 * May be OK since we are setting RX_EVENT and RX_FIFO_FULL interrupts
	 */
	regval = CCB_SMB_MSTRRXFIFOFLSH_MASK | CCB_SMB_MSTRTXFIFOFLSH_MASK;
	iproc_i2c_reg_write(&base->mstr_fifo_ctrl, regval);

	/* Enable SMbus block. Note, we are setting MASTER_RETRY_COUNT to zero
	 * since there will be only one master
	 */
	regval = iproc_i2c_reg_read(&base->cfg_reg);
	regval |= CCB_SMB_CFG_SMBEN_MASK;
	iproc_i2c_reg_write(&base->cfg_reg, regval);

	/* Set default clock frequency */
	iproc_i2c_set_clk_freq(bus_prvdata);

	/* Disable intrs */
	iproc_i2c_reg_write(&base->evt_en, 0);

	/* Clear intrs (W1TC) */
	regval = iproc_i2c_reg_read(&base->evt_sts);
	iproc_i2c_reg_write(&base->evt_sts, regval);

	bus_prvdata->i2c_init_done = 1;

	iproc_dump_i2c_regs(bus_prvdata);
	debug("%s: Init successful\n", __func__);

	return 0;
}

/*
 * This function copies data to SMBus's Tx FIFO. Valid for write transactions
 * only
 *
 * base_addr: Mapped address of this SMBus instance
 * dev_addr:  SMBus (I2C) device address. We are assuming 7-bit addresses
 *            initially
 * info:   Data to copy in to Tx FIFO. For read commands, the size should be
 *         set to zero by the caller
 *
 */
static void iproc_i2c_write_trans_data(struct iproc_i2c *bus_prvdata,
				       unsigned short dev_addr,
				       struct iproc_xact_info *info)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;
	unsigned int i;
	unsigned int num_data_bytes = 0;

	debug("%s: dev_addr=0x%X cmd_valid=%d cmd=0x%02x size=%u proto=%d buf[] %x\n",
	      __func__, dev_addr, info->cmd_valid,
	      info->command, info->size, info->smb_proto, info->data[0]);

	/* Write SMBus device address first */
	/* Note, we are assuming 7-bit addresses for now. For 10-bit addresses,
	 * we may have one more write to send the upper 3 bits of 10-bit addr
	 */
	iproc_i2c_reg_write(&base->mstr_datawr, dev_addr);

	/* If the protocol needs command code, copy it */
	if (info->cmd_valid)
		iproc_i2c_reg_write(&base->mstr_datawr, info->command);

	/* Depending on the SMBus protocol, we need to write additional
	 * transaction data in to Tx FIFO. Refer to section 5.5 of SMBus
	 * spec for sequence for a transaction
	 */
	switch (info->smb_proto) {
	case SMBUS_PROT_RECV_BYTE:
		/* No additional data to be written */
		num_data_bytes = 0;
		break;

	case SMBUS_PROT_SEND_BYTE:
		num_data_bytes = info->size;
		break;

	case SMBUS_PROT_RD_BYTE:
	case SMBUS_PROT_RD_WORD:
	case SMBUS_PROT_BLK_RD:
		/* Write slave address with R/W~ set (bit #0) */
		iproc_i2c_reg_write(&base->mstr_datawr,
				    dev_addr | 0x1);
		num_data_bytes = 0;
		break;

	case SMBUS_PROT_BLK_WR_BLK_RD_PROC_CALL:
		iproc_i2c_reg_write(&base->mstr_datawr,
				    dev_addr | 0x1 |
				    CCB_SMB_MSTRWRSTS_MASK);
		num_data_bytes = 0;
		break;

	case SMBUS_PROT_WR_BYTE:
	case SMBUS_PROT_WR_WORD:
		/* No additional bytes to be written.
		 * Data portion is written in the
		 * 'for' loop below
		 */
		num_data_bytes = info->size;
		break;

	case SMBUS_PROT_BLK_WR:
		/* 3rd byte is byte count */
		iproc_i2c_reg_write(&base->mstr_datawr, info->size);
		num_data_bytes = info->size;
		break;

	default:
		return;
	}

	/* Copy actual data from caller, next. In general, for reads,
	 * no data is copied
	 */
	for (i = 0; num_data_bytes; --num_data_bytes, i++) {
		/* For the last byte, set MASTER_WR_STATUS bit */
		regval = (num_data_bytes == 1) ?
			 info->data[i] | CCB_SMB_MSTRWRSTS_MASK :
			 info->data[i];

		iproc_i2c_reg_write(&base->mstr_datawr, regval);
	}
}

static int iproc_i2c_data_send(struct iproc_i2c *bus_prvdata,
			       unsigned short addr,
			       struct iproc_xact_info *info)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	int rc, retry = 3;
	unsigned int regval;

	/* Make sure the previous transaction completed */
	rc = iproc_i2c_startbusy_wait(bus_prvdata);

	if (rc < 0) {
		pr_err("%s: Send: bus is busy, exiting\n", __func__);
		return rc;
	}

	/* Write transaction bytes to Tx FIFO */
	iproc_i2c_write_trans_data(bus_prvdata, addr, info);

	/* Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	regval = (info->smb_proto << CCB_SMB_MSTRSMBUSPROTO_SHIFT) |
		 CCB_SMB_MSTRSTARTBUSYCMD_MASK;

	iproc_i2c_reg_write(&base->mstr_cmd, regval);

	/* Check for Master status */
	regval = iproc_i2c_reg_read(&base->mstr_cmd);
	while (regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK) {
		mdelay(10);
		if (retry-- <= 0)
			break;
		regval = iproc_i2c_reg_read(&base->mstr_cmd);
	}

	/* If start_busy bit cleared, check if there are any errors */
	if (!(regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK)) {
		/* start_busy bit cleared, check master_status field now */
		regval &= CCB_SMB_MSTRSTS_MASK;
		regval >>= CCB_SMB_MSTRSTS_SHIFT;

		if (regval != MSTR_STS_XACT_SUCCESS) {
			/* Error We can flush Tx FIFO here */
			pr_err("%s: ERROR: Error in transaction %u, exiting\n",
			       __func__, regval);
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int iproc_i2c_data_recv(struct iproc_i2c *bus_prvdata,
			       unsigned short addr,
			       struct iproc_xact_info *info,
			       unsigned int *num_bytes_read)
{
	struct iproc_i2c_regs *base = bus_prvdata->base;
	int rc, retry = 3;
	unsigned int regval;

	/* Make sure the previous transaction completed */
	rc = iproc_i2c_startbusy_wait(bus_prvdata);

	if (rc < 0) {
		pr_err("%s: Receive: Bus is busy, exiting\n", __func__);
		return rc;
	}

	/* Program all transaction bytes into master Tx FIFO */
	iproc_i2c_write_trans_data(bus_prvdata, addr, info);

	/* Program master command register (0x30) with protocol type and set
	 * start_busy_command bit to initiate the write transaction
	 */
	regval = (info->smb_proto << CCB_SMB_MSTRSMBUSPROTO_SHIFT) |
		 CCB_SMB_MSTRSTARTBUSYCMD_MASK | info->size;

	iproc_i2c_reg_write(&base->mstr_cmd, regval);

	/* Check for Master status */
	regval = iproc_i2c_reg_read(&base->mstr_cmd);
	while (regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK) {
		udelay(1000);
		if (retry-- <= 0)
			break;
		regval = iproc_i2c_reg_read(&base->mstr_cmd);
	}

	/* If start_busy bit cleared, check if there are any errors */
	if (!(regval & CCB_SMB_MSTRSTARTBUSYCMD_MASK)) {
		/* start_busy bit cleared, check master_status field now */
		regval &= CCB_SMB_MSTRSTS_MASK;
		regval >>= CCB_SMB_MSTRSTS_SHIFT;

		if (regval != MSTR_STS_XACT_SUCCESS) {
			/* We can flush Tx FIFO here */
			pr_err("%s: Error in transaction %d, exiting\n",
			       __func__, regval);
			return -EREMOTEIO;
		}
	}

	/* Read received byte(s), after TX out address etc */
	regval = iproc_i2c_reg_read(&base->mstr_datard);

	/* For block read, protocol (hw) returns byte count,
	 * as the first byte
	 */
	if (info->smb_proto == SMBUS_PROT_BLK_RD) {
		int i;

		*num_bytes_read = regval & CCB_SMB_MSTRRDDATA_MASK;

		/* Limit to reading a max of 32 bytes only; just a safeguard.
		 * If # bytes read is a number > 32, check transaction set up,
		 * and contact hw engg. Assumption: PEC is disabled
		 */
		for (i = 0;
		     (i < *num_bytes_read) && (i < I2C_SMBUS_BLOCK_MAX);
		     i++) {
			/* Read Rx FIFO for data bytes */
			regval = iproc_i2c_reg_read(&base->mstr_datard);
			info->data[i] = regval & CCB_SMB_MSTRRDDATA_MASK;
		}
	} else {
		/* 1 Byte data */
		*info->data = regval & CCB_SMB_MSTRRDDATA_MASK;
		*num_bytes_read = 1;
	}

	return 0;
}

static int i2c_write_byte(struct iproc_i2c *bus_prvdata,
			  u8 devaddr, u8 regoffset, u8 value)
{
	int rc;
	struct iproc_xact_info info;

	devaddr <<= 1;

	info.cmd_valid = 1;
	info.command = (unsigned char)regoffset;
	info.data = &value;
	info.size = 1;
	info.flags = 0;
	info.smb_proto = SMBUS_PROT_WR_BYTE;
	/* Refer to i2c_smbus_write_byte params passed. */
	rc = iproc_i2c_data_send(bus_prvdata, devaddr, &info);

	if (rc < 0) {
		pr_err("%s: %s error accessing device 0x%X\n",
		       __func__, "Write", devaddr);
		return -EREMOTEIO;
	}

	return 0;
}

int i2c_write(struct udevice *bus,
	      uchar chip, uint regaddr, int alen, uchar *buffer, int len)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	int i, data_len;
	u8 *data;

	if (len > 256) {
		pr_err("I2C write: address out of range\n");
		return 1;
	}

	if (len < 1) {
		pr_err("I2C write: Need offset addr and value\n");
		return 1;
	}

	/* buffer contains offset addr followed by value to be written */
	regaddr = buffer[0];
	data = &buffer[1];
	data_len = len - 1;

	for (i = 0; i < data_len; i++) {
		if (i2c_write_byte(bus_prvdata, chip, regaddr + i, data[i])) {
			pr_err("I2C write (%d): I/O error\n", i);
			iproc_i2c_init(bus);
			return 1;
		}
	}

	return 0;
}

static int i2c_read_byte(struct iproc_i2c *bus_prvdata,
			 u8 devaddr, u8 regoffset, u8 *value)
{
	int rc;
	struct iproc_xact_info info;
	unsigned int num_bytes_read = 0;

	devaddr <<= 1;

	info.cmd_valid = 1;
	info.command = (unsigned char)regoffset;
	info.data = value;
	info.size = 1;
	info.flags = 0;
	info.smb_proto = SMBUS_PROT_RD_BYTE;
	/* Refer to i2c_smbus_read_byte for params passed. */
	rc = iproc_i2c_data_recv(bus_prvdata, devaddr, &info, &num_bytes_read);

	if (rc < 0) {
		pr_err("%s: %s error accessing device 0x%X\n",
		       __func__, "Read", devaddr);
		return -EREMOTEIO;
	}

	return 0;
}

int i2c_read(struct udevice *bus,
	     uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	int i;

	if (len > 256) {
		pr_err("I2C read: address out of range\n");
		return 1;
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte(bus_prvdata, chip, addr + i, &buffer[i])) {
			pr_err("I2C read: I/O error\n");
			iproc_i2c_init(bus);
			return 1;
		}
	}

	return 0;
}

static int iproc_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	int ret = 0;

	debug("%s: %d messages\n", __func__, nmsgs);

	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD)
			ret = i2c_read(bus, msg->addr, 0, 0,
				       msg->buf, msg->len);
		else
			ret = i2c_write(bus, msg->addr, 0, 0,
					msg->buf, msg->len);
	}

	return ret;
}

static int iproc_i2c_probe_chip(struct udevice *bus, uint chip_addr,
				uint chip_flags)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	struct iproc_i2c_regs *base = bus_prvdata->base;
	u32 regval;

	debug("\n%s: Entering chip probe\n", __func__);

	/* Init internal regs, disable intrs (and then clear intrs), set fifo
	 * thresholds, etc.
	 */
	if (!bus_prvdata->i2c_init_done)
		iproc_i2c_init(bus);

	regval = (chip_addr << 1);
	iproc_i2c_reg_write(&base->mstr_datawr, regval);
	regval = ((SMBUS_PROT_QUICK_CMD << CCB_SMB_MSTRSMBUSPROTO_SHIFT) |
			(1 << CCB_SMB_MSTRSTARTBUSYCMD_SHIFT));
	iproc_i2c_reg_write(&base->mstr_cmd, regval);

	do {
		udelay(100);
		regval = iproc_i2c_reg_read(&base->mstr_cmd);
		regval &= CCB_SMB_MSTRSTARTBUSYCMD_MASK;
	}  while (regval);

	regval = iproc_i2c_reg_read(&base->mstr_cmd);

	if ((regval & CCB_SMB_MSTRSTS_MASK) != 0)
		return -1;

	iproc_dump_i2c_regs(bus_prvdata);
	debug("%s: chip probe successful\n", __func__);

	return 0;
}

static int iproc_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);

	bus_prvdata->bus_speed = speed;
	return iproc_i2c_set_clk_freq(bus_prvdata);
}

/**
 * i2c_get_bus_speed - get i2c bus speed
 *
 * This function returns the speed of operation in Hz
 */
int iproc_i2c_get_bus_speed(struct udevice *bus)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	struct iproc_i2c_regs *base = bus_prvdata->base;
	unsigned int regval;
	int ret = 0;

	regval = iproc_i2c_reg_read(&base->timg_cfg);
	regval = (regval & CCB_SMB_TIMGCFG_MODE400_MASK) >>
		  CCB_SMB_TIMGCFG_MODE400_SHIFT;

	switch (regval) {
	case 0:
		ret = I2C_SPEED_STANDARD_RATE;
		break;
	case 1:
		ret = I2C_SPEED_FAST_RATE;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int iproc_i2c_probe(struct udevice *bus)
{
	return iproc_i2c_init(bus);
}

static int iproc_i2c_of_to_plat(struct udevice *bus)
{
	struct iproc_i2c *bus_prvdata = dev_get_priv(bus);
	int node = dev_of_offset(bus);
	const void *blob = gd->fdt_blob;

	bus_prvdata->base = map_physmem(dev_read_addr(bus),
					sizeof(void *),
					MAP_NOCACHE);

	bus_prvdata->bus_speed =
		fdtdec_get_int(blob, node, "bus-frequency",
			       I2C_SPEED_STANDARD_RATE);

	return 0;
}

static const struct dm_i2c_ops iproc_i2c_ops = {
	.xfer		= iproc_i2c_xfer,
	.probe_chip	= iproc_i2c_probe_chip,
	.set_bus_speed	= iproc_i2c_set_bus_speed,
	.get_bus_speed	= iproc_i2c_get_bus_speed,
};

static const struct udevice_id iproc_i2c_ids[] = {
	{ .compatible = "brcm,iproc-i2c" },
	{ }
};

U_BOOT_DRIVER(iproc_i2c) = {
	.name	= "iproc_i2c",
	.id	= UCLASS_I2C,
	.of_match = iproc_i2c_ids,
	.of_to_plat = iproc_i2c_of_to_plat,
	.probe	= iproc_i2c_probe,
	.priv_auto	= sizeof(struct iproc_i2c),
	.ops	= &iproc_i2c_ops,
	.flags  = DM_FLAG_PRE_RELOC,
};
