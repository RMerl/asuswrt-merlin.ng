// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_variables_runtime
 *
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the runtime services for variables after
 * ExitBootServices():
 * GetVariable, GetNextVariableName, SetVariable, QueryVariableInfo.
 */

#include <efi_selftest.h>

#define EFI_ST_MAX_DATA_SIZE 16
#define EFI_ST_MAX_VARNAME_SIZE 40

static struct efi_boot_services *boottime;
static struct efi_runtime_services *runtime;
static const efi_guid_t guid_vendor0 = EFI_GLOBAL_VARIABLE_GUID;

/*
 * Setup unit test.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	runtime = systable->runtime;

	return EFI_ST_SUCCESS;
}

/**
 * execute() - execute unit test
 *
 * As runtime support is not implmented expect EFI_UNSUPPORTED to be returned.
 */
static int execute(void)
{
	efi_status_t ret;
	efi_uintn_t len;
	u32 attr;
	u8 v[16] = {0x5d, 0xd1, 0x5e, 0x51, 0x5a, 0x05, 0xc7, 0x0c,
		    0x35, 0x4a, 0xae, 0x87, 0xa5, 0xdf, 0x0f, 0x65,};
	u8 data[EFI_ST_MAX_DATA_SIZE];
	u16 varname[EFI_ST_MAX_VARNAME_SIZE];
	efi_guid_t guid;
	u64 max_storage, rem_storage, max_size;

	ret = runtime->query_variable_info(EFI_VARIABLE_BOOTSERVICE_ACCESS,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_UNSUPPORTED) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->set_variable(u"efi_st_var0", &guid_vendor0,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS |
				    EFI_VARIABLE_RUNTIME_ACCESS,
				    3, v + 4);
	if (ret != EFI_UNSUPPORTED) {
		efi_st_error("SetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	len = EFI_ST_MAX_DATA_SIZE;
	ret = runtime->get_variable(u"PlatformLangCodes", &guid_vendor0,
				    &attr, &len, data);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	memset(&guid, 0, 16);
	*varname = 0;
	len = 2 * EFI_ST_MAX_VARNAME_SIZE;
	ret = runtime->get_next_variable_name(&len, varname, &guid);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetNextVariableName failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(variables_run) = {
	.name = "variables at runtime",
	.phase = EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
