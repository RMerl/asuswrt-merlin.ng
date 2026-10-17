// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2019, Toradex AG
 */

/*
 * Helpers for Freescale PMIC PF0100
*/

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <linux/delay.h>

#include "pf0100_otp.inc"
#include "pf0100.h"

/* define for PMIC register dump */
/*#define DEBUG */

#define WARNBAR "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"

/* use Apalis GPIO1 to switch on VPGM, ON: 1 */
static __maybe_unused iomux_v3_cfg_t const pmic_prog_pads[] = {
	MX6_PAD_NANDF_D4__GPIO2_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL),
#	define PMIC_PROG_VOLTAGE IMX_GPIO_NR(2, 4)
};

unsigned pmic_init(void)
{
	int rc;
	struct udevice *dev = NULL;
	unsigned programmed = 0;
	uchar bus = 1;
	uchar devid, revid, val;

	puts("PMIC:  ");
	rc = i2c_get_chip_for_busnum(bus, PFUZE100_I2C_ADDR, 1, &dev);
	if (rc) {
		printf("failed to get device for PMIC at address 0x%x\n",
		       PFUZE100_I2C_ADDR);
		return 0;
	}

	/* check for errors in PMIC fuses */
	if (dm_i2c_read(dev, PFUZE100_INTSTAT3, &val, 1) < 0) {
		puts("i2c pmic INTSTAT3 register read failed\n");
		return 0;
	}
	if (val & PFUZE100_BIT_OTP_ECCI) {
		puts("\n" WARNBAR);
		puts("WARNING: ecc errors found in pmic fuse banks\n");
		puts(WARNBAR);
	}
	if (dm_i2c_read(dev, PFUZE100_OTP_ECC_SE1, &val, 1) < 0) {
		puts("i2c pmic ECC_SE1 register read failed\n");
		return 0;
	}
	if (val & PFUZE100_BITS_ECC_SE1) {
		puts(WARNBAR);
		puts("WARNING: ecc has made bit corrections in banks 1 to 5\n");
		puts(WARNBAR);
	}
	if (dm_i2c_read(dev, PFUZE100_OTP_ECC_SE2, &val, 1) < 0) {
		puts("i2c pmic ECC_SE2 register read failed\n");
		return 0;
	}
	if (val & PFUZE100_BITS_ECC_SE2) {
		puts(WARNBAR);
		puts("WARNING: ecc has made bit corrections in banks 6 to 10\n"
		    );
		puts(WARNBAR);
	}
	if (dm_i2c_read(dev, PFUZE100_OTP_ECC_DE1, &val, 1) < 0) {
		puts("i2c pmic ECC_DE register read failed\n");
		return 0;
	}
	if (val & PFUZE100_BITS_ECC_DE1) {
		puts(WARNBAR);
		puts("ERROR: banks 1 to 5 have uncorrectable bits\n");
		puts(WARNBAR);
	}
	if (dm_i2c_read(dev, PFUZE100_OTP_ECC_DE2, &val, 1) < 0) {
		puts("i2c pmic ECC_DE register read failed\n");
		return 0;
	}
	if (val & PFUZE100_BITS_ECC_DE2) {
		puts(WARNBAR);
		puts("ERROR: banks 6 to 10 have uncorrectable bits\n");
		puts(WARNBAR);
	}

	/* get device ident */
	if (dm_i2c_read(dev, PFUZE100_DEVICEID, &devid, 1) < 0) {
		puts("i2c pmic devid read failed\n");
		return 0;
	}
	if (dm_i2c_read(dev, PFUZE100_REVID, &revid, 1) < 0) {
		puts("i2c pmic revid read failed\n");
		return 0;
	}
	printf("device id: 0x%.2x, revision id: 0x%.2x, ", devid, revid);

	/* get device programmed state */
	val = PFUZE100_PAGE_REGISTER_PAGE1;
	if (dm_i2c_write(dev, PFUZE100_PAGE_REGISTER, &val, 1)) {
		puts("i2c write failed\n");
		return 0;
	}
	if (dm_i2c_read(dev, PFUZE100_FUSE_POR1, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return 0;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	if (dm_i2c_read(dev, PFUZE100_FUSE_POR2, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return programmed;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	if (dm_i2c_read(dev, PFUZE100_FUSE_POR3, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return programmed;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	switch (programmed) {
	case 0:
		puts("not programmed\n");
		break;
	case 3:
		puts("programmed\n");
		break;
	default:
		puts("undefined programming state\n");
		break;
	}

	/* The following is needed during production */
	if (programmed != 3) {
		/* set VGEN1 to 1.2V */
		val = PFUZE100_VGEN1_VAL;
		if (dm_i2c_write(dev, PFUZE100_VGEN1CTL, &val, 1)) {
			puts("i2c write failed\n");
			return programmed;
		}

		/* set SWBST to 5.0V */
		val = PFUZE100_SWBST_VAL;
		if (dm_i2c_write(dev, PFUZE100_SWBSTCTL, &val, 1))
			puts("i2c write failed\n");
	}

#ifdef DEBUG
	{
		unsigned int i, j;

		for (i = 0; i < 16; i++)
			printf("\t%x", i);
		for (j = 0; j < 0x80; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				dm_i2c_read(dev, j + i, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\nEXT Page 1");

		val = PFUZE100_PAGE_REGISTER_PAGE1;
		if (dm_i2c_write(dev, PFUZE100_PAGE_REGISTER, &val, 1)) {
			puts("i2c write failed\n");
			return 0;
		}

		for (j = 0x80; j < 0x100; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				dm_i2c_read(dev, j + i, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\nEXT Page 2");

		val = PFUZE100_PAGE_REGISTER_PAGE2;
		if (dm_i2c_write(dev, PFUZE100_PAGE_REGISTER, &val, 1)) {
			puts("i2c write failed\n");
			return 0;
		}

		for (j = 0x80; j < 0x100; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				dm_i2c_read(dev, j + i, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\n");
	}
#endif /* DEBUG */

	return programmed;
}

#ifndef CONFIG_SPL_BUILD
static int pf0100_prog(void)
{
	int rc;
	struct udevice *dev = NULL;
	unsigned char bus = 1;
	unsigned char val;
	unsigned int i;

	if (pmic_init() == 3) {
		puts("PMIC already programmed, exiting\n");
		return CMD_RET_FAILURE;
	}
	/* set up gpio to manipulate vprog, initially off */
	imx_iomux_v3_setup_multiple_pads(pmic_prog_pads,
					 ARRAY_SIZE(pmic_prog_pads));
	gpio_direction_output(PMIC_PROG_VOLTAGE, 0);

	rc = i2c_get_chip_for_busnum(bus, PFUZE100_I2C_ADDR, 1, &dev);
	if (rc) {
		printf("failed to get device for PMIC at address 0x%x\n",
		       PFUZE100_I2C_ADDR);
		return CMD_RET_FAILURE;
	}

	for (i = 0; i < ARRAY_SIZE(pmic_otp_prog); i++) {
		switch (pmic_otp_prog[i].cmd) {
		case pmic_i2c:
			val = (unsigned char) (pmic_otp_prog[i].value & 0xff);
			if (dm_i2c_write(dev, pmic_otp_prog[i].reg, &val, 1)) {
				printf("i2c write failed, reg 0x%2x, value 0x%2x\n",
				       pmic_otp_prog[i].reg, val);
				return CMD_RET_FAILURE;
			}
			break;
		case pmic_delay:
			udelay(pmic_otp_prog[i].value * 1000);
			break;
		case pmic_vpgm:
			gpio_direction_output(PMIC_PROG_VOLTAGE,
					      pmic_otp_prog[i].value);
			break;
		case pmic_pwr:
			/* TODO */
			break;
		}
	}
	return CMD_RET_SUCCESS;
}

static int do_pf0100_prog(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int ret;
	puts("Programming PMIC OTP...");
	ret = pf0100_prog();
	if (ret == CMD_RET_SUCCESS)
		puts("done.\n");
	else
		puts("failed.\n");
	return ret;
}

U_BOOT_CMD(
	pf0100_otp_prog, 1, 0, do_pf0100_prog,
	"Program the OTP fuses on the PMIC PF0100",
	""
);
#endif /* CONFIG_SPL_BUILD */
