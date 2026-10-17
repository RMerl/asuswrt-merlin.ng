// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019, Simon Goldschmidt <simon.k.r.goldschmidt@gmail.com>
 *
 * This tool helps to return the size available for SPL image during build
 */

#include <generated/autoconf.h>
#include <generated/generic-asm-offsets.h>

int main(int argc, char *argv[])
{
	int spl_size_limit = 0;

#ifdef CONFIG_SPL_SIZE_LIMIT
	spl_size_limit = CONFIG_SPL_SIZE_LIMIT;
#if defined(CONFIG_IMX_HAB) && defined(CONFIG_CSF_SIZE)
	spl_size_limit -= CONFIG_CSF_SIZE;
#endif
#ifdef CONFIG_SPL_SIZE_LIMIT_SUBTRACT_GD
	spl_size_limit -= GENERATED_GBL_DATA_SIZE;
#endif
#ifdef CONFIG_SPL_SIZE_LIMIT_SUBTRACT_MALLOC
	spl_size_limit -= CONFIG_SPL_SYS_MALLOC_F_LEN;
#endif
#ifdef CONFIG_SPL_SIZE_LIMIT_PROVIDE_STACK
	spl_size_limit -= CONFIG_SPL_SIZE_LIMIT_PROVIDE_STACK;
#endif
#endif

	printf("%d", spl_size_limit);
	return 0;
}
