// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * (C) Copyright 2017, 2018
 * Mario Six,  Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <axi.h>
#include <dm.h>
#include <regmap.h>

/**
 * struct ihs_axi_regs - Structure for the register map of a IHS AXI device
 * @interrupt_status:         Status register to indicate certain events (e.g.
 *			      error during transfer, transfer complete, etc.)
 * @interrupt_enable_control: Register to both control which statuses will be
 *			      indicated in the interrupt_status register, and
 *			      to change bus settings
 * @address_lsb:              Least significant 16-bit word of the address of a
 *			      device to transfer data from/to
 * @address_msb:              Most significant 16-bit word of the address of a
 *			      device to transfer data from/to
 * @write_data_lsb:           Least significant 16-bit word of the data to be
 *			      written to a device
 * @write_data_msb:           Most significant 16-bit word of the data to be
 *			      written to a device
 * @read_data_lsb:            Least significant 16-bit word of the data read
 *			      from a device
 * @read_data_msb:            Most significant 16-bit word of the data read
 *			      from a device
 */
struct ihs_axi_regs {
	u16 interrupt_status;
	u16 interrupt_enable_control;
	u16 address_lsb;
	u16 address_msb;
	u16 write_data_lsb;
	u16 write_data_msb;
	u16 read_data_lsb;
	u16 read_data_msb;
};

/**
 * ihs_axi_set() - Convenience macro to set values in register map
 * @map:    The register map to write to
 * @member: The member of the ihs_axi_regs structure to write
 * @val:    The value to write to the register map
 */
#define ihs_axi_set(map, member, val) \
	regmap_set(map, struct ihs_axi_regs, member, val)

/**
 * ihs_axi_get() - Convenience macro to read values from register map
 * @map:    The register map to read from
 * @member: The member of the ihs_axi_regs structure to read
 * @valp:   Pointer to a buffer to receive the value read
 */
#define ihs_axi_get(map, member, valp) \
	regmap_get(map, struct ihs_axi_regs, member, valp)

/**
 * struct ihs_axi_priv - Private data structure of IHS AXI devices
 * @map: Register map for the IHS AXI device
 */
struct ihs_axi_priv {
	struct regmap *map;
};

/**
 * enum status_reg - Description of bits in the interrupt_status register
 * @STATUS_READ_COMPLETE_EVENT:  A read transfer was completed
 * @STATUS_WRITE_COMPLETE_EVENT: A write transfer was completed
 * @STATUS_TIMEOUT_EVENT:        A timeout has occurred during the transfer
 * @STATUS_ERROR_EVENT:          A error has occurred during the transfer
 * @STATUS_AXI_INT:              A AXI interrupt has occurred
 * @STATUS_READ_DATA_AVAILABLE:  Data is available to be read
 * @STATUS_BUSY:                 The bus is busy
 * @STATUS_INIT_DONE:            The bus has finished initializing
 */
enum status_reg {
	STATUS_READ_COMPLETE_EVENT = BIT(15),
	STATUS_WRITE_COMPLETE_EVENT = BIT(14),
	STATUS_TIMEOUT_EVENT = BIT(13),
	STATUS_ERROR_EVENT = BIT(12),
	STATUS_AXI_INT = BIT(11),
	STATUS_READ_DATA_AVAILABLE = BIT(7),
	STATUS_BUSY = BIT(6),
	STATUS_INIT_DONE = BIT(5),
};

/**
 * enum control_reg - Description of bit fields in the interrupt_enable_control
 *		      register
 * @CONTROL_READ_COMPLETE_EVENT_ENABLE:  STATUS_READ_COMPLETE_EVENT will be
 *					 raised in the interrupt_status register
 * @CONTROL_WRITE_COMPLETE_EVENT_ENABLE: STATUS_WRITE_COMPLETE_EVENT will be
 *					 raised in the interrupt_status register
 * @CONTROL_TIMEOUT_EVENT_ENABLE:        STATUS_TIMEOUT_EVENT will be raised in
 *					 the interrupt_status register
 * @CONTROL_ERROR_EVENT_ENABLE:          STATUS_ERROR_EVENT will be raised in
 *					 the interrupt_status register
 * @CONTROL_AXI_INT_ENABLE:              STATUS_AXI_INT will be raised in the
 *					 interrupt_status register
 * @CONTROL_CMD_NOP:                     Configure bus to send a NOP command
 *					 for the next transfer
 * @CONTROL_CMD_WRITE:                   Configure bus to do a write transfer
 * @CONTROL_CMD_WRITE_POST_INC:          Auto-increment address after write
 *					 transfer
 * @CONTROL_CMD_READ:                    Configure bus to do a read transfer
 * @CONTROL_CMD_READ_POST_INC:           Auto-increment address after read
 *					 transfer
 */
enum control_reg {
	CONTROL_READ_COMPLETE_EVENT_ENABLE = BIT(15),
	CONTROL_WRITE_COMPLETE_EVENT_ENABLE = BIT(14),
	CONTROL_TIMEOUT_EVENT_ENABLE = BIT(13),
	CONTROL_ERROR_EVENT_ENABLE = BIT(12),
	CONTROL_AXI_INT_ENABLE = BIT(11),

	CONTROL_CMD_NOP = 0x0,
	CONTROL_CMD_WRITE = 0x8,
	CONTROL_CMD_WRITE_POST_INC = 0x9,
	CONTROL_CMD_READ = 0xa,
	CONTROL_CMD_READ_POST_INC = 0xb,
};

/**
 * enum axi_cmd - Determine if transfer is read or write transfer
 * @AXI_CMD_READ:  The transfer should be a read transfer
 * @AXI_CMD_WRITE: The transfer should be a write transfer
 */
enum axi_cmd {
	AXI_CMD_READ,
	AXI_CMD_WRITE,
};

/**
 * ihs_axi_transfer() - Run transfer on the AXI bus
 * @bus:           The AXI bus device on which to run the transfer on
 * @address:       The address to use in the transfer (i.e. which address to
 *		   read/write from/to)
 * @cmd:           Should the transfer be a read or write transfer?
 *
 * Return: 0 if OK, -ve on error
 */
static int ihs_axi_transfer(struct udevice *bus, ulong address,
			    enum axi_cmd cmd)
{
	struct ihs_axi_priv *priv = dev_get_priv(bus);
	/* Try waiting for events up to 10 times */
	const uint WAIT_TRIES = 10;
	u16 wait_mask = STATUS_TIMEOUT_EVENT |
			STATUS_ERROR_EVENT;
	u16 complete_flag;
	u16 status;
	uint k;

	if (cmd == AXI_CMD_READ) {
		complete_flag = STATUS_READ_COMPLETE_EVENT;
		cmd = CONTROL_CMD_READ;
	} else {
		complete_flag = STATUS_WRITE_COMPLETE_EVENT;
		cmd = CONTROL_CMD_WRITE;
	}

	wait_mask |= complete_flag;

	/* Lower 16 bit */
	ihs_axi_set(priv->map, address_lsb, address & 0xffff);
	/* Upper 16 bit */
	ihs_axi_set(priv->map, address_msb, (address >> 16) & 0xffff);

	ihs_axi_set(priv->map, interrupt_status, wait_mask);
	ihs_axi_set(priv->map, interrupt_enable_control, cmd);

	for (k = WAIT_TRIES; k > 0; --k) {
		ihs_axi_get(priv->map, interrupt_status, &status);
		if (status & wait_mask)
			break;
		udelay(1);
	}

	/*
	 * k == 0 -> Tries ran out with no event we were waiting for actually
	 * occurring.
	 */
	if (!k)
		ihs_axi_get(priv->map, interrupt_status, &status);

	if (status & complete_flag)
		return 0;

	if (status & STATUS_ERROR_EVENT) {
		debug("%s: Error occurred during transfer\n", bus->name);
		return -EIO;
	}

	debug("%s: Transfer timed out\n", bus->name);
	return -ETIMEDOUT;
}

/*
 * API
 */

static int ihs_axi_read(struct udevice *dev, ulong address, void *data,
			enum axi_size_t size)
{
	struct ihs_axi_priv *priv = dev_get_priv(dev);
	int ret;
	u16 data_lsb, data_msb;
	u32 *p = data;

	if (size != AXI_SIZE_32) {
		debug("%s: transfer size '%d' not supported\n",
		      dev->name, size);
		return -ENOSYS;
	}

	ret = ihs_axi_transfer(dev, address, AXI_CMD_READ);
	if (ret < 0) {
		debug("%s: Error during AXI transfer (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	ihs_axi_get(priv->map, read_data_lsb, &data_lsb);
	ihs_axi_get(priv->map, read_data_msb, &data_msb);

	/* Assemble data from two 16-bit words */
	*p = (data_msb << 16) | data_lsb;

	return 0;
}

static int ihs_axi_write(struct udevice *dev, ulong address, void *data,
			 enum axi_size_t size)
{
	struct ihs_axi_priv *priv = dev_get_priv(dev);
	int ret;
	u32 *p = data;

	if (size != AXI_SIZE_32) {
		debug("%s: transfer size '%d' not supported\n",
		      dev->name, size);
		return -ENOSYS;
	}

	/* Lower 16 bit */
	ihs_axi_set(priv->map, write_data_lsb, *p & 0xffff);
	/* Upper 16 bit */
	ihs_axi_set(priv->map, write_data_msb, (*p >> 16) & 0xffff);

	ret = ihs_axi_transfer(dev, address, AXI_CMD_WRITE);
	if (ret < 0) {
		debug("%s: Error during AXI transfer (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id ihs_axi_ids[] = {
	{ .compatible = "gdsys,ihs_axi" },
	{ /* sentinel */ }
};

static const struct axi_ops ihs_axi_ops = {
	.read = ihs_axi_read,
	.write = ihs_axi_write,
};

static int ihs_axi_probe(struct udevice *dev)
{
	struct ihs_axi_priv *priv = dev_get_priv(dev);

	regmap_init_mem(dev_ofnode(dev), &priv->map);

	return 0;
}

U_BOOT_DRIVER(ihs_axi_bus) = {
	.name           = "ihs_axi_bus",
	.id             = UCLASS_AXI,
	.of_match       = ihs_axi_ids,
	.ops		= &ihs_axi_ops,
	.priv_auto_alloc_size = sizeof(struct ihs_axi_priv),
	.probe          = ihs_axi_probe,
};
