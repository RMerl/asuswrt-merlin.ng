// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#include <common.h>
#include <i2c.h>
#include <eeprom_layout.h>
#include <eeprom_field.h>
#include <asm/setup.h>
#include <linux/kernel.h>
#include "eeprom.h"

#ifndef CONFIG_SYS_I2C_EEPROM_ADDR
# define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#endif

#ifndef CONFIG_SYS_I2C_EEPROM_BUS
#define CONFIG_SYS_I2C_EEPROM_BUS	0
#endif

#define EEPROM_LAYOUT_VER_OFFSET	44
#define BOARD_SERIAL_OFFSET		20
#define BOARD_SERIAL_OFFSET_LEGACY	8
#define BOARD_REV_OFFSET		0
#define BOARD_REV_OFFSET_LEGACY		6
#define BOARD_REV_SIZE			2
#define PRODUCT_NAME_OFFSET		128
#define PRODUCT_NAME_SIZE		16
#define MAC_ADDR_OFFSET			4
#define MAC_ADDR_OFFSET_LEGACY		0

#define LAYOUT_INVALID	0
#define LAYOUT_LEGACY	0xff

static int cl_eeprom_bus;
static int cl_eeprom_layout; /* Implicitly LAYOUT_INVALID */

static int cl_eeprom_read(uint offset, uchar *buf, int len)
{
	int res;
	unsigned int current_i2c_bus = i2c_get_bus_num();

	res = i2c_set_bus_num(cl_eeprom_bus);
	if (res < 0)
		return res;

	res = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, offset,
			CONFIG_SYS_I2C_EEPROM_ADDR_LEN, buf, len);

	i2c_set_bus_num(current_i2c_bus);

	return res;
}

static int cl_eeprom_setup(uint eeprom_bus)
{
	int res;

	/*
	 * We know the setup was already done when the layout is set to a valid
	 * value and we're using the same bus as before.
	 */
	if (cl_eeprom_layout != LAYOUT_INVALID && eeprom_bus == cl_eeprom_bus)
		return 0;

	cl_eeprom_bus = eeprom_bus;
	res = cl_eeprom_read(EEPROM_LAYOUT_VER_OFFSET,
			     (uchar *)&cl_eeprom_layout, 1);
	if (res) {
		cl_eeprom_layout = LAYOUT_INVALID;
		return res;
	}

	if (cl_eeprom_layout == 0 || cl_eeprom_layout >= 0x20)
		cl_eeprom_layout = LAYOUT_LEGACY;

	return 0;
}

void get_board_serial(struct tag_serialnr *serialnr)
{
	u32 serial[2];
	uint offset;

	memset(serialnr, 0, sizeof(*serialnr));

	if (cl_eeprom_setup(CONFIG_SYS_I2C_EEPROM_BUS))
		return;

	offset = (cl_eeprom_layout != LAYOUT_LEGACY) ?
		BOARD_SERIAL_OFFSET : BOARD_SERIAL_OFFSET_LEGACY;

	if (cl_eeprom_read(offset, (uchar *)serial, 8))
		return;

	if (serial[0] != 0xffffffff && serial[1] != 0xffffffff) {
		serialnr->low = serial[0];
		serialnr->high = serial[1];
	}
}

/*
 * Routine: cl_eeprom_read_mac_addr
 * Description: read mac address and store it in buf.
 */
int cl_eeprom_read_mac_addr(uchar *buf, uint eeprom_bus)
{
	uint offset;
	int err;

	err = cl_eeprom_setup(eeprom_bus);
	if (err)
		return err;

	offset = (cl_eeprom_layout != LAYOUT_LEGACY) ?
			MAC_ADDR_OFFSET : MAC_ADDR_OFFSET_LEGACY;

	return cl_eeprom_read(offset, buf, 6);
}

static u32 board_rev;

/*
 * Routine: cl_eeprom_get_board_rev
 * Description: read system revision from eeprom
 */
u32 cl_eeprom_get_board_rev(uint eeprom_bus)
{
	char str[5]; /* Legacy representation can contain at most 4 digits */
	uint offset = BOARD_REV_OFFSET_LEGACY;

	if (board_rev)
		return board_rev;

	if (cl_eeprom_setup(eeprom_bus))
		return 0;

	if (cl_eeprom_layout != LAYOUT_LEGACY)
		offset = BOARD_REV_OFFSET;

	if (cl_eeprom_read(offset, (uchar *)&board_rev, BOARD_REV_SIZE))
		return 0;

	/*
	 * Convert legacy syntactic representation to semantic
	 * representation. i.e. for rev 1.00: 0x100 --> 0x64
	 */
	if (cl_eeprom_layout == LAYOUT_LEGACY) {
		sprintf(str, "%x", board_rev);
		board_rev = simple_strtoul(str, NULL, 10);
	}

	return board_rev;
};

/*
 * Routine: cl_eeprom_get_board_rev
 * Description: read system revision from eeprom
 *
 * @buf: buffer to store the product name
 * @eeprom_bus: i2c bus num of the eeprom
 *
 * @return: 0 on success, < 0 on failure
 */
int cl_eeprom_get_product_name(uchar *buf, uint eeprom_bus)
{
	int err;

	if (buf == NULL)
		return -EINVAL;

	err = cl_eeprom_setup(eeprom_bus);
	if (err)
		return err;

	err = cl_eeprom_read(PRODUCT_NAME_OFFSET, buf, PRODUCT_NAME_SIZE);
	if (!err) /* Protect ourselves from invalid data (unterminated str) */
		buf[PRODUCT_NAME_SIZE - 1] = '\0';

	return err;
}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
/**
 * eeprom_field_print_bin_ver() - print a "version field" which contains binary
 *				  data
 *
 * Treat the field data as simple binary data, and print it formatted as a
 * version number (2 digits after decimal point).
 * The field size must be exactly 2 bytes.
 *
 * Sample output:
 *      Field Name      123.45
 *
 * @field:	an initialized field to print
 */
void eeprom_field_print_bin_ver(const struct eeprom_field *field)
{
	if ((field->buf[0] == 0xff) && (field->buf[1] == 0xff)) {
		field->buf[0] = 0;
		field->buf[1] = 0;
	}

	printf(PRINT_FIELD_SEGMENT, field->name);
	int major = (field->buf[1] << 8 | field->buf[0]) / 100;
	int minor = (field->buf[1] << 8 | field->buf[0]) - major * 100;
	printf("%d.%02d\n", major, minor);
}

/**
 * eeprom_field_update_bin_ver() - update a "version field" which contains
 *				   binary data
 *
 * This function takes a version string in the form of x.y (x and y are both
 * decimal values, y is limited to two digits), translates it to the binary
 * form, then writes it to the field. The field size must be exactly 2 bytes.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow.
 *
 * @field:	an initialized field
 * @value:	a version string
 *
 * Returns 0 on success, -1 on failure.
 */
int eeprom_field_update_bin_ver(struct eeprom_field *field, char *value)
{
	char *endptr;
	char *tok = strtok(value, ".");
	if (tok == NULL)
		return -1;

	int num = simple_strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	tok = strtok(NULL, "");
	if (tok == NULL)
		return -1;

	int remainder = simple_strtol(tok, &endptr, 0);
	if (*endptr != '\0')
		return -1;

	num = num * 100 + remainder;
	if (num >> 16)
		return -1;

	field->buf[0] = (unsigned char)num;
	field->buf[1] = num >> 8;

	return 0;
}

char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * eeprom_field_print_date() - print a field which contains date data
 *
 * Treat the field data as simple binary data, and print it formatted as a date.
 * Sample output:
 *      Field Name      07/Feb/2014
 *      Field Name      56/BAD/9999
 *
 * @field:	an initialized field to print
 */
void eeprom_field_print_date(const struct eeprom_field *field)
{
	printf(PRINT_FIELD_SEGMENT, field->name);
	printf("%02d/", field->buf[0]);
	if (field->buf[1] >= 1 && field->buf[1] <= 12)
		printf("%s", months[field->buf[1] - 1]);
	else
		printf("BAD");

	printf("/%d\n", field->buf[3] << 8 | field->buf[2]);
}

static int validate_date(unsigned char day, unsigned char month,
			unsigned int year)
{
	int days_in_february;

	switch (month) {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
		if (day > 31)
			return -1;
		break;
	case 3:
	case 5:
	case 8:
	case 10:
		if (day > 30)
			return -1;
		break;
	case 1:
		days_in_february = 28;
		if (year % 4 == 0) {
			if (year % 100 != 0)
				days_in_february = 29;
			else if (year % 400 == 0)
				days_in_february = 29;
		}

		if (day > days_in_february)
			return -1;

		break;
	default:
		return -1;
	}

	return 0;
}

/**
 * eeprom_field_update_date() - update a date field which contains binary data
 *
 * This function takes a date string in the form of x/Mon/y (x and y are both
 * decimal values), translates it to the binary representation, then writes it
 * to the field.
 *
 * This function strictly enforces the data syntax, and will not update the
 * field if there's any deviation from it. It also protects from overflow in the
 * year value, and checks the validity of the date.
 *
 * @field:	an initialized field
 * @value:	a date string
 *
 * Returns 0 on success, -1 on failure.
 */
int eeprom_field_update_date(struct eeprom_field *field, char *value)
{
	char *endptr;
	char *tok1 = strtok(value, "/");
	char *tok2 = strtok(NULL, "/");
	char *tok3 = strtok(NULL, "/");

	if (tok1 == NULL || tok2 == NULL || tok3 == NULL) {
		printf("%s: syntax error\n", field->name);
		return -1;
	}

	unsigned char day = (unsigned char)simple_strtol(tok1, &endptr, 0);
	if (*endptr != '\0' || day == 0) {
		printf("%s: invalid day\n", field->name);
		return -1;
	}

	unsigned char month;
	for (month = 1; month <= 12; month++)
		if (!strcmp(tok2, months[month - 1]))
			break;

	unsigned int year = simple_strtol(tok3, &endptr, 0);
	if (*endptr != '\0') {
		printf("%s: invalid year\n", field->name);
		return -1;
	}

	if (validate_date(day, month - 1, year)) {
		printf("%s: invalid date\n", field->name);
		return -1;
	}

	if (year >> 16) {
		printf("%s: year overflow\n", field->name);
		return -1;
	}

	field->buf[0] = day;
	field->buf[1] = month;
	field->buf[2] = (unsigned char)year;
	field->buf[3] = (unsigned char)(year >> 8);

	return 0;
}

#define	LAYOUT_VERSION_LEGACY 1
#define	LAYOUT_VERSION_VER1 2
#define	LAYOUT_VERSION_VER2 3
#define	LAYOUT_VERSION_VER3 4

extern struct eeprom_field layout_unknown[1];

#define DEFINE_PRINT_UPDATE(x) eeprom_field_print_##x, eeprom_field_update_##x

#ifdef CONFIG_CM_T3X
struct eeprom_field layout_legacy[5] = {
	{ "MAC address",          6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Board Revision",       2, NULL, DEFINE_PRINT_UPDATE(bin) },
	{ "Serial Number",        8, NULL, DEFINE_PRINT_UPDATE(bin) },
	{ "Board Configuration", 64, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ RESERVED_FIELDS,      176, NULL, eeprom_field_print_reserved,
					   eeprom_field_update_ascii },
};
#else
#define layout_legacy layout_unknown
#endif

#if defined(CONFIG_CM_T3X)
struct eeprom_field layout_v1[12] = {
	{ "Major Revision",      2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "Minor Revision",      2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "1st MAC Address",     6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "2nd MAC Address",     6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Production Date",     4, NULL, DEFINE_PRINT_UPDATE(date) },
	{ "Serial Number",      12, NULL, DEFINE_PRINT_UPDATE(bin_rev) },
	{ RESERVED_FIELDS,      96, NULL, DEFINE_PRINT_UPDATE(reserved) },
	{ "Product Name",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #1", 16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #2", 16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #3", 16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ RESERVED_FIELDS,      64, NULL, eeprom_field_print_reserved,
					  eeprom_field_update_ascii },
};
#else
#define layout_v1 layout_unknown
#endif

struct eeprom_field layout_v2[15] = {
	{ "Major Revision",            2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "Minor Revision",            2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "1st MAC Address",           6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "2nd MAC Address",           6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Production Date",           4, NULL, DEFINE_PRINT_UPDATE(date) },
	{ "Serial Number",            12, NULL, DEFINE_PRINT_UPDATE(bin_rev) },
	{ "3rd MAC Address (WIFI)",    6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "4th MAC Address (Bluetooth)", 6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Layout Version",            1, NULL, DEFINE_PRINT_UPDATE(bin) },
	{ RESERVED_FIELDS,            83, NULL, DEFINE_PRINT_UPDATE(reserved) },
	{ "Product Name",             16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #1",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #2",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #3",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ RESERVED_FIELDS,            64, NULL, eeprom_field_print_reserved,
						eeprom_field_update_ascii },
};

struct eeprom_field layout_v3[16] = {
	{ "Major Revision",            2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "Minor Revision",            2, NULL, DEFINE_PRINT_UPDATE(bin_ver) },
	{ "1st MAC Address",           6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "2nd MAC Address",           6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Production Date",           4, NULL, DEFINE_PRINT_UPDATE(date) },
	{ "Serial Number",            12, NULL, DEFINE_PRINT_UPDATE(bin_rev) },
	{ "3rd MAC Address (WIFI)",    6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "4th MAC Address (Bluetooth)", 6, NULL, DEFINE_PRINT_UPDATE(mac) },
	{ "Layout Version",            1, NULL, DEFINE_PRINT_UPDATE(bin) },
	{ "CompuLab EEPROM ID",        3, NULL, DEFINE_PRINT_UPDATE(bin) },
	{ RESERVED_FIELDS,            80, NULL, DEFINE_PRINT_UPDATE(reserved) },
	{ "Product Name",             16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #1",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #2",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ "Product Options #3",       16, NULL, DEFINE_PRINT_UPDATE(ascii) },
	{ RESERVED_FIELDS,            64, NULL, eeprom_field_print_reserved,
						eeprom_field_update_ascii },
};

void eeprom_layout_assign(struct eeprom_layout *layout, int layout_version)
{
	switch (layout->layout_version) {
	case LAYOUT_VERSION_LEGACY:
		layout->fields = layout_legacy;
		layout->num_of_fields = ARRAY_SIZE(layout_legacy);
		break;
	case LAYOUT_VERSION_VER1:
		layout->fields = layout_v1;
		layout->num_of_fields = ARRAY_SIZE(layout_v1);
		break;
	case LAYOUT_VERSION_VER2:
		layout->fields = layout_v2;
		layout->num_of_fields = ARRAY_SIZE(layout_v2);
		break;
	case LAYOUT_VERSION_VER3:
		layout->fields = layout_v3;
		layout->num_of_fields = ARRAY_SIZE(layout_v3);
		break;
	default:
		__eeprom_layout_assign(layout, layout_version);
	}
}

int eeprom_parse_layout_version(char *str)
{
	if (!strcmp(str, "legacy"))
		return LAYOUT_VERSION_LEGACY;
	else if (!strcmp(str, "v1"))
		return LAYOUT_VERSION_VER1;
	else if (!strcmp(str, "v2"))
		return LAYOUT_VERSION_VER2;
	else if (!strcmp(str, "v3"))
		return LAYOUT_VERSION_VER3;
	else
		return LAYOUT_VERSION_UNRECOGNIZED;
}

int eeprom_layout_detect(unsigned char *data)
{
	switch (data[EEPROM_LAYOUT_VER_OFFSET]) {
	case 0xff:
	case 0:
		return LAYOUT_VERSION_VER1;
	case 2:
		return LAYOUT_VERSION_VER2;
	case 3:
		return LAYOUT_VERSION_VER3;
	}

	if (data[EEPROM_LAYOUT_VER_OFFSET] >= 0x20)
		return LAYOUT_VERSION_LEGACY;

	return LAYOUT_VERSION_UNRECOGNIZED;
}
#endif
