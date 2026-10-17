// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <adc.h>
#include <log.h>
#include <net.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <bootm.h>
#include <clk.h>
#include <config.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <env.h>
#include <env_internal.h>
#include <g_dnl.h>
#include <generic-phy.h>
#include <hang.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <init.h>
#include <led.h>
#include <memalign.h>
#include <misc.h>
#include <mtd.h>
#include <mtd_node.h>
#include <netdev.h>
#include <phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <power/regulator.h>
#include <remoteproc.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <watchdog.h>
#include <dm/ofnode.h>
#include "../common/dh_common.h"
#include "../../st/common/stpmic1.h"

/* SYSCFG registers */
#define SYSCFG_BOOTR		0x00
#define SYSCFG_PMCSETR		0x04
#define SYSCFG_IOCTRLSETR	0x18
#define SYSCFG_ICNR		0x1C
#define SYSCFG_CMPCR		0x20
#define SYSCFG_CMPENSETR	0x24
#define SYSCFG_PMCCLRR		0x44

#define SYSCFG_BOOTR_BOOT_MASK		GENMASK(2, 0)
#define SYSCFG_BOOTR_BOOTPD_SHIFT	4

#define SYSCFG_IOCTRLSETR_HSLVEN_TRACE		BIT(0)
#define SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI	BIT(1)
#define SYSCFG_IOCTRLSETR_HSLVEN_ETH		BIT(2)
#define SYSCFG_IOCTRLSETR_HSLVEN_SDMMC		BIT(3)
#define SYSCFG_IOCTRLSETR_HSLVEN_SPI		BIT(4)

#define SYSCFG_CMPCR_SW_CTRL		BIT(1)
#define SYSCFG_CMPCR_READY		BIT(8)

#define SYSCFG_CMPENSETR_MPU_EN		BIT(0)

#define SYSCFG_PMCSETR_ETH_CLK_SEL	BIT(16)
#define SYSCFG_PMCSETR_ETH_REF_CLK_SEL	BIT(17)

#define SYSCFG_PMCSETR_ETH_SELMII	BIT(20)

#define SYSCFG_PMCSETR_ETH_SEL_MASK	GENMASK(23, 21)
#define SYSCFG_PMCSETR_ETH_SEL_GMII_MII	0
#define SYSCFG_PMCSETR_ETH_SEL_RGMII	BIT(21)
#define SYSCFG_PMCSETR_ETH_SEL_RMII	BIT(23)

#define KS_CCR		0x08
#define KS_CCR_EEPROM	BIT(9)
#define KS_BE0		BIT(12)
#define KS_BE1		BIT(13)
#define KS_CIDER	0xC0
#define CIDER_ID	0x8870

static bool dh_stm32_mac_is_in_ks8851(void)
{
	ofnode node;
	u32 reg, cider, ccr;

	node = ofnode_path("ethernet1");
	if (!ofnode_valid(node))
		return false;

	if (ofnode_device_is_compatible(node, "micrel,ks8851-mll"))
		return false;

	/*
	 * KS8851 with EEPROM may use custom MAC from EEPROM, read
	 * out the KS8851 CCR register to determine whether EEPROM
	 * is present. If EEPROM is present, it must contain valid
	 * MAC address.
	 */
	reg = ofnode_get_addr(node);
	if (!reg)
		return false;

	writew(KS_BE0 | KS_BE1 | KS_CIDER, reg + 2);
	cider = readw(reg);
	if ((cider & 0xfff0) != CIDER_ID)
		return true;

	writew(KS_BE0 | KS_BE1 | KS_CCR, reg + 2);
	ccr = readw(reg);
	if (ccr & KS_CCR_EEPROM)
		return true;

	return false;
}

static int dh_stm32_setup_ethaddr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		return eth_env_set_enetaddr("ethaddr", enetaddr);

	return -ENXIO;
}

static int dh_stm32_setup_eth1addr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("eth1addr"))
		return 0;

	if (dh_stm32_mac_is_in_ks8851())
		return 0;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0")) {
		enetaddr[5]++;
		return eth_env_set_enetaddr("eth1addr", enetaddr);
	}

	return -ENXIO;
}

int setup_mac_address(void)
{
	if (dh_stm32_setup_ethaddr())
		log_err("%s: Unable to setup ethaddr!\n", __func__);

	if (dh_stm32_setup_eth1addr())
		log_err("%s: Unable to setup eth1addr!\n", __func__);

	return 0;
}

int checkboard(void)
{
	char *mode;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_TFABOOT))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = ofnode_get_property(ofnode_root(), "compatible",
					 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
static u8 brdcode __section(".data");
static u8 ddr3code __section(".data");
static u8 somcode __section(".data");
static u32 opp_voltage_mv __section(".data");

static void board_get_coding_straps(void)
{
	struct gpio_desc gpio[4];
	ofnode node;
	int i, ret;

	brdcode = 0;
	ddr3code = 0;
	somcode = 0;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		printf("%s: no /config node?\n", __func__);
		return;
	}

	ret = gpio_request_list_by_name_nodev(node, "dh,som-coding-gpios",
					      gpio, ARRAY_SIZE(gpio),
					      GPIOD_IS_IN);
	for (i = 0; i < ret; i++)
		somcode |= !!dm_gpio_get_value(&(gpio[i])) << i;

	gpio_free_list_nodev(gpio, ret);

	ret = gpio_request_list_by_name_nodev(node, "dh,ddr3-coding-gpios",
					      gpio, ARRAY_SIZE(gpio),
					      GPIOD_IS_IN);
	for (i = 0; i < ret; i++)
		ddr3code |= !!dm_gpio_get_value(&(gpio[i])) << i;

	gpio_free_list_nodev(gpio, ret);

	ret = gpio_request_list_by_name_nodev(node, "dh,board-coding-gpios",
					      gpio, ARRAY_SIZE(gpio),
					      GPIOD_IS_IN);
	for (i = 0; i < ret; i++)
		brdcode |= !!dm_gpio_get_value(&(gpio[i])) << i;

	gpio_free_list_nodev(gpio, ret);

	if (CONFIG_IS_ENABLED(DISPLAY_PRINT))
		printf("Code:  SoM:rev=%d,ddr3=%d Board:rev=%d\n",
		       somcode, ddr3code, brdcode);
}

int board_stm32mp1_ddr_config_name_match(struct udevice *dev,
					 const char *name)
{
	if (ddr3code == 1 &&
	    !strcmp(name, "st,ddr3l-dhsom-1066-888-bin-g-2x1gb-533mhz"))
		return 0;

	if (ddr3code == 2 &&
	    !strcmp(name, "st,ddr3l-dhsom-1066-888-bin-g-2x2gb-533mhz"))
		return 0;

	if (ddr3code == 3 &&
	    !strcmp(name, "st,ddr3l-dhsom-1066-888-bin-g-2x4gb-533mhz"))
		return 0;

	return -EINVAL;
}

void board_vddcore_init(u32 voltage_mv)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		opp_voltage_mv = voltage_mv;
}

int board_early_init_f(void)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		stpmic1_init(opp_voltage_mv);
	board_get_coding_straps();

	return 0;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	const char *compat;
	char test[128];

	compat = ofnode_get_property(ofnode_root(), "compatible", NULL);

	snprintf(test, sizeof(test), "%s_somrev%d_boardrev%d",
		compat, somcode, brdcode);

	if (!strcmp(name, test))
		return 0;

	return -EINVAL;
}
#endif
#endif

static void board_key_check(void)
{
#if defined(CONFIG_FASTBOOT) || defined(CONFIG_CMD_STM32PROG)
	ofnode node;
	struct gpio_desc gpio;
	enum forced_boot_mode boot_mode = BOOT_NORMAL;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		debug("%s: no /config node?\n", __func__);
		return;
	}
#ifdef CONFIG_FASTBOOT
	if (gpio_request_by_name_nodev(node, "st,fastboot-gpios", 0,
				       &gpio, GPIOD_IS_IN)) {
		debug("%s: could not find a /config/st,fastboot-gpios\n",
		      __func__);
	} else {
		if (dm_gpio_get_value(&gpio)) {
			puts("Fastboot key pressed, ");
			boot_mode = BOOT_FASTBOOT;
		}

		dm_gpio_free(NULL, &gpio);
	}
#endif
#ifdef CONFIG_CMD_STM32PROG
	if (gpio_request_by_name_nodev(node, "st,stm32prog-gpios", 0,
				       &gpio, GPIOD_IS_IN)) {
		debug("%s: could not find a /config/st,stm32prog-gpios\n",
		      __func__);
	} else {
		if (dm_gpio_get_value(&gpio)) {
			puts("STM32Programmer key pressed, ");
			boot_mode = BOOT_STM32PROG;
		}
		dm_gpio_free(NULL, &gpio);
	}
#endif

	if (boot_mode != BOOT_NORMAL) {
		puts("entering download mode...\n");
		clrsetbits_le32(TAMP_BOOT_CONTEXT,
				TAMP_BOOT_FORCED_MASK,
				boot_mode);
	}
#endif
}

#if defined(CONFIG_USB_GADGET) && defined(CONFIG_USB_GADGET_DWC2_OTG)

#include <usb/dwc2_udc.h>
int g_dnl_board_usb_cable_connected(void)
{
	struct udevice *dwc2_udc_otg;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_USB_GADGET_GENERIC,
					  DM_DRIVER_GET(dwc2_udc_otg),
					  &dwc2_udc_otg);
	if (!ret)
		debug("dwc2_udc_otg init failed\n");

	return dwc2_udc_B_session_valid(dwc2_udc_otg);
}

#define STM32MP1_G_DNL_DFU_PRODUCT_NUM 0xdf11
#define STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM 0x0afb

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (!strcmp(name, "usb_dnl_dfu"))
		put_unaligned(STM32MP1_G_DNL_DFU_PRODUCT_NUM, &dev->idProduct);
	else if (!strcmp(name, "usb_dnl_fastboot"))
		put_unaligned(STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM,
			      &dev->idProduct);
	else
		put_unaligned(CONFIG_USB_GADGET_PRODUCT_NUM, &dev->idProduct);

	return 0;
}

#endif /* CONFIG_USB_GADGET */

#ifdef CONFIG_LED
static int get_led(struct udevice **dev, char *led_string)
{
	const char *led_name;
	int ret;

	led_name = ofnode_conf_read_str(led_string);
	if (!led_name) {
		pr_debug("%s: could not find %s config string\n",
			 __func__, led_string);
		return -ENOENT;
	}
	ret = led_get_by_label(led_name, dev);
	if (ret) {
		debug("%s: get=%d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int setup_led(enum led_state_t cmd)
{
	struct udevice *dev;
	int ret;

	ret = get_led(&dev, "u-boot,boot-led");
	if (ret)
		return ret;

	ret = led_set_state(dev, cmd);
	return ret;
}
#endif

static void __maybe_unused led_error_blink(u32 nb_blink)
{
#ifdef CONFIG_LED
	int ret;
	struct udevice *led;
	u32 i;
#endif

	if (!nb_blink)
		return;

#ifdef CONFIG_LED
	ret = get_led(&led, "u-boot,error-led");
	if (!ret) {
		/* make u-boot,error-led blinking */
		/* if U32_MAX and 125ms interval, for 17.02 years */
		for (i = 0; i < 2 * nb_blink; i++) {
			led_set_state(led, LEDST_TOGGLE);
			mdelay(125);
			schedule();
		}
	}
#endif

	/* infinite: the boot process must be stopped */
	if (nb_blink == U32_MAX)
		hang();
}

static void sysconf_init(void)
{
#ifndef CONFIG_TFABOOT
	u8 *syscfg;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *pwr_dev;
	struct udevice *pwr_reg;
	struct udevice *dev;
	int ret;
	u32 otp = 0;
#endif
	u32 bootr;

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	/* interconnect update : select master using the port 1 */
	/* LTDC = AXI_M9 */
	/* GPU  = AXI_M8 */
	/* today information is hardcoded in U-Boot */
	writel(BIT(9), syscfg + SYSCFG_ICNR);

	/* disable Pull-Down for boot pin connected to VDD */
	bootr = readl(syscfg + SYSCFG_BOOTR);
	bootr &= ~(SYSCFG_BOOTR_BOOT_MASK << SYSCFG_BOOTR_BOOTPD_SHIFT);
	bootr |= (bootr & SYSCFG_BOOTR_BOOT_MASK) << SYSCFG_BOOTR_BOOTPD_SHIFT;
	writel(bootr, syscfg + SYSCFG_BOOTR);

#ifdef CONFIG_DM_REGULATOR
	/* High Speed Low Voltage Pad mode Enable for SPI, SDMMC, ETH, QSPI
	 * and TRACE. Needed above ~50MHz and conditioned by AFMUX selection.
	 * The customer will have to disable this for low frequencies
	 * or if AFMUX is selected but the function not used, typically for
	 * TRACE. Otherwise, impact on power consumption.
	 *
	 * WARNING:
	 *   enabling High Speed mode while VDD>2.7V
	 *   with the OTP product_below_2v5 (OTP 18, BIT 13)
	 *   erroneously set to 1 can damage the IC!
	 *   => U-Boot set the register only if VDD < 2.7V (in DT)
	 *      but this value need to be consistent with board design
	 */
	ret = uclass_get_device_by_driver(UCLASS_PMIC,
					  DM_DRIVER_GET(stm32mp_pwr_pmic),
					  &pwr_dev);
	if (!ret) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (ret) {
			pr_err("Can't find stm32mp_bsec driver\n");
			return;
		}

		ret = misc_read(dev, STM32_BSEC_SHADOW(18), &otp, 4);
		if (ret > 0)
			otp = otp & BIT(13);

		/* get VDD = vdd-supply */
		ret = device_get_supply_regulator(pwr_dev, "vdd-supply",
						  &pwr_reg);

		/* check if VDD is Low Voltage */
		if (!ret) {
			if (regulator_get_value(pwr_reg) < 2700000) {
				writel(SYSCFG_IOCTRLSETR_HSLVEN_TRACE |
				       SYSCFG_IOCTRLSETR_HSLVEN_QUADSPI |
				       SYSCFG_IOCTRLSETR_HSLVEN_ETH |
				       SYSCFG_IOCTRLSETR_HSLVEN_SDMMC |
				       SYSCFG_IOCTRLSETR_HSLVEN_SPI,
				       syscfg + SYSCFG_IOCTRLSETR);

				if (!otp)
					pr_err("product_below_2v5=0: HSLVEN protected by HW\n");
			} else {
				if (otp)
					pr_err("product_below_2v5=1: HSLVEN update is destructive, no update as VDD>2.7V\n");
			}
		} else {
			debug("VDD unknown");
		}
	}
#endif

	/* activate automatic I/O compensation
	 * warning: need to ensure CSI enabled and ready in clock driver
	 */
	writel(SYSCFG_CMPENSETR_MPU_EN, syscfg + SYSCFG_CMPENSETR);

	while (!(readl(syscfg + SYSCFG_CMPCR) & SYSCFG_CMPCR_READY))
		;
	clrbits_le32(syscfg + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);
#endif
}

#ifdef CONFIG_DM_REGULATOR
#define STPMIC_NVM_BUCKS_VOUT_SHR			0xfc
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_1V2		0
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_1V8		1
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_3V0		2
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_3V3		3
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_MASK		GENMASK(1, 0)
#define STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_OFFSET(n)	((((n) - 1) & 3) * 2)
static int board_get_regulator_buck3_nvm_uv_av96(int *uv)
{
	struct udevice *dev;
	u8 bucks_vout = 0;
	const char *prop;
	int len, ret;

	/* Check whether this is Avenger96 board. */
	prop = ofnode_get_property(ofnode_root(), "compatible", &len);
	if (!prop || !len)
		return -ENODEV;

	if (!strstr(prop, "avenger96") && !strstr(prop, "dhcor-testbench"))
		return -EINVAL;

	/* Read out STPMIC1 NVM and determine default Buck3 voltage. */
	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stpmic1_nvm),
					  &dev);
	if (ret)
		return ret;

	ret = misc_read(dev, STPMIC_NVM_BUCKS_VOUT_SHR, &bucks_vout, 1);
	if (ret != 1)
		return -EINVAL;

	bucks_vout >>= STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_OFFSET(3);
	bucks_vout &= STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_MASK;

	if (strstr(prop, "avenger96")) {
		/*
		 * Avenger96 board comes in multiple regulator configurations:
		 * - rev.100 or rev.200 have Buck3 preconfigured to
		 *   3V3 operation on boot and contains extra Enpirion
		 *   EP53A8LQI DCDC converter which supplies the IO.
		 *   Reduce Buck3 voltage to 2V9 to not waste power.
		 * - rev.200L have Buck3 preconfigured to 1V8 operation
		 *   and have no Enpirion EP53A8LQI DCDC anymore, the
		 *   IO is supplied from Buck3.
		 */
		if (bucks_vout == STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_3V3)
			*uv = 2900000;
		else
			*uv = 1800000;
	} else {
		/* Testbench always respects Buck3 NVM settings */
		if (bucks_vout == STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_3V3)
			*uv = 3300000;
		else if (bucks_vout == STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_3V0)
			*uv = 3000000;
		else if (bucks_vout == STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_1V8)
			*uv = 1800000;
		else	/* STPMIC_NVM_BUCKS_VOUT_SHR_BUCK_1V2 */
			*uv = 1200000;
	}

	return 0;
}

static void board_init_regulator_av96(void)
{
	struct udevice *rdev;
	int ret, uv;

	ret = board_get_regulator_buck3_nvm_uv_av96(&uv);
	if (ret)	/* Not Avenger96 board. */
		return;

	ret = regulator_get_by_devname("buck3", &rdev);
	if (ret)
		return;

	/* Adjust Buck3 per preconfigured PMIC voltage from NVM. */
	regulator_set_value(rdev, uv);
	regulator_set_enable(rdev, true);
}

static void board_init_regulator(void)
{
	board_init_regulator_av96();

	regulators_enable_boot_on(_DEBUG);
}
#else
static inline int board_get_regulator_buck3_nvm_uv_av96(int *uv)
{
	return -EINVAL;
}

static inline void board_init_regulator(void) {}
#endif

/* board dependent setup after realloc */
int board_init(void)
{
	board_key_check();

	board_init_regulator();

	sysconf_init();

	return 0;
}

int board_late_init(void)
{
	char *boot_device;
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const void *fdt_compat;
	int fdt_compat_len;

	fdt_compat = ofnode_get_property(ofnode_root(), "compatible",
					 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len) {
		if (strncmp(fdt_compat, "st,", 3) != 0)
			env_set("board_name", fdt_compat);
		else
			env_set("board_name", fdt_compat + 3);
	}
#endif

	/* Check the boot-source to disable bootdelay */
	boot_device = env_get("boot_device");
	if (!strcmp(boot_device, "serial") || !strcmp(boot_device, "usb"))
		env_set("bootdelay", "0");

#ifdef CONFIG_BOARD_EARLY_INIT_F
	env_set_ulong("dh_som_rev", somcode);
	env_set_ulong("dh_board_rev", brdcode);
	env_set_ulong("dh_ddr3_code", ddr3code);
#endif

	return 0;
}

void board_quiesce_devices(void)
{
#ifdef CONFIG_LED
	setup_led(LEDST_OFF);
#endif
}

/* eth init function : weak called in eqos driver */
int board_interface_eth_init(struct udevice *dev,
			     phy_interface_t interface_type)
{
	u8 *syscfg;
	u32 value;
	bool eth_clk_sel_reg = false;
	bool eth_ref_clk_sel_reg = false;

	/* Gigabit Ethernet 125MHz clock selection. */
	eth_clk_sel_reg = dev_read_bool(dev, "st,eth-clk-sel");

	/* Ethernet 50Mhz RMII clock selection */
	eth_ref_clk_sel_reg =
		dev_read_bool(dev, "st,eth-ref-clk-sel");

	syscfg = (u8 *)syscon_get_first_range(STM32MP_SYSCON_SYSCFG);

	if (!syscfg)
		return -ENODEV;

	switch (interface_type) {
	case PHY_INTERFACE_MODE_MII:
		value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
			SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		debug("%s: PHY_INTERFACE_MODE_MII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_GMII:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII;
		debug("%s: PHY_INTERFACE_MODE_GMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (eth_ref_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RMII |
				SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RMII;
		debug("%s: PHY_INTERFACE_MODE_RMII\n", __func__);
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RGMII;
		debug("%s: PHY_INTERFACE_MODE_RGMII\n", __func__);
		break;
	default:
		debug("%s: Do not manage %d interface\n",
		      __func__, interface_type);
		/* Do not manage others interfaces */
		return -EINVAL;
	}

	/* clear and set ETH configuration bits */
	writel(SYSCFG_PMCSETR_ETH_SEL_MASK | SYSCFG_PMCSETR_ETH_SELMII |
	       SYSCFG_PMCSETR_ETH_REF_CLK_SEL | SYSCFG_PMCSETR_ETH_CLK_SEL,
	       syscfg + SYSCFG_PMCCLRR);
	writel(value, syscfg + SYSCFG_PMCSETR);

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	const char *buck3path = "/soc/i2c@5c002000/stpmic@33/regulators/buck3";
	int buck3off, ret, uv;

	ret = board_get_regulator_buck3_nvm_uv_av96(&uv);
	if (ret)	/* Not Avenger96 board, do not patch Buck3 in DT. */
		return 0;

	buck3off = fdt_path_offset(blob, buck3path);
	if (buck3off < 0)	/* No Buck3 regulator found. */
		return 0;

	ret = fdt_setprop_u32(blob, buck3off, "regulator-min-microvolt", uv);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_u32(blob, buck3off, "regulator-max-microvolt", uv);
	if (ret < 0)
		return ret;

	return 0;
}
#endif

static void board_copro_image_process(ulong fw_image, size_t fw_size)
{
	int ret, id = 0; /* Copro id fixed to 0 as only one coproc on mp1 */

	if (!rproc_is_initialized())
		if (rproc_init()) {
			printf("Remote Processor %d initialization failed\n",
			       id);
			return;
		}

	ret = rproc_load(id, fw_image, fw_size);
	printf("Load Remote Processor %d with data@addr=0x%08lx %u bytes:%s\n",
	       id, fw_image, fw_size, ret ? " Failed!" : " Success!");

	if (!ret) {
		rproc_start(id);
		env_set("copro_state", "booted");
	}
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_COPRO, board_copro_image_process);
