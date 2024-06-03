// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Linaro
 * Peter Griffin <peter.griffin@linaro.org>
 */
#include <common.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include <errno.h>
#include <malloc.h>
#include <netdev.h>
#include <asm/io.h>
#include <usb.h>
#include <power/hi6553_pmic.h>
#include <asm-generic/gpio.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/periph.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/hi6220.h>
#include <asm/armv8/mmu.h>

/*TODO drop this table in favour of device tree */
static const struct hikey_gpio_platdata hi6220_gpio[] = {
	{ 0, HI6220_GPIO_BASE(0)},
	{ 1, HI6220_GPIO_BASE(1)},
	{ 2, HI6220_GPIO_BASE(2)},
	{ 3, HI6220_GPIO_BASE(3)},
	{ 4, HI6220_GPIO_BASE(4)},
	{ 5, HI6220_GPIO_BASE(5)},
	{ 6, HI6220_GPIO_BASE(6)},
	{ 7, HI6220_GPIO_BASE(7)},
	{ 8, HI6220_GPIO_BASE(8)},
	{ 9, HI6220_GPIO_BASE(9)},
	{ 10, HI6220_GPIO_BASE(10)},
	{ 11, HI6220_GPIO_BASE(11)},
	{ 12, HI6220_GPIO_BASE(12)},
	{ 13, HI6220_GPIO_BASE(13)},
	{ 14, HI6220_GPIO_BASE(14)},
	{ 15, HI6220_GPIO_BASE(15)},
	{ 16, HI6220_GPIO_BASE(16)},
	{ 17, HI6220_GPIO_BASE(17)},
	{ 18, HI6220_GPIO_BASE(18)},
	{ 19, HI6220_GPIO_BASE(19)},

};

U_BOOT_DEVICES(hi6220_gpios) = {
	{ "gpio_hi6220", &hi6220_gpio[0] },
	{ "gpio_hi6220", &hi6220_gpio[1] },
	{ "gpio_hi6220", &hi6220_gpio[2] },
	{ "gpio_hi6220", &hi6220_gpio[3] },
	{ "gpio_hi6220", &hi6220_gpio[4] },
	{ "gpio_hi6220", &hi6220_gpio[5] },
	{ "gpio_hi6220", &hi6220_gpio[6] },
	{ "gpio_hi6220", &hi6220_gpio[7] },
	{ "gpio_hi6220", &hi6220_gpio[8] },
	{ "gpio_hi6220", &hi6220_gpio[9] },
	{ "gpio_hi6220", &hi6220_gpio[10] },
	{ "gpio_hi6220", &hi6220_gpio[11] },
	{ "gpio_hi6220", &hi6220_gpio[12] },
	{ "gpio_hi6220", &hi6220_gpio[13] },
	{ "gpio_hi6220", &hi6220_gpio[14] },
	{ "gpio_hi6220", &hi6220_gpio[15] },
	{ "gpio_hi6220", &hi6220_gpio[16] },
	{ "gpio_hi6220", &hi6220_gpio[17] },
	{ "gpio_hi6220", &hi6220_gpio[18] },
	{ "gpio_hi6220", &hi6220_gpio[19] },
};

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)

static const struct pl01x_serial_platdata serial_platdata = {
#if CONFIG_CONS_INDEX == 1
	.base = HI6220_UART0_BASE,
#elif CONFIG_CONS_INDEX == 4
	.base = HI6220_UART3_BASE,
#else
#error "Unsupported console index value."
#endif
	.type = TYPE_PL011,
	.clock = 19200000
};

U_BOOT_DEVICE(hikey_seriala) = {
	.name = "serial_pl01x",
	.platdata = &serial_platdata,
};
#endif

static struct mm_region hikey_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = hikey_mem_map;

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_uart_init(void)
{
	switch (CONFIG_CONS_INDEX) {
	case 1:
		hi6220_pinmux_config(PERIPH_ID_UART0);
		break;
	case 4:
		hi6220_pinmux_config(PERIPH_ID_UART3);
		break;
	default:
		debug("%s: Unsupported UART selected\n", __func__);
		return -1;
	}

	return 0;
}

int board_early_init_f(void)
{
	board_uart_init();
	return 0;
}
#endif

struct peri_sc_periph_regs *peri_sc =
	(struct peri_sc_periph_regs *)HI6220_PERI_BASE;

struct alwayson_sc_regs *ao_sc =
	(struct alwayson_sc_regs *)ALWAYSON_CTRL_BASE;

/* status offset from enable reg */
#define STAT_EN_OFF 0x2

void hi6220_clk_enable(u32 bitfield, unsigned int *clk_base)
{
	uint32_t data;

	data = readl(clk_base);
	data |= bitfield;

	writel(bitfield, clk_base);
	do {
		data = readl(clk_base + STAT_EN_OFF);
	} while ((data & bitfield) == 0);
}

/* status offset from disable reg */
#define STAT_DIS_OFF 0x1

void hi6220_clk_disable(u32 bitfield, unsigned int *clk_base)
{
	uint32_t data;

	data = readl(clk_base);
	data |= bitfield;

	writel(data, clk_base);
	do {
		data = readl(clk_base + STAT_DIS_OFF);
	} while (data & bitfield);
}

#define EYE_PATTERN	0x70533483

int board_usb_init(int index, enum usb_init_type init)
{
	unsigned int data;

	/* enable USB clock */
	hi6220_clk_enable(PERI_CLK0_USBOTG, &peri_sc->clk0_en);

	/* take usb IPs out of reset */
	writel(PERI_RST0_USBOTG_BUS | PERI_RST0_POR_PICOPHY |
		PERI_RST0_USBOTG | PERI_RST0_USBOTG_32K,
		&peri_sc->rst0_dis);
	do {
		data = readl(&peri_sc->rst0_stat);
		data &= PERI_RST0_USBOTG_BUS | PERI_RST0_POR_PICOPHY |
			PERI_RST0_USBOTG | PERI_RST0_USBOTG_32K;
	} while (data);

	/*CTRL 5*/
	data = readl(&peri_sc->ctrl5);
	data &= ~PERI_CTRL5_PICOPHY_BC_MODE;
	data |= PERI_CTRL5_USBOTG_RES_SEL | PERI_CTRL5_PICOPHY_ACAENB;
	data |= 0x300;
	writel(data, &peri_sc->ctrl5);

	/*CTRL 4*/

	/* configure USB PHY */
	data = readl(&peri_sc->ctrl4);

	/* make PHY out of low power mode */
	data &= ~PERI_CTRL4_PICO_SIDDQ;
	data &= ~PERI_CTRL4_PICO_OGDISABLE;
	data |= PERI_CTRL4_PICO_VBUSVLDEXTSEL | PERI_CTRL4_PICO_VBUSVLDEXT;
	writel(data, &peri_sc->ctrl4);

	writel(EYE_PATTERN, &peri_sc->ctrl8);

	mdelay(5);
	return 0;
}

static int config_sd_carddetect(void)
{
	int ret;

	/* configure GPIO8 as nopull */
	writel(0, 0xf8001830);

	gpio_request(8, "SD CD");

	gpio_direction_input(8);
	ret = gpio_get_value(8);

	if (!ret) {
		printf("%s: SD card present\n", __func__);
		return 1;
	}

	printf("%s: SD card not present\n", __func__);
	return 0;
}


static void mmc1_init_pll(void)
{
	uint32_t data;

	/* select SYSPLL as the source of MMC1 */
	/* select SYSPLL as the source of MUX1 (SC_CLK_SEL0) */
	writel(1 << 11 | 1 << 27, &peri_sc->clk0_sel);
	do {
		data = readl(&peri_sc->clk0_sel);
	} while (!(data & (1 << 11)));

	/* select MUX1 as the source of MUX2 (SC_CLK_SEL0) */
	writel(1 << 30, &peri_sc->clk0_sel);
	do {
		data = readl(&peri_sc->clk0_sel);
	} while (data & (1 << 14));

	hi6220_clk_enable(PERI_CLK0_MMC1, &peri_sc->clk0_en);

	hi6220_clk_enable(PERI_CLK12_MMC1_SRC, &peri_sc->clk12_en);

	do {
		/* 1.2GHz / 50 = 24MHz */
		writel(0x31 | (1 << 7), &peri_sc->clkcfg8bit2);
		data = readl(&peri_sc->clkcfg8bit2);
	} while ((data & 0x31) != 0x31);
}

static void mmc1_reset_clk(void)
{
	unsigned int data;

	/* disable mmc1 bus clock */
	hi6220_clk_disable(PERI_CLK0_MMC1, &peri_sc->clk0_dis);

	/* enable mmc1 bus clock */
	hi6220_clk_enable(PERI_CLK0_MMC1, &peri_sc->clk0_en);

	/* reset mmc1 clock domain */
	writel(PERI_RST0_MMC1, &peri_sc->rst0_en);

	/* bypass mmc1 clock phase */
	data = readl(&peri_sc->ctrl2);
	data |= 3 << 2;
	writel(data, &peri_sc->ctrl2);

	/* disable low power */
	data = readl(&peri_sc->ctrl13);
	data |= 1 << 4;
	writel(data, &peri_sc->ctrl13);
	do {
		data = readl(&peri_sc->rst0_stat);
	} while (!(data & PERI_RST0_MMC1));

	/* unreset mmc1 clock domain */
	writel(PERI_RST0_MMC1, &peri_sc->rst0_dis);
	do {
		data = readl(&peri_sc->rst0_stat);
	} while (data & PERI_RST0_MMC1);
}

static void mmc0_reset_clk(void)
{
	unsigned int data;

	/* disable mmc0 bus clock */
	hi6220_clk_disable(PERI_CLK0_MMC0, &peri_sc->clk0_dis);

	/* enable mmc0 bus clock */
	hi6220_clk_enable(PERI_CLK0_MMC0, &peri_sc->clk0_en);

	/* reset mmc0 clock domain */
	writel(PERI_RST0_MMC0, &peri_sc->rst0_en);

	/* bypass mmc0 clock phase */
	data = readl(&peri_sc->ctrl2);
	data |= 3;
	writel(data, &peri_sc->ctrl2);

	/* disable low power */
	data = readl(&peri_sc->ctrl13);
	data |= 1 << 3;
	writel(data, &peri_sc->ctrl13);
	do {
		data = readl(&peri_sc->rst0_stat);
	} while (!(data & PERI_RST0_MMC0));

	/* unreset mmc0 clock domain */
	writel(PERI_RST0_MMC0, &peri_sc->rst0_dis);
	do {
		data = readl(&peri_sc->rst0_stat);
	} while (data & PERI_RST0_MMC0);
}


/* PMU SSI is the IP that maps the external PMU hi6553 registers as IO */
static void hi6220_pmussi_init(void)
{
	uint32_t data;

	/* Take PMUSSI out of reset */
	writel(ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_PMUSSI_N,
	       &ao_sc->rst4_dis);
	do {
		data = readl(&ao_sc->rst4_stat);
	} while (data & ALWAYSON_SC_PERIPH_RST4_DIS_PRESET_PMUSSI_N);

	/* set PMU SSI clock latency for read operation */
	data = readl(&ao_sc->mcu_subsys_ctrl3);
	data &= ~ALWAYSON_SC_MCU_SUBSYS_CTRL3_RCLK_MASK;
	data |= ALWAYSON_SC_MCU_SUBSYS_CTRL3_RCLK_3;
	writel(data, &ao_sc->mcu_subsys_ctrl3);

	/* enable PMUSSI clock */
	data = ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_PMUSSI_CCPU |
	       ALWAYSON_SC_PERIPH_CLK5_EN_PCLK_PMUSSI_MCU;

	hi6220_clk_enable(data, &ao_sc->clk5_en);

	/* Output high to PMIC on PWR_HOLD_GPIO0_0 */
	gpio_request(0, "PWR_HOLD_GPIO0_0");
	gpio_direction_output(0, 1);
}

int misc_init_r(void)
{
	return 0;
}

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_MMC

static int init_dwmmc(void)
{
	int ret = 0;

#ifdef CONFIG_MMC_DW

	/* mmc0 pll is already configured by ATF */
	mmc0_reset_clk();
	ret = hi6220_pinmux_config(PERIPH_ID_SDMMC0);
	if (ret)
		printf("%s: Error configuring pinmux for eMMC (%d)\n"
			, __func__, ret);

	ret |= hi6220_dwmci_add_port(0, HI6220_MMC0_BASE, 8);
	if (ret)
		printf("%s: Error adding eMMC port (%d)\n", __func__, ret);


	/* take mmc1 (sd slot) out of reset, configure clocks and pinmuxing */
	mmc1_init_pll();
	mmc1_reset_clk();

	ret |= hi6220_pinmux_config(PERIPH_ID_SDMMC1);
	if (ret)
		printf("%s: Error configuring pinmux for eMMC (%d)\n"
			, __func__, ret);

	config_sd_carddetect();

	ret |= hi6220_dwmci_add_port(1, HI6220_MMC1_BASE, 4);
	if (ret)
		printf("%s: Error adding SD port (%d)\n", __func__, ret);

#endif
	return ret;
}

/* setup board specific PMIC */
int power_init_board(void)
{
	/* init the hi6220 pmussi ip */
	hi6220_pmussi_init();

	power_hi6553_init((u8 *)HI6220_PMUSSI_BASE);

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	int ret;

	/* add the eMMC and sd ports */
	ret = init_dwmmc();

	if (ret)
		debug("init_dwmmc failed\n");

	return ret;
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	/*
	 * Reserve regions below from DT memory node (which gets generated
	 * by U-Boot from the dram banks in arch_fixup_fdt() before booting
	 * the kernel. This will then match the kernel hikey dts memory node.
	 *
	 *  0x05e0,0000 - 0x05ef,ffff: MCU firmware runtime using
	 *  0x05f0,1000 - 0x05f0,1fff: Reboot reason
	 *  0x06df,f000 - 0x06df,ffff: Mailbox message data
	 *  0x0740,f000 - 0x0740,ffff: MCU firmware section
	 *  0x21f0,0000 - 0x21ff,ffff: pstore/ramoops buffer
	 *  0x3e00,0000 - 0x3fff,ffff: OP-TEE
	*/

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = 0x05e00000;

	gd->bd->bi_dram[1].start = 0x05f00000;
	gd->bd->bi_dram[1].size = 0x00001000;

	gd->bd->bi_dram[2].start = 0x05f02000;
	gd->bd->bi_dram[2].size = 0x00efd000;

	gd->bd->bi_dram[3].start = 0x06e00000;
	gd->bd->bi_dram[3].size = 0x0060f000;

	gd->bd->bi_dram[4].start = 0x07410000;
	gd->bd->bi_dram[4].size = 0x1aaf0000;

	gd->bd->bi_dram[5].start = 0x22000000;
	gd->bd->bi_dram[5].size = 0x1c000000;

	return 0;
}

void reset_cpu(ulong addr)
{
	writel(0x48698284, &ao_sc->stat0);
	wfi();
}
