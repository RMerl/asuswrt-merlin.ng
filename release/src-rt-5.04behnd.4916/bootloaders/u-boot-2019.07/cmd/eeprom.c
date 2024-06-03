// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Support for read and write access to EEPROM like memory devices. This
 * includes regular EEPROM as well as  FRAM (ferroelectic nonvolaile RAM).
 * FRAM devices read and write data at bus speed. In particular, there is no
 * write delay. Also, there is no limit imposed on the number of bytes that can
 * be transferred with a single read or write.
 *
 * Use the following configuration options to ensure no unneeded performance
 * degradation (typical for EEPROM) is incured for FRAM memory:
 *
 * #define CONFIG_SYS_I2C_FRAM
 * #undef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
 *
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <i2c.h>
#include <eeprom_layout.h>

#ifndef	CONFIG_SYS_I2C_SPEED
#define	CONFIG_SYS_I2C_SPEED	50000
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	0
#endif

#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	8
#endif

#ifndef	I2C_RXTX_LEN
#define I2C_RXTX_LEN	128
#endif

#define	EEPROM_PAGE_SIZE	(1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)
#define	EEPROM_PAGE_OFFSET(x)	((x) & (EEPROM_PAGE_SIZE - 1))

/*
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 2 (16-bit EEPROM address) offset is
 *   0x000nxxxx for EEPROM address selectors at n, offset xxxx in EEPROM.
 *
 * for CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1 (8-bit EEPROM page address) offset is
 *   0x00000nxx for EEPROM address selectors and page number at n.
 */
#if !defined(CONFIG_SPI) || defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
#if !defined(CONFIG_SYS_I2C_EEPROM_ADDR_LEN) || \
	(CONFIG_SYS_I2C_EEPROM_ADDR_LEN < 1) || \
	(CONFIG_SYS_I2C_EEPROM_ADDR_LEN > 2)
#error CONFIG_SYS_I2C_EEPROM_ADDR_LEN must be 1 or 2
#endif
#endif

#if defined(CONFIG_DM_I2C)
int eeprom_i2c_bus;
#endif

__weak int eeprom_write_enable(unsigned dev_addr, int state)
{
	return 0;
}

void eeprom_init(int bus)
{
	/* I2C EEPROM */
#if defined(CONFIG_DM_I2C)
	eeprom_i2c_bus = bus;
#elif defined(CONFIG_SYS_I2C)
	if (bus >= 0)
		i2c_set_bus_num(bus);
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
}

static int eeprom_addr(unsigned dev_addr, unsigned offset, uchar *addr)
{
	unsigned blk_off;
	int alen;

	blk_off = offset & 0xff;	/* block offset */
#if CONFIG_SYS_I2C_EEPROM_ADDR_LEN == 1
	addr[0] = offset >> 8;		/* block number */
	addr[1] = blk_off;		/* block offset */
	alen = 2;
#else
	addr[0] = offset >> 16;		/* block number */
	addr[1] = offset >>  8;		/* upper address octet */
	addr[2] = blk_off;		/* lower address octet */
	alen = 3;
#endif	/* CONFIG_SYS_I2C_EEPROM_ADDR_LEN */

	addr[0] |= dev_addr;		/* insert device address */

	return alen;
}

static int eeprom_len(unsigned offset, unsigned end)
{
	unsigned len = end - offset;

	/*
	 * For a FRAM device there is no limit on the number of the
	 * bytes that can be ccessed with the single read or write
	 * operation.
	 */
#if !defined(CONFIG_SYS_I2C_FRAM)
	unsigned blk_off = offset & 0xff;
	unsigned maxlen = EEPROM_PAGE_SIZE - EEPROM_PAGE_OFFSET(blk_off);

	if (maxlen > I2C_RXTX_LEN)
		maxlen = I2C_RXTX_LEN;

	if (len > maxlen)
		len = maxlen;
#endif

	return len;
}

static int eeprom_rw_block(unsigned offset, uchar *addr, unsigned alen,
			   uchar *buffer, unsigned len, bool read)
{
	int ret = 0;

#if defined(CONFIG_DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(eeprom_i2c_bus, addr[0],
				      alen - 1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       eeprom_i2c_bus);
		return CMD_RET_FAILURE;
	}

	if (read)
		ret = dm_i2c_read(dev, offset, buffer, len);
	else
		ret = dm_i2c_write(dev, offset, buffer, len);

#else /* Non DM I2C support - will be removed */

	if (read)
		ret = i2c_read(addr[0], offset, alen - 1, buffer, len);
	else
		ret = i2c_write(addr[0], offset, alen - 1, buffer, len);
#endif /* CONFIG_DM_I2C */
	if (ret)
		ret = CMD_RET_FAILURE;

	return ret;
}

static int eeprom_rw(unsigned dev_addr, unsigned offset, uchar *buffer,
		     unsigned cnt, bool read)
{
	unsigned end = offset + cnt;
	unsigned alen, len;
	int rcode = 0;
	uchar addr[3];

#if defined(CONFIG_SYS_I2C_EEPROM_BUS)
	eeprom_init(CONFIG_SYS_I2C_EEPROM_BUS);
#endif

	while (offset < end) {
		alen = eeprom_addr(dev_addr, offset, addr);

		len = eeprom_len(offset, end);

		rcode = eeprom_rw_block(offset, addr, alen, buffer, len, read);

		buffer += len;
		offset += len;

		if (!read)
			udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
	}

	return rcode;
}

int eeprom_read(unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt)
{
	/*
	 * Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	return eeprom_rw(dev_addr, offset, buffer, cnt, 1);
}

int eeprom_write(unsigned dev_addr, unsigned offset,
		 uchar *buffer, unsigned cnt)
{
	int ret;

	eeprom_write_enable(dev_addr, 1);

	/*
	 * Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */
	ret = eeprom_rw(dev_addr, offset, buffer, cnt, 0);

	eeprom_write_enable(dev_addr, 0);
	return ret;
}

static int parse_numeric_param(char *str)
{
	char *endptr;
	int value = simple_strtol(str, &endptr, 16);

	return (*endptr != '\0') ? -1 : value;
}

/**
 * parse_i2c_bus_addr - parse the i2c bus and i2c devaddr parameters
 *
 * @i2c_bus:	address to store the i2c bus
 * @i2c_addr:	address to store the device i2c address
 * @argc:	count of command line arguments left to parse
 * @argv:	command line arguments left to parse
 * @argc_no_bus_addr:	argc value we expect to see when bus & addr aren't given
 *
 * @returns:	number of arguments parsed or CMD_RET_USAGE if error
 */
static int parse_i2c_bus_addr(int *i2c_bus, ulong *i2c_addr, int argc,
			      char * const argv[], int argc_no_bus_addr)
{
	int argc_no_bus = argc_no_bus_addr + 1;
	int argc_bus_addr = argc_no_bus_addr + 2;

#ifdef CONFIG_SYS_DEF_EEPROM_ADDR
	if (argc == argc_no_bus_addr) {
		*i2c_bus = -1;
		*i2c_addr = CONFIG_SYS_DEF_EEPROM_ADDR;

		return 0;
	}
#endif
	if (argc == argc_no_bus) {
		*i2c_bus = -1;
		*i2c_addr = parse_numeric_param(argv[0]);

		return 1;
	}

	if (argc == argc_bus_addr) {
		*i2c_bus = parse_numeric_param(argv[0]);
		*i2c_addr = parse_numeric_param(argv[1]);

		return 2;
	}

	return CMD_RET_USAGE;
}

#ifdef CONFIG_CMD_EEPROM_LAYOUT

__weak int eeprom_parse_layout_version(char *str)
{
	return LAYOUT_VERSION_UNRECOGNIZED;
}

static unsigned char eeprom_buf[CONFIG_SYS_EEPROM_SIZE];

#endif

enum eeprom_action {
	EEPROM_READ,
	EEPROM_WRITE,
	EEPROM_PRINT,
	EEPROM_UPDATE,
	EEPROM_ACTION_INVALID,
};

static enum eeprom_action parse_action(char *cmd)
{
	if (!strncmp(cmd, "read", 4))
		return EEPROM_READ;
	if (!strncmp(cmd, "write", 5))
		return EEPROM_WRITE;
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (!strncmp(cmd, "print", 5))
		return EEPROM_PRINT;
	if (!strncmp(cmd, "update", 6))
		return EEPROM_UPDATE;
#endif

	return EEPROM_ACTION_INVALID;
}

static int eeprom_execute_command(enum eeprom_action action, int i2c_bus,
				  ulong i2c_addr, int layout_ver, char *key,
				  char *value, ulong addr, ulong off, ulong cnt)
{
	int rcode = 0;
	const char *const fmt =
		"\nEEPROM @0x%lX %s: addr %08lx  off %04lx  count %ld ... ";
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	struct eeprom_layout layout;
#endif

	if (action == EEPROM_ACTION_INVALID)
		return CMD_RET_USAGE;

	eeprom_init(i2c_bus);
	if (action == EEPROM_READ) {
		printf(fmt, i2c_addr, "read", addr, off, cnt);

		rcode = eeprom_read(i2c_addr, off, (uchar *)addr, cnt);

		puts("done\n");
		return rcode;
	} else if (action == EEPROM_WRITE) {
		printf(fmt, i2c_addr, "write", addr, off, cnt);

		rcode = eeprom_write(i2c_addr, off, (uchar *)addr, cnt);

		puts("done\n");
		return rcode;
	}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	rcode = eeprom_read(i2c_addr, 0, eeprom_buf, CONFIG_SYS_EEPROM_SIZE);
	if (rcode < 0)
		return rcode;

	eeprom_layout_setup(&layout, eeprom_buf, CONFIG_SYS_EEPROM_SIZE,
			    layout_ver);

	if (action == EEPROM_PRINT) {
		layout.print(&layout);
		return 0;
	}

	layout.update(&layout, key, value);

	rcode = eeprom_write(i2c_addr, 0, layout.data, CONFIG_SYS_EEPROM_SIZE);
#endif

	return rcode;
}

#define NEXT_PARAM(argc, index)	{ (argc)--; (index)++; }
int do_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int layout_ver = LAYOUT_VERSION_AUTODETECT;
	enum eeprom_action action = EEPROM_ACTION_INVALID;
	int i2c_bus = -1, index = 0;
	ulong i2c_addr = -1, addr = 0, cnt = 0, off = 0;
	int ret;
	char *field_name = "";
	char *field_value = "";

	if (argc <= 1)
		return CMD_RET_USAGE;

	NEXT_PARAM(argc, index); /* Skip program name */

	action = parse_action(argv[index]);
	NEXT_PARAM(argc, index);

	if (action == EEPROM_ACTION_INVALID)
		return CMD_RET_USAGE;

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (action == EEPROM_PRINT || action == EEPROM_UPDATE) {
		if (!strcmp(argv[index], "-l")) {
			NEXT_PARAM(argc, index);
			layout_ver = eeprom_parse_layout_version(argv[index]);
			NEXT_PARAM(argc, index);
		}
	}
#endif

	switch (action) {
	case EEPROM_READ:
	case EEPROM_WRITE:
		ret = parse_i2c_bus_addr(&i2c_bus, &i2c_addr, argc,
					 argv + index, 3);
		break;
	case EEPROM_PRINT:
		ret = parse_i2c_bus_addr(&i2c_bus, &i2c_addr, argc,
					 argv + index, 0);
		break;
	case EEPROM_UPDATE:
		ret = parse_i2c_bus_addr(&i2c_bus, &i2c_addr, argc,
					 argv + index, 2);
		break;
	default:
		/* Get compiler to stop whining */
		return CMD_RET_USAGE;
	}

	if (ret == CMD_RET_USAGE)
		return ret;

	while (ret--)
		NEXT_PARAM(argc, index);

	if (action == EEPROM_READ || action == EEPROM_WRITE) {
		addr = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		off = parse_numeric_param(argv[index]);
		NEXT_PARAM(argc, index);
		cnt = parse_numeric_param(argv[index]);
	}

#ifdef CONFIG_CMD_EEPROM_LAYOUT
	if (action == EEPROM_UPDATE) {
		field_name = argv[index];
		NEXT_PARAM(argc, index);
		field_value = argv[index];
		NEXT_PARAM(argc, index);
	}
#endif

	return eeprom_execute_command(action, i2c_bus, i2c_addr, layout_ver,
				      field_name, field_value, addr, off, cnt);
}

U_BOOT_CMD(
	eeprom,	8,	1,	do_eeprom,
	"EEPROM sub-system",
	"read  <bus> <devaddr> addr off cnt\n"
	"eeprom write <bus> <devaddr> addr off cnt\n"
	"       - read/write `cnt' bytes from `devaddr` EEPROM at offset `off'"
#ifdef CONFIG_CMD_EEPROM_LAYOUT
	"\n"
	"eeprom print [-l <layout_version>] <bus> <devaddr>\n"
	"       - Print layout fields and their data in human readable format\n"
	"eeprom update [-l <layout_version>] <bus> <devaddr> field_name field_value\n"
	"       - Update a specific eeprom field with new data.\n"
	"         The new data must be written in the same human readable format as shown by the print command.\n"
	"\n"
	"LAYOUT VERSIONS\n"
	"The -l option can be used to force the command to interpret the EEPROM data using the chosen layout.\n"
	"If the -l option is omitted, the command will auto detect the layout based on the data in the EEPROM.\n"
	"The values which can be provided with the -l option are:\n"
	CONFIG_EEPROM_LAYOUT_HELP_STRING"\n"
#endif
)
