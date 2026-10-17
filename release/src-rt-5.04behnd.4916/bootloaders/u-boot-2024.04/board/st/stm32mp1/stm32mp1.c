// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_BOARD

#include <common.h>
#include <adc.h>
#include <bootm.h>
#include <clk.h>
#include <config.h>
#include <dm.h>
#include <efi_loader.h>
#include <env.h>
#include <env_internal.h>
#include <fdt_simplefb.h>
#include <fdt_support.h>
#include <g_dnl.h>
#include <generic-phy.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <led.h>
#include <log.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <remoteproc.h>
#include <reset.h>
#include <syscon.h>
#include <usb.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>
#include <dm/ofnode.h>
#include <jffs2/load_kernel.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/printk.h>
#include <power/regulator.h>
#include <usb/dwc2_udc.h>

#include "../../st/common/stusb160x.h"

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

#define USB_LOW_THRESHOLD_UV		200000
#define USB_WARNING_LOW_THRESHOLD_UV	660000
#define USB_START_LOW_THRESHOLD_UV	1230000
#define USB_START_HIGH_THRESHOLD_UV	2150000

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[1];

struct efi_capsule_update_info update_info = {
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	/* nothing to do, only used in SPL */
	return 0;
}

int checkboard(void)
{
	int ret;
	char *mode;
	u32 otp;
	struct udevice *dev;
	const char *fdt_compat;
	int fdt_compat_len;

	if (IS_ENABLED(CONFIG_TFABOOT)) {
		if (IS_ENABLED(CONFIG_STM32MP15X_STM32IMAGE))
			mode = "trusted - stm32image";
		else
			mode = "trusted";
	} else {
		mode = "basic";
	}

	fdt_compat = ofnode_get_property(ofnode_root(), "compatible",
					 &fdt_compat_len);

	log_info("Board: stm32mp1 in %s mode (%s)\n", mode,
		 fdt_compat && fdt_compat_len ? fdt_compat : "");

	/* display the STMicroelectronics board identification */
	if (IS_ENABLED(CONFIG_CMD_STBOARD)) {
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);
		if (!ret)
			ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
					&otp, sizeof(otp));
		if (ret > 0 && otp)
			log_info("Board: MB%04x Var%d.%d Rev.%c-%02d\n",
				 otp >> 16,
				 (otp >> 12) & 0xF,
				 (otp >> 4) & 0xF,
				 ((otp >> 8) & 0xF) - 1 + 'A',
				 otp & 0xF);
	}

	return 0;
}

static void board_key_check(void)
{
	ofnode node;
	struct gpio_desc gpio;
	enum forced_boot_mode boot_mode = BOOT_NORMAL;

	if (!IS_ENABLED(CONFIG_FASTBOOT) && !IS_ENABLED(CONFIG_CMD_STM32PROG))
		return;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		log_debug("no /config node?\n");
		return;
	}
	if (IS_ENABLED(CONFIG_FASTBOOT)) {
		if (gpio_request_by_name_nodev(node, "st,fastboot-gpios", 0,
					       &gpio, GPIOD_IS_IN)) {
			log_debug("could not find a /config/st,fastboot-gpios\n");
		} else {
			udelay(20);
			if (dm_gpio_get_value(&gpio)) {
				log_notice("Fastboot key pressed, ");
				boot_mode = BOOT_FASTBOOT;
			}

			dm_gpio_free(NULL, &gpio);
		}
	}
	if (IS_ENABLED(CONFIG_CMD_STM32PROG)) {
		if (gpio_request_by_name_nodev(node, "st,stm32prog-gpios", 0,
					       &gpio, GPIOD_IS_IN)) {
			log_debug("could not find a /config/st,stm32prog-gpios\n");
		} else {
			udelay(20);
			if (dm_gpio_get_value(&gpio)) {
				log_notice("STM32Programmer key pressed, ");
				boot_mode = BOOT_STM32PROG;
			}
			dm_gpio_free(NULL, &gpio);
		}
	}
	if (boot_mode != BOOT_NORMAL) {
		log_notice("entering download mode...\n");
		clrsetbits_le32(TAMP_BOOT_CONTEXT,
				TAMP_BOOT_FORCED_MASK,
				boot_mode);
	}
}

int g_dnl_board_usb_cable_connected(void)
{
	struct udevice *dwc2_udc_otg;
	int ret;

	if (!IS_ENABLED(CONFIG_USB_GADGET_DWC2_OTG))
		return -ENODEV;

	/*
	 * In case of USB boot device is detected, consider USB cable is
	 * connected
	 */
	if ((get_bootmode() & TAMP_BOOT_DEVICE_MASK) == BOOT_SERIAL_USB)
		return true;

	/* if typec stusb160x is present, means DK1 or DK2 board */
	ret = stusb160x_cable_connected();
	if (ret >= 0)
		return ret;

	ret = uclass_get_device_by_driver(UCLASS_USB_GADGET_GENERIC,
					  DM_DRIVER_GET(dwc2_udc_otg),
					  &dwc2_udc_otg);
	if (ret) {
		log_debug("dwc2_udc_otg init failed\n");
		return ret;
	}

	return dwc2_udc_B_session_valid(dwc2_udc_otg);
}

#ifdef CONFIG_USB_GADGET_DOWNLOAD
#define STM32MP1_G_DNL_DFU_PRODUCT_NUM 0xdf11
#define STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM 0x0afb

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	if (IS_ENABLED(CONFIG_DFU_OVER_USB) &&
	    !strcmp(name, "usb_dnl_dfu"))
		put_unaligned(STM32MP1_G_DNL_DFU_PRODUCT_NUM, &dev->idProduct);
	else if (IS_ENABLED(CONFIG_FASTBOOT) &&
		 !strcmp(name, "usb_dnl_fastboot"))
		put_unaligned(STM32MP1_G_DNL_FASTBOOT_PRODUCT_NUM,
			      &dev->idProduct);
	else
		put_unaligned(CONFIG_USB_GADGET_PRODUCT_NUM, &dev->idProduct);

	return 0;
}
#endif /* CONFIG_USB_GADGET_DOWNLOAD */

static int get_led(struct udevice **dev, char *led_string)
{
	const char *led_name;
	int ret;

	led_name = ofnode_conf_read_str(led_string);
	if (!led_name) {
		log_debug("could not find %s config string\n", led_string);
		return -ENOENT;
	}
	ret = led_get_by_label(led_name, dev);
	if (ret) {
		log_debug("get=%d\n", ret);
		return ret;
	}

	return 0;
}

static int setup_led(enum led_state_t cmd)
{
	struct udevice *dev;
	int ret;

	if (!CONFIG_IS_ENABLED(LED))
		return 0;

	ret = get_led(&dev, "u-boot,boot-led");
	if (ret)
		return ret;

	ret = led_set_state(dev, cmd);
	return ret;
}

static void __maybe_unused led_error_blink(u32 nb_blink)
{
	int ret;
	struct udevice *led;
	u32 i;

	if (!nb_blink)
		return;

	if (CONFIG_IS_ENABLED(LED)) {
		ret = get_led(&led, "u-boot,error-led");
		if (!ret) {
			/* make u-boot,error-led blinking */
			/* if U32_MAX and 125ms interval, for 17.02 years */
			for (i = 0; i < 2 * nb_blink; i++) {
				led_set_state(led, LEDST_TOGGLE);
				mdelay(125);
				schedule();
			}
			led_set_state(led, LEDST_ON);
		}
	}

	/* infinite: the boot process must be stopped */
	if (nb_blink == U32_MAX)
		hang();
}

static int adc_measurement(ofnode node, int adc_count, int *min_uV, int *max_uV)
{
	struct ofnode_phandle_args adc_args;
	struct udevice *adc;
	unsigned int raw;
	int ret, uV;
	int i;

	for (i = 0; i < adc_count; i++) {
		if (ofnode_parse_phandle_with_args(node, "st,adc_usb_pd",
						   "#io-channel-cells", 0, i,
						   &adc_args)) {
			log_debug("can't find /config/st,adc_usb_pd\n");
			return 0;
		}

		ret = uclass_get_device_by_ofnode(UCLASS_ADC, adc_args.node,
						  &adc);

		if (ret) {
			log_err("Can't get adc device(%d)\n", ret);
			return ret;
		}

		ret = adc_channel_single_shot(adc->name, adc_args.args[0],
					      &raw);
		if (ret) {
			log_err("single shot failed for %s[%d]!\n",
				adc->name, adc_args.args[0]);
			return ret;
		}
		/* Convert to uV */
		if (!adc_raw_to_uV(adc, raw, &uV)) {
			if (uV > *max_uV)
				*max_uV = uV;
			if (uV < *min_uV)
				*min_uV = uV;
			log_debug("%s[%02d] = %u, %d uV\n",
				  adc->name, adc_args.args[0], raw, uV);
		} else {
			log_err("Can't get uV value for %s[%d]\n",
				adc->name, adc_args.args[0]);
		}
	}

	return 0;
}

static int board_check_usb_power(void)
{
	ofnode node;
	int max_uV = 0;
	int min_uV = USB_START_HIGH_THRESHOLD_UV;
	int adc_count, ret;
	u32 nb_blink;
	u8 i;

	if (!IS_ENABLED(CONFIG_ADC))
		return -ENODEV;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		log_debug("no /config node?\n");
		return -ENOENT;
	}

	/*
	 * Retrieve the ADC channels devices and get measurement
	 * for each of them
	 */
	adc_count = ofnode_count_phandle_with_args(node, "st,adc_usb_pd",
						   "#io-channel-cells", 0);
	if (adc_count < 0) {
		if (adc_count == -ENOENT)
			return 0;

		log_err("Can't find adc channel (%d)\n", adc_count);

		return adc_count;
	}

	/* perform maximum of 2 ADC measurements to detect power supply current */
	for (i = 0; i < 2; i++) {
		ret = adc_measurement(node, adc_count, &min_uV, &max_uV);
		if (ret)
			return ret;

		/*
		 * If highest value is inside 1.23 Volts and 2.10 Volts, that means
		 * board is plugged on an USB-C 3A power supply and boot process can
		 * continue.
		 */
		if (max_uV > USB_START_LOW_THRESHOLD_UV &&
		    max_uV <= USB_START_HIGH_THRESHOLD_UV &&
		    min_uV <= USB_LOW_THRESHOLD_UV)
			return 0;

		if (i == 0) {
			log_err("Previous ADC measurements was not the one expected, retry in 20ms\n");
			mdelay(20);  /* equal to max tPDDebounce duration (min 10ms - max 20ms) */
		}
	}

	log_notice("****************************************************\n");
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
		log_notice("* ERROR USB TYPE-C connection in unattached mode   *\n");
		log_notice("* Check that USB TYPE-C cable is correctly plugged *\n");
		/* with 125ms interval, led will blink for 17.02 years ....*/
		nb_blink = U32_MAX;
	}

	if (max_uV > USB_LOW_THRESHOLD_UV &&
	    max_uV <= USB_WARNING_LOW_THRESHOLD_UV &&
	    min_uV <= USB_LOW_THRESHOLD_UV) {
		log_notice("*        WARNING 500mA power supply detected       *\n");
		nb_blink = 2;
	}

	if (max_uV > USB_WARNING_LOW_THRESHOLD_UV &&
	    max_uV <= USB_START_LOW_THRESHOLD_UV &&
	    min_uV <= USB_LOW_THRESHOLD_UV) {
		log_notice("*       WARNING 1.5A power supply detected        *\n");
		nb_blink = 3;
	}

	/*
	 * If highest value is above 2.15 Volts that means that the USB TypeC
	 * supplies more than 3 Amp, this is not compliant with TypeC specification
	 */
	if (max_uV > USB_START_HIGH_THRESHOLD_UV) {
		log_notice("*      USB TYPE-C charger not compliant with       *\n");
		log_notice("*                   specification                  *\n");
		log_notice("****************************************************\n\n");
		/* with 125ms interval, led will blink for 17.02 years ....*/
		nb_blink = U32_MAX;
	} else {
		log_notice("*     Current too low, use a 3A power supply!      *\n");
		log_notice("****************************************************\n\n");
	}

	led_error_blink(nb_blink);

	return 0;
}

static void sysconf_init(void)
{
	u8 *syscfg;
	struct udevice *pwr_dev;
	struct udevice *pwr_reg;
	struct udevice *dev;
	u32 otp = 0;
	int ret;
	u32 bootr, val;

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
			log_err("Can't find stm32mp_bsec driver\n");
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
					log_err("product_below_2v5=0: HSLVEN protected by HW\n");
			} else {
				if (otp)
					log_err("product_below_2v5=1: HSLVEN update is destructive, no update as VDD>2.7V\n");
			}
		} else {
			log_debug("VDD unknown");
		}
	}

	/* activate automatic I/O compensation
	 * warning: need to ensure CSI enabled and ready in clock driver
	 */
	writel(SYSCFG_CMPENSETR_MPU_EN, syscfg + SYSCFG_CMPENSETR);

	/* poll until ready (1s timeout) */
	ret = readl_poll_timeout(syscfg + SYSCFG_CMPCR, val,
				 val & SYSCFG_CMPCR_READY,
				 1000000);
	if (ret) {
		log_err("SYSCFG: I/O compensation failed, timeout.\n");
		led_error_blink(10);
	}

	clrbits_le32(syscfg + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);
}

static int board_stm32mp15x_dk2_init(void)
{
	ofnode node;
	struct gpio_desc hdmi, audio;
	int ret = 0;

	/* Fix to make I2C1 usable on DK2 for touchscreen usage in kernel */
	node = ofnode_path("/soc/i2c@40012000/hdmi-transmitter@39");
	if (!ofnode_valid(node)) {
		log_debug("no hdmi-transmitter@39 ?\n");
		return -ENOENT;
	}

	if (gpio_request_by_name_nodev(node, "reset-gpios", 0,
				       &hdmi, GPIOD_IS_OUT)) {
		log_debug("could not find reset-gpios\n");
		return -ENOENT;
	}

	node = ofnode_path("/soc/i2c@40012000/cs42l51@4a");
	if (!ofnode_valid(node)) {
		log_debug("no cs42l51@4a ?\n");
		return -ENOENT;
	}

	if (gpio_request_by_name_nodev(node, "reset-gpios", 0,
				       &audio, GPIOD_IS_OUT)) {
		log_debug("could not find reset-gpios\n");
		return -ENOENT;
	}

	/* before power up, insure that HDMI and AUDIO IC is under reset */
	ret = dm_gpio_set_value(&hdmi, 1);
	if (ret) {
		log_err("can't set_value for hdmi_nrst gpio");
		goto error;
	}
	ret = dm_gpio_set_value(&audio, 1);
	if (ret) {
		log_err("can't set_value for audio_nrst gpio");
		goto error;
	}

	/* power-up audio IC */
	regulator_autoset_by_name("v1v8_audio", NULL);

	/* power-up HDMI IC */
	regulator_autoset_by_name("v1v2_hdmi", NULL);
	regulator_autoset_by_name("v3v3_hdmi", NULL);

error:
	return ret;
}

static bool board_is_stm32mp15x_dk2(void)
{
	if (CONFIG_IS_ENABLED(TARGET_ST_STM32MP15X) &&
	    of_machine_is_compatible("st,stm32mp157c-dk2"))
		return true;

	return false;
}

static bool board_is_stm32mp15x_ev1(void)
{
	if (CONFIG_IS_ENABLED(TARGET_ST_STM32MP15X) &&
	    (of_machine_is_compatible("st,stm32mp157a-ev1") ||
	     of_machine_is_compatible("st,stm32mp157c-ev1") ||
	     of_machine_is_compatible("st,stm32mp157d-ev1") ||
	     of_machine_is_compatible("st,stm32mp157f-ev1")))
		return true;

	return false;
}

/* touchscreen driver: only used for pincontrol configuration */
static const struct udevice_id goodix_ids[] = {
	{ .compatible = "goodix,gt9147", },
	{ }
};

U_BOOT_DRIVER(goodix) = {
	.name		= "goodix",
	.id		= UCLASS_NOP,
	.of_match	= goodix_ids,
};

static void board_stm32mp15x_ev1_init(void)
{
	struct udevice *dev;

	/* configure IRQ line on EV1 for touchscreen before LCD reset */
	uclass_get_device_by_driver(UCLASS_NOP, DM_DRIVER_GET(goodix), &dev);
}

/* board dependent setup after realloc */
int board_init(void)
{
	board_key_check();

	if (board_is_stm32mp15x_ev1())
		board_stm32mp15x_ev1_init();

	if (board_is_stm32mp15x_dk2())
		board_stm32mp15x_dk2_init();

	regulators_enable_boot_on(_DEBUG);

	/*
	 * sysconf initialisation done only when U-Boot is running in secure
	 * done in TF-A for TFABOOT.
	 */
	if (IS_ENABLED(CONFIG_ARMV7_NONSEC))
		sysconf_init();

	setup_led(LEDST_ON);

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
	efi_guid_t image_type_guid = STM32MP_FIP_IMAGE_GUID;

	guidcpy(&fw_images[0].image_type_id, &image_type_guid);
	fw_images[0].fw_name = u"STM32MP-FIP";
	fw_images[0].image_index = 1;
#endif
	return 0;
}

int board_late_init(void)
{
	const void *fdt_compat;
	int fdt_compat_len;
	int ret;
	u32 otp;
	struct udevice *dev;
	char buf[10];
	char dtb_name[256];
	int buf_len;

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		fdt_compat = ofnode_get_property(ofnode_root(), "compatible",
						 &fdt_compat_len);
		if (fdt_compat && fdt_compat_len) {
			if (strncmp(fdt_compat, "st,", 3) != 0) {
				env_set("board_name", fdt_compat);
			} else {
				env_set("board_name", fdt_compat + 3);

				buf_len = sizeof(dtb_name);
				strncpy(dtb_name, fdt_compat + 3, buf_len);
				buf_len -= strlen(fdt_compat + 3);
				strncat(dtb_name, ".dtb", buf_len);
				env_set("fdtfile", dtb_name);
			}
		}
		ret = uclass_get_device_by_driver(UCLASS_MISC,
						  DM_DRIVER_GET(stm32mp_bsec),
						  &dev);

		if (!ret)
			ret = misc_read(dev, STM32_BSEC_SHADOW(BSEC_OTP_BOARD),
					&otp, sizeof(otp));
		if (ret > 0 && otp) {
			snprintf(buf, sizeof(buf), "0x%04x", otp >> 16);
			env_set("board_id", buf);

			snprintf(buf, sizeof(buf), "0x%04x",
				 ((otp >> 8) & 0xF) - 1 + 0xA);
			env_set("board_rev", buf);
		}
	}

	/* for DK1/DK2 boards */
	board_check_usb_power();

	return 0;
}

void board_quiesce_devices(void)
{
	setup_led(LEDST_OFF);
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
		log_debug("PHY_INTERFACE_MODE_MII\n");
		break;
	case PHY_INTERFACE_MODE_GMII:
		if (eth_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII |
				SYSCFG_PMCSETR_ETH_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_GMII_MII;
		log_debug("PHY_INTERFACE_MODE_GMII\n");
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (eth_ref_clk_sel_reg)
			value = SYSCFG_PMCSETR_ETH_SEL_RMII |
				SYSCFG_PMCSETR_ETH_REF_CLK_SEL;
		else
			value = SYSCFG_PMCSETR_ETH_SEL_RMII;
		log_debug("PHY_INTERFACE_MODE_RMII\n");
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
		log_debug("PHY_INTERFACE_MODE_RGMII\n");
		break;
	default:
		log_debug("Do not manage %d interface\n",
			  interface_type);
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
	case BOOT_FLASH_SD:
	case BOOT_FLASH_EMMC:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;
		else if (CONFIG_IS_ENABLED(ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NAND:
	case BOOT_FLASH_SPINAND:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_UBI))
			return ENVL_UBI;
		else
			return ENVL_NOWHERE;

	case BOOT_FLASH_NOR:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		else
			return ENVL_NOWHERE;

	default:
		return ENVL_NOWHERE;
	}
}

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

int mmc_get_boot(void)
{
	struct udevice *dev;
	u32 boot_mode = get_bootmode();
	unsigned int instance = (boot_mode & TAMP_BOOT_INSTANCE_MASK) - 1;
	char cmd[20];
	const u32 sdmmc_addr[] = {
		STM32_SDMMC1_BASE,
		STM32_SDMMC2_BASE,
		STM32_SDMMC3_BASE
	};

	if (instance >= ARRAY_SIZE(sdmmc_addr))
		return 0;

	/* search associated sdmmc node in devicetree */
	snprintf(cmd, sizeof(cmd), "mmc@%x", sdmmc_addr[instance]);
	if (uclass_get_device_by_name(UCLASS_MMC, cmd, &dev)) {
		log_err("mmc%d = %s not found in device tree!\n", instance, cmd);
		return 0;
	}

	return dev_seq(dev);
};

const char *env_ext4_get_dev_part(void)
{
	static char *const env_dev_part =
#ifdef CONFIG_ENV_EXT4_DEVICE_AND_PART
		CONFIG_ENV_EXT4_DEVICE_AND_PART;
#else
		"";
#endif
	static char *const dev_part[] = {"0:auto", "1:auto", "2:auto"};

	if (strlen(env_dev_part) > 0)
		return env_dev_part;

	return dev_part[mmc_get_boot()];
}

int mmc_get_env_dev(void)
{
	const int mmc_env_dev = CONFIG_IS_ENABLED(ENV_IS_IN_MMC, (CONFIG_SYS_MMC_ENV_DEV), (-1));

	if (mmc_env_dev >= 0)
		return mmc_env_dev;

	/* use boot instance to select the correct mmc device identifier */
	return mmc_get_boot();
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	fdt_copy_fixed_partitions(blob);

	if (IS_ENABLED(CONFIG_FDT_SIMPLEFB))
		fdt_simplefb_enable_and_mem_rsv(blob);

	return 0;
}
#endif

static void board_copro_image_process(ulong fw_image, size_t fw_size)
{
	int ret, id = 0; /* Copro id fixed to 0 as only one coproc on mp1 */

	if (!rproc_is_initialized())
		if (rproc_init()) {
			log_err("Remote Processor %d initialization failed\n",
				id);
			return;
		}

	ret = rproc_load(id, fw_image, fw_size);
	log_err("Load Remote Processor %d with data@addr=0x%08lx %u bytes:%s\n",
		id, fw_image, fw_size, ret ? " Failed!" : " Success!");

	if (!ret)
		rproc_start(id);
}

U_BOOT_FIT_LOADABLE_HANDLER(IH_TYPE_COPRO, board_copro_image_process);

#if defined(CONFIG_FWU_MULTI_BANK_UPDATE)

#include <fwu.h>

/**
 * fwu_plat_get_bootidx() - Get the value of the boot index
 * @boot_idx: Boot index value
 *
 * Get the value of the bank(partition) from which the platform
 * has booted. This value is passed to U-Boot from the earlier
 * stage bootloader which loads and boots all the relevant
 * firmware images
 *
 */
void fwu_plat_get_bootidx(uint *boot_idx)
{
	*boot_idx = (readl(TAMP_FWU_BOOT_INFO_REG) >>
		    TAMP_FWU_BOOT_IDX_OFFSET) & TAMP_FWU_BOOT_IDX_MASK;
}
#endif /* CONFIG_FWU_MULTI_BANK_UPDATE */
