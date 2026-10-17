// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <part.h>
#include <part_efi.h>
#include <dm/test.h>
#include <test/ut.h>

static int do_test(struct unit_test_state *uts, int expected,
		   const char *part_str, bool whole)
{
	struct blk_desc *mmc_dev_desc;
	struct disk_partition part_info;

	int ret = part_get_info_by_dev_and_name_or_num("mmc", part_str,
						       &mmc_dev_desc,
						       &part_info, whole);

	ut_assertf(expected == ret, "test(%d, \"%s\", %d) == %d", expected,
		   part_str, whole, ret);
	return 0;
}

static int dm_test_part(struct unit_test_state *uts)
{
	char *oldbootdevice;
	char str_disk_guid[UUID_STR_LEN + 1];
	int ret;
	struct blk_desc *mmc_dev_desc;
	struct disk_partition parts[2] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 1,
			.name = "test1",
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
		},
	};

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, parts,
				ARRAY_SIZE(parts)));

	oldbootdevice = env_get("bootdevice");

#define test(expected, part_str, whole) \
	ut_assertok(do_test(uts, expected, part_str, whole))

	env_set("bootdevice", NULL);
	test(-ENODEV, NULL, true);
	test(-ENODEV, "", true);
	env_set("bootdevice", "0");
	test(0, NULL, true);
	test(0, "", true);
	env_set("bootdevice", "2");
	test(1, NULL, false);
	test(1, "", false);
	test(1, "-", false);
	env_set("bootdevice", "");
	test(-EPROTONOSUPPORT, "0", false);
	test(0, "0", true);
	test(0, ":0", true);
	test(0, ".0", true);
	test(0, ".0:0", true);
	test(-EINVAL, "#test1", true);
	test(1, "2", false);
	test(1, "2", true);
	test(-ENOENT, "2:0", false);
	test(0, "2:0", true);
	test(1, "2:1", false);
	test(2, "2:2", false);
	test(1, "2.0", false);
	test(0, "2.0:0", true);
	test(1, "2.0:1", false);
	test(2, "2.0:2", false);
	test(-EINVAL, "2#bogus", false);
	test(1, "2#test1", false);
	test(2, "2#test2", false);
	ret = 0;

	env_set("bootdevice", oldbootdevice);
	return ret;
}
DM_TEST(dm_test_part, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_part_bootable(struct unit_test_state *uts)
{
	struct blk_desc *desc;
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_BLK, "mmc1.blk", &dev));
	desc = dev_get_uclass_plat(dev);
	ut_asserteq(1, part_get_bootable(desc));

	return 0;
}
DM_TEST(dm_test_part_bootable, UT_TESTF_SCAN_FDT);

static int do_get_info_test(struct unit_test_state *uts,
			    struct blk_desc *dev_desc, int part, int part_type,
			    struct disk_partition const *reference)
{
	struct disk_partition p;
	int ret;

	memset(&p, 0, sizeof(p));

	ret = part_get_info_by_type(dev_desc, part, part_type, &p);
	printf("part_get_info_by_type(%d, 0x%x) = %d\n", part, part_type, ret);
	if (ut_assertok(ret)) {
		return 0;
	}

	ut_asserteq(reference->start, p.start);
	ut_asserteq(reference->size, p.size);
	ut_asserteq(reference->sys_ind, p.sys_ind);

	return 0;
}

static int dm_test_part_get_info_by_type(struct unit_test_state *uts)
{
	char str_disk_guid[UUID_STR_LEN + 1];
	struct blk_desc *mmc_dev_desc;
	struct disk_partition gpt_parts[] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 1,
			.name = "test1",
			.sys_ind = 0,
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
			.sys_ind = 0,
		},
	};
	struct disk_partition mbr_parts[] = {
		{
			.start = 1,
			.size = 33,
			.name = "gpt",
			.sys_ind = EFI_PMBR_OSTYPE_EFI_GPT,
		},
		{
			.start = 48,
			.size = 1,
			.name = "test1",
			.sys_ind = 0x83,
		},
	};

	ut_asserteq(2, blk_get_device_by_str("mmc", "2", &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(gpt_parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(gpt_parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, gpt_parts,
				ARRAY_SIZE(gpt_parts)));

	ut_assertok(write_mbr_partitions(mmc_dev_desc, mbr_parts,
					 ARRAY_SIZE(mbr_parts), 0));

#define get_info_test(_part, _part_type, _reference) \
	ut_assertok(do_get_info_test(uts, mmc_dev_desc, _part, _part_type, \
				     _reference))

	for (int i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		get_info_test(i + 1, PART_TYPE_UNKNOWN, &gpt_parts[i]);
	}

	for (int i = 0; i < ARRAY_SIZE(mbr_parts); i++) {
		get_info_test(i + 1, PART_TYPE_DOS, &mbr_parts[i]);
	}

	for (int i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		get_info_test(i + 1, PART_TYPE_EFI, &gpt_parts[i]);
	}

	return 0;
}
DM_TEST(dm_test_part_get_info_by_type, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
