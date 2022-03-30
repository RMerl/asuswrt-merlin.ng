// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#include <common.h>
#include <i2c.h>
#include <nand.h>
#include <netdev.h>
#include <miiphy.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/arch/mpp.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * BOCO FPGA definitions
 */
#define BOCO		0x10
#define REG_CTRL_H		0x02
#define MASK_WRL_UNITRUN	0x01
#define MASK_RBX_PGY_PRESENT	0x40
#define REG_IRQ_CIRQ2		0x2d
#define MASK_RBI_DEFECT_16	0x01

/*
 * PHY registers definitions
 */
#define PHY_MARVELL_OUI					0x5043
#define PHY_MARVELL_88E1118_MODEL			0x0022
#define PHY_MARVELL_88E1118R_MODEL			0x0024

#define PHY_MARVELL_PAGE_REG				0x0016
#define PHY_MARVELL_DEFAULT_PAGE			0x0000

#define PHY_MARVELL_88E1118R_LED_CTRL_PAGE		0x0003
#define PHY_MARVELL_88E1118R_LED_CTRL_REG		0x0010

#define PHY_MARVELL_88E1118R_LED_CTRL_RESERVED		0x1000
#define PHY_MARVELL_88E1118R_LED_CTRL_LED0_1000MB	(0x7<<0)
#define PHY_MARVELL_88E1118R_LED_CTRL_LED1_ACT		(0x3<<4)
#define PHY_MARVELL_88E1118R_LED_CTRL_LED2_LINK		(0x0<<8)

/* I/O pin to erase flash RGPP09 = MPP43 */
#define KM_FLASH_ERASE_ENABLE	43

/* Multi-Purpose Pins Functionality configuration */
static const u32 kwmpp_config[] = {
	MPP0_NF_IO2,
	MPP1_NF_IO3,
	MPP2_NF_IO4,
	MPP3_NF_IO5,
	MPP4_NF_IO6,
	MPP5_NF_IO7,
	MPP6_SYSRST_OUTn,
#if defined(KM_PCIE_RESET_MPP7)
	MPP7_GPO,
#else
	MPP7_PEX_RST_OUTn,
#endif
#if defined(CONFIG_SYS_I2C_SOFT)
	MPP8_GPIO,		/* SDA */
	MPP9_GPIO,		/* SCL */
#endif
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP12_GPO,		/* Reserved */
	MPP13_UART1_TXD,
	MPP14_UART1_RXD,
	MPP15_GPIO,		/* Not used */
	MPP16_GPIO,		/* Not used */
	MPP17_GPIO,		/* Reserved */
	MPP18_NF_IO0,
	MPP19_NF_IO1,
	MPP20_GPIO,
	MPP21_GPIO,
	MPP22_GPIO,
	MPP23_GPIO,
	MPP24_GPIO,
	MPP25_GPIO,
	MPP26_GPIO,
	MPP27_GPIO,
	MPP28_GPIO,
	MPP29_GPIO,
	MPP30_GPIO,
	MPP31_GPIO,
	MPP32_GPIO,
	MPP33_GPIO,
	MPP34_GPIO,		/* CDL1 (input) */
	MPP35_GPIO,		/* CDL2 (input) */
	MPP36_GPIO,		/* MAIN_IRQ (input) */
	MPP37_GPIO,		/* BOARD_LED */
	MPP38_GPIO,		/* Piggy3 LED[1] */
	MPP39_GPIO,		/* Piggy3 LED[2] */
	MPP40_GPIO,		/* Piggy3 LED[3] */
	MPP41_GPIO,		/* Piggy3 LED[4] */
	MPP42_GPIO,		/* Piggy3 LED[5] */
	MPP43_GPIO,		/* Piggy3 LED[6] */
	MPP44_GPIO,		/* Piggy3 LED[7], BIST_EN_L */
	MPP45_GPIO,		/* Piggy3 LED[8] */
	MPP46_GPIO,		/* Reserved */
	MPP47_GPIO,		/* Reserved */
	MPP48_GPIO,		/* Reserved */
	MPP49_GPIO,		/* SW_INTOUTn */
	0
};

static uchar ivm_content[CONFIG_SYS_IVM_EEPROM_MAX_LEN];

#if defined(CONFIG_KM_MGCOGE3UN)
/*
 * Wait for startup OK from mgcoge3ne
 */
static int startup_allowed(void)
{
	unsigned char buf;

	/*
	 * Read CIRQ16 bit (bit 0)
	 */
	if (i2c_read(BOCO, REG_IRQ_CIRQ2, 1, &buf, 1) != 0)
		printf("%s: Error reading Boco\n", __func__);
	else
		if ((buf & MASK_RBI_DEFECT_16) == MASK_RBI_DEFECT_16)
			return 1;
	return 0;
}
#endif

#if (defined(CONFIG_KM_PIGGY4_88E6061)|defined(CONFIG_KM_PIGGY4_88E6352))
/*
 * All boards with PIGGY4 connected via a simple switch have ethernet always
 * present.
 */
int ethernet_present(void)
{
	return 1;
}
#else
int ethernet_present(void)
{
	uchar	buf;
	int	ret = 0;

	if (i2c_read(BOCO, REG_CTRL_H, 1, &buf, 1) != 0) {
		printf("%s: Error reading Boco\n", __func__);
		return -1;
	}
	if ((buf & MASK_RBX_PGY_PRESENT) == MASK_RBX_PGY_PRESENT)
		ret = 1;

	return ret;
}
#endif

static int initialize_unit_leds(void)
{
	/*
	 * Init the unit LEDs per default they all are
	 * ok apart from bootstat
	 */
	uchar buf;

	if (i2c_read(BOCO, REG_CTRL_H, 1, &buf, 1) != 0) {
		printf("%s: Error reading Boco\n", __func__);
		return -1;
	}
	buf |= MASK_WRL_UNITRUN;
	if (i2c_write(BOCO, REG_CTRL_H, 1, &buf, 1) != 0) {
		printf("%s: Error writing Boco\n", __func__);
		return -1;
	}
	return 0;
}

static void set_bootcount_addr(void)
{
	uchar buf[32];
	unsigned int bootcountaddr;
	bootcountaddr = gd->ram_size - BOOTCOUNT_ADDR;
	sprintf((char *)buf, "0x%x", bootcountaddr);
	env_set("bootcountaddr", (char *)buf);
}

int misc_init_r(void)
{
#if defined(CONFIG_KM_MGCOGE3UN)
	char *wait_for_ne;
	u8 dip_switch = kw_gpio_get_value(KM_FLASH_ERASE_ENABLE);
	wait_for_ne = env_get("waitforne");

	if ((wait_for_ne != NULL) && (dip_switch == 0)) {
		if (strcmp(wait_for_ne, "true") == 0) {
			int cnt = 0;
			int abort = 0;
			puts("NE go: ");
			while (startup_allowed() == 0) {
				if (tstc()) {
					(void) getc(); /* consume input */
					abort = 1;
					break;
				}
				udelay(200000);
				cnt++;
				if (cnt == 5)
					puts("wait\b\b\b\b");
				if (cnt == 10) {
					cnt = 0;
					puts("    \b\b\b\b");
				}
			}
			if (abort == 1)
				printf("\nAbort waiting for ne\n");
			else
				puts("OK\n");
		}
	}
#endif

	ivm_read_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);

	initialize_unit_leds();
	set_km_env();
	set_bootcount_addr();
	return 0;
}

int board_early_init_f(void)
{
#if defined(CONFIG_SYS_I2C_SOFT)
	u32 tmp;

	/* set the 2 bitbang i2c pins as output gpios */
	tmp = readl(MVEBU_GPIO0_BASE + 4);
	writel(tmp & (~KM_KIRKWOOD_SOFT_I2C_GPIOS) , MVEBU_GPIO0_BASE + 4);
#endif
	/* adjust SDRAM size for bank 0 */
	mvebu_sdram_size_adjust(0);
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/*
	 * The KM_FLASH_GPIO_PIN switches between using a
	 * NAND or a SPI FLASH. Set this pin on start
	 * to NAND mode.
	 */
	kw_gpio_set_valid(KM_FLASH_GPIO_PIN, 1);
	kw_gpio_direction_output(KM_FLASH_GPIO_PIN, 1);

#if defined(CONFIG_SYS_I2C_SOFT)
	/*
	 * Reinit the GPIO for I2C Bitbang driver so that the now
	 * available gpio framework is consistent. The calls to
	 * direction output in are not necessary, they are already done in
	 * board_early_init_f
	 */
	kw_gpio_set_valid(KM_KIRKWOOD_SDA_PIN, 1);
	kw_gpio_set_valid(KM_KIRKWOOD_SCL_PIN, 1);
#endif

#if defined(CONFIG_SYS_EEPROM_WREN)
	kw_gpio_set_valid(KM_KIRKWOOD_ENV_WP, 38);
	kw_gpio_direction_output(KM_KIRKWOOD_ENV_WP, 1);
#endif

#if defined(CONFIG_KM_FPGA_CONFIG)
	trigger_fpga_config();
#endif

	return 0;
}

int board_late_init(void)
{
#if (defined(CONFIG_KM_COGE5UN) | defined(CONFIG_KM_MGCOGE3UN))
	u8 dip_switch = kw_gpio_get_value(KM_FLASH_ERASE_ENABLE);

	/* if pin 1 do full erase */
	if (dip_switch != 0) {
		/* start bootloader */
		puts("DIP:   Enabled\n");
		env_set("actual_bank", "0");
	}
#endif

#if defined(CONFIG_KM_FPGA_CONFIG)
	wait_for_fpga_config();
	fpga_reset();
	toggle_eeprom_spi_bus();
#endif
	return 0;
}

int board_spi_claim_bus(struct spi_slave *slave)
{
	kw_gpio_set_value(KM_FLASH_GPIO_PIN, 0);

	return 0;
}

void board_spi_release_bus(struct spi_slave *slave)
{
	kw_gpio_set_value(KM_FLASH_GPIO_PIN, 1);
}

#if (defined(CONFIG_KM_PIGGY4_88E6061))

#define	PHY_LED_SEL_REG		0x18
#define PHY_LED0_LINK		(0x5)
#define PHY_LED1_ACT		(0x8<<4)
#define PHY_LED2_INT		(0xe<<8)
#define	PHY_SPEC_CTRL_REG	0x1c
#define PHY_RGMII_CLK_STABLE	(0x1<<10)
#define PHY_CLSA		(0x1<<1)

/* Configure and enable MV88E3018 PHY */
void reset_phy(void)
{
	char *name = "egiga0";
	unsigned short reg;

	if (miiphy_set_current_dev(name))
		return;

	/* RGMII clk transition on data stable */
	if (miiphy_read(name, CONFIG_PHY_BASE_ADR, PHY_SPEC_CTRL_REG, &reg))
		printf("Error reading PHY spec ctrl reg\n");
	if (miiphy_write(name, CONFIG_PHY_BASE_ADR, PHY_SPEC_CTRL_REG,
			 reg | PHY_RGMII_CLK_STABLE | PHY_CLSA))
		printf("Error writing PHY spec ctrl reg\n");

	/* leds setup */
	if (miiphy_write(name, CONFIG_PHY_BASE_ADR, PHY_LED_SEL_REG,
			 PHY_LED0_LINK | PHY_LED1_ACT | PHY_LED2_INT))
		printf("Error writing PHY LED reg\n");

	/* reset the phy */
	miiphy_reset(name, CONFIG_PHY_BASE_ADR);
}
#elif defined(CONFIG_KM_PIGGY4_88E6352)

#include <mv88e6352.h>

#if defined(CONFIG_KM_NUSA)
struct mv88e_sw_reg extsw_conf[] = {
	/*
	 * port 0, PIGGY4, autoneg
	 * first the fix for the 1000Mbits Autoneg, this is from
	 * a Marvell errata, the regs are undocumented
	 */
	{ PHY(0), PHY_PAGE, AN1000FIX_PAGE },
	{ PHY(0), PHY_STATUS, AN1000FIX },
	{ PHY(0), PHY_PAGE, 0 },
	/* now the real port and phy configuration */
	{ PORT(0), PORT_PHY, NO_SPEED_FOR },
	{ PORT(0), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	{ PHY(0), PHY_1000_CTRL, NO_ADV },
	{ PHY(0), PHY_SPEC_CTRL, AUTO_MDIX_EN },
	{ PHY(0), PHY_CTRL, PHY_100_MBPS | AUTONEG_EN | AUTONEG_RST |
		FULL_DUPLEX },
	/* port 1, unused */
	{ PORT(1), PORT_CTRL, PORT_DIS },
	{ PHY(1), PHY_CTRL, PHY_PWR_DOWN },
	{ PHY(1), PHY_SPEC_CTRL, SPEC_PWR_DOWN },
	/* port 2, unused */
	{ PORT(2), PORT_CTRL, PORT_DIS },
	{ PHY(2), PHY_CTRL, PHY_PWR_DOWN },
	{ PHY(2), PHY_SPEC_CTRL, SPEC_PWR_DOWN },
	/* port 3, unused */
	{ PORT(3), PORT_CTRL, PORT_DIS },
	{ PHY(3), PHY_CTRL, PHY_PWR_DOWN },
	{ PHY(3), PHY_SPEC_CTRL, SPEC_PWR_DOWN },
	/* port 4, ICNEV, SerDes, SGMII */
	{ PORT(4), PORT_STATUS, NO_PHY_DETECT },
	{ PORT(4), PORT_PHY, SPEED_1000_FOR },
	{ PORT(4), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	{ PHY(4), PHY_CTRL, PHY_PWR_DOWN },
	{ PHY(4), PHY_SPEC_CTRL, SPEC_PWR_DOWN },
	/* port 5, CPU_RGMII */
	{ PORT(5), PORT_PHY, RX_RGMII_TIM | TX_RGMII_TIM | FLOW_CTRL_EN |
		FLOW_CTRL_FOR | LINK_VAL | LINK_FOR | FULL_DPX |
		FULL_DPX_FOR | SPEED_1000_FOR },
	{ PORT(5), PORT_CTRL, FORWARDING | EGRS_FLD_ALL },
	/* port 6, unused, this port has no phy */
	{ PORT(6), PORT_CTRL, PORT_DIS },
};
#else
struct mv88e_sw_reg extsw_conf[] = {};
#endif

void reset_phy(void)
{
#if defined(CONFIG_KM_MVEXTSW_ADDR)
	char *name = "egiga0";

	if (miiphy_set_current_dev(name))
		return;

	mv88e_sw_program(name, CONFIG_KM_MVEXTSW_ADDR, extsw_conf,
		ARRAY_SIZE(extsw_conf));
	mv88e_sw_reset(name, CONFIG_KM_MVEXTSW_ADDR);
#endif
}

#else
/* Configure and enable MV88E1118 PHY on the piggy*/
void reset_phy(void)
{
	unsigned int oui;
	unsigned char model, rev;

	char *name = "egiga0";

	if (miiphy_set_current_dev(name))
		return;

	/* reset the phy */
	miiphy_reset(name, CONFIG_PHY_BASE_ADR);

	/* get PHY model */
	if (miiphy_info(name, CONFIG_PHY_BASE_ADR, &oui, &model, &rev))
		return;

	/* check for Marvell 88E1118R Gigabit PHY (PIGGY3) */
	if ((oui == PHY_MARVELL_OUI) &&
	    (model == PHY_MARVELL_88E1118R_MODEL)) {
		/* set page register to 3 */
		if (miiphy_write(name, CONFIG_PHY_BASE_ADR,
				 PHY_MARVELL_PAGE_REG,
				 PHY_MARVELL_88E1118R_LED_CTRL_PAGE))
			printf("Error writing PHY page reg\n");

		/*
		 * leds setup as printed on PCB:
		 * LED2 (Link): 0x0 (On Link, Off No Link)
		 * LED1 (Activity): 0x3 (On Activity, Off No Activity)
		 * LED0 (Speed): 0x7 (On 1000 MBits, Off Else)
		 */
		if (miiphy_write(name, CONFIG_PHY_BASE_ADR,
				 PHY_MARVELL_88E1118R_LED_CTRL_REG,
				 PHY_MARVELL_88E1118R_LED_CTRL_RESERVED |
				 PHY_MARVELL_88E1118R_LED_CTRL_LED0_1000MB |
				 PHY_MARVELL_88E1118R_LED_CTRL_LED1_ACT |
				 PHY_MARVELL_88E1118R_LED_CTRL_LED2_LINK))
			printf("Error writing PHY LED reg\n");

		/* set page register back to 0 */
		if (miiphy_write(name, CONFIG_PHY_BASE_ADR,
				 PHY_MARVELL_PAGE_REG,
				 PHY_MARVELL_DEFAULT_PAGE))
			printf("Error writing PHY page reg\n");
	}
}
#endif


#if defined(CONFIG_HUSH_INIT_VAR)
int hush_init_var(void)
{
	ivm_analyze_eeprom(ivm_content, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	return 0;
}
#endif

#if defined(CONFIG_SYS_I2C_SOFT)
void set_sda(int state)
{
	I2C_ACTIVE;
	I2C_SDA(state);
}

void set_scl(int state)
{
	I2C_SCL(state);
}

int get_sda(void)
{
	I2C_TRISTATE;
	return I2C_READ;
}

int get_scl(void)
{
	return kw_gpio_get_value(KM_KIRKWOOD_SCL_PIN) ? 1 : 0;
}
#endif

#if defined(CONFIG_POST)

#define KM_POST_EN_L	44
#define POST_WORD_OFF	8

int post_hotkeys_pressed(void)
{
#if defined(CONFIG_KM_COGE5UN)
	return kw_gpio_get_value(KM_POST_EN_L);
#else
	return !kw_gpio_get_value(KM_POST_EN_L);
#endif
}

ulong post_word_load(void)
{
	void* addr = (void *) (gd->ram_size - BOOTCOUNT_ADDR + POST_WORD_OFF);
	return in_le32(addr);

}
void post_word_store(ulong value)
{
	void* addr = (void *) (gd->ram_size - BOOTCOUNT_ADDR + POST_WORD_OFF);
	out_le32(addr, value);
}

int arch_memory_test_prepare(u32 *vstart, u32 *size, phys_addr_t *phys_offset)
{
	*vstart = CONFIG_SYS_SDRAM_BASE;

	/* we go up to relocation plus a 1 MB margin */
	*size = CONFIG_SYS_TEXT_BASE - (1<<20);

	return 0;
}
#endif

#if defined(CONFIG_SYS_EEPROM_WREN)
int eeprom_write_enable(unsigned dev_addr, int state)
{
	kw_gpio_set_value(KM_KIRKWOOD_ENV_WP, !state);

	return !kw_gpio_get_value(KM_KIRKWOOD_ENV_WP);
}
#endif
