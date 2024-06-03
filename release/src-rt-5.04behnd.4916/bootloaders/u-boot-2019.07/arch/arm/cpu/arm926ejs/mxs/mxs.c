// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX23/i.MX28 common code
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/mach-imx/dma.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

/* Lowlevel init isn't used on i.MX28, so just have a dummy here */
__weak void lowlevel_init(void) {}

void reset_cpu(ulong ignored) __attribute__((noreturn));

void reset_cpu(ulong ignored)
{
	struct mxs_rtc_regs *rtc_regs =
		(struct mxs_rtc_regs *)MXS_RTC_BASE;
	struct mxs_lcdif_regs *lcdif_regs =
		(struct mxs_lcdif_regs *)MXS_LCDIF_BASE;

	/*
	 * Shut down the LCD controller as it interferes with BootROM boot mode
	 * pads sampling.
	 */
	writel(LCDIF_CTRL_RUN, &lcdif_regs->hw_lcdif_ctrl_clr);

	/* Wait 1 uS before doing the actual watchdog reset */
	writel(1, &rtc_regs->hw_rtc_watchdog);
	writel(RTC_CTRL_WATCHDOGEN, &rtc_regs->hw_rtc_ctrl_set);

	/* Endless loop, reset will exit from here */
	for (;;)
		;
}

/*
 * This function will craft a jumptable at 0x0 which will redirect interrupt
 * vectoring to proper location of U-Boot in RAM.
 *
 * The structure of the jumptable will be as follows:
 *  ldr pc, [pc, #0x18] ..... for each vector, thus repeated 8 times
 *  <destination address> ... for each previous ldr, thus also repeated 8 times
 *
 * The "ldr pc, [pc, #0x18]" instruction above loads address from memory at
 * offset 0x18 from current value of PC register. Note that PC is already
 * incremented by 4 when computing the offset, so the effective offset is
 * actually 0x20, this the associated <destination address>. Loading the PC
 * register with an address performs a jump to that address.
 */
void mx28_fixup_vt(uint32_t start_addr)
{
	/* ldr pc, [pc, #0x18] */
	const uint32_t ldr_pc = 0xe59ff018;
	/* Jumptable location is 0x0 */
	uint32_t *vt = (uint32_t *)0x0;
	int i;

	for (i = 0; i < 8; i++) {
		/* cppcheck-suppress nullPointer */
		vt[i] = ldr_pc;
		/* cppcheck-suppress nullPointer */
		vt[i + 8] = start_addr + (4 * i);
	}
}

#ifdef	CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	mx28_fixup_vt(gd->relocaddr);
	return 0;
}
#endif

int arch_cpu_init(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	extern uint32_t _start;

	mx28_fixup_vt((uint32_t)&_start);

	/*
	 * Enable NAND clock
	 */
	/* Clear bypass bit */
	writel(CLKCTRL_CLKSEQ_BYPASS_GPMI,
		&clkctrl_regs->hw_clkctrl_clkseq_set);

	/* Set GPMI clock to ref_gpmi / 12 */
	clrsetbits_le32(&clkctrl_regs->hw_clkctrl_gpmi,
		CLKCTRL_GPMI_CLKGATE | CLKCTRL_GPMI_DIV_MASK, 1);

	udelay(1000);

	/*
	 * Configure GPIO unit
	 */
	mxs_gpio_init();

#ifdef	CONFIG_APBH_DMA
	/* Start APBH DMA */
	mxs_dma_init();
#endif

	return 0;
}

u32 get_cpu_rev(void)
{
	struct mxs_digctl_regs *digctl_regs =
		(struct mxs_digctl_regs *)MXS_DIGCTL_BASE;
	uint8_t rev = readl(&digctl_regs->hw_digctl_chipid) & 0x000000FF;

	switch (readl(&digctl_regs->hw_digctl_chipid) & HW_DIGCTL_CHIPID_MASK) {
	case HW_DIGCTL_CHIPID_MX23:
		switch (rev) {
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
			return (MXC_CPU_MX23 << 12) | (rev + 0x10);
		default:
			return 0;
		}
	case HW_DIGCTL_CHIPID_MX28:
		switch (rev) {
		case 0x1:
			return (MXC_CPU_MX28 << 12) | 0x12;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

#if defined(CONFIG_DISPLAY_CPUINFO)
const char *get_imx_type(u32 imxtype)
{
	switch (imxtype) {
	case MXC_CPU_MX23:
		return "23";
	case MXC_CPU_MX28:
		return "28";
	default:
		return "??";
	}
}

int print_cpuinfo(void)
{
	u32 cpurev;
	struct mxs_spl_data *data = MXS_SPL_DATA;

	cpurev = get_cpu_rev();
	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
		get_imx_type((cpurev & 0xFF000) >> 12),
		(cpurev & 0x000F0) >> 4,
		(cpurev & 0x0000F) >> 0,
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("BOOT:  %s\n", mxs_boot_modes[data->boot_mode_idx].mode);
	return 0;
}
#endif

int do_mx28_showclocks(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printf("CPU:   %3d MHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("BUS:   %3d MHz\n", mxc_get_clock(MXC_AHB_CLK) / 1000000);
	printf("EMI:   %3d MHz\n", mxc_get_clock(MXC_EMI_CLK));
	printf("GPMI:  %3d MHz\n", mxc_get_clock(MXC_GPMI_CLK) / 1000000);
	return 0;
}

/*
 * Initializes on-chip ethernet controllers.
 */
#if defined(CONFIG_MX28) && defined(CONFIG_CMD_NET)
int cpu_eth_init(bd_t *bis)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	/* Turn on ENET clocks */
	clrbits_le32(&clkctrl_regs->hw_clkctrl_enet,
		CLKCTRL_ENET_SLEEP | CLKCTRL_ENET_DISABLE);

	/* Set up ENET PLL for 50 MHz */
	/* Power on ENET PLL */
	writel(CLKCTRL_PLL2CTRL0_POWER,
		&clkctrl_regs->hw_clkctrl_pll2ctrl0_set);

	udelay(10);

	/* Gate on ENET PLL */
	writel(CLKCTRL_PLL2CTRL0_CLKGATE,
		&clkctrl_regs->hw_clkctrl_pll2ctrl0_clr);

	/* Enable pad output */
	setbits_le32(&clkctrl_regs->hw_clkctrl_enet, CLKCTRL_ENET_CLK_OUT_EN);

	return 0;
}
#endif

__weak void mx28_adjust_mac(int dev_id, unsigned char *mac)
{
	mac[0] = 0x00;
	mac[1] = 0x04; /* Use FSL vendor MAC address by default */

	if (dev_id == 1) /* Let MAC1 be MAC0 + 1 by default */
		mac[5] += 1;
}

#ifdef	CONFIG_MX28_FEC_MAC_IN_OCOTP

#define	MXS_OCOTP_MAX_TIMEOUT	1000000
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	struct mxs_ocotp_regs *ocotp_regs =
		(struct mxs_ocotp_regs *)MXS_OCOTP_BASE;
	uint32_t data;

	memset(mac, 0, 6);

	writel(OCOTP_CTRL_RD_BANK_OPEN, &ocotp_regs->hw_ocotp_ctrl_set);

	if (mxs_wait_mask_clr(&ocotp_regs->hw_ocotp_ctrl_reg, OCOTP_CTRL_BUSY,
				MXS_OCOTP_MAX_TIMEOUT)) {
		printf("MXS FEC: Can't get MAC from OCOTP\n");
		return;
	}

	data = readl(&ocotp_regs->hw_ocotp_cust0);

	mac[2] = (data >> 24) & 0xff;
	mac[3] = (data >> 16) & 0xff;
	mac[4] = (data >> 8) & 0xff;
	mac[5] = data & 0xff;
	mx28_adjust_mac(dev_id, mac);
}
#else
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	memset(mac, 0, 6);
}
#endif

int mxs_dram_init(void)
{
	struct mxs_spl_data *data = MXS_SPL_DATA;

	if (data->mem_dram_size == 0) {
		printf("MXS:\n"
			"Error, the RAM size passed up from SPL is 0!\n");
		hang();
	}

	gd->ram_size = data->mem_dram_size;
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx28_showclocks,
	"display clocks",
	""
);
