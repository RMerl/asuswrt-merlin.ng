// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <dm.h>
#include <fsl_validate.h>
#include <fsl_secboot_err.h>
#include <fsl_sfp.h>
#include <dm/root.h>

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_FRAMEWORK)
#include <spl.h>
#endif

#ifdef CONFIG_ADDR_MAP
#include <asm/mmu.h>
#endif

#ifdef CONFIG_FSL_CORENET
#include <asm/fsl_pamu.h>
#endif

#ifdef CONFIG_ARCH_LS1021A
#include <asm/arch/immap_ls102xa.h>
#endif

#if defined(CONFIG_MPC85xx)
#define CONFIG_DCFG_ADDR	CONFIG_SYS_MPC85xx_GUTS_ADDR
#else
#define CONFIG_DCFG_ADDR	CONFIG_SYS_FSL_GUTS_ADDR
#endif

#ifdef CONFIG_SYS_FSL_CCSR_GUR_LE
#define gur_in32(a)       in_le32(a)
#else
#define gur_in32(a)       in_be32(a)
#endif

/* Check the Boot Mode. If Secure, return 1 else return 0 */
int fsl_check_boot_mode_secure(void)
{
	uint32_t val;
	struct ccsr_sfp_regs *sfp_regs = (void *)(CONFIG_SYS_SFP_ADDR);
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_DCFG_ADDR);

	val = sfp_in32(&sfp_regs->ospr) & ITS_MASK;
	if (val == ITS_MASK)
		return 1;

#if defined(CONFIG_FSL_CORENET) || !defined(CONFIG_MPC85xx)
	/* For PBL based platforms check the SB_EN bit in RCWSR */
	val = gur_in32(&gur->rcwsr[RCW_SB_EN_REG_INDEX - 1]) & RCW_SB_EN_MASK;
	if (val == RCW_SB_EN_MASK)
		return 1;
#endif

#if defined(CONFIG_MPC85xx) && !defined(CONFIG_FSL_CORENET)
	/* For Non-PBL Platforms, check the Device Status register 2*/
	val = gur_in32(&gur->pordevsr2) & MPC85xx_PORDEVSR2_SBC_MASK;
	if (val != MPC85xx_PORDEVSR2_SBC_MASK)
		return 1;

#endif
	return 0;
}

#ifndef CONFIG_SPL_BUILD
int fsl_setenv_chain_of_trust(void)
{
	/* Check Boot Mode
	 * If Boot Mode is Non-Secure, no changes are required
	 */
	if (fsl_check_boot_mode_secure() == 0)
		return 0;

	/* If Boot mode is Secure, set the environment variables
	 * bootdelay = 0 (To disable Boot Prompt)
	 * bootcmd = CONFIG_CHAIN_BOOT_CMD (Validate and execute Boot script)
	 */
	env_set("bootdelay", "-2");

#ifdef CONFIG_ARM
	env_set("secureboot", "y");
#else
	env_set("bootcmd", CONFIG_CHAIN_BOOT_CMD);
#endif

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_validate_uboot(uint32_t hdr_addr, uintptr_t img_addr)
{
	int res;

	/*
	 * Check Boot Mode
	 * If Boot Mode is Non-Secure, skip validation
	 */
	if (fsl_check_boot_mode_secure() == 0)
		return;

	printf("SPL: Validating U-Boot image\n");

#ifdef CONFIG_ADDR_MAP
	init_addr_map();
#endif

#ifdef CONFIG_FSL_CORENET
	if (pamu_init() < 0)
		fsl_secboot_handle_error(ERROR_ESBC_PAMU_INIT);
#endif

#ifdef CONFIG_FSL_CAAM
	if (sec_init() < 0)
		fsl_secboot_handle_error(ERROR_ESBC_SEC_INIT);
#endif

/*
 * dm_init_and_scan() is called as part of common SPL framework, so no
 * need to call it again but in case of powerpc platforms which currently
 * do not use common SPL framework, so need to call this function here.
 */
#if defined(CONFIG_SPL_DM) && (!defined(CONFIG_SPL_FRAMEWORK))
	dm_init_and_scan(true);
#endif
	res = fsl_secboot_validate(hdr_addr, CONFIG_SPL_UBOOT_KEY_HASH,
				   &img_addr);

	if (res == 0)
		printf("SPL: Validation of U-boot successful\n");
}

#ifdef CONFIG_SPL_FRAMEWORK
/* Override weak funtion defined in SPL framework to enable validation
 * of main u-boot image before jumping to u-boot image.
 */
void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	typedef void __noreturn (*image_entry_noargs_t)(void);
	uint32_t hdr_addr;

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)(unsigned long)spl_image->entry_point;

	hdr_addr = (spl_image->entry_point + spl_image->size -
			CONFIG_U_BOOT_HDR_SIZE);
	spl_validate_uboot(hdr_addr, (uintptr_t)spl_image->entry_point);
	/*
	 * In case of failure in validation, spl_validate_uboot would
	 * not return back in case of Production environment with ITS=1.
	 * Thus U-Boot will not start.
	 * In Development environment (ITS=0 and SB_EN=1), the function
	 * may return back in case of non-fatal failures.
	 */

	debug("image entry point: 0x%lX\n", spl_image->entry_point);
	image_entry();
}
#endif /* ifdef CONFIG_SPL_FRAMEWORK */
#endif /* ifdef CONFIG_SPL_BUILD */
