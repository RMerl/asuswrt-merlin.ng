// SPDX-License-Identifier: GPL-2.0
/*
 * LCD: LG4573, TFT 4.3", 480x800, RGB24
 * LCD initialization via SPI
 *
 */
#include <common.h>
#include <errno.h>
#include <spi.h>

#define PWR_ON_DELAY_MSECS  120

static int lb043wv_spi_write_u16(struct spi_slave *spi, u16 val)
{
	unsigned long flags = SPI_XFER_BEGIN;
	unsigned short buf16 = htons(val);
	int ret = 0;

	flags |= SPI_XFER_END;

	ret = spi_xfer(spi, 16, &buf16, NULL, flags);
	if (ret)
		debug("%s: Failed to send: %d\n", __func__, ret);

	return ret;
}

static void lb043wv_spi_write_u16_array(struct spi_slave *spi, u16 *buff,
					int size)
{
	int i;

	for (i = 0; i < size; i++)
		lb043wv_spi_write_u16(spi, buff[i]);
}

static void lb043wv_display_mode_settings(struct spi_slave *spi)
{
	static u16 display_mode_settings[] = {
	  0x703A,
	  0x7270,
	  0x70B1,
	  0x7208,
	  0x723B,
	  0x720F,
	  0x70B2,
	  0x7200,
	  0x72C8,
	  0x70B3,
	  0x7200,
	  0x70B4,
	  0x7200,
	  0x70B5,
	  0x7242,
	  0x7210,
	  0x7210,
	  0x7200,
	  0x7220,
	  0x70B6,
	  0x720B,
	  0x720F,
	  0x723C,
	  0x7213,
	  0x7213,
	  0x72E8,
	  0x70B7,
	  0x7246,
	  0x7206,
	  0x720C,
	  0x7200,
	  0x7200,
	};

	debug("transfer display mode settings\n");
	lb043wv_spi_write_u16_array(spi, display_mode_settings,
				    ARRAY_SIZE(display_mode_settings));
}

static void lb043wv_power_settings(struct spi_slave *spi)
{
	static u16 power_settings[] = {
	  0x70C0,
	  0x7201,
	  0x7211,
	  0x70C3,
	  0x7207,
	  0x7203,
	  0x7204,
	  0x7204,
	  0x7204,
	  0x70C4,
	  0x7212,
	  0x7224,
	  0x7218,
	  0x7218,
	  0x7202,
	  0x7249,
	  0x70C5,
	  0x726F,
	  0x70C6,
	  0x7241,
	  0x7263,
	};

	debug("transfer power settings\n");
	lb043wv_spi_write_u16_array(spi, power_settings,
				    ARRAY_SIZE(power_settings));
}

static void lb043wv_gamma_settings(struct spi_slave *spi)
{
	static u16 gamma_settings[] = {
	  0x70D0,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D1,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D2,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D3,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D4,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	  0x70D5,
	  0x7203,
	  0x7207,
	  0x7273,
	  0x7235,
	  0x7200,
	  0x7201,
	  0x7220,
	  0x7200,
	  0x7203,
	};

	debug("transfer gamma settings\n");
	lb043wv_spi_write_u16_array(spi, gamma_settings,
				    ARRAY_SIZE(gamma_settings));
}

static void lb043wv_display_on(struct spi_slave *spi)
{
	static u16 sleep_out = 0x7011;
	static u16 display_on = 0x7029;

	lb043wv_spi_write_u16(spi, sleep_out);
	mdelay(PWR_ON_DELAY_MSECS);
	lb043wv_spi_write_u16(spi, display_on);
}

int lg4573_spi_startup(unsigned int bus, unsigned int cs,
	unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	int ret;

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		debug("%s: Failed to set up slave\n", __func__);
		return -1;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		debug("%s: Failed to claim SPI bus: %d\n", __func__, ret);
		goto err_claim_bus;
	}

	lb043wv_display_mode_settings(spi);
	lb043wv_power_settings(spi);
	lb043wv_gamma_settings(spi);

	lb043wv_display_on(spi);
	return 0;
err_claim_bus:
	spi_free_slave(spi);
	return -1;
}

static int do_lgset(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	lg4573_spi_startup(CONFIG_LG4573_BUS, CONFIG_LG4573_CS, 10000000,
			   SPI_MODE_0);
	return 0;
}

U_BOOT_CMD(
	lgset,	2,	1,	do_lgset,
	"set lgdisplay",
	""
);
