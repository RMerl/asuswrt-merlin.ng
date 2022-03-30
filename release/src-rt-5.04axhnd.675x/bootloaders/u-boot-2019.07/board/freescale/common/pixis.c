// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006,2010 Freescale Semiconductor
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>

#define pixis_base (u8 *)PIXIS_BASE

/*
 * Simple board reset.
 */
void pixis_reset(void)
{
	out_8(pixis_base + PIXIS_RST, 0);

	while (1);
}

/*
 * Per table 27, page 58 of MPC8641HPCN spec.
 */
static int set_px_sysclk(unsigned long sysclk)
{
	u8 sysclk_s, sysclk_r, sysclk_v, vclkh, vclkl, sysclk_aux;

	switch (sysclk) {
	case 33:
		sysclk_s = 0x04;
		sysclk_r = 0x04;
		sysclk_v = 0x07;
		sysclk_aux = 0x00;
		break;
	case 40:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x20;
		sysclk_aux = 0x01;
		break;
	case 50:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x2A;
		sysclk_aux = 0x02;
		break;
	case 66:
		sysclk_s = 0x01;
		sysclk_r = 0x04;
		sysclk_v = 0x04;
		sysclk_aux = 0x03;
		break;
	case 83:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x4B;
		sysclk_aux = 0x04;
		break;
	case 100:
		sysclk_s = 0x01;
		sysclk_r = 0x1F;
		sysclk_v = 0x5C;
		sysclk_aux = 0x05;
		break;
	case 134:
		sysclk_s = 0x06;
		sysclk_r = 0x1F;
		sysclk_v = 0x3B;
		sysclk_aux = 0x06;
		break;
	case 166:
		sysclk_s = 0x06;
		sysclk_r = 0x1F;
		sysclk_v = 0x4B;
		sysclk_aux = 0x07;
		break;
	default:
		printf("Unsupported SYSCLK frequency.\n");
		return 0;
	}

	vclkh = (sysclk_s << 5) | sysclk_r;
	vclkl = sysclk_v;

	out_8(pixis_base + PIXIS_VCLKH, vclkh);
	out_8(pixis_base + PIXIS_VCLKL, vclkl);

	out_8(pixis_base + PIXIS_AUX, sysclk_aux);

	return 1;
}

/* Set the CFG_SYSPLL bits
 *
 * This only has effect if PX_VCFGEN0[SYSPLL]=1, which is true if
 * read_from_px_regs() is called.
 */
static int set_px_mpxpll(unsigned long mpxpll)
{
	switch (mpxpll) {
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
		clrsetbits_8(pixis_base + PIXIS_VSPEED1, 0x1F, mpxpll);
		return 1;
	}

	printf("Unsupported MPXPLL ratio.\n");
	return 0;
}

static int set_px_corepll(unsigned long corepll)
{
	u8 val;

	switch (corepll) {
	case 20:
		val = 0x08;
		break;
	case 25:
		val = 0x0C;
		break;
	case 30:
		val = 0x10;
		break;
	case 35:
		val = 0x1C;
		break;
	case 40:
		val = 0x14;
		break;
	case 45:
		val = 0x0E;
		break;
	default:
		printf("Unsupported COREPLL ratio.\n");
		return 0;
	}

	clrsetbits_8(pixis_base + PIXIS_VSPEED0, 0x1F, val);
	return 1;
}

#ifndef CONFIG_SYS_PIXIS_VCFGEN0_ENABLE
#define CONFIG_SYS_PIXIS_VCFGEN0_ENABLE		0x1C
#endif

/* Tell the PIXIS where to find the COREPLL, MPXPLL, SYSCLK values
 *
 * The PIXIS can be programmed to look at either the on-board dip switches
 * or various other PIXIS registers to determine the values for COREPLL,
 * MPXPLL, and SYSCLK.
 *
 * CONFIG_SYS_PIXIS_VCFGEN0_ENABLE is the value to write to the PIXIS_VCFGEN0
 * register that tells the pixis to use the various PIXIS register.
 */
static void read_from_px_regs(int set)
{
	u8 tmp = in_8(pixis_base + PIXIS_VCFGEN0);

	if (set)
		tmp = tmp | CONFIG_SYS_PIXIS_VCFGEN0_ENABLE;
	else
		tmp = tmp & ~CONFIG_SYS_PIXIS_VCFGEN0_ENABLE;

	out_8(pixis_base + PIXIS_VCFGEN0, tmp);
}

/* CONFIG_SYS_PIXIS_VBOOT_ENABLE is the value to write to the PX_VCFGEN1
 * register that tells the pixis to use the PX_VBOOT[LBMAP] register.
 */
#ifndef CONFIG_SYS_PIXIS_VBOOT_ENABLE
#define CONFIG_SYS_PIXIS_VBOOT_ENABLE	0x04
#endif

/* Configure the source of the boot location
 *
 * The PIXIS can be programmed to look at either the on-board dip switches
 * or the PX_VBOOT[LBMAP] register to determine where we should boot.
 *
 * If we want to boot from the alternate boot bank, we need to tell the PIXIS
 * to ignore the on-board dip switches and use the PX_VBOOT[LBMAP] instead.
 */
static void read_from_px_regs_altbank(int set)
{
	u8 tmp = in_8(pixis_base + PIXIS_VCFGEN1);

	if (set)
		tmp = tmp | CONFIG_SYS_PIXIS_VBOOT_ENABLE;
	else
		tmp = tmp & ~CONFIG_SYS_PIXIS_VBOOT_ENABLE;

	out_8(pixis_base + PIXIS_VCFGEN1, tmp);
}

/* CONFIG_SYS_PIXIS_VBOOT_MASK contains the bits to set in VBOOT register that
 * tells the PIXIS what the alternate flash bank is.
 *
 * Note that it's not really a mask.  It contains the actual LBMAP bits that
 * must be set to select the alternate bank.  This code assumes that the
 * primary bank has these bits set to 0, and the alternate bank has these
 * bits set to 1.
 */
#ifndef CONFIG_SYS_PIXIS_VBOOT_MASK
#define CONFIG_SYS_PIXIS_VBOOT_MASK	(0x40)
#endif

/* Tell the PIXIS to boot from the default flash bank
 *
 * Program the default flash bank into the VBOOT register.  This register is
 * used only if PX_VCFGEN1[FLASH]=1.
 */
static void clear_altbank(void)
{
	clrbits_8(pixis_base + PIXIS_VBOOT, CONFIG_SYS_PIXIS_VBOOT_MASK);
}

/* Tell the PIXIS to boot from the alternate flash bank
 *
 * Program the alternate flash bank into the VBOOT register.  This register is
 * used only if PX_VCFGEN1[FLASH]=1.
 */
static void set_altbank(void)
{
	setbits_8(pixis_base + PIXIS_VBOOT, CONFIG_SYS_PIXIS_VBOOT_MASK);
}

/* Reset the board with watchdog disabled.
 *
 * This respects the altbank setting.
 */
static void set_px_go(void)
{
	/* Disable the VELA sequencer and watchdog */
	clrbits_8(pixis_base + PIXIS_VCTL, 9);

	/* Reboot by starting the VELA sequencer */
	setbits_8(pixis_base + PIXIS_VCTL, 0x1);

	while (1);
}

/* Reset the board with watchdog enabled.
 *
 * This respects the altbank setting.
 */
static void set_px_go_with_watchdog(void)
{
	/* Disable the VELA sequencer */
	clrbits_8(pixis_base + PIXIS_VCTL, 1);

	/* Enable the watchdog and reboot by starting the VELA sequencer */
	setbits_8(pixis_base + PIXIS_VCTL, 0x9);

	while (1);
}

/* Disable the watchdog
 *
 */
static int pixis_disable_watchdog_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
				      char * const argv[])
{
	/* Disable the VELA sequencer and the watchdog */
	clrbits_8(pixis_base + PIXIS_VCTL, 9);

	return 0;
}

U_BOOT_CMD(
	diswd, 1, 0, pixis_disable_watchdog_cmd,
	"Disable watchdog timer",
	""
);

#ifdef CONFIG_PIXIS_SGMII_CMD

/* Enable or disable SGMII mode for a TSEC
 */
static int pixis_set_sgmii(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int which_tsec = -1;
	unsigned char mask;
	unsigned char switch_mask;

	if ((argc > 2) && (strcmp(argv[1], "all") != 0))
		which_tsec = simple_strtoul(argv[1], NULL, 0);

	switch (which_tsec) {
#ifdef CONFIG_TSEC1
	case 1:
		mask = PIXIS_VSPEED2_TSEC1SER;
		switch_mask = PIXIS_VCFGEN1_TSEC1SER;
		break;
#endif
#ifdef CONFIG_TSEC2
	case 2:
		mask = PIXIS_VSPEED2_TSEC2SER;
		switch_mask = PIXIS_VCFGEN1_TSEC2SER;
		break;
#endif
#ifdef CONFIG_TSEC3
	case 3:
		mask = PIXIS_VSPEED2_TSEC3SER;
		switch_mask = PIXIS_VCFGEN1_TSEC3SER;
		break;
#endif
#ifdef CONFIG_TSEC4
	case 4:
		mask = PIXIS_VSPEED2_TSEC4SER;
		switch_mask = PIXIS_VCFGEN1_TSEC4SER;
		break;
#endif
	default:
		mask = PIXIS_VSPEED2_MASK;
		switch_mask = PIXIS_VCFGEN1_MASK;
		break;
	}

	/* Toggle whether the switches or FPGA control the settings */
	if (!strcmp(argv[argc - 1], "switch"))
		clrbits_8(pixis_base + PIXIS_VCFGEN1, switch_mask);
	else
		setbits_8(pixis_base + PIXIS_VCFGEN1, switch_mask);

	/* If it's not the switches, enable or disable SGMII, as specified */
	if (!strcmp(argv[argc - 1], "on"))
		clrbits_8(pixis_base + PIXIS_VSPEED2, mask);
	else if (!strcmp(argv[argc - 1], "off"))
		setbits_8(pixis_base + PIXIS_VSPEED2, mask);

	return 0;
}

U_BOOT_CMD(
	pixis_set_sgmii, CONFIG_SYS_MAXARGS, 1, pixis_set_sgmii,
	"pixis_set_sgmii"
	" - Enable or disable SGMII mode for a given TSEC \n",
	"\npixis_set_sgmii [TSEC num] <on|off|switch>\n"
	"    TSEC num: 1,2,3,4 or 'all'.  'all' is default.\n"
	"    on - enables SGMII\n"
	"    off - disables SGMII\n"
	"    switch - use switch settings"
);

#endif

/*
 * This function takes the non-integral cpu:mpx pll ratio
 * and converts it to an integer that can be used to assign
 * FPGA register values.
 * input: strptr i.e. argv[2]
 */
static unsigned long strfractoint(char *strptr)
{
	int i, j;
	int mulconst;
	int no_dec = 0;
	unsigned long intval = 0, decval = 0;
	char intarr[3], decarr[3];

	/* Assign the integer part to intarr[]
	 * If there is no decimal point i.e.
	 * if the ratio is an integral value
	 * simply create the intarr.
	 */
	i = 0;
	while (strptr[i] != '.') {
		if (strptr[i] == 0) {
			no_dec = 1;
			break;
		}
		intarr[i] = strptr[i];
		i++;
	}

	intarr[i] = '\0';

	if (no_dec) {
		/* Currently needed only for single digit corepll ratios */
		mulconst = 10;
		decval = 0;
	} else {
		j = 0;
		i++;		/* Skipping the decimal point */
		while ((strptr[i] >= '0') && (strptr[i] <= '9')) {
			decarr[j] = strptr[i];
			i++;
			j++;
		}

		decarr[j] = '\0';

		mulconst = 1;
		for (i = 0; i < j; i++)
			mulconst *= 10;
		decval = simple_strtoul(decarr, NULL, 10);
	}

	intval = simple_strtoul(intarr, NULL, 10);
	intval = intval * mulconst;

	return intval + decval;
}

static int pixis_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int i;
	char *p_cf = NULL;
	char *p_cf_sysclk = NULL;
	char *p_cf_corepll = NULL;
	char *p_cf_mpxpll = NULL;
	char *p_altbank = NULL;
	char *p_wd = NULL;
	int unknown_param = 0;

	/*
	 * No args is a simple reset request.
	 */
	if (argc <= 1) {
		pixis_reset();
		/* not reached */
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "cf") == 0) {
			p_cf = argv[i];
			if (i + 3 >= argc) {
				break;
			}
			p_cf_sysclk = argv[i+1];
			p_cf_corepll = argv[i+2];
			p_cf_mpxpll = argv[i+3];
			i += 3;
			continue;
		}

		if (strcmp(argv[i], "altbank") == 0) {
			p_altbank = argv[i];
			continue;
		}

		if (strcmp(argv[i], "wd") == 0) {
			p_wd = argv[i];
			continue;
		}

		unknown_param = 1;
	}

	/*
	 * Check that cf has all required parms
	 */
	if ((p_cf && !(p_cf_sysclk && p_cf_corepll && p_cf_mpxpll))
	    ||	unknown_param) {
#ifdef CONFIG_SYS_LONGHELP
		puts(cmdtp->help);
		putc('\n');
#endif
		return 1;
	}

	/*
	 * PIXIS seems to be sensitive to the ordering of
	 * the registers that are touched.
	 */
	read_from_px_regs(0);

	if (p_altbank)
		read_from_px_regs_altbank(0);

	clear_altbank();

	/*
	 * Clock configuration specified.
	 */
	if (p_cf) {
		unsigned long sysclk;
		unsigned long corepll;
		unsigned long mpxpll;

		sysclk = simple_strtoul(p_cf_sysclk, NULL, 10);
		corepll = strfractoint(p_cf_corepll);
		mpxpll = simple_strtoul(p_cf_mpxpll, NULL, 10);

		if (!(set_px_sysclk(sysclk)
		      && set_px_corepll(corepll)
		      && set_px_mpxpll(mpxpll))) {
#ifdef CONFIG_SYS_LONGHELP
			puts(cmdtp->help);
			putc('\n');
#endif
			return 1;
		}
		read_from_px_regs(1);
	}

	/*
	 * Altbank specified
	 *
	 * NOTE CHANGE IN BEHAVIOR: previous code would default
	 * to enabling watchdog if altbank is specified.
	 * Now the watchdog must be enabled explicitly using 'wd'.
	 */
	if (p_altbank) {
		set_altbank();
		read_from_px_regs_altbank(1);
	}

	/*
	 * Reset with watchdog specified.
	 */
	if (p_wd)
		set_px_go_with_watchdog();
	else
		set_px_go();

	/*
	 * Shouldn't be reached.
	 */
	return 0;
}


U_BOOT_CMD(
	pixis_reset, CONFIG_SYS_MAXARGS, 1, pixis_reset_cmd,
	"Reset the board using the FPGA sequencer",
	"    pixis_reset\n"
	"    pixis_reset [altbank]\n"
	"    pixis_reset altbank wd\n"
	"    pixis_reset altbank cf <SYSCLK freq> <COREPLL ratio> <MPXPLL ratio>\n"
	"    pixis_reset cf <SYSCLK freq> <COREPLL ratio> <MPXPLL ratio>"
);
