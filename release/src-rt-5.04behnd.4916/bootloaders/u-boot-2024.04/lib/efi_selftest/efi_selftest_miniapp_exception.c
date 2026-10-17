// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_miniapp_return
 *
 * Copyright (c) 2019 Heinrich Schuchardt
 *
 * This EFI application triggers an exception.
 */

#include <efi_api.h>
#include <host_arch.h>

/*
 * Entry point of the EFI application.
 *
 * @handle	handle of the loaded image
 * @systable	system table
 * Return:	status code
 */
efi_status_t EFIAPI efi_main(efi_handle_t handle,
			     struct efi_system_table *systable)
{
	struct efi_simple_text_output_protocol *con_out = systable->con_out;

	con_out->output_string(con_out,
			       u"EFI application triggers exception.\n");

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
#elif defined(CONFIG_SANDBOX)
#if (HOST_ARCH == HOST_ARCH_ARM || HOST_ARCH == HOST_ARCH_AARCH64)
	asm volatile (".word 0xe7f7defb\n");
#elif (HOST_ARCH == HOST_ARCH_RISCV32 || HOST_ARCH == HOST_ARCH_RISCV64)
	asm volatile (".word 0xffffffff\n");
#elif (HOST_ARCH == HOST_ARCH_X86 || HOST_ARCH == HOST_ARCH_X86_64)
	asm volatile (".word 0xffff\n");
#endif
#endif
	con_out->output_string(con_out, u"Exception not triggered.\n");
	return EFI_ABORTED;
}
