// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <efi_loader.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define UT_REG_CAPACITY 6

static int lib_test_efi_image_region_add(struct unit_test_state *uts)
{
	struct efi_image_regions *regs;

	regs = calloc(sizeof(*regs) +
		      sizeof(struct image_region) * UT_REG_CAPACITY, 1);
	ut_assert(regs);

	regs->max = UT_REG_CAPACITY;

	ut_asserteq(0, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x4000,
					    (void *)0x3000, 1));
	ut_asserteq(0, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x3100,
					    (void *)0x4000, 1));
	ut_asserteq(1, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x2000,
					    (void *)0x3100, 1));
	ut_asserteq(2, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x1000,
					    (void *)0x1f00, 1));
	ut_asserteq(3, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x4000,
					    (void *)0x4e00, 1));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x1f00,
					    (void *)0x2001, 1));
	ut_asserteq(5, regs->num);

	ut_asserteq_ptr((void *)0x3100, regs->reg[0].data);
	ut_asserteq(0x0f00, regs->reg[0].size);

	ut_asserteq_ptr((void *)0x2000, regs->reg[1].data);
	ut_asserteq(0x1100, regs->reg[1].size);

	ut_asserteq_ptr((void *)0x1000, regs->reg[2].data);
	ut_asserteq(0x0f00, regs->reg[2].size);

	ut_asserteq_ptr((void *)0x4000, regs->reg[3].data);
	ut_asserteq(0x0e00, regs->reg[3].size);

	ut_asserteq_ptr((void *)0x1f00, regs->reg[4].data);
	ut_asserteq(0x0101, regs->reg[4].size);

	free(regs);

	return 0;
}

LIB_TEST(lib_test_efi_image_region_add, 0);

static int lib_test_efi_image_region_sort(struct unit_test_state *uts)
{
	struct efi_image_regions *regs;

	regs = calloc(sizeof(*regs) +
		      sizeof(struct image_region) * UT_REG_CAPACITY, 1);
	ut_assert(regs);

	regs->max = UT_REG_CAPACITY;

	ut_asserteq(0, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x4000,
					    (void *)0x3000, 0));
	ut_asserteq(0, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x3100,
					    (void *)0x4000, 0));
	ut_asserteq(1, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x2000,
					    (void *)0x3100, 0));
	ut_asserteq(2, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x1000,
					    (void *)0x1f00, 0));
	ut_asserteq(3, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x4000,
					    (void *)0x4e00, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x1f00,
					    (void *)0x2001, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x10ff,
					    (void *)0x11ff, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x0000,
					    (void *)0x6000, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x3100,
					    (void *)0x0e00, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x3200,
					    (void *)0x0e00, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_INVALID_PARAMETER,
		       efi_image_region_add(regs, (void *)0x3200,
					    (void *)0x0d00, 0));
	ut_asserteq(4, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x1f00,
					    (void *)0x2000, 0));
	ut_asserteq(5, regs->num);
	ut_asserteq_64(EFI_SUCCESS,
		       efi_image_region_add(regs, (void *)0x4000,
					    (void *)0x4000, 0));
	ut_asserteq(6, regs->num);
	ut_asserteq_64(EFI_OUT_OF_RESOURCES,
		       efi_image_region_add(regs, (void *)0x6000,
					    (void *)0x0100, 0));
	ut_asserteq(6, regs->num);

	ut_asserteq_ptr((void *)0x1000, regs->reg[0].data);
	ut_asserteq(0x0f00, regs->reg[0].size);

	ut_asserteq_ptr((void *)0x1f00, regs->reg[1].data);
	ut_asserteq(0x0100, regs->reg[1].size);

	ut_asserteq_ptr((void *)0x2000, regs->reg[2].data);
	ut_asserteq(0x1100, regs->reg[2].size);

	ut_asserteq_ptr((void *)0x3100, regs->reg[3].data);
	ut_asserteq(0x0f00, regs->reg[3].size);

	ut_asserteq_ptr((void *)0x4000, regs->reg[4].data);
	ut_asserteq(0x0000, regs->reg[4].size);

	ut_asserteq_ptr((void *)0x4000, regs->reg[5].data);
	ut_asserteq(0x0e00, regs->reg[5].size);

	free(regs);

	return 0;
}

LIB_TEST(lib_test_efi_image_region_sort, 0);
