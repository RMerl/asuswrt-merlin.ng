/* SPDX-License-Identifier:	LGPL-2.1 */
/*
 * Definitons for libkvcutil
 *
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _LIBKVCUTIL_H_
#define _LIBKVCUTIL_H_

#include <stdint.h>
#include <sys/types.h>

union conf_item {
	char *pair[2];
	struct {
		char *key;
		char *value;
	};
};

struct kvc_context;
struct kvc_enum_context;

/* flags for context */
#define LF_FLOCK			0x01
#define LF_STRIP_KEY_WHITESPACE		0x02
#define LF_STRIP_VALUE_WHITESPACE	0x04
#define LF_STRIP_COMMENTS		0x08
#define LF_STRIP_INVALID		0x10
#define LF_PRESERVE_LINE_ENDING		0x20
#define LF_KEY_CASE_SENSITIVE		0x40

#define LF_STRIP_WHITESPACE		\
	(LF_STRIP_KEY_WHITESPACE | LF_STRIP_VALUE_WHITESPACE)

/* line ending */
#define EOL_LF				0
#define EOL_CR				1
#define EOL_CRLF			2

/* load kvc file from path */
struct kvc_context *kvc_load_opt(const char *file, int flags,
				 const char *nostrip_list[]);
struct kvc_context *kvc_load(const char *file, int flags);

/* load from memory buffer */
struct kvc_context *kvc_load_buf_opt(const char *buf, size_t len, int flags,
				     const char *nostrip_list[]);
struct kvc_context *kvc_load_buf(const char *buf, size_t len, int flags);

/* unload kvc file. discard all uncommited changes */
int kvc_unload(struct kvc_context *ctx);

/* save all changes */
int kvc_commit(struct kvc_context *ctx);

/* return total number of valid pairs */
size_t kvc_get_count(struct kvc_context *ctx);

/* get value from a certain key */
const char *kvc_get(struct kvc_context *ctx, const char *key);

/* get a list of key-value pairs */
size_t kvc_get_all(struct kvc_context *ctx, const union conf_item **items);

/* free resources allocated by kvc_get_all */
void kvc_free_items(const union conf_item *items);

/* set value for a certain key. key will be created if not exist */
int kvc_set(struct kvc_context *ctx, const char *key, const char *value);

/* set a list of key-value pairs */
int kvc_set_batch(struct kvc_context *ctx, const union conf_item *items,
		  size_t num);

/* delete a key with its value */
int kvc_unset(struct kvc_context *ctx, const char *key);

/* start enumeration of this kvc file */
struct kvc_enum_context *kvc_enum_init(struct kvc_context *ctx);

/* return next pair of an enumeration */
int kvc_enum_next(struct kvc_enum_context *ectx, const union conf_item **item);

/* stop enumeration and free its resources */
int kvc_enum_end(struct kvc_enum_context *ectx);

/* set default line ending for new pairs */
int kvc_set_default_eol(struct kvc_context *ctx, int eol);

#endif /* _LIBKVCUTIL_H_ */
