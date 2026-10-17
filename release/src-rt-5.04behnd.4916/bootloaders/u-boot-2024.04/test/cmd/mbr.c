// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for mbr command
 *
 * Copyright 2023 Matrox Video
 * Written by Alex Gendin <agendin@matrox.com>
 */

#include <dm.h>
#include <console.h>
#include <dm/test.h>
#include <mapmem.h>
#include <part.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;
/*
 * Requirements for running test manually:
 * mmc6.img - File size needs to be at least 12 MiB
 *
 * Command to create mmc6.img:
 * $ dd if=/dev/zero of=mmc6.img bs=12M count=1
 *
 * To run this test manually, place mmc6.img into the same directory as u-boot,
 * then run:
 * $ ./u-boot -Tc 'ut mbr'
 *
 * To run this test as part of U-Boot test:
 * $ ./test/py/test.py --bd sandbox --build -k ut_dm -v
 * Note: mmc6.img will be created by the test suit.
 */

static char * mbr_parts_header = "setenv mbr_parts '";
static char * mbr_parts_p1 = "uuid_disk=0x12345678;name=p1,start=8M,bootable,size=1M,id=0x0e";
static char * mbr_parts_p2 = ";name=p2,size=1M,id=0x0e";
static char * mbr_parts_p3 = ";name=p3,size=1M,id=0x0e";
static char * mbr_parts_p4 = ";name=p4,size=1M,id=0x0e";
static char * mbr_parts_p5 = ";name=[ext],size=2M,id=0x05;name=p5,size=1M,id=0x0e";
static char * mbr_parts_tail = "'";

/*
 * One MBR partition
000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 00  |...%$..@........|
000001d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
*/
static unsigned mbr_cmp_start = 0x1B8;
static unsigned mbr_cmp_size = 0x48;
static unsigned char mbr_parts_ref_p1[] = {
                                                0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x80, 0x05,
0x05, 0x01, 0x0e, 0x25, 0x24, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

/*
 * Two MBR partitions
000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 00  |%..F...H........|
000001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
*/
static unsigned char mbr_parts_ref_p2[] = {
                                                0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x80, 0x05,
0x05, 0x01, 0x0e, 0x25, 0x24, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x25,
0x25, 0x01, 0x0e, 0x46, 0x05, 0x01, 0x00, 0x48, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

/*
 * Three MBR partitions
000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 00  |...f%..P........|
000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
*/
static unsigned char mbr_parts_ref_p3[] = {
                                                0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x80, 0x05,
0x05, 0x01, 0x0e, 0x25, 0x24, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x25,
0x25, 0x01, 0x0e, 0x46, 0x05, 0x01, 0x00, 0x48, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x46,
0x06, 0x01, 0x0e, 0x66, 0x25, 0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

/*
 * Four MBR partitions
000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 66  |...f%..P.......f|
000001f0  26 01 0e 87 06 01 00 58  00 00 00 08 00 00 55 aa  |&......X......U.|
*/
static unsigned char mbr_parts_ref_p4[] = {
                                                0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x80, 0x05,
0x05, 0x01, 0x0e, 0x25, 0x24, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x25,
0x25, 0x01, 0x0e, 0x46, 0x05, 0x01, 0x00, 0x48, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x46,
0x06, 0x01, 0x0e, 0x66, 0x25, 0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x66,
0x26, 0x01, 0x0e, 0x87, 0x06, 0x01, 0x00, 0x58, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x55, 0xaa
};

/*
 * Five MBR partitions
000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 66  |...f%..P.......f|
000001f0  26 01 05 a7 26 01 00 58  00 00 00 10 00 00 55 aa  |&...&..X......U.|
*/
static unsigned char mbr_parts_ref_p5[] = {
                                                0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x80, 0x05,
0x05, 0x01, 0x0e, 0x25, 0x24, 0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x25,
0x25, 0x01, 0x0e, 0x46, 0x05, 0x01, 0x00, 0x48, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x46,
0x06, 0x01, 0x0e, 0x66, 0x25, 0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x66,
0x26, 0x01, 0x05, 0xa7, 0x26, 0x01, 0x00, 0x58, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x55, 0xaa
};
static unsigned ebr_cmp_start = 0x1B8;
static unsigned ebr_cmp_size = 0x48;
/*
 * EBR
00b001b0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 87  |................|
00b001c0  07 01 0e a7 26 01 00 08  00 00 00 08 00 00 00 00  |....&...........|
00b001d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00b001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
00b001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
*/
static unsigned char ebr_parts_ref_p5[] = {
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87,
0x07, 0x01, 0x0e, 0xa7, 0x26, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
};

/* Fill write buffers with pseudo-random data */
static void init_write_buffers(char *mbr_wb, size_t mbr_wb_size,
			char *ebr_wb, size_t ebr_wb_size, unsigned seed)
{
	while (mbr_wb_size--) {
		*mbr_wb++ = seed;
		seed *= 43;
		seed += 17 + mbr_wb_size/4;
	}
	while (ebr_wb_size--) {
		*ebr_wb++ = seed;
		seed *= 43;
		seed += 17 + ebr_wb_size/4;
	}
}

/* Build string with MBR partition(s) layout */
static unsigned build_mbr_parts(char *buf, size_t buf_size, unsigned num_parts)
{
	size_t bytes_remaining, cur_str_size;
	char * cur_buf;

	if (!num_parts || num_parts > 5 || !buf)
		return 1;

	cur_buf = buf;
	*cur_buf = '\0';
	bytes_remaining = buf_size;

	cur_str_size = sizeof(mbr_parts_header);
	if (cur_str_size + 1 > bytes_remaining)
		return 1;
	strcat(cur_buf, mbr_parts_header);
	bytes_remaining -= cur_str_size;

	if (num_parts >= 1) {
		cur_str_size = sizeof(mbr_parts_p1);
		if (cur_str_size + 1 > bytes_remaining)
			return 1;
		strcat(cur_buf, mbr_parts_p1);
		bytes_remaining -= cur_str_size;

		if (num_parts >= 2) {
			cur_str_size = sizeof(mbr_parts_p2);
			if (cur_str_size + 1 > bytes_remaining)
				return 1;
			strcat(cur_buf, mbr_parts_p2);
			bytes_remaining -= cur_str_size;

			if (num_parts >= 3) {
				cur_str_size = sizeof(mbr_parts_p3);
				if (cur_str_size + 1 > bytes_remaining)
					return 1;
				strcat(cur_buf, mbr_parts_p3);
				bytes_remaining -= cur_str_size;

				if (num_parts == 4) {
					cur_str_size = sizeof(mbr_parts_p4);
					if (cur_str_size + 1 > bytes_remaining)
						return 1;
					strcat(cur_buf, mbr_parts_p4);
					bytes_remaining -= cur_str_size;

				}
				else if (num_parts == 5) {
					cur_str_size = sizeof(mbr_parts_p5);
					if (cur_str_size + 1 > bytes_remaining)
						return 1;
					strcat(cur_buf, mbr_parts_p5);
					bytes_remaining -= cur_str_size;

				}
			}
		}
	}

	cur_str_size = sizeof(mbr_parts_tail);
	if (cur_str_size + 1 > bytes_remaining)
		return 1;
	strcat(cur_buf, mbr_parts_tail);

	return 0;
}

static int mbr_test_run(struct unit_test_state *uts)
{
	struct blk_desc *mmc_dev_desc;
	unsigned char mbr_wbuf[512], ebr_wbuf[512], rbuf[512];
	char mbr_parts_buf[256];
	ulong mbr_wa, ebr_wa, ra, ebr_blk, mbr_parts_max;
	struct udevice *dev;
	ofnode root, node;

	/* Enable the mmc6 node for this test */
	root = oftree_root(oftree_default());
	node = ofnode_find_subnode(root, "mmc6");
	ut_assert(ofnode_valid(node));
	ut_assertok(lists_bind_fdt(gd->dm_root, node, &dev, NULL, false));

	mbr_parts_max = sizeof('\0') + 2 +
		strlen(mbr_parts_header) +
		strlen(mbr_parts_p1) +
		strlen(mbr_parts_p2) +
		strlen(mbr_parts_p3) +
		max(strlen(mbr_parts_p4), strlen(mbr_parts_p5)) +
		strlen(mbr_parts_tail);
	ut_assertf(sizeof(mbr_parts_buf) >= mbr_parts_max, "Buffer avail: %ld; buffer req: %ld\n",
		sizeof(mbr_parts_buf), mbr_parts_max);

	mbr_wa = map_to_sysmem(mbr_wbuf);
	ebr_wa = map_to_sysmem(ebr_wbuf);
	ra = map_to_sysmem(rbuf);
	ebr_blk = (ulong)0xB00000 / 0x200;

	/* Make sure mmc6 exists */
	ut_asserteq(6, blk_get_device_by_str("mmc", "6", &mmc_dev_desc));
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("mmc dev 6"));
	ut_assert_nextline("switch to partitions #0, OK");
	ut_assert_nextline("mmc6 is current device");
	ut_assertok(ut_check_console_end(uts));

	/* Make sure mmc6 is 12+ MiB in size */
	ut_assertok(run_commandf("mmc read 0x%lx 0x%lx 1", ra, (ulong)0xBFFE00 / 0x200));

	/* Test one MBR partition */
	init_write_buffers(mbr_wbuf, sizeof(mbr_wbuf), ebr_wbuf, sizeof(ebr_wbuf), __LINE__);
	ut_assertok(build_mbr_parts(mbr_parts_buf, sizeof(mbr_parts_buf), 1));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0 1", mbr_wa));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	ut_assertok(memcmp(mbr_wbuf, rbuf, 512));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0x%lx 1", ebr_wa, ebr_blk));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(console_record_reset_enable());
	ut_assertf(0 == run_commandf(mbr_parts_buf), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assertok(run_commandf("mbr write mmc 6"));
	ut_assert_nextline("MBR: write success!");
	ut_assertok(run_commandf("mbr verify mmc 6"));
	ut_assert_nextline("MBR: verify success!");
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(ut_check_console_end(uts));
	/*
	000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
	000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 00  |...%$..@........|
	000001d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
	000001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
	000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	for (unsigned i = 0; i < mbr_cmp_size; i++) {
		ut_assertf(rbuf[mbr_cmp_start + i] == mbr_parts_ref_p1[i],
			"1P MBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			mbr_cmp_start + i, mbr_parts_ref_p1[i], rbuf[mbr_cmp_start + i]);
	}

	/* Test two MBR partitions */
	init_write_buffers(mbr_wbuf, sizeof(mbr_wbuf), ebr_wbuf, sizeof(ebr_wbuf), __LINE__);
	ut_assertok(build_mbr_parts(mbr_parts_buf, sizeof(mbr_parts_buf), 2));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0 1", mbr_wa));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	ut_assertok(memcmp(mbr_wbuf, rbuf, 512));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0x%lx 1", ebr_wa, ebr_blk));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(console_record_reset_enable());
	ut_assertf(0 == run_commandf(mbr_parts_buf), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assertok(run_commandf("mbr write mmc 6"));
	ut_assert_nextline("MBR: write success!");
	ut_assertok(run_commandf("mbr verify mmc 6"));
	ut_assert_nextline("MBR: verify success!");
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(ut_check_console_end(uts));
	/*
	000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
	000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
	000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 00  |%..F...H........|
	000001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
	000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	for (unsigned i = 0; i < mbr_cmp_size; i++) {
		ut_assertf(rbuf[mbr_cmp_start + i] == mbr_parts_ref_p2[i],
			"2P MBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			mbr_cmp_start + i, mbr_parts_ref_p2[i], rbuf[mbr_cmp_start + i]);
	}

	/* Test three MBR partitions */
	init_write_buffers(mbr_wbuf, sizeof(mbr_wbuf), ebr_wbuf, sizeof(ebr_wbuf), __LINE__);
	ut_assertok(build_mbr_parts(mbr_parts_buf, sizeof(mbr_parts_buf), 3));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0 1", mbr_wa));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	ut_assertok(memcmp(mbr_wbuf, rbuf, 512));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0x%lx 1", ebr_wa, ebr_blk));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(console_record_reset_enable());
	ut_assertf(0 == run_commandf(mbr_parts_buf), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assertok(run_commandf("mbr write mmc 6"));
	ut_assert_nextline("MBR: write success!");
	ut_assertok(run_commandf("mbr verify mmc 6"));
	ut_assert_nextline("MBR: verify success!");
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(ut_check_console_end(uts));
	/*
	000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
	000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
	000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
	000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 00  |...f%..P........|
	000001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	for (unsigned i = 0; i < mbr_cmp_size; i++) {
		ut_assertf(rbuf[mbr_cmp_start + i] == mbr_parts_ref_p3[i],
			"3P MBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			mbr_cmp_start + i, mbr_parts_ref_p3[i], rbuf[mbr_cmp_start + i]);
	}

	/* Test four MBR partitions */
	init_write_buffers(mbr_wbuf, sizeof(mbr_wbuf), ebr_wbuf, sizeof(ebr_wbuf), __LINE__);
	ut_assertok(build_mbr_parts(mbr_parts_buf, sizeof(mbr_parts_buf), 4));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0 1", mbr_wa));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	ut_assertok(memcmp(mbr_wbuf, rbuf, 512));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0x%lx 1", ebr_wa, ebr_blk));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(console_record_reset_enable());
	ut_assertf(0 == run_commandf(mbr_parts_buf), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assertok(run_commandf("mbr write mmc 6"));
	ut_assert_nextline("MBR: write success!");
	ut_assertok(run_commandf("mbr verify mmc 6"));
	ut_assert_nextline("MBR: verify success!");
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(ut_check_console_end(uts));
	/*
	000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
	000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
	000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
	000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 66  |...f%..P.......f|
	000001f0  26 01 0e 87 06 01 00 58  00 00 00 08 00 00 55 aa  |&......X......U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	for (unsigned i = 0; i < mbr_cmp_size; i++) {
		ut_assertf(rbuf[mbr_cmp_start + i] == mbr_parts_ref_p4[i],
			"4P MBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			mbr_cmp_start + i, mbr_parts_ref_p4[i], rbuf[mbr_cmp_start + i]);
	}

	/* Test five MBR partitions */
	init_write_buffers(mbr_wbuf, sizeof(mbr_wbuf), ebr_wbuf, sizeof(ebr_wbuf), __LINE__);
	ut_assertok(build_mbr_parts(mbr_parts_buf, sizeof(mbr_parts_buf), 5));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0 1", mbr_wa));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	ut_assertok(memcmp(mbr_wbuf, rbuf, 512));
	ut_assertok(run_commandf("write mmc 6:0 0x%lx 0x%lx 1", ebr_wa, ebr_blk));
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	ut_assertok(memcmp(ebr_wbuf, rbuf, 512));
	ut_assertok(console_record_reset_enable());
	ut_assertf(0 == run_commandf(mbr_parts_buf), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assertf(0 == run_commandf("mbr write mmc 6"), "Invalid partitions string: %s\n", mbr_parts_buf);
	ut_assert_nextline("MBR: write success!");
	ut_assertok(run_commandf("mbr verify mmc 6"));
	ut_assert_nextline("MBR: verify success!");
	ut_assertok(ut_check_console_end(uts));
	/*
	000001b0  00 00 00 00 00 00 00 00  78 56 34 12 00 00 80 05  |........xV4.....|
	000001c0  05 01 0e 25 24 01 00 40  00 00 00 08 00 00 00 25  |...%$..@.......%|
	000001d0  25 01 0e 46 05 01 00 48  00 00 00 08 00 00 00 46  |%..F...H.......F|
	000001e0  06 01 0e 66 25 01 00 50  00 00 00 08 00 00 00 66  |...f%..P.......f|
	000001f0  26 01 05 a7 26 01 00 58  00 00 00 10 00 00 55 aa  |&...&..X......U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0 1", ra));
	for (unsigned i = 0; i < mbr_cmp_size; i++) {
		ut_assertf(rbuf[mbr_cmp_start + i] == mbr_parts_ref_p5[i],
			"5P MBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			mbr_cmp_start + i, mbr_parts_ref_p5[i], rbuf[mbr_cmp_start + i]);
	}
	/*
	00b001b0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 87  |................|
	00b001c0  07 01 0e a7 26 01 00 08  00 00 00 08 00 00 00 00  |....&...........|
	00b001d0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
	00b001e0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
	00b001f0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 55 aa  |..............U.|
	*/
	memset(rbuf, 0, sizeof(rbuf));
	ut_assertok(run_commandf("read mmc 6:0 0x%lx 0x%lx 1", ra, ebr_blk));
	for (unsigned i = 0; i < ebr_cmp_size; i++) {
		ut_assertf(rbuf[ebr_cmp_start + i] == ebr_parts_ref_p5[i],
			"5P EBR+0x%04X: expected 0x%02X, actual: 0x%02X\n",
			ebr_cmp_start + i, ebr_parts_ref_p5[i], rbuf[ebr_cmp_start + i]);
	}

	return 0;
}

/* Declare mbr test */
UNIT_TEST(mbr_test_run, UT_TESTF_CONSOLE_REC, mbr_test);

int do_ut_mbr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(mbr_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(mbr_test);

	return cmd_ut_category("mbr", "mbr_test_", tests, n_ents, argc, argv);
}

static int dm_test_cmd_mbr(struct unit_test_state *uts)
{
	return mbr_test_run(uts);
}

DM_TEST(dm_test_cmd_mbr, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
