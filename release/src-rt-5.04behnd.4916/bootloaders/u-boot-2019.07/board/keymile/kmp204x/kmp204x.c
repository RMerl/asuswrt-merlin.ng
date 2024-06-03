// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2011,2012 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "../common/common.h"
#include "kmp204x.h"

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

int checkboard(void)
{
	printf("Board: Keymile %s\n", CONFIG_KM_BOARD_NAME);

	return 0;
}

/* I2C deblocking uses the algorithm defined in board/keymile/common/common.c
 * 2 dedicated QRIO GPIOs externally pull the SCL and SDA lines
 * For I2C only the low state is activly driven and high state is pulled-up
 * by a resistor. Therefore the deblock GPIOs are used
 *  -> as an active output to drive a low state
 *  -> as an open-drain input to have a pulled-up high state
 */

/* QRIO GPIOs used for deblocking */
#define DEBLOCK_PORT1	GPIO_A
#define DEBLOCK_SCL1	20
#define DEBLOCK_SDA1	21

/* By default deblock GPIOs are floating */
static void i2c_deblock_gpio_cfg(void)
{
	/* set I2C bus 1 deblocking GPIOs input, but 0 value for open drain */
	qrio_gpio_direction_input(DEBLOCK_PORT1, DEBLOCK_SCL1);
	qrio_gpio_direction_input(DEBLOCK_PORT1, DEBLOCK_SDA1);

	qrio_set_gpio(DEBLOCK_PORT1, DEBLOCK_SCL1, 0);
	qrio_set_gpio(DEBLOCK_PORT1, DEBLOCK_SDA1, 0);
}

void set_sda(int state)
{
	qrio_set_opendrain_gpio(DEBLOCK_PORT1, DEBLOCK_SDA1, state);
}

void set_scl(int state)
{
	qrio_set_opendrain_gpio(DEBLOCK_PORT1, DEBLOCK_SCL1, state);
}

int get_sda(void)
{
	return qrio_get_gpio(DEBLOCK_PORT1, DEBLOCK_SDA1);
}

int get_scl(void)
{
	return qrio_get_gpio(DEBLOCK_PORT1, DEBLOCK_SCL1);
}


#define ZL30158_RST	8
#define BFTIC4_RST	0
#define RSTRQSR1_WDT_RR	0x00200000
#define RSTRQSR1_SW_RR	0x00100000

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	bool cpuwd_flag = false;

	/* configure mode for uP reset request */
	qrio_uprstreq(UPREQ_CORE_RST);

	/* board only uses the DDR_MCK0, so disable the DDR_MCK1/2/3 */
	setbits_be32(&gur->ddrclkdr, 0x001f000f);

	/* set reset reason according CPU register */
	if ((gur->rstrqsr1 & (RSTRQSR1_WDT_RR | RSTRQSR1_SW_RR)) ==
	    RSTRQSR1_WDT_RR)
		cpuwd_flag = true;

	qrio_cpuwd_flag(cpuwd_flag);
	/* clear CPU bits by writing 1 */
	setbits_be32(&gur->rstrqsr1, RSTRQSR1_WDT_RR | RSTRQSR1_SW_RR);

	/* set the BFTIC's prstcfg to reset at power-up and unit reset only */
	qrio_prstcfg(BFTIC4_RST, PRSTCFG_POWUP_UNIT_RST);
	/* and enable WD on it */
	qrio_wdmask(BFTIC4_RST, true);

	/* set the ZL30138's prstcfg to reset at power-up only */
	qrio_prstcfg(ZL30158_RST, PRSTCFG_POWUP_RST);
	/* and take it out of reset as soon as possible (needed for Hooper) */
	qrio_prst(ZL30158_RST, false, false);

	return 0;
}

int board_early_init_r(void)
{
	int ret = 0;
	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	set_liodns();
	setup_qbman_portals();

	ret = trigger_fpga_config();
	if (ret)
		printf("error triggering PCIe FPGA config\n");

	/* enable the Unit LED (red) & Boot LED (on) */
	qrio_set_leds();

	/* enable Application Buffer */
	qrio_enable_app_buffer();

	return ret;
}

unsigned long get_board_sys_clk(unsigned long dummy)
{
	return 66666666;
}

#define ETH_FRONT_PHY_RST	15
#define QSFP2_RST		11
#define QSFP1_RST		10
#define ZL30343_RST		9

int misc_init_f(void)
{
	/* configure QRIO pis for i2c deblocking */
	i2c_deblock_gpio_cfg();

	/* configure the front phy's prstcfg and take it out of reset */
	qrio_prstcfg(ETH_FRONT_PHY_RST, PRSTCFG_POWUP_UNIT_CORE_RST);
	qrio_prst(ETH_FRONT_PHY_RST, false, false);

	/* set the ZL30343 prstcfg to reset at power-up only */
	qrio_prstcfg(ZL30343_RST, PRSTCFG_POWUP_RST);
	/* and enable the WD on it */
	qrio_wdmask(ZL30343_RST, true);

	/* set the QSFPs' prstcfg to reset at power-up and unit rst only */
	qrio_prstcfg(QSFP1_RST, PRSTCFG_POWUP_UNIT_RST);
	qrio_prstcfg(QSFP2_RST, PRSTCFG_POWUP_UNIT_RST);

	/* and enable the WD on them */
	qrio_wdmask(QSFP1_RST, true);
	qrio_wdmask(QSFP2_RST, true);

	return 0;
}

#define NUM_SRDS_BANKS	2

int misc_init_r(void)
{
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 expected[NUM_SRDS_BANKS] = {SRDS_PLLCR0_RFCK_SEL_100,
		SRDS_PLLCR0_RFCK_SEL_125};
	unsigned int i;

	/* check SERDES reference clocks */
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 actual = in_be32(&regs->bank[i].pllcr0);
		actual &= SRDS_PLLCR0_RFCK_SEL_MASK;
		if (actual != expected[i]) {
			printf("Warning: SERDES bank %u expects reference \
			       clock %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected[i]),
			       serdes_clock_to_string(actual));
		}
	}

	ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}

#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}
#endif

#if defined(CONFIG_LAST_STAGE_INIT)

int last_stage_init(void)
{
#if defined(CONFIG_KMCOGE4)
	/* on KMCOGE4, the BFTIC4 is on the LBAPP2 */
	struct bfticu_iomap *bftic4 =
		(struct bfticu_iomap *)CONFIG_SYS_LBAPP2_BASE;
	u8 dip_switch = in_8((u8 *)&(bftic4->mswitch)) & BFTICU_DIPSWITCH_MASK;

	if (dip_switch != 0) {
		/* start bootloader */
		puts("DIP:   Enabled\n");
		env_set("actual_bank", "0");
	}
#endif
	set_km_env();

	return 0;
}
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
void fdt_fixup_fman_mac_addresses(void *blob)
{
	int node, i, ret;
	char *tmp, *end;
	unsigned char mac_addr[6];

	/* get the mac addr from env */
	tmp = env_get("ethaddr");
	if (!tmp) {
		printf("ethaddr env variable not defined\n");
		return;
	}
	for (i = 0; i < 6; i++) {
		mac_addr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end+1 : end;
	}

	/* find the correct fdt ethernet path and correct it */
	node = fdt_path_offset(blob, "/soc/fman/ethernet@e8000");
	if (node < 0) {
		printf("no /soc/fman/ethernet path offset\n");
		return;
	}
	ret = fdt_setprop(blob, node, "local-mac-address", &mac_addr, 6);
	if (ret) {
		printf("error setting local-mac-address property\n");
		return;
	}
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
#ifdef CONFIG_SYS_DPAA_FMAN
	fdt_fixup_fman_ethernet(blob);
	fdt_fixup_fman_mac_addresses(blob);
#endif

	return 0;
}

#if defined(CONFIG_POST)

/* DIC26_SELFTEST GPIO used to start factory test sw */
#define SELFTEST_PORT	GPIO_A
#define SELFTEST_PIN	31

int post_hotkeys_pressed(void)
{
	qrio_gpio_direction_input(SELFTEST_PORT, SELFTEST_PIN);
	return qrio_get_gpio(SELFTEST_PORT, SELFTEST_PIN);
}
#endif
