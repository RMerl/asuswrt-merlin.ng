/*
 * kernfs.h - pseudo filesystem decoupled from vfs locking
 *
 * This file is released under the GPLv2.
 */

#ifndef __LINUX_KERNFS_H
#define __LINUX_KERNFS_H

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/lockdep.h>
#include <linux/rbtree.h>
#include <linux/atomic.h>
#include <linux/wait.h>

struct file;
struct dentry;
struct iattr;
struct seq_file;
struct vm_area_struct;
struct super_block;
struct file_system_type;

struct kernfs_open_node;
struct kernfs_iattrs;

enum kernfs_node_type {
	KERNFS_DIR		= 0x0001,
	KERNFS_FILE		= 0x0002,
	KERNFS_LINK		= 0x0004,
};

#define KERNFS_TYPE_MASK	0x000f
#define KERNFS_FLAG_MASK	~KERNFS_TYPE_MASK

enum kernfs_node_flag {
	KERNFS_ACTIVATED	= 0x0010,
	KERNFS_NS		= 0x0020,
	KERNFS_HAS_SEQ_SHOW	= 0x0040,
	KERNFS_HAS_MMAP		= 0x0080,
	KERNFS_LOCKDEP		= 0x0100,
	KERNFS_SUICIDAL		= 0x0400,
	KERNFS_SUICIDED		= 0x0800,
	KERNFS_EMPTY_DIR	= 0x1000,
};

/* @flags for kernfs_create_root() */
enum kernfs_root_flag {
	/*
	 * kernfs_nodes are created in the deactivated state and invisible.
	 * They require explicit kernfs_activate() to become visible.  This
	 * can be used to make related nodes become visible atomically
	 * after all nodes are created successfully.
	 */
	KERNFS_ROOT_CREATE_DEACTIVATED		= 0x0001,

	/*
	 * For regular flies, if the opener has CAP_DAC_OVERRIDE, open(2)
	 * succeeds regardless of the RW permissions.  sysfs had an extra
	 * layer of enforcement where open(2) fails with -EACCES regardless
	 * of CAP_DAC_OVERRIDE if the permission doesn't have the
	 * respective read or write access at all (none of S_IRUGO or
	 * S_IWUGO) or the respective operation isn't implemented.  The
	 * following flag enables that behavior.
	 */
	KERNFS_ROOT_EXTRA_OPEN_PERM_CHECK	= 0x0002,
};

/* type-specific structures for kernfs_node union members */
struct kernfs_elem_dir {
	unsigned long		subdirs;
	/* children rbtree starts here and goes through kn->rb */
	struct rb_root		children;

	/*
	 * The kernfs hierarchy this directory belongs to.  This fits
	 * better directly in kernfs_node but is here to save space.
	 */
	struct kernfs_root	*root;
};

struct kernfs_elem_symlink {
	struct kernfs_node	*target_kn;
};

struct kernfs_elem_attr {
	const struct kernfs_ops	*ops;
	struct kernfs_open_node	*open;
	loff_t			size;
	struct kernfs_node	*notify_next;	/* for kernfs_notify() */
};

/*
 * kernfs_node - the building block of kernfs hierarchy.  Each and every
 * kernfs node is represented by single kernfs_node.  Most fields are
 * private to kernfs and shouldn't be accessed directly by kernfs users.
 *
 * As long as s_count reference is held, the kernfs_node itself is
 * accessible.  Dereferencing elem or any other outer entity requires
 * active reference.
 */
struct kernfs_node {
	atomic_t		count;
	atomic_t		active;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
	/*
	 * Use kernfs_get_parent() and kernfs_name/path() instead of
	 * accessing the following two fields directly.  If the node is
	 * never moved to a different parent, it is safe to access the
	 * parent directly.
	 */
	struct kernfs_node	*parent;
	const char		*name;

	struct rb_node		rb;

	const void		*ns;	/* namespace tag */
	unsigned int		hash;	/* ns + name hash */
	union {
		struct kernfs_elem_dir		dir;
		struct kernfs_elem_symlink	symlink;
		struct kernfs_elem_attr		attr;
	};

	void			*priv;

	unsigned short		flags;
	umode_t			mode;
	unsigned int		ino;
	struct kernfs_iattrs	*iattr;
};

/*
 * kernfs_syscall_ops may be specified on kernfs_create_root() to support
 * syscalls.  These optional callbacks are invoked on the matching syscalls
 * and can perform any kernfs operations which don't necessarily have to be
 * the exact operation requested.  An active reference is held for each
 * kernfs_node parameter.
 */
struct kernfs_syscall_ops {
	int (*remount_fs)(struct kernfs_root *root, int *flags, char *data);
	int (*show_options)(struct seq_file *sf, struct kernfs_root *root);

	int (*mkdir)(struct kernfs_node *parent, const char *name,
		     umode_t mode);
	int (*rmdir)(struct kernfs_node *kn);
	int (*rename)(struct kernfs_node *kn, struct kernfs_node *new_parent,
		      const char *new_name);
};

struct kernfs_root {
	/* published fields */
	struct kernfs_node	*kn;
	unsigned int		flags;	/* KERNFS_ROOT_* flags */

	/* private fields, do not use outside kernfs proper */
	struct ida		ino_ida;
	struct kernfs_syscall_ops *syscall_ops;

	/* list of kernfs_super_info of this root, protected by kernfs_mutex */
	struct list_head	supers;

	wait_queue_head_t	deactivate_waitq;
};

struct kernfs_open_file {
	/* published fields */
	struct kernfs_node	*kn;
	struct file		*file;
	void			*priv;

	/* private fields, do not use outside kernfs proper */
	struct mutex		mutex;
	int			event;
	struct list_head	list;
	char			*prealloc_buf;

	size_t			atomic_write_len;
	bool			mmapped;
	const struct vm_operations_struct *vm_ops;
};

struct kernfs_ops {
	/*
	 * Read is handled by either seq_file or raw_read().
	 *
	 * If seq_show() is present, seq_file path is active.  Other seq
	 * operations are optional and if not implemented, the behavior is
	 * equivalent to single_open().  @sf->private points to the
	 * associated kernfs_open_file.
	 *
	 * read() is bounced through kernel buffer and a read larger than
	 * PAGE_SIZE results in partial operation of PAGE_SIZE.
	 */
	int (*seq_show)(struct seq_file *sf, void *v);

	void *(*seq_start)(struct seq_file *sf, loff_t *ppos);
	void *(*seq_next)(struct seq_file *sf, void *v, loff_t *ppos);
	void (*seq_stop)(struct seq_file *sf, void *v);

	ssize_t (*read)(struct kernfs_open_file *of, char *buf, size_t bytes,
			loff_t off);

	/*
	 * write() is bounced through kernel buffer.  If atomic_write_len
	 * is not set, a write larger than PAGE_SIZE results in partial
	 * operations of PAGE_SIZE chunks.  If atomic_write_len is set,
	 * writes upto the specified size are executed atomically but
	 * larger ones are rejected with -E2BIG.
	 */
	size_t atomic_write_len;
	/*
	 * "prealloc" causes a buffer to be allocated at open for
	 * all read/write requests.  As ->seq_show uses seq_read()
	 * which does its own allocation, it is incompatible with
	 * ->prealloc.  Provide ->read and ->write with ->prealloc.
	 */
	bool prealloc;
	ssize_t (*write)(struct kernfs_open_file *of, char *buf, size_t bytes,
			 loff_t off);

	int (*mmap)(struct kernfs_open_file *of, struct vm_area_struct *vma);

#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lock_class_key	lockdep_key;
#endif
};

#ifdef CONFIG_KERNFS

static inline enum kernfs_node_type kernfs_type(struct kernfs_node *kn)
{
	return kn->flags & KERNFS_TYPE_MASK;
}

/**
 * kernfs_enable_ns - enable namespace under a directory
 * @kn: directory of interest, should be empty
 *
 * This is to be called right after @kn is created to enable namespace
 * under it.  All children of @kn must have non-NULL namespace tags and
 * only the ones which match the super_block's tag will be visible.
 */
static inline void kernfs_enable_ns(struct kernfs_node *kn)
{
	WARN_ON_ONCE(kernfs_type(kn) != KERNFS_DIR);
	WARN_ON_ONCE(!RB_EMPTY_ROOT(&kn->dir.children));
	kn->flags |= KERNFS_NS;
}

/**
 * kernfs_ns_enabled - test whether namespace is enabled
 * @kn: the node to test
 *
 * Test whether namespace filtering is enabled for the children of @ns.
 */
static inline bool kernfs_ns_enabled(struct kernfs_node *kn)
{
	return kn->flags & KERNFS_NS;
}

int kernfs_name(struct kernfs_node *kn, char *buf, size_t buflen);
char * __must_check kernfs_path(struct kernfs_node *kn, char *buf,
				size_t buflen);
void pr_cont_kernfs_name(struct kernfs_node *kn);
void pr_cont_kernfs_path(struct kernfs_node *kn);
struct kernfs_node *kernfs_get_parent(struct kernfs_node *kn);
struct kernfs_node *kernfs_find_and_get_ns(struct kernfs_node *parent,
					   const char *name, const void *ns);
void kernfs_get(struct kernfs_node *kn);
void kernfs_put(struct kernfs_node *kn);

struct kernfs_node *kernfs_node_from_dentry(struct dentry *dentry);
struct kernfs_root *kernfs_root_from_sb(struct super_block *sb);

struct kernfs_root *kernfs_create_root(struct kernfs_syscall_ops *scops,
				       unsigned int flags, void *priv);
void kernfs_destroy_root(struct kernfs_root *root);

struct kernfs_node *kernfs_create_dir_ns(struct kernfs_node *parent,
					 const char *name, umode_t mode,
					 void *priv, const void *ns);
struct kernfs_node *kernfs_create_empty_dir(struct kernfs_node *parent,
					    const char *name);
struct kernfs_node *__kernfs_create_file(struct kernfs_node *parent,
					 const char *name,
					 umode_t mode, loff_t size,
					 const struct kernfs_ops *ops,
					 void *priv, const void *ns,
					 struct lock_class_key *key);
struct kernfs_node *kernfs_create_link(struct kernfs_node *parent,
				       const char *name,
				       struct kernfs_node *target);
void kernfs_activate(struct kernfs_node *kn);
void kernfs_remove(struct kernfs_node *kn);
void kernfs_break_active_protection(struct kernfs_node *kn);
void kernfs_unbreak_active_protection(struct kernfs_node *kn);
bool kernfs_remove_self(struct kernfs_node *kn);
int kernfs_remove_by_name_ns(struct kernfs_node *parent, const char *name,
			     const void *ns);
int kernfs_rename_ns(struct kernfs_node *kn, struct kernfs_node *new_parent,
		     const char *new_name, const void *new_ns);
int kernfs_setattr(struct kernfs_node *kn, const struct iattr *iattr);
void kernfs_notify(struct kernfs_node *kn);

const void *kernfs_super_ns(struct super_block *sb);
struct dentry *kernfs_mount_ns(struct file_system_type *fs_type, int flags,
			       struct kernfs_root *root, unsigned long magic,
			       bool *new_sb_created, const void *ns);
void kernfs_kill_sb(struct super_block *sb);
struct super_block *kernfs_pin_sb(struct kernfs_root *root, const void *ns);

void kernfs_init(void);

#else	/* CONFIG_KERNFS */

static inline enum kernfs_node_type kernfs_type(struct kernfs_node *kn)
{ return 0; }	/* whatever */

static inline void kernfs_enable_ns(struct kernfs_node *kn) { }

static inline bool kernfs_ns_enabled(struct kernfs_node *kn)
{ return false; }

static inline int kernfs_name(struct kernfs_node *kn, char *buf, size_t buflen)
{ return -ENOSYS; }

static inline char * __must_check kernfs_path(struct kernfs_node *kn, char *buf,
					      size_t buflen)
{ return NULL; }

static inline void pr_cont_kernfs_name(struct kernfs_node *kn) { }
static inline void pr_cont_kernfs_path(struct kernfs_node *kn) { }

static inline struct kernfs_node *kernfs_get_parent(struct kernfs_node *kn)
{ return NULL; }

static inline struct kernfs_node *
kernfs_find_and_get_ns(struct kernfs_node *parent, const char *name,
		       const void *ns)
{ return NULL; }

static inline void kernfs_get(struct kernfs_node *kn) { }
static inline void kernfs_put(struct kernfs_node *kn) { }

static inline struct kernfs_node *kernfs_node_from_dentry(struct dentry *dentry)
{ return NULL; }

static inline struct kernfs_root *kernfs_root_from_sb(struct super_block *sb)
{ return NULL; }

static inline struct kernfs_root *
kernfs_create_root(struct kernfs_syscall_ops *scops, unsigned int flags,
		   void *priv)
{ return ERR_PTR(-ENOSYS); }

static inline void kernfs_destroy_root(struct kernfs_root *root) { }

static inline struct kernfs_node *
kernfs_create_dir_ns(struct kernfs_node *parent, const char *name,
		     umode_t mode, void *priv, const void *ns)
{ return ERR_PTR(-ENOSYS); }

static inline struct kernfs_node *
__kernfs_create_file(struct kernfs_node *parent, const char *name,
		     umode_t mode, loff_t size, const struct kernfs_ops *ops,
		     void *priv, const void *ns, struct lock_class_key *key)
{ return ERR_PTR(-ENOSYS); }

static inline struct kernfs_node *
kernfs_create_link(struct kernfs_node *parent, const char *name,
		   struct kernfs_node *target)
{ return ERR_PTR(-ENOSYS); }

static inline void kernfs_activate(struct kernfs_node *kn) { }

static inline void kernfs_remove(struct kernfs_node *kn) { }

static inline bool kernfs_remove_self(struct kernfs_node *kn)
{ return false; }

static inline int kernfs_remove_by_name_ns(struct kernfs_node *kn,
					   const char *name, const void *ns)
{ return -ENOSYS; }

static inline int kernfs_rename_ns(struct kernfs_node *kn,
				   struct kernfs_node *new_parent,
				   const char *new_name, const void *new_ns)
{ return -ENOSYS; }

static inline int kernfs_setattr(struct kernfs_node *kn,
				 const struct iattr *iattr)
{ return -ENOSYS; }

static inline void kernfs_notify(struct kernfs_node *kn) { }

static inline const void *kernfs_super_ns(struct super_block *sb)
{ return NULL; }

static inline struct dentry *
kernfs_mount_ns(struct file_system_type *fs_type, int flags,
		struct kernfs_root *root, unsigned long magic,
		bool *new_sb_created, const void *ns)
{ return ERR_PTR(-ENOSYS); }

static inline void kernfs_kill_sb(struct super_block *sb) { }

static inline void kernfs_init(void) { }

#endif	/* CONFIG_KERNFS */

static inline struct kernfs_node *
kernfs_find_and_get(struct kernfs_node *kn, const char *name)
{
	return kernfs_find_and_get_ns(kn, name, NULL);
}

static inline struct kernfs_node *
kernfs_create_dir(struct kernfs_node *parent, const char *name, umode_t mode,
		  void *priv)
{
	return kernfs_create_dir_ns(parent, name, mode, priv, NULL);
}

static inline struct kernfs_node *
kernfs_create_file_ns(struct kernfs_node *parent, const char *name,
		      umode_t mode, loff_t size, const struct kernfs_ops *ops,
		      void *priv, const void *ns)
{
	struct lock_class_key *key = NULL;

#ifdef CONFIG_DEBUG_LOCK_ALLOC
	key = (struct lock_class_key *)&ops->lockdep_key;
#endif
	return __kernfs_create_file(parent, name, mode, size, ops, priv, ns,
				    key);
}

static inline struct kernfs_node *
kernfs_create_file(struct kernfs_node *parent, const char *name, umode_t mode,
		   loff_t size, const struct kernfs_ops *ops, void *priv)
{
	return kernfs_create_file_ns(parent, name, mode, size, ops, priv, NULL);
}

static inline int kernfs_remove_by_name(struct kernfs_node *parent,
					const char *name)
{
	return kernfs_remove_by_name_ns(parent, name, NULL);
}

static inline int kernfs_rename(struct kernfs_node *kn,
				struct kernfs_node *new_parent,
				const char *new_name)
{
	return kernfs_rename_ns(kn, new_parent, new_name, NULL);
}

static inline struct dentry *
kernfs_mount(struct file_system_type *fs_type, int flags,
		struct kernfs_root *root, unsigned long magic,
		bool *new_sb_created)
{
	return kernfs_mount_ns(fs_type, flags, root,
				magic, new_sb_created, NULL);
}

#endif	/* __LINUX_KERNFS_H */
