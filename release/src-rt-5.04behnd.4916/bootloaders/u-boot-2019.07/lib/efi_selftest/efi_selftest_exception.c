// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_exception
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the handling of exceptions by trying to execute an undefined
 * instruction.
 */

#include <efi_selftest.h>

/**
 * undefined_instruction() - try to executed an undefined instruction
 */
static void undefined_instruction(void)
{
#if defined(CONFIG_ARM)
	/*
	 * 0xe7f...f.	is undefined in ARM mode
	 * 0xde..	is undefined in Thumb mode
	 */
	asm volatile (".word 0xe7f7defb\n");
#elif defined(CONFIG_RISCV)
	asm volatile (".word 0xffffffff\n");
#elif defined(CONFIG_X86)
	asm volatile (".word 0xffff\n");
#endif
}

/**
 * execute() - execute unit test
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	undefined_instruction();

	efi_st_error("An undefined instruction exception was not raised\n");

	return EFI_ST_FAILURE;
}

EFI_UNIT_TEST(exception) = {
	.name = "exception",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
	.on_request = true,
};
