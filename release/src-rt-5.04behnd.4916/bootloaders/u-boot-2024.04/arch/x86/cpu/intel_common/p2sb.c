// SPDX-License-Identifier: GPL-2.0
/*
 * Primary-to-Sideband Bridge
 *
 * Copyright 2019 Google LLC
 */

#define LOG_CATEGORY UCLASS_P2SB

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <log.h>
#include <p2sb.h>
#include <spl.h>
#include <asm/p2sb.h>
#include <asm/pci.h>
#include <linux/bitops.h>

#define PCH_P2SB_E0		0xe0
#define HIDE_BIT		BIT(0)

/* PCI config space registers */
#define HPTC_OFFSET		0x60
#define HPTC_ADDR_ENABLE_BIT	BIT(7)

/* High Performance Event Timer Configuration */
#define P2SB_HPTC				0x60
#define P2SB_HPTC_ADDRESS_ENABLE		BIT(7)

/*
 * ADDRESS_SELECT            ENCODING_RANGE
 *      0                 0xfed0 0000 - 0xfed0 03ff
 *      1                 0xfed0 1000 - 0xfed0 13ff
 *      2                 0xfed0 2000 - 0xfed0 23ff
 *      3                 0xfed0 3000 - 0xfed0 33ff
 */
#define P2SB_HPTC_ADDRESS_SELECT_0		(0 << 0)
#define P2SB_HPTC_ADDRESS_SELECT_1		(1 << 0)
#define P2SB_HPTC_ADDRESS_SELECT_2		(2 << 0)
#define P2SB_HPTC_ADDRESS_SELECT_3		(3 << 0)

/*
 * p2sb_early_init() - Enable decoding for HPET range
 *
 * This is needed by FSP-M which uses the High Precision Event Timer.
 *
 * @dev: P2SB device
 * Return: 0 if OK, -ve on error
 */
static int p2sb_early_init(struct udevice *dev)
{
	struct p2sb_plat *plat = dev_get_plat(dev);
	pci_dev_t pdev = plat->bdf;

	/*
	 * Enable decoding for HPET memory address range.
	 * HPTC_OFFSET(0x60) bit 7, when set the P2SB will decode
	 * the High Performance Timer memory address range
	 * selected by bits 1:0
	 */
	pci_x86_write_config(pdev, HPTC_OFFSET, HPTC_ADDR_ENABLE_BIT,
			     PCI_SIZE_8);

	/* Enable PCR Base address in PCH */
	pci_x86_write_config(pdev, PCI_BASE_ADDRESS_0, plat->mmio_base,
			     PCI_SIZE_32);
	pci_x86_write_config(pdev, PCI_BASE_ADDRESS_1, 0, PCI_SIZE_32);

	/* Enable P2SB MSE */
	pci_x86_write_config(pdev, PCI_COMMAND, PCI_COMMAND_MASTER |
			     PCI_COMMAND_MEMORY, PCI_SIZE_8);

	return 0;
}

static int p2sb_spl_init(struct udevice *dev)
{
	/* Enable decoding for HPET. Needed for FSP global pointer storage */
	dm_pci_write_config(dev, P2SB_HPTC, P2SB_HPTC_ADDRESS_SELECT_0 |
			    P2SB_HPTC_ADDRESS_ENABLE, PCI_SIZE_8);

	return 0;
}

int p2sb_of_to_plat(struct udevice *dev)
{
	struct p2sb_uc_priv *upriv = dev_get_uclass_priv(dev);
	struct p2sb_plat *plat = dev_get_plat(dev);

#if CONFIG_IS_ENABLED(OF_REAL)
	int ret;
	u32 base[2];

	ret = dev_read_u32_array(dev, "early-regs", base, ARRAY_SIZE(base));
	if (ret)
		return log_msg_ret("Missing/short early-regs", ret);
	plat->mmio_base = base[0];
	/* TPL sets up the initial BAR */
	if (spl_phase() == PHASE_TPL) {
		plat->bdf = pci_get_devfn(dev);
		if (plat->bdf < 0)
			return log_msg_ret("Cannot get p2sb PCI address",
					   plat->bdf);
	}
	upriv->mmio_base = plat->mmio_base;
#else
	plat->mmio_base = plat->dtplat.early_regs[0];
	plat->bdf = pci_ofplat_get_devfn(plat->dtplat.reg[0]);
	upriv->mmio_base = plat->mmio_base;
#endif

	return 0;
}

static int p2sb_probe(struct udevice *dev)
{
	if (spl_phase() == PHASE_TPL)
		return p2sb_early_init(dev);
	else if (spl_phase() == PHASE_SPL)
		return p2sb_spl_init(dev);

	return 0;
}

static void p2sb_set_hide_bit(struct udevice *dev, bool hide)
{
	dm_pci_clrset_config8(dev, PCH_P2SB_E0 + 1, HIDE_BIT,
			      hide ? HIDE_BIT : 0);
}

static int intel_p2sb_set_hide(struct udevice *dev, bool hide)
{
	u16 vendor;

	if (!CONFIG_IS_ENABLED(PCI))
		return -EPERM;
	p2sb_set_hide_bit(dev, hide);

	dm_pci_read_config16(dev, PCI_VENDOR_ID, &vendor);
	if (hide && vendor != 0xffff)
		return log_msg_ret("hide", -EEXIST);
	else if (!hide && vendor != PCI_VENDOR_ID_INTEL)
		return log_msg_ret("unhide", -ENOMEDIUM);

	return 0;
}

static int p2sb_remove(struct udevice *dev)
{
	int ret;

	ret = intel_p2sb_set_hide(dev, true);
	if (ret)
		return log_msg_ret("hide", ret);

	return 0;
}

static int p2sb_child_post_bind(struct udevice *dev)
{
	if (CONFIG_IS_ENABLED(OF_REAL)) {
		struct p2sb_child_plat *pplat = dev_get_parent_plat(dev);
		int ret;
		u32 pid;

		ret = dev_read_u32(dev, "intel,p2sb-port-id", &pid);
		if (ret)
			return ret;
		pplat->pid = pid;
	}

	return 0;
}

static const struct p2sb_ops p2sb_ops = {
	.set_hide	= intel_p2sb_set_hide,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id p2sb_ids[] = {
	{ .compatible = "intel,p2sb" },
	{ }
};
#endif

U_BOOT_DRIVER(intel_p2sb) = {
	.name		= "intel_p2sb",
	.id		= UCLASS_P2SB,
	.of_match	= of_match_ptr(p2sb_ids),
	.probe		= p2sb_probe,
	.remove		= p2sb_remove,
	.ops		= &p2sb_ops,
	.of_to_plat = p2sb_of_to_plat,
	.plat_auto	= sizeof(struct p2sb_plat),
	.per_child_plat_auto	= sizeof(struct p2sb_child_plat),
	.child_post_bind = p2sb_child_post_bind,
	.flags		= DM_FLAG_OS_PREPARE,
};
