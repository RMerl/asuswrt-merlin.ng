// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 */

/*
 * Generic driver for Freescale MMDC(Multi Mode DDR Controller).
 */

#include <common.h>
#include <fsl_mmdc.h>
#include <asm/io.h>
#include <linux/delay.h>

static void set_wait_for_bits_clear(void *ptr, u32 value, u32 bits)
{
	int timeout = 1000;

	out_be32(ptr, value);

	while (in_be32(ptr) & bits) {
		udelay(100);
		timeout--;
	}
	if (timeout <= 0)
		printf("Error: %p wait for clear timeout.\n", ptr);
}

void mmdc_init(const struct fsl_mmdc_info *priv)
{
	struct mmdc_regs *mmdc = (struct mmdc_regs *)CFG_SYS_FSL_DDR_ADDR;
	unsigned int tmp;

	/* 1. set configuration request */
	out_be32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	/* 2. configure the desired timing parameters */
	out_be32(&mmdc->mdotc, priv->mdotc);
	out_be32(&mmdc->mdcfg0, priv->mdcfg0);
	out_be32(&mmdc->mdcfg1, priv->mdcfg1);
	out_be32(&mmdc->mdcfg2, priv->mdcfg2);

	/* 3. configure DDR type and other miscellaneous parameters */
	out_be32(&mmdc->mdmisc, priv->mdmisc);
	out_be32(&mmdc->mpmur0,	MMDC_MPMUR0_FRC_MSR);
	out_be32(&mmdc->mdrwd, priv->mdrwd);
	out_be32(&mmdc->mpodtctrl, priv->mpodtctrl);

	/* 4. configure the required delay while leaving reset */
	out_be32(&mmdc->mdor, priv->mdor);

	/* 5. configure DDR physical parameters */
	/* set row/column address width, burst length, data bus width */
	tmp = priv->mdctl & ~(MDCTL_SDE0 | MDCTL_SDE1);
	out_be32(&mmdc->mdctl, tmp);
	/* configure address space partition */
	out_be32(&mmdc->mdasp, priv->mdasp);

	/* 6. perform a ZQ calibration - not needed here, doing in #8b */

	/* 7. enable MMDC with the desired chip select */
#if (CONFIG_CHIP_SELECTS_PER_CTRL == 1)
		out_be32(&mmdc->mdctl, tmp | MDCTL_SDE0);
#elif (CONFIG_CHIP_SELECTS_PER_CTRL == 2)
		out_be32(&mmdc->mdctl, tmp | MDCTL_SDE0 | MDCTL_SDE1);
#endif

	/* 8a. dram init sequence: update MRs for ZQ, ODT, PRE, etc */
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(8) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_2);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(0) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(0x19) |
				CMD_ADDR_LSB_MR_ADDR(0x30) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_0);

	/* 8b. ZQ calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(0x4) | MDSCR_ENABLE_CON_REQ |
				CMD_ZQ_CALIBRATION | CMD_BANK_ADDR_0);

	set_wait_for_bits_clear(&mmdc->mpzqhwctrl, priv->mpzqhwctrl,
				MPZQHWCTRL_ZQ_HW_FORCE);

	/* 9a. calibrations now, wr lvl */
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(0x84) |
				MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | MDSCR_WL_EN |
				CMD_NORMAL);

	set_wait_for_bits_clear(&mmdc->mpwlgcr, MPWLGCR_HW_WL_EN,
				MPWLGCR_HW_WL_EN);

	mdelay(1);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_1);
	out_be32(&mmdc->mdscr, MDSCR_ENABLE_CON_REQ);

	mdelay(1);

	/* 9b. read DQS gating calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);

	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);

	out_be32(&mmdc->mppdcmpr2, MPPDCMPR2_MPR_COMPARE_EN);

	/* set absolute read delay offset */
	if (priv->mprddlctl)
		out_be32(&mmdc->mprddlctl, priv->mprddlctl);
	else
		out_be32(&mmdc->mprddlctl, MMDC_MPRDDLCTL_DEFAULT_DELAY);

	set_wait_for_bits_clear(&mmdc->mpdgctrl0,
				AUTO_RD_DQS_GATING_CALIBRATION_EN,
				AUTO_RD_DQS_GATING_CALIBRATION_EN);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 9c. read calibration */
	out_be32(&mmdc->mdscr,  CMD_ADDR_MSB_MR_OP(4) | MDSCR_ENABLE_CON_REQ |
				CMD_PRECHARGE_BANK_OPEN | CMD_BANK_ADDR_0);
	out_be32(&mmdc->mdscr,  CMD_ADDR_LSB_MR_ADDR(4) | MDSCR_ENABLE_CON_REQ |
				CMD_LOAD_MODE_REG | CMD_BANK_ADDR_3);
	out_be32(&mmdc->mppdcmpr2,  MPPDCMPR2_MPR_COMPARE_EN);
	set_wait_for_bits_clear(&mmdc->mprddlhwctl,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN,
				MPRDDLHWCTL_AUTO_RD_CALIBRATION_EN);

	out_be32(&mmdc->mdscr,  MDSCR_ENABLE_CON_REQ | CMD_LOAD_MODE_REG |
				CMD_BANK_ADDR_3);

	/* 10. configure power-down, self-refresh entry, exit parameters */
	out_be32(&mmdc->mdpdc, priv->mdpdc);
	out_be32(&mmdc->mapsr, MMDC_MAPSR_PWR_SAV_CTRL_STAT);

	/* 11. ZQ config again? do nothing here */

	/* 12. refresh scheme */
	set_wait_for_bits_clear(&mmdc->mdref, priv->mdref,
				MDREF_START_REFRESH);

	/* 13. disable CON_REQ */
	out_be32(&mmdc->mdscr, MDSCR_DISABLE_CFG_REQ);
}
