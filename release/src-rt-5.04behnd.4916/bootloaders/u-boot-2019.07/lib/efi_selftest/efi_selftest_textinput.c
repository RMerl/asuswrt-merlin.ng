// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_textinput
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Provides a unit test for the EFI_SIMPLE_TEXT_INPUT_PROTOCOL.
 * The Unicode character and the scan code are printed for text
 * input. To run the test:
 *
 *	setenv efi_selftest text input
 *	bootefi selftest
 */

#include <efi_selftest.h>

static struct efi_boot_services *boottime;

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
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct efi_input_key input_key = {0};
	efi_status_t ret;
	efi_uintn_t index;

	/* Drain the console input */
	ret = con_in->reset(con_in, true);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Reset failed\n");
		return EFI_ST_FAILURE;
	}
	ret = con_in->read_key_stroke(con_in, &input_key);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Empty buffer not reported\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Waiting for your input\n");
	efi_st_printf("To terminate type 'x'\n");

	for (;;) {
		/* Wait for next key */
		ret = boottime->wait_for_event(1, &con_in->wait_for_key,
					       &index);
		if (ret != EFI_ST_SUCCESS) {
			efi_st_error("WaitForEvent failed\n");
			return EFI_ST_FAILURE;
		}
		ret = con_in->read_key_stroke(con_in, &input_key);
		if (ret != EFI_SUCCESS) {
			efi_st_error("ReadKeyStroke failed\n");
			return EFI_ST_FAILURE;
		}

		/* Allow 5 minutes until time out */
		boottime->set_watchdog_timer(300, 0, 0, NULL);

		efi_st_printf("Unicode char %u (%ps), scan code %u (%ps)\n",
			      (unsigned int)input_key.unicode_char,
			      efi_st_translate_char(input_key.unicode_char),
			      (unsigned int)input_key.scan_code,
			      efi_st_translate_code(input_key.scan_code));

		switch (input_key.unicode_char) {
		case 'x':
		case 'X':
			return EFI_ST_SUCCESS;
		}
	}
}

EFI_UNIT_TEST(textinput) = {
	.name = "text input",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};
