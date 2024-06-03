// SPDX-License-Identifier: GPL-2.0+
/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <spi.h>
#include <spi_flash.h>
#include <jffs2/jffs2.h>
#include <linux/mtd/mtd.h>

#include <asm/io.h>
#include <dm/device-internal.h>

static struct spi_flash *flash;

/*
 * This function computes the length argument for the erase command.
 * The length on which the command is to operate can be given in two forms:
 * 1. <cmd> offset len  - operate on <'offset',  'len')
 * 2. <cmd> offset +len - operate on <'offset',  'round_up(len)')
 * If the second form is used and the length doesn't fall on the
 * sector boundary, than it will be adjusted to the next sector boundary.
 * If it isn't in the flash, the function will fail (return -1).
 * Input:
 *    arg: length specification (i.e. both command arguments)
 * Output:
 *    len: computed length for operation
 * Return:
 *    1: success
 *   -1: failure (bad format, bad address).
 */
static int sf_parse_len_arg(char *arg, ulong *len)
{
	char *ep;
	char round_up_len; /* indicates if the "+length" form used */
	ulong len_arg;

	round_up_len = 0;
	if (*arg == '+') {
		round_up_len = 1;
		++arg;
	}

	len_arg = simple_strtoul(arg, &ep, 16);
	if (ep == arg || *ep != '\0')
		return -1;

	if (round_up_len && flash->sector_size > 0)
		*len = ROUND(len_arg, flash->sector_size);
	else
		*len = len_arg;

	return 1;
}

/**
 * This function takes a byte length and a delta unit of time to compute the
 * approximate bytes per second
 *
 * @param len		amount of bytes currently processed
 * @param start_ms	start time of processing in ms
 * @return bytes per second if OK, 0 on error
 */
static ulong bytes_per_second(unsigned int len, ulong start_ms)
{
	/* less accurate but avoids overflow */
	if (len >= ((unsigned int) -1) / 1024)
		return len / (max(get_timer(start_ms) / 1024, 1UL));
	else
		return 1024 * len / max(get_timer(start_ms), 1UL);
}

static int do_spi_flash_probe(int argc, char * const argv[])
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	/* In DM mode, defaults speed and mode will be taken from DT */
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char *endp;
#ifdef CONFIG_DM_SPI_FLASH
	struct udevice *new, *bus_dev;
	int ret;
#else
	struct spi_flash *new;
#endif

	if (argc >= 2) {
		cs = simple_strtoul(argv[1], &endp, 0);
		if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
			return -1;
		if (*endp == ':') {
			if (endp[1] == 0)
				return -1;

			bus = cs;
			cs = simple_strtoul(endp + 1, &endp, 0);
			if (*endp != 0)
				return -1;
		}
	}

	if (argc >= 3) {
		speed = simple_strtoul(argv[2], &endp, 0);
		if (*argv[2] == 0 || *endp != 0)
			return -1;
	}
	if (argc >= 4) {
		mode = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			return -1;
	}

#ifdef CONFIG_DM_SPI_FLASH
	/* Remove the old device, otherwise probe will just be a nop */
	ret = spi_find_bus_and_cs(bus, cs, &bus_dev, &new);
	if (!ret) {
		device_remove(new, DM_REMOVE_NORMAL);
	}
	flash = NULL;
	ret = spi_flash_probe_bus_cs(bus, cs, speed, mode, &new);
	if (ret) {
		printf("Failed to initialize SPI flash at %u:%u (error %d)\n",
		       bus, cs, ret);
		return 1;
	}

	flash = dev_get_uclass_priv(new);
#else
	if (flash)
		spi_flash_free(flash);

	new = spi_flash_probe(bus, cs, speed, mode);
	flash = new;

	if (!new) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	flash = new;
#endif

	return 0;
}

/**
 * Write a block of data to SPI flash, first checking if it is different from
 * what is already there.
 *
 * If the data being written is the same, then *skipped is incremented by len.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @param cmp_buf	read buffer to use to compare data
 * @param skipped	Count of skipped data (incremented by this function)
 * @return NULL if OK, else a string containing the stage which failed
 */
static const char *spi_flash_update_block(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf, char *cmp_buf, size_t *skipped)
{
	char *ptr = (char *)buf;

	debug("offset=%#x, sector_size=%#x, len=%#zx\n",
	      offset, flash->sector_size, len);
	/* Read the entire sector so to allow for rewriting */
	if (spi_flash_read(flash, offset, flash->sector_size, cmp_buf))
		return "read";
	/* Compare only what is meaningful (len) */
	if (memcmp(cmp_buf, buf, len) == 0) {
		debug("Skip region %x size %zx: no change\n",
		      offset, len);
		*skipped += len;
		return NULL;
	}
	/* Erase the entire sector */
	if (spi_flash_erase(flash, offset, flash->sector_size))
		return "erase";
	/* If it's a partial sector, copy the data into the temp-buffer */
	if (len != flash->sector_size) {
		memcpy(cmp_buf, buf, len);
		ptr = cmp_buf;
	}
	/* Write one complete sector */
	if (spi_flash_write(flash, offset, flash->sector_size, ptr))
		return "write";

	return NULL;
}

/**
 * Update an area of SPI flash by erasing and writing any blocks which need
 * to change. Existing blocks with the correct data are left unchanged.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @return 0 if ok, 1 on error
 */
static int spi_flash_update(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf)
{
	const char *err_oper = NULL;
	char *cmp_buf;
	const char *end = buf + len;
	size_t todo;		/* number of bytes to do in this pass */
	size_t skipped = 0;	/* statistics */
	const ulong start_time = get_timer(0);
	size_t scale = 1;
	const char *start_buf = buf;
	ulong delta;

	if (end - buf >= 200)
		scale = (end - buf) / 100;
	cmp_buf = memalign(ARCH_DMA_MINALIGN, flash->sector_size);
	if (cmp_buf) {
		ulong last_update = get_timer(0);

		for (; buf < end && !err_oper; buf += todo, offset += todo) {
			todo = min_t(size_t, end - buf, flash->sector_size);
			if (get_timer(last_update) > 100) {
				printf("   \rUpdating, %zu%% %lu B/s",
				       100 - (end - buf) / scale,
					bytes_per_second(buf - start_buf,
							 start_time));
				last_update = get_timer(0);
			}
			err_oper = spi_flash_update_block(flash, offset, todo,
					buf, cmp_buf, &skipped);
		}
	} else {
		err_oper = "malloc";
	}
	free(cmp_buf);
	putc('\r');
	if (err_oper) {
		printf("SPI flash failed in %s step\n", err_oper);
		return 1;
	}

	delta = get_timer(start_time);
	printf("%zu bytes written, %zu bytes skipped", len - skipped,
	       skipped);
	printf(" in %ld.%lds, speed %ld B/s\n",
	       delta / 1000, delta % 1000, bytes_per_second(len, start_time));

	return 0;
}

static int do_spi_flash_read_write(int argc, char * const argv[])
{
	unsigned long addr;
	void *buf;
	char *endp;
	int ret = 1;
	int dev = 0;
	loff_t offset, len, maxsize;

	if (argc < 3)
		return -1;

	addr = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	if (mtd_arg_off_size(argc - 2, &argv[2], &dev, &offset, &len,
			     &maxsize, MTD_DEV_TYPE_NOR, flash->size))
		return -1;

	/* Consistency checking */
	if (offset + len > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return 1;
	}

	buf = map_physmem(addr, len, MAP_WRBACK);
	if (!buf && addr) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	if (strcmp(argv[0], "update") == 0) {
		ret = spi_flash_update(flash, offset, len, buf);
	} else if (strncmp(argv[0], "read", 4) == 0 ||
			strncmp(argv[0], "write", 5) == 0) {
		int read;

		read = strncmp(argv[0], "read", 4) == 0;
		if (read)
			ret = spi_flash_read(flash, offset, len, buf);
		else
			ret = spi_flash_write(flash, offset, len, buf);

		printf("SF: %zu bytes @ %#x %s: ", (size_t)len, (u32)offset,
		       read ? "Read" : "Written");
		if (ret)
			printf("ERROR %d\n", ret);
		else
			printf("OK\n");
	}

	unmap_physmem(buf, len);

	return ret == 0 ? 0 : 1;
}

static int do_spi_flash_erase(int argc, char * const argv[])
{
	int ret;
	int dev = 0;
	loff_t offset, len, maxsize;
	ulong size;

	if (argc < 3)
		return -1;

	if (mtd_arg_off(argv[1], &dev, &offset, &len, &maxsize,
			MTD_DEV_TYPE_NOR, flash->size))
		return -1;

	ret = sf_parse_len_arg(argv[2], &size);
	if (ret != 1)
		return -1;

	/* Consistency checking */
	if (offset + size > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return 1;
	}

	ret = spi_flash_erase(flash, offset, size);
	printf("SF: %zu bytes @ %#x Erased: %s\n", (size_t)size, (u32)offset,
	       ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

static int do_spi_protect(int argc, char * const argv[])
{
	int ret = 0;
	loff_t start, len;
	bool prot = false;

	if (argc != 4)
		return -1;

	if (!str2off(argv[2], &start)) {
		puts("start sector is not a valid number\n");
		return 1;
	}

	if (!str2off(argv[3], &len)) {
		puts("len is not a valid number\n");
		return 1;
	}

	if (strcmp(argv[1], "lock") == 0)
		prot = true;
	else if (strcmp(argv[1], "unlock") == 0)
		prot = false;
	else
		return -1;  /* Unknown parameter */

	ret = spi_flash_protect(flash, start, len, prot);

	return ret == 0 ? 0 : 1;
}

#ifdef CONFIG_CMD_SF_TEST
enum {
	STAGE_ERASE,
	STAGE_CHECK,
	STAGE_WRITE,
	STAGE_READ,

	STAGE_COUNT,
};

static char *stage_name[STAGE_COUNT] = {
	"erase",
	"check",
	"write",
	"read",
};

struct test_info {
	int stage;
	int bytes;
	unsigned base_ms;
	unsigned time_ms[STAGE_COUNT];
};

static void show_time(struct test_info *test, int stage)
{
	uint64_t speed;	/* KiB/s */
	int bps;	/* Bits per second */

	speed = (long long)test->bytes * 1000;
	if (test->time_ms[stage])
		do_div(speed, test->time_ms[stage] * 1024);
	bps = speed * 8;

	printf("%d %s: %u ticks, %d KiB/s %d.%03d Mbps\n", stage,
	       stage_name[stage], test->time_ms[stage],
	       (int)speed, bps / 1000, bps % 1000);
}

static void spi_test_next_stage(struct test_info *test)
{
	test->time_ms[test->stage] = get_timer(test->base_ms);
	show_time(test, test->stage);
	test->base_ms = get_timer(0);
	test->stage++;
}

/**
 * Run a test on the SPI flash
 *
 * @param flash		SPI flash to use
 * @param buf		Source buffer for data to write
 * @param len		Size of data to read/write
 * @param offset	Offset within flash to check
 * @param vbuf		Verification buffer
 * @return 0 if ok, -1 on error
 */
static int spi_flash_test(struct spi_flash *flash, uint8_t *buf, ulong len,
			   ulong offset, uint8_t *vbuf)
{
	struct test_info test;
	int i;

	printf("SPI flash test:\n");
	memset(&test, '\0', sizeof(test));
	test.base_ms = get_timer(0);
	test.bytes = len;
	if (spi_flash_erase(flash, offset, len)) {
		printf("Erase failed\n");
		return -1;
	}
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("Check read failed\n");
		return -1;
	}
	for (i = 0; i < len; i++) {
		if (vbuf[i] != 0xff) {
			printf("Check failed at %d\n", i);
			print_buffer(i, vbuf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			return -1;
		}
	}
	spi_test_next_stage(&test);

	if (spi_flash_write(flash, offset, len, buf)) {
		printf("Write failed\n");
		return -1;
	}
	memset(vbuf, '\0', len);
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("Read failed\n");
		return -1;
	}
	spi_test_next_stage(&test);

	for (i = 0; i < len; i++) {
		if (buf[i] != vbuf[i]) {
			printf("Verify failed at %d, good data:\n", i);
			print_buffer(i, buf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			printf("Bad data:\n");
			print_buffer(i, vbuf + i, 1,
				     min_t(uint, len - i, 0x40), 0);
			return -1;
		}
	}
	printf("Test passed\n");
	for (i = 0; i < STAGE_COUNT; i++)
		show_time(&test, i);

	return 0;
}

static int do_spi_flash_test(int argc, char * const argv[])
{
	unsigned long offset;
	unsigned long len;
	uint8_t *buf, *from;
	char *endp;
	uint8_t *vbuf;
	int ret;

	if (argc < 3)
		return -1;
	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	len = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		return -1;

	vbuf = memalign(ARCH_DMA_MINALIGN, len);
	if (!vbuf) {
		printf("Cannot allocate memory (%lu bytes)\n", len);
		return 1;
	}
	buf = memalign(ARCH_DMA_MINALIGN, len);
	if (!buf) {
		free(vbuf);
		printf("Cannot allocate memory (%lu bytes)\n", len);
		return 1;
	}

	from = map_sysmem(CONFIG_SYS_TEXT_BASE, 0);
	memcpy(buf, from, len);
	ret = spi_flash_test(flash, buf, len, offset, vbuf);
	free(vbuf);
	free(buf);
	if (ret) {
		printf("Test failed\n");
		return 1;
	}

	return 0;
}
#endif /* CONFIG_CMD_SF_TEST */

static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *cmd;
	int ret;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "probe") == 0) {
		ret = do_spi_flash_probe(argc, argv);
		goto done;
	}

	/* The remaining commands require a selected device */
	if (!flash) {
		puts("No SPI flash selected. Please run `sf probe'\n");
		return 1;
	}

	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0 ||
	    strcmp(cmd, "update") == 0)
		ret = do_spi_flash_read_write(argc, argv);
	else if (strcmp(cmd, "erase") == 0)
		ret = do_spi_flash_erase(argc, argv);
	else if (strcmp(cmd, "protect") == 0)
		ret = do_spi_protect(argc, argv);
#ifdef CONFIG_CMD_SF_TEST
	else if (!strcmp(cmd, "test"))
		ret = do_spi_flash_test(argc, argv);
#endif
	else
		ret = -1;

done:
	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_CMD_SF_TEST
#define SF_TEST_HELP "\nsf test offset len		" \
		"- run a very basic destructive test"
#else
#define SF_TEST_HELP
#endif

U_BOOT_CMD(
	sf,	5,	1,	do_spi_flash,
	"SPI flash sub-system",
	"probe [[bus:]cs] [hz] [mode]	- init flash device on given SPI bus\n"
	"				  and chip select\n"
	"sf read addr offset|partition len	- read `len' bytes starting at\n"
	"				          `offset' or from start of mtd\n"
	"					  `partition'to memory at `addr'\n"
	"sf write addr offset|partition len	- write `len' bytes from memory\n"
	"				          at `addr' to flash at `offset'\n"
	"					  or to start of mtd `partition'\n"
	"sf erase offset|partition [+]len	- erase `len' bytes from `offset'\n"
	"					  or from start of mtd `partition'\n"
	"					 `+len' round up `len' to block size\n"
	"sf update addr offset|partition len	- erase and write `len' bytes from memory\n"
	"					  at `addr' to flash at `offset'\n"
	"					  or to start of mtd `partition'\n"
	"sf protect lock/unlock sector len	- protect/unprotect 'len' bytes starting\n"
	"					  at address 'sector'\n"
	SF_TEST_HELP
);
