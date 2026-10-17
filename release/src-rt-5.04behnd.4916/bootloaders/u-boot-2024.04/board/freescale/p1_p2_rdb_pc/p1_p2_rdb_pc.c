// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011, 2013 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <hang.h>
#include <hwconfig.h>
#include <image.h>
#include <init.h>
#include <net.h>
#include <pci.h>
#include <i2c.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_law.h>
#include <asm/fsl_lbc.h>
#include <asm/mp.h>
#include <miiphy.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <vsc7385.h>
#include <ioports.h>
#include <asm/fsl_serdes.h>
#include <netdev.h>

#ifdef CONFIG_QE

#define GPIO_GETH_SW_PORT	1
#define GPIO_GETH_SW_PIN	29
#define GPIO_GETH_SW_DATA	(1 << (31 - GPIO_GETH_SW_PIN))

#define GPIO_SLIC_PORT		1
#define GPIO_SLIC_PIN		30
#define GPIO_SLIC_DATA		(1 << (31 - GPIO_SLIC_PIN))

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* GPIO */
	{1,   1, 2, 0, 0}, /* GPIO7/PB1   - LOAD_DEFAULT_N */
	{0,  15, 1, 0, 0}, /* GPIO11/A15  - WDI */
	{GPIO_GETH_SW_PORT, GPIO_GETH_SW_PIN, 1, 0, 0},	/* RST_GETH_SW_N */
	{GPIO_SLIC_PORT, GPIO_SLIC_PIN, 1, 0, 0},	/* RST_SLIC_N */
	{0,  0, 0, 0, QE_IOP_TAB_END} /* END of table */
};
#endif

struct cpld_data {
	u8 cpld_rev_major;
	u8 pcba_rev;
	u8 wd_cfg;
	u8 rst_bps_sw;
	u8 load_default_n;
	u8 rst_bps_wd;
	u8 bypass_enable;
	u8 bps_led;
	u8 status_led;                  /* offset: 0x8 */
	u8 fxo_led;                     /* offset: 0x9 */
	u8 fxs_led;                     /* offset: 0xa */
	u8 rev4[2];
	u8 system_rst;                  /* offset: 0xd */
	u8 bps_out;
	u8 rev5[3];
	u8 cpld_rev_minor;
};

#define CPLD_WD_CFG	0x03
#define CPLD_RST_BSW	0x00
#define CPLD_RST_BWD	0x00
#define CPLD_BYPASS_EN	0x03
#define CPLD_STATUS_LED	0x01
#define CPLD_FXO_LED	0x01
#define CPLD_FXS_LED	0x0F
#define CPLD_SYS_RST	0x00

void board_reset_prepare(void)
{
	/*
	 * During reset preparation, turn off external watchdog.
	 * This ensures that external watchdog does not trigger
	 * another reset or possible infinite reset loop.
	 */
	struct cpld_data *cpld_data = (void *)(CFG_SYS_CPLD_BASE);
	out_8(&cpld_data->wd_cfg, CPLD_WD_CFG);
	in_8(&cpld_data->wd_cfg); /* Read back to sync write */
}

void board_reset_last(void)
{
	struct cpld_data *cpld_data = (void *)(CFG_SYS_CPLD_BASE);
	out_8(&cpld_data->system_rst, 1);
}

void board_cpld_init(void)
{
	struct cpld_data *cpld_data = (void *)(CFG_SYS_CPLD_BASE);
	u8 prev_wd_cfg = in_8(&cpld_data->wd_cfg);

	out_8(&cpld_data->wd_cfg, CPLD_WD_CFG);
	out_8(&cpld_data->status_led, CPLD_STATUS_LED);
	out_8(&cpld_data->fxo_led, CPLD_FXO_LED);
	out_8(&cpld_data->fxs_led, CPLD_FXS_LED);

	/*
	 * CPLD's system reset register on P1/P2 RDB boards is not autocleared
	 * after flipping it. If this register is set to one then CPLD triggers
	 * reset of CPU in few ms.
	 *
	 * CPLD does not trigger reset of CPU for 100ms after the last reset.
	 *
	 * This means that trying to reset board via CPLD system reset register
	 * cause reboot loop. To prevent this reboot loop, the only workaround
	 * is to try to clear CPLD's system reset register as early as possible
	 * and it has to be done in 100ms since the last start of reset.
	 */
	out_8(&cpld_data->system_rst, CPLD_SYS_RST);

	/*
	 * If watchdog timer was already set to non-disabled value then it means
	 * that watchdog timer was already activated, has already expired and
	 * caused CPU reset. If this happened then due to CPLD firmware bug,
	 * writing to wd_cfg register has no effect and therefore it is not
	 * possible to reactivate watchdog timer again. Also if CPU was reset
	 * via watchdog then some peripherals like i2c do not work. Watchdog and
	 * i2c start working again after CPU reset via non-watchdog method.
	 *
	 * So in case watchdog timer register in CPLD was already enabled then
	 * disable it in CPLD and reset CPU which cause new boot. Watchdog timer
	 * is disabled few lines above, after reading CPLD previous value.
	 * This logic (disabling timer before reset) prevents reboot loop.
	 */
	if (prev_wd_cfg != CPLD_WD_CFG) {
		eieio();
		do_reset(NULL, 0, 0, NULL);
		while (1); /* do_reset() does not occur immediately */
	}
}

void board_gpio_init(void)
{
#ifdef CONFIG_QE
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	par_io_t *par_io = (par_io_t *) &(gur->qe_par_io);

	/* Enable VSC7385 switch */
	setbits_be32(&par_io[GPIO_GETH_SW_PORT].cpdat, GPIO_GETH_SW_DATA);

	/* Enable SLIC */
	setbits_be32(&par_io[GPIO_SLIC_PORT].cpdat, GPIO_SLIC_DATA);
#else

	ccsr_gpio_t *pgpio = (void *)(CFG_SYS_MPC85xx_GPIO_ADDR);

	/*
	 * GPIO10 DDR Reset, open drain
	 * GPIO7  LOAD_DEFAULT_N          Input
	 * GPIO11  WDI (watchdog input)
	 * GPIO12  Ethernet Switch Reset
	 * GPIO13  SLIC Reset
	 */

	setbits_be32(&pgpio->gpdir, 0x02130000);
#if !defined(CONFIG_SYS_RAMBOOT) && !defined(CONFIG_SPL)
	/* init DDR3 reset signal */
	setbits_be32(&pgpio->gpdir, 0x00200000);
	setbits_be32(&pgpio->gpodr, 0x00200000);
	clrbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(1000);
	setbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(1000);
	clrbits_be32(&pgpio->gpdir, 0x00200000);
#endif

#ifdef CONFIG_VSC7385_ENET
	/* reset VSC7385 Switch */
	setbits_be32(&pgpio->gpdir, 0x00080000);
	setbits_be32(&pgpio->gpdat, 0x00080000);
#endif

#ifdef CFG_SLIC
	/* reset SLIC */
	setbits_be32(&pgpio->gpdir, 0x00040000);
	setbits_be32(&pgpio->gpdat, 0x00040000);
#endif
#endif
}

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SDHC_CD);
#ifndef SDHC_WP_IS_GPIO
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SDHC_WP);
#endif
	clrbits_be32(&gur->sdhcdcr, SDHCDCR_CD_INV);

	clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);
#if defined(CONFIG_TARGET_P1020RDB_PD) || defined(CONFIG_TARGET_P1020RDB_PC)
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_TDM_ENA);
#endif

	board_gpio_init();
	board_cpld_init();

	return 0;
}

#if defined(CONFIG_TARGET_P1020RDB_PC)
#define BOARD_NAME "P1020RDB-PC"
#elif defined(CONFIG_TARGET_P1020RDB_PD)
#define BOARD_NAME "P1020RDB-PD"
#elif defined(CONFIG_TARGET_P2020RDB)
#define BOARD_NAME "P2020RDB-PC"
#endif

int checkboard(void)
{
	struct cpld_data *cpld_data = (void *)(CFG_SYS_CPLD_BASE);
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u8 in, out, invert, io_config, val;
	int bus_num = CONFIG_SYS_SPD_BUS_NUM;

	/* FIXME: This should just use the model from the device tree or similar */
#ifdef BOARD_NAME
	printf("Board: %s ", BOARD_NAME);
#endif

	printf("CPLD: V%d.%d PCBA: V%d.0\n",
		in_8(&cpld_data->cpld_rev_major) & 0x0F,
		in_8(&cpld_data->cpld_rev_minor) & 0x0F,
		in_8(&cpld_data->pcba_rev) & 0x0F);

	/* Initialize i2c early for rom_loc and flash bank information */
	#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(bus_num, CFG_SYS_I2C_PCA9557_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return 0; /* Don't want to hang() on this error */
	}

	if (dm_i2c_read(dev, 0, &in, 1) < 0 ||
	    dm_i2c_read(dev, 1, &out, 1) < 0 ||
	    dm_i2c_read(dev, 2, &invert, 1) < 0 ||
	    dm_i2c_read(dev, 3, &io_config, 1) < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}
	#else /* Non DM I2C support - will be removed */
	i2c_set_bus_num(bus_num);

	if (i2c_read(CFG_SYS_I2C_PCA9557_ADDR, 0, 1, &in, 1) < 0 ||
	    i2c_read(CFG_SYS_I2C_PCA9557_ADDR, 1, 1, &out, 1) < 0 ||
	    i2c_read(CFG_SYS_I2C_PCA9557_ADDR, 2, 1, &invert, 1) < 0 ||
	    i2c_read(CFG_SYS_I2C_PCA9557_ADDR, 3, 1, &io_config, 1) < 0) {
		printf("Error reading i2c boot information!\n");
		return 0; /* Don't want to hang() on this error */
	}
	#endif

	val = ((in ^ invert) & io_config) | (out & (~io_config));

	puts("rom_loc: ");
	if (0) {
#ifdef __SW_BOOT_SD
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_SD) {
		puts("sd");
#endif
#ifdef __SW_BOOT_SD2
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_SD2) {
		puts("sd");
#endif
#ifdef __SW_BOOT_SPI
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_SPI) {
		puts("spi");
#endif
#ifdef __SW_BOOT_NAND
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_NAND) {
		puts("nand");
#endif
#ifdef __SW_BOOT_PCIE
	} else if ((val & (~__SW_BOOT_MASK)) == __SW_BOOT_PCIE) {
		puts("pcie");
#endif
	} else {
		if (val & 0x2)
			puts("nor lower bank");
		else
			puts("nor upper bank");
	}
	puts("\n");

	if (val & 0x1) {
		setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_SD_DATA);
		puts("SD/MMC : 8-bit Mode\n");
		puts("eSPI : Disabled\n");
	} else {
		puts("SD/MMC : 4-bit Mode\n");
		puts("eSPI : Enabled\n");
	}

	return 0;
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CFG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);
#ifdef CONFIG_VSC7385_ENET
	unsigned int vscfw_addr;
	char *tmp;
#endif

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	if (flash_esel == -1) {
		/* very unlikely unless something is messed up */
		puts("Error: Could not find TLB for FLASH BASE\n");
		flash_esel = 2;	/* give our best effort to continue */
	} else {
		/* invalidate existing TLB entry for flash */
		disable_tlb(flash_esel);
	}

	set_tlb(1, flashbase, CFG_SYS_FLASH_BASE_PHYS, /* tlb, epn, rpn */
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,/* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_64M, 1);/* ts, esel, tsize, iprot */

#ifdef CONFIG_VSC7385_ENET
	/* If a VSC7385 microcode image is present, then upload it. */
	tmp = env_get("vscfw_addr");
	if (tmp) {
		vscfw_addr = hextoul(tmp, NULL);
		printf("uploading VSC7385 microcode from %x\n", vscfw_addr);
		if (vsc7385_upload_firmware((void *)vscfw_addr,
					    CFG_VSC7385_IMAGE_SIZE))
			puts("Failure uploading VSC7385 microcode.\n");
	} else {
		puts("No address specified for VSC7385 microcode.\n");
	}
#endif
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP) || defined(CONFIG_OF_BOARD_FIXUP)
static void fix_max6370_watchdog(void *blob)
{
	int off = fdt_node_offset_by_compatible(blob, -1, "maxim,max6370");
	ccsr_gpio_t *pgpio = (void *)(CFG_SYS_MPC85xx_GPIO_ADDR);
	u32 gpioval = in_be32(&pgpio->gpdat);

	/*
	 * Delete watchdog max6370 node in load_default mode (detected by
	 * GPIO7 - LOAD_DEFAULT_N) because CPLD in load_default mode ignores
	 * watchdog reset signal. CPLD in load_default mode does not reset
	 * board when watchdog triggers reset signal.
	 */
	if (!(gpioval & BIT(31-7)) && off >= 0)
		fdt_del_node(blob, off);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	phys_addr_t base;
	phys_size_t size;
#if defined(CONFIG_TARGET_P1020RDB_PD) || defined(CONFIG_TARGET_P1020RDB_PC)
	const char *soc_usb_compat = "fsl-usb2-dr";
	int usb_err, usb1_off, usb2_off;
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	int err;
#endif
#endif

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_QE
	do_fixup_by_compat(blob, "fsl,qe", "status", "okay",
			sizeof("okay"), 0);
#endif

	fix_max6370_watchdog(blob);

#if defined(CONFIG_HAS_FSL_DR_USB)
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#if defined(CONFIG_TARGET_P1020RDB_PD) || defined(CONFIG_TARGET_P1020RDB_PC)
#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	/* Delete eLBC node as it is muxed with USB2 controller */
	if (hwconfig("usb2")) {
		const char *soc_elbc_compat = "fsl,p1020-elbc";
		int off = fdt_node_offset_by_compatible(blob, -1,
				soc_elbc_compat);
		if (off < 0) {
			printf("WARNING: could not find compatible node %s\n",
			       soc_elbc_compat);
			return off;
		}
		err = fdt_del_node(blob, off);
		if (err < 0) {
			printf("WARNING: could not remove %s\n",
			       soc_elbc_compat);
			return err;
		}
		return 0;
	}
#endif

/* Delete USB2 node as it is muxed with eLBC */
	usb1_off = fdt_node_offset_by_compatible(blob, -1,
		soc_usb_compat);
	if (usb1_off < 0) {
		printf("WARNING: could not find compatible node %s\n",
		       soc_usb_compat);
		return usb1_off;
	}
	usb2_off = fdt_node_offset_by_compatible(blob, usb1_off,
			soc_usb_compat);
	if (usb2_off < 0) {
		printf("WARNING: could not find compatible node %s\n",
		       soc_usb_compat);
		return usb2_off;
	}
	usb_err = fdt_del_node(blob, usb2_off);
	if (usb_err < 0) {
		printf("WARNING: could not remove %s\n", soc_usb_compat);
		return usb_err;
	}
#endif

	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_FIXUP
int board_fix_fdt(void *blob)
{
	fix_max6370_watchdog(blob);
	return 0;
}
#endif
