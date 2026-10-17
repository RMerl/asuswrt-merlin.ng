// SPDX-License-Identifier: GPL-2.0
/*
 * driver for mmio TCG/TIS TPM (trusted platform module).
 *
 * Specifications at www.trustedcomputinggroup.org
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <tpm-v2.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/unaligned/be_byteshift.h>
#include "tpm_tis.h"
#include "tpm_internal.h"

/**
 * struct tpm_tis_chip_data - Information about an MMIO TPM
 * @pcr_count:          Number of PCR per bank
 * @pcr_select_min:	Minimum size in bytes of the pcrSelect array
 * @iobase:		Base address
 */
struct tpm_tis_chip_data {
	unsigned int pcr_count;
	unsigned int pcr_select_min;
	void __iomem *iobase;
};

static int mmio_read_bytes(struct udevice *dev, u32 addr, u16 len,
			   u8 *result)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);

	while (len--)
		*result++ = ioread8(drv_data->iobase + addr);

	return 0;
}

static int mmio_write_bytes(struct udevice *dev, u32 addr, u16 len,
			    const u8 *value)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);

	while (len--)
		iowrite8(*value++, drv_data->iobase + addr);

	return 0;
}

static int mmio_read32(struct udevice *dev, u32 addr, u32 *result)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);

	*result = ioread32(drv_data->iobase + addr);

	return 0;
}

static int mmio_write32(struct udevice *dev, u32 addr, u32 value)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);

	iowrite32(value, drv_data->iobase + addr);

	return 0;
}

static struct tpm_tis_phy_ops phy_ops = {
	.read_bytes = mmio_read_bytes,
	.write_bytes = mmio_write_bytes,
	.read32 = mmio_read32,
	.write32 = mmio_write32,
};

static int tpm_tis_probe(struct udevice *dev)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	int ret = 0;
	fdt_addr_t ioaddr;
	u64 sz;

	ioaddr = dev_read_addr(dev);
	if (ioaddr == FDT_ADDR_T_NONE)
		return log_msg_ret("ioaddr", -EINVAL);

	ret = dev_read_u64(dev, "reg", &sz);
	if (ret)
		return -EINVAL;

	drv_data->iobase = ioremap(ioaddr, sz);
	tpm_tis_ops_register(dev, &phy_ops);
	ret = tpm_tis_init(dev);
	if (ret)
		goto iounmap;

	priv->pcr_count = drv_data->pcr_count;
	priv->pcr_select_min = drv_data->pcr_select_min;
	/*
	 * Although the driver probably works with a TPMv1 our Kconfig
	 * limits the driver to TPMv2 only
	 */
	priv->version = TPM_V2;

	return ret;
iounmap:
	iounmap(drv_data->iobase);

	return -EINVAL;
}

static int tpm_tis_remove(struct udevice *dev)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);
	int ret;

	ret = tpm_tis_cleanup(dev);

	iounmap(drv_data->iobase);

	return ret;
}

static const struct tpm_ops tpm_tis_ops = {
	.open		= tpm_tis_open,
	.close		= tpm_tis_close,
	.get_desc	= tpm_tis_get_desc,
	.send		= tpm_tis_send,
	.recv		= tpm_tis_recv,
	.cleanup	= tpm_tis_cleanup,
};

static const struct tpm_tis_chip_data tpm_tis_std_chip_data = {
	.pcr_count = 24,
	.pcr_select_min = 3,
};

static const struct udevice_id tpm_tis_ids[] = {
	{
		.compatible = "tcg,tpm-tis-mmio",
		.data = (ulong)&tpm_tis_std_chip_data,
	},
	{ }
};

U_BOOT_DRIVER(tpm_tis_mmio) = {
	.name   = "tpm_tis_mmio",
	.id     = UCLASS_TPM,
	.of_match = tpm_tis_ids,
	.ops    = &tpm_tis_ops,
	.probe	= tpm_tis_probe,
	.remove	= tpm_tis_remove,
	.priv_auto	= sizeof(struct tpm_chip),
};
