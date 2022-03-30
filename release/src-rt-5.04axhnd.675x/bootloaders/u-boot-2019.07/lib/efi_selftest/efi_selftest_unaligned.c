// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_unaligned
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test unaligned memory access on ARMv7.
 */

#include <efi_selftest.h>

struct aligned_buffer {
	char a[8] __aligned(8);
};

/*
 * Return an u32 at a give address.
 * If the address is not four byte aligned, an unaligned memory access
 * occurs.
 *
 * @addr:	address to read
 * @return:	value at the address
 */
static inline u32 deref(u32 *addr)
{
	int ret;

	asm(
		"ldr %[out], [%[in]]\n\t"
		: [out] "=r" (ret)
		: [in] "r" (addr)
	);
	return ret;
}

/*
 * Execute unit test.
 * An unaligned memory access is executed. The result is checked.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	struct aligned_buffer buf = {
		{0, 1, 2, 3, 4, 5, 6, 7},
		};
	void *v = &buf;
	u32 r = 0;

	/* Read an unaligned address */
	r = deref(v + 1);

	/* UEFI only supports low endian systems */
	if (r != 0x04030201) {
		efi_st_error("Unaligned access failed");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(unaligned) = {
	.name = "unaligned memory access",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.execute = execute,
};
