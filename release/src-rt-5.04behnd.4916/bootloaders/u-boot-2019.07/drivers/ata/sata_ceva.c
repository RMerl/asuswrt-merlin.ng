// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 - 2016 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */
#include <common.h>
#include <dm.h>
#include <ahci.h>
#include <scsi.h>
#include <asm/io.h>
#include <linux/ioport.h>

/* Vendor Specific Register Offsets */
#define AHCI_VEND_PCFG  0xA4
#define AHCI_VEND_PPCFG 0xA8
#define AHCI_VEND_PP2C  0xAC
#define AHCI_VEND_PP3C  0xB0
#define AHCI_VEND_PP4C  0xB4
#define AHCI_VEND_PP5C  0xB8
#define AHCI_VEND_AXICC 0xBc
#define AHCI_VEND_PAXIC 0xC0
#define AHCI_VEND_PTC   0xC8

/* Vendor Specific Register bit definitions */
#define PAXIC_ADBW_BW64 0x1
#define PAXIC_MAWIDD	(1 << 8)
#define PAXIC_MARIDD	(1 << 16)
#define PAXIC_OTL	(0x4 << 20)

#define PCFG_TPSS_VAL	(0x32 << 16)
#define PCFG_TPRS_VAL	(0x2 << 12)
#define PCFG_PAD_VAL	0x2

#define PPCFG_TTA	0x1FFFE
#define PPCFG_PSSO_EN	(1 << 28)
#define PPCFG_PSS_EN	(1 << 29)
#define PPCFG_ESDF_EN	(1 << 31)

#define PP2C_CIBGMN	0x0F
#define PP2C_CIBGMX	(0x25 << 8)
#define PP2C_CIBGN	(0x18 << 16)
#define PP2C_CINMP	(0x29 << 24)

#define PP3C_CWBGMN	0x04
#define PP3C_CWBGMX	(0x0B << 8)
#define PP3C_CWBGN	(0x08 << 16)
#define PP3C_CWNMP	(0x0F << 24)

#define PP4C_BMX	0x0a
#define PP4C_BNM	(0x08 << 8)
#define PP4C_SFD	(0x4a << 16)
#define PP4C_PTST	(0x06 << 24)

#define PP5C_RIT	0x60216
#define PP5C_RCT	(0x7f0 << 20)

#define PTC_RX_WM_VAL	0x40
#define PTC_RSVD	(1 << 27)

#define PORT0_BASE	0x100
#define PORT1_BASE	0x180

/* Port Control Register Bit Definitions */
#define PORT_SCTL_SPD_GEN3	(0x3 << 4)
#define PORT_SCTL_SPD_GEN2	(0x2 << 4)
#define PORT_SCTL_SPD_GEN1	(0x1 << 4)
#define PORT_SCTL_IPM		(0x3 << 8)

#define PORT_BASE	0x100
#define PORT_OFFSET	0x80
#define NR_PORTS	2
#define DRV_NAME	"ahci-ceva"
#define CEVA_FLAG_BROKEN_GEN2	1

/* flag bit definition */
#define FLAG_COHERENT	1

/* register config value */
#define CEVA_PHY1_CFG	0xa003fffe
#define CEVA_PHY2_CFG	0x28184d1f
#define CEVA_PHY3_CFG	0x0e081509
#define CEVA_TRANS_CFG	0x08000029
#define CEVA_AXICC_CFG	0x3fffffff

/* for ls1021a */
#define LS1021_AHCI_VEND_AXICC	0xC0
#define LS1021_CEVA_PHY2_CFG	0x28183414
#define LS1021_CEVA_PHY3_CFG	0x0e080e06
#define LS1021_CEVA_PHY4_CFG	0x064a080b
#define LS1021_CEVA_PHY5_CFG	0x2aa86470

/* ecc val pair */
#define ECC_DIS_VAL_CH1		0x00020000
#define ECC_DIS_VAL_CH2		0x80000000
#define ECC_DIS_VAL_CH3		0x40000000

enum ceva_soc {
	CEVA_1V84,
	CEVA_LS1012A,
	CEVA_LS1021A,
	CEVA_LS1028A,
	CEVA_LS1043A,
	CEVA_LS1046A,
	CEVA_LS1088A,
	CEVA_LS2080A,
};

struct ceva_sata_priv {
	ulong base;
	ulong ecc_base;
	enum ceva_soc soc;
	ulong flag;
};

static int ceva_init_sata(struct ceva_sata_priv *priv)
{
	ulong ecc_addr = priv->ecc_base;
	ulong base = priv->base;
	ulong tmp;

	switch (priv->soc) {
	case CEVA_1V84:
		tmp = PAXIC_ADBW_BW64 | PAXIC_MAWIDD | PAXIC_MARIDD | PAXIC_OTL;
		writel(tmp, base + AHCI_VEND_PAXIC);
		tmp = PCFG_TPSS_VAL | PCFG_TPRS_VAL | PCFG_PAD_VAL;
		writel(tmp, base + AHCI_VEND_PCFG);
		tmp = PPCFG_TTA | PPCFG_PSS_EN | PPCFG_ESDF_EN;
		writel(tmp, base + AHCI_VEND_PPCFG);
		tmp = PTC_RX_WM_VAL | PTC_RSVD;
		writel(tmp, base + AHCI_VEND_PTC);
		break;

	case CEVA_LS1021A:
		if (!ecc_addr)
			return -EINVAL;
		writel(ECC_DIS_VAL_CH1, ecc_addr);
		writel(CEVA_PHY1_CFG, base + AHCI_VEND_PPCFG);
		writel(LS1021_CEVA_PHY2_CFG, base + AHCI_VEND_PP2C);
		writel(LS1021_CEVA_PHY3_CFG, base + AHCI_VEND_PP3C);
		writel(LS1021_CEVA_PHY4_CFG, base + AHCI_VEND_PP4C);
		writel(LS1021_CEVA_PHY5_CFG, base + AHCI_VEND_PP5C);
		writel(CEVA_TRANS_CFG, base + AHCI_VEND_PTC);
		break;

	case CEVA_LS1012A:
	case CEVA_LS1043A:
	case CEVA_LS1046A:
		if (!ecc_addr)
			return -EINVAL;
		writel(ECC_DIS_VAL_CH2, ecc_addr);
		/* fallthrough */
	case CEVA_LS2080A:
		writel(CEVA_PHY1_CFG, base + AHCI_VEND_PPCFG);
		writel(CEVA_TRANS_CFG, base + AHCI_VEND_PTC);
		break;

	case CEVA_LS1028A:
	case CEVA_LS1088A:
		if (!ecc_addr)
			return -EINVAL;
		writel(ECC_DIS_VAL_CH3, ecc_addr);
		writel(CEVA_PHY1_CFG, base + AHCI_VEND_PPCFG);
		writel(CEVA_TRANS_CFG, base + AHCI_VEND_PTC);
		break;
	}

	if (priv->flag & FLAG_COHERENT)
		writel(CEVA_AXICC_CFG, base + AHCI_VEND_AXICC);

	return 0;
}

static int sata_ceva_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ahci_bind_scsi(dev, &scsi_dev);
}

static int sata_ceva_probe(struct udevice *dev)
{
	struct ceva_sata_priv *priv = dev_get_priv(dev);

	ceva_init_sata(priv);

	return ahci_probe_scsi(dev, priv->base);
}

static const struct udevice_id sata_ceva_ids[] = {
	{ .compatible = "ceva,ahci-1v84", .data = CEVA_1V84 },
	{ .compatible = "fsl,ls1012a-ahci", .data = CEVA_LS1012A },
	{ .compatible = "fsl,ls1021a-ahci", .data = CEVA_LS1021A },
	{ .compatible = "fsl,ls1028a-ahci", .data = CEVA_LS1028A },
	{ .compatible = "fsl,ls1043a-ahci", .data = CEVA_LS1043A },
	{ .compatible = "fsl,ls1046a-ahci", .data = CEVA_LS1046A },
	{ .compatible = "fsl,ls1088a-ahci", .data = CEVA_LS1088A },
	{ .compatible = "fsl,ls2080a-ahci", .data = CEVA_LS2080A },
	{ }
};

static int sata_ceva_ofdata_to_platdata(struct udevice *dev)
{
	struct ceva_sata_priv *priv = dev_get_priv(dev);
	struct resource res_regs;
	int ret;

	if (dev_read_bool(dev, "dma-coherent"))
		priv->flag |= FLAG_COHERENT;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_resource_byname(dev, "ecc-addr", &res_regs);
	if (ret)
		priv->ecc_base = 0;
	else
		priv->ecc_base = res_regs.start;

	priv->soc = dev_get_driver_data(dev);

	debug("ccsr-sata-base %lx\t ecc-base %lx\n",
	      priv->base,
	      priv->ecc_base);

	return 0;
}

U_BOOT_DRIVER(ceva_host_blk) = {
	.name = "ceva_sata",
	.id = UCLASS_AHCI,
	.of_match = sata_ceva_ids,
	.bind = sata_ceva_bind,
	.ops = &scsi_ops,
	.priv_auto_alloc_size = sizeof(struct ceva_sata_priv),
	.probe = sata_ceva_probe,
	.ofdata_to_platdata = sata_ceva_ofdata_to_platdata,
};
