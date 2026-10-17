// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/test.h>
#include <test/ut.h>

static char buf[64];
#define test_muxing(selector, expected) do { \
	ut_assertok(pinctrl_get_pin_muxing(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(expected, (char *)&buf); \
} while (0)

#define test_muxing_regaddr(selector, regaddr, expected) do { \
	char estr[64] = { 0 }; \
	if (IS_ENABLED(CONFIG_PHYS_64BIT)) \
		snprintf(estr, sizeof(estr), "0x%016llx %s", (u64)regaddr, expected); \
	else \
		snprintf(estr, sizeof(estr), "0x%08x %s", (u32)regaddr, expected); \
	ut_assertok(pinctrl_get_pin_muxing(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(estr, (char *)&buf); \
} while (0)

#define test_name(selector, expected) do { \
	ut_assertok(pinctrl_get_pin_name(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(expected, (char *)&buf); \
} while (0)

static int dm_test_pinmux(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL, "pinctrl", &dev));
	test_muxing(0, "UART TX.");
	test_muxing(1, "UART RX.");
	test_muxing(2, "I2S SCK.");
	test_muxing(3, "I2S SD.");
	test_muxing(4, "I2S WS.");
	test_muxing(5, "GPIO0 bias-pull-up input-disable.");
	test_muxing(6, "GPIO1 drive-open-drain.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	ut_assertok(pinctrl_select_state(dev, "alternate"));
	test_muxing(0, "I2C SCL drive-open-drain.");
	test_muxing(1, "I2C SDA drive-open-drain.");
	test_muxing(2, "SPI SCLK.");
	test_muxing(3, "SPI MOSI.");
	test_muxing(4, "SPI MISO.");
	test_muxing(5, "SPI CS0.");
	test_muxing(6, "SPI CS1.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	ut_assertok(pinctrl_select_state(dev, "0"));
	test_muxing(0, "I2C SCL drive-open-drain.");
	test_muxing(1, "I2C SDA drive-open-drain.");
	test_muxing(2, "I2S SCK.");
	test_muxing(3, "I2S SD.");
	test_muxing(4, "I2S WS.");
	test_muxing(5, "GPIO0 bias-pull-up input-disable.");
	test_muxing(6, "GPIO1 drive-open-drain.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	return 0;
}

DM_TEST(dm_test_pinmux, UT_TESTF_SCAN_FDT);

static int dm_test_pinctrl_single(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_PINCTRL,
					"pinctrl-single-no-width", &dev);
	ut_asserteq(-EINVAL, ret);
	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "pwm", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_SERIAL, "serial", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI, "spi@0", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL,
					      "pinctrl-single-pins", &dev));
	ut_asserteq(142, pinctrl_get_pins_count(dev));
	test_name(0, "PIN0");
	test_name(141, "PIN141");
	test_name(142, "Error");
	test_muxing_regaddr(0, 0x0, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(18, 0x48, "0x00000006 pinmux_pwm_pins");
	test_muxing_regaddr(28, 0x70, "0x00000030 pinmux_uart0_pins");
	test_muxing_regaddr(29, 0x74, "0x00000000 pinmux_uart0_pins");
	test_muxing_regaddr(100, 0x190, "0x0000000c pinmux_spi0_pins");
	test_muxing_regaddr(101, 0x194, "0x0000000c pinmux_spi0_pins");
	test_muxing_regaddr(102, 0x198, "0x00000023 pinmux_spi0_pins");
	test_muxing_regaddr(103, 0x19c, "0x0000000c pinmux_spi0_pins");
	ret = pinctrl_get_pin_muxing(dev, 142, buf, sizeof(buf));
	ut_asserteq(-EINVAL, ret);
	ut_assertok(uclass_get_device_by_name(UCLASS_I2C, "i2c@0", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_VIDEO, "lcd", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL,
					      "pinctrl-single-bits", &dev));
	ut_asserteq(160, pinctrl_get_pins_count(dev));
	test_name(0, "PIN0");
	test_name(159, "PIN159");
	test_name(160, "Error");
	test_muxing_regaddr(0, 0x0, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(34, 0x10, "0x00000200 pinmux_i2c0_pins");
	test_muxing_regaddr(35, 0x10, "0x00002000 pinmux_i2c0_pins");
	test_muxing_regaddr(130, 0x40, "0x00000200 pinmux_lcd_pins");
	test_muxing_regaddr(131, 0x40, "0x00002000 pinmux_lcd_pins");
	test_muxing_regaddr(132, 0x40, "0x00020000 pinmux_lcd_pins");
	test_muxing_regaddr(133, 0x40, "0x00200000 pinmux_lcd_pins");
	test_muxing_regaddr(134, 0x40, "0x02000000 pinmux_lcd_pins");
	test_muxing_regaddr(135, 0x40, "0x20000000 pinmux_lcd_pins");
	test_muxing_regaddr(136, 0x44, "0x00000002 pinmux_lcd_pins");
	test_muxing_regaddr(137, 0x44, "0x00000020 pinmux_lcd_pins");
	test_muxing_regaddr(138, 0x44, "0x00000200 pinmux_lcd_pins");
	test_muxing_regaddr(139, 0x44, "0x00002000 pinmux_lcd_pins");
	test_muxing_regaddr(140, 0x44, "0x00020000 pinmux_lcd_pins");
	test_muxing_regaddr(141, 0x44, "0x00200000 pinmux_lcd_pins");
	test_muxing_regaddr(142, 0x44, "0x02000000 pinmux_lcd_pins");
	test_muxing_regaddr(143, 0x44, "0x20000000 pinmux_lcd_pins");
	test_muxing_regaddr(144, 0x48, "0x00000002 pinmux_lcd_pins");
	test_muxing_regaddr(145, 0x48, "0x00000020 pinmux_lcd_pins");
	test_muxing_regaddr(146, 0x48, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(147, 0x48, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(148, 0x48, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(149, 0x48, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(150, 0x48, "0x02000000 pinmux_lcd_pins");
	test_muxing_regaddr(151, 0x48, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(152, 0x4c, "0x00000002 pinmux_lcd_pins");
	test_muxing_regaddr(153, 0x4c, "0x00000020 pinmux_lcd_pins");
	test_muxing_regaddr(154, 0x4c, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(155, 0x4c, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(156, 0x4c, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(157, 0x4c, "0x00000000 UNCLAIMED");
	test_muxing_regaddr(158, 0x4c, "0x02000000 pinmux_lcd_pins");
	test_muxing_regaddr(159, 0x4c, "0x00000000 UNCLAIMED");
	ret = pinctrl_get_pin_muxing(dev, 160, buf, sizeof(buf));
	ut_asserteq(-EINVAL, ret);
	return 0;
}

DM_TEST(dm_test_pinctrl_single, UT_TESTF_SCAN_FDT);
