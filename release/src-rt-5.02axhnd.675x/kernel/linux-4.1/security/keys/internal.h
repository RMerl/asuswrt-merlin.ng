/* Authentication token and access key management internal defs
 *
 * Copyright (C) 2003-5, 2007 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef _INTERNAL_H
#define _INTERNAL_H

#include <linux/sched.h>
#include <linux/key-type.h>
#include <linux/task_work.h>

struct iovec;

#ifdef __KDEBUG
#define kenter(FMT, ...) \
	printk(KERN_DEBUG "==> %s("FMT")\n", __func__, ##__VA_ARGS__)
#define kleave(FMT, ...) \
	printk(KERN_DEBUG "<== %s()"FMT"\n", __func__, ##__VA_ARGS__)
#define kdebug(FMT, ...) \
	printk(KERN_DEBUG "   "FMT"\n", ##__VA_ARGS__)
#else
#define kenter(FMT, ...) \
	no_printk(KERN_DEBUG "==> %s("FMT")\n", __func__, ##__VA_ARGS__)
#define kleave(FMT, ...) \
	no_printk(KERN_DEBUG "<== %s()"FMT"\n", __func__, ##__VA_ARGS__)
#define kdebug(FMT, ...) \
	no_printk(KERN_DEBUG FMT"\n", ##__VA_ARGS__)
#endif

extern struct key_type key_type_dead;
extern struct key_type key_type_user;
extern struct key_type key_type_logon;

/*****************************************************************************/
/*
 * Keep track of keys for a user.
 *
 * This needs to be separate to user_struct to avoid a refcount-loop
 * (user_struct pins some keyrings which pin this struct).
 *
 * We also keep track of keys under request from userspace for this UID here.
 */
struct key_user {
	struct rb_node		node;
	struct mutex		cons_lock;	/* construction initiation lock */
	spinlock_t		lock;
	atomic_t		usage;		/* for accessing qnkeys & qnbytes */
	atomic_t		nkeys;		/* number of keys */
	atomic_t		nikeys;		/* number of instantiated keys */
	kuid_t			uid;
	int			qnkeys;		/* number of keys allocated to this user */
	int			qnbytes;	/* number of bytes allocated to this user */
};

extern struct rb_root	key_user_tree;
extern spinlock_t	key_user_lock;
extern struct key_user	root_key_user;

extern struct key_user *key_user_lookup(kuid_t uid);
extern void key_user_put(struct key_user *user);

/*
 * Key quota limits.
 * - root has its own separate limits to everyone else
 */
extern unsigned key_quota_root_maxkeys;
extern unsigned key_quota_root_maxbytes;
extern unsigned key_quota_maxkeys;
extern unsigned key_quota_maxbytes;

#define KEYQUOTA_LINK_BYTES	4		/* a link in a keyring is worth 4 bytes */


extern struct kmem_cache *key_jar;
extern struct rb_root key_serial_tree;
extern spinlock_t key_serial_lock;
extern struct mutex key_construction_mutex;
extern wait_queue_head_t request_key_conswq;


extern struct key_type *key_type_lookup(const char *type);
extern void key_type_put(struct key_type *ktype);

extern int __key_link_begin(struct key *keyring,
			    const struct keyring_index_key *index_key,
			    struct assoc_array_edit **_edit);
extern int __key_link_check_live_key(struct key *keyring, struct key *key);
extern void __key_link(struct key *key, struct assoc_array_edit **_edit);
extern void __key_link_end(struct key *keyring,
			   const struct keyring_index_key *index_key,
			   struct assoc_array_edit *edit);

extern key_ref_t find_key_to_update(key_ref_t keyring_ref,
				    const struct keyring_index_key *index_key);

extern struct key *keyring_search_instkey(struct key *keyring,
					  key_serial_t target_id);

extern int iterate_over_keyring(const struct key *keyring,
				int (*func)(const struct key *key, void *data),
				void *data);

struct keyring_search_context {
	struct keyring_index_key index_key;
	const struct cred	*cred;
	struct key_match_data	match_data;
	unsigned		flags;
#define KEYRING_SEARCH_NO_STATE_CHECK	0x0001	/* Skip state checks */
#define KEYRING_SEARCH_DO_STATE_CHECK	0x0002	/* Override NO_STATE_CHECK */
#define KEYRING_SEARCH_NO_UPDATE_TIME	0x0004	/* Don't update times */
#define KEYRING_SEARCH_NO_CHECK_PERM	0x0008	/* Don't check permissions */
#define KEYRING_SEARCH_DETECT_TOO_DEEP	0x0010	/* Give an error on excessive depth */
#define KEYRING_SEARCH_SKIP_EXPIRED	0x0020	/* Ignore expired keys (intention to replace) */

	int (*iterator)(const void *object, void *iterator_data);

	/* Internal stuff */
	int			skipped_ret;
	bool			possessed;
	key_ref_t		result;
	struct timespec		now;
};

extern bool key_default_cmp(const struct key *key,
			    const struct key_match_data *match_data);
extern key_ref_t keyring_search_aux(key_ref_t keyring_ref,
				    struct keyring_search_context *ctx);

extern key_ref_t search_my_process_keyrings(struct keyring_search_context *ctx);
extern key_ref_t search_process_keyrings(struct keyring_search_context *ctx);

extern struct key *find_keyring_by_name(const char *name, bool uid_keyring);

extern int install_user_keyrings(void);
extern int install_thread_keyring_to_cred(struct cred *);
extern int install_process_keyring_to_cred(struct cred *);
extern int install_session_keyring_to_cred(struct cred *, struct key *);

extern struct key *request_key_and_link(struct key_type *type,
					const char *description,
					const void *callout_info,
					size_t callout_len,
					void *aux,
					struct key *dest_keyring,
					unsigned long flags);

extern bool lookup_user_key_possessed(const struct key *key,
				      const struct key_match_data *match_data);
extern key_ref_t lookup_user_key(key_serial_t id, unsigned long flags,
				 key_perm_t perm);
#define KEY_LOOKUP_CREATE	0x01
#define KEY_LOOKUP_PARTIAL	0x02
#define KEY_LOOKUP_FOR_UNLINK	0x04

extern long join_session_keyring(const char *name);
extern void key_change_session_keyring(struct callback_head *twork);

extern struct work_struct key_gc_work;
extern unsigned key_gc_delay;
extern void keyring_gc(struct key *keyring, time_t limit);
extern void key_schedule_gc(time_t gc_at);
extern void key_schedule_gc_links(void);
extern void key_gc_keytype(struct key_type *ktype);

extern int key_task_permission(const key_ref_t key_ref,
			       const struct cred *cred,
			       key_perm_t perm);

/*
 * Check to see whether permission is granted to use a key in the desired way.
 */
static inline int key_permission(const key_ref_t key_ref, unsigned perm)
{
	return key_task_permission(key_ref, current_cred(), perm);
}

/*
 * Authorisation record for request_key().
 */
struct request_key_auth {
	struct key		*target_key;
	struct key		*dest_keyring;
	const struct cred	*cred;
	void			*callout_info;
	size_t			callout_len;
	pid_t			pid;
};

extern struct key_type key_type_request_key_auth;
extern struct key *request_key_auth_new(struct key *target,
					const void *callout_info,
					size_t callout_len,
					struct key *dest_keyring);

extern struct key *key_get_instantiation_authkey(key_serial_t target_id);

/*
 * Determine whether a key is dead.
 */
static inline bool key_is_dead(const struct key *key, time_t limit)
{
	return
		key->flags & ((1 << KEY_FLAG_DEAD) |
			      (1 << KEY_FLAG_INVALIDATED)) ||
		(key->expiry > 0 && key->expiry <= limit);
}

/*
 * keyctl() functions
 */
extern long keyctl_get_keyring_ID(key_serial_t, int);
extern long keyctl_join_session_keyring(const char __user *);
extern long keyctl_update_key(key_serial_t, const void __user *, size_t);
extern long keyctl_revoke_key(key_serial_t);
extern long keyctl_keyring_clear(key_serial_t);
extern long keyctl_keyring_link(key_serial_t, key_serial_t);
extern long keyctl_keyring_unlink(key_serial_t, key_serial_t);
extern long keyctl_describe_key(key_serial_t, char __user *, size_t);
extern long keyctl_keyring_search(key_serial_t, const char __user *,
				  const char __user *, key_serial_t);
extern long keyctl_read_key(key_serial_t, char __user *, size_t);
extern long keyctl_chown_key(key_serial_t, uid_t, gid_t);
extern long keyctl_setperm_key(key_serial_t, key_perm_t);
extern long keyctl_instantiate_key(key_serial_t, const void __user *,
				   size_t, key_serial_t);
extern long keyctl_negate_key(key_serial_t, unsigned, key_serial_t);
extern long keyctl_set_reqkey_keyring(int);
extern long keyctl_set_timeout(key_serial_t, unsigned);
extern long keyctl_assume_authority(key_serial_t);
extern long keyctl_get_security(key_serial_t keyid, char __user *buffer,
				size_t buflen);
extern long keyctl_session_to_parent(void);
extern long keyctl_reject_key(key_serial_t, unsigned, unsigned, key_serial_t);
extern long keyctl_instantiate_key_iov(key_serial_t,
				       const struct iovec __user *,
				       unsigned, key_serial_t);
extern long keyctl_invalidate_key(key_serial_t);

struct iov_iter;
extern long keyctl_instantiate_key_common(key_serial_t,
					  struct iov_iter *,
					  key_serial_t);
#ifdef CONFIG_PERSISTENT_KEYRINGS
extern long keyctl_get_persistent(uid_t, key_serial_t);
extern unsigned persistent_keyring_expiry;
#else
static inline long keyctl_get_persistent(uid_t uid, key_serial_t destring)
{
	return -EOPNOTSUPP;
}
#endif

/*
 * Debugging key validation
 */
#ifdef KEY_DEBUGGING
extern void __key_check(const struct key *);

static inline void key_check(const struct key *key)
{
	if (key && (IS_ERR(key) || key->magic != KEY_DEBUG_MAGIC))
		__key_check(key);
}

#else

#define key_check(key) do {} while(0)

#endif

#endif /* _INTERNAL_H */
