// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_config_tables
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * This test checks the following service:
 * InstallConfigurationTable.
 */

#include <efi_selftest.h>

static const struct efi_system_table *sys_table;
static struct efi_boot_services *boottime;

static efi_guid_t table_guid =
	EFI_GUID(0xff1c3f9e, 0x795b, 0x1529, 0xf1, 0x55,
		 0x17, 0x2e, 0x51, 0x6b, 0x49, 0x75);

/*
 * Notification function, increments the notification count if parameter
 * context is provided.
 *
 * @event	notified event
 * @context	pointer to the notification count
 */
static void EFIAPI notify(struct efi_event *event, void *context)
{
	unsigned int *count = context;

	if (count)
		++*count;
}

/*
 * Check CRC32 of a table.
 */
static int check_table(const void *table)
{
	efi_status_t ret;
	u32 crc32, res;
	/* Casting from constant to not constant */
	struct efi_table_hdr *hdr = (struct efi_table_hdr *)table;

	crc32 = hdr->crc32;
	/*
	 * Setting the CRC32 of the 'const' table to zero is easier than
	 * copying
	 */
	hdr->crc32 = 0;
	ret = boottime->calculate_crc32(table, hdr->headersize, &res);
	/* Reset table CRC32 so it stays constant */
	hdr->crc32 = crc32;
	if (ret != EFI_ST_SUCCESS) {
		efi_st_error("CalculateCrc32 failed\n");
		return EFI_ST_FAILURE;
	}
	if (res != crc32) {
		efi_st_error("Incorrect CRC32\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	sys_table = systable;
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * A table is installed, updated, removed. The table entry and the
 * triggering of events is checked.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	unsigned int counter = 0;
	struct efi_event *event;
	void *table;
	const unsigned int tables[2];
	efi_uintn_t i;
	efi_uintn_t tabcnt;
	efi_uintn_t table_count = sys_table->nr_tables;

	ret = boottime->create_event_ex(0, TPL_NOTIFY,
					notify, (void *)&counter,
					&table_guid, &event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to create event\n");
		return EFI_ST_FAILURE;
	}

	/* Try to delete non-existent table */
	ret = boottime->install_configuration_table(&table_guid, NULL);
	if (ret != EFI_NOT_FOUND) {
		efi_st_error("Failed to detect missing table\n");
		return EFI_ST_FAILURE;
	}
	if (counter) {
		efi_st_error("Notification function was called.\n");
		return EFI_ST_FAILURE;
	}
	/* Check if the event was signaled  */
	ret = boottime->check_event(event);
	if (ret == EFI_SUCCESS) {
		efi_st_error("Event was signaled on EFI_NOT_FOUND\n");
		return EFI_ST_FAILURE;
	}
	if (counter != 1) {
		efi_st_error("Notification function was not called.\n");
		return EFI_ST_FAILURE;
	}
	if (table_count != sys_table->nr_tables) {
		efi_st_error("Incorrect table count %u, expected %u\n",
			     (unsigned int)sys_table->nr_tables,
			     (unsigned int)table_count);
		return EFI_ST_FAILURE;
	}

	/* Install table */
	ret = boottime->install_configuration_table(&table_guid,
						    (void *)&tables[0]);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to install table\n");
		return EFI_ST_FAILURE;
	}
	/* Check signaled state */
	ret = boottime->check_event(event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Event was not signaled on insert\n");
		return EFI_ST_FAILURE;
	}
	if (++table_count != sys_table->nr_tables) {
		efi_st_error("Incorrect table count %u, expected %u\n",
			     (unsigned int)sys_table->nr_tables,
			     (unsigned int)table_count);
		return EFI_ST_FAILURE;
	}
	table = NULL;
	for (i = 0; i < sys_table->nr_tables; ++i) {
		if (!memcmp(&sys_table->tables[i].guid, &table_guid,
			    sizeof(efi_guid_t)))
			table = sys_table->tables[i].table;
	}
	if (!table) {
		efi_st_error("Installed table not found\n");
		return EFI_ST_FAILURE;
	}
	if (table != &tables[0]) {
		efi_st_error("Incorrect table address\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(sys_table) != EFI_ST_SUCCESS) {
		efi_st_error("Checking system table\n");
		return EFI_ST_FAILURE;
	}

	/* Update table */
	ret = boottime->install_configuration_table(&table_guid,
						    (void *)&tables[1]);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to update table\n");
		return EFI_ST_FAILURE;
	}
	/* Check signaled state */
	ret = boottime->check_event(event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Event was not signaled on update\n");
		return EFI_ST_FAILURE;
	}
	if (table_count != sys_table->nr_tables) {
		efi_st_error("Incorrect table count %u, expected %u\n",
			     (unsigned int)sys_table->nr_tables,
			     (unsigned int)table_count);
		return EFI_ST_FAILURE;
	}
	table = NULL;
	tabcnt = 0;
	for (i = 0; i < sys_table->nr_tables; ++i) {
		if (!memcmp(&sys_table->tables[i].guid, &table_guid,
			    sizeof(efi_guid_t))) {
			table = sys_table->tables[i].table;
			++tabcnt;
		}
	}
	if (!table) {
		efi_st_error("Installed table not found\n");
		return EFI_ST_FAILURE;
	}
	if (tabcnt > 1) {
		efi_st_error("Duplicate table GUID\n");
		return EFI_ST_FAILURE;
	}
	if (table != &tables[1]) {
		efi_st_error("Incorrect table address\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(sys_table) != EFI_ST_SUCCESS) {
		efi_st_error("Checking system table\n");
		return EFI_ST_FAILURE;
	}

	/* Delete table */
	ret = boottime->install_configuration_table(&table_guid, NULL);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to delete table\n");
		return EFI_ST_FAILURE;
	}
	/* Check signaled state */
	ret = boottime->check_event(event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Event was not signaled on delete\n");
		return EFI_ST_FAILURE;
	}
	if (--table_count != sys_table->nr_tables) {
		efi_st_error("Incorrect table count %u, expected %u\n",
			     (unsigned int)sys_table->nr_tables,
			     (unsigned int)table_count);
		return EFI_ST_FAILURE;
	}
	table = NULL;
	for (i = 0; i < sys_table->nr_tables; ++i) {
		if (!memcmp(&sys_table->tables[i].guid, &table_guid,
			    sizeof(efi_guid_t))) {
			table = sys_table->tables[i].table;
		}
	}
	if (table) {
		efi_st_error("Wrong table deleted\n");
		return EFI_ST_FAILURE;
	}

	ret = boottime->close_event(event);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Failed to close event\n");
		return EFI_ST_FAILURE;
	}
	if (check_table(sys_table) != EFI_ST_SUCCESS) {
		efi_st_error("Checking system table\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(configtables) = {
	.name = "configuration tables",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
