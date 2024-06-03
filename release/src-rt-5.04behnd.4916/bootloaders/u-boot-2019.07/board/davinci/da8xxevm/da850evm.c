// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Based on da830evm.c. Original Copyrights follow:
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc, Ltd. <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */

#include <common.h>
#include <dm.h>
#include <environment.h>
#include <i2c.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/davinci_nand.h>
#include <asm/arch/emac_defs.h>
#include <asm/arch/pinmux_defs.h>
#include <asm/io.h>
#include <asm/arch/davinci_misc.h>
#include <linux/errno.h>
#include <hwconfig.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>

#ifdef CONFIG_MMC_DAVINCI
#include <mmc.h>
#include <asm/arch/sdmmc_defs.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DRIVER_TI_EMAC
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
#define HAS_RMII 1
#else
#define HAS_RMII 0
#endif
#endif /* CONFIG_DRIVER_TI_EMAC */

#define CFG_MAC_ADDR_SPI_BUS	0
#define CFG_MAC_ADDR_SPI_CS	0
#define CFG_MAC_ADDR_SPI_MAX_HZ	CONFIG_SF_DEFAULT_SPEED
#define CFG_MAC_ADDR_SPI_MODE	SPI_MODE_3

#define CFG_MAC_ADDR_OFFSET	(flash->size - SZ_64K)

#ifdef CONFIG_MAC_ADDR_IN_SPIFLASH
static int get_mac_addr(u8 *addr)
{
	struct spi_flash *flash;
	int ret;

	flash = spi_flash_probe(CFG_MAC_ADDR_SPI_BUS, CFG_MAC_ADDR_SPI_CS,
			CFG_MAC_ADDR_SPI_MAX_HZ, CFG_MAC_ADDR_SPI_MODE);
	if (!flash) {
		printf("Error - unable to probe SPI flash.\n");
		return -1;
	}

	ret = spi_flash_read(flash, (CFG_MAC_ADDR_OFFSET), 6, addr);
	if (ret) {
		printf("Error - unable to read MAC address from SPI flash.\n");
		return -1;
	}

	return ret;
}
#endif

void dsp_lpsc_on(unsigned domain, unsigned int id)
{
	dv_reg_p mdstat, mdctl, ptstat, ptcmd;
	struct davinci_psc_regs *psc_regs;

	psc_regs = davinci_psc0_regs;
	mdstat = &psc_regs->psc0.mdstat[id];
	mdctl = &psc_regs->psc0.mdctl[id];
	ptstat = &psc_regs->ptstat;
	ptcmd = &psc_regs->ptcmd;

	while (*ptstat & (0x1 << domain))
		;

	if ((*mdstat & 0x1f) == 0x03)
		return;                 /* Already on and enabled */

	*mdctl |= 0x03;

	*ptcmd = 0x1 << domain;

	while (*ptstat & (0x1 << domain))
		;
	while ((*mdstat & 0x1f) != 0x03)
		;		/* Probably an overkill... */
}

static void dspwake(void)
{
	unsigned *resetvect = (unsigned *)DAVINCI_L3CBARAM_BASE;
	u32 val;

	/* if the device is ARM only, return */
	if ((readl(CHIP_REV_ID_REG) & 0x3f) == 0x10)
		return;

	if (hwconfig_subarg_cmp_f("dsp", "wake", "no", NULL))
		return;

	*resetvect++ = 0x1E000; /* DSP Idle */
	/* clear out the next 10 words as NOP */
	memset(resetvect, 0, sizeof(unsigned) *10);

	/* setup the DSP reset vector */
	writel(DAVINCI_L3CBARAM_BASE, HOST1CFG);

	dsp_lpsc_on(1, DAVINCI_LPSC_GEM);
	val = readl(PSC0_MDCTL + (15 * 4));
	val |= 0x100;
	writel(val, (PSC0_MDCTL + (15 * 4)));
}

int misc_init_r(void)
{
	dspwake();

#if defined(CONFIG_MAC_ADDR_IN_SPIFLASH) || defined(CONFIG_MAC_ADDR_IN_EEPROM)

	uchar env_enetaddr[6];
	int enetaddr_found;

	enetaddr_found = eth_env_get_enetaddr("ethaddr", env_enetaddr);

#endif

#ifdef CONFIG_MAC_ADDR_IN_SPIFLASH
	int spi_mac_read;
	uchar buff[6];

	spi_mac_read = get_mac_addr(buff);
	buff[0] = 0;

	/*
	 * MAC address not present in the environment
	 * try and read the MAC address from SPI flash
	 * and set it.
	 */
	if (!enetaddr_found) {
		if (!spi_mac_read) {
			if (is_valid_ethaddr(buff)) {
				if (eth_env_set_enetaddr("ethaddr", buff)) {
					printf("Warning: Failed to "
					"set MAC address from SPI flash\n");
				}
			} else {
					printf("Warning: Invalid "
					"MAC address read from SPI flash\n");
			}
		}
	} else {
		/*
		 * MAC address present in environment compare it with
		 * the MAC address in SPI flash and warn on mismatch
		 */
		if (!spi_mac_read && is_valid_ethaddr(buff) &&
		    memcmp(env_enetaddr, buff, 6))
			printf("Warning: MAC address in SPI flash don't match "
					"with the MAC address in the environment\n");
		printf("Default using MAC address from environment\n");
	}

#elif defined(CONFIG_MAC_ADDR_IN_EEPROM)
	uint8_t enetaddr[8];
	int eeprom_mac_read;

	/* Read Ethernet MAC address from EEPROM */
	eeprom_mac_read = dvevm_read_mac_address(enetaddr);

	/*
	 * MAC address not present in the environment
	 * try and read the MAC address from EEPROM flash
	 * and set it.
	 */
	if (!enetaddr_found) {
		if (eeprom_mac_read)
			/* Set Ethernet MAC address from EEPROM */
			davinci_sync_env_enetaddr(enetaddr);
	} else {
		/*
		 * MAC address present in environment compare it with
		 * the MAC address in EEPROM and warn on mismatch
		 */
		if (eeprom_mac_read && memcmp(enetaddr, env_enetaddr, 6))
			printf("Warning: MAC address in EEPROM don't match "
					"with the MAC address in the environment\n");
		printf("Default using MAC address from environment\n");
	}

#endif
	return 0;
}

static const struct pinmux_config gpio_pins[] = {
#ifdef CONFIG_USE_NOR
	/* GP0[11] is required for NOR to work on Rev 3 EVMs */
	{ pinmux(0), 8, 4 },	/* GP0[11] */
#endif
#ifdef CONFIG_MMC_DAVINCI
	/* GP0[11] is required for SD to work on Rev 3 EVMs */
	{ pinmux(0),  8, 4 },	/* GP0[11] */
#endif
};

const struct pinmux_resource pinmuxes[] = {
#ifdef CONFIG_DRIVER_TI_EMAC
	PINMUX_ITEM(emac_pins_mdio),
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	PINMUX_ITEM(emac_pins_rmii),
#else
	PINMUX_ITEM(emac_pins_mii),
#endif
#endif
#ifdef CONFIG_SPI_FLASH
	PINMUX_ITEM(spi1_pins_base),
	PINMUX_ITEM(spi1_pins_scs0),
#endif
	PINMUX_ITEM(uart2_pins_txrx),
	PINMUX_ITEM(uart2_pins_rtscts),
	PINMUX_ITEM(i2c0_pins),
#ifdef CONFIG_NAND_DAVINCI
	PINMUX_ITEM(emifa_pins_cs3),
	PINMUX_ITEM(emifa_pins_cs4),
	PINMUX_ITEM(emifa_pins_nand),
#elif defined(CONFIG_USE_NOR)
	PINMUX_ITEM(emifa_pins_cs2),
	PINMUX_ITEM(emifa_pins_nor),
#endif
	PINMUX_ITEM(gpio_pins),
#ifdef CONFIG_MMC_DAVINCI
	PINMUX_ITEM(mmc0_pins),
#endif
};

const int pinmuxes_size = ARRAY_SIZE(pinmuxes);

const struct lpsc_resource lpsc[] = {
	{ DAVINCI_LPSC_AEMIF },	/* NAND, NOR */
	{ DAVINCI_LPSC_SPI1 },	/* Serial Flash */
	{ DAVINCI_LPSC_EMAC },	/* image download */
	{ DAVINCI_LPSC_UART2 },	/* console */
	{ DAVINCI_LPSC_GPIO },
#ifdef CONFIG_MMC_DAVINCI
	{ DAVINCI_LPSC_MMC_SD },
#endif
};

const int lpsc_size = ARRAY_SIZE(lpsc);

#ifndef CONFIG_DA850_EVM_MAX_CPU_CLK
#define CONFIG_DA850_EVM_MAX_CPU_CLK	300000000
#endif

#define REV_AM18X_EVM		0x100

/*
 * get_board_rev() - setup to pass kernel board revision information
 * Returns:
 * bit[0-3]	Maximum cpu clock rate supported by onboard SoC
 *		0000b - 300 MHz
 *		0001b - 372 MHz
 *		0010b - 408 MHz
 *		0011b - 456 MHz
 */
u32 get_board_rev(void)
{
	char *s;
	u32 maxcpuclk = CONFIG_DA850_EVM_MAX_CPU_CLK;
	u32 rev = 0;

	s = env_get("maxcpuclk");
	if (s)
		maxcpuclk = simple_strtoul(s, NULL, 10);

	if (maxcpuclk >= 456000000)
		rev = 3;
	else if (maxcpuclk >= 408000000)
		rev = 2;
	else if (maxcpuclk >= 372000000)
		rev = 1;
#ifdef CONFIG_DA850_AM18X_EVM
	rev |= REV_AM18X_EVM;
#endif
	return rev;
}

int board_early_init_f(void)
{
	/*
	 * Power on required peripherals
	 * ARM does not have access by default to PSC0 and PSC1
	 * assuming here that the DSP bootloader has set the IOPU
	 * such that PSC access is available to ARM
	 */
	if (da8xx_configure_lpsc_items(lpsc, ARRAY_SIZE(lpsc)))
		return 1;

	return 0;
}

int board_init(void)
{
	irq_init();

#ifdef CONFIG_NAND_DAVINCI
	/*
	 * NAND CS setup - cycle counts based on da850evm NAND timings in the
	 * Linux kernel @ 25MHz EMIFA
	 */
	writel((DAVINCI_ABCR_WSETUP(2) |
		DAVINCI_ABCR_WSTROBE(2) |
		DAVINCI_ABCR_WHOLD(1) |
		DAVINCI_ABCR_RSETUP(1) |
		DAVINCI_ABCR_RSTROBE(4) |
		DAVINCI_ABCR_RHOLD(0) |
		DAVINCI_ABCR_TA(1) |
		DAVINCI_ABCR_ASIZE_8BIT),
	       &davinci_emif_regs->ab2cr); /* CS3 */
#endif

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_DA850_EVM;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* setup the SUSPSRC for ARM to control emulation suspend */
	writel(readl(&davinci_syscfg_regs->suspsrc) &
	       ~(DAVINCI_SYSCFG_SUSPSRC_EMAC | DAVINCI_SYSCFG_SUSPSRC_I2C |
		 DAVINCI_SYSCFG_SUSPSRC_SPI1 | DAVINCI_SYSCFG_SUSPSRC_TIMER0 |
		 DAVINCI_SYSCFG_SUSPSRC_UART2),
	       &davinci_syscfg_regs->suspsrc);

	/* configure pinmux settings */
	if (davinci_configure_pin_mux_items(pinmuxes, ARRAY_SIZE(pinmuxes)))
		return 1;

#ifdef CONFIG_USE_NOR
	/* Set the GPIO direction as output */
	clrbits_le32((u32 *)GPIO_BANK0_REG_DIR_ADDR, (0x01 << 11));

	/* Set the output as low */
	writel(0x01 << 11, GPIO_BANK0_REG_CLR_ADDR);
#endif

#ifdef CONFIG_MMC_DAVINCI
	/* Set the GPIO direction as output */
	clrbits_le32((u32 *)GPIO_BANK0_REG_DIR_ADDR, (0x01 << 11));

	/* Set the output as high */
	writel(0x01 << 11, GPIO_BANK0_REG_SET_ADDR);
#endif

#ifdef CONFIG_DRIVER_TI_EMAC
	davinci_emac_mii_mode_sel(HAS_RMII);
#endif /* CONFIG_DRIVER_TI_EMAC */

	/* enable the console UART */
	writel((DAVINCI_UART_PWREMU_MGMT_FREE | DAVINCI_UART_PWREMU_MGMT_URRST |
		DAVINCI_UART_PWREMU_MGMT_UTRST),
	       &davinci_uart2_ctrl_regs->pwremu_mgmt);

	return 0;
}

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
/**
 * rmii_hw_init
 *
 * DA850/OMAP-L138 EVM can interface to a daughter card for
 * additional features. This card has an I2C GPIO Expander TCA6416
 * to select the required functions like camera, RMII Ethernet,
 * character LCD, video.
 *
 * Initialization of the expander involves configuring the
 * polarity and direction of the ports. P07-P05 are used here.
 * These ports are connected to a Mux chip which enables only one
 * functionality at a time.
 *
 * For RMII phy to respond, the MII MDIO clock has to be  disabled
 * since both the PHY devices have address as zero. The MII MDIO
 * clock is controlled via GPIO2[6].
 *
 * This code is valid for Beta version of the hardware
 */
int rmii_hw_init(void)
{
	const struct pinmux_config gpio_pins[] = {
		{ pinmux(6), 8, 1 }
	};
	u_int8_t buf[2];
	unsigned int temp;
	int ret;

	/* PinMux for GPIO */
	if (davinci_configure_pin_mux(gpio_pins, ARRAY_SIZE(gpio_pins)) != 0)
		return 1;

	/* I2C Exapnder configuration */
	/* Set polarity to non-inverted */
	buf[0] = 0x0;
	buf[1] = 0x0;
	ret = i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 4, 1, buf, 2);
	if (ret) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
		return ret;
	}

	/* Configure P07-P05 as outputs */
	buf[0] = 0x1f;
	buf[1] = 0xff;
	ret = i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 6, 1, buf, 2);
	if (ret) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	/* For Ethernet RMII selection
	 * P07(SelA)=0
	 * P06(SelB)=1
	 * P05(SelC)=1
	 */
	if (i2c_read(CONFIG_SYS_I2C_EXPANDER_ADDR, 2, 1, buf, 1)) {
		printf("\nExpander @ 0x%02x read FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	buf[0] &= 0x1f;
	buf[0] |= (0 << 7) | (1 << 6) | (1 << 5);
	if (i2c_write(CONFIG_SYS_I2C_EXPANDER_ADDR, 2, 1, buf, 1)) {
		printf("\nExpander @ 0x%02x write FAILED!!!\n",
				CONFIG_SYS_I2C_EXPANDER_ADDR);
	}

	/* Set the output as high */
	temp = REG(GPIO_BANK2_REG_SET_ADDR);
	temp |= (0x01 << 6);
	REG(GPIO_BANK2_REG_SET_ADDR) = temp;

	/* Set the GPIO direction as output */
	temp = REG(GPIO_BANK2_REG_DIR_ADDR);
	temp &= ~(0x01 << 6);
	REG(GPIO_BANK2_REG_DIR_ADDR) = temp;

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC_USE_RMII */

/*
 * Initializes on-board ethernet controllers.
 */
int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_DRIVER_TI_EMAC_USE_RMII
	/* Select RMII fucntion through the expander */
	if (rmii_hw_init())
		printf("RMII hardware init failed!!!\n");
#endif
	if (!davinci_emac_initialize()) {
		printf("Error: Ethernet init failed!\n");
		return -1;
	}

	return 0;
}
#endif /* CONFIG_DRIVER_TI_EMAC */
