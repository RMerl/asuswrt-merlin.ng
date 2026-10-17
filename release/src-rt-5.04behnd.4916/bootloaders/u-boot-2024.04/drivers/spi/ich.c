// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011-12 The Chromium OS Authors.
 *
 * This file is derived from the flashrom project.
 */

#define LOG_CATEGORY	UCLASS_SPI

#include <common.h>
#include <bootstage.h>
#include <div64.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <pch.h>
#include <pci.h>
#include <pci_ids.h>
#include <spi.h>
#include <spi_flash.h>
#include <spi-mem.h>
#include <spl.h>
#include <asm/fast_spi.h>
#include <asm/io.h>
#include <dm/uclass-internal.h>
#include <asm/mtrr.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>

#include "ich.h"

#ifdef DEBUG_TRACE
#define debug_trace(fmt, args...) debug(fmt, ##args)
#else
#define debug_trace(x, args...)
#endif

static u8 ich_readb(struct ich_spi_priv *priv, int reg)
{
	u8 value = readb(priv->base + reg);

	debug_trace("read %2.2x from %4.4x\n", value, reg);

	return value;
}

static u16 ich_readw(struct ich_spi_priv *priv, int reg)
{
	u16 value = readw(priv->base + reg);

	debug_trace("read %4.4x from %4.4x\n", value, reg);

	return value;
}

static u32 ich_readl(struct ich_spi_priv *priv, int reg)
{
	u32 value = readl(priv->base + reg);

	debug_trace("read %8.8x from %4.4x\n", value, reg);

	return value;
}

static void ich_writeb(struct ich_spi_priv *priv, u8 value, int reg)
{
	writeb(value, priv->base + reg);
	debug_trace("wrote %2.2x to %4.4x\n", value, reg);
}

static void ich_writew(struct ich_spi_priv *priv, u16 value, int reg)
{
	writew(value, priv->base + reg);
	debug_trace("wrote %4.4x to %4.4x\n", value, reg);
}

static void ich_writel(struct ich_spi_priv *priv, u32 value, int reg)
{
	writel(value, priv->base + reg);
	debug_trace("wrote %8.8x to %4.4x\n", value, reg);
}

static void write_reg(struct ich_spi_priv *priv, const void *value,
		      int dest_reg, uint32_t size)
{
	memcpy_toio(priv->base + dest_reg, value, size);
}

static void read_reg(struct ich_spi_priv *priv, int src_reg, void *value,
		     uint32_t size)
{
	memcpy_fromio(value, priv->base + src_reg, size);
}

static void ich_set_bbar(struct ich_spi_priv *ctlr, uint32_t minaddr)
{
	const uint32_t bbar_mask = 0x00ffff00;
	uint32_t ichspi_bbar;

	if (ctlr->bbar) {
		minaddr &= bbar_mask;
		ichspi_bbar = ich_readl(ctlr, ctlr->bbar) & ~bbar_mask;
		ichspi_bbar |= minaddr;
		ich_writel(ctlr, ichspi_bbar, ctlr->bbar);
	}
}

/* @return 1 if the SPI flash supports the 33MHz speed */
static bool ich9_can_do_33mhz(struct udevice *dev)
{
	struct ich_spi_priv *priv = dev_get_priv(dev);
	u32 fdod, speed;

	if (!CONFIG_IS_ENABLED(PCI) || !priv->pch)
		return false;
	/* Observe SPI Descriptor Component Section 0 */
	dm_pci_write_config32(priv->pch, 0xb0, 0x1000);

	/* Extract the Write/Erase SPI Frequency from descriptor */
	dm_pci_read_config32(priv->pch, 0xb4, &fdod);

	/* Bits 23:21 have the fast read clock frequency, 0=20MHz, 1=33MHz */
	speed = (fdod >> 21) & 7;

	return speed == 1;
}

static void spi_lock_down(struct ich_spi_plat *plat, void *sbase)
{
	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = sbase;

		setbits_le16(&ich7_spi->spis, SPIS_LOCK);
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = sbase;

		setbits_le16(&ich9_spi->hsfs, HSFS_FLOCKDN);
	}
}

static bool spi_lock_status(struct ich_spi_plat *plat, void *sbase)
{
	int lock = 0;

	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = sbase;

		lock = readw(&ich7_spi->spis) & SPIS_LOCK;
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = sbase;

		lock = readw(&ich9_spi->hsfs) & HSFS_FLOCKDN;
	}

	return lock != 0;
}

static int spi_setup_opcode(struct ich_spi_priv *ctlr, struct spi_trans *trans,
			    bool lock)
{
	uint16_t optypes;
	uint8_t opmenu[ctlr->menubytes];

	if (!lock) {
		/* The lock is off, so just use index 0. */
		ich_writeb(ctlr, trans->opcode, ctlr->opmenu);
		optypes = ich_readw(ctlr, ctlr->optype);
		optypes = (optypes & 0xfffc) | (trans->type & 0x3);
		ich_writew(ctlr, optypes, ctlr->optype);
		return 0;
	} else {
		/* The lock is on. See if what we need is on the menu. */
		uint8_t optype;
		uint16_t opcode_index;

		/* Write Enable is handled as atomic prefix */
		if (trans->opcode == SPI_OPCODE_WREN)
			return 0;

		read_reg(ctlr, ctlr->opmenu, opmenu, sizeof(opmenu));
		for (opcode_index = 0; opcode_index < ctlr->menubytes;
				opcode_index++) {
			if (opmenu[opcode_index] == trans->opcode)
				break;
		}

		if (opcode_index == ctlr->menubytes) {
			debug("ICH SPI: Opcode %x not found\n", trans->opcode);
			return -EINVAL;
		}

		optypes = ich_readw(ctlr, ctlr->optype);
		optype = (optypes >> (opcode_index * 2)) & 0x3;

		if (optype != trans->type) {
			debug("ICH SPI: Transaction doesn't fit type %d\n",
			      optype);
			return -ENOSPC;
		}
		return opcode_index;
	}
}

/*
 * Wait for up to 6s til status register bit(s) turn 1 (in case wait_til_set
 * below is true) or 0. In case the wait was for the bit(s) to set - write
 * those bits back, which would cause resetting them.
 *
 * Return the last read status value on success or -1 on failure.
 */
static int ich_status_poll(struct ich_spi_priv *ctlr, u16 bitmask,
			   int wait_til_set)
{
	int timeout = 600000; /* This will result in 6s */
	u16 status = 0;

	while (timeout--) {
		status = ich_readw(ctlr, ctlr->status);
		if (wait_til_set ^ ((status & bitmask) == 0)) {
			if (wait_til_set) {
				ich_writew(ctlr, status & bitmask,
					   ctlr->status);
			}
			return status;
		}
		udelay(10);
	}
	debug("ICH SPI: SCIP timeout, read %x, expected %x, wts %x %x\n",
	      status, bitmask, wait_til_set, status & bitmask);

	return -ETIMEDOUT;
}

static void ich_spi_config_opcode(struct udevice *dev)
{
	struct ich_spi_priv *ctlr = dev_get_priv(dev);

	/*
	 * PREOP, OPTYPE, OPMENU1/OPMENU2 registers can be locked down
	 * to prevent accidental or intentional writes. Before they get
	 * locked down, these registers should be initialized properly.
	 */
	ich_writew(ctlr, SPI_OPPREFIX, ctlr->preop);
	ich_writew(ctlr, SPI_OPTYPE, ctlr->optype);
	ich_writel(ctlr, SPI_OPMENU_LOWER, ctlr->opmenu);
	ich_writel(ctlr, SPI_OPMENU_UPPER, ctlr->opmenu + sizeof(u32));
}

static int ich_spi_exec_op_swseq(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct ich_spi_plat *plat = dev_get_plat(bus);
	struct ich_spi_priv *ctlr = dev_get_priv(bus);
	uint16_t control;
	int16_t opcode_index;
	int with_address;
	int status;
	struct spi_trans *trans = &ctlr->trans;
	bool lock = spi_lock_status(plat, ctlr->base);
	int ret = 0;

	trans->in = NULL;
	trans->out = NULL;
	trans->type = 0xFF;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN) {
			trans->in = op->data.buf.in;
			trans->bytesin = op->data.nbytes;
		} else {
			trans->out = op->data.buf.out;
			trans->bytesout = op->data.nbytes;
		}
	}

	if (trans->opcode != op->cmd.opcode)
		trans->opcode = op->cmd.opcode;

	if (lock && trans->opcode == SPI_OPCODE_WRDIS)
		return 0;

	if (trans->opcode == SPI_OPCODE_WREN) {
		/*
		 * Treat Write Enable as Atomic Pre-Op if possible
		 * in order to prevent the Management Engine from
		 * issuing a transaction between WREN and DATA.
		 */
		if (!lock)
			ich_writew(ctlr, trans->opcode, ctlr->preop);
		return 0;
	}

	ret = ich_status_poll(ctlr, SPIS_SCIP, 0);
	if (ret < 0)
		return ret;

	if (plat->ich_version == ICHV_7)
		ich_writew(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);
	else
		ich_writeb(ctlr, SPIS_CDS | SPIS_FCERR, ctlr->status);

	/* Try to guess spi transaction type */
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		if (op->addr.nbytes)
			trans->type = SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS;
		else
			trans->type = SPI_OPCODE_TYPE_WRITE_NO_ADDRESS;
	} else {
		if (op->addr.nbytes)
			trans->type = SPI_OPCODE_TYPE_READ_WITH_ADDRESS;
		else
			trans->type = SPI_OPCODE_TYPE_READ_NO_ADDRESS;
	}
	/* Special erase case handling */
	if (op->addr.nbytes && !op->data.buswidth)
		trans->type = SPI_OPCODE_TYPE_WRITE_WITH_ADDRESS;

	opcode_index = spi_setup_opcode(ctlr, trans, lock);
	if (opcode_index < 0)
		return -EINVAL;

	if (op->addr.nbytes) {
		trans->offset = op->addr.val;
		with_address = 1;
	}

	if (ctlr->speed && ctlr->max_speed >= 33000000) {
		int byte;

		byte = ich_readb(ctlr, ctlr->speed);
		if (ctlr->cur_speed >= 33000000)
			byte |= SSFC_SCF_33MHZ;
		else
			byte &= ~SSFC_SCF_33MHZ;
		ich_writeb(ctlr, byte, ctlr->speed);
	}

	/* Preset control fields */
	control = SPIC_SCGO | ((opcode_index & 0x07) << 4);

	/* Issue atomic preop cycle if needed */
	if (ich_readw(ctlr, ctlr->preop))
		control |= SPIC_ACS;

	if (!trans->bytesout && !trans->bytesin) {
		/* SPI addresses are 24 bit only */
		if (with_address) {
			ich_writel(ctlr, trans->offset & 0x00FFFFFF,
				   ctlr->addr);
		}
		/*
		 * This is a 'no data' command (like Write Enable), its
		 * bitesout size was 1, decremented to zero while executing
		 * spi_setup_opcode() above. Tell the chip to send the
		 * command.
		 */
		ich_writew(ctlr, control, ctlr->control);

		/* wait for the result */
		status = ich_status_poll(ctlr, SPIS_CDS | SPIS_FCERR, 1);
		if (status < 0)
			return status;

		if (status & SPIS_FCERR) {
			debug("ICH SPI: Command transaction error\n");
			return -EIO;
		}

		return 0;
	}

	while (trans->bytesout || trans->bytesin) {
		uint32_t data_length;

		/* SPI addresses are 24 bit only */
		ich_writel(ctlr, trans->offset & 0x00FFFFFF, ctlr->addr);

		if (trans->bytesout)
			data_length = min(trans->bytesout, ctlr->databytes);
		else
			data_length = min(trans->bytesin, ctlr->databytes);

		/* Program data into FDATA0 to N */
		if (trans->bytesout) {
			write_reg(ctlr, trans->out, ctlr->data, data_length);
			trans->bytesout -= data_length;
		}

		/* Add proper control fields' values */
		control &= ~((ctlr->databytes - 1) << 8);
		control |= SPIC_DS;
		control |= (data_length - 1) << 8;

		/* write it */
		ich_writew(ctlr, control, ctlr->control);

		/* Wait for Cycle Done Status or Flash Cycle Error */
		status = ich_status_poll(ctlr, SPIS_CDS | SPIS_FCERR, 1);
		if (status < 0)
			return status;

		if (status & SPIS_FCERR) {
			debug("ICH SPI: Data transaction error %x\n", status);
			return -EIO;
		}

		if (trans->bytesin) {
			read_reg(ctlr, ctlr->data, trans->in, data_length);
			trans->bytesin -= data_length;
		}
	}

	/* Clear atomic preop now that xfer is done */
	if (!lock)
		ich_writew(ctlr, 0, ctlr->preop);

	return 0;
}

/*
 * Ensure read/write xfer len is not greater than SPIBAR_FDATA_FIFO_SIZE and
 * that the operation does not cross page boundary.
 */
static uint get_xfer_len(u32 offset, int len, int page_size)
{
	uint xfer_len = min(len, SPIBAR_FDATA_FIFO_SIZE);
	uint bytes_left = ALIGN(offset, page_size) - offset;

	if (bytes_left)
		xfer_len = min(xfer_len, bytes_left);

	return xfer_len;
}

/* Fill FDATAn FIFO in preparation for a write transaction */
static void fill_xfer_fifo(struct fast_spi_regs *regs, const void *data,
			   uint len)
{
	memcpy(regs->fdata, data, len);
}

/* Drain FDATAn FIFO after a read transaction populates data */
static void drain_xfer_fifo(struct fast_spi_regs *regs, void *dest, uint len)
{
	memcpy(dest, regs->fdata, len);
}

/* Fire up a transfer using the hardware sequencer */
static void start_hwseq_xfer(struct fast_spi_regs *regs, uint hsfsts_cycle,
			     uint offset, uint len)
{
	/* Make sure all W1C status bits get cleared */
	u32 hsfsts;

	hsfsts = readl(&regs->hsfsts_ctl);
	hsfsts &= ~(HSFSTS_FCYCLE_MASK | HSFSTS_FDBC_MASK);
	hsfsts |= HSFSTS_AEL | HSFSTS_FCERR | HSFSTS_FDONE;

	/* Set up transaction parameters */
	hsfsts |= hsfsts_cycle << HSFSTS_FCYCLE_SHIFT;
	hsfsts |= ((len - 1) << HSFSTS_FDBC_SHIFT) & HSFSTS_FDBC_MASK;
	hsfsts |= HSFSTS_FGO;

	writel(offset, &regs->faddr);
	writel(hsfsts, &regs->hsfsts_ctl);
}

static int wait_for_hwseq_xfer(struct fast_spi_regs *regs, uint offset)
{
	ulong start;
	u32 hsfsts;

	start = get_timer(0);
	do {
		hsfsts = readl(&regs->hsfsts_ctl);
		if (hsfsts & HSFSTS_FCERR) {
			debug("SPI transaction error at offset %x HSFSTS = %08x\n",
			      offset, hsfsts);
			return -EIO;
		}
		if (hsfsts & HSFSTS_AEL)
			return -EPERM;

		if (hsfsts & HSFSTS_FDONE)
			return 0;
	} while (get_timer(start) < SPIBAR_HWSEQ_XFER_TIMEOUT_MS);

	debug("SPI transaction timeout at offset %x HSFSTS = %08x, timer %d\n",
	      offset, hsfsts, (uint)get_timer(start));

	return -ETIMEDOUT;
}

/**
 * exec_sync_hwseq_xfer() - Execute flash transfer by hardware sequencing
 *
 * This waits until complete or timeout
 *
 * @regs: SPI registers
 * @hsfsts_cycle: Cycle type (enum hsfsts_cycle_t)
 * @offset: Offset to access
 * @len: Number of bytes to transfer (can be 0)
 * Return: 0 if OK, -EIO on flash-cycle error (FCERR), -EPERM on access error
 *	(AEL), -ETIMEDOUT on timeout
 */
static int exec_sync_hwseq_xfer(struct fast_spi_regs *regs, uint hsfsts_cycle,
				uint offset, uint len)
{
	start_hwseq_xfer(regs, hsfsts_cycle, offset, len);

	return wait_for_hwseq_xfer(regs, offset);
}

static int ich_spi_exec_op_hwseq(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct spi_flash *flash = dev_get_uclass_priv(slave->dev);
	struct udevice *bus = dev_get_parent(slave->dev);
	struct ich_spi_priv *priv = dev_get_priv(bus);
	struct fast_spi_regs *regs = priv->base;
	uint page_size;
	uint offset;
	int cycle;
	uint len;
	bool out;
	int ret;
	u8 *buf;

	offset = op->addr.val;
	len = op->data.nbytes;

	switch (op->cmd.opcode) {
	case SPINOR_OP_RDID:
		cycle = HSFSTS_CYCLE_RDID;
		break;
	case SPINOR_OP_READ_FAST:
		cycle = HSFSTS_CYCLE_READ;
		break;
	case SPINOR_OP_PP:
		cycle = HSFSTS_CYCLE_WRITE;
		break;
	case SPINOR_OP_WREN:
		/* Nothing needs to be done */
		return 0;
	case SPINOR_OP_WRSR:
		cycle = HSFSTS_CYCLE_WR_STATUS;
		break;
	case SPINOR_OP_RDSR:
		cycle = HSFSTS_CYCLE_RD_STATUS;
		break;
	case SPINOR_OP_WRDI:
		return 0;  /* ignore */
	case SPINOR_OP_BE_4K:
		cycle = HSFSTS_CYCLE_4K_ERASE;
		ret = exec_sync_hwseq_xfer(regs, cycle, offset, 0);
		return ret;
	default:
		debug("Unknown cycle %x\n", op->cmd.opcode);
		return -EINVAL;
	};

	out = op->data.dir == SPI_MEM_DATA_OUT;
	buf = out ? (u8 *)op->data.buf.out : op->data.buf.in;
	page_size = flash->page_size ? : 256;

	while (len) {
		uint xfer_len = get_xfer_len(offset, len, page_size);

		if (out)
			fill_xfer_fifo(regs, buf, xfer_len);

		ret = exec_sync_hwseq_xfer(regs, cycle, offset, xfer_len);
		if (ret)
			return ret;

		if (!out)
			drain_xfer_fifo(regs, buf, xfer_len);

		offset += xfer_len;
		buf += xfer_len;
		len -= xfer_len;
	}

	return 0;
}

static int ich_spi_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct ich_spi_plat *plat = dev_get_plat(bus);
	int ret;

	bootstage_start(BOOTSTAGE_ID_ACCUM_SPI, "fast_spi");
	if (plat->hwseq)
		ret = ich_spi_exec_op_hwseq(slave, op);
	else
		ret = ich_spi_exec_op_swseq(slave, op);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_SPI);

	return ret;
}

#if CONFIG_IS_ENABLED(OF_REAL)
/**
 * ich_spi_get_basics() - Get basic information about the ICH device
 *
 * This works without probing any devices if requested.
 *
 * @bus: SPI controller to use
 * @can_probe: true if this function is allowed to probe the PCH
 * @pchp: Returns a pointer to the pch, or NULL if not found
 * @ich_versionp: Returns ICH version detected on success
 * @mmio_basep: Returns the address of the SPI registers on success
 * Return: 0 if OK, -EPROTOTYPE if the PCH could not be found, -EAGAIN if
 *	the function cannot success without probing, possible another error if
 *	pch_get_spi_base() fails
 */
static int ich_spi_get_basics(struct udevice *bus, bool can_probe,
			      struct udevice **pchp,
			      enum ich_version *ich_versionp, ulong *mmio_basep)
{
	struct udevice *pch = NULL;
	int ret = 0;

	/* Find a PCH if there is one */
	if (can_probe) {
		pch = dev_get_parent(bus);
		if (device_get_uclass_id(pch) != UCLASS_PCH) {
			uclass_first_device(UCLASS_PCH, &pch);
			if (!pch)
				; /* ignore this error since we don't need it */
		}
	}

	*ich_versionp = dev_get_driver_data(bus);
	if (*ich_versionp == ICHV_APL)
		*mmio_basep = dm_pci_read_bar32(bus, 0);
	else if (pch)
		ret = pch_get_spi_base(pch, mmio_basep);
	else
		return -EAGAIN;
	*pchp = pch;

	return ret;
}
#endif

/**
 * ich_get_mmap_bus() - Handle the get_mmap() method for a bus
 *
 * There are several cases to consider:
 * 1. Using of-platdata, in which case we have the BDF and can access the
 *	registers by reading the BAR
 * 2. Not using of-platdata, but still with a SPI controller that is on its own
 * PCI PDF. In this case we read the BDF from the parent plat and again get
 *	the registers by reading the BAR
 * 3. Using a SPI controller that is a child of the PCH, in which case we try
 *	to find the registers by asking the PCH. This only works if the PCH has
 *	been probed (which it will be if the bus is probed since parents are
 *	probed before children), since the PCH may not have a PCI address until
 *	its parent (the PCI bus itself) has been probed. If you are using this
 *	method then you should make sure the SPI bus is probed.
 *
 * The first two cases are useful in early init. The last one is more useful
 * afterwards.
 */
static int ich_get_mmap_bus(struct udevice *bus, ulong *map_basep,
			    uint *map_sizep, uint *offsetp)
{
	pci_dev_t spi_bdf;
#if CONFIG_IS_ENABLED(OF_REAL)
	if (device_is_on_pci_bus(bus)) {
		struct pci_child_plat *pplat;

		pplat = dev_get_parent_plat(bus);
		spi_bdf = pplat->devfn;
	} else {
		enum ich_version ich_version;
		struct fast_spi_regs *regs;
		struct udevice *pch;
		ulong mmio_base;
		int ret;

		ret = ich_spi_get_basics(bus, device_active(bus), &pch,
					 &ich_version, &mmio_base);
		if (ret)
			return log_msg_ret("basics", ret);
		regs = (struct fast_spi_regs *)mmio_base;

		return fast_spi_get_bios_mmap_regs(regs, map_basep, map_sizep,
						   offsetp);
	}
#else
	struct ich_spi_plat *plat = dev_get_plat(bus);

	/*
	 * We cannot rely on plat->bdf being set up yet since this method can
	 * be called before the device is probed. Use the of-platdata directly
	 * instead.
	 */
	spi_bdf = pci_ofplat_get_devfn(plat->dtplat.reg[0]);
#endif

	return fast_spi_get_bios_mmap(spi_bdf, map_basep, map_sizep, offsetp);
}

static int ich_get_mmap(struct udevice *dev, ulong *map_basep, uint *map_sizep,
			uint *offsetp)
{
	struct udevice *bus = dev_get_parent(dev);

	return ich_get_mmap_bus(bus, map_basep, map_sizep, offsetp);
}

static int ich_spi_adjust_size(struct spi_slave *slave, struct spi_mem_op *op)
{
	unsigned int page_offset;
	int addr = op->addr.val;
	unsigned int byte_count = op->data.nbytes;

	if (hweight32(ICH_BOUNDARY) == 1) {
		page_offset = addr & (ICH_BOUNDARY - 1);
	} else {
		u64 aux = addr;

		page_offset = do_div(aux, ICH_BOUNDARY);
	}

	if (op->data.dir == SPI_MEM_DATA_IN) {
		if (slave->max_read_size) {
			op->data.nbytes = min(ICH_BOUNDARY - page_offset,
					      slave->max_read_size);
		}
	} else if (slave->max_write_size) {
		op->data.nbytes = min(ICH_BOUNDARY - page_offset,
				      slave->max_write_size);
	}

	op->data.nbytes = min(op->data.nbytes, byte_count);

	return 0;
}

static int ich_protect_lockdown(struct udevice *dev)
{
	struct ich_spi_plat *plat = dev_get_plat(dev);
	struct ich_spi_priv *priv = dev_get_priv(dev);
	int ret = -ENOSYS;

	/* Disable the BIOS write protect so write commands are allowed */
	if (priv->pch)
		ret = pch_set_spi_protect(priv->pch, false);
	if (ret == -ENOSYS) {
		u8 bios_cntl;

		bios_cntl = ich_readb(priv, priv->bcr);
		bios_cntl &= ~BIT(5);	/* clear Enable InSMM_STS (EISS) */
		bios_cntl |= 1;		/* Write Protect Disable (WPD) */
		ich_writeb(priv, bios_cntl, priv->bcr);
	} else if (ret) {
		debug("%s: Failed to disable write-protect: err=%d\n",
		      __func__, ret);
		return ret;
	}

	/* Lock down SPI controller settings if required */
	if (plat->lockdown) {
		ich_spi_config_opcode(dev);
		spi_lock_down(plat, priv->base);
	}

	return 0;
}

static int ich_init_controller(struct udevice *dev,
			       struct ich_spi_plat *plat,
			       struct ich_spi_priv *ctlr)
{
	if (spl_phase() == PHASE_TPL) {
		struct ich_spi_plat *plat = dev_get_plat(dev);
		int ret;

		ret = fast_spi_early_init(plat->bdf, plat->mmio_base);
		if (ret)
			return ret;
	}

	ctlr->base = (void *)plat->mmio_base;
	if (plat->ich_version == ICHV_7) {
		struct ich7_spi_regs *ich7_spi = ctlr->base;

		ctlr->opmenu = offsetof(struct ich7_spi_regs, opmenu);
		ctlr->menubytes = sizeof(ich7_spi->opmenu);
		ctlr->optype = offsetof(struct ich7_spi_regs, optype);
		ctlr->addr = offsetof(struct ich7_spi_regs, spia);
		ctlr->data = offsetof(struct ich7_spi_regs, spid);
		ctlr->databytes = sizeof(ich7_spi->spid);
		ctlr->status = offsetof(struct ich7_spi_regs, spis);
		ctlr->control = offsetof(struct ich7_spi_regs, spic);
		ctlr->bbar = offsetof(struct ich7_spi_regs, bbar);
		ctlr->preop = offsetof(struct ich7_spi_regs, preop);
	} else if (plat->ich_version == ICHV_9) {
		struct ich9_spi_regs *ich9_spi = ctlr->base;

		ctlr->opmenu = offsetof(struct ich9_spi_regs, opmenu);
		ctlr->menubytes = sizeof(ich9_spi->opmenu);
		ctlr->optype = offsetof(struct ich9_spi_regs, optype);
		ctlr->addr = offsetof(struct ich9_spi_regs, faddr);
		ctlr->data = offsetof(struct ich9_spi_regs, fdata);
		ctlr->databytes = sizeof(ich9_spi->fdata);
		ctlr->status = offsetof(struct ich9_spi_regs, ssfs);
		ctlr->control = offsetof(struct ich9_spi_regs, ssfc);
		ctlr->speed = ctlr->control + 2;
		ctlr->bbar = offsetof(struct ich9_spi_regs, bbar);
		ctlr->preop = offsetof(struct ich9_spi_regs, preop);
		ctlr->bcr = offsetof(struct ich9_spi_regs, bcr);
		ctlr->pr = &ich9_spi->pr[0];
	} else if (plat->ich_version == ICHV_APL) {
	} else {
		debug("ICH SPI: Unrecognised ICH version %d\n",
		      plat->ich_version);
		return -EINVAL;
	}

	/* Work out the maximum speed we can support */
	ctlr->max_speed = 20000000;
	if (plat->ich_version == ICHV_9 && ich9_can_do_33mhz(dev))
		ctlr->max_speed = 33000000;
	debug("ICH SPI: Version ID %d detected at %lx, speed %ld\n",
	      plat->ich_version, plat->mmio_base, ctlr->max_speed);

	ich_set_bbar(ctlr, 0);

	return 0;
}

static int ich_cache_bios_region(struct udevice *dev)
{
	ulong map_base;
	uint map_size;
	uint offset;
	ulong base;
	int ret;

	ret = ich_get_mmap_bus(dev, &map_base, &map_size, &offset);
	if (ret)
		return ret;

	/* Don't use WRBACK since we are not supposed to write to SPI flash */
	base = SZ_4G - map_size;
	mtrr_set_next_var(MTRR_TYPE_WRPROT, base, map_size);
	log_debug("BIOS cache base=%lx, size=%x\n", base, (uint)map_size);

	return 0;
}

static int ich_spi_probe(struct udevice *dev)
{
	struct ich_spi_plat *plat = dev_get_plat(dev);
	struct ich_spi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ich_init_controller(dev, plat, priv);
	if (ret)
		return ret;

	if (spl_phase() == PHASE_TPL) {
		/* Cache the BIOS to speed things up */
		ret = ich_cache_bios_region(dev);
		if (ret)
			return ret;
	} else {
		ret = ich_protect_lockdown(dev);
		if (ret)
			return ret;
	}
	priv->cur_speed = priv->max_speed;

	return 0;
}

static int ich_spi_remove(struct udevice *bus)
{
	/*
	 * Configure SPI controller so that the Linux MTD driver can fully
	 * access the SPI NOR chip
	 */
	ich_spi_config_opcode(bus);

	return 0;
}

static int ich_spi_set_speed(struct udevice *bus, uint speed)
{
	struct ich_spi_priv *priv = dev_get_priv(bus);

	priv->cur_speed = speed;

	return 0;
}

static int ich_spi_set_mode(struct udevice *bus, uint mode)
{
	debug("%s: mode=%d\n", __func__, mode);

	return 0;
}

static int ich_spi_child_pre_probe(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ich_spi_plat *plat = dev_get_plat(bus);
	struct ich_spi_priv *priv = dev_get_priv(bus);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	/*
	 * Yes this controller can only transfer a small number of bytes at
	 * once! The limit is typically 64 bytes. For hardware sequencing a
	 * a loop is used to get around this.
	 */
	if (!plat->hwseq) {
		slave->max_read_size = priv->databytes;
		slave->max_write_size = priv->databytes;
	}
	/*
	 * ICH 7 SPI controller only supports array read command
	 * and byte program command for SST flash
	 */
	if (plat->ich_version == ICHV_7)
		slave->mode = SPI_RX_SLOW | SPI_TX_BYTE;

	return 0;
}

static int ich_spi_of_to_plat(struct udevice *dev)
{
	struct ich_spi_plat *plat = dev_get_plat(dev);

#if CONFIG_IS_ENABLED(OF_REAL)
	struct ich_spi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ich_spi_get_basics(dev, true, &priv->pch, &plat->ich_version,
				 &plat->mmio_base);
	if (ret)
		return log_msg_ret("basics", ret);
	plat->lockdown = dev_read_bool(dev, "intel,spi-lock-down");
	/*
	 * Use an int so that the property is present in of-platdata even
	 * when false.
	 */
	plat->hwseq = dev_read_u32_default(dev, "intel,hardware-seq", 0);
#else
	plat->ich_version = ICHV_APL;
	plat->mmio_base = plat->dtplat.early_regs[0];
	plat->bdf = pci_ofplat_get_devfn(plat->dtplat.reg[0]);
	plat->hwseq = plat->dtplat.intel_hardware_seq;
#endif
	debug("%s: mmio_base=%lx\n", __func__, plat->mmio_base);

	return 0;
}

static const struct spi_controller_mem_ops ich_controller_mem_ops = {
	.adjust_op_size	= ich_spi_adjust_size,
	.supports_op	= NULL,
	.exec_op	= ich_spi_exec_op,
};

static const struct dm_spi_ops ich_spi_ops = {
	/* xfer is not supported */
	.set_speed	= ich_spi_set_speed,
	.set_mode	= ich_spi_set_mode,
	.mem_ops	= &ich_controller_mem_ops,
	.get_mmap	= ich_get_mmap,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id ich_spi_ids[] = {
	{ .compatible = "intel,ich7-spi", ICHV_7 },
	{ .compatible = "intel,ich9-spi", ICHV_9 },
	{ .compatible = "intel,fast-spi", ICHV_APL },
	{ }
};

U_BOOT_DRIVER(intel_fast_spi) = {
	.name	= "intel_fast_spi",
	.id	= UCLASS_SPI,
	.of_match = ich_spi_ids,
	.ops	= &ich_spi_ops,
	.of_to_plat = ich_spi_of_to_plat,
	.plat_auto	= sizeof(struct ich_spi_plat),
	.priv_auto	= sizeof(struct ich_spi_priv),
	.child_pre_probe = ich_spi_child_pre_probe,
	.probe	= ich_spi_probe,
	.remove	= ich_spi_remove,
	.flags	= DM_FLAG_OS_PREPARE,
};
