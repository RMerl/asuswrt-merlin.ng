// SPDX-License-Identifier: GPL-2.0+
/*
 *Copyright (c) 2018 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <init.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/grf_rk3308.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/gpio.h>
#include <debug_uart.h>
#include <linux/bitops.h>

#include <asm/armv8/mmu.h>
static struct mm_region rk3308_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xff000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xff000000UL,
		.phys = 0xff000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3308_mem_map;

#define GRF_BASE	0xff000000
#define SGRF_BASE	0xff2b0000

enum {
	GPIO1C7_SHIFT		= 8,
	GPIO1C7_MASK		= GENMASK(11, 8),
	GPIO1C7_GPIO		= 0,
	GPIO1C7_UART1_RTSN,
	GPIO1C7_UART2_TX_M0,
	GPIO1C7_SPI2_MOSI,
	GPIO1C7_JTAG_TMS,

	GPIO1C6_SHIFT		= 4,
	GPIO1C6_MASK		= GENMASK(7, 4),
	GPIO1C6_GPIO		= 0,
	GPIO1C6_UART1_CTSN,
	GPIO1C6_UART2_RX_M0,
	GPIO1C6_SPI2_MISO,
	GPIO1C6_JTAG_TCLK,

	GPIO4D3_SHIFT           = 6,
	GPIO4D3_MASK            = GENMASK(7, 6),
	GPIO4D3_GPIO            = 0,
	GPIO4D3_SDMMC_D3,
	GPIO4D3_UART2_TX_M1,

	GPIO4D2_SHIFT           = 4,
	GPIO4D2_MASK            = GENMASK(5, 4),
	GPIO4D2_GPIO            = 0,
	GPIO4D2_SDMMC_D2,
	GPIO4D2_UART2_RX_M1,

	UART2_IO_SEL_SHIFT	= 2,
	UART2_IO_SEL_MASK	= GENMASK(3, 2),
	UART2_IO_SEL_M0		= 0,
	UART2_IO_SEL_M1,
	UART2_IO_SEL_USB,

	GPIO2C0_SEL_SRC_CTRL_SHIFT	= 11,
	GPIO2C0_SEL_SRC_CTRL_MASK	= BIT(11),
	GPIO2C0_SEL_SRC_CTRL_IOMUX	= 0,
	GPIO2C0_SEL_SRC_CTRL_SEL_PLUS,

	GPIO3B3_SEL_SRC_CTRL_SHIFT	= 7,
	GPIO3B3_SEL_SRC_CTRL_MASK	= BIT(7),
	GPIO3B3_SEL_SRC_CTRL_IOMUX	= 0,
	GPIO3B3_SEL_SRC_CTRL_SEL_PLUS,

	GPIO3B3_SEL_PLUS_SHIFT		= 4,
	GPIO3B3_SEL_PLUS_MASK		= GENMASK(6, 4),
	GPIO3B3_SEL_PLUS_GPIO3_B3	= 0,
	GPIO3B3_SEL_PLUS_FLASH_ALE,
	GPIO3B3_SEL_PLUS_EMMC_PWREN,
	GPIO3B3_SEL_PLUS_SPI1_CLK,
	GPIO3B3_SEL_PLUS_LCDC_D23_M1,

	GPIO3B2_SEL_SRC_CTRL_SHIFT	= 3,
	GPIO3B2_SEL_SRC_CTRL_MASK	= BIT(3),
	GPIO3B2_SEL_SRC_CTRL_IOMUX	= 0,
	GPIO3B2_SEL_SRC_CTRL_SEL_PLUS,

	GPIO3B2_SEL_PLUS_SHIFT		= 0,
	GPIO3B2_SEL_PLUS_MASK		= GENMASK(2, 0),
	GPIO3B2_SEL_PLUS_GPIO3_B2	= 0,
	GPIO3B2_SEL_PLUS_FLASH_RDN,
	GPIO3B2_SEL_PLUS_EMMC_RSTN,
	GPIO3B2_SEL_PLUS_SPI1_MISO,
	GPIO3B2_SEL_PLUS_LCDC_D22_M1,

	I2C3_IOFUNC_SRC_CTRL_SHIFT	= 10,
	I2C3_IOFUNC_SRC_CTRL_MASK	= BIT(10),
	I2C3_IOFUNC_SRC_CTRL_SEL_PLUS	= 1,

	GPIO2A3_SEL_SRC_CTRL_SHIFT	= 7,
	GPIO2A3_SEL_SRC_CTRL_MASK	= BIT(7),
	GPIO2A3_SEL_SRC_CTRL_SEL_PLUS	= 1,

	GPIO2A2_SEL_SRC_CTRL_SHIFT	= 3,
	GPIO2A2_SEL_SRC_CTRL_MASK	= BIT(3),
	GPIO2A2_SEL_SRC_CTRL_SEL_PLUS	= 1,
};

enum {
	IOVSEL3_CTRL_SHIFT	= 8,
	IOVSEL3_CTRL_MASK	= BIT(8),
	VCCIO3_SEL_BY_GPIO	= 0,
	VCCIO3_SEL_BY_IOVSEL3,

	IOVSEL3_SHIFT		= 3,
	IOVSEL3_MASK		= BIT(3),
	VCCIO3_3V3		= 0,
	VCCIO3_1V8,
};

/*
 * The voltage of VCCIO3(which is the voltage domain of emmc/flash/sfc
 * interface) can indicated by GPIO0_A4 or io_vsel3. The SOC defaults
 * use GPIO0_A4 to indicate power supply voltage for VCCIO3 by hardware,
 * then we can switch to io_vsel3 after system power on, and release GPIO0_A4
 * for other usage.
 */

#define GPIO0_A4	4

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ff490000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ff480000",
};

int rk_board_init(void)
{
	static struct rk3308_grf * const grf = (void *)GRF_BASE;
	u32 val;
	int ret;

	ret = gpio_request(GPIO0_A4, "gpio0_a4");
	if (ret < 0) {
		printf("request for gpio0_a4 failed:%d\n", ret);
		return 0;
	}

	gpio_direction_input(GPIO0_A4);

	if (gpio_get_value(GPIO0_A4))
		val = VCCIO3_SEL_BY_IOVSEL3 << IOVSEL3_CTRL_SHIFT |
		      VCCIO3_1V8 << IOVSEL3_SHIFT;
	else
		val = VCCIO3_SEL_BY_IOVSEL3 << IOVSEL3_CTRL_SHIFT |
		      VCCIO3_3V3 << IOVSEL3_SHIFT;
	rk_clrsetreg(&grf->soc_con0, IOVSEL3_CTRL_MASK | IOVSEL3_MASK, val);

	gpio_free(GPIO0_A4);
	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
__weak void board_debug_uart_init(void)
{
	static struct rk3308_grf * const grf = (void *)GRF_BASE;

	/* Enable early UART2 channel m1 on the rk3308 */
	rk_clrsetreg(&grf->soc_con5, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M1 << UART2_IO_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio4d_iomux,
		     GPIO4D3_MASK | GPIO4D2_MASK,
		     GPIO4D2_UART2_RX_M1 << GPIO4D2_SHIFT |
		     GPIO4D3_UART2_TX_M1 << GPIO4D3_SHIFT);
}
#endif

#if defined(CONFIG_SPL_BUILD)
int arch_cpu_init(void)
{
	static struct rk3308_sgrf * const sgrf = (void *)SGRF_BASE;
	static struct rk3308_grf * const grf = (void *)GRF_BASE;

	/* Set CRYPTO SDMMC EMMC NAND SFC USB master bus to be secure access */
	rk_clrreg(&sgrf->con_secure0, 0x2b83);

	/*
	 * Enable plus options to use more pinctrl functions, including
	 * GPIO2A2_PLUS, GPIO2A3_PLUS and I2C3_MULTI_SRC_PLUS.
	 */
	rk_clrsetreg(&grf->soc_con13,
		     I2C3_IOFUNC_SRC_CTRL_MASK | GPIO2A3_SEL_SRC_CTRL_MASK |
		     GPIO2A2_SEL_SRC_CTRL_MASK,
		     I2C3_IOFUNC_SRC_CTRL_SEL_PLUS << I2C3_IOFUNC_SRC_CTRL_SHIFT |
		     GPIO2A3_SEL_SRC_CTRL_SEL_PLUS << GPIO2A3_SEL_SRC_CTRL_SHIFT |
		     GPIO2A2_SEL_SRC_CTRL_SEL_PLUS << GPIO2A2_SEL_SRC_CTRL_SHIFT);

	/* Plus options about GPIO3B2_PLUS, GPIO3B3_PLUS and GPIO2C0_PLUS. */
	rk_clrsetreg(&grf->soc_con15,
		     GPIO2C0_SEL_SRC_CTRL_MASK | GPIO3B3_SEL_SRC_CTRL_MASK |
		     GPIO3B2_SEL_SRC_CTRL_MASK,
		     GPIO2C0_SEL_SRC_CTRL_SEL_PLUS << GPIO2C0_SEL_SRC_CTRL_SHIFT |
		     GPIO3B3_SEL_SRC_CTRL_SEL_PLUS << GPIO3B3_SEL_SRC_CTRL_SHIFT |
		     GPIO3B2_SEL_SRC_CTRL_SEL_PLUS << GPIO3B2_SEL_SRC_CTRL_SHIFT);

	return 0;
}
#endif
