// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008-2013 Eric Jarrige <eric.jarrige@armadeus.org>
 *
 * based on the files by
 * Sascha Hauer, Pengutronix
 */

#include <common.h>
#include <environment.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <linux/errno.h>
#include <u-boot/crc.h>
#include "apf27.h"
#include "fpga.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Fuse bank 1 row 8 is "reserved for future use" and therefore available for
 * customer use. The APF27 board uses this fuse to store the board revision:
 * 0: initial board revision
 * 1: first revision - Presence of the second RAM chip on the board is blown in
 *     fuse bank 1 row 9  bit 0 - No hardware change
 * N: to be defined
 */
static u32 get_board_rev(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;

	return readl(&iim->bank[1].fuse_regs[8]);
}

/*
 * Fuse bank 1 row 9 is "reserved for future use" and therefore available for
 * customer use. The APF27 board revision 1 uses the bit 0 to permanently store
 * the presence of the second RAM chip
 * 0: AFP27 with 1 RAM of 64 MiB
 * 1: AFP27 with 2 RAM chips of 64 MiB each (128MB)
 */
static int get_num_ram_bank(void)
{
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE;
	int nr_dram_banks = 1;

	if ((get_board_rev() > 0) && (CONFIG_NR_DRAM_BANKS > 1))
		nr_dram_banks += readl(&iim->bank[1].fuse_regs[9]) & 0x01;
	else
		nr_dram_banks = CONFIG_NR_DRAM_POPULATED;

	return nr_dram_banks;
}

static void apf27_port_init(int port, u32 gpio_dr, u32 ocr1, u32 ocr2,
			    u32 iconfa1, u32 iconfa2, u32 iconfb1, u32 iconfb2,
			    u32 icr1, u32 icr2, u32 imr, u32 gpio_dir, u32 gpr,
			    u32 puen, u32 gius)
{
	struct gpio_port_regs *regs = (struct gpio_port_regs *)IMX_GPIO_BASE;

	writel(gpio_dr,   &regs->port[port].gpio_dr);
	writel(ocr1,      &regs->port[port].ocr1);
	writel(ocr2,      &regs->port[port].ocr2);
	writel(iconfa1,   &regs->port[port].iconfa1);
	writel(iconfa2,   &regs->port[port].iconfa2);
	writel(iconfb1,   &regs->port[port].iconfb1);
	writel(iconfb2,   &regs->port[port].iconfb2);
	writel(icr1,      &regs->port[port].icr1);
	writel(icr2,      &regs->port[port].icr2);
	writel(imr,       &regs->port[port].imr);
	writel(gpio_dir,  &regs->port[port].gpio_dir);
	writel(gpr,       &regs->port[port].gpr);
	writel(puen,      &regs->port[port].puen);
	writel(gius,      &regs->port[port].gius);
}

#define APF27_PORT_INIT(n) apf27_port_init(PORT##n, ACFG_DR_##n##_VAL,	  \
	ACFG_OCR1_##n##_VAL, ACFG_OCR2_##n##_VAL, ACFG_ICFA1_##n##_VAL,	  \
	ACFG_ICFA2_##n##_VAL, ACFG_ICFB1_##n##_VAL, ACFG_ICFB2_##n##_VAL, \
	ACFG_ICR1_##n##_VAL, ACFG_ICR2_##n##_VAL, ACFG_IMR_##n##_VAL,	  \
	ACFG_DDIR_##n##_VAL, ACFG_GPR_##n##_VAL, ACFG_PUEN_##n##_VAL,	  \
	ACFG_GIUS_##n##_VAL)

static void apf27_iomux_init(void)
{
	APF27_PORT_INIT(A);
	APF27_PORT_INIT(B);
	APF27_PORT_INIT(C);
	APF27_PORT_INIT(D);
	APF27_PORT_INIT(E);
	APF27_PORT_INIT(F);
}

static int apf27_devices_init(void)
{
	int i;
	unsigned int mode[] = {
		PC5_PF_I2C2_DATA,
		PC6_PF_I2C2_CLK,
		PD17_PF_I2C_DATA,
		PD18_PF_I2C_CLK,
	};

	for (i = 0; i < ARRAY_SIZE(mode); i++)
		imx_gpio_mode(mode[i]);

#ifdef CONFIG_MXC_UART
	mx27_uart1_init_pins();
#endif

#ifdef CONFIG_FEC_MXC
	mx27_fec_init_pins();
#endif

#ifdef CONFIG_MMC_MXC
	mx27_sd2_init_pins();
	imx_gpio_mode((GPIO_PORTF | GPIO_OUT | GPIO_PUEN | GPIO_GPIO | 16));
	gpio_request(PC_PWRON, "pc_pwron");
	gpio_set_value(PC_PWRON, 1);
#endif
	return 0;
}

static void apf27_setup_csx(void)
{
	struct weim_regs *weim = (struct weim_regs *)IMX_WEIM_BASE;

	writel(ACFG_CS0U_VAL, &weim->cs0u);
	writel(ACFG_CS0L_VAL, &weim->cs0l);
	writel(ACFG_CS0A_VAL, &weim->cs0a);

	writel(ACFG_CS1U_VAL, &weim->cs1u);
	writel(ACFG_CS1L_VAL, &weim->cs1l);
	writel(ACFG_CS1A_VAL, &weim->cs1a);

	writel(ACFG_CS2U_VAL, &weim->cs2u);
	writel(ACFG_CS2L_VAL, &weim->cs2l);
	writel(ACFG_CS2A_VAL, &weim->cs2a);

	writel(ACFG_CS3U_VAL, &weim->cs3u);
	writel(ACFG_CS3L_VAL, &weim->cs3l);
	writel(ACFG_CS3A_VAL, &weim->cs3a);

	writel(ACFG_CS4U_VAL, &weim->cs4u);
	writel(ACFG_CS4L_VAL, &weim->cs4l);
	writel(ACFG_CS4A_VAL, &weim->cs4a);

	writel(ACFG_CS5U_VAL, &weim->cs5u);
	writel(ACFG_CS5L_VAL, &weim->cs5l);
	writel(ACFG_CS5A_VAL, &weim->cs5a);

	writel(ACFG_EIM_VAL, &weim->eim);
}

static void apf27_setup_port(void)
{
	struct system_control_regs *system =
		(struct system_control_regs *)IMX_SYSTEM_CTL_BASE;

	writel(ACFG_FMCR_VAL, &system->fmcr);
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	apf27_setup_csx();
	apf27_setup_port();
	apf27_iomux_init();
	apf27_devices_init();
#if defined(CONFIG_FPGA)
	APF27_init_fpga();
#endif


	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	if (get_num_ram_bank() > 1)
		gd->ram_size += get_ram_size((void *)PHYS_SDRAM_2,
					     PHYS_SDRAM_2_SIZE);

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size  = get_ram_size((void *)PHYS_SDRAM_1,
						PHYS_SDRAM_1_SIZE);
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	if (get_num_ram_bank() > 1)
		gd->bd->bi_dram[1].size = get_ram_size((void *)PHYS_SDRAM_2,
					     PHYS_SDRAM_2_SIZE);
	else
		gd->bd->bi_dram[1].size = 0;

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	ulong ramtop;

	if (get_num_ram_bank() > 1)
		ramtop = PHYS_SDRAM_2 + get_ram_size((void *)PHYS_SDRAM_2,
						     PHYS_SDRAM_2_SIZE);
	else
		ramtop = PHYS_SDRAM_1 + get_ram_size((void *)PHYS_SDRAM_1,
						     PHYS_SDRAM_1_SIZE);

	return ramtop;
}

int checkboard(void)
{
	printf("Board: Armadeus APF27 revision %d\n", get_board_rev());
	return 0;
}

#ifdef CONFIG_SPL_BUILD
inline void hang(void)
{
	for (;;)
		;
}

void board_init_f(ulong bootflag)
{
	/*
	 * copy ourselves from where we are running to where we were
	 * linked at. Use ulong pointers as all addresses involved
	 * are 4-byte-aligned.
	 */
	ulong *start_ptr, *end_ptr, *link_ptr, *run_ptr, *dst;
	asm volatile ("ldr %0, =_start" : "=r"(start_ptr));
	asm volatile ("ldr %0, =_end" : "=r"(end_ptr));
	asm volatile ("ldr %0, =board_init_f" : "=r"(link_ptr));
	asm volatile ("adr %0, board_init_f" : "=r"(run_ptr));
	for (dst = start_ptr; dst < end_ptr; dst++)
		*dst = *(dst+(run_ptr-link_ptr));

	/*
	 * branch to nand_boot's link-time address.
	 */
	asm volatile("ldr pc, =nand_boot");
}
#endif /* CONFIG_SPL_BUILD */
