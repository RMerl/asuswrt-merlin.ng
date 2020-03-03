/* user_defined.c: user defined key type
 *
 * Copyright (C) 2004 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <keys/user-type.h>
#include <asm/uaccess.h>
#include "internal.h"

static int logon_vet_description(const char *desc);

/*
 * user defined keys take an arbitrary string as the description and an
 * arbitrary blob of data as the payload
 */
struct key_type key_type_user = {
	.name			= "user",
	.preparse		= user_preparse,
	.free_preparse		= user_free_preparse,
	.instantiate		= generic_key_instantiate,
	.update			= user_update,
	.revoke			= user_revoke,
	.destroy		= user_destroy,
	.describe		= user_describe,
	.read			= user_read,
};

EXPORT_SYMBOL_GPL(key_type_user);

/*
 * This key type is essentially the same as key_type_user, but it does
 * not define a .read op. This is suitable for storing username and
 * password pairs in the keyring that you do not want to be readable
 * from userspace.
 */
struct key_type key_type_logon = {
	.name			= "logon",
	.preparse		= user_preparse,
	.free_preparse		= user_free_preparse,
	.instantiate		= generic_key_instantiate,
	.update			= user_update,
	.revoke			= user_revoke,
	.destroy		= user_destroy,
	.describe		= user_describe,
	.vet_description	= logon_vet_description,
};
EXPORT_SYMBOL_GPL(key_type_logon);

/*
 * Preparse a user defined key payload
 */
int user_preparse(struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload;
	size_t datalen = prep->datalen;

	if (datalen <= 0 || datalen > 32767 || !prep->data)
		return -EINVAL;

	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		return -ENOMEM;

	/* attach the data */
	prep->quotalen = datalen;
	prep->payload[0] = upayload;
	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);
	return 0;
}
EXPORT_SYMBOL_GPL(user_preparse);

/*
 * Free a preparse of a user defined key payload
 */
void user_free_preparse(struct key_preparsed_payload *prep)
{
	kfree(prep->payload[0]);
}
EXPORT_SYMBOL_GPL(user_free_preparse);

/*
 * update a user defined key
 * - the key's semaphore is write-locked
 */
int user_update(struct key *key, struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload, *zap;
	size_t datalen = prep->datalen;
	int ret;

	ret = -EINVAL;
	if (datalen <= 0 || datalen > 32767 || !prep->data)
		goto error;

	/* construct a replacement payload */
	ret = -ENOMEM;
	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		goto error;

	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);

	/* check the quota and attach the new data */
	zap = upayload;

	ret = key_payload_reserve(key, datalen);

	if (ret == 0) {
		/* attach the new data, displacing the old */
		if (!test_bit(KEY_FLAG_NEGATIVE, &key->flags))
			zap = key->payload.data;
		else
			zap = NULL;
		rcu_assign_keypointer(key, upayload);
		key->expiry = 0;
	}

	if (zap)
		kfree_rcu(zap, rcu);

error:
	return ret;
}

EXPORT_SYMBOL_GPL(user_update);

/*
 * dispose of the links from a revoked keyring
 * - called with the key sem write-locked
 */
void user_revoke(struct key *key)
{
	struct user_key_payload *upayload = key->payload.data;

	/* clear the quota */
	key_payload_reserve(key, 0);

	if (upayload) {
		rcu_assign_keypointer(key, NULL);
		kfree_rcu(upayload, rcu);
	}
}

EXPORT_SYMBOL(user_revoke);

/*
 * dispose of the data dangling from the corpse of a user key
 */
void user_destroy(struct key *key)
{
	struct user_key_payload *upayload = key->payload.data;

	kfree(upayload);
}

EXPORT_SYMBOL_GPL(user_destroy);

/*
 * describe the user key
 */
void user_describe(const struct key *key, struct seq_file *m)
{
	seq_puts(m, key->description);
	if (key_is_instantiated(key))
		seq_printf(m, ": %u", key->datalen);
}

EXPORT_SYMBOL_GPL(user_describe);

/*
 * read the key data
 * - the key's semaphore is read-locked
 */
long user_read(const struct key *key, char __user *buffer, size_t buflen)
{
	struct user_key_payload *upayload;
	long ret;

	upayload = rcu_dereference_key(key);
	ret = upayload->datalen;

	/* we can return the data as is */
	if (buffer && buflen > 0) {
		if (buflen > upayload->datalen)
			buflen = upayload->datalen;

		if (copy_to_user(buffer, upayload->data, buflen) != 0)
			ret = -EFAULT;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(user_read);

/* Vet the description for a "logon" key */
static int logon_vet_description(const char *desc)
{
	char *p;

	/* require a "qualified" description string */
	p = strchr(desc, ':');
	if (!p)
		return -EINVAL;

	/* also reject description with ':' as first char */
	if (p == desc)
		return -EINVAL;

	return 0;
}
