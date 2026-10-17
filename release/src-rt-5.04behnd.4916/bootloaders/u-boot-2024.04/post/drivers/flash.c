/*
 * Parallel NOR Flash tests
 *
 * Copyright (c) 2005-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#if CFG_POST & CFG_SYS_POST_FLASH
#include <common.h>
#include <malloc.h>
#include <post.h>
#include <flash.h>


/*
 * This code will walk over the declared sectors erasing them,
 * then programming them, then verifying the written contents.
 * Possible future work:
 *  - verify sectors before/after are not erased/written
 *  - verify partial writes (e.g. programming only middle of sector)
 *  - verify the contents of the erased sector
 *  - better seed pattern than 0x00..0xff
 */

#ifndef CFG_SYS_POST_FLASH_NUM
# define CFG_SYS_POST_FLASH_NUM 0
#endif
#if CFG_SYS_POST_FLASH_START >= CFG_SYS_POST_FLASH_END
# error "invalid flash block start/end"
#endif

static void *seed_src_data(void *ptr, ulong *old_len, ulong new_len)
{
	unsigned char *p;
	ulong i;

	p = ptr = realloc(ptr, new_len);
	if (!ptr)
		return ptr;

	for (i = *old_len; i < new_len; ++i)
		p[i] = i;

	*old_len = new_len;

	return ptr;
}

int flash_post_test(int flags)
{
	ulong len;
	void *src;
	int ret, n, n_start, n_end;
	flash_info_t *info;

	/* the output from the common flash layers needs help */
	puts("\n");

	len = 0;
	src = NULL;
	info = &flash_info[CFG_SYS_POST_FLASH_NUM];
	n_start = CFG_SYS_POST_FLASH_START;
	n_end = CFG_SYS_POST_FLASH_END;

	for (n = n_start; n < n_end; ++n) {
		ulong s_start, s_len, s_off;

		s_start = info->start[n];
		s_len = flash_sector_size(info, n);
		s_off = s_start - info->start[0];

		src = seed_src_data(src, &len, s_len);
		if (!src) {
			printf("malloc(%#lx) failed\n", s_len);
			return 1;
		}

		printf("\tsector %i: %#lx +%#lx", n, s_start, s_len);

		ret = flash_erase(info, n, n + 1);
		if (ret) {
			flash_perror(ret);
			break;
		}

		ret = write_buff(info, src, s_start, s_len);
		if (ret) {
			flash_perror(ret);
			break;
		}

		ret = memcmp(src, (void *)s_start, s_len);
		if (ret) {
			printf(" verify failed with %i\n", ret);
			break;
		}
	}

	free(src);

	return ret;
}

#endif
