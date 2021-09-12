/*
 * Stubs for NVRAM functions for platforms without flash
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: nvramstubs.c 599296 2015-11-13 06:36:13Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#undef strcmp
#define strcmp(s1,s2)	0	/* always match */
#include <bcmnvram.h>

int
nvram_init(void *sih)
{
	BCM_REFERENCE(sih);

	return 0;
}

#if defined(_CFE_) && defined(BCM_DEVINFO)
int
devinfo_nvram_init(void *sih)
{
	BCM_REFERENCE(sih);

	return 0;
}
#endif // endif

int
nvram_append(void *sb, char *vars, uint varsz)
{
	BCM_REFERENCE(sb);
	BCM_REFERENCE(vars);
	BCM_REFERENCE(varsz);

	return 0;
}

void
nvram_exit(void *sih)
{
	BCM_REFERENCE(sih);
}

char *
nvram_get(const char *name)
{
	BCM_REFERENCE(name);

	return (char *) 0;
}

int
nvram_set(const char *name, const char *value)
{
	BCM_REFERENCE(name);
	BCM_REFERENCE(value);
	return 0;
}

int
nvram_unset(const char *name)
{
	BCM_REFERENCE(name);

	return 0;
}

int
nvram_commit(void)
{
	return 0;
}

int
nvram_getall(char *buf, int count)
{
	/* add null string as terminator */
	if (count < 1)
		return BCME_BUFTOOSHORT;
	*buf = '\0';
	return 0;
}
