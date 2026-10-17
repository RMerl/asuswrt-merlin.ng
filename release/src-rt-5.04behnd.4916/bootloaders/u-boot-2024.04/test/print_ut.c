// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, The Chromium Authors
 */

#include <common.h>
#include <command.h>
#include <efi_api.h>
#include <display_options.h>
#include <log.h>
#include <mapmem.h>
#include <version_string.h>
#include <vsprintf.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

#define BUF_SIZE	0x100

#define FAKE_BUILD_TAG	"jenkins-u-boot-denx_uboot_dm-master-build-aarch64" \
			"and a lot more text to come"

/* Declare a new print test */
#define PRINT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, print_test)

#if CONFIG_IS_ENABLED(LIB_UUID)
/* Test printing GUIDs */
static int print_guid(struct unit_test_state *uts)
{
	unsigned char guid[16] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	};
	unsigned char guid_esp[16] = {
		0x28, 0x73, 0x2a, 0xc1, 0x1f, 0xf8, 0xd2, 0x11,
		0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
	};
	char str[40];
	int ret;

	sprintf(str, "%pUb", guid);
	ut_asserteq_str("01020304-0506-0708-090a-0b0c0d0e0f10", str);
	sprintf(str, "%pUB", guid);
	ut_asserteq_str("01020304-0506-0708-090A-0B0C0D0E0F10", str);
	sprintf(str, "%pUl", guid);
	ut_asserteq_str("04030201-0605-0807-090a-0b0c0d0e0f10", str);
	sprintf(str, "%pUs", guid);
	ut_asserteq_str("04030201-0605-0807-090a-0b0c0d0e0f10", str);
	sprintf(str, "%pUL", guid);
	ut_asserteq_str("04030201-0605-0807-090A-0B0C0D0E0F10", str);
	sprintf(str, "%pUs", guid_esp);
	if (IS_ENABLED(CONFIG_PARTITION_TYPE_GUID)) { /* brace needed */
		ut_asserteq_str("system", str);
	} else {
		ut_asserteq_str("c12a7328-f81f-11d2-ba4b-00a0c93ec93b", str);
	}
	ret = snprintf(str, 4, "%pUL", guid);
	ut_asserteq(0, str[3]);
	ut_asserteq(36, ret);

	return 0;
}
PRINT_TEST(print_guid, 0);
#endif

#if CONFIG_IS_ENABLED(EFI_LOADER) && !defined(API_BUILD)
/* Test efi_loader specific printing */
static int print_efi_ut(struct unit_test_state *uts)
{
	char str[10];
	u8 buf[sizeof(struct efi_device_path_sd_mmc_path) +
	       sizeof(struct efi_device_path)];
	u8 *pos = buf;
	struct efi_device_path *dp_end;
	struct efi_device_path_sd_mmc_path *dp_sd =
			(struct efi_device_path_sd_mmc_path *)pos;

	/* Create a device path for an SD card */
	dp_sd->dp.type = DEVICE_PATH_TYPE_MESSAGING_DEVICE;
	dp_sd->dp.sub_type = DEVICE_PATH_SUB_TYPE_MSG_SD;
	dp_sd->dp.length = sizeof(struct efi_device_path_sd_mmc_path);
	dp_sd->slot_number = 3;
	pos += sizeof(struct efi_device_path_sd_mmc_path);
	/* Append end node */
	dp_end = (struct efi_device_path *)pos;
	dp_end->type = DEVICE_PATH_TYPE_END;
	dp_end->sub_type = DEVICE_PATH_SUB_TYPE_END;
	dp_end->length = sizeof(struct efi_device_path);

	snprintf(str, sizeof(str), "_%pD_", buf);
	ut_assertok(strcmp("_/SD(3)_", str));

	/* NULL device path */
	snprintf(str, sizeof(str), "_%pD_", NULL);
	ut_assertok(strcmp("_<NULL>_", str));

	return 0;
}
PRINT_TEST(print_efi_ut, 0);
#endif

static int print_printf(struct unit_test_state *uts)
{
	char big_str[400];
	int big_str_len;
	char str[10], *s;
	int len;

	snprintf(str, sizeof(str), "testing");
	ut_assertok(strcmp("testing", str));

	snprintf(str, sizeof(str), "testing but too long");
	ut_assertok(strcmp("testing b", str));

	snprintf(str, 1, "testing none");
	ut_assertok(strcmp("", str));

	*str = 'x';
	snprintf(str, 0, "testing none");
	ut_asserteq('x', *str);

	sprintf(big_str, "_%ls_", u"foo");
	ut_assertok(strcmp("_foo_", big_str));

	/* Test the banner function */
	s = display_options_get_banner(true, str, sizeof(str));
	ut_asserteq_ptr(str, s);
	ut_assertok(strcmp("\n\nU-Boo\n\n", s));

	/* Assert that we do not overwrite memory before the buffer */
	str[0] = '`';
	s = display_options_get_banner(true, str + 1, 1);
	ut_asserteq_ptr(str + 1, s);
	ut_assertok(strcmp("`", str));

	str[0] = '~';
	s = display_options_get_banner(true, str + 1, 2);
	ut_asserteq_ptr(str + 1, s);
	ut_assertok(strcmp("~\n", str));

	/* The last two characters are set to \n\n for all buffer sizes > 2 */
	s = display_options_get_banner(false, str, sizeof(str));
	ut_asserteq_ptr(str, s);
	ut_assertok(strcmp("U-Boot \n\n", s));

	/* Give it enough space for some of the version */
	big_str_len = strlen(version_string) - 5;
	s = display_options_get_banner_priv(false, FAKE_BUILD_TAG, big_str,
					    big_str_len);
	ut_asserteq_ptr(big_str, s);
	ut_assertok(strncmp(version_string, s, big_str_len - 3));
	ut_assertok(strcmp("\n\n", s + big_str_len - 3));

	/* Give it enough space for the version and some of the build tag */
	big_str_len = strlen(version_string) + 9 + 20;
	s = display_options_get_banner_priv(false, FAKE_BUILD_TAG, big_str,
					    big_str_len);
	ut_asserteq_ptr(big_str, s);
	len = strlen(version_string);
	ut_assertok(strncmp(version_string, s, len));
	ut_assertok(strncmp(", Build: ", s + len, 9));
	ut_assertok(strncmp(FAKE_BUILD_TAG, s + 9 + len, 12));
	ut_assertok(strcmp("\n\n", s + big_str_len - 3));

	return 0;
}
PRINT_TEST(print_printf, 0);

static int print_display_buffer(struct unit_test_state *uts)
{
	u8 *buf;
	int i;

	/* This test requires writable memory at zero */
	if (IS_ENABLED(CONFIG_X86))
		return -EAGAIN;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	for (i = 0; i < 0x11; i++)
		buf[i] = i * 0x11;

	/* bytes */
	console_record_reset();
	print_buffer(0, buf, 1, 0x12, 0);
	ut_assert_nextline("00000000: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff  ..\"3DUfw........");
	ut_assert_nextline("00000010: 10 00                                            ..");
	ut_assert_console_end();

	/* line length */
	console_record_reset();
	print_buffer(0, buf, 1, 0x12, 8);
	ut_assert_nextline("00000000: 00 11 22 33 44 55 66 77  ..\"3DUfw");
	ut_assert_nextline("00000008: 88 99 aa bb cc dd ee ff  ........");
	ut_assert_nextline("00000010: 10 00                    ..");
	ut_assert_console_end();

	/* long line */
	console_record_reset();
	buf[0x41] = 0x41;
	print_buffer(0, buf, 1, 0x42, 0x40);
	ut_assert_nextline("00000000: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ..\"3DUfw........................................................");
	ut_assert_nextline("00000040: 00 41                                                                                                                                                                                            .A");
	ut_assert_console_end();

	/* address */
	console_record_reset();
	print_buffer(0x12345678, buf, 1, 0x12, 0);
	ut_assert_nextline("12345678: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff  ..\"3DUfw........");
	ut_assert_nextline("12345688: 10 00                                            ..");
	ut_assert_console_end();

	/* 16-bit */
	console_record_reset();
	print_buffer(0, buf, 2, 9, 0);
	ut_assert_nextline("00000000: 1100 3322 5544 7766 9988 bbaa ddcc ffee  ..\"3DUfw........");
	ut_assert_nextline("00000010: 0010                                     ..");
	ut_assert_console_end();

	/* 32-bit */
	console_record_reset();
	print_buffer(0, buf, 4, 5, 0);
	ut_assert_nextline("00000000: 33221100 77665544 bbaa9988 ffeeddcc  ..\"3DUfw........");
	ut_assert_nextline("00000010: 00000010                             ....");
	ut_assert_console_end();

	/* 64-bit */
	console_record_reset();
	print_buffer(0, buf, 8, 3, 0);
	ut_assert_nextline("00000000: 7766554433221100 ffeeddccbbaa9988  ..\"3DUfw........");
	ut_assert_nextline("00000010: 0000000000000010                   ........");
	ut_assert_console_end();

	/* ASCII */
	console_record_reset();
	buf[1] = 31;
	buf[2] = 32;
	buf[3] = 33;
	for (i = 0; i < 4; i++)
		buf[4 + i] = 126 + i;
	buf[8] = 255;
	print_buffer(0, buf, 1, 10, 0);
	ut_assert_nextline("00000000: 00 1f 20 21 7e 7f 80 81 ff 99                    .. !~.....");
	ut_assert_console_end();

	unmap_sysmem(buf);

	return 0;
}
PRINT_TEST(print_display_buffer, UT_TESTF_CONSOLE_REC);

static int print_hexdump_line(struct unit_test_state *uts)
{
	char *linebuf;
	u8 *buf;
	int i;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	for (i = 0; i < 0x11; i++)
		buf[i] = i * 0x11;

	/* Check buffer size calculations */
	linebuf = map_sysmem(0x400, BUF_SIZE);
	memset(linebuf, '\xff', BUF_SIZE);
	ut_asserteq(-ENOSPC, hexdump_line(0, buf, 1, 0x10, 0, linebuf, 75));
	ut_asserteq(-1, linebuf[0]);
	ut_asserteq(0x10, hexdump_line(0, buf, 1, 0x10, 0, linebuf, 76));
	ut_asserteq(0, linebuf[75]);
	ut_asserteq(-1, linebuf[76]);

	unmap_sysmem(buf);

	return 0;
}
PRINT_TEST(print_hexdump_line, UT_TESTF_CONSOLE_REC);

static int print_do_hex_dump(struct unit_test_state *uts)
{
	u8 *buf;
	int i;

	/* This test requires writable memory at zero */
	if (IS_ENABLED(CONFIG_X86))
		return -EAGAIN;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	for (i = 0; i < 0x11; i++)
		buf[i] = i * 0x11;

	/* bytes */
	console_record_reset();
	print_hex_dump_bytes("", DUMP_PREFIX_ADDRESS, buf, 0x12);
	ut_assert_nextline("%0*lx: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff  ..\"3DUfw........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 10 00                                            ..",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x10UL);
	ut_assert_console_end();

	/* line length */
	console_record_reset();
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 8, 1, buf, 0x12, true);
	ut_assert_nextline("%0*lx: 00 11 22 33 44 55 66 77  ..\"3DUfw",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 88 99 aa bb cc dd ee ff  ........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x8UL);
	ut_assert_nextline("%0*lx: 10 00                    ..",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x10UL);
	ut_assert_console_end();
	unmap_sysmem(buf);

	/* long line */
	console_record_reset();
	buf[0x41] = 0x41;
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 0x40, 1, buf, 0x42, true);
	ut_assert_nextline("%0*lx: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ..\"3DUfw........................................................",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 00 41                                                                                                                                                                                            .A",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x40UL);
	ut_assert_console_end();

	/* 16-bit */
	console_record_reset();
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 0, 2, buf, 0x12, true);
	ut_assert_nextline("%0*lx: 1100 3322 5544 7766 9988 bbaa ddcc ffee  ..\"3DUfw........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 0010                                     ..",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x10UL);
	ut_assert_console_end();
	unmap_sysmem(buf);

	/* 32-bit */
	console_record_reset();
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 0, 4, buf, 0x14, true);
	ut_assert_nextline("%0*lx: 33221100 77665544 bbaa9988 ffeeddcc  ..\"3DUfw........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 00000010                             ....",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x10UL);
	ut_assert_console_end();
	unmap_sysmem(buf);

	/* 64-bit */
	console_record_reset();
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 16, 8, buf, 0x18, true);
	ut_assert_nextline("%0*lx: 7766554433221100 ffeeddccbbaa9988  ..\"3DUfw........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_nextline("%0*lx: 0000000000000010                   ........",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x10UL);
	ut_assert_console_end();
	unmap_sysmem(buf);

	/* ASCII */
	console_record_reset();
	buf[1] = 31;
	buf[2] = 32;
	buf[3] = 33;
	for (i = 0; i < 4; i++)
		buf[4 + i] = 126 + i;
	buf[8] = 255;
	print_hex_dump("", DUMP_PREFIX_ADDRESS, 0, 1, buf, 10, true);
	ut_assert_nextline("%0*lx: 00 1f 20 21 7e 7f 80 81 ff 99                    .. !~.....",
			   IS_ENABLED(CONFIG_PHYS_64BIT) ? 16 : 8, 0x0UL);
	ut_assert_console_end();
	unmap_sysmem(buf);

	return 0;
}
PRINT_TEST(print_do_hex_dump, UT_TESTF_CONSOLE_REC);

static int snprint(struct unit_test_state *uts)
{
	char buf[10] = "xxxxxxxxx";
	int ret;

	ret = snprintf(buf, 5, "%d", 12345678);
	ut_asserteq_str("1234", buf);
	ut_asserteq(8, ret);
	ret = snprintf(buf, 5, "0x%x", 0x1234);
	ut_asserteq_str("0x12", buf);
	ut_asserteq(6, ret);
	ret = snprintf(buf, 5, "0x%08x", 0x1234);
	ut_asserteq_str("0x00", buf);
	ut_asserteq(10, ret);
	ret = snprintf(buf, 3, "%s", "abc");
	ut_asserteq_str("ab", buf);
	ut_asserteq(3, ret);
	ret = snprintf(buf, 4, "%s:%s", "abc", "def");
	ut_asserteq(0, buf[3]);
	ut_asserteq(7, ret);
	ret = snprintf(buf, 4, "%s:%d", "abc", 9999);
	ut_asserteq(8, ret);
	return 0;
}
PRINT_TEST(snprint, 0);

int do_ut_print(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(print_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(print_test);

	return cmd_ut_category("print", "print_", tests, n_ents, argc, argv);
}
