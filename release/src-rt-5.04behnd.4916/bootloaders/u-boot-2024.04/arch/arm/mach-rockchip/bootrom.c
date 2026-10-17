// SPDX-License-Identifier: GPL-2.0+
/**
 * Copyright (c) 2017 Google, Inc
 */

#include <common.h>
#include <hang.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/boot_mode.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/setjmp.h>
#include <asm/system.h>

/*
 * Force the jmp_buf to the data-section, as .bss will not be valid
 * when save_boot_params is invoked.
 */
static jmp_buf brom_ctx __section(".data");

static void _back_to_bootrom(enum rockchip_bootrom_cmd brom_cmd)
{
	longjmp(brom_ctx, brom_cmd);
}

void back_to_bootrom(enum rockchip_bootrom_cmd brom_cmd)
{
#if CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT)
	puts("Returning to boot ROM...\n");
#endif
	_back_to_bootrom(brom_cmd);
}

/*
 * we back to bootrom download mode if get a
 * BOOT_BROM_DOWNLOAD flag in boot mode register
 *
 * note: the boot mode register is configured by
 * application(next stage bootloader, kernel, etc),
 * and the bootrom never check this register, so we need
 * to check it and back to bootrom at very early bootstage(before
 * some basic configurations(such as interrupts) been
 * changed by TPL/SPL, as the bootrom download operation
 * relies on many default settings(such as interrupts) by
 * itself.
 */
static bool check_back_to_brom_dnl_flag(void)
{
	u32 boot_mode;

	if (CONFIG_ROCKCHIP_BOOT_MODE_REG) {
		boot_mode = readl(CONFIG_ROCKCHIP_BOOT_MODE_REG);
		if (boot_mode == BOOT_BROM_DOWNLOAD) {
			writel(0, CONFIG_ROCKCHIP_BOOT_MODE_REG);
			return true;
		}
	}

	return false;
}

/*
 * All Rockchip BROM implementations enter with a valid stack-pointer,
 * so this can safely be implemented in C (providing a single
 * implementation both for ARMv7 and AArch64).
 */
int save_boot_params(void)
{
	int  ret = setjmp(brom_ctx);

	switch (ret) {
	case 0:
		if (check_back_to_brom_dnl_flag())
			_back_to_bootrom(BROM_BOOT_ENTER_DNL);
		/*
		 * This is the initial pass through this function
		 * (i.e. saving the context), setjmp just setup up the
		 * brom_ctx: transfer back into the startup-code at
		 * 'save_boot_params_ret' and let the compiler know
		 * that this will not return.
		 */
		save_boot_params_ret();
		while (true)
			/* does not return */;
		break;

	case BROM_BOOT_NEXTSTAGE:
		/*
		 * To instruct the BROM to boot the next stage, we
		 * need to return 0 to it: i.e. we need to rewrite
		 * the return code once more.
		 */
		ret = 0;
		break;
	case BROM_BOOT_ENTER_DNL:
		/*
		 * A non-zero return value will instruct the BROM enter
		 * download mode.
		 */
		ret = 1;
		break;
	default:
#if CONFIG_IS_ENABLED(LIBCOMMON_SUPPORT)
		puts("FATAL: unexpected command to back_to_bootrom()\n");
#endif
		hang();
	};

	return ret;
}
