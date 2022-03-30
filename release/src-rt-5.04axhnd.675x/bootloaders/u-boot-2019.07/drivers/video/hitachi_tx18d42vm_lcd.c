// SPDX-License-Identifier: GPL-2.0+
/*
 * Hitachi tx18d42vm LVDS LCD panel driver
 *
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 */

#include <common.h>

#include <asm/gpio.h>
#include <errno.h>

/*
 * Very simple write only SPI support, this does not use the generic SPI infra
 * because that assumes R/W SPI, requiring a MISO pin. Also the necessary glue
 * code alone would be larger then this minimal version.
 */
static void lcd_panel_spi_write(int cs, int clk, int mosi,
				unsigned int data, int bits)
{
	int i, offset;

	gpio_direction_output(cs, 0);
	for (i = 0; i < bits; i++) {
		gpio_direction_output(clk, 0);
		offset = (bits - 1) - i;
		gpio_direction_output(mosi, (data >> offset) & 1);
		udelay(2);
		gpio_direction_output(clk, 1);
		udelay(2);
	}
	gpio_direction_output(cs, 1);
	udelay(2);
}

int hitachi_tx18d42vm_init(void)
{
	const u16 init_data[] = {
		0x0029,		/* reset */
		0x0025,		/* standby */
		0x0840,		/* enable normally black */
		0x0430,		/* enable FRC/dither */
		0x385f,		/* enter test mode(1) */
		0x3ca4,		/* enter test mode(2) */
		0x3409,		/* enable SDRRS, enlarge OE width */
		0x4041,		/* adopt 2 line / 1 dot */
	};
	int i, cs, clk, mosi, ret = 0;

	cs = name_to_gpio(CONFIG_VIDEO_LCD_SPI_CS);
	clk = name_to_gpio(CONFIG_VIDEO_LCD_SPI_SCLK);
	mosi = name_to_gpio(CONFIG_VIDEO_LCD_SPI_MOSI);

	if (cs == -1 || clk == -1 || mosi == 1) {
		printf("Error tx18d42vm spi gpio config is invalid\n");
		return -EINVAL;
	}

	if (gpio_request(cs, "tx18d42vm-spi-cs") != 0 ||
	    gpio_request(clk, "tx18d42vm-spi-clk") != 0 ||
	    gpio_request(mosi, "tx18d42vm-spi-mosi") != 0) {
		printf("Error cannot request tx18d42vm spi gpios\n");
		ret = -EBUSY;
		goto out;
	}

	for (i = 0; i < ARRAY_SIZE(init_data); i++)
		lcd_panel_spi_write(cs, clk, mosi, init_data[i], 16);

	mdelay(50); /* All the tx18d42vm drivers have a delay here ? */

	lcd_panel_spi_write(cs, clk, mosi, 0x00ad, 16); /* display on */

out:
	gpio_free(mosi);
	gpio_free(clk);
	gpio_free(cs);

	return ret;
}
