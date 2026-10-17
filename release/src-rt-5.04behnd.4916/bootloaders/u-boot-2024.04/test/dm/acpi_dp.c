// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for ACPI code generation via a device-property table
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <uuid.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_dp.h>
#include <asm/unaligned.h>
#include <dm/acpi.h>
#include <dm/test.h>
#include <test/ut.h>
#include "acpi.h"

/* Maximum size of the ACPI context needed for most tests */
#define ACPI_CONTEXT_SIZE	500

#define TEST_INT8	0x7d
#define TEST_INT16	0x2345
#define TEST_INT32	0x12345678
#define TEST_INT64	0x4567890123456
#define TEST_STR	"testing acpi strings"
#define TEST_REF	"\\SB.I2C0.TPM2"
#define EXPECT_REF	"SB__I2C0TPM2"

static int alloc_context(struct acpi_ctx **ctxp)
{
	return acpi_test_alloc_context_size(ctxp, ACPI_CONTEXT_SIZE);

	return 0;
}

static void free_context(struct acpi_ctx **ctxp)
{
	free(*ctxp);
	*ctxp = NULL;
}

/* Test emitting an empty table */
static int dm_test_acpi_dp_new_table(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(10, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, *(u8 *)ptr);
	ut_asserteq_strn("FRED", (char *)ptr + 1);
	ut_asserteq(PACKAGE_OP, ptr[5]);
	ut_asserteq(4, acpi_test_get_length(ptr + 6));
	ut_asserteq(0, ptr[9]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_new_table, 0);

/* Test emitting an integer */
static int dm_test_acpi_dp_int(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	char uuid[UUID_STR_LEN + 1];
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	ut_assertnonnull(acpi_dp_add_integer(dp, "MARY", TEST_INT32));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(54, acpigen_get_current(ctx) - ptr);
	ut_asserteq(NAME_OP, *(u8 *)ptr);
	ut_asserteq_strn("FRED", (char *)ptr + 1);
	ut_asserteq(PACKAGE_OP, ptr[5]);
	ut_asserteq(48, acpi_test_get_length(ptr + 6));
	ut_asserteq(2, ptr[9]);

	/* UUID */
	ut_asserteq(BUFFER_OP, ptr[10]);
	ut_asserteq(22, acpi_test_get_length(ptr + 11));
	ut_asserteq(WORD_PREFIX, ptr[14]);
	ut_asserteq(16, get_unaligned((u16 *)(ptr + 15)));
	uuid_bin_to_str(ptr + 17, uuid, 1);
	ut_asserteq_str(ACPI_DP_UUID, uuid);

	/* Container package */
	ut_asserteq(PACKAGE_OP, ptr[33]);
	ut_asserteq(20, acpi_test_get_length(ptr + 34));
	ut_asserteq(1, ptr[37]);

	/* Package with name and (integer) value */
	ut_asserteq(PACKAGE_OP, ptr[38]);
	ut_asserteq(15, acpi_test_get_length(ptr + 39));
	ut_asserteq(2, ptr[42]);
	ut_asserteq(STRING_PREFIX, ptr[43]);
	ut_asserteq_str("MARY", (char *)ptr + 44);

	ut_asserteq(DWORD_PREFIX, ptr[49]);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)(ptr + 50)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_int, 0);

/* Test emitting a 64-bit integer */
static int dm_test_acpi_dp_int64(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	ut_assertnonnull(acpi_dp_add_integer(dp, "MARY", TEST_INT64));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(58, acpigen_get_current(ctx) - ptr);

	ut_asserteq(QWORD_PREFIX, ptr[49]);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 50)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_int64, 0);

/* Test emitting a 16-bit integer */
static int dm_test_acpi_dp_int16(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	ut_assertnonnull(acpi_dp_add_integer(dp, "MARY", TEST_INT16));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(52, acpigen_get_current(ctx) - ptr);

	ut_asserteq(WORD_PREFIX, ptr[49]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 50)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_int16, 0);

/* Test emitting a 8-bit integer */
static int dm_test_acpi_dp_int8(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	ut_assertnonnull(acpi_dp_add_integer(dp, "MARY", TEST_INT8));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(51, acpigen_get_current(ctx) - ptr);

	ut_asserteq(BYTE_PREFIX, ptr[49]);
	ut_asserteq(TEST_INT8, ptr[50]);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_int8, 0);

/* Test emitting multiple values */
static int dm_test_acpi_dp_multiple(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	ut_assertnonnull(acpi_dp_add_integer(dp, "int16", TEST_INT16));
	ut_assertnonnull(acpi_dp_add_string(dp, "str", TEST_STR));
	ut_assertnonnull(acpi_dp_add_reference(dp, "ref", TEST_REF));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(110, acpigen_get_current(ctx) - ptr);

	ut_asserteq(WORD_PREFIX, ptr[0x32]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 0x33)));
	ut_asserteq(STRING_PREFIX, ptr[0x3f]);
	ut_asserteq_str(TEST_STR, (char *)ptr + 0x40);
	ut_asserteq(ROOT_PREFIX, ptr[0x5f]);
	ut_asserteq(MULTI_NAME_PREFIX, ptr[0x60]);
	ut_asserteq(3, ptr[0x61]);
	ut_asserteq_strn(EXPECT_REF, (char *)ptr + 0x62);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_multiple, 0);

/* Test emitting an array */
static int dm_test_acpi_dp_array(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u64 speed[4];
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);
	speed[0] = TEST_INT8;
	speed[1] = TEST_INT16;
	speed[2] = TEST_INT32;
	speed[3] = TEST_INT64;
	ut_assertnonnull(acpi_dp_add_integer_array(dp, "speeds", speed,
						   ARRAY_SIZE(speed)));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(75, acpigen_get_current(ctx) - ptr);

	ut_asserteq(BYTE_PREFIX, ptr[0x38]);
	ut_asserteq(TEST_INT8, ptr[0x39]);

	ut_asserteq(WORD_PREFIX, ptr[0x3a]);
	ut_asserteq(TEST_INT16, get_unaligned((u16 *)(ptr + 0x3b)));

	ut_asserteq(DWORD_PREFIX, ptr[0x3d]);
	ut_asserteq(TEST_INT32, get_unaligned((u32 *)(ptr + 0x3e)));

	ut_asserteq(QWORD_PREFIX, ptr[0x42]);
	ut_asserteq_64(TEST_INT64, get_unaligned((u64 *)(ptr + 0x43)));

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_array, 0);

/* Test emitting a child */
static int dm_test_acpi_dp_child(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp, *child1, *child2;
	char uuid[UUID_STR_LEN + 1];
	u8 *ptr, *pptr;
	int i;

	ut_assertok(alloc_context(&ctx));

	child1 = acpi_dp_new_table("child");
	ut_assertnonnull(child1);
	ut_assertnonnull(acpi_dp_add_integer(child1, "height", TEST_INT16));

	child2 = acpi_dp_new_table("child");
	ut_assertnonnull(child2);
	ut_assertnonnull(acpi_dp_add_integer(child2, "age", TEST_INT8));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);

	ut_assertnonnull(acpi_dp_add_child(dp, "anna", child1));
	ut_assertnonnull(acpi_dp_add_child(dp, "john", child2));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(178, acpigen_get_current(ctx) - ptr);

	/* UUID for child extension using Hierarchical Data Extension UUID */
	ut_asserteq(BUFFER_OP, ptr[10]);
	ut_asserteq(22, acpi_test_get_length(ptr + 11));
	ut_asserteq(WORD_PREFIX, ptr[14]);
	ut_asserteq(16, get_unaligned((u16 *)(ptr + 15)));
	uuid_bin_to_str(ptr + 17, uuid, 1);
	ut_asserteq_str(ACPI_DP_CHILD_UUID, uuid);

	/* Package with two children */
	ut_asserteq(PACKAGE_OP, ptr[0x21]);
	ut_asserteq(0x28, acpi_test_get_length(ptr + 0x22));
	ut_asserteq(2, ptr[0x25]);

	/* First we expect the two children as string/value */
	pptr = ptr + 0x26;
	for (i = 0; i < 2; i++) {
		ut_asserteq(PACKAGE_OP, pptr[0]);
		ut_asserteq(0x11, acpi_test_get_length(pptr + 1));
		ut_asserteq(2, pptr[4]);
		ut_asserteq(STRING_PREFIX, pptr[5]);
		ut_asserteq_str(i ? "john" : "anna", (char *)pptr + 6);
		ut_asserteq(STRING_PREFIX, pptr[11]);
		ut_asserteq_str("child", (char *)pptr + 12);
		pptr += 0x12;
	}

	/* Write the two children */
	ut_asserteq(0x4a, pptr - ptr);
	for (i = 0; i < 2; i++) {
		const char *prop = i ? "age" : "height";
		const int datalen = i ? 1 : 2;
		int len = strlen(prop) + 1;

		ut_asserteq(NAME_OP, pptr[0]);
		ut_asserteq_strn("chil", (char *)pptr + 1);
		ut_asserteq(PACKAGE_OP, pptr[5]);
		ut_asserteq(0x27 + len + datalen, acpi_test_get_length(pptr + 6));
		ut_asserteq(2, pptr[9]);

		/* UUID */
		ut_asserteq(BUFFER_OP, pptr[10]);
		ut_asserteq(22, acpi_test_get_length(pptr + 11));
		ut_asserteq(WORD_PREFIX, pptr[14]);
		ut_asserteq(16, get_unaligned((u16 *)(pptr + 15)));
		uuid_bin_to_str(pptr + 17, uuid, 1);
		ut_asserteq_str(ACPI_DP_UUID, uuid);
		pptr += 33;

		/* Containing package */
		ut_asserteq(i ? 0xa1 : 0x6b, pptr - ptr);
		ut_asserteq(PACKAGE_OP, pptr[0]);
		ut_asserteq(0xb + len + datalen, acpi_test_get_length(pptr + 1));
		ut_asserteq(1, pptr[4]);

		/* Package containing the property-name string and the value */
		pptr += 5;
		ut_asserteq(i ? 0xa6 : 0x70, pptr - ptr);
		ut_asserteq(PACKAGE_OP, pptr[0]);
		ut_asserteq(6 + len + datalen, acpi_test_get_length(pptr + 1));
		ut_asserteq(2, pptr[4]);

		ut_asserteq(STRING_PREFIX, pptr[5]);
		ut_asserteq_str(i ? "age" : "height", (char *)pptr + 6);
		pptr += 6 + len;
		if (i) {
			ut_asserteq(BYTE_PREFIX, pptr[0]);
			ut_asserteq(TEST_INT8, pptr[1]);
		} else {
			ut_asserteq(WORD_PREFIX, pptr[0]);
			ut_asserteq(TEST_INT16,
				    get_unaligned((u16 *)(pptr + 1)));
		}
		pptr += 1 + datalen;
	}
	ut_asserteq(178, pptr - ptr);

	free_context(&ctx);

	return 0;
}
DM_TEST(dm_test_acpi_dp_child, 0);

/* Test emitting a GPIO */
static int dm_test_acpi_dp_gpio(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct acpi_dp *dp;
	u8 *ptr, *pptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);

	/* Try a few different parameters */
	ut_assertnonnull(acpi_dp_add_gpio(dp, "reset", TEST_REF, 0x23, 0x24,
					  ACPI_GPIO_ACTIVE_HIGH));
	ut_assertnonnull(acpi_dp_add_gpio(dp, "allow", TEST_REF, 0, 0,
					  ACPI_GPIO_ACTIVE_LOW));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(0x6e, acpigen_get_current(ctx) - ptr);

	pptr = ptr + 0x2c; //0x3a;
	ut_asserteq_str("reset", (char *)pptr);
	ut_asserteq_strn(EXPECT_REF, (char *)pptr + 0xe);
	ut_asserteq(0x23, pptr[0x1b]);
	ut_asserteq(0x24, pptr[0x1d]);
	ut_asserteq(ZERO_OP, pptr[0x1e]);

	pptr = ptr + 0x51;
	ut_asserteq_str("allow", (char *)pptr);
	ut_asserteq_strn(EXPECT_REF, (char *)pptr + 0xe);
	ut_asserteq(ZERO_OP, pptr[0x1a]);
	ut_asserteq(ZERO_OP, pptr[0x1b]);
	ut_asserteq(ONE_OP, pptr[0x1c]);

	return 0;
}
DM_TEST(dm_test_acpi_dp_gpio, 0);

/* Test copying info from the device tree to ACPI tables */
static int dm_test_acpi_dp_copy(struct unit_test_state *uts)
{
	struct acpi_ctx *ctx;
	struct udevice *dev;
	struct acpi_dp *dp;
	ofnode node;
	u8 *ptr;

	ut_assertok(alloc_context(&ctx));

	dp = acpi_dp_new_table("FRED");
	ut_assertnonnull(dp);

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 0, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(acpi_dp_dev_copy_int(dev, dp, "int-value"));
	ut_asserteq(-EINVAL, acpi_dp_dev_copy_int(dev, dp, "missing-value"));
	ut_assertok(acpi_dp_dev_copy_int(dev, dp, "uint-value"));

	ut_assertok(acpi_dp_dev_copy_str(dev, dp, "str-value"));
	ut_asserteq(-EINVAL, acpi_dp_dev_copy_str(dev, dp, "missing-value"));

	node = ofnode_path("/chosen");
	ut_assert(ofnode_valid(node));
	ut_assertok(acpi_dp_ofnode_copy_int(node, dp, "int-values"));
	ut_asserteq(-EINVAL,
		    acpi_dp_ofnode_copy_int(node, dp, "missing-value"));

	ut_assertok(acpi_dp_ofnode_copy_str(node, dp, "setting"));
	ut_asserteq(-EINVAL,
		    acpi_dp_ofnode_copy_str(node, dp, "missing-value"));

	ptr = acpigen_get_current(ctx);
	ut_assertok(acpi_dp_write(ctx, dp));
	ut_asserteq(0x9d, acpigen_get_current(ctx) - ptr);

	ut_asserteq(STRING_PREFIX, ptr[0x2b]);
	ut_asserteq_str("int-value", (char *)ptr + 0x2c);
	ut_asserteq(WORD_PREFIX, ptr[0x36]);
	ut_asserteq(1234, get_unaligned((u16 *)(ptr + 0x37)));

	ut_asserteq(STRING_PREFIX, ptr[0x3e]);
	ut_asserteq_str("uint-value", (char *)ptr + 0x3f);
	ut_asserteq(DWORD_PREFIX, ptr[0x4a]);
	ut_asserteq(-1234, get_unaligned((u32 *)(ptr + 0x4b)));

	ut_asserteq(STRING_PREFIX, ptr[0x54]);
	ut_asserteq_str("str-value", (char *)ptr + 0x55);
	ut_asserteq(STRING_PREFIX, ptr[0x5f]);
	ut_asserteq_str("test string", (char *)ptr + 0x60);

	ut_asserteq(STRING_PREFIX, ptr[0x71]);
	ut_asserteq_str("int-values", (char *)ptr + 0x72);
	ut_asserteq(WORD_PREFIX, ptr[0x7d]);
	ut_asserteq(0x1937, get_unaligned((u16 *)(ptr + 0x7e)));

	ut_asserteq(STRING_PREFIX, ptr[0x85]);
	ut_asserteq_str("setting", (char *)ptr + 0x86);
	ut_asserteq(STRING_PREFIX, ptr[0x8e]);
	ut_asserteq_str("sunrise ohoka", (char *)(ptr + 0x8f));

	return 0;
}
DM_TEST(dm_test_acpi_dp_copy, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
