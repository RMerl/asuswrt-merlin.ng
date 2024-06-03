// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Common board functions for OMAP3 based boards.
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Sunil Kumar <sunilsaini05@gmail.com>
 *      Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 *
 */
#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/cache.h>
#include <asm/armv7.h>
#include <asm/gpio.h>
#include <asm/omap_common.h>
#include <linux/compiler.h>

/* Declarations */
extern omap3_sysinfo sysinfo;
#ifndef CONFIG_SYS_L2CACHE_OFF
static void omap3_invalidate_l2_cache_secure(void);
#endif

#ifdef CONFIG_DM_GPIO
#if !CONFIG_IS_ENABLED(OF_CONTROL)
/* Manually initialize GPIO banks when OF_CONTROL doesn't */
static const struct omap_gpio_platdata omap34xx_gpio[] = {
	{ 0, OMAP34XX_GPIO1_BASE },
	{ 1, OMAP34XX_GPIO2_BASE },
	{ 2, OMAP34XX_GPIO3_BASE },
	{ 3, OMAP34XX_GPIO4_BASE },
	{ 4, OMAP34XX_GPIO5_BASE },
	{ 5, OMAP34XX_GPIO6_BASE },
};

U_BOOT_DEVICES(omap34xx_gpios) = {
	{ "gpio_omap", &omap34xx_gpio[0] },
	{ "gpio_omap", &omap34xx_gpio[1] },
	{ "gpio_omap", &omap34xx_gpio[2] },
	{ "gpio_omap", &omap34xx_gpio[3] },
	{ "gpio_omap", &omap34xx_gpio[4] },
	{ "gpio_omap", &omap34xx_gpio[5] },
};
#endif
#else

static const struct gpio_bank gpio_bank_34xx[6] = {
	{ (void *)OMAP34XX_GPIO1_BASE },
	{ (void *)OMAP34XX_GPIO2_BASE },
	{ (void *)OMAP34XX_GPIO3_BASE },
	{ (void *)OMAP34XX_GPIO4_BASE },
	{ (void *)OMAP34XX_GPIO5_BASE },
	{ (void *)OMAP34XX_GPIO6_BASE },
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_34xx;

#endif

/******************************************************************************
 * Routine: secure_unlock
 * Description: Setup security registers for access
 *              (GP Device only)
 *****************************************************************************/
void secure_unlock_mem(void)
{
	struct pm *pm_rt_ape_base = (struct pm *)PM_RT_APE_BASE_ADDR_ARM;
	struct pm *pm_gpmc_base = (struct pm *)PM_GPMC_BASE_ADDR_ARM;
	struct pm *pm_ocm_ram_base = (struct pm *)PM_OCM_RAM_BASE_ADDR_ARM;
	struct pm *pm_iva2_base = (struct pm *)PM_IVA2_BASE_ADDR_ARM;
	struct sms *sms_base = (struct sms *)OMAP34XX_SMS_BASE;

	/* Protection Module Register Target APE (PM_RT) */
	writel(UNLOCK_1, &pm_rt_ape_base->req_info_permission_1);
	writel(UNLOCK_1, &pm_rt_ape_base->read_permission_0);
	writel(UNLOCK_1, &pm_rt_ape_base->wirte_permission_0);
	writel(UNLOCK_2, &pm_rt_ape_base->addr_match_1);

	writel(UNLOCK_3, &pm_gpmc_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_gpmc_base->read_permission_0);
	writel(UNLOCK_3, &pm_gpmc_base->wirte_permission_0);

	writel(UNLOCK_3, &pm_ocm_ram_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_ocm_ram_base->read_permission_0);
	writel(UNLOCK_3, &pm_ocm_ram_base->wirte_permission_0);
	writel(UNLOCK_2, &pm_ocm_ram_base->addr_match_2);

	/* IVA Changes */
	writel(UNLOCK_3, &pm_iva2_base->req_info_permission_0);
	writel(UNLOCK_3, &pm_iva2_base->read_permission_0);
	writel(UNLOCK_3, &pm_iva2_base->wirte_permission_0);

	/* SDRC region 0 public */
	writel(UNLOCK_1, &sms_base->rg_att0);
}

/******************************************************************************
 * Routine: secureworld_exit()
 * Description: If chip is EMU and boot type is external
 *		configure secure registers and exit secure world
 *              general use.
 *****************************************************************************/
void secureworld_exit(void)
{
	unsigned long i;

	/* configure non-secure access control register */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 2":"=r"(i));
	/* enabling co-processor CP10 and CP11 accesses in NS world */
	__asm__ __volatile__("orr %0, %0, #0xC00":"=r"(i));
	/*
	 * allow allocation of locked TLBs and L2 lines in NS world
	 * allow use of PLE registers in NS world also
	 */
	__asm__ __volatile__("orr %0, %0, #0x70000":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 2":"=r"(i));

	/* Enable ASA in ACR register */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c0, 1":"=r"(i));
	__asm__ __volatile__("orr %0, %0, #0x10":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c0, 1":"=r"(i));

	/* Exiting secure world */
	__asm__ __volatile__("mrc p15, 0, %0, c1, c1, 0":"=r"(i));
	__asm__ __volatile__("orr %0, %0, #0x31":"=r"(i));
	__asm__ __volatile__("mcr p15, 0, %0, c1, c1, 0":"=r"(i));
}

/******************************************************************************
 * Routine: try_unlock_sram()
 * Description: If chip is GP/EMU(special) type, unlock the SRAM for
 *              general use.
 *****************************************************************************/
void try_unlock_memory(void)
{
	int mode;
	int in_sdram = is_running_in_sdram();

	/*
	 * if GP device unlock device SRAM for general use
	 * secure code breaks for Secure/Emulation device - HS/E/T
	 */
	mode = get_device_type();
	if (mode == GP_DEVICE)
		secure_unlock_mem();

	/*
	 * If device is EMU and boot is XIP external booting
	 * Unlock firewalls and disable L2 and put chip
	 * out of secure world
	 *
	 * Assuming memories are unlocked by the demon who put us in SDRAM
	 */
	if ((mode <= EMU_DEVICE) && (get_boot_type() == 0x1F)
	    && (!in_sdram)) {
		secure_unlock_mem();
		secureworld_exit();
	}

	return;
}

void early_system_init(void)
{
	hw_data_init();
}

/******************************************************************************
 * Routine: s_init
 * Description: Does early system init of muxing and clocks.
 *              - Called path is with SRAM stack.
 *****************************************************************************/
void s_init(void)
{
	watchdog_init();
	early_system_init();

	try_unlock_memory();

#ifndef CONFIG_SYS_L2CACHE_OFF
	/* Invalidate L2-cache from secure mode */
	omap3_invalidate_l2_cache_secure();
#endif

	set_muxconf_regs();
	sdelay(100);

	prcm_init();

	per_clocks_enable();

#ifdef CONFIG_USB_EHCI_OMAP
	ehci_clocks_enable();
#endif
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	early_system_init();
	mem_init();
	/*
	* Save the boot parameters passed from romcode.
	* We cannot delay the saving further than this,
	* to prevent overwrites.
	*/
	save_omap_boot_params();
}
#endif

/*
 * Routine: misc_init_r
 * Description: A basic misc_init_r that just displays the die ID
 */
int __weak misc_init_r(void)
{
	omap_die_id_display();

	return 0;
}

/******************************************************************************
 * Routine: wait_for_command_complete
 * Description: Wait for posting to finish on watchdog
 *****************************************************************************/
static void wait_for_command_complete(struct watchdog *wd_base)
{
	int pending = 1;
	do {
		pending = readl(&wd_base->wwps);
	} while (pending);
}

/******************************************************************************
 * Routine: watchdog_init
 * Description: Shut down watch dogs
 *****************************************************************************/
void watchdog_init(void)
{
	struct watchdog *wd2_base = (struct watchdog *)WD2_BASE;
	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;

	/*
	 * There are 3 watch dogs WD1=Secure, WD2=MPU, WD3=IVA. WD1 is
	 * either taken care of by ROM (HS/EMU) or not accessible (GP).
	 * We need to take care of WD2-MPU or take a PRCM reset. WD3
	 * should not be running and does not generate a PRCM reset.
	 */

	setbits_le32(&prcm_base->fclken_wkup, 0x20);
	setbits_le32(&prcm_base->iclken_wkup, 0x20);
	wait_on_value(ST_WDT2, 0x20, &prcm_base->idlest_wkup, 5);

	writel(WD_UNLOCK1, &wd2_base->wspr);
	wait_for_command_complete(wd2_base);
	writel(WD_UNLOCK2, &wd2_base->wspr);
}

/******************************************************************************
 * Dummy function to handle errors for EABI incompatibility
 *****************************************************************************/
void abort(void)
{
}

#if defined(CONFIG_NAND_OMAP_GPMC) & !defined(CONFIG_SPL_BUILD)
/******************************************************************************
 * OMAP3 specific command to switch between NAND HW and SW ecc
 *****************************************************************************/
static int do_switch_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int hw, strength = 1;

	if (argc < 2 || argc > 3)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		hw = 1;
		if (argc == 3) {
			if (strncmp(argv[2], "bch8", 4) == 0)
				strength = 8;
			else if (strncmp(argv[2], "bch16", 5) == 0)
				strength = 16;
			else if (strncmp(argv[2], "hamming", 7) != 0)
				goto usage;
		}
	} else if (strncmp(argv[1], "sw", 2) == 0) {
		hw = 0;
		if (argc == 3) {
			if (strncmp(argv[2], "bch8", 4) == 0)
				strength = 8;
			else if (strncmp(argv[2], "hamming", 7) != 0)
				goto usage;
		}
	} else {
		goto usage;
	}

	return -omap_nand_switch_ecc(hw, strength);

usage:
	printf ("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 3, 1,	do_switch_ecc,
	"switch OMAP3 NAND ECC calculation algorithm",
	"hw [hamming|bch8|bch16] - Switch between NAND hardware 1-bit hamming"
	" and 8-bit/16-bit BCH\n"
	"                           ecc calculation (second parameter may"
	" be omitted).\n"
	"nandecc sw               - Switch to NAND software ecc algorithm."
);

#endif /* CONFIG_NAND_OMAP_GPMC & !CONFIG_SPL_BUILD */

#ifdef CONFIG_DISPLAY_BOARDINFO
/**
 * Print board information
 */
int checkboard (void)
{
	char *mem_s ;

	if (is_mem_sdr())
		mem_s = "mSDR";
	else
		mem_s = "LPDDR";

	printf("%s + %s/%s\n", sysinfo.board_string, mem_s,
			sysinfo.nand_string);

	return 0;
}
#endif	/* CONFIG_DISPLAY_BOARDINFO */

static void omap3_emu_romcode_call(u32 service_id, u32 *parameters)
{
	u32 i, num_params = *parameters;
	u32 *sram_scratch_space = (u32 *)OMAP3_PUBLIC_SRAM_SCRATCH_AREA;

	/*
	 * copy the parameters to an un-cached area to avoid coherency
	 * issues
	 */
	for (i = 0; i < num_params; i++) {
		__raw_writel(*parameters, sram_scratch_space);
		parameters++;
		sram_scratch_space++;
	}

	/* Now make the PPA call */
	do_omap3_emu_romcode_call(service_id, OMAP3_PUBLIC_SRAM_SCRATCH_AREA);
}

void __weak omap3_set_aux_cr_secure(u32 acr)
{
	struct emu_hal_params emu_romcode_params;

	emu_romcode_params.num_params = 1;
	emu_romcode_params.param1 = acr;
	omap3_emu_romcode_call(OMAP3_EMU_HAL_API_WRITE_ACR,
			       (u32 *)&emu_romcode_params);
}

void v7_arch_cp15_set_l2aux_ctrl(u32 l2auxctrl, u32 cpu_midr,
				 u32 cpu_rev_comb, u32 cpu_variant,
				 u32 cpu_rev)
{
	if (get_device_type() == GP_DEVICE)
		omap_smc1(OMAP3_GP_ROMCODE_API_WRITE_L2ACR, l2auxctrl);

	/* L2 Cache Auxiliary Control Register is not banked */
}

void v7_arch_cp15_set_acr(u32 acr, u32 cpu_midr, u32 cpu_rev_comb,
			  u32 cpu_variant, u32 cpu_rev)
{
	/* Write ACR - affects secure banked bits */
	if (get_device_type() == GP_DEVICE)
		omap_smc1(OMAP3_GP_ROMCODE_API_WRITE_ACR, acr);
	else
		omap3_set_aux_cr_secure(acr);

	/* Write ACR - affects non-secure banked bits - some erratas need it */
	asm volatile ("mcr p15, 0, %0, c1, c0, 1" : : "r" (acr));
}


#ifndef CONFIG_SYS_L2CACHE_OFF
static void omap3_update_aux_cr(u32 set_bits, u32 clear_bits)
{
	u32 acr;

	/* Read ACR */
	asm volatile ("mrc p15, 0, %0, c1, c0, 1" : "=r" (acr));
	acr &= ~clear_bits;
	acr |= set_bits;
	v7_arch_cp15_set_acr(acr, 0, 0, 0, 0);

}

/* Invalidate the entire L2 cache from secure mode */
static void omap3_invalidate_l2_cache_secure(void)
{
	if (get_device_type() == GP_DEVICE) {
		omap_smc1(OMAP3_GP_ROMCODE_API_L2_INVAL, 0);
	} else {
		struct emu_hal_params emu_romcode_params;
		emu_romcode_params.num_params = 1;
		emu_romcode_params.param1 = 0;
		omap3_emu_romcode_call(OMAP3_EMU_HAL_API_L2_INVAL,
				       (u32 *)&emu_romcode_params);
	}
}

void v7_outer_cache_enable(void)
{

	/*
	 * Set L2EN
	 * On some revisions L2EN bit is banked on some revisions it's not
	 * No harm in setting both banked bits(in fact this is required
	 * by an erratum)
	 */
	omap3_update_aux_cr(0x2, 0);
}

void omap3_outer_cache_disable(void)
{
	/*
	 * Clear L2EN
	 * On some revisions L2EN bit is banked on some revisions it's not
	 * No harm in clearing both banked bits(in fact this is required
	 * by an erratum)
	 */
	omap3_update_aux_cr(0, 0x2);
}
#endif /* !CONFIG_SYS_L2CACHE_OFF */
