// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_variables
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This unit test checks the runtime services for variables:
 * GetVariable, GetNextVariableName, SetVariable, QueryVariableInfo.
 */

#include <efi_selftest.h>

#define EFI_ST_MAX_DATA_SIZE 16
#define EFI_ST_MAX_VARNAME_SIZE 40

static struct efi_boot_services *boottime;
static struct efi_runtime_services *runtime;
static const efi_guid_t guid_vendor0 =
	EFI_GUID(0x67029eb5, 0x0af2, 0xf6b1,
		 0xda, 0x53, 0xfc, 0xb5, 0x66, 0xdd, 0x1c, 0xe6);
static const efi_guid_t guid_vendor1 =
	EFI_GUID(0xff629290, 0x1fc1, 0xd73f,
		 0x8f, 0xb1, 0x32, 0xf9, 0x0c, 0xa0, 0x42, 0xea);

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

/*
 * Execute unit test.
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
	int flag;
	efi_guid_t guid;
	u64 max_storage, rem_storage, max_size;

	ret = runtime->query_variable_info(EFI_VARIABLE_BOOTSERVICE_ACCESS,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_SUCCESS) {
		efi_st_todo("QueryVariableInfo failed\n");
	} else if (!max_storage || !rem_storage || !max_size) {
		efi_st_error("QueryVariableInfo: wrong info\n");
		return EFI_ST_FAILURE;
	}
	/* Set variable 0 */
	ret = runtime->set_variable(L"efi_st_var0", &guid_vendor0,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS,
				    3, v + 4);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	data[3] = 0xff;
	len = 3;
	ret = runtime->get_variable(L"efi_st_var0", &guid_vendor0,
				    &attr, &len, data);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	if (memcmp(data, v + 4, 3)) {
		efi_st_error("GetVariable returned wrong value\n");
		return EFI_ST_FAILURE;
	}
	if (data[3] != 0xff) {
		efi_st_error("GetVariable wrote past the end of the buffer\n");
		return EFI_ST_FAILURE;
	}
	/* Set variable 1 */
	ret = runtime->set_variable(L"efi_st_var1", &guid_vendor1,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS,
				    8, v);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	len = EFI_ST_MAX_DATA_SIZE;
	ret = runtime->get_variable(L"efi_st_var1", &guid_vendor1,
				    &attr, &len, data);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	if (len != 8) {
		efi_st_error("GetVariable returned wrong length %u\n",
			     (unsigned int)len);
		return EFI_ST_FAILURE;
	}
	if (memcmp(data, v, 8)) {
		efi_st_error("GetVariable returned wrong value\n");
		return EFI_ST_FAILURE;
	}
	/* Append variable 1 */
	ret = runtime->set_variable(L"efi_st_var1", &guid_vendor1,
				    EFI_VARIABLE_BOOTSERVICE_ACCESS |
				    EFI_VARIABLE_APPEND_WRITE,
				    7, v + 8);
	if (ret != EFI_SUCCESS) {
		efi_st_todo("SetVariable(APPEND_WRITE) failed\n");
	} else {
		len = EFI_ST_MAX_DATA_SIZE;
		ret = runtime->get_variable(L"efi_st_var1", &guid_vendor1,
					    &attr, &len, data);
		if (ret != EFI_SUCCESS) {
			efi_st_error("GetVariable failed\n");
			return EFI_ST_FAILURE;
		}
		if (len != 15)
			efi_st_todo("GetVariable returned wrong length %u\n",
				    (unsigned int)len);
		if (memcmp(data, v, len))
			efi_st_todo("GetVariable returned wrong value\n");
	}
	/* Enumerate variables */
	boottime->set_mem(&guid, 16, 0);
	*varname = 0;
	flag = 0;
	for (;;) {
		len = EFI_ST_MAX_VARNAME_SIZE;
		ret = runtime->get_next_variable_name(&len, varname, &guid);
		if (ret == EFI_NOT_FOUND)
			break;
		if (ret != EFI_SUCCESS) {
			efi_st_error("GetNextVariableName failed (%u)\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
		if (!memcmp(&guid, &guid_vendor0, sizeof(efi_guid_t)) &&
		    !efi_st_strcmp_16_8(varname, "efi_st_var0"))
			flag |= 1;
		if (!memcmp(&guid, &guid_vendor1, sizeof(efi_guid_t)) &&
		    !efi_st_strcmp_16_8(varname, "efi_st_var1"))
			flag |= 2;
	}
	if (flag != 3) {
		efi_st_error(
			"GetNextVariableName did not return all variables\n");
		return EFI_ST_FAILURE;
	}
	/* Delete variable 1 */
	ret = runtime->set_variable(L"efi_st_var1", &guid_vendor1,
				    0, 0, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	len = EFI_ST_MAX_DATA_SIZE;
	ret = runtime->get_variable(L"efi_st_var1", &guid_vendor1,
				    &attr, &len, data);
	if (ret != EFI_NOT_FOUND) {
		efi_st_error("Variable was not deleted\n");
		return EFI_ST_FAILURE;
	}
	/* Delete variable 0 */
	ret = runtime->set_variable(L"efi_st_var0", &guid_vendor0,
				    0, 0, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("SetVariable failed\n");
		return EFI_ST_FAILURE;
	}
	len = EFI_ST_MAX_DATA_SIZE;
	ret = runtime->get_variable(L"efi_st_var0", &guid_vendor0,
				    &attr, &len, data);
	if (ret != EFI_NOT_FOUND) {
		efi_st_error("Variable was not deleted\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(variables) = {
	.name = "variables",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
