// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <axi.h>
#include <dm.h>
#include <asm/axi.h>

/*
 * This driver implements a AXI bus for the sandbox architecture for testing
 * purposes.
 *
 * The bus forwards every access to it to a special AXI emulation device (which
 * it gets via the axi_emul_get_ops function) that implements a simple
 * read/write storage.
 *
 * The emulator device must still be contained in the device tree in the usual
 * way, since configuration data for the storage is read from the DT.
 */

static int axi_sandbox_read(struct udevice *bus, ulong address, void *data,
			    enum axi_size_t size)
{
	struct axi_emul_ops *ops;
	struct udevice *emul;
	int ret;

	/* Get emulator device */
	ret = axi_sandbox_get_emul(bus, address, size, &emul);
	if (ret)
		return ret == -ENODEV ? 0 : ret;
	/* Forward all reads to the AXI emulator */
	ops = axi_emul_get_ops(emul);
	if (!ops || !ops->read)
		return -ENOSYS;

	return ops->read(emul, address, data, size);
}

static int axi_sandbox_write(struct udevice *bus, ulong address, void *data,
			     enum axi_size_t size)
{
	struct axi_emul_ops *ops;
	struct udevice *emul;
	int ret;

	/* Get emulator device */
	ret = axi_sandbox_get_emul(bus, address, size, &emul);
	if (ret)
		return ret == -ENODEV ? 0 : ret;
	/* Forward all writes to the AXI emulator */
	ops = axi_emul_get_ops(emul);
	if (!ops || !ops->write)
		return -ENOSYS;

	return ops->write(emul, address, data, size);
}

static const struct udevice_id axi_sandbox_ids[] = {
	{ .compatible = "sandbox,axi" },
	{ /* sentinel */ }
};

static const struct axi_ops axi_sandbox_ops = {
	.read = axi_sandbox_read,
	.write = axi_sandbox_write,
};

U_BOOT_DRIVER(axi_sandbox_bus) = {
	.name           = "axi_sandbox_bus",
	.id             = UCLASS_AXI,
	.of_match       = axi_sandbox_ids,
	.ops		= &axi_sandbox_ops,
};
