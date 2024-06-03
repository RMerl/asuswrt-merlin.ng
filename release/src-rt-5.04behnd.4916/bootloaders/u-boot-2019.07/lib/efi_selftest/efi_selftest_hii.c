// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_hii
 *
 * Copyright (c) 2018 AKASHI Takahiro, Linaro Limited
 *
 * Test HII database protocols
 */

#include <efi_selftest.h>
#include "efi_selftest_hii_data.c"

#define PRINT_TESTNAME efi_st_printf("%s:\n", __func__)

static struct efi_boot_services *boottime;

static const efi_guid_t hii_database_protocol_guid =
	EFI_HII_DATABASE_PROTOCOL_GUID;
static const efi_guid_t hii_string_protocol_guid =
	EFI_HII_STRING_PROTOCOL_GUID;

static struct efi_hii_database_protocol *hii_database_protocol;
static struct efi_hii_string_protocol *hii_string_protocol;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	efi_status_t ret;

	boottime = systable->boottime;

	/* HII database protocol */
	ret = boottime->locate_protocol(&hii_database_protocol_guid, NULL,
					(void **)&hii_database_protocol);
	if (ret != EFI_SUCCESS) {
		hii_database_protocol = NULL;
		efi_st_error("HII database protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	/* HII string protocol */
	ret = boottime->locate_protocol(&hii_string_protocol_guid, NULL,
					(void **)&hii_string_protocol);
	if (ret != EFI_SUCCESS) {
		hii_string_protocol = NULL;
		efi_st_error("HII string protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * HII database protocol tests
 */

/**
 * test_hii_database_new_package_list() - test creation and removal of
 *	package list
 *
 * This test adds a new package list and then tries to remove it using
 * the provided handle.
 *
 * @Return:     status code
 */
static int test_hii_database_new_package_list(void)
{
	efi_hii_handle_t handle;
	efi_status_t ret;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	ret = hii_database_protocol->remove_package_list(hii_database_protocol,
			handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error("remove_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/**
 * test_hii_database_update_package_list() - test update of package list
 *
 * This test adds a new package list and then tries to update it using
 * another package list.
 *
 * @Return:     status code
 */
static int test_hii_database_update_package_list(void)
{
	efi_hii_handle_t handle = NULL;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	ret = hii_database_protocol->update_package_list(hii_database_protocol,
			handle,
			(struct efi_hii_package_list_header *)packagelist2);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * test_hii_database_list_package_lists() - test listing of package lists
 *
 * This test adds two package lists and then tries to enumerate them
 * against different package types. We will get an array of handles.
 *
 * @Return:     status code
 */
static int test_hii_database_list_package_lists(void)
{
	efi_hii_handle_t handle1 = NULL, handle2 = NULL, *handles;
	efi_uintn_t handles_size;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle1);
	if (ret != EFI_SUCCESS || !handle1) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist2,
			NULL, &handle2);
	if (ret != EFI_SUCCESS || !handle2) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	/* TYPE_ALL */
	handles = NULL;
	handles_size = 0;
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_TYPE_ALL, NULL,
			&handles_size, handles);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, handles_size,
				      (void **)&handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_TYPE_ALL, NULL,
			&handles_size, handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		goto out;
	}

	/* STRINGS */
	handles = NULL;
	handles_size = 0;
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_STRINGS, NULL,
			&handles_size, handles);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, handles_size,
				      (void **)&handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_STRINGS, NULL,
			&handles_size, handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		goto out;
	}

	/* GUID */
	handles = NULL;
	handles_size = 0;
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_TYPE_GUID, &package_guid,
			&handles_size, handles);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, handles_size,
				      (void **)&handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_TYPE_GUID, &package_guid,
			&handles_size, handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		ret = EFI_ST_FAILURE;
		goto out;
	}

	/* KEYBOARD_LAYOUT */
	handles = NULL;
	handles_size = 0;
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_KEYBOARD_LAYOUT, NULL,
			&handles_size, handles);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, handles_size,
				      (void **)&handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = hii_database_protocol->list_package_lists(hii_database_protocol,
			EFI_HII_PACKAGE_KEYBOARD_LAYOUT, NULL,
			&handles_size, handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("list_package_lists returned %u\n",
			     (unsigned int)ret);
		ret = EFI_ST_FAILURE;
		goto out;
	}
	ret = boottime->free_pool(handles);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		ret = EFI_ST_FAILURE;
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle1) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle1);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}
	if (handle2) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle2);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}

	return result;
}

/**
 * test_hii_database_export_package_lists() - test export of package lists
 *
 * @Return:     status code
 */
static int test_hii_database_export_package_lists(void)
{
	PRINT_TESTNAME;
	/* export_package_lists() not implemented yet */
	return EFI_ST_SUCCESS;
}

/**
 * test_hii_database_register_package_notify() - test registration of
 *	notification function
 *
 * @Return:     status code
 */
static int test_hii_database_register_package_notify(void)
{
	PRINT_TESTNAME;
	/* register_package_notify() not implemented yet */
	return EFI_ST_SUCCESS;
}

/**
 * test_hii_database_unregister_package_notify() - test removal of
 *	notification function
 *
 * @Return:     status code
 */
static int test_hii_database_unregister_package_notify(void)
{
	PRINT_TESTNAME;
	/* unregsiter_package_notify() not implemented yet */
	return EFI_ST_SUCCESS;
}

/**
 * test_hii_database_find_keyboard_layouts() - test listing of
 *	all the keyboard layouts in the system
 *
 * This test adds two package lists, each of which has two keyboard layouts
 * and then tries to enumerate them. We will get an array of handles.
 *
 * @Return:     status code
 */
static int test_hii_database_find_keyboard_layouts(void)
{
	efi_hii_handle_t handle1 = NULL, handle2 = NULL;
	efi_guid_t *guids;
	u16 guids_size;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle1);
	if (ret != EFI_SUCCESS || !handle1) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist2,
			NULL, &handle2);
	if (ret != EFI_SUCCESS || !handle2) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	guids = NULL;
	guids_size = 0;
	ret = hii_database_protocol->find_keyboard_layouts(
			hii_database_protocol, &guids_size, guids);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("find_keyboard_layouts returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, guids_size,
				      (void **)&guids);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_database_protocol->find_keyboard_layouts(
			hii_database_protocol, &guids_size, guids);
	if (ret != EFI_SUCCESS) {
		efi_st_error("find_keyboard_layouts returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->free_pool(guids);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle1) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle1);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}
	if (handle2) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle2);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}

	return result;
}

/**
 * test_hii_database_get_keyboard_layout() - test retrieval of keyboard layout
 *
 * This test adds two package lists, each of which has two keyboard layouts
 * and then tries to get a handle to keyboard layout with a specific guid
 * and the current one.
 *
 * @Return:     status code
 */
static int test_hii_database_get_keyboard_layout(void)
{
	efi_hii_handle_t handle1 = NULL, handle2 = NULL;
	struct efi_hii_keyboard_layout *kb_layout;
	u16 kb_layout_size;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle1);
	if (ret != EFI_SUCCESS || !handle1) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist2,
			NULL, &handle2);
	if (ret != EFI_SUCCESS || !handle2) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	/* specific keyboard_layout(guid11) */
	kb_layout = NULL;
	kb_layout_size = 0;
	ret = hii_database_protocol->get_keyboard_layout(hii_database_protocol,
			&kb_layout_guid11, &kb_layout_size, kb_layout);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("get_keyboard_layout returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, kb_layout_size,
				      (void **)&kb_layout);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_database_protocol->get_keyboard_layout(hii_database_protocol,
			&kb_layout_guid11, &kb_layout_size, kb_layout);
	if (ret != EFI_SUCCESS) {
		efi_st_error("get_keyboard_layout returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->free_pool(kb_layout);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePool failed\n");
		goto out;
	}

	/* current */
	kb_layout = NULL;
	kb_layout_size = 0;
	ret = hii_database_protocol->get_keyboard_layout(hii_database_protocol,
			NULL, &kb_layout_size, kb_layout);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("get_keyboard_layout returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle1) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle1);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}
	if (handle2) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle2);
		if (ret != EFI_SUCCESS)
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
	}

	return result;
}

/**
 * test_hii_database_set_keyboard_layout() - test change of
 *	current keyboard layout
 *
 * @Return:     status code
 */
static int test_hii_database_set_keyboard_layout(void)
{
	PRINT_TESTNAME;
	/* set_keyboard_layout() not implemented yet */
	return EFI_ST_SUCCESS;
}

/**
 * test_hii_database_get_package_list_handle() - test retrieval of
 *	driver associated with a package list
 *
 * This test adds a package list, and then tries to get a handle to driver
 * which is associated with a package list.
 *
 * @Return:     status code
 */
static int test_hii_database_get_package_list_handle(void)
{
	efi_hii_handle_t handle = NULL;
	efi_handle_t driver_handle;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	driver_handle = (efi_handle_t)0x12345678; /* dummy */
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			driver_handle, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	driver_handle = NULL;
	ret = hii_database_protocol->get_package_list_handle(
			hii_database_protocol, handle, &driver_handle);
	if (ret != EFI_SUCCESS || driver_handle != (efi_handle_t)0x12345678) {
		efi_st_error("get_package_list_handle returned %u, driver:%p\n",
			     (unsigned int)ret, driver_handle);
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

static int test_hii_database_protocol(void)
{
	int ret;

	ret = test_hii_database_new_package_list();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_update_package_list();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_list_package_lists();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_export_package_lists();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_register_package_notify();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_unregister_package_notify();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_find_keyboard_layouts();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_get_keyboard_layout();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_set_keyboard_layout();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_database_get_package_list_handle();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	return EFI_ST_SUCCESS;
}

/*
 * HII string protocol tests
 */

/**
 * test_hii_string_new_string() - test creation of a new string entry
 *
 * This test adds a package list, and then tries to add a new string
 * entry for a specific language.
 *
 * @Return:     status code
 */
static int test_hii_string_new_string(void)
{
	efi_hii_handle_t handle = NULL;
	efi_string_id_t id;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	ret = hii_string_protocol->new_string(hii_string_protocol, handle,
					      &id, (u8 *)"en-US",
					      L"Japanese", L"Japanese", NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("new_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	efi_st_printf("new string id is %u\n", id);

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * test_hii_string_get_string() - test retrieval of a string entry
 *
 * This test adds a package list, create a new string entry and then tries
 * to get it with its string id.
 *
 * @Return:     status code
 */
static int test_hii_string_get_string(void)
{
	efi_hii_handle_t handle = NULL;
	efi_string_id_t id;
	efi_string_t string;
	efi_uintn_t string_len;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	ret = hii_string_protocol->new_string(hii_string_protocol, handle,
					      &id, (u8 *)"en-US",
					      L"Japanese", L"Japanese", NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("new_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	string = NULL;
	string_len = 0;
	ret = hii_string_protocol->get_string(hii_string_protocol,
			(u8 *)"en-US", handle, id, string, &string_len, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("get_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	string_len += sizeof(u16);
	ret = boottime->allocate_pool(EFI_LOADER_DATA, string_len,
				      (void **)&string);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_string_protocol->get_string(hii_string_protocol,
			(u8 *)"en-US", handle, id, string, &string_len, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("get_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	if (efi_st_strcmp_16_8(string, "Japanese")) {
		efi_st_error("get_string returned incorrect string\n");
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * test_hii_string_set_string() - test change of a string entry
 *
 * This test adds a package list, create a new string entry and then tries
 * to modify it.
 *
 * @Return:     status code
 */
static int test_hii_string_set_string(void)
{
	efi_hii_handle_t handle = NULL;
	efi_string_id_t id;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	ret = hii_string_protocol->new_string(hii_string_protocol, handle,
					      &id, (u8 *)"en-US",
					      L"Japanese", L"Japanese", NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("new_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	ret = hii_string_protocol->set_string(hii_string_protocol, handle,
					      id, (u8 *)"en-US",
					      L"Nihongo", NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("set_string returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * test_hii_string_get_languages() - test listing of languages
 *
 * This test adds a package list, and then tries to enumerate languages
 * in it. We will get an string of language names.
 *
 * @Return:     status code
 */
static int test_hii_string_get_languages(void)
{
	efi_hii_handle_t handle = NULL;
	u8 *languages;
	efi_uintn_t languages_len;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	languages = NULL;
	languages_len = 0;
	ret = hii_string_protocol->get_languages(hii_string_protocol, handle,
			languages, &languages_len);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("get_languages returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, languages_len,
				      (void **)&languages);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_string_protocol->get_languages(hii_string_protocol, handle,
			languages, &languages_len);
	if (ret != EFI_SUCCESS) {
		efi_st_error("get_languages returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	efi_st_printf("got languages are %s\n", languages);

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * test_hii_string_get_secondary_languages() - test listing of secondary
 *	languages
 *
 * This test adds a package list, and then tries to enumerate secondary
 * languages with a specific language. We will get an string of language names.
 *
 * @Return:     status code
 */
static int test_hii_string_get_secondary_languages(void)
{
	efi_hii_handle_t handle = NULL;
	u8 *languages;
	efi_uintn_t languages_len;
	efi_status_t ret;
	int result = EFI_ST_FAILURE;

	PRINT_TESTNAME;
	ret = hii_database_protocol->new_package_list(hii_database_protocol,
			(struct efi_hii_package_list_header *)packagelist1,
			NULL, &handle);
	if (ret != EFI_SUCCESS || !handle) {
		efi_st_error("new_package_list returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	languages = NULL;
	languages_len = 0;
	ret = hii_string_protocol->get_secondary_languages(hii_string_protocol,
			handle, (u8 *)"en-US", languages, &languages_len);
	if (ret == EFI_NOT_FOUND) {
		efi_st_printf("no secondary languages\n");
		result = EFI_ST_SUCCESS;
		goto out;
	}
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("get_secondary_languages returned %u\n",
			     (unsigned int)ret);
		goto out;
	}
	ret = boottime->allocate_pool(EFI_LOADER_DATA, languages_len,
				      (void **)&languages);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		goto out;
	}
	ret = hii_string_protocol->get_secondary_languages(hii_string_protocol,
			handle, (u8 *)"en-US", languages, &languages_len);
	if (ret != EFI_SUCCESS) {
		efi_st_error("get_secondary_languages returned %u\n",
			     (unsigned int)ret);
		goto out;
	}

	efi_st_printf("got secondary languages are %s\n", languages);

	result = EFI_ST_SUCCESS;

out:
	if (handle) {
		ret = hii_database_protocol->remove_package_list(
				hii_database_protocol, handle);
		if (ret != EFI_SUCCESS) {
			efi_st_error("remove_package_list returned %u\n",
				     (unsigned int)ret);
			return EFI_ST_FAILURE;
		}
	}

	return result;
}

static int test_hii_string_protocol(void)
{
	int ret;

	ret = test_hii_string_new_string();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_string_get_string();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_string_set_string();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_string_get_languages();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	ret = test_hii_string_get_secondary_languages();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success, EFI_ST_FAILURE for failure
 */
static int execute(void)
{
	int ret;

	/* HII database protocol */
	ret = test_hii_database_protocol();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	/* HII string protocol */
	ret = test_hii_string_protocol();
	if (ret != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(hii) = {
	.name = "HII database protocols",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
