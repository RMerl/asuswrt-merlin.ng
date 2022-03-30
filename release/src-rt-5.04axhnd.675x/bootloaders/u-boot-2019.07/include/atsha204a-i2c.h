/*
 * I2C Driver for Atmel ATSHA204 over I2C
 *
 * Copyright (C) 2014 Josh Datko, Cryptotronix, jbd@cryptotronix.com
 * 		 2016 Tomas Hlavacek, CZ.NIC, tmshlvck@gmail.com
 * 		 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ATSHA204_I2C_H_
#define _ATSHA204_I2C_H_

enum atsha204a_zone
{
	ATSHA204A_ZONE_CONFIG	= 0,
	ATSHA204A_ZONE_OTP	= 1,
	ATSHA204A_ZONE_DATA	= 2,
};

enum atsha204a_status
{
	ATSHA204A_STATUS_SUCCESS	= 0x00,
	ATSHA204A_STATUS_MISCOMPARE	= 0x01,
	ATSHA204A_STATUS_PARSE_ERROR	= 0x03,
	ATSHA204A_STATUS_EXEC_ERROR	= 0x0F,
	ATSHA204A_STATUS_AFTER_WAKE	= 0x11,
	ATSHA204A_STATUS_CRC_ERROR	= 0xFF,
};

enum atsha204a_func
{
	ATSHA204A_FUNC_RESET	= 0x00,
	ATSHA204A_FUNC_SLEEP	= 0x01,
	ATSHA204A_FUNC_IDLE	= 0x02,
	ATSHA204A_FUNC_COMMAND	= 0x03,
};

enum atsha204a_cmd
{
	ATSHA204A_CMD_READ	= 0x02,
	ATSHA204A_CMD_RANDOM	= 0x1B,
};

struct atsha204a_resp
{
	u8 length;
	u8 code;
	u8 data[82];
} __attribute__ ((packed));

struct atsha204a_req
{
	u8 function;
	u8 length;
	u8 command;
	u8 param1;
	u16 param2;
	u8 data[78];
} __attribute__ ((packed));

int atsha204a_wakeup(struct udevice *);
int atsha204a_idle(struct udevice *);
int atsha204a_sleep(struct udevice *);
int atsha204a_read(struct udevice *, enum atsha204a_zone, bool, u16, u8 *);
int atsha204a_get_random(struct udevice *, u8 *, size_t);

#endif /* _ATSHA204_I2C_H_ */
