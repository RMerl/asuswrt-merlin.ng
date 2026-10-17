// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 */

#include <common.h>
#include <command.h>
#include <eeprom.h>
#include <env.h>
#include <env_internal.h>
#include <asm/global_data.h>
#include <linux/stddef.h>
#include <u-boot/crc.h>
#include <search.h>
#include <errno.h>
#include <linux/compiler.h>	/* for BUG_ON */

DECLARE_GLOBAL_DATA_PTR;

static int env_eeprom_load(void)
{
	char buf_env[CONFIG_ENV_SIZE];
	unsigned int off = CONFIG_ENV_OFFSET;

#ifdef CONFIG_ENV_OFFSET_REDUND
	ulong len, crc[2], crc_tmp;
	unsigned int off_env[2];
	uchar rdbuf[64], flags[2];
	int i, crc_ok[2] = {0, 0};

	eeprom_init(-1);	/* prepare for EEPROM read/write */

	off_env[0] = CONFIG_ENV_OFFSET;
	off_env[1] = CONFIG_ENV_OFFSET_REDUND;

	for (i = 0; i < 2; i++) {
		/* read CRC */
		eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
				off_env[i] + offsetof(env_t, crc),
				(uchar *)&crc[i], sizeof(ulong));
		/* read FLAGS */
		eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
				off_env[i] + offsetof(env_t, flags),
				(uchar *)&flags[i], sizeof(uchar));

		crc_tmp = 0;
		len = ENV_SIZE;
		off = off_env[i] + offsetof(env_t, data);
		while (len > 0) {
			int n = (len > sizeof(rdbuf)) ? sizeof(rdbuf) : len;

			eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR, off,
					rdbuf, n);

			crc_tmp = crc32(crc_tmp, rdbuf, n);
			len -= n;
			off += n;
		}

		if (crc_tmp == crc[i])
			crc_ok[i] = 1;
	}

	if (!crc_ok[0] && !crc_ok[1]) {
		gd->env_addr	= 0;
		gd->env_valid = ENV_INVALID;
	} else if (crc_ok[0] && !crc_ok[1]) {
		gd->env_valid = ENV_VALID;
	} else if (!crc_ok[0] && crc_ok[1]) {
		gd->env_valid = ENV_REDUND;
	} else {
		/* both ok - check serial */
		if (flags[0] == ENV_REDUND_ACTIVE &&
		    flags[1] == ENV_REDUND_OBSOLETE)
			gd->env_valid = ENV_VALID;
		else if (flags[0] == ENV_REDUND_OBSOLETE &&
			 flags[1] == ENV_REDUND_ACTIVE)
			gd->env_valid = ENV_REDUND;
		else if (flags[0] == 0xFF && flags[1] == 0)
			gd->env_valid = ENV_REDUND;
		else if (flags[1] == 0xFF && flags[0] == 0)
			gd->env_valid = ENV_VALID;
		else /* flags are equal - almost impossible */
			gd->env_valid = ENV_VALID;
	}

#else /* CONFIG_ENV_OFFSET_REDUND */
	ulong crc, len, new;
	uchar rdbuf[64];

	eeprom_init(-1);	/* prepare for EEPROM read/write */

	/* read old CRC */
	eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
			CONFIG_ENV_OFFSET + offsetof(env_t, crc),
			(uchar *)&crc, sizeof(ulong));

	new = 0;
	len = ENV_SIZE;
	off = offsetof(env_t, data);
	while (len > 0) {
		int n = (len > sizeof(rdbuf)) ? sizeof(rdbuf) : len;

		eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
				CONFIG_ENV_OFFSET + off, rdbuf, n);
		new = crc32(new, rdbuf, n);
		len -= n;
		off += n;
	}

	if (crc == new) {
		gd->env_valid = ENV_VALID;
	} else {
		gd->env_valid = ENV_INVALID;
	}
#endif /* CONFIG_ENV_OFFSET_REDUND */

	off = CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == ENV_REDUND)
		off = CONFIG_ENV_OFFSET_REDUND;
#endif

	eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
		off, (uchar *)buf_env, CONFIG_ENV_SIZE);

	return env_import(buf_env, 1, H_EXTERNAL);
}

static int env_eeprom_save(void)
{
	env_t	env_new;
	int	rc;
	unsigned int off	= CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	unsigned int off_red	= CONFIG_ENV_OFFSET_REDUND;
	char flag_obsolete	= ENV_REDUND_OBSOLETE;
#endif

	rc = env_export(&env_new);
	if (rc)
		return rc;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == ENV_VALID) {
		off	= CONFIG_ENV_OFFSET_REDUND;
		off_red	= CONFIG_ENV_OFFSET;
	}

	env_new.flags = ENV_REDUND_ACTIVE;
#endif

	rc = eeprom_write(CONFIG_SYS_I2C_EEPROM_ADDR,
			      off, (uchar *)&env_new, CONFIG_ENV_SIZE);

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (rc == 0) {
		eeprom_write(CONFIG_SYS_I2C_EEPROM_ADDR,
				 off_red + offsetof(env_t, flags),
				 (uchar *)&flag_obsolete, 1);

		if (gd->env_valid == ENV_VALID)
			gd->env_valid = ENV_REDUND;
		else
			gd->env_valid = ENV_VALID;
	}
#endif
	return rc;
}

U_BOOT_ENV_LOCATION(eeprom) = {
	.location	= ENVL_EEPROM,
	ENV_NAME("EEPROM")
	.load		= env_eeprom_load,
	.save		= env_save_ptr(env_eeprom_save),
};
