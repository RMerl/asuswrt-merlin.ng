// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 */
#include <common.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <serial.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <env.h>
#include <twl6030.h>

#include "panda_mux_data.h"

#define PANDA_ULPI_PHY_TYPE_GPIO       182
#define PANDA_BOARD_ID_1_GPIO          101
#define PANDA_ES_BOARD_ID_1_GPIO        48
#define PANDA_BOARD_ID_2_GPIO          171
#define PANDA_ES_BOARD_ID_3_GPIO         3
#define PANDA_ES_BOARD_ID_4_GPIO         2

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	"Board: OMAP4 Panda\n"
};

struct omap4_scrm_regs *const scrm = (struct omap4_scrm_regs *)0x4a30a000;

/**
 * @brief board_init
 *
 * Return: 0
 */
int board_init(void)
{
	gpmc_init();

	gd->bd->bi_arch_number = MACH_TYPE_OMAP4_PANDA;
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	return 0;
}

#if defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

	return 0;
}
#endif /* CONFIG_SPL_OS_BOOT */

int board_eth_init(struct bd_info *bis)
{
	return 0;
}

/*
* Routine: get_board_revision
* Description: Detect if we are running on a panda revision A1-A6,
*              or an ES panda board. This can be done by reading
*              the level of GPIOs and checking the processor revisions.
*              This should result in:
*			Panda 4430:
*              GPIO171, GPIO101, GPIO182: 0 1 1 => A1-A5
*              GPIO171, GPIO101, GPIO182: 1 0 1 => A6
*			Panda ES:
*              GPIO2, GPIO3, GPIO171, GPIO48, GPIO182: 0 0 0 1 1 => B1/B2
*              GPIO2, GPIO3, GPIO171, GPIO48, GPIO182: 0 0 1 1 1 => B3
*/
int get_board_revision(void)
{
	int board_id0, board_id1, board_id2;
	int board_id3, board_id4;
	int board_id;

	int processor_rev = omap_revision();

	/* Setup the mux for the common board ID pins (gpio 171 and 182) */
	writew((IEN | M3), (*ctrl)->control_padconf_core_base + UNIPRO_TX0);
	writew((IEN | M3), (*ctrl)->control_padconf_core_base + FREF_CLK2_OUT);

	board_id0 = gpio_get_value(PANDA_ULPI_PHY_TYPE_GPIO);
	board_id2 = gpio_get_value(PANDA_BOARD_ID_2_GPIO);

	if ((processor_rev >= OMAP4460_ES1_0 &&
	     processor_rev <= OMAP4460_ES1_1)) {
		/*
		 * Setup the mux for the ES specific board ID pins (gpio 101,
		 * 2 and 3.
		 */
		writew((IEN | M3), (*ctrl)->control_padconf_core_base +
				GPMC_A24);
		writew((IEN | M3), (*ctrl)->control_padconf_core_base +
				UNIPRO_RY0);
		writew((IEN | M3), (*ctrl)->control_padconf_core_base +
				UNIPRO_RX1);

		board_id1 = gpio_get_value(PANDA_ES_BOARD_ID_1_GPIO);
		board_id3 = gpio_get_value(PANDA_ES_BOARD_ID_3_GPIO);
		board_id4 = gpio_get_value(PANDA_ES_BOARD_ID_4_GPIO);

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
		env_set("board_name", "panda-es");
#endif
		board_id = ((board_id4 << 4) | (board_id3 << 3) |
			(board_id2 << 2) | (board_id1 << 1) | (board_id0));
	} else {
		/* Setup the mux for the Ax specific board ID pins (gpio 101) */
		writew((IEN | M3), (*ctrl)->control_padconf_core_base +
				FREF_CLK2_OUT);

		board_id1 = gpio_get_value(PANDA_BOARD_ID_1_GPIO);
		board_id = ((board_id2 << 2) | (board_id1 << 1) | (board_id0));

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
		if ((board_id >= 0x3) && (processor_rev == OMAP4430_ES2_3))
			env_set("board_name", "panda-a4");
#endif
	}

	return board_id;
}

/**
 * is_panda_es_rev_b3() - Detect if we are running on rev B3 of panda board ES
 *
 *
 * Detect if we are running on B3 version of ES panda board,
 * This can be done by reading the level of GPIO 171 and checking the
 * processor revisions.
 * GPIO171: 1 => Panda ES Rev B3
 *
 * Return : return 1 if Panda ES Rev B3 , else return 0
 */
u8 is_panda_es_rev_b3(void)
{
        int processor_rev = omap_revision();
        int ret = 0;

        if ((processor_rev >= OMAP4460_ES1_0 &&
             processor_rev <= OMAP4460_ES1_1)) {

                /* Setup the mux for the common board ID pins (gpio 171) */
                writew((IEN | M3),
			(*ctrl)->control_padconf_core_base + UNIPRO_TX0);

                /* if processor_rev is panda ES and GPIO171 is 1,it is rev b3 */
                ret = gpio_get_value(PANDA_BOARD_ID_2_GPIO);
        }
        return ret;
}

#ifdef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
/*
 * emif_get_reg_dump() - emif_get_reg_dump strong function
 *
 * @emif_nr - emif base
 * @regs - reg dump of timing values
 *
 * Strong function to override emif_get_reg_dump weak function in sdram_elpida.c
 */
void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	u32 omap4_rev = omap_revision();

	/* Same devices and geometry on both EMIFs */
	if (omap4_rev == OMAP4430_ES1_0)
		*regs = &emif_regs_elpida_380_mhz_1cs;
	else if (omap4_rev == OMAP4430_ES2_0)
		*regs = &emif_regs_elpida_200_mhz_2cs;
	else if (omap4_rev == OMAP4430_ES2_3)
		*regs = &emif_regs_elpida_400_mhz_1cs;
	else if (omap4_rev < OMAP4470_ES1_0) {
		if(is_panda_es_rev_b3())
			*regs = &emif_regs_elpida_400_mhz_1cs;
		else
			*regs = &emif_regs_elpida_400_mhz_2cs;
	}
	else
		*regs = &emif_regs_elpida_400_mhz_1cs;
}

void emif_get_dmm_regs(const struct dmm_lisa_map_regs
						**dmm_lisa_regs)
{
	u32 omap_rev = omap_revision();

	if (omap_rev == OMAP4430_ES1_0)
		*dmm_lisa_regs = &lisa_map_2G_x_1_x_2;
	else if (omap_rev == OMAP4430_ES2_3)
		*dmm_lisa_regs = &lisa_map_2G_x_2_x_2;
	else if (omap_rev < OMAP4460_ES1_0)
		*dmm_lisa_regs = &lisa_map_2G_x_2_x_2;
	else
		*dmm_lisa_regs = &ma_lisa_map_2G_x_2_x_2;
}

#endif

/**
 * @brief misc_init_r - Configure Panda board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * Return: 0
 */
int misc_init_r(void)
{
	int phy_type;
	u32 auxclk, altclksrc;

	/* EHCI is not supported on ES1.0 */
	if (omap_revision() == OMAP4430_ES1_0)
		return 0;

	get_board_revision();

	gpio_direction_input(PANDA_ULPI_PHY_TYPE_GPIO);
	phy_type = gpio_get_value(PANDA_ULPI_PHY_TYPE_GPIO);

	if (phy_type == 1) {
		/* ULPI PHY supplied by auxclk3 derived from sys_clk */
		debug("ULPI PHY supplied by auxclk3\n");

		auxclk = readl(&scrm->auxclk3);
		/* Select sys_clk */
		auxclk &= ~AUXCLK_SRCSELECT_MASK;
		auxclk |=  AUXCLK_SRCSELECT_SYS_CLK << AUXCLK_SRCSELECT_SHIFT;
		/* Set the divisor to 2 */
		auxclk &= ~AUXCLK_CLKDIV_MASK;
		auxclk |= AUXCLK_CLKDIV_2 << AUXCLK_CLKDIV_SHIFT;
		/* Request auxilary clock #3 */
		auxclk |= AUXCLK_ENABLE_MASK;

		writel(auxclk, &scrm->auxclk3);
	} else {
		/* ULPI PHY supplied by auxclk1 derived from PER dpll */
		debug("ULPI PHY supplied by auxclk1\n");

		auxclk = readl(&scrm->auxclk1);
		/* Select per DPLL */
		auxclk &= ~AUXCLK_SRCSELECT_MASK;
		auxclk |=  AUXCLK_SRCSELECT_PER_DPLL << AUXCLK_SRCSELECT_SHIFT;
		/* Set the divisor to 16 */
		auxclk &= ~AUXCLK_CLKDIV_MASK;
		auxclk |= AUXCLK_CLKDIV_16 << AUXCLK_CLKDIV_SHIFT;
		/* Request auxilary clock #3 */
		auxclk |= AUXCLK_ENABLE_MASK;

		writel(auxclk, &scrm->auxclk1);
	}

	altclksrc = readl(&scrm->altclksrc);

	/* Activate alternate system clock supplier */
	altclksrc &= ~ALTCLKSRC_MODE_MASK;
	altclksrc |= ALTCLKSRC_MODE_ACTIVE;

	/* enable clocks */
	altclksrc |= ALTCLKSRC_ENABLE_INT_MASK | ALTCLKSRC_ENABLE_EXT_MASK;

	writel(altclksrc, &scrm->altclksrc);

	omap_die_id_usbethaddr();

	return 0;
}

void set_muxconf_regs(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base,
		   core_padconf_array_essential,
		   sizeof(core_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	do_set_mux((*ctrl)->control_padconf_wkup_base,
		   wkup_padconf_array_essential,
		   sizeof(wkup_padconf_array_essential) /
		   sizeof(struct pad_conf_entry));

	if (omap_revision() >= OMAP4460_ES1_0)
		do_set_mux((*ctrl)->control_padconf_wkup_base,
			   wkup_padconf_array_essential_4460,
			   sizeof(wkup_padconf_array_essential_4460) /
			   sizeof(struct pad_conf_entry));
}

#if defined(CONFIG_MMC)
int board_mmc_init(struct bd_info *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}

#if !defined(CONFIG_SPL_BUILD)
void board_mmc_power_init(void)
{
	twl6030_power_mmc_init(0);
}
#endif
#endif

#ifdef CONFIG_REVISION_TAG
/*
 * get_board_rev() - get board revision
 */
u32 get_board_rev(void)
{
	return 0x20;
}
#endif
