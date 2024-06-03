// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_textinput
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Provides a unit test for the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
 * The unicode character and the scan code are printed for text
 * input. To run the test:
 *
 *	setenv efi_selftest extended text input
 *	bootefi selftest
 */

#include <efi_selftest.h>

static const efi_guid_t text_input_ex_protocol_guid =
		EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;

static struct efi_simple_text_input_ex_protocol *con_in_ex;

static struct efi_boot_services *boottime;

static void *efi_key_notify_handle;
static bool efi_running;

/**
 * efi_key_notify_function() - key notification function
 *
 * This function is called when the registered key is hit.
 *
 * @key_data:		next key
 * Return:		status code
 */
static efi_status_t EFIAPI efi_key_notify_function
				(struct efi_key_data *key_data)
{
	efi_running = false;

	return EFI_SUCCESS;
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
	efi_status_t ret;
	struct efi_key_data key_data = {
		.key = {
			.scan_code = 0,
			.unicode_char = 0x18
		},
		.key_state = {
			.key_shift_state = EFI_SHIFT_STATE_VALID |
					   EFI_LEFT_CONTROL_PRESSED,
			.key_toggle_state = EFI_TOGGLE_STATE_INVALID,
		},
	};

	boottime = systable->boottime;

	ret = boottime->locate_protocol(&text_input_ex_protocol_guid, NULL,
					(void **)&con_in_ex);
	if (ret != EFI_SUCCESS) {
		con_in_ex = NULL;
		efi_st_error
			("Extended text input protocol is not available.\n");
		return EFI_ST_FAILURE;
	}

	ret = con_in_ex->register_key_notify(con_in_ex, &key_data,
					     efi_key_notify_function,
					     &efi_key_notify_handle);
	if (ret != EFI_SUCCESS) {
		efi_key_notify_handle = NULL;
		efi_st_error
			("Notify function could not be registered.\n");
		return EFI_ST_FAILURE;
	}
	efi_running = true;

	return EFI_ST_SUCCESS;
}

/*
 * Tear down unit test.
 *
 * Unregister notify function.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	efi_status_t ret;

	ret = con_in_ex->unregister_key_notify
			(con_in_ex, efi_key_notify_handle);
	if (ret != EFI_SUCCESS) {
		efi_st_error
			("Notify function could not be registered.\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}
/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct efi_key_data input_key = { {0, 0}, {0, 0} };
	efi_status_t ret;
	efi_uintn_t index;

	if (!con_in_ex) {
		efi_st_printf("Setup failed\n");
		return EFI_ST_FAILURE;
	}

	/* Drain the console input */
	ret = con_in_ex->reset(con_in_ex, true);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Reset failed\n");
		return EFI_ST_FAILURE;
	}
	ret = con_in_ex->read_key_stroke_ex(con_in_ex, &input_key);
	if (ret != EFI_NOT_READY) {
		efi_st_error("Empty buffer not reported\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Waiting for your input\n");
	efi_st_printf("To terminate type 'CTRL+x'\n");

	while (efi_running) {
		/* Wait for next key */
		ret = boottime->wait_for_event(1, &con_in_ex->wait_for_key_ex,
					       &index);
		if (ret != EFI_ST_SUCCESS) {
			efi_st_error("WaitForEvent failed\n");
			return EFI_ST_FAILURE;
		}
		ret = con_in_ex->read_key_stroke_ex(con_in_ex, &input_key);
		if (ret != EFI_SUCCESS) {
			efi_st_error("ReadKeyStroke failed\n");
			return EFI_ST_FAILURE;
		}

		/* Allow 5 minutes until time out */
		boottime->set_watchdog_timer(300, 0, 0, NULL);

		efi_st_printf("Unicode char %u (%ps), scan code %u (",
			      (unsigned int)input_key.key.unicode_char,
			      efi_st_translate_char(input_key.key.unicode_char),
			      (unsigned int)input_key.key.scan_code);
		if (input_key.key_state.key_shift_state &
		    EFI_SHIFT_STATE_VALID) {
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_SHIFT_PRESSED | EFI_RIGHT_SHIFT_PRESSED))
				efi_st_printf("SHIFT+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_ALT_PRESSED | EFI_RIGHT_ALT_PRESSED))
				efi_st_printf("ALT+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_CONTROL_PRESSED |
			     EFI_RIGHT_CONTROL_PRESSED))
				efi_st_printf("CTRL+");
			if (input_key.key_state.key_shift_state &
			    (EFI_LEFT_LOGO_PRESSED | EFI_RIGHT_LOGO_PRESSED))
				efi_st_printf("META+");
			if (input_key.key_state.key_shift_state ==
			    EFI_SHIFT_STATE_VALID)
				efi_st_printf("+");
		}

		efi_st_printf("%ps)\n",
			      efi_st_translate_code(input_key.key.scan_code));

	}
	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(textinputex) = {
	.name = "extended text input",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
	.on_request = true,
};
