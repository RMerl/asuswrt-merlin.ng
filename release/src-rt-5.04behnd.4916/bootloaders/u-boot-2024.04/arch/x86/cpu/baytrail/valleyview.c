// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <mmc.h>
#include <pci_ids.h>
#include <asm/irq.h>
#include <asm/mrccache.h>
#include <asm/post.h>
#include <asm/arch/iomap.h>
#include <linux/bitops.h>

/* GPIO SUS */
#define GPIO_SUS_PAD_BASE	(IO_BASE_ADDRESS + IO_BASE_OFFSET_GPSSUS)
#define GPIO_SUS_DFX5_CONF0	0x150
#define BYT_TRIG_LVL		BIT(24)
#define BYT_TRIG_POS		BIT(25)

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

int arch_misc_init(void)
{
	if (!ll_boot_init())
		return 0;

#ifdef CONFIG_ENABLE_MRC_CACHE
	/*
	 * We intend not to check any return value here, as even MRC cache
	 * is not saved successfully, it is not a severe error that will
	 * prevent system from continuing to boot.
	 */
	mrccache_save();
#endif

	/*
	 * For some unknown reason, FSP (gold4) for BayTrail configures
	 * the GPIO DFX5 PAD to enable level interrupt (bit 24 and 25).
	 * This does not cause any issue when Linux kernel runs w/ or w/o
	 * the pinctrl driver for BayTrail. However this causes unstable
	 * S3 resume if the pinctrl driver is included in the kernel build.
	 * As this pin keeps generating interrupts during an S3 resume,
	 * and there is no IRQ requester in the kernel to handle it, the
	 * kernel seems to hang and does not continue resuming.
	 *
	 * Clear the mysterious interrupt bits for this pin.
	 */
	clrbits_le32(GPIO_SUS_PAD_BASE + GPIO_SUS_DFX5_CONF0,
		     BYT_TRIG_LVL | BYT_TRIG_POS);

	return 0;
}
