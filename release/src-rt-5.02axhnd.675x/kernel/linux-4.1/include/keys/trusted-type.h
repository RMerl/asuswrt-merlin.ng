/*
 * Copyright (C) 2010 IBM Corporation
 * Author: David Safford <safford@us.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 */

#ifndef _KEYS_TRUSTED_TYPE_H
#define _KEYS_TRUSTED_TYPE_H

#include <linux/key.h>
#include <linux/rcupdate.h>

#define MIN_KEY_SIZE			32
#define MAX_KEY_SIZE			128
#define MAX_BLOB_SIZE			320

struct trusted_key_payload {
	struct rcu_head rcu;
	unsigned int key_len;
	unsigned int blob_len;
	unsigned char migratable;
	unsigned char key[MAX_KEY_SIZE + 1];
	unsigned char blob[MAX_BLOB_SIZE];
};

extern struct key_type key_type_trusted;

#endif /* _KEYS_TRUSTED_TYPE_H */
