// SPDX-License-Identifier: GPL-2.0+
/*
 * Copied from simple-panel
 * Copyright (c) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2018 Sjoerd Simons <sjoerd.simons@collabora.co.uk>
 * Modified by Moses Christopher <BollavarapuMoses.Christopher@in.bosch.com>
 *
 * Panel Initialization for HX8238D panel from Himax
 * Resolution: 320x240
 * Color-Mode: RGB
 *
 */

#include <common.h>
#include <dm.h>
#include <panel.h>
#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

/* Register Address */
#define HX8238D_OUTPUT_CTRL_ADDR        0x01
#define HX8238D_LCD_AC_CTRL_ADDR        0x02
#define HX8238D_POWER_CTRL_1_ADDR       0x03
#define HX8238D_DATA_CLR_CTRL_ADDR      0X04
#define HX8238D_FUNCTION_CTRL_ADDR      0x05
#define HX8238D_LED_CTRL_ADDR           0x08
#define HX8238D_CONT_BRIGHT_CTRL_ADDR   0x0A
#define HX8238D_FRAME_CYCLE_CTRL_ADDR   0x0B
#define HX8238D_POWER_CTRL_2_ADDR       0x0D
#define HX8238D_POWER_CTRL_3_ADDR       0x0E
#define HX8238D_GATE_SCAN_POS_ADDR      0x0F
#define HX8238D_HORIZONTAL_PORCH_ADDR   0x16
#define HX8238D_VERTICAL_PORCH_ADDR     0x17
#define HX8238D_POWER_CTRL_4_ADDR       0x1E
#define HX8238D_GAMMA_CTRL_1_ADDR       0x30
#define HX8238D_GAMMA_CTRL_2_ADDR       0x31
#define HX8238D_GAMMA_CTRL_3_ADDR       0x32
#define HX8238D_GAMMA_CTRL_4_ADDR       0x33
#define HX8238D_GAMMA_CTRL_5_ADDR       0x34
#define HX8238D_GAMMA_CTRL_6_ADDR       0x35
#define HX8238D_GAMMA_CTRL_7_ADDR       0x36
#define HX8238D_GAMMA_CTRL_8_ADDR       0x37
#define HX8238D_GAMMA_CTRL_9_ADDR       0x3A
#define HX8238D_GAMMA_CTRL_10_ADDR      0x3B

/* Register Data */
#define HX8238D_OUTPUT_CTRL             0x6300
#define HX8238D_LCD_AC_CTRL             0x0200
#define HX8238D_POWER_CTRL_1            0x6564
#define HX8238D_DATA_CLR_CTRL           0x04C7
#define HX8238D_FUNCTION_CTRL           0xA884
#define HX8238D_LED_CTRL                0x00CE
#define HX8238D_CONT_BRIGHT_CTRL        0x4008
#define HX8238D_FRAME_CYCLE_CTRL        0xD400
#define HX8238D_POWER_CTRL_2            0x3229
#define HX8238D_POWER_CTRL_3            0x1200
#define HX8238D_GATE_SCAN_POS           0x0000
#define HX8238D_HORIZONTAL_PORCH        0x9F80
#define HX8238D_VERTICAL_PORCH          0x3F02
#define HX8238D_POWER_CTRL_4            0x005C

/* Gamma Control */
#define HX8238D_GAMMA_CTRL_1            0x0103
#define HX8238D_GAMMA_CTRL_2            0x0407
#define HX8238D_GAMMA_CTRL_3            0x0705
#define HX8238D_GAMMA_CTRL_4            0x0002
#define HX8238D_GAMMA_CTRL_5            0x0505
#define HX8238D_GAMMA_CTRL_6            0x0303
#define HX8238D_GAMMA_CTRL_7            0x0707
#define HX8238D_GAMMA_CTRL_8            0x0100
#define HX8238D_GAMMA_CTRL_9            0x1F00
#define HX8238D_GAMMA_CTRL_10           0x000F

/* Primary SPI register identification, 011100 */
/* Select register, RS=0, RS=0 */
/* Write  register, RS=1, RW=0 */
#define HX8238D_PRIMARY_SELECT_REG 0x70
#define HX8238D_PRIMARY_WRITE_REG  (HX8238D_PRIMARY_SELECT_REG | (0x1 << 1))

#define HX8238D_REG_BIT_LEN        24

struct hx8238d_priv {
	struct spi_slave *spi;
};

static int hx8238d_ofdata_to_platdata(struct udevice *dev)
{
	struct hx8238d_priv *priv = dev_get_priv(dev);

	priv->spi = dev_get_parent_priv(dev);

	return 0;
}

/* data[0] => REGISTER ADDRESS */
/* data[1] => REGISTER VALUE   */
struct hx8238d_command {
	u16 data[2];
};

static struct hx8238d_command hx8238d_init_commands[] = {
	{ .data = { HX8238D_OUTPUT_CTRL_ADDR,      HX8238D_OUTPUT_CTRL } },
	{ .data = { HX8238D_LCD_AC_CTRL_ADDR,      HX8238D_LCD_AC_CTRL } },
	{ .data = { HX8238D_POWER_CTRL_1_ADDR,     HX8238D_POWER_CTRL_1 } },
	{ .data = { HX8238D_DATA_CLR_CTRL_ADDR,    HX8238D_DATA_CLR_CTRL } },
	{ .data = { HX8238D_FUNCTION_CTRL_ADDR,    HX8238D_FUNCTION_CTRL } },
	{ .data = { HX8238D_LED_CTRL_ADDR,         HX8238D_LED_CTRL } },
	{ .data = { HX8238D_CONT_BRIGHT_CTRL_ADDR, HX8238D_CONT_BRIGHT_CTRL } },
	{ .data = { HX8238D_FRAME_CYCLE_CTRL_ADDR, HX8238D_FRAME_CYCLE_CTRL } },
	{ .data = { HX8238D_POWER_CTRL_2_ADDR,     HX8238D_POWER_CTRL_2 } },
	{ .data = { HX8238D_POWER_CTRL_3_ADDR,     HX8238D_POWER_CTRL_3 } },
	{ .data = { HX8238D_GATE_SCAN_POS_ADDR,    HX8238D_GATE_SCAN_POS } },
	{ .data = { HX8238D_HORIZONTAL_PORCH_ADDR, HX8238D_HORIZONTAL_PORCH } },
	{ .data = { HX8238D_VERTICAL_PORCH_ADDR,   HX8238D_VERTICAL_PORCH } },
	{ .data = { HX8238D_POWER_CTRL_4_ADDR,     HX8238D_POWER_CTRL_4 } },
	{ .data = { HX8238D_GAMMA_CTRL_1_ADDR,     HX8238D_GAMMA_CTRL_1 } },
	{ .data = { HX8238D_GAMMA_CTRL_2_ADDR,     HX8238D_GAMMA_CTRL_2 } },
	{ .data = { HX8238D_GAMMA_CTRL_3_ADDR,     HX8238D_GAMMA_CTRL_3 } },
	{ .data = { HX8238D_GAMMA_CTRL_4_ADDR,     HX8238D_GAMMA_CTRL_4 } },
	{ .data = { HX8238D_GAMMA_CTRL_5_ADDR,     HX8238D_GAMMA_CTRL_5 } },
	{ .data = { HX8238D_GAMMA_CTRL_6_ADDR,     HX8238D_GAMMA_CTRL_6 } },
	{ .data = { HX8238D_GAMMA_CTRL_7_ADDR,     HX8238D_GAMMA_CTRL_7 } },
	{ .data = { HX8238D_GAMMA_CTRL_8_ADDR,     HX8238D_GAMMA_CTRL_8 } },
	{ .data = { HX8238D_GAMMA_CTRL_9_ADDR,     HX8238D_GAMMA_CTRL_9 } },
	{ .data = { HX8238D_GAMMA_CTRL_10_ADDR,    HX8238D_GAMMA_CTRL_10 } },
};

/*
 * Generate Primary Register Buffer for Register Select and Register Write
 * First 6 MSB bits of Primary Register is represented with 011100
 *
 */
static void hx8238d_generate_reg_buffers(struct hx8238d_command command,
					 u8 *sr_buf, uint8_t *wr_buf)
{
	struct hx8238d_command cmd = command;

	sr_buf[0] = HX8238D_PRIMARY_SELECT_REG;
	sr_buf[1] = (cmd.data[0] >> 8) & 0xff;
	sr_buf[2] = (cmd.data[0]) & 0xff;

	wr_buf[0] = HX8238D_PRIMARY_WRITE_REG;
	wr_buf[1] = (cmd.data[1] >> 8) & 0xff;
	wr_buf[2] = (cmd.data[1]) & 0xff;
}

static int hx8238d_probe(struct udevice *dev)
{
	struct hx8238d_priv *priv = dev_get_priv(dev);
	int ret;

	ret = spi_claim_bus(priv->spi);
	if (ret) {
		debug("Failed to claim bus: %d\n", ret);
		return ret;
	}

	for (int i = 0; i < ARRAY_SIZE(hx8238d_init_commands); i++) {
		u8 sr_buf[3], wr_buf[3];
		const struct hx8238d_command cmd = hx8238d_init_commands[i];

		hx8238d_generate_reg_buffers(cmd, sr_buf, wr_buf);
		ret = spi_xfer(priv->spi, HX8238D_REG_BIT_LEN, sr_buf, NULL,
			       SPI_XFER_BEGIN | SPI_XFER_END);
		if (ret) {
			debug("Failed to select register %d\n", ret);
			goto free;
		}

		ret = spi_xfer(priv->spi, HX8238D_REG_BIT_LEN, wr_buf, NULL,
			       SPI_XFER_BEGIN | SPI_XFER_END);
		if (ret) {
			debug("Failed to write value %d\n", ret);
			goto free;
		}
	}

free:
	spi_release_bus(priv->spi);
	return ret;
}

static const struct udevice_id hx8238d_ids[] = {
	{ .compatible = "himax,hx8238d" },
	{ }
};

U_BOOT_DRIVER(hx8238d) = {
	.name = "hx8238d",
	.id = UCLASS_PANEL,
	.of_match = hx8238d_ids,
	.of_to_plat = hx8238d_ofdata_to_platdata,
	.probe = hx8238d_probe,
	.priv_auto = sizeof(struct hx8238d_priv),
};
