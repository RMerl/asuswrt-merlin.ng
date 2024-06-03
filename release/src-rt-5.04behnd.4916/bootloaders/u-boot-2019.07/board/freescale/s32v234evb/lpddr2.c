// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015, Freescale Semiconductor, Inc.
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/lpddr2.h>
#include <asm/arch/mmdc.h>

volatile int mscr_offset_ck0;

void lpddr2_config_iomux(uint8_t module)
{
	int i;

	switch (module) {
	case DDR0:
		mscr_offset_ck0 = SIUL2_MSCRn(_DDR0_CKE0);
		writel(LPDDR2_CLK0_PAD, SIUL2_MSCRn(_DDR0_CLK0));

		writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR0_CKE0));
		writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR0_CKE1));

		writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR0_CS_B0));
		writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR0_CS_B1));

		for (i = _DDR0_DM0; i <= _DDR0_DM3; i++)
			writel(LPDDR2_DMn_PAD, SIUL2_MSCRn(i));

		for (i = _DDR0_DQS0; i <= _DDR0_DQS3; i++)
			writel(LPDDR2_DQSn_PAD, SIUL2_MSCRn(i));

		for (i = _DDR0_A0; i <= _DDR0_A9; i++)
			writel(LPDDR2_An_PAD, SIUL2_MSCRn(i));

		for (i = _DDR0_D0; i <= _DDR0_D31; i++)
			writel(LPDDR2_Dn_PAD, SIUL2_MSCRn(i));
		break;
	case DDR1:
		writel(LPDDR2_CLK0_PAD, SIUL2_MSCRn(_DDR1_CLK0));

		writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR1_CKE0));
		writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR1_CKE1));

		writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR1_CS_B0));
		writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR1_CS_B1));

		for (i = _DDR1_DM0; i <= _DDR1_DM3; i++)
			writel(LPDDR2_DMn_PAD, SIUL2_MSCRn(i));

		for (i = _DDR1_DQS0; i <= _DDR1_DQS3; i++)
			writel(LPDDR2_DQSn_PAD, SIUL2_MSCRn(i));

		for (i = _DDR1_A0; i <= _DDR1_A9; i++)
			writel(LPDDR2_An_PAD, SIUL2_MSCRn(i));

		for (i = _DDR1_D0; i <= _DDR1_D31; i++)
			writel(LPDDR2_Dn_PAD, SIUL2_MSCRn(i));
		break;
	}
}

void config_mmdc(uint8_t module)
{
	unsigned long mmdc_addr = (module) ? MMDC1_BASE_ADDR : MMDC0_BASE_ADDR;

	writel(MMDC_MDSCR_CFG_VALUE, mmdc_addr + MMDC_MDSCR);

	writel(MMDC_MDCFG0_VALUE, mmdc_addr + MMDC_MDCFG0);
	writel(MMDC_MDCFG1_VALUE, mmdc_addr + MMDC_MDCFG1);
	writel(MMDC_MDCFG2_VALUE, mmdc_addr + MMDC_MDCFG2);
	writel(MMDC_MDCFG3LP_VALUE, mmdc_addr + MMDC_MDCFG3LP);
	writel(MMDC_MDOTC_VALUE, mmdc_addr + MMDC_MDOTC);
	writel(MMDC_MDMISC_VALUE, mmdc_addr + MMDC_MDMISC);
	writel(MMDC_MDOR_VALUE, mmdc_addr + MMDC_MDOR);
	writel(_MDCTL, mmdc_addr + MMDC_MDCTL);

	writel(MMDC_MPMUR0_VALUE, mmdc_addr + MMDC_MPMUR0);

	while (readl(mmdc_addr + MMDC_MPMUR0) & MMDC_MPMUR0_FRC_MSR) {
	}

	writel(MMDC_MDSCR_RST_VALUE, mmdc_addr + MMDC_MDSCR);

	/* Perform ZQ calibration */
	writel(MMDC_MPZQLP2CTL_VALUE, mmdc_addr + MMDC_MPZQLP2CTL);
	writel(MMDC_MPZQHWCTRL_VALUE, mmdc_addr + MMDC_MPZQHWCTRL);
	while (readl(mmdc_addr + MMDC_MPZQHWCTRL) & MMDC_MPZQHWCTRL_ZQ_HW_FOR) {
	}

	/* Enable MMDC with CS0 */
	writel(_MDCTL + 0x80000000, mmdc_addr + MMDC_MDCTL);

	/* Complete the initialization sequence as defined by JEDEC */
	writel(MMDC_MDSCR_MR1_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR2_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR3_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR10_VALUE, mmdc_addr + MMDC_MDSCR);

	/* Set the amount of DRAM */
	/* Set DQS settings based on board type */

	switch (module) {
	case MMDC0:
		writel(MMDC_MDASP_MODULE0_VALUE, mmdc_addr + MMDC_MDASP);
		writel(MMDC_MPRDDLCTL_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPRDDLCTL);
		writel(MMDC_MPWRDLCTL_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPWRDLCTL);
		writel(MMDC_MPDGCTRL0_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL0);
		writel(MMDC_MPDGCTRL1_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL1);
		break;
	case MMDC1:
		writel(MMDC_MDASP_MODULE1_VALUE, mmdc_addr + MMDC_MDASP);
		writel(MMDC_MPRDDLCTL_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPRDDLCTL);
		writel(MMDC_MPWRDLCTL_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPWRDLCTL);
		writel(MMDC_MPDGCTRL0_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL0);
		writel(MMDC_MPDGCTRL1_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL1);
		break;
	}

	writel(MMDC_MDRWD_VALUE, mmdc_addr + MMDC_MDRWD);
	writel(MMDC_MDPDC_VALUE, mmdc_addr + MMDC_MDPDC);
	writel(MMDC_MDREF_VALUE, mmdc_addr + MMDC_MDREF);
	writel(MMDC_MPODTCTRL_VALUE, mmdc_addr + MMDC_MPODTCTRL);
	writel(MMDC_MDSCR_DEASSERT_VALUE, mmdc_addr + MMDC_MDSCR);

}
