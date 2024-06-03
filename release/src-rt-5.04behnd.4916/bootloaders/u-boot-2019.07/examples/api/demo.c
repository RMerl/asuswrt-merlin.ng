// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 */

#include <common.h>
#include <linux/types.h>
#include <api_public.h>

#include "glue.h"

#define errf(fmt, args...) do { printf("ERROR @ %s(): ", __func__); printf(fmt, ##args); } while (0)

#define BUF_SZ		2048
#define WAIT_SECS	5

void	test_dump_buf(void *, int);
void	test_dump_di(int);
void	test_dump_si(struct sys_info *);
void	test_dump_sig(struct api_signature *);

static char buf[BUF_SZ];

int main(int argc, char * const argv[])
{
	int rv = 0, h, i, j, devs_no;
	struct api_signature *sig = NULL;
	ulong start, now;
	struct device_info *di;
	lbasize_t rlen;
	struct display_info disinfo;

	if (!api_search_sig(&sig))
		return -1;

	syscall_ptr = sig->syscall;
	if (syscall_ptr == NULL)
		return -2;

	if (sig->version > API_SIG_VERSION)
		return -3;

	printf("API signature found @%x\n", (unsigned int)sig);
	test_dump_sig(sig);

	printf("\n*** Consumer API test ***\n");
	printf("syscall ptr 0x%08x@%08x\n", (unsigned int)syscall_ptr,
		(unsigned int)&syscall_ptr);

	/* console activities */
	ub_putc('B');

	printf("*** Press any key to continue ***\n");
	printf("got char 0x%x\n", ub_getc());

	/* system info */
	test_dump_si(ub_get_sys_info());

	/* timing */
	printf("\n*** Timing - wait a couple of secs ***\n");
	start = ub_get_timer(0);
	printf("\ntime: start %lu\n\n", start);
	for (i = 0; i < WAIT_SECS; i++)
		for (j = 0; j < 1000; j++)
			ub_udelay(1000);	/* wait 1 ms */

	/* this is the number of milliseconds that passed from ub_get_timer(0) */
	now = ub_get_timer(start);
	printf("\ntime: now %lu\n\n", now);

	/* enumerate devices */
	printf("\n*** Enumerate devices ***\n");
	devs_no = ub_dev_enum();

	printf("Number of devices found: %d\n", devs_no);
	if (devs_no == 0)
		return -1;

	printf("\n*** Show devices ***\n");
	for (i = 0; i < devs_no; i++) {
		test_dump_di(i);
		printf("\n");
	}

	printf("\n*** Operations on devices ***\n");

	/* test opening a device already opened */
	h = 0;
	if ((rv = ub_dev_open(h)) != 0) {
		errf("open device %d error %d\n", h, rv);
		return -1;
	}
	if ((rv = ub_dev_open(h)) != 0)
		errf("open device %d error %d\n", h, rv);

	ub_dev_close(h);

	/* test storage */
	printf("Trying storage devices...\n");
	for (i = 0; i < devs_no; i++) {
		di = ub_dev_get(i);

		if (di->type & DEV_TYP_STOR)
			break;

	}
	if (i == devs_no)
		printf("No storage devices available\n");
	else {
		memset(buf, 0, BUF_SZ);

		if ((rv = ub_dev_open(i)) != 0)
			errf("open device %d error %d\n", i, rv);

		else if ((rv = ub_dev_read(i, buf, 1, 0, &rlen)) != 0)
			errf("could not read from device %d, error %d\n", i, rv);
		else {
			printf("Sector 0 dump (512B):\n");
			test_dump_buf(buf, 512);
		}

		ub_dev_close(i);
	}

	/* test networking */
	printf("Trying network devices...\n");
	for (i = 0; i < devs_no; i++) {
		di = ub_dev_get(i);

		if (di->type == DEV_TYP_NET)
			break;

	}
	if (i == devs_no)
		printf("No network devices available\n");
	else {
		if ((rv = ub_dev_open(i)) != 0)
			errf("open device %d error %d\n", i, rv);
		else if ((rv = ub_dev_send(i, &buf, 2048)) != 0)
			errf("could not send to device %d, error %d\n", i, rv);

		ub_dev_close(i);
	}

	if (ub_dev_close(h) != 0)
		errf("could not close device %d\n", h);

	printf("\n*** Env vars ***\n");

	printf("ethact = %s\n", ub_env_get("ethact"));
	printf("old fileaddr = %s\n", ub_env_get("fileaddr"));
	ub_env_set("fileaddr", "deadbeef");
	printf("new fileaddr = %s\n", ub_env_get("fileaddr"));

	const char *env = NULL;

	while ((env = ub_env_enum(env)) != NULL)
		printf("%s = %s\n", env, ub_env_get(env));

	printf("\n*** Display ***\n");

	if (ub_display_get_info(DISPLAY_TYPE_LCD, &disinfo)) {
		printf("LCD info: failed\n");
	} else {
		printf("LCD info:\n");
		printf("  pixel width:  %d\n", disinfo.pixel_width);
		printf("  pixel height: %d\n", disinfo.pixel_height);
		printf("  screen rows:  %d\n", disinfo.screen_rows);
		printf("  screen cols:  %d\n", disinfo.screen_cols);
	}
	if (ub_display_get_info(DISPLAY_TYPE_VIDEO, &disinfo)) {
		printf("video info: failed\n");
	} else {
		printf("video info:\n");
		printf("  pixel width:  %d\n", disinfo.pixel_width);
		printf("  pixel height: %d\n", disinfo.pixel_height);
		printf("  screen rows:  %d\n", disinfo.screen_rows);
		printf("  screen cols:  %d\n", disinfo.screen_cols);
	}

	printf("*** Press any key to continue ***\n");
	printf("got char 0x%x\n", ub_getc());

	/*
	 * This only clears messages on screen, not on serial port. It is
	 * equivalent to a no-op if no display is available.
	 */
	ub_display_clear();

	/* reset */
	printf("\n*** Resetting board ***\n");
	ub_reset();
	printf("\nHmm, reset returned...?!\n");

	return rv;
}

void test_dump_sig(struct api_signature *sig)
{
	printf("signature:\n");
	printf("  version\t= %d\n", sig->version);
	printf("  checksum\t= 0x%08x\n", sig->checksum);
	printf("  sc entry\t= 0x%08x\n", (unsigned int)sig->syscall);
}

void test_dump_si(struct sys_info *si)
{
	int i;

	printf("sys info:\n");
	printf("  clkbus\t= 0x%08x\n", (unsigned int)si->clk_bus);
	printf("  clkcpu\t= 0x%08x\n", (unsigned int)si->clk_cpu);
	printf("  bar\t\t= 0x%08x\n", (unsigned int)si->bar);

	printf("---\n");
	for (i = 0; i < si->mr_no; i++) {
		if (si->mr[i].flags == 0)
			break;

		printf("  start\t= 0x%08lx\n", si->mr[i].start);
		printf("  size\t= 0x%08lx\n", si->mr[i].size);

		switch(si->mr[i].flags & 0x000F) {
			case MR_ATTR_FLASH:
				printf("  type FLASH\n");
				break;
			case MR_ATTR_DRAM:
				printf("  type DRAM\n");
				break;
			case MR_ATTR_SRAM:
				printf("  type SRAM\n");
				break;
			default:
				printf("  type UNKNOWN\n");
		}
		printf("---\n");
	}
}

static char *test_stor_typ(int type)
{
	if (type & DT_STOR_IDE)
		return "IDE";

	if (type & DT_STOR_MMC)
		return "MMC";

	if (type & DT_STOR_SATA)
		return "SATA";

	if (type & DT_STOR_SCSI)
		return "SCSI";

	if (type & DT_STOR_USB)
		return "USB";

	return "Unknown";
}

void test_dump_buf(void *buf, int len)
{
	int i;
	int line_counter = 0;
	int sep_flag = 0;
	int addr = 0;

	printf("%07x:\t", addr);

	for (i = 0; i < len; i++) {
		if (line_counter++ > 15) {
			line_counter = 0;
			sep_flag = 0;
			addr += 16;
			i--;
			printf("\n%07x:\t", addr);
			continue;
		}

		if (sep_flag++ > 1) {
			sep_flag = 1;
			printf(" ");
		}

		printf("%02x", *((char *)buf++));
	}

	printf("\n");
}

void test_dump_di(int handle)
{
	int i;
	struct device_info *di = ub_dev_get(handle);

	printf("device info (%d):\n", handle);
	printf("  cookie\t= 0x%08x\n", (uint32_t)di->cookie);
	printf("  type\t\t= 0x%08x\n", di->type);

	if (di->type == DEV_TYP_NET) {
		printf("  hwaddr\t= ");
		for (i = 0; i < 6; i++)
			printf("%02x ", di->di_net.hwaddr[i]);

		printf("\n");

	} else if (di->type & DEV_TYP_STOR) {
		printf("  type\t\t= %s\n", test_stor_typ(di->type));
		printf("  blk size\t\t= %d\n", (unsigned int)di->di_stor.block_size);
		printf("  blk count\t\t= %d\n", (unsigned int)di->di_stor.block_count);
	}
}
