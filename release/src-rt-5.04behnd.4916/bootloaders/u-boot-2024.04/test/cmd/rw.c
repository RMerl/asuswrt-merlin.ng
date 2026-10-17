// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Tests for read and write commands
 */

#include <common.h>
#include <dm/test.h>
#include <mapmem.h>
#include <part.h>
#include <test/test.h>
#include <test/ut.h>

static int setup_partitions(struct unit_test_state *uts, struct blk_desc **mmc_dev_desc)
{
	char str_disk_guid[UUID_STR_LEN + 1];
	struct disk_partition parts[2] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 4,
			.name = "data",
		},
		{
			.start = 52,
			.size = 10,
			.name = "log",
		},
	};

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(*mmc_dev_desc, str_disk_guid, parts,
				ARRAY_SIZE(parts)));
	return 0;
}

/* Fill the write buffer with pseudo-random data, clear the read buffer. */
static void init_buffers(char *rb, char *wb, size_t size, unsigned seed)
{
	memset(rb, 0, size);
	while (size--) {
		*wb++ = seed;
		seed *= 43;
		seed += 17 + size/4;
	}
}

static int dm_test_read_write(struct unit_test_state *uts)
{
	struct blk_desc *dev_desc;
	char wbuf[1024], rbuf[1024];
	ulong wa, ra;

#define INIT_BUFFERS() init_buffers(rbuf, wbuf, sizeof(rbuf), __LINE__)

	ut_assertok(setup_partitions(uts, &dev_desc));

	wa = map_to_sysmem(wbuf);
	ra = map_to_sysmem(rbuf);

	/* Simple test, write to/read from same partition. */
	INIT_BUFFERS();
	ut_assertok(run_commandf("write mmc 2:1 0x%lx 0 2", wa));
	ut_assertok(run_commandf("read  mmc 2:1 0x%lx 0 2", ra));
	ut_assertok(memcmp(wbuf, rbuf, sizeof(wbuf)));
	ut_assertok(run_commandf("read  mmc 2:1 0x%lx 1 1", ra));
	ut_assertok(memcmp(&wbuf[512], rbuf, 512));

	/* Use name for write, number for read. */
	INIT_BUFFERS();
	ut_assertok(run_commandf("write mmc 2#log 0x%lx 0 2", wa));
	ut_assertok(run_commandf("read  mmc 2:2   0x%lx 0 2", ra));
	ut_assertok(memcmp(wbuf, rbuf, sizeof(wbuf)));

	/* Use full device for write, name for read. */
	INIT_BUFFERS();
	ut_assertok(run_commandf("write mmc 2:0    0x%lx 0x30 2", wa));
	ut_assertok(run_commandf("read  mmc 2#data 0x%lx    0 2", ra));
	ut_assertok(memcmp(wbuf, rbuf, sizeof(wbuf)));

	/* Use name for write, full device for read */
	INIT_BUFFERS();
	ut_assertok(run_commandf("write mmc 2#log 0x%lx    1 2", wa));
	ut_assertok(run_commandf("read  mmc 2:0   0x%lx 0x35 2", ra));
	ut_assertok(memcmp(wbuf, rbuf, sizeof(wbuf)));

	/* Read/write outside partition bounds should be rejected upfront. */
	console_record_reset_enable();
	ut_asserteq(1, run_commandf("read mmc 2#data 0x%lx 3 2", ra));
	ut_assert_nextlinen("read out of range");
	ut_assert_console_end();

	console_record_reset_enable();
	ut_asserteq(1, run_commandf("write mmc 2#log 0x%lx 9 2", wa));
	ut_assert_nextlinen("write out of range");
	ut_assert_console_end();

	return 0;
}

DM_TEST(dm_test_read_write, UT_TESTF_SCAN_FDT | UT_TESTF_CONSOLE_REC);
