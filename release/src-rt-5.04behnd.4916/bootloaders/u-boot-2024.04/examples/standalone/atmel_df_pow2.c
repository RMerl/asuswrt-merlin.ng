/*
 * atmel_df_pow2.c - convert Atmel Dataflashes to Power of 2 mode
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the 2-clause BSD.
 */

#include <common.h>
#include <exports.h>
#include <spi.h>
#include <linux/delay.h>

#define CMD_ID    0x9f
#define CMD_STAT  0xd7
#define CMD_CFG   0x3d

static int flash_cmd(struct spi_slave *slave, uchar cmd, uchar *buf, int len)
{
	buf[0] = cmd;
	return spi_xfer(slave, 8 * len, buf, buf, SPI_XFER_BEGIN | SPI_XFER_END);
}

static int flash_status(struct spi_slave *slave)
{
	uchar buf[2];
	if (flash_cmd(slave, CMD_STAT, buf, sizeof(buf)))
		return -1;
	return buf[1];
}

static int flash_set_pow2(struct spi_slave *slave)
{
	int ret;
	uchar buf[4];

	buf[1] = 0x2a;
	buf[2] = 0x80;
	buf[3] = 0xa6;

	ret = flash_cmd(slave, CMD_CFG, buf, sizeof(buf));
	if (ret)
		return ret;

	/* wait Tp, or 6 msec */
	udelay(6000);

	ret = flash_status(slave);
	if (ret == -1)
		return 1;

	return ret & 0x1 ? 0 : 1;
}

static int flash_check(struct spi_slave *slave)
{
	int ret;
	uchar buf[4];

	ret = flash_cmd(slave, CMD_ID, buf, sizeof(buf));
	if (ret)
		return ret;

	if (buf[1] != 0x1F) {
		printf("atmel flash not found (id[0] = %#x)\n", buf[1]);
		return 1;
	}

	if ((buf[2] >> 5) != 0x1) {
		printf("AT45 flash not found (id[0] = %#x)\n", buf[2]);
		return 2;
	}

	return 0;
}

static char *getline(void)
{
	static char buffer[100];
	char c;
	size_t i;

	i = 0;
	while (1) {
		buffer[i] = '\0';

		c = getc();

		switch (c) {
		case '\r':	/* Enter/Return key */
		case '\n':
			puts("\n");
			return buffer;

		case 0x03:	/* ^C - break */
			return NULL;

		case 0x5F:
		case 0x08:	/* ^H  - backspace */
		case 0x7F:	/* DEL - backspace */
			if (i) {
				puts("\b \b");
				i--;
			}
			break;

		default:
			/* Ignore control characters */
			if (c < 0x20)
				break;
			/* Queue up all other characters */
			buffer[i++] = c;
			printf("%c", c);
			break;
		}
	}
}

int atmel_df_pow2(int argc, char *const argv[])
{
	/* Print the ABI version */
	app_startup(argv);
	if (XF_VERSION != get_version()) {
		printf("Expects ABI version %d\n", XF_VERSION);
		printf("Actual U-Boot ABI version %lu\n", get_version());
		printf("Can't run\n\n");
		return 1;
	}

	while (1) {
		struct spi_slave *slave;
		char *line, *p;
		int bus, cs, status;

		puts("\nenter the [BUS:]CS of the SPI flash: ");
		line = getline();

		/* CTRL+C */
		if (!line)
			return 0;
		if (line[0] == '\0')
			continue;

		bus = dectoul(line, &p);
		cs = bus;
		if (*p) {
			if (*p == ':') {
				++p;
				cs = dectoul(p, &p);
			}
			if (*p) {
				puts("invalid format, please try again\n");
				continue;
			}
		} else
			bus = 0;

		printf("\ngoing to work with dataflash at %i:%i\n", bus, cs);

		/* use a low speed -- it'll work with all devices, and
		 * speed here doesn't really matter.
		 */
		slave = spi_setup_slave(bus, cs, 1000, SPI_MODE_3);
		if (!slave) {
			puts("unable to setup slave\n");
			continue;
		}

		if (spi_claim_bus(slave)) {
			spi_free_slave(slave);
			continue;
		}

		if (flash_check(slave)) {
			puts("no flash found\n");
			goto done;
		}

		status = flash_status(slave);
		if (status == -1) {
			puts("unable to read status register\n");
			goto done;
		}
		if (status & 0x1) {
			puts("flash is already in power-of-2 mode!\n");
			goto done;
		}

		puts("are you sure you wish to set power-of-2 mode?\n");
		puts("this operation is permanent and irreversible\n");
		printf("enter YES to continue: ");
		line = getline();
		if (!line || strcmp(line, "YES"))
			goto done;

		if (flash_set_pow2(slave)) {
			puts("setting pow2 mode failed\n");
			goto done;
		}

		puts(
			"Configuration should be updated now.  You will have to\n"
			"power cycle the part in order to finish the conversion.\n"
		);

 done:
		spi_release_bus(slave);
		spi_free_slave(slave);
	}
}
