// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <adc.h>
#include <config.h>
#include <clk.h>
#include <dm.h>
#include <environment.h>
#include <g_dnl.h>
#include <generic-phy.h>
#include <i2c.h>
#include <led.h>
#include <misc.h>
#include <phy.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <power/regulator.h>
#include <usb/dwc2_udc.h>

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

/*
 * Get a global data pointer
 */
DECLARE_GLOBAL_DATA_PTR;

#define USB_LOW_THRESHOLD_UV		200000
#define USB_WARNING_LOW_THRESHOLD_UV	660000
#define USB_START_LOW_THRESHOLD_UV	1230000
#define USB_START_HIGH_THRESHOLD_UV	2150000

int checkboard(void)
{
	int ret;
	char *mode;
	u32 otp;
	struct udevice *dev;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_STM32MP1_TRUSTED))
		mode = "trusted";
	else
		mode = "basic";

	printf("Board: stm32mp1 in %s mode", mode);
	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len)
		printf(" (%s)", fdt_compat);
	puts("\n");

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(stm32mp_bsec),
					  &dev);

	if (!ret)
		ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
				&otp, sizeof(otp));
	if (!ret && otp) {
		printf("Board: MB%04x Var%d Rev.%c-%02d\n",
		       otp >> 16,
		       (otp >> 12) & 0xF,
		       ((otp >> 8) & 0xF) - 1 + 'A',
		       otp & 0xF);
	}

	return 0;
}

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

/* STMicroelectronics STUSB1600 Type-C controller */
#define STUSB1600_CC_CONNECTION_STATUS		0x0E

/* STUSB1600_CC_CONNECTION_STATUS bitfields */
#define STUSB1600_CC_ATTACH			BIT(0)

static int stusb1600_init(struct udevice **dev_stusb1600)
{
	ofnode node;
	struct udevice *dev, *bus;
	int ret;
	u32 chip_addr;

	*dev_stusb1600 = NULL;

	/* if node stusb1600 is present, means DK1 or DK2 board */
	node = ofnode_by_compatible(ofnode_null(), "st,stusb1600");
	if (!ofnode_valid(node))
		return -ENODEV;

	ret = ofnode_read_u32(node, "reg", &chip_addr);
	if (ret)
		return -EINVAL;

	ret = uclass_get_device_by_ofnode(UCLASS_I2C, ofnode_get_parent(node),
					  &bus);
	if (ret) {
		printf("bus for stusb1600 not found\n");
		return -ENODEV;
	}

	ret = dm_i2c_probe(bus, chip_addr, 0, &dev);
	if (!ret)
		*dev_stusb1600 = dev;

	return ret;
}

static int stusb1600_cable_connected(struct udevice *dev)
{
	u8 status;

	if (dm_i2c_read(dev, STUSB1600_CC_CONNECTION_STATUS, &status, 1))
		return 0;

	return status & STUSB1600_CC_ATTACH;
}

#include <usb/dwc2_udc.h>
int g_dnl_board_usb_cable_connected(void)
{
	struct udevice *stusb1600;
	struct udevice *dwc2_udc_otg;
	int ret;

	if (!stusb1600_init(&stusb1600))
		return stusb1600_cable_connected(stusb1600);

	ret = uclass_get_device_by_driver(UCLASS_USB_GADGET_GENERIC,
					  DM_GET_DRIVER(dwc2_udc_otg),
					  &dwc2_udc_otg);
	if (!ret)
		debug("dwc2_udc_otg init failed\n");

	return dwc2_udc_B_session_valid(dwc2_udc_otg);
}
#endif /* CONFIG_USB_GADGET */

static int get_led(struct udevice **dev, char *led_string)
{
	char *led_name;
	int ret;

	led_name = fdtdec_get_config_string(gd->fdt_blob, led_string);
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

static int board_check_usb_power(void)
{
	struct ofnode_phandle_args adc_args;
	struct udevice *adc;
	struct udevice *led;
	ofnode node;
	unsigned int raw;
	int max_uV = 0;
	int min_uV = USB_START_HIGH_THRESHOLD_UV;
	int ret, uV, adc_count;
	u32 nb_blink;
	u8 i;
	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		debug("%s: no /config node?\n", __func__);
		return -ENOENT;
	}

	/*
	 * Retrieve the ADC channels devices and get measurement
	 * for each of them
	 */
	adc_count = ofnode_count_phandle_with_args(node, "st,adc_usb_pd",
						   "#io-channel-cells");
	if (adc_count < 0) {
		if (adc_count == -ENOENT)
			return 0;

		pr_err("%s: can't find adc channel (%d)\n", __func__,
		       adc_count);

		return adc_count;
	}

	for (i = 0; i < adc_count; i++) {
		if (ofnode_parse_phandle_with_args(node, "st,adc_usb_pd",
						   "#io-channel-cells", 0, i,
						   &adc_args)) {
			pr_debug("%s: can't find /config/st,adc_usb_pd\n",
				 __func__);
			return 0;
		}

		ret = uclass_get_device_by_ofnode(UCLASS_ADC, adc_args.node,
						  &adc);

		if (ret) {
			pr_err("%s: Can't get adc device(%d)\n", __func__,
			       ret);
			return ret;
		}

		ret = adc_channel_single_shot(adc->name, adc_args.args[0],
					      &raw);
		if (ret) {
			pr_err("%s: single shot failed for %s[%d]!\n",
			       __func__, adc->name, adc_args.args[0]);
			return ret;
		}
		/* Convert to uV */
		if (!adc_raw_to_uV(adc, raw, &uV)) {
			if (uV > max_uV)
				max_uV = uV;
			if (uV < min_uV)
				min_uV = uV;
			pr_debug("%s: %s[%02d] = %u, %d uV\n", __func__,
				 adc->name, adc_args.args[0], raw, uV);
		} else {
			pr_err("%s: Can't get uV value for %s[%d]\n",
			       __func__, adc->name, adc_args.args[0]);
		}
	}

	/*
	 * If highest value is inside 1.23 Volts and 2.10 Volts, that means
	 * board is plugged on an USB-C 3A power supply and boot process can
	 * continue.
	 */
	if (max_uV > USB_START_LOW_THRESHOLD_UV &&
	    max_uV <= USB_START_HIGH_THRESHOLD_UV &&
	    min_uV <= USB_LOW_THRESHOLD_UV)
		return 0;

	pr_err("****************************************************\n");

	/*
	 * If highest and lowest value are either both below
	 * USB_LOW_THRESHOLD_UV or both above USB_LOW_THRESHOLD_UV, that
	 * means USB TYPE-C is in unattached mode, this is an issue, make
	 * u-boot,error-led blinking and stop boot process.
	 */
	if ((max_uV > USB_LOW_THRESHOLD_UV &&
	     min_uV > USB_LOW_THRESHOLD_UV) ||
	     (max_uV <= USB_LOW_THRESHOLD_UV &&
	     min_uV <= USB_LOW_THRESHOLD_UV)) {
		pr_err("* ERROR USB TYPE-C connection in unattached mode   *\n");
		pr_err("* Check that USB TYPE-C cable is correctly plugged *\n");
		/* with 125ms interval, led will blink for 17.02 years ....*/
		nb_blink = U32_MAX;
	}

	if (max_uV > USB_LOW_THRESHOLD_UV &&
	    max_uV <= USB_WARNING_LOW_THRESHOLD_UV &&
	    min_uV <= USB_LOW_THRESHOLD_UV) {
		pr_err("*        WARNING 500mA power supply detected       *\n");
		nb_blink = 2;
	}

	if (max_uV > USB_WARNING_LOW_THRESHOLD_UV &&
	    max_uV <= USB_START_LOW_THRESHOLD_UV &&
	    min_uV <= USB_LOW_THRESHOLD_UV) {
		pr_err("*       WARNING 1.5mA power supply detected        *\n");
		nb_blink = 3;
	}

	/*
	 * If highest value is above 2.15 Volts that means that the USB TypeC
	 * supplies more than 3 Amp, this is not compliant with TypeC specification
	 */
	if (max_uV > USB_START_HIGH_THRESHOLD_UV) {
		pr_err("*      USB TYPE-C charger not compliant with       *\n");
		pr_err("*                   specification                  *\n");
		pr_err("****************************************************\n\n");
		/* with 125ms interval, led will blink for 17.02 years ....*/
		nb_blink = U32_MAX;
	} else {
		pr_err("*     Current too low, use a 3A power supply!      *\n");
		pr_err("****************************************************\n\n");
	}

	ret = get_led(&led, "u-boot,error-led");
	if (ret) {
		/* in unattached case, the boot process must be stopped */
		if (nb_blink == U32_MAX)
			hang();
		return ret;
	}

	/* make u-boot,error-led blinking */
	for (i = 0; i < nb_blink * 2; i++) {
		led_set_state(led, LEDST_TOGGLE);
		mdelay(125);
	}
	led_set_state(led, LEDST_ON);

	return 0;
}

static void sysconf_init(void)
{
#ifndef CONFIG_STM32MP1_TRUSTED
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
	ret = syscon_get_by_driver_data(STM32MP_SYSCON_PWR, &pwr_dev);
	if (!ret) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_GET_DRIVER(stm32mp_bsec),
						  &dev);
		if (ret) {
			pr_err("Can't find stm32mp_bsec driver\n");
			return;
		}

		ret = misc_read(dev, STM32_BSEC_SHADOW(18), &otp, 4);
		if (!ret)
			otp = otp & BIT(13);

		/* get VDD = pwr-supply */
		ret = device_get_supply_regulator(pwr_dev, "pwr-supply",
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

/* board dependent setup after realloc */
int board_init(void)
{
	struct udevice *dev;

	/* address of boot parameters */
	gd->bd->bi_boot_params = STM32_DDR_BASE + 0x100;

	/* probe all PINCTRL for hog */
	for (uclass_first_device(UCLASS_PINCTRL, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		pr_debug("probe pincontrol = %s\n", dev->name);
	}

	board_key_check();

	sysconf_init();

	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	const void *fdt_compat;
	int fdt_compat_len;

	fdt_compat = fdt_getprop(gd->fdt_blob, 0, "compatible",
				 &fdt_compat_len);
	if (fdt_compat && fdt_compat_len) {
		if (strncmp(fdt_compat, "st,", 3) != 0)
			env_set("board_name", fdt_compat);
		else
			env_set("board_name", fdt_compat + 3);
	}
#endif

	/* for DK1/DK2 boards */
	board_check_usb_power();

	return 0;
}

void board_quiesce_devices(void)
{
	setup_led(LEDST_OFF);
}

/* board interface eth init */
/* this is a weak define that we are overriding */
int board_interface_eth_init(phy_interface_t interface_type,
			     bool eth_clk_sel_reg, bool eth_ref_clk_sel_reg)
{
	u8 *syscfg;
	u32 value;

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

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
#ifdef CONFIG_ENV_IS_IN_EXT4
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		return ENVL_EXT4;
#endif
#ifdef CONFIG_ENV_IS_IN_UBI
	case BOOT_FLASH_NAND:
		return ENVL_UBI;
#endif
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
	case BOOT_FLASH_NOR:
		return ENVL_SPI_FLASH;
#endif
	default:
		return ENVL_NOWHERE;
	}
}

#if defined(CONFIG_ENV_IS_IN_EXT4)
const char *env_ext4_get_intf(void)
{
	u32 bootmode = get_bootmode();

	switch (bootmode & TAMP_BOOT_DEVICE_MASK) {
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		return "mmc";
	default:
		return "";
	}
}

const char *env_ext4_get_dev_part(void)
{
	static char *const dev_part[] = {"0:auto", "1:auto", "2:auto"};
	u32 bootmode = get_bootmode();

	return dev_part[(bootmode & TAMP_BOOT_INSTANCE_MASK) - 1];
}
#endif

#ifdef CONFIG_SYS_MTDPARTS_RUNTIME

#define MTDPARTS_LEN		256
#define MTDIDS_LEN		128

/**
 * The mtdparts_nand0 and mtdparts_nor0 variable tends to be long.
 * If we need to access it before the env is relocated, then we need
 * to use our own stack buffer. gd->env_buf will be too small.
 *
 * @param buf temporary buffer pointer MTDPARTS_LEN long
 * @return mtdparts variable string, NULL if not found
 */
static const char *env_get_mtdparts(const char *str, char *buf)
{
	if (gd->flags & GD_FLG_ENV_READY)
		return env_get(str);
	if (env_get_f(str, buf, MTDPARTS_LEN) != -1)
		return buf;

	return NULL;
}

/**
 * update the variables "mtdids" and "mtdparts" with content of mtdparts_<dev>
 */
static void board_get_mtdparts(const char *dev,
			       char *mtdids,
			       char *mtdparts)
{
	char env_name[32] = "mtdparts_";
	char tmp_mtdparts[MTDPARTS_LEN];
	const char *tmp;

	/* name of env variable to read = mtdparts_<dev> */
	strcat(env_name, dev);
	tmp = env_get_mtdparts(env_name, tmp_mtdparts);
	if (tmp) {
		/* mtdids: "<dev>=<dev>, ...." */
		if (mtdids[0] != '\0')
			strcat(mtdids, ",");
		strcat(mtdids, dev);
		strcat(mtdids, "=");
		strcat(mtdids, dev);

		/* mtdparts: "mtdparts=<dev>:<mtdparts_<dev>>;..." */
		if (mtdparts[0] != '\0')
			strncat(mtdparts, ";", MTDPARTS_LEN);
		else
			strcat(mtdparts, "mtdparts=");
		strncat(mtdparts, dev, MTDPARTS_LEN);
		strncat(mtdparts, ":", MTDPARTS_LEN);
		strncat(mtdparts, tmp, MTDPARTS_LEN);
	}
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct udevice *dev;
	static char parts[2 * MTDPARTS_LEN + 1];
	static char ids[MTDIDS_LEN + 1];
	static bool mtd_initialized;

	if (mtd_initialized) {
		*mtdids = ids;
		*mtdparts = parts;
		return;
	}

	memset(parts, 0, sizeof(parts));
	memset(ids, 0, sizeof(ids));

	if (!uclass_get_device(UCLASS_MTD, 0, &dev))
		board_get_mtdparts("nand0", ids, parts);

	if (!uclass_get_device(UCLASS_SPI_FLASH, 0, &dev))
		board_get_mtdparts("nor0", ids, parts);

	mtd_initialized = true;
	*mtdids = ids;
	*mtdparts = parts;
	debug("%s:mtdids=%s & mtdparts=%s\n", __func__, ids, parts);
}
#endif
