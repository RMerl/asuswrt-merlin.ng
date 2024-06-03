// SPDX-License-Identifier: GPL-2.0+
/*
 * scf0403.c -- support for DataImage SCF0403 LCD
 *
 * Copyright (c) 2013 Adapted from Linux driver:
 * Copyright (c) 2012 Anders Electronics plc. All Rights Reserved.
 * Copyright (c) 2012 CompuLab, Ltd
 *           Dmitry Lifshitz <lifshitz@compulab.co.il>
 *           Ilya Ledvich <ilya@compulab.co.il>
 * Inspired by Alberto Panizzo <maramaopercheseimorto@gmail.com> &
 *	Marek Vasut work in l4f00242t03.c
 *
 * U-Boot port: Nikita Kiryanov <nikita@compulab.co.il>
 */

#include <common.h>
#include <asm/gpio.h>
#include <spi.h>

struct scf0403_cmd {
	u16 cmd;
	u16 *params;
	int count;
};

struct scf0403_initseq_entry {
	struct scf0403_cmd cmd;
	int delay_ms;
};

struct scf0403_priv {
	struct spi_slave *spi;
	unsigned int reset_gpio;
	u32 rddid;
	struct scf0403_initseq_entry *init_seq;
	int seq_size;
};

struct scf0403_priv priv;

#define SCF0403852GGU04_ID 0x000080

/* SCF0403526GGU20 model commands parameters */
static u16 extcmd_params_sn20[]		= {0xff, 0x98, 0x06};
static u16 spiinttype_params_sn20[]	= {0x60};
static u16 bc_params_sn20[]		= {
		0x01, 0x10, 0x61, 0x74, 0x01, 0x01, 0x1B,
		0x12, 0x71, 0x00, 0x00, 0x00, 0x01, 0x01,
		0x05, 0x00, 0xFF, 0xF2, 0x01, 0x00, 0x40,
};
static u16 bd_params_sn20[] = {0x01, 0x23, 0x45, 0x67, 0x01, 0x23, 0x45, 0x67};
static u16 be_params_sn20[] = {
		0x01, 0x22, 0x22, 0xBA, 0xDC, 0x26, 0x28, 0x22,	0x22,
};
static u16 vcom_params_sn20[]		= {0x74};
static u16 vmesur_params_sn20[]		= {0x7F, 0x0F, 0x00};
static u16 powerctl_params_sn20[]	= {0x03, 0x0b, 0x00};
static u16 lvglvolt_params_sn20[]	= {0x08};
static u16 engsetting_params_sn20[]	= {0x00, 0x00, 0x00, 0x00, 0x00, 0x20};
static u16 dispfunc_params_sn20[]	= {0xa0};
static u16 dvddvolt_params_sn20[]	= {0x74};
static u16 dispinv_params_sn20[]	= {0x00, 0x00, 0x00};
static u16 panelres_params_sn20[]	= {0x82};
static u16 framerate_params_sn20[]	= {0x00, 0x13, 0x13};
static u16 timing_params_sn20[]		= {0x80, 0x05, 0x40, 0x28};
static u16 powerctl2_params_sn20[]	= {0x17, 0x75, 0x79, 0x20};
static u16 memaccess_params_sn20[]	= {0x00};
static u16 pixfmt_params_sn20[]		= {0x66};
static u16 pgamma_params_sn20[]		= {
		0x00, 0x03, 0x0b, 0x0c, 0x0e, 0x08, 0xc5, 0x04,
		0x08, 0x0c, 0x13, 0x11, 0x11, 0x14, 0x0c, 0x10,
};
static u16 ngamma_params_sn20[] = {
		0x00, 0x0d, 0x11, 0x0c, 0x0c, 0x04, 0x76, 0x03,
		0x08, 0x0b, 0x16, 0x10, 0x0d, 0x16, 0x0a, 0x00,
};
static u16 tearing_params_sn20[] = {0x00};

/* SCF0403852GGU04 model commands parameters */
static u16 memaccess_params_sn04[]	= {0x08};
static u16 pixfmt_params_sn04[]		= {0x66};
static u16 modectl_params_sn04[]	= {0x01};
static u16 dispfunc_params_sn04[]	= {0x22, 0xe2, 0xFF, 0x04};
static u16 vcom_params_sn04[]		= {0x00, 0x6A};
static u16 pgamma_params_sn04[]		= {
		0x00, 0x07, 0x0d, 0x10, 0x13, 0x19, 0x0f, 0x0c,
		0x05, 0x08, 0x06, 0x13,	0x0f, 0x30, 0x20, 0x1f,
};
static u16 ngamma_params_sn04[]		= {
		0x1F, 0x20, 0x30, 0x0F, 0x13, 0x06, 0x08, 0x05,
		0x0C, 0x0F, 0x19, 0x13, 0x10, 0x0D, 0x07, 0x00,
};
static u16 dispinv_params_sn04[]	= {0x02};

/* Common commands */
static struct scf0403_cmd scf0403_cmd_slpout	= {0x11, NULL, 0};
static struct scf0403_cmd scf0403_cmd_dison	= {0x29, NULL, 0};

/* SCF0403852GGU04 init sequence */
static struct scf0403_initseq_entry scf0403_initseq_sn04[] = {
	{{0x36, memaccess_params_sn04,	ARRAY_SIZE(memaccess_params_sn04)}, 0},
	{{0x3A, pixfmt_params_sn04,	ARRAY_SIZE(pixfmt_params_sn04)}, 0},
	{{0xB6, dispfunc_params_sn04,	ARRAY_SIZE(dispfunc_params_sn04)}, 0},
	{{0xC5, vcom_params_sn04,	ARRAY_SIZE(vcom_params_sn04)}, 0},
	{{0xE0, pgamma_params_sn04,	ARRAY_SIZE(pgamma_params_sn04)}, 0},
	{{0xE1, ngamma_params_sn04,	ARRAY_SIZE(ngamma_params_sn04)}, 20},
	{{0xB0, modectl_params_sn04,	ARRAY_SIZE(modectl_params_sn04)}, 0},
	{{0xB4, dispinv_params_sn04,	ARRAY_SIZE(dispinv_params_sn04)}, 100},
};

/* SCF0403526GGU20 init sequence */
static struct scf0403_initseq_entry scf0403_initseq_sn20[] = {
	{{0xff, extcmd_params_sn20,	ARRAY_SIZE(extcmd_params_sn20)}, 0},
	{{0xba, spiinttype_params_sn20,	ARRAY_SIZE(spiinttype_params_sn20)}, 0},
	{{0xbc, bc_params_sn20,		ARRAY_SIZE(bc_params_sn20)}, 0},
	{{0xbd, bd_params_sn20,		ARRAY_SIZE(bd_params_sn20)}, 0},
	{{0xbe, be_params_sn20,		ARRAY_SIZE(be_params_sn20)}, 0},
	{{0xc7, vcom_params_sn20,	ARRAY_SIZE(vcom_params_sn20)}, 0},
	{{0xed, vmesur_params_sn20,	ARRAY_SIZE(vmesur_params_sn20)}, 0},
	{{0xc0, powerctl_params_sn20,	ARRAY_SIZE(powerctl_params_sn20)}, 0},
	{{0xfc, lvglvolt_params_sn20,	ARRAY_SIZE(lvglvolt_params_sn20)}, 0},
	{{0xb6, dispfunc_params_sn20,	ARRAY_SIZE(dispfunc_params_sn20)}, 0},
	{{0xdf, engsetting_params_sn20,	ARRAY_SIZE(engsetting_params_sn20)}, 0},
	{{0xf3, dvddvolt_params_sn20,	ARRAY_SIZE(dvddvolt_params_sn20)}, 0},
	{{0xb4, dispinv_params_sn20,	ARRAY_SIZE(dispinv_params_sn20)}, 0},
	{{0xf7, panelres_params_sn20,	ARRAY_SIZE(panelres_params_sn20)}, 0},
	{{0xb1, framerate_params_sn20,	ARRAY_SIZE(framerate_params_sn20)}, 0},
	{{0xf2, timing_params_sn20,	ARRAY_SIZE(timing_params_sn20)}, 0},
	{{0xc1, powerctl2_params_sn20,	ARRAY_SIZE(powerctl2_params_sn20)}, 0},
	{{0x36, memaccess_params_sn20,	ARRAY_SIZE(memaccess_params_sn20)}, 0},
	{{0x3a, pixfmt_params_sn20,	ARRAY_SIZE(pixfmt_params_sn20)}, 0},
	{{0xe0, pgamma_params_sn20,	ARRAY_SIZE(pgamma_params_sn20)}, 0},
	{{0xe1, ngamma_params_sn20,	ARRAY_SIZE(ngamma_params_sn20)}, 0},
	{{0x35, tearing_params_sn20,	ARRAY_SIZE(tearing_params_sn20)}, 0},
};

static void scf0403_gpio_reset(unsigned int gpio)
{
	if (!gpio_is_valid(gpio))
		return;

	gpio_set_value(gpio, 1);
	mdelay(100);
	gpio_set_value(gpio, 0);
	mdelay(40);
	gpio_set_value(gpio, 1);
	mdelay(100);
}

static int scf0403_spi_read_rddid(struct spi_slave *spi, u32 *rddid)
{
	int error = 0;
	u8 ids_buf = 0x00;
	u16 dummy_buf = 0x00;
	u16 cmd = 0x04;

	error = spi_set_wordlen(spi, 9);
	if (error)
		return error;

	/* Here 9 bits required to transmit a command */
	error = spi_xfer(spi, 9, &cmd, NULL, SPI_XFER_ONCE);
	if (error)
		return error;

	/*
	 * Here 8 + 1 bits required to arrange extra clock cycle
	 * before the first data bit.
	 * According to the datasheet - first parameter is the dummy data.
	 */
	error = spi_xfer(spi, 9, NULL, &dummy_buf, SPI_XFER_ONCE);
	if (error)
		return error;

	error = spi_set_wordlen(spi, 8);
	if (error)
		return error;

	/* Read rest of the data */
	error = spi_xfer(spi, 8, NULL, &ids_buf, SPI_XFER_ONCE);
	if (error)
		return error;

	*rddid = ids_buf;

	return 0;
}

static int scf0403_spi_transfer(struct spi_slave *spi, struct scf0403_cmd *cmd)
{
	int i, error;
	u32 command = cmd->cmd;
	u32 msg;

	error = spi_set_wordlen(spi, 9);
	if (error)
		return error;

	error = spi_xfer(spi, 9, &command, NULL, SPI_XFER_ONCE);
	if (error)
		return error;

	for (i = 0; i < cmd->count; i++) {
		msg = (cmd->params[i] | 0x100);
		error = spi_xfer(spi, 9, &msg, NULL, SPI_XFER_ONCE);
		if (error)
			return error;
	}

	return 0;
}

static void scf0403_lcd_init(struct scf0403_priv *priv)
{
	int i;

	/* reset LCD */
	scf0403_gpio_reset(priv->reset_gpio);

	for (i = 0; i < priv->seq_size; i++) {
		if (scf0403_spi_transfer(priv->spi, &priv->init_seq[i].cmd) < 0)
			puts("SPI transfer failed\n");

		mdelay(priv->init_seq[i].delay_ms);
	}
}

static int scf0403_request_reset_gpio(unsigned gpio)
{
	int err = gpio_request(gpio, "lcd reset");

	if (err)
		return err;

	err = gpio_direction_output(gpio, 0);
	if (err)
		gpio_free(gpio);

	return err;
}

int scf0403_init(int reset_gpio)
{
	int error;

	if (gpio_is_valid(reset_gpio)) {
		error = scf0403_request_reset_gpio(reset_gpio);
		if (error) {
			printf("Failed requesting reset GPIO%d: %d\n",
			       reset_gpio, error);
			return error;
		}
	}

	priv.reset_gpio = reset_gpio;
	priv.spi = spi_setup_slave(3, 0, 1000000, SPI_MODE_0);
	error = spi_claim_bus(priv.spi);
	if (error)
		goto bus_claim_fail;

	/* reset LCD */
	scf0403_gpio_reset(reset_gpio);

	error = scf0403_spi_read_rddid(priv.spi, &priv.rddid);
	if (error) {
		puts("IDs read failed\n");
		goto readid_fail;
	}

	if (priv.rddid == SCF0403852GGU04_ID) {
		priv.init_seq = scf0403_initseq_sn04;
		priv.seq_size = ARRAY_SIZE(scf0403_initseq_sn04);
	} else {
		priv.init_seq = scf0403_initseq_sn20;
		priv.seq_size = ARRAY_SIZE(scf0403_initseq_sn20);
	}

	scf0403_lcd_init(&priv);

	/* Start operation */
	scf0403_spi_transfer(priv.spi, &scf0403_cmd_dison);
	mdelay(100);
	scf0403_spi_transfer(priv.spi, &scf0403_cmd_slpout);
	spi_release_bus(priv.spi);

	return 0;

readid_fail:
	spi_release_bus(priv.spi);
bus_claim_fail:
	if (gpio_is_valid(priv.reset_gpio))
		gpio_free(priv.reset_gpio);

	return error;
}
