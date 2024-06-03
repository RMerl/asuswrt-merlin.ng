/* SPDX-License-Identifier:	LGPL-2.1 */
/*
 * Definitons for libdatconf
 *
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _LIBDATCONF_H_
#define _LIBDATCONF_H_

#include <libkvcutil.h>

#define DATCONF_LF_FLAGS	(LF_FLOCK | \
				 LF_KEY_CASE_SENSITIVE | \
				 LF_STRIP_WHITESPACE)

#define L1PROFILE_PATH		"/etc/wireless/l1profile.dat"

#define ENV_L1PROFILE_PATH	"L1PROFILE_PATH"

extern const char *dat_nostrip_list[];

/* get dat file path from l1profile.dat by index */
const char *get_dat_path_by_index(uint32_t index);

/* get dat file path from l1profile.dat by name */
const char *get_dat_path_by_name(const char *name);

/* free resources allocated by get_dat_path_by_* */
void free_dat_path(const char *path);


/* load dat file from path */
struct kvc_context *dat_load(const char *file);

/* load dat file by index */
struct kvc_context *dat_load_by_index(uint32_t index);

/* load dat file by name */
struct kvc_context *dat_load_by_name(const char *name);

/* load dat file by ordinal */
const char *get_dat_path_by_ord(uint32_t ord);

/* load from memory */
struct kvc_context *dat_load_raw(const char *str, size_t len);


/* extract a field seperated by ';' and specified by idx */
const char *dat_get_indexed_value(const char *str, size_t idx);

/* set a field seperated by ';' and specified by idx */
const char *dat_set_indexed_value(const char *str, size_t idx, const char *val);

/* free resources allocated by dat_{get,set}_indexed_value */
#define dat_free_value	free_dat_path

#endif /* _LIBDATCONF_H_ */
