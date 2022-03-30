// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_misc.h>

int arch_cpu_init(void)
{
	struct misc_regs *const misc_p =
	    (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 periph1_clken, periph_clk_cfg;

	periph1_clken = readl(&misc_p->periph1_clken);

#if defined(CONFIG_SPEAR3XX)
	periph1_clken |= MISC_GPT2ENB;
#elif defined(CONFIG_SPEAR600)
	periph1_clken |= MISC_GPT3ENB;
#endif

#if defined(CONFIG_PL011_SERIAL)
	periph1_clken |= MISC_UART0ENB;

	periph_clk_cfg = readl(&misc_p->periph_clk_cfg);
	periph_clk_cfg &= ~CONFIG_SPEAR_UARTCLKMSK;
	periph_clk_cfg |= CONFIG_SPEAR_UART48M;
	writel(periph_clk_cfg, &misc_p->periph_clk_cfg);
#endif
#if defined(CONFIG_ETH_DESIGNWARE)
	periph1_clken |= MISC_ETHENB;
#endif
#if defined(CONFIG_DW_UDC)
	periph1_clken |= MISC_USBDENB;
#endif
#if defined(CONFIG_SYS_I2C_DW)
	periph1_clken |= MISC_I2CENB;
#endif
#if defined(CONFIG_ST_SMI)
	periph1_clken |= MISC_SMIENB;
#endif
#if defined(CONFIG_NAND_FSMC)
	periph1_clken |= MISC_FSMCENB;
#endif
#if defined(CONFIG_USB_EHCI_SPEAR)
	periph1_clken |= PERIPH_USBH1 | PERIPH_USBH2;
#endif
#if defined(CONFIG_SPEAR_GPIO)
	periph1_clken |= MISC_GPIO3ENB | MISC_GPIO4ENB;
#endif
#if defined(CONFIG_PL022_SPI)
	periph1_clken |= MISC_SSP1ENB | MISC_SSP2ENB | MISC_SSP3ENB;
#endif

	writel(periph1_clken, &misc_p->periph1_clken);

	return 0;
}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
#ifdef CONFIG_SPEAR300
	printf("CPU:   SPEAr300\n");
#elif defined(CONFIG_SPEAR310)
	printf("CPU:   SPEAr310\n");
#elif defined(CONFIG_SPEAR320)
	printf("CPU:   SPEAr320\n");
#elif defined(CONFIG_SPEAR600)
	printf("CPU:   SPEAr600\n");
#else
#error CPU not supported in spear platform
#endif
	return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_NAND_ECC_BCH) && defined(CONFIG_NAND_FSMC)
static int do_switch_ecc(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	if (argc != 2)
		goto usage;

	if (strncmp(argv[1], "hw", 2) == 0) {
		/* 1-bit HW ECC */
		printf("Switching to 1-bit HW ECC\n");
		fsmc_nand_switch_ecc(1);
	} else if (strncmp(argv[1], "bch4", 2) == 0) {
		/* 4-bit SW ECC BCH4 */
		printf("Switching to 4-bit SW ECC (BCH4)\n");
		fsmc_nand_switch_ecc(4);
	} else {
		goto usage;
	}

	return 0;

usage:
	printf("Usage: nandecc %s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	nandecc, 2, 0,	do_switch_ecc,
	"switch NAND ECC calculation algorithm",
	"hw|bch4 - Switch between NAND hardware 1-bit HW and"
	" 4-bit SW BCH\n"
);
#endif
