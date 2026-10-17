// SPDX-License-Identifier: GPL-2.0+
/*
 * sl28 extension commands
 *
 * Copyright (c) 2020 Kontron Europe GmbH
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <linux/delay.h>

#define CPLD_I2C_ADDR 0x4a
#define REG_UFM_CTRL 0x02
#define   UFM_CTRL_DCLK    BIT(1)
#define   UFM_CTRL_DIN     BIT(2)
#define   UFM_CTRL_PROGRAM BIT(3)
#define   UFM_CTRL_ERASE   BIT(4)
#define   UFM_CTRL_DSHIFT  BIT(5)
#define   UFM_CTRL_DOUT    BIT(6)
#define   UFM_CTRL_BUSY    BIT(7)

static int ufm_shift_data(struct udevice *dev, u16 data_in, u16 *data_out)
{
	int i;
	int ret;
	u16 data = 0;

	/* latch data */
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, 0);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, UFM_CTRL_DCLK);
	if (ret < 0)
		return ret;

	/* assert drshift */
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL,
			       UFM_CTRL_DSHIFT | UFM_CTRL_DCLK);
	if (ret < 0)
		return ret;

	/* clock 16 data bits, reverse order */
	for (i = 15; i >= 0; i--) {
		u8 din = (data_in & (1 << i)) ? UFM_CTRL_DIN : 0;

		ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, UFM_CTRL_DSHIFT
				| din);
		if (ret < 0)
			return ret;
		if (data_out) {
			ret = dm_i2c_reg_read(dev, REG_UFM_CTRL);
			if (ret < 0)
				return ret;
			if (ret & UFM_CTRL_DOUT)
				data |= (1 << i);
		}
		ret = dm_i2c_reg_write(dev, REG_UFM_CTRL,
				       UFM_CTRL_DSHIFT | UFM_CTRL_DCLK | din);
		if (ret < 0)
			return ret;
	}

	/* deassert drshift */
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, UFM_CTRL_DCLK);
	if (ret < 0)
		return ret;

	if (data_out)
		*data_out = data;

	return ret;
}

static int ufm_erase(struct udevice *dev)
{
	int ret;

	/* erase, tEPMX is 500ms */
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL,
			       UFM_CTRL_DCLK | UFM_CTRL_ERASE);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, UFM_CTRL_DCLK);
	if (ret < 0)
		return ret;
	mdelay(500);

	return 0;
}

static int ufm_program(struct udevice *dev)
{
	int ret;

	/* program, tPPMX is 100us */
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL,
			       UFM_CTRL_DCLK | UFM_CTRL_PROGRAM);
	if (ret < 0)
		return ret;
	ret = dm_i2c_reg_write(dev, REG_UFM_CTRL, UFM_CTRL_DCLK);
	if (ret < 0)
		return ret;
	udelay(100);

	return 0;
}

static int ufm_write(struct udevice *dev, u16 data)
{
	int ret;

	ret = ufm_shift_data(dev, data, NULL);
	if (ret < 0)
		return ret;

	ret = ufm_erase(dev);
	if (ret < 0)
		return ret;

	return ufm_program(dev);
}

static int ufm_read(struct udevice *dev, u16 *data)
{
	return ufm_shift_data(dev, 0, data);
}

static int do_sl28_nvm(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	u16 nvm;
	int ret;
	char *endp;

	if (i2c_get_chip_for_busnum(0, CPLD_I2C_ADDR, 1, &dev))
		return CMD_RET_FAILURE;

	if (argc > 1) {
		nvm = hextoul(argv[1], &endp);
		if (*endp != '\0') {
			printf("ERROR: argument is not a valid number\n");
			ret = -EINVAL;
			goto out;
		}

		/*
		 * We swap all bits, because the a zero bit in hardware means the
		 * feature is enabled. But this is hard for the user.
		 */
		nvm ^= 0xffff;

		ret = ufm_write(dev, nvm);
		if (ret)
			goto out;
		printf("New settings will be activated after the next power cycle!\n");
	} else {
		ret = ufm_read(dev, &nvm);
		if (ret)
			goto out;
		nvm ^= 0xffff;

		printf("%04hx\n", nvm);
	}

	return CMD_RET_SUCCESS;

out:
	printf("command failed (%d)\n", ret);
	return CMD_RET_FAILURE;
}

static char sl28_help_text[] =
	"nvm [<hex>] - display/set the 16 non-volatile bits\n";

U_BOOT_CMD_WITH_SUBCMDS(sl28, "SMARC-sAL28 specific", sl28_help_text,
			U_BOOT_SUBCMD_MKENT(nvm, 2, 1, do_sl28_nvm));
