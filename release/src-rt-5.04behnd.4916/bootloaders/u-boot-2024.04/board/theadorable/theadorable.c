// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2019 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <i2c.h>
#include <init.h>
#include <net.h>
#include <pci.h>
#if !defined(CONFIG_SPL_BUILD)
#include <bootcount.h>
#endif
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>
#include <linux/mbus.h>
#ifdef CONFIG_NET
#include <netdev.h>
#endif
#include <u-boot/crc.h>
#include "theadorable.h"

#include "../drivers/ddr/marvell/axp/ddr3_hw_training.h"
#include "../arch/arm/mach-mvebu/serdes/axp/high_speed_env_spec.h"

DECLARE_GLOBAL_DATA_PTR;

#define MV_USB_PHY_BASE			(MVEBU_AXP_USB_BASE + 0x800)
#define PHY_CHANNEL_RX_CTRL0_REG(port, chan) \
	(MV_USB_PHY_BASE + ((port) << 12) + ((chan) << 6) + 0x8)

#define THEADORABLE_GPP_OUT_ENA_LOW	0x00336780
#define THEADORABLE_GPP_OUT_ENA_MID	0x00003cf0
#define THEADORABLE_GPP_OUT_ENA_HIGH	(~(0x0))

#define THEADORABLE_GPP_OUT_VAL_LOW	0x2c0c983f
#define THEADORABLE_GPP_OUT_VAL_MID	0x0007000c
#define THEADORABLE_GPP_OUT_VAL_HIGH	0x00000000

#define GPIO_USB0_PWR_ON		18
#define GPIO_USB1_PWR_ON		19

#define PEX_SWITCH_NOT_FOUNT_LIMIT	3

#define STM_I2C_BUS	1
#define STM_I2C_ADDR	0x27
#define REBOOT_DELAY	1000		/* reboot-delay in ms */
#define ABORT_TIMEOUT	3000		/* 3 seconds reboot abort timeout */

/* DDR3 static configuration */
static MV_DRAM_MC_INIT ddr3_theadorable[MV_MAX_DDR3_STATIC_SIZE] = {
	{0x00001400, 0x7301ca28},	/* DDR SDRAM Configuration Register */
	{0x00001404, 0x30000800},	/* Dunit Control Low Register */
	{0x00001408, 0x44149887},	/* DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x38d93fc7},	/* DDR SDRAM Timing (High) Register */
	{0x00001410, 0x1b100001},	/* DDR SDRAM Address Control Register */
	{0x00001424, 0x0000f3ff},	/* Dunit Control High Register */
	{0x00001428, 0x000f8830},	/* ODT Timing (Low) Register */
	{0x0000142C, 0x014c50f4},	/* DDR3 Timing Register */
	{0x0000147C, 0x0000c671},	/* ODT Timing (High) Register */

	{0x00001494, 0x00010000},	/* DDR SDRAM ODT Control (Low) Reg */
	{0x0000149C, 0x00000001},	/* DDR Dunit ODT Control Register */
	{0x000014A0, 0x00000001},	/* DRAM FIFO Control Register */
	{0x000014A8, 0x00000101},	/* AXI Control Register */

	/*
	 * DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the
	 * training sequence
	 */
	{0x000200e8, 0x3fff0e01},
	{0x00020184, 0x3fffffe0},	/* Close fast path Window to - 2G */

	{0x0001504, 0x7fffffe1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x00020220, 0x00000007},	/* Reserved */

	{0x00001538, 0x00000009},	/* Read Data Sample Delays Register */
	{0x0000153C, 0x00000009},	/* Read Data Ready Delay Register */

	{0x000015D0, 0x00000650},	/* MR0 */
	{0x000015D4, 0x00000044},	/* MR1 */
	{0x000015D8, 0x00000010},	/* MR2 */
	{0x000015DC, 0x00000000},	/* MR3 */
	{0x000015E0, 0x00000001},
	{0x000015E4, 0x00203c18},	/* ZQDS Configuration Register */
	{0x000015EC, 0xf800a225},	/* DDR PHY */

	/* Recommended Settings from Marvell for 4 x 16 bit devices: */
	{0x000014C0, 0x192424c9},	/* DRAM addr and Ctrl Driving Strenght*/
	{0x000014C4, 0x0aaa24c9},	/* DRAM Data and DQS Driving Strenght */

	{0x0, 0x0}
};

static MV_DRAM_MODES board_ddr_modes[MV_DDR3_MODES_NUMBER] = {
	{"theadorable_1333-667", 0x3, 0x5, 0x0, A0, ddr3_theadorable,  NULL},
};

extern MV_SERDES_CHANGE_M_PHY serdes_change_m_phy[];

/*
 * Lane0 - PCIE0.0 X1 (to WIFI Module)
 * Lane5 - SATA0
 * Lane6 - SATA1
 * Lane7 - SGMII0 (to Ethernet Phy)
 * Lane8-11 - PCIE2.0 X4 (to PEX Switch)
 * all other lanes are disabled
 */
MV_BIN_SERDES_CFG theadorable_serdes_cfg[] = {
	{ MV_PEX_ROOT_COMPLEX, 0x22200001, 0x00001111,
	  { PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4,
	    PEX_BUS_DISABLED },
	  0x0060, serdes_change_m_phy
	},
};

/*
 * Define a board-specific detection pulse-width array for the SerDes PCIe
 * interfaces. If not defined in the board code, the default of currently 2
 * is used. Values from 0...3 are possible (2 bits).
 */
u8 serdes_pex_pulse_width[4] = { 0, 2, 2, 2 };

MV_DRAM_MODES *ddr3_get_static_ddr_mode(void)
{
	/* Only one mode supported for this board */
	return &board_ddr_modes[0];
}

MV_BIN_SERDES_CFG *board_serdes_cfg_get(void)
{
	return &theadorable_serdes_cfg[0];
}

u8 board_sat_r_get(u8 dev_num, u8 reg)
{
	/* Bit x enables PCI 2.0 link capabilities instead of PCI 1.x */
	return 0xe;	/* PEX port 0 is PCIe Gen1, PEX port 1..3 PCIe Gen2 */
}

#define PCIE_LNK_CTRL_STAT_2_OFF	0x0090
#define PCIE_LNK_CTRL_STAT_2_DEEM_BIT	BIT(6)

static void pcie_set_deemphasis(u32 base)
{
	u32 reg;

	reg = readl((void *)base + PCIE_LNK_CTRL_STAT_2_OFF);
	reg |= PCIE_LNK_CTRL_STAT_2_DEEM_BIT;
	writel(reg, (void *)base + PCIE_LNK_CTRL_STAT_2_OFF);
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x00000000, MVEBU_MPP_BASE + 0x00);
	writel(0x03300000, MVEBU_MPP_BASE + 0x04);
	writel(0x00000033, MVEBU_MPP_BASE + 0x08);
	writel(0x00000000, MVEBU_MPP_BASE + 0x0c);
	writel(0x11110000, MVEBU_MPP_BASE + 0x10);
	writel(0x00221100, MVEBU_MPP_BASE + 0x14);
	writel(0x00000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00000000, MVEBU_MPP_BASE + 0x1c);
	writel(0x00000000, MVEBU_MPP_BASE + 0x20);

	/* Configure GPIO */
	writel(THEADORABLE_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(THEADORABLE_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
	writel(THEADORABLE_GPP_OUT_VAL_HIGH, MVEBU_GPIO2_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_HIGH, MVEBU_GPIO2_BASE + 0x04);

	/*
	 * Set deephasis bit in the PCIe configuration of both PCIe ports
	 * used on this board.
	 *
	 * This needs to be done very early, even before the SERDES setup
	 * code is run. This way, the first link will already be established
	 * with this setup. Testing has shown, that this results in a more
	 * stable PCIe link with better signal quality.
	 */
	pcie_set_deemphasis(MVEBU_REG_PCIE_BASE);		/* Port 0 */
	pcie_set_deemphasis(MVEBU_REG_PCIE_BASE + 0x2000);	/* Port 2 */

	return 0;
}

int board_init(void)
{
	int ret;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	/*
	 * Map SPI devices via MBUS so that they can be accessed via
	 * the SPI direct access mode
	 */
	mbus_dt_setup_win(SPI_BUS0_DEV1_BASE, SPI_BUS0_DEV1_SIZE,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_SPI0_CS1);
	mbus_dt_setup_win(SPI_BUS1_DEV2_BASE, SPI_BUS0_DEV1_SIZE,
			  CPU_TARGET_DEVICEBUS_BOOTROM_SPI, CPU_ATTR_SPI1_CS2);

	/*
	 * Set RX Channel Control 0 Register:
	 * Tests have shown, that setting the LPF_COEF from 0 (1/8)
	 * to 3 (1/1) results in a more stable USB connection.
	 */
	setbits_le32(PHY_CHANNEL_RX_CTRL0_REG(0, 1), 0xc);
	setbits_le32(PHY_CHANNEL_RX_CTRL0_REG(0, 2), 0xc);
	setbits_le32(PHY_CHANNEL_RX_CTRL0_REG(0, 3), 0xc);

	/* Toggle USB power */
	ret = gpio_request(GPIO_USB0_PWR_ON, "USB0_PWR_ON");
	if (ret < 0)
		return ret;
	gpio_direction_output(GPIO_USB0_PWR_ON, 0);
	ret = gpio_request(GPIO_USB1_PWR_ON, "USB1_PWR_ON");
	if (ret < 0)
		return ret;
	gpio_direction_output(GPIO_USB1_PWR_ON, 0);
	mdelay(1);
	gpio_set_value(GPIO_USB0_PWR_ON, 1);
	gpio_set_value(GPIO_USB1_PWR_ON, 1);

	return 0;
}

int checkboard(void)
{
	board_fpga_add();

	return 0;
}

#ifdef CONFIG_NET
int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	pci_dev_t bdf;
	ulong bootcount;

	/*
	 * Check if the PEX switch is detected (somtimes its not available
	 * on the PCIe bus). In this case, try to recover by issuing a
	 * soft-reset or even a power-cycle, depending on the bootcounter
	 * value.
	 */
	bdf = pci_find_device(PCI_VENDOR_ID_PLX, 0x8619, 0);
	if (bdf == -1) {
		unsigned long start_time = get_timer(0);
		u8 i2c_buf[8];
		int ret;

		/* PEX switch not found! */
		bootcount = bootcount_load();
		printf("Failed to find PLX PEX-switch (bootcount=%ld)\n",
		       bootcount);

		/*
		 * The user can exit this boot-loop in the error case by
		 * hitting Ctrl-C. So wait some time for this key here.
		 */
		printf("Continue booting with Ctrl-C, otherwise rebooting\n");
		do {
			/* Handle control-c and timeouts */
			if (ctrlc()) {
				printf("PEX error boot-loop aborted!\n");
				return 0;
			}
		} while (get_timer(start_time) < ABORT_TIMEOUT);


		/*
		 * At this stage the bootcounter has not been incremented
		 * yet. We need to do this manually here to get an actually
		 * working bootcounter in this error case.
		 */
		bootcount_inc();

		if (bootcount > PEX_SWITCH_NOT_FOUNT_LIMIT) {
			struct udevice *dev;

			printf("Issuing power-switch via uC!\n");
			ret = i2c_get_chip_for_busnum(STM_I2C_BUS, STM_I2C_ADDR,
						      1, &dev);
			if (ret) {
				printf("Error selecting STM on I2C bus (ret=%d)\n",
				       ret);
				printf("Issuing soft-reset...\n");
				/* default handling: SOFT reset */
				do_reset(NULL, 0, 0, NULL);
			}

			i2c_buf[0] = STM_I2C_ADDR << 1;
			i2c_buf[1] = 0xc5;	/* cmd */
			i2c_buf[2] = 0x01;	/* enable */
			/* Delay before reboot */
			i2c_buf[3] = REBOOT_DELAY & 0x00ff;
			i2c_buf[4] = (REBOOT_DELAY & 0xff00) >> 8;
			/* Delay before shutdown */
			i2c_buf[5] = 0x00;
			i2c_buf[6] = 0x00;
			i2c_buf[7] = crc8(0x72, &i2c_buf[0], 7);

			ret = dm_i2c_write(dev, 0, &i2c_buf[1], 7);
			if (ret) {
				printf("I2C write error (ret=%d)\n", ret);
				printf("Issuing soft-reset...\n");
				/* default handling: SOFT reset */
				do_reset(NULL, 0, 0, NULL);
			}

			/* Wait for power-cycle to occur... */
			printf("Waiting for power-cycle via uC...\n");
			while (1)
				;
		} else {
			printf("Issuing soft-reset...\n");
			/* default handling: SOFT reset */
			do_reset(NULL, 0, 0, NULL);
		}
	}

	return 0;
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_PCI)
static int pcie_get_link_speed_width(pci_dev_t bdf, int *speed, int *width)
{
	struct udevice *dev;
	u16 ven_id, dev_id;
	u16 lnksta;
	int ret;
	int pos;

	/*
	 * Check if the PCIe device is detected (sometimes its not available
	 * on the PCIe bus)
	 */
	ret = dm_pci_bus_find_bdf(bdf, &dev);
	if (ret)
		return -ENODEV;

	/* PCIe device found */
	dm_pci_read_config16(dev, PCI_VENDOR_ID, &ven_id);
	dm_pci_read_config16(dev, PCI_DEVICE_ID, &dev_id);
	printf("Detected PCIe device: VendorID 0x%04x DeviceId 0x%04x @ BDF %d.%d.%d\n",
	       ven_id, dev_id, PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	/* Now read EXP_LNKSTA register */
	pos = dm_pci_find_capability(dev, PCI_CAP_ID_EXP);
	dm_pci_read_config16(dev, pos + PCI_EXP_LNKSTA, &lnksta);
	*speed = lnksta & PCI_EXP_LNKSTA_CLS;
	*width = (lnksta & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;

	return 0;
}

/*
 * U-Boot cmd to test for the presence of the directly connected PCIe devices
 * the theadorable board. This cmd can be used by U-Boot scripts for automated
 * testing, if the PCIe setup is correct. Meaning, that all PCIe devices are
 * correctly detected and the link speed and width is corrent.
 *
 * Here a short script that may be used for an automated test. It results in
 * an endless reboot loop, if the PCIe devices are detected correctly. If at
 * any time a problem is detected (PCIe device not available or link is
 * incorrect), then booting will halt. So just use this "bootcmd" and let the
 * board run over a longer time (e.g. one night) and if the board still reboots
 * after this time, then everything is okay.
 *
 * bootcmd=echo bootcount=$bootcount; pcie ;if test $? -eq 0;
 *         then echo PCIe status okay, resetting...; reset; else;
 *         echo PCIe status NOT okay, hanging (bootcount=$bootcount); fi;
 */
int do_pcie_test(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	pci_dev_t bdf;
	int speed;
	int width;
	int ret;

	if (argc != 1)
		return cmd_usage(cmdtp);

	/*
	 * Check if the PCIe device is detected (sometimes its not available
	 * on the PCIe bus)
	 */

	/* Check for PCIe device on PCIe port/bus 0 */
	bdf = PCI_BDF(0, 1, 0);
	ret = pcie_get_link_speed_width(bdf, &speed, &width);
	if (ret) {
		/* PCIe device not found! */
		printf("Failed to find PCIe device @ BDF %d.%d.%d\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		return CMD_RET_FAILURE;
	}

	printf("Established speed=%d width=%d\n", speed, width);
	if ((speed != 1 || width != 1)) {
		printf("Detected incorrect speed/width!!!\n");
		return CMD_RET_FAILURE;
	}

	/* Check for PCIe device on PCIe port/bus 1 */
	bdf = PCI_BDF(1, 1, 0);
	ret = pcie_get_link_speed_width(bdf, &speed, &width);
	if (ret) {
		/* PCIe device not found! */
		printf("Failed to find PCIe device @ BDF %d.%d.%d\n",
		       PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
		return CMD_RET_FAILURE;
	}

	printf("Established speed=%d width=%d\n", speed, width);
	if ((speed != 2 || width != 4)) {
		printf("Detected incorrect speed/width!!!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	pcie,   1,   0,     do_pcie_test,
	"Test for presence of a PCIe devices with correct link",
	""
);
#endif
