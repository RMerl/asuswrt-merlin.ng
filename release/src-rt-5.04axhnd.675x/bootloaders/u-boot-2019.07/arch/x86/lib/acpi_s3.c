// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/acpi.h>
#include <asm/acpi_s3.h>
#include <asm/acpi_table.h>
#include <asm/post.h>
#include <linux/linkage.h>

DECLARE_GLOBAL_DATA_PTR;

static void asmlinkage (*acpi_do_wakeup)(void *vector) = (void *)WAKEUP_BASE;

static void acpi_jump_to_wakeup(void *vector)
{
	/* Copy wakeup trampoline in place */
	memcpy((void *)WAKEUP_BASE, __wakeup, __wakeup_size);

	printf("Jumping to OS waking vector %p\n", vector);
	acpi_do_wakeup(vector);
}

void acpi_resume(struct acpi_fadt *fadt)
{
	void *wake_vec;

	/* Turn on ACPI mode for S3 */
	enter_acpi_mode(fadt->pm1a_cnt_blk);

	wake_vec = acpi_find_wakeup_vector(fadt);

	/*
	 * Restore the memory content starting from address 0x1000 which is
	 * used for the real mode interrupt handler stubs.
	 */
	memcpy((void *)0x1000, (const void *)gd->arch.backup_mem,
	       S3_RESERVE_SIZE);

	post_code(POST_OS_RESUME);
	acpi_jump_to_wakeup(wake_vec);
}

int acpi_s3_reserve(void)
{
	/* adjust stack pointer for ACPI S3 resume backup memory */
	gd->start_addr_sp -= S3_RESERVE_SIZE;
	gd->arch.backup_mem = gd->start_addr_sp;

	gd->start_addr_sp &= ~0xf;

	/*
	 * U-Boot sets up the real mode interrupt handler stubs starting from
	 * address 0x1000. In most cases, the first 640K (0x00000 - 0x9ffff)
	 * system memory is reported as system RAM in E820 table to the OS.
	 * (see install_e820_map() implementation for each platform). So OS
	 * can use these memories whatever it wants.
	 *
	 * If U-Boot is in an S3 resume path, care must be taken not to corrupt
	 * these memorie otherwise OS data gets lost. Testing shows that, on
	 * Microsoft Windows 10 on Intel Baytrail its wake up vector happens to
	 * be installed at the same address 0x1000. While on Linux its wake up
	 * vector does not overlap this memory range, but after resume kernel
	 * checks low memory range per config option CONFIG_X86_RESERVE_LOW
	 * which is 64K by default to see whether a memory corruption occurs
	 * during the suspend/resume (it's harmless, but warnings are shown
	 * in the kernel dmesg logs).
	 *
	 * We cannot simply mark the these memory as reserved in E820 table
	 * because such configuration makes GRUB complain: unable to allocate
	 * real mode page. Hence we choose to back up these memories to the
	 * place where we reserved on our stack for our S3 resume work.
	 * Before jumping to OS wake up vector, we need restore the original
	 * content there (see acpi_resume() above).
	 */
	if (gd->arch.prev_sleep_state == ACPI_S3)
		memcpy((void *)gd->arch.backup_mem, (const void *)0x1000,
		       S3_RESERVE_SIZE);

	return 0;
}
