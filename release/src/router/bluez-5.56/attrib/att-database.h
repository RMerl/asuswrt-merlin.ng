/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Texas Instruments Corporation
 *
 */

/* Requirements for read/write operations */
enum {
	ATT_NONE,		/* No restrictions */
	ATT_AUTHENTICATION,	/* Authentication required */
	ATT_AUTHORIZATION,	/* Authorization required */
	ATT_NOT_PERMITTED,	/* Operation not permitted */
};

struct attribute {
	uint16_t handle;
	bt_uuid_t uuid;
	int read_req;		/* Read requirement */
	int write_req;		/* Write requirement */
	uint8_t (*read_cb)(struct attribute *a, struct btd_device *device,
							gpointer user_data);
	uint8_t (*write_cb)(struct attribute *a, struct btd_device *device,
							gpointer user_data);
	gpointer cb_user_data;
	size_t len;
	uint8_t *data;
};
