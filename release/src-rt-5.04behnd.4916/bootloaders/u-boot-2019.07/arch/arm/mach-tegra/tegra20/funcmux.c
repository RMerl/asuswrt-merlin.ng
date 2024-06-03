// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

/* Tegra20 high-level function multiplexing */
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

/*
 * The PINMUX macro is used to set up pinmux tables.
 */
#define PINMUX(grp, mux, pupd, tri)                   \
	{PMUX_PINGRP_##grp, PMUX_FUNC_##mux, PMUX_PULL_##pupd, PMUX_TRI_##tri}

static const struct pmux_pingrp_config disp1_default[] = {
	PINMUX(LDI,   DISPA,      NORMAL,    NORMAL),
	PINMUX(LHP0,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LHP1,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LHP2,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LHS,   DISPA,      NORMAL,    NORMAL),
	PINMUX(LM0,   RSVD4,      NORMAL,    NORMAL),
	PINMUX(LPP,   DISPA,      NORMAL,    NORMAL),
	PINMUX(LPW0,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LPW2,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LSC0,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LSPI,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LVP1,  DISPA,      NORMAL,    NORMAL),
	PINMUX(LVS,   DISPA,      NORMAL,    NORMAL),
	PINMUX(SLXD,  SPDIF,      NORMAL,    NORMAL),
};


int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART1:
		switch (config) {
		case FUNCMUX_UART1_IRRX_IRTX:
			pinmux_set_func(PMUX_PINGRP_IRRX, PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_IRTX, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PMUX_PINGRP_IRRX);
			pinmux_tristate_disable(PMUX_PINGRP_IRTX);
			break;
		case FUNCMUX_UART1_UAA_UAB:
			pinmux_set_func(PMUX_PINGRP_UAA, PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_UAB, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PMUX_PINGRP_UAA);
			pinmux_tristate_disable(PMUX_PINGRP_UAB);
			bad_config = 0;
			break;
		case FUNCMUX_UART1_GPU:
			pinmux_set_func(PMUX_PINGRP_GPU, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PMUX_PINGRP_GPU);
			bad_config = 0;
			break;
		case FUNCMUX_UART1_SDIO1:
			pinmux_set_func(PMUX_PINGRP_SDIO1, PMUX_FUNC_UARTA);
			pinmux_tristate_disable(PMUX_PINGRP_SDIO1);
			bad_config = 0;
			break;
		}
		if (!bad_config) {
			/*
			 * Tegra appears to boot with function UARTA pre-
			 * selected on mux group SDB. If two mux groups are
			 * both set to the same function, it's unclear which
			 * group's pins drive the RX signals into the HW.
			 * For UARTA, SDB certainly overrides group IRTX in
			 * practice. To solve this, configure some alternative
			 * function on SDB to avoid the conflict. Also, tri-
			 * state the group to avoid driving any signal onto it
			 * until we know what's connected.
			 */
			pinmux_tristate_enable(PMUX_PINGRP_SDB);
			pinmux_set_func(PMUX_PINGRP_SDB,  PMUX_FUNC_SDIO3);
		}
		break;

	case PERIPH_ID_UART2:
		if (config == FUNCMUX_UART2_UAD) {
			pinmux_set_func(PMUX_PINGRP_UAD, PMUX_FUNC_UARTB);
			pinmux_tristate_disable(PMUX_PINGRP_UAD);
		}
		break;

	case PERIPH_ID_UART4:
		if (config == FUNCMUX_UART4_GMC) {
			pinmux_set_func(PMUX_PINGRP_GMC, PMUX_FUNC_UARTD);
			pinmux_tristate_disable(PMUX_PINGRP_GMC);
		}
		break;

	case PERIPH_ID_DVC_I2C:
		/* there is only one selection, pinmux_config is ignored */
		if (config == FUNCMUX_DVC_I2CP) {
			pinmux_set_func(PMUX_PINGRP_I2CP, PMUX_FUNC_I2C);
			pinmux_tristate_disable(PMUX_PINGRP_I2CP);
		}
		break;

	case PERIPH_ID_I2C1:
		/* support pinmux_config of 0 for now, */
		if (config == FUNCMUX_I2C1_RM) {
			pinmux_set_func(PMUX_PINGRP_RM, PMUX_FUNC_I2C);
			pinmux_tristate_disable(PMUX_PINGRP_RM);
		}
		break;
	case PERIPH_ID_I2C2: /* I2C2 */
		switch (config) {
		case FUNCMUX_I2C2_DDC:	/* DDC pin group, select I2C2 */
			pinmux_set_func(PMUX_PINGRP_DDC, PMUX_FUNC_I2C2);
			/* PTA to HDMI */
			pinmux_set_func(PMUX_PINGRP_PTA, PMUX_FUNC_HDMI);
			pinmux_tristate_disable(PMUX_PINGRP_DDC);
			break;
		case FUNCMUX_I2C2_PTA:	/* PTA pin group, select I2C2 */
			pinmux_set_func(PMUX_PINGRP_PTA, PMUX_FUNC_I2C2);
			/* set DDC_SEL to RSVDx (RSVD2 works for now) */
			pinmux_set_func(PMUX_PINGRP_DDC, PMUX_FUNC_RSVD2);
			pinmux_tristate_disable(PMUX_PINGRP_PTA);
			bad_config = 0;
			break;
		}
		break;
	case PERIPH_ID_I2C3: /* I2C3 */
		/* support pinmux_config of 0 for now */
		if (config == FUNCMUX_I2C3_DTF) {
			pinmux_set_func(PMUX_PINGRP_DTF, PMUX_FUNC_I2C3);
			pinmux_tristate_disable(PMUX_PINGRP_DTF);
		}
		break;

	case PERIPH_ID_SDMMC1:
		if (config == FUNCMUX_SDMMC1_SDIO1_4BIT) {
			pinmux_set_func(PMUX_PINGRP_SDIO1, PMUX_FUNC_SDIO1);
			pinmux_tristate_disable(PMUX_PINGRP_SDIO1);
		}
		break;

	case PERIPH_ID_SDMMC2:
		if (config == FUNCMUX_SDMMC2_DTA_DTD_8BIT) {
			pinmux_set_func(PMUX_PINGRP_DTA, PMUX_FUNC_SDIO2);
			pinmux_set_func(PMUX_PINGRP_DTD, PMUX_FUNC_SDIO2);

			pinmux_tristate_disable(PMUX_PINGRP_DTA);
			pinmux_tristate_disable(PMUX_PINGRP_DTD);
		}
		break;

	case PERIPH_ID_SDMMC3:
		switch (config) {
		case FUNCMUX_SDMMC3_SDB_SLXA_8BIT:
			pinmux_set_func(PMUX_PINGRP_SLXA, PMUX_FUNC_SDIO3);
			pinmux_set_func(PMUX_PINGRP_SLXC, PMUX_FUNC_SDIO3);
			pinmux_set_func(PMUX_PINGRP_SLXD, PMUX_FUNC_SDIO3);
			pinmux_set_func(PMUX_PINGRP_SLXK, PMUX_FUNC_SDIO3);

			pinmux_tristate_disable(PMUX_PINGRP_SLXA);
			pinmux_tristate_disable(PMUX_PINGRP_SLXC);
			pinmux_tristate_disable(PMUX_PINGRP_SLXD);
			pinmux_tristate_disable(PMUX_PINGRP_SLXK);
			/* fall through */

		case FUNCMUX_SDMMC3_SDB_4BIT:
			pinmux_set_func(PMUX_PINGRP_SDB, PMUX_FUNC_SDIO3);
			pinmux_set_func(PMUX_PINGRP_SDC, PMUX_FUNC_SDIO3);
			pinmux_set_func(PMUX_PINGRP_SDD, PMUX_FUNC_SDIO3);

			pinmux_tristate_disable(PMUX_PINGRP_SDB);
			pinmux_tristate_disable(PMUX_PINGRP_SDC);
			pinmux_tristate_disable(PMUX_PINGRP_SDD);
			bad_config = 0;
			break;
		}
		break;

	case PERIPH_ID_SDMMC4:
		switch (config) {
		case FUNCMUX_SDMMC4_ATC_ATD_8BIT:
			pinmux_set_func(PMUX_PINGRP_ATC, PMUX_FUNC_SDIO4);
			pinmux_set_func(PMUX_PINGRP_ATD, PMUX_FUNC_SDIO4);

			pinmux_tristate_disable(PMUX_PINGRP_ATC);
			pinmux_tristate_disable(PMUX_PINGRP_ATD);
			break;

		case FUNCMUX_SDMMC4_ATB_GMA_GME_8_BIT:
			pinmux_set_func(PMUX_PINGRP_GME, PMUX_FUNC_SDIO4);
			pinmux_tristate_disable(PMUX_PINGRP_GME);
			/* fall through */

		case FUNCMUX_SDMMC4_ATB_GMA_4_BIT:
			pinmux_set_func(PMUX_PINGRP_ATB, PMUX_FUNC_SDIO4);
			pinmux_set_func(PMUX_PINGRP_GMA, PMUX_FUNC_SDIO4);

			pinmux_tristate_disable(PMUX_PINGRP_ATB);
			pinmux_tristate_disable(PMUX_PINGRP_GMA);
			bad_config = 0;
			break;
		}
		break;

	case PERIPH_ID_KBC:
		if (config == FUNCMUX_DEFAULT) {
			enum pmux_pingrp grp[] = {PMUX_PINGRP_KBCA,
				PMUX_PINGRP_KBCB, PMUX_PINGRP_KBCC,
				PMUX_PINGRP_KBCD, PMUX_PINGRP_KBCE,
				PMUX_PINGRP_KBCF};
			int i;

			for (i = 0; i < ARRAY_SIZE(grp); i++) {
				pinmux_tristate_disable(grp[i]);
				pinmux_set_func(grp[i], PMUX_FUNC_KBC);
				pinmux_set_pullupdown(grp[i], PMUX_PULL_UP);
			}
		}
		break;

	case PERIPH_ID_USB2:
		if (config == FUNCMUX_USB2_ULPI) {
			pinmux_set_func(PMUX_PINGRP_UAA, PMUX_FUNC_ULPI);
			pinmux_set_func(PMUX_PINGRP_UAB, PMUX_FUNC_ULPI);
			pinmux_set_func(PMUX_PINGRP_UDA, PMUX_FUNC_ULPI);

			pinmux_tristate_disable(PMUX_PINGRP_UAA);
			pinmux_tristate_disable(PMUX_PINGRP_UAB);
			pinmux_tristate_disable(PMUX_PINGRP_UDA);
		}
		break;

	case PERIPH_ID_SPI1:
		if (config == FUNCMUX_SPI1_GMC_GMD) {
			pinmux_set_func(PMUX_PINGRP_GMC, PMUX_FUNC_SFLASH);
			pinmux_set_func(PMUX_PINGRP_GMD, PMUX_FUNC_SFLASH);

			pinmux_tristate_disable(PMUX_PINGRP_GMC);
			pinmux_tristate_disable(PMUX_PINGRP_GMD);
		}
		break;

	case PERIPH_ID_NDFLASH:
		switch (config) {
		case FUNCMUX_NDFLASH_ATC:
			pinmux_set_func(PMUX_PINGRP_ATC, PMUX_FUNC_NAND);
			pinmux_tristate_disable(PMUX_PINGRP_ATC);
			break;
		case FUNCMUX_NDFLASH_KBC_8_BIT:
			pinmux_set_func(PMUX_PINGRP_KBCA, PMUX_FUNC_NAND);
			pinmux_set_func(PMUX_PINGRP_KBCB, PMUX_FUNC_NAND);
			pinmux_set_func(PMUX_PINGRP_KBCC, PMUX_FUNC_NAND);
			pinmux_set_func(PMUX_PINGRP_KBCD, PMUX_FUNC_NAND);
			pinmux_set_func(PMUX_PINGRP_KBCE, PMUX_FUNC_NAND);
			pinmux_set_func(PMUX_PINGRP_KBCF, PMUX_FUNC_NAND);

			pinmux_tristate_disable(PMUX_PINGRP_KBCA);
			pinmux_tristate_disable(PMUX_PINGRP_KBCB);
			pinmux_tristate_disable(PMUX_PINGRP_KBCC);
			pinmux_tristate_disable(PMUX_PINGRP_KBCD);
			pinmux_tristate_disable(PMUX_PINGRP_KBCE);
			pinmux_tristate_disable(PMUX_PINGRP_KBCF);

			bad_config = 0;
			break;
		}
		break;
	case PERIPH_ID_DISP1:
		if (config == FUNCMUX_DEFAULT) {
			int i;

			for (i = PMUX_PINGRP_LD0; i <= PMUX_PINGRP_LD17; i++) {
				pinmux_set_func(i, PMUX_FUNC_DISPA);
				pinmux_tristate_disable(i);
				pinmux_set_pullupdown(i, PMUX_PULL_NORMAL);
			}
			pinmux_config_pingrp_table(disp1_default,
						   ARRAY_SIZE(disp1_default));
		}
		break;

	default:
		debug("%s: invalid periph_id %d", __func__, id);
		return -1;
	}

	if (bad_config) {
		debug("%s: invalid config %d for periph_id %d", __func__,
		      config, id);
		return -1;
	}

	return 0;
}
