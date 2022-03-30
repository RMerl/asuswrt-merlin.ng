// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/ls102xa_soc.h>
#include <asm/arch/ls102xa_stream_id.h>
#include <fsl_csu.h>
#include <fsl_ddr_sdram.h>

struct liodn_id_table sec_liodn_tbl[] = {
	SET_SEC_JR_LIODN_ENTRY(0, 0x10, 0x10),
	SET_SEC_JR_LIODN_ENTRY(1, 0x10, 0x10),
	SET_SEC_JR_LIODN_ENTRY(2, 0x10, 0x10),
	SET_SEC_JR_LIODN_ENTRY(3, 0x10, 0x10),
	SET_SEC_RTIC_LIODN_ENTRY(a, 0x10),
	SET_SEC_RTIC_LIODN_ENTRY(b, 0x10),
	SET_SEC_RTIC_LIODN_ENTRY(c, 0x10),
	SET_SEC_RTIC_LIODN_ENTRY(d, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(0, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(1, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(2, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(3, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(4, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(5, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(6, 0x10, 0x10),
	SET_SEC_DECO_LIODN_ENTRY(7, 0x10, 0x10),
};

struct smmu_stream_id dev_stream_id[] = {
	{ 0x100, 0x01, "ETSEC MAC1" },
	{ 0x104, 0x02, "ETSEC MAC2" },
	{ 0x108, 0x03, "ETSEC MAC3" },
	{ 0x10c, 0x04, "PEX1" },
	{ 0x110, 0x05, "PEX2" },
	{ 0x114, 0x06, "qDMA" },
	{ 0x118, 0x07, "SATA" },
	{ 0x11c, 0x08, "USB3" },
	{ 0x120, 0x09, "QE" },
	{ 0x124, 0x0a, "eSDHC" },
	{ 0x128, 0x0b, "eMA" },
	{ 0x14c, 0x0c, "2D-ACE" },
	{ 0x150, 0x0d, "USB2" },
	{ 0x18c, 0x0e, "DEBUG" },
};

unsigned int get_soc_major_rev(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int svr, major;

	svr = in_be32(&gur->svr);
	major = SVR_MAJ(svr);

	return major;
}

static void erratum_a009008(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009008
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	clrsetbits_be32(scfg + SCFG_USB3PRM1CR / 4,
			0xF << 6,
			SCFG_USB_TXVREFTUNE << 6);
#endif /* CONFIG_SYS_FSL_ERRATUM_A009008 */
}

static void erratum_a009798(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009798
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	clrbits_be32(scfg + SCFG_USB3PRM1CR / 4,
			SCFG_USB_SQRXTUNE_MASK << 23);
#endif /* CONFIG_SYS_FSL_ERRATUM_A009798 */
}

static void erratum_a008997(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008997
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;

	clrsetbits_be32(scfg + SCFG_USB3PRM2CR / 4,
			SCFG_USB_PCSTXSWINGFULL_MASK,
			SCFG_USB_PCSTXSWINGFULL_VAL);
#endif /* CONFIG_SYS_FSL_ERRATUM_A008997 */
}

static void erratum_a009007(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A009007
	void __iomem *usb_phy = (void __iomem *)USB_PHY_BASE;

	out_le16(usb_phy + USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_1);
	out_le16(usb_phy + USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_2);
	out_le16(usb_phy + USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_3);
	out_le16(usb_phy + USB_PHY_RX_OVRD_IN_HI, USB_PHY_RX_EQ_VAL_4);
#endif /* CONFIG_SYS_FSL_ERRATUM_A009007 */
}

static void erratum_a008850_early(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008850
	/* part 1 of 2 */
	struct ccsr_cci400 __iomem *cci = (void *)(CONFIG_SYS_IMMR +
						CONFIG_SYS_CCI400_OFFSET);
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;

	/* disables propagation of barrier transactions to DDRC from CCI400 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_TERM_BARRIER);

	/* disable the re-ordering in DDRC */
	out_be32(&ddr->eor, DDR_EOR_RD_REOD_DIS | DDR_EOR_WD_REOD_DIS);
#endif
}

void erratum_a008850_post(void)
{
#ifdef CONFIG_SYS_FSL_ERRATUM_A008850
	/* part 2 of 2 */
	struct ccsr_cci400 __iomem *cci = (void *)(CONFIG_SYS_IMMR +
						CONFIG_SYS_CCI400_OFFSET);
	struct ccsr_ddr __iomem *ddr = (void *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 tmp;

	/* enable propagation of barrier transactions to DDRC from CCI400 */
	out_le32(&cci->ctrl_ord, CCI400_CTRLORD_EN_BARRIER);

	/* enable the re-ordering in DDRC */
	tmp = in_be32(&ddr->eor);
	tmp &= ~(DDR_EOR_RD_REOD_DIS | DDR_EOR_WD_REOD_DIS);
	out_be32(&ddr->eor, tmp);
#endif
}

void s_init(void)
{
}

#ifdef CONFIG_SYS_FSL_ERRATUM_A010315
void erratum_a010315(void)
{
	int i;

	for (i = PCIE1; i <= PCIE2; i++)
		if (!is_serdes_configured(i)) {
			debug("PCIe%d: disabled all R/W permission!\n", i);
			set_pcie_ns_access(i, 0);
		}
}
#endif

int arch_soc_init(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)(CONFIG_SYS_IMMR +
					CONFIG_SYS_CCI400_OFFSET);
	unsigned int major;

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_FSL_QSPI
	out_be32(&scfg->qspi_cfg, SCFG_QSPI_CLKSEL);
#endif

#ifdef CONFIG_VIDEO_FSL_DCU_FB
	out_be32(&scfg->pixclkcr, SCFG_PIXCLKCR_PXCKEN);
#endif

	/* Configure Little endian for SAI, ASRC and SPDIF */
	out_be32(&scfg->endiancr, SCFG_ENDIANCR_LE);

	/*
	 * Enable snoop requests and DVM message requests for
	 * All the slave insterfaces.
	 */
	out_le32(&cci->slave[0].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
	out_le32(&cci->slave[1].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
	out_le32(&cci->slave[2].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
	out_le32(&cci->slave[4].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);

	major = get_soc_major_rev();
	if (major == SOC_MAJOR_VER_1_0) {
		/*
		 * Set CCI-400 Slave interface S1, S2 Shareable Override
		 * Register All transactions are treated as non-shareable
		 */
		out_le32(&cci->slave[1].sha_ord, CCI400_SHAORD_NON_SHAREABLE);
		out_le32(&cci->slave[2].sha_ord, CCI400_SHAORD_NON_SHAREABLE);
	}

	/* Enable all the snoop signal for various masters */
	out_be32(&scfg->snpcnfgcr, SCFG_SNPCNFGCR_SEC_RD_WR |
				SCFG_SNPCNFGCR_DCU_RD_WR |
				SCFG_SNPCNFGCR_SATA_RD_WR |
				SCFG_SNPCNFGCR_USB3_RD_WR |
				SCFG_SNPCNFGCR_DBG_RD_WR |
				SCFG_SNPCNFGCR_EDMA_SNP);

	/*
	 * Memory controller require a register write before being enabled.
	 * Affects: DDR
	 * Register: EDDRTQCFG
	 * Description: Memory controller performance is not optimal with
	 *		default internal target queue register values.
	 * Workaround: Write a value of 63b2_0042h to address: 157_020Ch.
	 */
	out_be32(&scfg->eddrtqcfg, 0x63b20042);

	/* Erratum */
	erratum_a008850_early();
	erratum_a009008();
	erratum_a009798();
	erratum_a008997();
	erratum_a009007();

	return 0;
}

int ls102xa_smmu_stream_id_init(void)
{
	ls1021x_config_caam_stream_id(sec_liodn_tbl,
				      ARRAY_SIZE(sec_liodn_tbl));

	ls102xa_config_smmu_stream_id(dev_stream_id,
				      ARRAY_SIZE(dev_stream_id));

	return 0;
}
