/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  EFI application loader
 *
 *  Copyright (c) 2017 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ifndef _EFI_SELFTEST_H
#define _EFI_SELFTEST_H

#include <efi.h>
#include <efi_api.h>
#include <efi_loader.h>
#include <linker_lists.h>

#define EFI_ST_SUCCESS 0
#define EFI_ST_FAILURE 1
#define EFI_ST_SUCCESS_STR u"SUCCESS"

extern const struct efi_system_table *st_systable;
extern const struct efi_boot_services *st_boottime;

/**
 * efi_st_printf() - print a message
 *
 * @...:	format string followed by fields to print
 */
#define efi_st_printf(...) \
	(efi_st_printc(-1, __VA_ARGS__))

/**
 * efi_st_error() - prints an error message
 *
 * @...:	format string followed by fields to print
 */
#define efi_st_error(...) \
	(efi_st_printc(EFI_LIGHTRED, "%s(%u):\nERROR: ", __FILE__, __LINE__), \
	efi_st_printc(EFI_LIGHTRED, __VA_ARGS__))

/**
 * efi_st_todo() - prints a TODO message
 *
 * @...:	format string followed by fields to print
 */
#define efi_st_todo(...) \
	(efi_st_printc(EFI_YELLOW, "%s(%u):\nTODO: ", __FILE__, __LINE__), \
	efi_st_printc(EFI_YELLOW, __VA_ARGS__)) \

/**
 * enum efi_test_phase - phase when test will be executed
 *
 * A test may be setup and executed at boottime,
 * it may be setup at boottime and executed at runtime,
 * or it may be setup and executed at runtime.
 */
enum efi_test_phase {
	/**
	 * @EFI_EXECUTE_BEFORE_BOOTTIME_EXIT:
	 *
	 * Setup, execute, and teardown are executed before ExitBootServices().
	 */
	EFI_EXECUTE_BEFORE_BOOTTIME_EXIT = 1,
	/**
	 * @EFI_SETUP_BEFORE_BOOTTIME_EXIT:
	 *
	 * Setup is executed before ExitBootServices() while execute, and
	 * teardown are executed after ExitBootServices().
	 */
	EFI_SETUP_BEFORE_BOOTTIME_EXIT,
	/**
	 * @EFI_SETTING_VIRTUAL_ADDRESS_MAP:
	 *
	 * Execute calls SetVirtualAddressMap(). Setup is executed before
	 * ExitBootServices() while execute is executed after
	 * ExitBootServices(), and after the execute of tests marked as
	 * @EFI_SETUP_BEFORE_BOOTTIME_EXIT. Teardown is executed thereafter.
	 */
	EFI_SETTING_VIRTUAL_ADDRESS_MAP,
};

extern struct efi_simple_text_output_protocol *con_out;
extern struct efi_simple_text_input_protocol *con_in;

/**
 * efi_st_exit_boot_services() - exit the boot services
 *
 * * The size of the memory map is determined.
 * * Pool memory is allocated to copy the memory map.
 * * The memory map is copied and the map key is obtained.
 * * The map key is used to exit the boot services.
 */
void efi_st_exit_boot_services(void);

/**
 * efi_st_printc() - print a colored message
 *
 * @color:	color, see constants in efi_api.h, use -1 for no color
 * @fmt:	printf style format string
 * @...:	arguments to be printed
 */
void efi_st_printc(int color, const char *fmt, ...)
		 __attribute__ ((format (__printf__, 2, 3)));

/**
 * efi_st_translate_char() - translate a Unicode character to a string
 *
 * @code:	Unicode character
 * Return:	string
 */
u16 *efi_st_translate_char(u16 code);

/**
 * efi_st_translate_code() - translate a scan code to a human readable string
 *
 * This function translates the scan code returned by the simple text input
 * protocol to a human readable string, e.g. 0x04 is translated to u"Left".
 *
 * @code:	scan code
 * Return:	Unicode string
 */
u16 *efi_st_translate_code(u16 code);

/**
 * efi_st_strcmp_16_8() - compare an u16 string to a char string
 *
 * This function compares each u16 value to the char value at the same
 * position. This function is only useful for ANSI strings.
 *
 * @buf1:	u16 string
 * @buf2:	char string
 * Return:	0 if both buffers contain equivalent strings
 */
int efi_st_strcmp_16_8(const u16 *buf1, const unsigned char *buf2);

/**
 * efi_st_get_config_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
void *efi_st_get_config_table(const efi_guid_t *guid);

/**
 * efi_st_get_key() - reads an Unicode character from the input device
 *
 * Return:	Unicode character
 */
u16 efi_st_get_key(void);

/**
 * struct efi_unit_test - EFI unit test
 *
 * The &struct efi_unit_test structure provides a interface to an EFI unit test.
 *
 * @name:	name of the unit test used in the user interface
 * @phase:	specifies when setup and execute are executed
 * @setup:	set up function of the unit test
 * @execute:	execute function of the unit test
 * @teardown:	tear down function of the unit test
 * @on_request:	flag indicating that the test shall only be executed on request
 */
struct efi_unit_test {
	const char *name;
	const enum efi_test_phase phase;
	int (*setup)(const efi_handle_t handle,
		     const struct efi_system_table *systable);
	int (*execute)(void);
	int (*teardown)(void);
	bool on_request;
};

/**
 * EFI_UNIT_TEST() - macro to declare a new EFI unit test
 *
 * The macro EFI_UNIT_TEST() declares an EFI unit test using the &struct
 * efi_unit_test structure. The test is added to a linker generated list which
 * is evaluated by the 'bootefi selftest' command.
 *
 * @__name:	string identifying the unit test in the linker generated list
 */
#define EFI_UNIT_TEST(__name)						\
	ll_entry_declare(struct efi_unit_test, __name, efi_unit_test)

#endif /* _EFI_SELFTEST_H */
