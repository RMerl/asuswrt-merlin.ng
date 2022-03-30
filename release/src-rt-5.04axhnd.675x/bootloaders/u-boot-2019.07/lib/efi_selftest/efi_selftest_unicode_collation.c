// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_unicode_collation
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test unicode collation protocol.
 */

#include <efi_selftest.h>

static const efi_guid_t unicode_collation_protocol_guid =
	EFI_UNICODE_COLLATION_PROTOCOL2_GUID;

static struct efi_boot_services *boottime;

static struct efi_unicode_collation_protocol *unicode_collation_protocol;

/**
 * setup() - setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * ReturnValue:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&unicode_collation_protocol_guid, NULL,
					(void **)&unicode_collation_protocol);
	if (ret != EFI_SUCCESS) {
		unicode_collation_protocol = NULL;
		efi_st_error("Unicode collation protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_stri_coll(void)
{
	efi_intn_t ret;
	u16 c1[] = L"first";
	u16 c2[] = L"FIRST";
	u16 c3[] = L"second";

	ret = unicode_collation_protocol->stri_coll(unicode_collation_protocol,
						    c1, c2);
	if (ret) {
		efi_st_error(
			"stri_coll(\"%ps\", \"%ps\") = %d\n", c1, c2, (int)ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->stri_coll(unicode_collation_protocol,
						    c1, c3);
	if (ret >= 0) {
		efi_st_error(
			"stri_coll(\"%ps\", \"%ps\") = %d\n", c1, c3, (int)ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->stri_coll(unicode_collation_protocol,
						    c3, c1);
	if (ret <= 0) {
		efi_st_error(
			"stri_coll(\"%ps\", \"%ps\") = %d\n", c3, c1, (int)ret);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_metai_match(void)
{
	bool ret;
	const u16 c[] = L"Das U-Boot";

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"*");
	if (!ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da[rstu] U-Boot");
	if (!ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da[q-v] U-Boot");
	if (!ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da? U-Boot");
	if (!ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"D*Bo*t");
	if (!ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da[xyz] U-Boot");
	if (ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da[a-d] U-Boot");
	if (ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"Da?? U-Boot");
	if (ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	ret = unicode_collation_protocol->metai_match(
		unicode_collation_protocol, c, L"D*Bo*tt");
	if (ret) {
		efi_st_error("metai_match returned %u\n", ret);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_str_lwr(void)
{
	u16 c[] = L"U-Boot";

	unicode_collation_protocol->str_lwr(unicode_collation_protocol, c);
	if (efi_st_strcmp_16_8(c, "u-boot")) {
		efi_st_error("str_lwr returned \"%ps\"\n", c);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_str_upr(void)
{
	u16 c[] = L"U-Boot";

	unicode_collation_protocol->str_upr(unicode_collation_protocol, c);
	if (efi_st_strcmp_16_8(c, "U-BOOT")) {
		efi_st_error("str_lwr returned \"%ps\"\n", c);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_fat_to_str(void)
{
	u16 str[16];

	boottime->set_mem(str, sizeof(str), 0);
	unicode_collation_protocol->fat_to_str(unicode_collation_protocol, 6,
					       "U-BOOT", str);
	if (efi_st_strcmp_16_8(str, "U-BOOT")) {
		efi_st_error("fat_to_str returned \"%ps\"\n", str);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

static int test_str_to_fat(void)
{
	char fat[16];
	bool ret;

	boottime->set_mem(fat, sizeof(fat), 0);
	ret = unicode_collation_protocol->str_to_fat(unicode_collation_protocol,
						     L"U -Boo.t", 6, fat);
	if (ret || efi_st_strcmp_16_8(L"U-BOOT", fat)) {
		efi_st_error("str_to_fat returned %u, \"%s\"\n", ret, fat);
		return EFI_ST_FAILURE;
	}

	boottime->set_mem(fat, 16, 0);
	ret = unicode_collation_protocol->str_to_fat(unicode_collation_protocol,
						     L"U\\Boot", 6, fat);
	if (!ret || efi_st_strcmp_16_8(L"U_BOOT", fat)) {
		efi_st_error("str_to_fat returned %u, \"%s\"\n", ret, fat);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/**
 * execute() - Execute unit test.
 *
 * ReturnValue:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	int ret;

	if (!unicode_collation_protocol) {
		efi_st_printf("Unicode collation protocol missing\n");
		return EFI_ST_FAILURE;
	}

	ret = test_stri_coll();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	ret = test_metai_match();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	ret = test_str_lwr();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	ret = test_str_upr();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	ret = test_fat_to_str();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	ret = test_str_to_fat();
	if (ret != EFI_ST_SUCCESS)
		return ret;

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(unicoll) = {
	.name = "unicode collation",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
	.setup = setup,
};
