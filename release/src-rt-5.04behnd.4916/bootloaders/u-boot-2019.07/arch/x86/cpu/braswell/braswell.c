// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <asm/mrccache.h>
#include <asm/post.h>

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	return x86_cpu_init_f();
}

int arch_misc_init(void)
{
#ifdef CONFIG_ENABLE_MRC_CACHE
	/*
	 * We intend not to check any return value here, as even MRC cache
	 * is not saved successfully, it is not a severe error that will
	 * prevent system from continuing to boot.
	 */
	mrccache_save();
#endif

	return 0;
}
