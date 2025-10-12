/*
 * The wlcsm kernel module
 *
 * Copyright (C) 2024, Broadcom. All Rights Reserved.
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
 * <<Broadcom-WL-IPTag/Open:>
 *
 * $Id: wlcsm_nvram.h 832801 2023-11-13 20:14:38Z $
 */

#ifndef __WLCSM_NVRAM_H_
#define __WLCSM_NVRAM_H_

#include <wlcsm_linux.h>
#include <linux/list.h>
typedef struct {
	unsigned int pid;
	char name[256];
	struct list_head list;
} PROCESS_REG_LIST;

typedef struct wlcsm_nvram_tuple {
    struct rb_node node;
    t_WLCSM_NAME_VALUEPAIR *tuple;
} t_WLCSM_NVRAM_TUPLE;

int wlcsm_nvram_set(char *buf, int len);
void wlcsm_nvram_unset (char *name );
int wlcsm_nvram_get(char *name,char *result);

extern char * wlcsm_nvram_k_get(char *name);
extern int wlcsm_nvram_k_set(char *name, char *value);
extern int wlcsm_nvram_getall(char *buf, int count, int pos);

extern int wlcsm_nvram_setcount_get(void);
extern void wlcsm_nvram_setcount_clear(void);
#endif // endif
