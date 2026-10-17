// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 samtec automotive software & electronics gmbh
 * Copyright (C) 2017-2019 softing automotive electronics gmbH
 *
 * Author: Christoph Fritz <chf.fritz@googlemail.com>
 */

#include <init.h>
#include <net.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/io.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/sections.h>
#include <env.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <common.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <pwm.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	PAD_CTL_SRE_FAST)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PKE |	\
	PAD_CTL_SPEED_HIGH | PAD_CTL_DSE_48ohm |		\
	PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  PAD_CTL_DSE_34ohm

#define ENET_RX_PAD_CTRL  (PAD_CTL_PKE |			\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_HIGH |		\
	PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE | PAD_CTL_ODE | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm)

#define USDHC_CLK_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_80ohm | PAD_CTL_SRE_FAST)

#define USDHC_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_47K_UP |	\
	PAD_CTL_PKE |  PAD_CTL_SPEED_MED | PAD_CTL_DSE_80ohm |	\
	PAD_CTL_SRE_FAST)

#define USDHC_RESET_CTRL (PAD_CTL_HYS | PAD_CTL_PUS_47K_UP |	\
	PAD_CTL_PKE |  PAD_CTL_SPEED_MED | PAD_CTL_DSE_80ohm)

#define GPIO_PAD_CTRL  (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_PKE)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static iomux_v3_cfg_t const pwm_led_pads[] = {
	MX6_PAD_RGMII2_RD2__PWM2_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* green */
	MX6_PAD_RGMII2_TD2__PWM6_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* red */
	MX6_PAD_RGMII2_RD3__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL), /* blue */
};

static int board_net_init(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	unsigned char eth1addr[6];
	int ret;

	/* just to get second mac address */
	imx_get_mac_from_fuse(1, eth1addr);
	if (!env_get("eth1addr") && is_valid_ethaddr(eth1addr))
		eth_env_set_enetaddr("eth1addr", eth1addr);

	/*
	 * Generate phy reference clock via pin IOMUX ENET_REF_CLK1/2 by erasing
	 * ENET1/2_TX_CLK_DIR gpr1[14:13], so that reference clock is driven by
	 * ref_enetpll0/1 and enable ENET1/2_TX_CLK output driver.
	 */
	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUX_GPR1_FEC1_CLOCK_MUX2_SEL_MASK |
			IOMUX_GPR1_FEC2_CLOCK_MUX2_SEL_MASK,
			IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK |
			IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);

	ret = enable_fec_anatop_clock(0, ENET_50MHZ);
	if (ret)
		goto eth_fail;

	ret = enable_fec_anatop_clock(1, ENET_50MHZ);
	if (ret)
		goto eth_fail;

	return ret;

eth_fail:
	printf("FEC MXC: %s:failed (%i)\n", __func__, ret);
	return ret;
}

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C1 for PMIC */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO1_IO00__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO00__GPIO1_IO_0 | PC,
		.gp = IMX_GPIO_NR(1, 0),
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO1_IO01__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO01__GPIO1_IO_1 | PC,
		.gp = IMX_GPIO_NR(1, 1),
	},
};

static struct pmic *pfuze_init(unsigned char i2cbus)
{
	struct pmic *p;
	int ret;
	u32 reg;

	ret = power_pfuze100_init(i2cbus);
	if (ret)
		return NULL;

	p = pmic_get("PFUZE100");
	ret = pmic_probe(p);
	if (ret)
		return NULL;

	pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
	printf("PMIC:  PFUZE%i00 ID=0x%02x\n", (reg & 1) ? 2 : 1, reg);

	/* Set SW1AB stanby volage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1ABSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1ABSTBY, reg);

	/* Set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE100_SW1ABCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PFUZE100_SW1ABCONF, reg);

	/* Set SW1C standby voltage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1CSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1CSTBY, reg);

	/* Set SW1C/VDDSOC step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE100_SW1CCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PFUZE100_SW1CCONF, reg);

	return p;
}

static int pfuze_mode_init(struct pmic *p, u32 mode)
{
	unsigned char offset, i, switch_num;
	u32 id;
	int ret;

	pmic_reg_read(p, PFUZE100_DEVICEID, &id);
	id = id & 0xf;

	if (id == 0) {
		switch_num = 6;
		offset = PFUZE100_SW1CMODE;
	} else if (id == 1) {
		switch_num = 4;
		offset = PFUZE100_SW2MODE;
	} else {
		printf("Not supported, id=%d\n", id);
		return -EINVAL;
	}

	ret = pmic_reg_write(p, PFUZE100_SW1ABMODE, mode);
	if (ret < 0) {
		printf("Set SW1AB mode error!\n");
		return ret;
	}

	for (i = 0; i < switch_num - 1; i++) {
		ret = pmic_reg_write(p, offset + i * SWITCH_SIZE, mode);
		if (ret < 0) {
			printf("Set switch 0x%x mode error!\n",
			       offset + i * SWITCH_SIZE);
			return ret;
		}
	}

	return ret;
}

int power_init_board(void)
{
	struct pmic *p;
	int ret;

	p = pfuze_init(I2C_PMIC);
	if (!p)
		return -ENODEV;

	ret = pfuze_mode_init(p, APS_PFM);
	if (ret < 0)
		return ret;

	set_ldo_voltage(LDO_ARM, 1175);	/* Set VDDARM to 1.175V */
	set_ldo_voltage(LDO_SOC, 1175);	/* Set VDDSOC to 1.175V */

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
static iomux_v3_cfg_t const usb_otg_pads[] = {
	/* OGT1 */
	MX6_PAD_GPIO1_IO09__USB_OTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO1_IO10__ANATOP_OTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG2 */
	MX6_PAD_GPIO1_IO12__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL)
};

static void setup_iomux_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
					 ARRAY_SIZE(usb_otg_pads));
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}
#endif

#ifdef CONFIG_PWM_IMX
static int set_pwm_leds(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(pwm_led_pads,
					 ARRAY_SIZE(pwm_led_pads));
	/* enable backlight PWM 2, green LED */
	ret = pwm_init(1, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(1, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(1);
	if (ret)
		goto error;

	/* enable backlight PWM 1, blue LED */
	ret = pwm_init(0, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(0, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(0);
	if (ret)
		goto error;

	/* enable backlight PWM 6, red LED */
	ret = pwm_init(5, 0, 0);
	if (ret)
		goto error;
	/* duty cycle 200ns, period: 8000ns */
	ret = pwm_config(5, 200, 8000);
	if (ret)
		goto error;
	ret = pwm_enable(5);

error:
	return ret;
}
#else
static int set_pwm_leds(void)
{
	return 0;
}
#endif

#define ADCx_HC0        0x00
#define ADCx_HS         0x08
#define ADCx_HS_C0      BIT(0)
#define ADCx_R0         0x0c
#define ADCx_CFG        0x14
#define ADCx_CFG_SWMODE 0x308
#define ADCx_GC         0x18
#define ADCx_GC_CAL     BIT(7)

static int read_adc(u32 *val)
{
	int ret;
	void __iomem *b = map_physmem(ADC1_BASE_ADDR, 0x100, MAP_NOCACHE);

	/* use software mode */
	writel(ADCx_CFG_SWMODE, b + ADCx_CFG);

	/* start auto calibration */
	setbits_le32(b + ADCx_GC, ADCx_GC_CAL);
	ret = wait_for_bit_le32(b + ADCx_GC, ADCx_GC_CAL, ADCx_GC_CAL, 10, 0);
	if (ret)
		goto adc_exit;

	/* start conversion */
	writel(0, b + ADCx_HC0);

	/* wait for conversion */
	ret = wait_for_bit_le32(b + ADCx_HS, ADCx_HS_C0, ADCx_HS_C0, 10, 0);
	if (ret)
		goto adc_exit;

	/* read result */
	*val = readl(b + ADCx_R0);

adc_exit:
	if (ret)
		printf("ADC failure (ret=%i)\n", ret);
	unmap_physmem(b, MAP_NOCACHE);
	return ret;
}

#define VAL_UPPER	2498
#define VAL_LOWER	1550

static int set_pin_state(void)
{
	u32 val;
	int ret;

	ret = read_adc(&val);
	if (ret)
		return ret;

	if (val >= VAL_UPPER)
		env_set("pin_state", "connected");
	else if (val < VAL_UPPER && val > VAL_LOWER)
		env_set("pin_state", "open");
	else
		env_set("pin_state", "button");

	return ret;
}

int board_late_init(void)
{
	int ret;

	ret = set_pwm_leds();
	if (ret)
		return ret;

	ret = set_pin_state();

	return ret;
}

int board_early_init_f(void)
{
	setup_iomux_usb();

	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
#endif

	return board_net_init();
}

int checkboard(void)
{
	puts("Board: VIN|ING 2000\n");

	return 0;
}

#define PCIE_PHY_PUP_REQ		BIT(7)

void board_preboot_os(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct gpc *gpc_regs = (struct gpc *)GPC_BASE_ADDR;

	/* Bring the PCI power domain up, so that old vendorkernel works. */
	setbits_le32(&iomuxc_regs->gpr[12], IOMUXC_GPR12_TEST_POWERDOWN);
	setbits_le32(&iomuxc_regs->gpr[5], IOMUXC_GPR5_PCIE_BTNRST);
	setbits_le32(&gpc_regs->cntr, PCIE_PHY_PUP_REQ);
}

#ifdef CONFIG_SPL_BUILD
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

static iomux_v3_cfg_t const pcie_pads[] = {
	MX6_PAD_NAND_DATA02__GPIO4_IO_6 | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static iomux_v3_cfg_t const uart_pads[] = {
	MX6_PAD_GPIO1_IO04__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_GPIO1_IO05__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__USDHC4_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__USDHC4_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA0__USDHC4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA1__USDHC4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA2__USDHC4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA3__USDHC4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA4__USDHC4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA5__USDHC4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA6__USDHC4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA7__USDHC4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static void vining2000_spl_setup_iomux_pcie(void)
{
	imx_iomux_v3_setup_multiple_pads(pcie_pads, ARRAY_SIZE(pcie_pads));
}

static void vining2000_spl_setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static struct fsl_esdhc_cfg usdhc_cfg = { USDHC4_BASE_ADDR };

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc4_pads, ARRAY_SIZE(usdhc4_pads));

	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
	gd->arch.sdhc_clk = usdhc_cfg.sdhc_clk;
	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

const struct mx6sx_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_dqm0		= 0x00000028,
	.dram_dqm1		= 0x00000028,
	.dram_dqm2		= 0x00000028,
	.dram_dqm3		= 0x00000028,
	.dram_ras		= 0x00000028,
	.dram_cas		= 0x00000028,
	.dram_odt0		= 0x00000028,
	.dram_odt1		= 0x00000028,
	.dram_sdba2		= 0x00000000,
	.dram_sdcke0		= 0x00003000,
	.dram_sdcke1		= 0x00003000,
	.dram_sdclk_0		= 0x00000030,
	.dram_sdqs0		= 0x00000028,
	.dram_sdqs1		= 0x00000028,
	.dram_sdqs2		= 0x00000028,
	.dram_sdqs3		= 0x00000028,
	.dram_reset		= 0x00000028,
};

const struct mx6sx_iomux_grp_regs mx6_grp_ioregs = {
	.grp_addds		= 0x00000028,
	.grp_b0ds		= 0x00000028,
	.grp_b1ds		= 0x00000028,
	.grp_b2ds		= 0x00000028,
	.grp_b3ds		= 0x00000028,
	.grp_ctlds		= 0x00000028,
	.grp_ddr_type		= 0x000c0000,
	.grp_ddrmode		= 0x00020000,
	.grp_ddrmode_ctl	= 0x00020000,
	.grp_ddrpke		= 0x00000000,
};

const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0		= 0x0022001C,
	.p0_mpwldectrl1		= 0x001F001A,
	.p0_mpdgctrl0		= 0x01380134,
	.p0_mpdgctrl1		= 0x0124011C,
	.p0_mprddlctl		= 0x42404444,
	.p0_mpwrdlctl		= 0x36383C38,
};

static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed	= 1600,
	.density	= 4,
	.width		= 32,
	.banks		= 8,
	.rowaddr	= 15,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1391,
	.trcmin		= 4875,
	.trasmin	= 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xF000000F, &ccm->CCGR0);	/* AIPS_TZ{1,2,3} */
	writel(0x303C0000, &ccm->CCGR1);	/* GPT, OCRAM */
	writel(0x00FFFCC0, &ccm->CCGR2);	/* IPMUX, I2C1, I2C3 */
	writel(0x3F300030, &ccm->CCGR3);	/* OCRAM, MMDC, ENET */
	writel(0x0000C003, &ccm->CCGR4);	/* PCI, PL301 */
	writel(0x0F0330C3, &ccm->CCGR5);	/* UART, ROM */
	writel(0x00000F00, &ccm->CCGR6);	/* SDHI4, EIM */
}

static void vining2000_spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		.dsize		= mem_ddr.width / 32,
		.cs_density	= 24,
		.ncs		= 1,
		.cs1_mirror	= 0,
		.rtt_wr		= 1,	/* RTT_wr = RZQ/4 */
		.rtt_nom	= 1,	/* RTT_Nom = RZQ/4 */
		.walat		= 1,	/* Write additional latency */
		.ralat		= 5,	/* Read additional latency */
		.mif3_mode	= 3,	/* Command prediction working mode */
		.bi_on		= 1,	/* Bank interleaving enabled */
		.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type	= DDR_TYPE_DDR3,
		.refsel		= 1,	/* Refresh cycles at 32KHz */
		.refr		= 7,	/* 8 refresh commands per refresh cycle */
	};

	mx6sx_dram_iocfg(mem_ddr.width, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);

	/* Perform DDR DRAM calibration */
	udelay(100);
	mmdc_do_write_level_calibration(&sysinfo);
	mmdc_do_dqs_calibration(&sysinfo);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();

	/* iomux setup */
	vining2000_spl_setup_iomux_pcie();
	vining2000_spl_setup_iomux_uart();

	/* setup GP timer */
	timer_init();

	/* reset the PCIe device */
	gpio_set_value(IMX_GPIO_NR(4, 6), 1);
	udelay(50);
	gpio_set_value(IMX_GPIO_NR(4, 6), 0);

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	vining2000_spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
