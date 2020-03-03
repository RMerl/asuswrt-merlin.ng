/*
 *  linux/fs/locks.c
 *
 *  Provide support for fcntl()'s F_GETLK, F_SETLK, and F_SETLKW calls.
 *  Doug Evans (dje@spiff.uucp), August 07, 1992
 *
 *  Deadlock detection added.
 *  FIXME: one thing isn't handled yet:
 *	- mandatory locks (requires lots of changes elsewhere)
 *  Kelly Carmichael (kelly@[142.24.8.65]), September 17, 1994.
 *
 *  Miscellaneous edits, and a total rewrite of posix_lock_file() code.
 *  Kai Petzke (wpp@marie.physik.tu-berlin.de), 1994
 *  
 *  Converted file_lock_table to a linked list from an array, which eliminates
 *  the limits on how many active file locks are open.
 *  Chad Page (pageone@netcom.com), November 27, 1994
 * 
 *  Removed dependency on file descriptors. dup()'ed file descriptors now
 *  get the same locks as the original file descriptors, and a close() on
 *  any file descriptor removes ALL the locks on the file for the current
 *  process. Since locks still depend on the process id, locks are inherited
 *  after an exec() but not after a fork(). This agrees with POSIX, and both
 *  BSD and SVR4 practice.
 *  Andy Walker (andy@lysaker.kvaerner.no), February 14, 1995
 *
 *  Scrapped free list which is redundant now that we allocate locks
 *  dynamically with kmalloc()/kfree().
 *  Andy Walker (andy@lysaker.kvaerner.no), February 21, 1995
 *
 *  Implemented two lock personalities - FL_FLOCK and FL_POSIX.
 *
 *  FL_POSIX locks are created with calls to fcntl() and lockf() through the
 *  fcntl() system call. They have the semantics described above.
 *
 *  FL_FLOCK locks are created with calls to flock(), through the flock()
 *  system call, which is new. Old C libraries implement flock() via fcntl()
 *  and will continue to use the old, broken implementation.
 *
 *  FL_FLOCK locks follow the 4.4 BSD flock() semantics. They are associated
 *  with a file pointer (filp). As a result they can be shared by a parent
 *  process and its children after a fork(). They are removed when the last
 *  file descriptor referring to the file pointer is closed (unless explicitly
 *  unlocked). 
 *
 *  FL_FLOCK locks never deadlock, an existing lock is always removed before
 *  upgrading from shared to exclusive (or vice versa). When this happens
 *  any processes blocked by the current lock are woken up and allowed to
 *  run before the new lock is applied.
 *  Andy Walker (andy@lysaker.kvaerner.no), June 09, 1995
 *
 *  Removed some race conditions in flock_lock_file(), marked other possible
 *  races. Just grep for FIXME to see them. 
 *  Dmitry Gorodchanin (pgmdsg@ibi.com), February 09, 1996.
 *
 *  Addressed Dmitry's concerns. Deadlock checking no longer recursive.
 *  Lock allocation changed to GFP_ATOMIC as we can't afford to sleep
 *  once we've checked for blocking and deadlocking.
 *  Andy Walker (andy@lysaker.kvaerner.no), April 03, 1996.
 *
 *  Initial implementation of mandatory locks. SunOS turned out to be
 *  a rotten model, so I implemented the "obvious" semantics.
 *  See 'Documentation/filesystems/mandatory-locking.txt' for details.
 *  Andy Walker (andy@lysaker.kvaerner.no), April 06, 1996.
 *
 *  Don't allow mandatory locks on mmap()'ed files. Added simple functions to
 *  check if a file has mandatory locks, used by mmap(), open() and creat() to
 *  see if system call should be rejected. Ref. HP-UX/SunOS/Solaris Reference
 *  Manual, Section 2.
 *  Andy Walker (andy@lysaker.kvaerner.no), April 09, 1996.
 *
 *  Tidied up block list handling. Added '/proc/locks' interface.
 *  Andy Walker (andy@lysaker.kvaerner.no), April 24, 1996.
 *
 *  Fixed deadlock condition for pathological code that mixes calls to
 *  flock() and fcntl().
 *  Andy Walker (andy@lysaker.kvaerner.no), April 29, 1996.
 *
 *  Allow only one type of locking scheme (FL_POSIX or FL_FLOCK) to be in use
 *  for a given file at a time. Changed the CONFIG_LOCK_MANDATORY scheme to
 *  guarantee sensible behaviour in the case where file system modules might
 *  be compiled with different options than the kernel itself.
 *  Andy Walker (andy@lysaker.kvaerner.no), May 15, 1996.
 *
 *  Added a couple of missing wake_up() calls. Thanks to Thomas Meckel
 *  (Thomas.Meckel@mni.fh-giessen.de) for spotting this.
 *  Andy Walker (andy@lysaker.kvaerner.no), May 15, 1996.
 *
 *  Changed FL_POSIX locks to use the block list in the same way as FL_FLOCK
 *  locks. Changed process synchronisation to avoid dereferencing locks that
 *  have already been freed.
 *  Andy Walker (andy@lysaker.kvaerner.no), Sep 21, 1996.
 *
 *  Made the block list a circular list to minimise searching in the list.
 *  Andy Walker (andy@lysaker.kvaerner.no), Sep 25, 1996.
 *
 *  Made mandatory locking a mount option. Default is not to allow mandatory
 *  locking.
 *  Andy Walker (andy@lysaker.kvaerner.no), Oct 04, 1996.
 *
 *  Some adaptations for NFS support.
 *  Olaf Kirch (okir@monad.swb.de), Dec 1996,
 *
 *  Fixed /proc/locks interface so that we can't overrun the buffer we are handed.
 *  Andy Walker (andy@lysaker.kvaerner.no), May 12, 1997.
 *
 *  Use slab allocator instead of kmalloc/kfree.
 *  Use generic list implementation from <linux/list.h>.
 *  Sped up posix_locks_deadlock by only considering blocked locks.
 *  Matthew Wilcox <willy@debian.org>, March, 2000.
 *
 *  Leases and LOCK_MAND
 *  Matthew Wilcox <willy@debian.org>, June, 2000.
 *  Stephen Rothwell <sfr@canb.auug.org.au>, June, 2000.
 */

#include <linux/capability.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/security.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/time.h>
#include <linux/rcupdate.h>
#include <linux/pid_namespace.h>
#include <linux/hashtable.h>
#include <linux/percpu.h>
#include <linux/lglock.h>

#define CREATE_TRACE_POINTS
#include <trace/events/filelock.h>

#include <asm/uaccess.h>

#define IS_POSIX(fl)	(fl->fl_flags & FL_POSIX)
#define IS_FLOCK(fl)	(fl->fl_flags & FL_FLOCK)
#define IS_LEASE(fl)	(fl->fl_flags & (FL_LEASE|FL_DELEG|FL_LAYOUT))
#define IS_OFDLCK(fl)	(fl->fl_flags & FL_OFDLCK)

static bool lease_breaking(struct file_lock *fl)
{
	return fl->fl_flags & (FL_UNLOCK_PENDING | FL_DOWNGRADE_PENDING);
}

static int target_leasetype(struct file_lock *fl)
{
	if (fl->fl_flags & FL_UNLOCK_PENDING)
		return F_UNLCK;
	if (fl->fl_flags & FL_DOWNGRADE_PENDING)
		return F_RDLCK;
	return fl->fl_type;
}

int leases_enable = 1;
int lease_break_time = 45;

/*
 * The global file_lock_list is only used for displaying /proc/locks, so we
 * keep a list on each CPU, with each list protected by its own spinlock via
 * the file_lock_lglock. Note that alterations to the list also require that
 * the relevant flc_lock is held.
 */
DEFINE_STATIC_LGLOCK(file_lock_lglock);
static DEFINE_PER_CPU(struct hlist_head, file_lock_list);

/*
 * The blocked_hash is used to find POSIX lock loops for deadlock detection.
 * It is protected by blocked_lock_lock.
 *
 * We hash locks by lockowner in order to optimize searching for the lock a
 * particular lockowner is waiting on.
 *
 * FIXME: make this value scale via some heuristic? We generally will want more
 * buckets when we have more lockowners holding locks, but that's a little
 * difficult to determine without knowing what the workload will look like.
 */
#define BLOCKED_HASH_BITS	7
static DEFINE_HASHTABLE(blocked_hash, BLOCKED_HASH_BITS);

/*
 * This lock protects the blocked_hash. Generally, if you're accessing it, you
 * want to be holding this lock.
 *
 * In addition, it also protects the fl->fl_block list, and the fl->fl_next
 * pointer for file_lock structures that are acting as lock requests (in
 * contrast to those that are acting as records of acquired locks).
 *
 * Note that when we acquire this lock in order to change the above fields,
 * we often hold the flc_lock as well. In certain cases, when reading the fields
 * protected by this lock, we can skip acquiring it iff we already hold the
 * flc_lock.
 *
 * In particular, adding an entry to the fl_block list requires that you hold
 * both the flc_lock and the blocked_lock_lock (acquired in that order).
 * Deleting an entry from the list however only requires the file_lock_lock.
 */
static DEFINE_SPINLOCK(blocked_lock_lock);

static struct kmem_cache *flctx_cache __read_mostly;
static struct kmem_cache *filelock_cache __read_mostly;

static struct file_lock_context *
locks_get_lock_context(struct inode *inode, int type)
{
	struct file_lock_context *new;

	if (likely(inode->i_flctx) || type == F_UNLCK)
		goto out;

	new = kmem_cache_alloc(flctx_cache, GFP_KERNEL);
	if (!new)
		goto out;

	spin_lock_init(&new->flc_lock);
	INIT_LIST_HEAD(&new->flc_flock);
	INIT_LIST_HEAD(&new->flc_posix);
	INIT_LIST_HEAD(&new->flc_lease);

	/*
	 * Assign the pointer if it's not already assigned. If it is, then
	 * free the context we just allocated.
	 */
	if (cmpxchg(&inode->i_flctx, NULL, new))
		kmem_cache_free(flctx_cache, new);
out:
	return inode->i_flctx;
}

void
locks_free_lock_context(struct file_lock_context *ctx)
{
	if (ctx) {
		WARN_ON_ONCE(!list_empty(&ctx->flc_flock));
		WARN_ON_ONCE(!list_empty(&ctx->flc_posix));
		WARN_ON_ONCE(!list_empty(&ctx->flc_lease));
		kmem_cache_free(flctx_cache, ctx);
	}
}

static void locks_init_lock_heads(struct file_lock *fl)
{
	INIT_HLIST_NODE(&fl->fl_link);
	INIT_LIST_HEAD(&fl->fl_list);
	INIT_LIST_HEAD(&fl->fl_block);
	init_waitqueue_head(&fl->fl_wait);
}

/* Allocate an empty lock structure. */
struct file_lock *locks_alloc_lock(void)
{
	struct file_lock *fl = kmem_cache_zalloc(filelock_cache, GFP_KERNEL);

	if (fl)
		locks_init_lock_heads(fl);

	return fl;
}
EXPORT_SYMBOL_GPL(locks_alloc_lock);

void locks_release_private(struct file_lock *fl)
{
	if (fl->fl_ops) {
		if (fl->fl_ops->fl_release_private)
			fl->fl_ops->fl_release_private(fl);
		fl->fl_ops = NULL;
	}

	if (fl->fl_lmops) {
		if (fl->fl_lmops->lm_put_owner) {
			fl->fl_lmops->lm_put_owner(fl->fl_owner);
			fl->fl_owner = NULL;
		}
		fl->fl_lmops = NULL;
	}
}
EXPORT_SYMBOL_GPL(locks_release_private);

/* Free a lock which is not in use. */
void locks_free_lock(struct file_lock *fl)
{
	BUG_ON(waitqueue_active(&fl->fl_wait));
	BUG_ON(!list_empty(&fl->fl_list));
	BUG_ON(!list_empty(&fl->fl_block));
	BUG_ON(!hlist_unhashed(&fl->fl_link));

	locks_release_private(fl);
	kmem_cache_free(filelock_cache, fl);
}
EXPORT_SYMBOL(locks_free_lock);

static void
locks_dispose_list(struct list_head *dispose)
{
	struct file_lock *fl;

	while (!list_empty(dispose)) {
		fl = list_first_entry(dispose, struct file_lock, fl_list);
		list_del_init(&fl->fl_list);
		locks_free_lock(fl);
	}
}

void locks_init_lock(struct file_lock *fl)
{
	memset(fl, 0, sizeof(struct file_lock));
	locks_init_lock_heads(fl);
}

EXPORT_SYMBOL(locks_init_lock);

/*
 * Initialize a new lock from an existing file_lock structure.
 */
void locks_copy_conflock(struct file_lock *new, struct file_lock *fl)
{
	new->fl_owner = fl->fl_owner;
	new->fl_pid = fl->fl_pid;
	new->fl_file = NULL;
	new->fl_flags = fl->fl_flags;
	new->fl_type = fl->fl_type;
	new->fl_start = fl->fl_start;
	new->fl_end = fl->fl_end;
	new->fl_lmops = fl->fl_lmops;
	new->fl_ops = NULL;

	if (fl->fl_lmops) {
		if (fl->fl_lmops->lm_get_owner)
			fl->fl_lmops->lm_get_owner(fl->fl_owner);
	}
}
EXPORT_SYMBOL(locks_copy_conflock);

void locks_copy_lock(struct file_lock *new, struct file_lock *fl)
{
	/* "new" must be a freshly-initialized lock */
	WARN_ON_ONCE(new->fl_ops);

	locks_copy_conflock(new, fl);

	new->fl_file = fl->fl_file;
	new->fl_ops = fl->fl_ops;

	if (fl->fl_ops) {
		if (fl->fl_ops->fl_copy_lock)
			fl->fl_ops->fl_copy_lock(new, fl);
	}
}

EXPORT_SYMBOL(locks_copy_lock);

static inline int flock_translate_cmd(int cmd) {
	if (cmd & LOCK_MAND)
		return cmd & (LOCK_MAND | LOCK_RW);
	switch (cmd) {
	case LOCK_SH:
		return F_RDLCK;
	case LOCK_EX:
		return F_WRLCK;
	case LOCK_UN:
		return F_UNLCK;
	}
	return -EINVAL;
}

/* Fill in a file_lock structure with an appropriate FLOCK lock. */
static struct file_lock *
flock_make_lock(struct file *filp, unsigned int cmd)
{
	struct file_lock *fl;
	int type = flock_translate_cmd(cmd);

	if (type < 0)
		return ERR_PTR(type);
	
	fl = locks_alloc_lock();
	if (fl == NULL)
		return ERR_PTR(-ENOMEM);

	fl->fl_file = filp;
	fl->fl_owner = filp;
	fl->fl_pid = current->tgid;
	fl->fl_flags = FL_FLOCK;
	fl->fl_type = type;
	fl->fl_end = OFFSET_MAX;
	
	return fl;
}

static int assign_type(struct file_lock *fl, long type)
{
	switch (type) {
	case F_RDLCK:
	case F_WRLCK:
	case F_UNLCK:
		fl->fl_type = type;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int flock64_to_posix_lock(struct file *filp, struct file_lock *fl,
				 struct flock64 *l)
{
	switch (l->l_whence) {
	case SEEK_SET:
		fl->fl_start = 0;
		break;
	case SEEK_CUR:
		fl->fl_start = filp->f_pos;
		break;
	case SEEK_END:
		fl->fl_start = i_size_read(file_inode(filp));
		break;
	default:
		return -EINVAL;
	}
	if (l->l_start > OFFSET_MAX - fl->fl_start)
		return -EOVERFLOW;
	fl->fl_start += l->l_start;
	if (fl->fl_start < 0)
		return -EINVAL;

	/* POSIX-1996 leaves the case l->l_len < 0 undefined;
	   POSIX-2001 defines it. */
	if (l->l_len > 0) {
		if (l->l_len - 1 > OFFSET_MAX - fl->fl_start)
			return -EOVERFLOW;
		fl->fl_end = fl->fl_start + l->l_len - 1;

	} else if (l->l_len < 0) {
		if (fl->fl_start + l->l_len < 0)
			return -EINVAL;
		fl->fl_end = fl->fl_start - 1;
		fl->fl_start += l->l_len;
	} else
		fl->fl_end = OFFSET_MAX;

	fl->fl_owner = current->files;
	fl->fl_pid = current->tgid;
	fl->fl_file = filp;
	fl->fl_flags = FL_POSIX;
	fl->fl_ops = NULL;
	fl->fl_lmops = NULL;

	return assign_type(fl, l->l_type);
}

/* Verify a "struct flock" and copy it to a "struct file_lock" as a POSIX
 * style lock.
 */
static int flock_to_posix_lock(struct file *filp, struct file_lock *fl,
			       struct flock *l)
{
	struct flock64 ll = {
		.l_type = l->l_type,
		.l_whence = l->l_whence,
		.l_start = l->l_start,
		.l_len = l->l_len,
	};

	return flock64_to_posix_lock(filp, fl, &ll);
}

/* default lease lock manager operations */
static bool
lease_break_callback(struct file_lock *fl)
{
	kill_fasync(&fl->fl_fasync, SIGIO, POLL_MSG);
	return false;
}

static void
lease_setup(struct file_lock *fl, void **priv)
{
	struct file *filp = fl->fl_file;
	struct fasync_struct *fa = *priv;

	/*
	 * fasync_insert_entry() returns the old entry if any. If there was no
	 * old entry, then it used "priv" and inserted it into the fasync list.
	 * Clear the pointer to indicate that it shouldn't be freed.
	 */
	if (!fasync_insert_entry(fa->fa_fd, filp, &fl->fl_fasync, fa))
		*priv = NULL;

	__f_setown(filp, task_pid(current), PIDTYPE_PID, 0);
}

static const struct lock_manager_operations lease_manager_ops = {
	.lm_break = lease_break_callback,
	.lm_change = lease_modify,
	.lm_setup = lease_setup,
};

/*
 * Initialize a lease, use the default lock manager operations
 */
static int lease_init(struct file *filp, long type, struct file_lock *fl)
 {
	if (assign_type(fl, type) != 0)
		return -EINVAL;

	fl->fl_owner = filp;
	fl->fl_pid = current->tgid;

	fl->fl_file = filp;
	fl->fl_flags = FL_LEASE;
	fl->fl_start = 0;
	fl->fl_end = OFFSET_MAX;
	fl->fl_ops = NULL;
	fl->fl_lmops = &lease_manager_ops;
	return 0;
}

/* Allocate a file_lock initialised to this type of lease */
static struct file_lock *lease_alloc(struct file *filp, long type)
{
	struct file_lock *fl = locks_alloc_lock();
	int error = -ENOMEM;

	if (fl == NULL)
		return ERR_PTR(error);

	error = lease_init(filp, type, fl);
	if (error) {
		locks_free_lock(fl);
		return ERR_PTR(error);
	}
	return fl;
}

/* Check if two locks overlap each other.
 */
static inline int locks_overlap(struct file_lock *fl1, struct file_lock *fl2)
{
	return ((fl1->fl_end >= fl2->fl_start) &&
		(fl2->fl_end >= fl1->fl_start));
}

/*
 * Check whether two locks have the same owner.
 */
static int posix_same_owner(struct file_lock *fl1, struct file_lock *fl2)
{
	if (fl1->fl_lmops && fl1->fl_lmops->lm_compare_owner)
		return fl2->fl_lmops == fl1->fl_lmops &&
			fl1->fl_lmops->lm_compare_owner(fl1, fl2);
	return fl1->fl_owner == fl2->fl_owner;
}

/* Must be called with the flc_lock held! */
static void locks_insert_global_locks(struct file_lock *fl)
{
	lg_local_lock(&file_lock_lglock);
	fl->fl_link_cpu = smp_processor_id();
	hlist_add_head(&fl->fl_link, this_cpu_ptr(&file_lock_list));
	lg_local_unlock(&file_lock_lglock);
}

/* Must be called with the flc_lock held! */
static void locks_delete_global_locks(struct file_lock *fl)
{
	/*
	 * Avoid taking lock if already unhashed. This is safe since this check
	 * is done while holding the flc_lock, and new insertions into the list
	 * also require that it be held.
	 */
	if (hlist_unhashed(&fl->fl_link))
		return;
	lg_local_lock_cpu(&file_lock_lglock, fl->fl_link_cpu);
	hlist_del_init(&fl->fl_link);
	lg_local_unlock_cpu(&file_lock_lglock, fl->fl_link_cpu);
}

static unsigned long
posix_owner_key(struct file_lock *fl)
{
	if (fl->fl_lmops && fl->fl_lmops->lm_owner_key)
		return fl->fl_lmops->lm_owner_key(fl);
	return (unsigned long)fl->fl_owner;
}

static void locks_insert_global_blocked(struct file_lock *waiter)
{
	lockdep_assert_held(&blocked_lock_lock);

	hash_add(blocked_hash, &waiter->fl_link, posix_owner_key(waiter));
}

static void locks_delete_global_blocked(struct file_lock *waiter)
{
	lockdep_assert_held(&blocked_lock_lock);

	hash_del(&waiter->fl_link);
}

/* Remove waiter from blocker's block list.
 * When blocker ends up pointing to itself then the list is empty.
 *
 * Must be called with blocked_lock_lock held.
 */
static void __locks_delete_block(struct file_lock *waiter)
{
	locks_delete_global_blocked(waiter);
	list_del_init(&waiter->fl_block);
	waiter->fl_next = NULL;
}

static void locks_delete_block(struct file_lock *waiter)
{
	spin_lock(&blocked_lock_lock);
	__locks_delete_block(waiter);
	spin_unlock(&blocked_lock_lock);
}

/* Insert waiter into blocker's block list.
 * We use a circular list so that processes can be easily woken up in
 * the order they blocked. The documentation doesn't require this but
 * it seems like the reasonable thing to do.
 *
 * Must be called with both the flc_lock and blocked_lock_lock held. The
 * fl_block list itself is protected by the blocked_lock_lock, but by ensuring
 * that the flc_lock is also held on insertions we can avoid taking the
 * blocked_lock_lock in some cases when we see that the fl_block list is empty.
 */
static void __locks_insert_block(struct file_lock *blocker,
					struct file_lock *waiter)
{
	BUG_ON(!list_empty(&waiter->fl_block));
	waiter->fl_next = blocker;
	list_add_tail(&waiter->fl_block, &blocker->fl_block);
	if (IS_POSIX(blocker) && !IS_OFDLCK(blocker))
		locks_insert_global_blocked(waiter);
}

/* Must be called with flc_lock held. */
static void locks_insert_block(struct file_lock *blocker,
					struct file_lock *waiter)
{
	spin_lock(&blocked_lock_lock);
	__locks_insert_block(blocker, waiter);
	spin_unlock(&blocked_lock_lock);
}

/*
 * Wake up processes blocked waiting for blocker.
 *
 * Must be called with the inode->flc_lock held!
 */
static void locks_wake_up_blocks(struct file_lock *blocker)
{
	/*
	 * Avoid taking global lock if list is empty. This is safe since new
	 * blocked requests are only added to the list under the flc_lock, and
	 * the flc_lock is always held here. Note that removal from the fl_block
	 * list does not require the flc_lock, so we must recheck list_empty()
	 * after acquiring the blocked_lock_lock.
	 */
	if (list_empty(&blocker->fl_block))
		return;

	spin_lock(&blocked_lock_lock);
	while (!list_empty(&blocker->fl_block)) {
		struct file_lock *waiter;

		waiter = list_first_entry(&blocker->fl_block,
				struct file_lock, fl_block);
		__locks_delete_block(waiter);
		if (waiter->fl_lmops && waiter->fl_lmops->lm_notify)
			waiter->fl_lmops->lm_notify(waiter);
		else
			wake_up(&waiter->fl_wait);
	}
	spin_unlock(&blocked_lock_lock);
}

static void
locks_insert_lock_ctx(struct file_lock *fl, struct list_head *before)
{
	fl->fl_nspid = get_pid(task_tgid(current));
	list_add_tail(&fl->fl_list, before);
	locks_insert_global_locks(fl);
}

static void
locks_unlink_lock_ctx(struct file_lock *fl)
{
	locks_delete_global_locks(fl);
	list_del_init(&fl->fl_list);
	if (fl->fl_nspid) {
		put_pid(fl->fl_nspid);
		fl->fl_nspid = NULL;
	}
	locks_wake_up_blocks(fl);
}

static void
locks_delete_lock_ctx(struct file_lock *fl, struct list_head *dispose)
{
	locks_unlink_lock_ctx(fl);
	if (dispose)
		list_add(&fl->fl_list, dispose);
	else
		locks_free_lock(fl);
}

/* Determine if lock sys_fl blocks lock caller_fl. Common functionality
 * checks for shared/exclusive status of overlapping locks.
 */
static int locks_conflict(struct file_lock *caller_fl, struct file_lock *sys_fl)
{
	if (sys_fl->fl_type == F_WRLCK)
		return 1;
	if (caller_fl->fl_type == F_WRLCK)
		return 1;
	return 0;
}

/* Determine if lock sys_fl blocks lock caller_fl. POSIX specific
 * checking before calling the locks_conflict().
 */
static int posix_locks_conflict(struct file_lock *caller_fl, struct file_lock *sys_fl)
{
	/* POSIX locks owned by the same process do not conflict with
	 * each other.
	 */
	if (posix_same_owner(caller_fl, sys_fl))
		return (0);

	/* Check whether they overlap */
	if (!locks_overlap(caller_fl, sys_fl))
		return 0;

	return (locks_conflict(caller_fl, sys_fl));
}

/* Determine if lock sys_fl blocks lock caller_fl. FLOCK specific
 * checking before calling the locks_conflict().
 */
static int flock_locks_conflict(struct file_lock *caller_fl, struct file_lock *sys_fl)
{
	/* FLOCK locks referring to the same filp do not conflict with
	 * each other.
	 */
	if (caller_fl->fl_file == sys_fl->fl_file)
		return (0);
	if ((caller_fl->fl_type & LOCK_MAND) || (sys_fl->fl_type & LOCK_MAND))
		return 0;

	return (locks_conflict(caller_fl, sys_fl));
}

void
posix_test_lock(struct file *filp, struct file_lock *fl)
{
	struct file_lock *cfl;
	struct file_lock_context *ctx;
	struct inode *inode = file_inode(filp);

	ctx = inode->i_flctx;
	if (!ctx || list_empty_careful(&ctx->flc_posix)) {
		fl->fl_type = F_UNLCK;
		return;
	}

	spin_lock(&ctx->flc_lock);
	list_for_each_entry(cfl, &ctx->flc_posix, fl_list) {
		if (posix_locks_conflict(fl, cfl)) {
			locks_copy_conflock(fl, cfl);
			if (cfl->fl_nspid)
				fl->fl_pid = pid_vnr(cfl->fl_nspid);
			goto out;
		}
	}
	fl->fl_type = F_UNLCK;
out:
	spin_unlock(&ctx->flc_lock);
	return;
}
EXPORT_SYMBOL(posix_test_lock);

/*
 * Deadlock detection:
 *
 * We attempt to detect deadlocks that are due purely to posix file
 * locks.
 *
 * We assume that a task can be waiting for at most one lock at a time.
 * So for any acquired lock, the process holding that lock may be
 * waiting on at most one other lock.  That lock in turns may be held by
 * someone waiting for at most one other lock.  Given a requested lock
 * caller_fl which is about to wait for a conflicting lock block_fl, we
 * follow this chain of waiters to ensure we are not about to create a
 * cycle.
 *
 * Since we do this before we ever put a process to sleep on a lock, we
 * are ensured that there is never a cycle; that is what guarantees that
 * the while() loop in posix_locks_deadlock() eventually completes.
 *
 * Note: the above assumption may not be true when handling lock
 * requests from a broken NFS client. It may also fail in the presence
 * of tasks (such as posix threads) sharing the same open file table.
 * To handle those cases, we just bail out after a few iterations.
 *
 * For FL_OFDLCK locks, the owner is the filp, not the files_struct.
 * Because the owner is not even nominally tied to a thread of
 * execution, the deadlock detection below can't reasonably work well. Just
 * skip it for those.
 *
 * In principle, we could do a more limited deadlock detection on FL_OFDLCK
 * locks that just checks for the case where two tasks are attempting to
 * upgrade from read to write locks on the same inode.
 */

#define MAX_DEADLK_ITERATIONS 10

/* Find a lock that the owner of the given block_fl is blocking on. */
static struct file_lock *what_owner_is_waiting_for(struct file_lock *block_fl)
{
	struct file_lock *fl;

	hash_for_each_possible(blocked_hash, fl, fl_link, posix_owner_key(block_fl)) {
		if (posix_same_owner(fl, block_fl))
			return fl->fl_next;
	}
	return NULL;
}

/* Must be called with the blocked_lock_lock held! */
static int posix_locks_deadlock(struct file_lock *caller_fl,
				struct file_lock *block_fl)
{
	int i = 0;

	lockdep_assert_held(&blocked_lock_lock);

	/*
	 * This deadlock detector can't reasonably detect deadlocks with
	 * FL_OFDLCK locks, since they aren't owned by a process, per-se.
	 */
	if (IS_OFDLCK(caller_fl))
		return 0;

	while ((block_fl = what_owner_is_waiting_for(block_fl))) {
		if (i++ > MAX_DEADLK_ITERATIONS)
			return 0;
		if (posix_same_owner(caller_fl, block_fl))
			return 1;
	}
	return 0;
}

/* Try to create a FLOCK lock on filp. We always insert new FLOCK locks
 * after any leases, but before any posix locks.
 *
 * Note that if called with an FL_EXISTS argument, the caller may determine
 * whether or not a lock was successfully freed by testing the return
 * value for -ENOENT.
 */
static int flock_lock_inode(struct inode *inode, struct file_lock *request)
{
	struct file_lock *new_fl = NULL;
	struct file_lock *fl;
	struct file_lock_context *ctx;
	int error = 0;
	bool found = false;
	LIST_HEAD(dispose);

	ctx = locks_get_lock_context(inode, request->fl_type);
	if (!ctx) {
		if (request->fl_type != F_UNLCK)
			return -ENOMEM;
		return (request->fl_flags & FL_EXISTS) ? -ENOENT : 0;
	}

	if (!(request->fl_flags & FL_ACCESS) && (request->fl_type != F_UNLCK)) {
		new_fl = locks_alloc_lock();
		if (!new_fl)
			return -ENOMEM;
	}

	spin_lock(&ctx->flc_lock);
	if (request->fl_flags & FL_ACCESS)
		goto find_conflict;

	list_for_each_entry(fl, &ctx->flc_flock, fl_list) {
		if (request->fl_file != fl->fl_file)
			continue;
		if (request->fl_type == fl->fl_type)
			goto out;
		found = true;
		locks_delete_lock_ctx(fl, &dispose);
		break;
	}

	if (request->fl_type == F_UNLCK) {
		if ((request->fl_flags & FL_EXISTS) && !found)
			error = -ENOENT;
		goto out;
	}

find_conflict:
	list_for_each_entry(fl, &ctx->flc_flock, fl_list) {
		if (!flock_locks_conflict(request, fl))
			continue;
		error = -EAGAIN;
		if (!(request->fl_flags & FL_SLEEP))
			goto out;
		error = FILE_LOCK_DEFERRED;
		locks_insert_block(fl, request);
		goto out;
	}
	if (request->fl_flags & FL_ACCESS)
		goto out;
	locks_copy_lock(new_fl, request);
	locks_insert_lock_ctx(new_fl, &ctx->flc_flock);
	new_fl = NULL;
	error = 0;

out:
	spin_unlock(&ctx->flc_lock);
	if (new_fl)
		locks_free_lock(new_fl);
	locks_dispose_list(&dispose);
	return error;
}

static int __posix_lock_file(struct inode *inode, struct file_lock *request, struct file_lock *conflock)
{
	struct file_lock *fl, *tmp;
	struct file_lock *new_fl = NULL;
	struct file_lock *new_fl2 = NULL;
	struct file_lock *left = NULL;
	struct file_lock *right = NULL;
	struct file_lock_context *ctx;
	int error;
	bool added = false;
	LIST_HEAD(dispose);

	ctx = locks_get_lock_context(inode, request->fl_type);
	if (!ctx)
		return (request->fl_type == F_UNLCK) ? 0 : -ENOMEM;

	/*
	 * We may need two file_lock structures for this operation,
	 * so we get them in advance to avoid races.
	 *
	 * In some cases we can be sure, that no new locks will be needed
	 */
	if (!(request->fl_flags & FL_ACCESS) &&
	    (request->fl_type != F_UNLCK ||
	     request->fl_start != 0 || request->fl_end != OFFSET_MAX)) {
		new_fl = locks_alloc_lock();
		new_fl2 = locks_alloc_lock();
	}

	spin_lock(&ctx->flc_lock);
	/*
	 * New lock request. Walk all POSIX locks and look for conflicts. If
	 * there are any, either return error or put the request on the
	 * blocker's list of waiters and the global blocked_hash.
	 */
	if (request->fl_type != F_UNLCK) {
		list_for_each_entry(fl, &ctx->flc_posix, fl_list) {
			if (!posix_locks_conflict(request, fl))
				continue;
			if (conflock)
				locks_copy_conflock(conflock, fl);
			error = -EAGAIN;
			if (!(request->fl_flags & FL_SLEEP))
				goto out;
			/*
			 * Deadlock detection and insertion into the blocked
			 * locks list must be done while holding the same lock!
			 */
			error = -EDEADLK;
			spin_lock(&blocked_lock_lock);
			if (likely(!posix_locks_deadlock(request, fl))) {
				error = FILE_LOCK_DEFERRED;
				__locks_insert_block(fl, request);
			}
			spin_unlock(&blocked_lock_lock);
			goto out;
  		}
  	}

	/* If we're just looking for a conflict, we're done. */
	error = 0;
	if (request->fl_flags & FL_ACCESS)
		goto out;

	/* Find the first old lock with the same owner as the new lock */
	list_for_each_entry(fl, &ctx->flc_posix, fl_list) {
		if (posix_same_owner(request, fl))
			break;
	}

	/* Process locks with this owner. */
	list_for_each_entry_safe_from(fl, tmp, &ctx->flc_posix, fl_list) {
		if (!posix_same_owner(request, fl))
			break;

		/* Detect adjacent or overlapping regions (if same lock type) */
		if (request->fl_type == fl->fl_type) {
			/* In all comparisons of start vs end, use
			 * "start - 1" rather than "end + 1". If end
			 * is OFFSET_MAX, end + 1 will become negative.
			 */
			if (fl->fl_end < request->fl_start - 1)
				continue;
			/* If the next lock in the list has entirely bigger
			 * addresses than the new one, insert the lock here.
			 */
			if (fl->fl_start - 1 > request->fl_end)
				break;

			/* If we come here, the new and old lock are of the
			 * same type and adjacent or overlapping. Make one
			 * lock yielding from the lower start address of both
			 * locks to the higher end address.
			 */
			if (fl->fl_start > request->fl_start)
				fl->fl_start = request->fl_start;
			else
				request->fl_start = fl->fl_start;
			if (fl->fl_end < request->fl_end)
				fl->fl_end = request->fl_end;
			else
				request->fl_end = fl->fl_end;
			if (added) {
				locks_delete_lock_ctx(fl, &dispose);
				continue;
			}
			request = fl;
			added = true;
		} else {
			/* Processing for different lock types is a bit
			 * more complex.
			 */
			if (fl->fl_end < request->fl_start)
				continue;
			if (fl->fl_start > request->fl_end)
				break;
			if (request->fl_type == F_UNLCK)
				added = true;
			if (fl->fl_start < request->fl_start)
				left = fl;
			/* If the next lock in the list has a higher end
			 * address than the new one, insert the new one here.
			 */
			if (fl->fl_end > request->fl_end) {
				right = fl;
				break;
			}
			if (fl->fl_start >= request->fl_start) {
				/* The new lock completely replaces an old
				 * one (This may happen several times).
				 */
				if (added) {
					locks_delete_lock_ctx(fl, &dispose);
					continue;
				}
				/*
				 * Replace the old lock with new_fl, and
				 * remove the old one. It's safe to do the
				 * insert here since we know that we won't be
				 * using new_fl later, and that the lock is
				 * just replacing an existing lock.
				 */
				error = -ENOLCK;
				if (!new_fl)
					goto out;
				locks_copy_lock(new_fl, request);
				request = new_fl;
				new_fl = NULL;
				locks_insert_lock_ctx(request, &fl->fl_list);
				locks_delete_lock_ctx(fl, &dispose);
				added = true;
			}
		}
	}

	/*
	 * The above code only modifies existing locks in case of merging or
	 * replacing. If new lock(s) need to be inserted all modifications are
	 * done below this, so it's safe yet to bail out.
	 */
	error = -ENOLCK; /* "no luck" */
	if (right && left == right && !new_fl2)
		goto out;

	error = 0;
	if (!added) {
		if (request->fl_type == F_UNLCK) {
			if (request->fl_flags & FL_EXISTS)
				error = -ENOENT;
			goto out;
		}

		if (!new_fl) {
			error = -ENOLCK;
			goto out;
		}
		locks_copy_lock(new_fl, request);
		locks_insert_lock_ctx(new_fl, &fl->fl_list);
		fl = new_fl;
		new_fl = NULL;
	}
	if (right) {
		if (left == right) {
			/* The new lock breaks the old one in two pieces,
			 * so we have to use the second new lock.
			 */
			left = new_fl2;
			new_fl2 = NULL;
			locks_copy_lock(left, right);
			locks_insert_lock_ctx(left, &fl->fl_list);
		}
		right->fl_start = request->fl_end + 1;
		locks_wake_up_blocks(right);
	}
	if (left) {
		left->fl_end = request->fl_start - 1;
		locks_wake_up_blocks(left);
	}
 out:
	spin_unlock(&ctx->flc_lock);
	/*
	 * Free any unused locks.
	 */
	if (new_fl)
		locks_free_lock(new_fl);
	if (new_fl2)
		locks_free_lock(new_fl2);
	locks_dispose_list(&dispose);
	return error;
}

/**
 * posix_lock_file - Apply a POSIX-style lock to a file
 * @filp: The file to apply the lock to
 * @fl: The lock to be applied
 * @conflock: Place to return a copy of the conflicting lock, if found.
 *
 * Add a POSIX style lock to a file.
 * We merge adjacent & overlapping locks whenever possible.
 * POSIX locks are sorted by owner task, then by starting address
 *
 * Note that if called with an FL_EXISTS argument, the caller may determine
 * whether or not a lock was successfully freed by testing the return
 * value for -ENOENT.
 */
int posix_lock_file(struct file *filp, struct file_lock *fl,
			struct file_lock *conflock)
{
	return __posix_lock_file(file_inode(filp), fl, conflock);
}
EXPORT_SYMBOL(posix_lock_file);

/**
 * posix_lock_inode_wait - Apply a POSIX-style lock to a file
 * @inode: inode of file to which lock request should be applied
 * @fl: The lock to be applied
 *
 * Variant of posix_lock_file_wait that does not take a filp, and so can be
 * used after the filp has already been torn down.
 */
int posix_lock_inode_wait(struct inode *inode, struct file_lock *fl)
{
	int error;
	might_sleep ();
	for (;;) {
		error = __posix_lock_file(inode, fl, NULL);
		if (error != FILE_LOCK_DEFERRED)
			break;
		error = wait_event_interruptible(fl->fl_wait, !fl->fl_next);
		if (!error)
			continue;

		locks_delete_block(fl);
		break;
	}
	return error;
}
EXPORT_SYMBOL(posix_lock_inode_wait);

/**
 * locks_mandatory_locked - Check for an active lock
 * @file: the file to check
 *
 * Searches the inode's list of locks to find any POSIX locks which conflict.
 * This function is called from locks_verify_locked() only.
 */
int locks_mandatory_locked(struct file *file)
{
	int ret;
	struct inode *inode = file_inode(file);
	struct file_lock_context *ctx;
	struct file_lock *fl;

	ctx = inode->i_flctx;
	if (!ctx || list_empty_careful(&ctx->flc_posix))
		return 0;

	/*
	 * Search the lock list for this inode for any POSIX locks.
	 */
	spin_lock(&ctx->flc_lock);
	ret = 0;
	list_for_each_entry(fl, &ctx->flc_posix, fl_list) {
		if (fl->fl_owner != current->files &&
		    fl->fl_owner != file) {
			ret = -EAGAIN;
			break;
		}
	}
	spin_unlock(&ctx->flc_lock);
	return ret;
}

/**
 * locks_mandatory_area - Check for a conflicting lock
 * @read_write: %FLOCK_VERIFY_WRITE for exclusive access, %FLOCK_VERIFY_READ
 *		for shared
 * @inode:      the file to check
 * @filp:       how the file was opened (if it was)
 * @offset:     start of area to check
 * @count:      length of area to check
 *
 * Searches the inode's list of locks to find any POSIX locks which conflict.
 * This function is called from rw_verify_area() and
 * locks_verify_truncate().
 */
int locks_mandatory_area(int read_write, struct inode *inode,
			 struct file *filp, loff_t offset,
			 size_t count)
{
	struct file_lock fl;
	int error;
	bool sleep = false;

	locks_init_lock(&fl);
	fl.fl_pid = current->tgid;
	fl.fl_file = filp;
	fl.fl_flags = FL_POSIX | FL_ACCESS;
	if (filp && !(filp->f_flags & O_NONBLOCK))
		sleep = true;
	fl.fl_type = (read_write == FLOCK_VERIFY_WRITE) ? F_WRLCK : F_RDLCK;
	fl.fl_start = offset;
	fl.fl_end = offset + count - 1;

	for (;;) {
		if (filp) {
			fl.fl_owner = filp;
			fl.fl_flags &= ~FL_SLEEP;
			error = __posix_lock_file(inode, &fl, NULL);
			if (!error)
				break;
		}

		if (sleep)
			fl.fl_flags |= FL_SLEEP;
		fl.fl_owner = current->files;
		error = __posix_lock_file(inode, &fl, NULL);
		if (error != FILE_LOCK_DEFERRED)
			break;
		error = wait_event_interruptible(fl.fl_wait, !fl.fl_next);
		if (!error) {
			/*
			 * If we've been sleeping someone might have
			 * changed the permissions behind our back.
			 */
			if (__mandatory_lock(inode))
				continue;
		}

		locks_delete_block(&fl);
		break;
	}

	return error;
}

EXPORT_SYMBOL(locks_mandatory_area);

static void lease_clear_pending(struct file_lock *fl, int arg)
{
	switch (arg) {
	case F_UNLCK:
		fl->fl_flags &= ~FL_UNLOCK_PENDING;
		/* fall through: */
	case F_RDLCK:
		fl->fl_flags &= ~FL_DOWNGRADE_PENDING;
	}
}

/* We already had a lease on this file; just change its type */
int lease_modify(struct file_lock *fl, int arg, struct list_head *dispose)
{
	int error = assign_type(fl, arg);

	if (error)
		return error;
	lease_clear_pending(fl, arg);
	locks_wake_up_blocks(fl);
	if (arg == F_UNLCK) {
		struct file *filp = fl->fl_file;

		f_delown(filp);
		filp->f_owner.signum = 0;
		fasync_helper(0, fl->fl_file, 0, &fl->fl_fasync);
		if (fl->fl_fasync != NULL) {
			printk(KERN_ERR "locks_delete_lock: fasync == %p\n", fl->fl_fasync);
			fl->fl_fasync = NULL;
		}
		locks_delete_lock_ctx(fl, dispose);
	}
	return 0;
}
EXPORT_SYMBOL(lease_modify);

static bool past_time(unsigned long then)
{
	if (!then)
		/* 0 is a special value meaning "this never expires": */
		return false;
	return time_after(jiffies, then);
}

static void time_out_leases(struct inode *inode, struct list_head *dispose)
{
	struct file_lock_context *ctx = inode->i_flctx;
	struct file_lock *fl, *tmp;

	lockdep_assert_held(&ctx->flc_lock);

	list_for_each_entry_safe(fl, tmp, &ctx->flc_lease, fl_list) {
		trace_time_out_leases(inode, fl);
		if (past_time(fl->fl_downgrade_time))
			lease_modify(fl, F_RDLCK, dispose);
		if (past_time(fl->fl_break_time))
			lease_modify(fl, F_UNLCK, dispose);
	}
}

static bool leases_conflict(struct file_lock *lease, struct file_lock *breaker)
{
	if ((breaker->fl_flags & FL_LAYOUT) != (lease->fl_flags & FL_LAYOUT))
		return false;
	if ((breaker->fl_flags & FL_DELEG) && (lease->fl_flags & FL_LEASE))
		return false;
	return locks_conflict(breaker, lease);
}

static bool
any_leases_conflict(struct inode *inode, struct file_lock *breaker)
{
	struct file_lock_context *ctx = inode->i_flctx;
	struct file_lock *fl;

	lockdep_assert_held(&ctx->flc_lock);

	list_for_each_entry(fl, &ctx->flc_lease, fl_list) {
		if (leases_conflict(fl, breaker))
			return true;
	}
	return false;
}

/**
 *	__break_lease	-	revoke all outstanding leases on file
 *	@inode: the inode of the file to return
 *	@mode: O_RDONLY: break only write leases; O_WRONLY or O_RDWR:
 *	    break all leases
 *	@type: FL_LEASE: break leases and delegations; FL_DELEG: break
 *	    only delegations
 *
 *	break_lease (inlined for speed) has checked there already is at least
 *	some kind of lock (maybe a lease) on this file.  Leases are broken on
 *	a call to open() or truncate().  This function can sleep unless you
 *	specified %O_NONBLOCK to your open().
 */
int __break_lease(struct inode *inode, unsigned int mode, unsigned int type)
{
	int error = 0;
	struct file_lock_context *ctx = inode->i_flctx;
	struct file_lock *new_fl, *fl, *tmp;
	unsigned long break_time;
	int want_write = (mode & O_ACCMODE) != O_RDONLY;
	LIST_HEAD(dispose);

	new_fl = lease_alloc(NULL, want_write ? F_WRLCK : F_RDLCK);
	if (IS_ERR(new_fl))
		return PTR_ERR(new_fl);
	new_fl->fl_flags = type;

	/* typically we will check that ctx is non-NULL before calling */
	if (!ctx) {
		WARN_ON_ONCE(1);
		return error;
	}

	spin_lock(&ctx->flc_lock);

	time_out_leases(inode, &dispose);

	if (!any_leases_conflict(inode, new_fl))
		goto out;

	break_time = 0;
	if (lease_break_time > 0) {
		break_time = jiffies + lease_break_time * HZ;
		if (break_time == 0)
			break_time++;	/* so that 0 means no break time */
	}

	list_for_each_entry_safe(fl, tmp, &ctx->flc_lease, fl_list) {
		if (!leases_conflict(fl, new_fl))
			continue;
		if (want_write) {
			if (fl->fl_flags & FL_UNLOCK_PENDING)
				continue;
			fl->fl_flags |= FL_UNLOCK_PENDING;
			fl->fl_break_time = break_time;
		} else {
			if (lease_breaking(fl))
				continue;
			fl->fl_flags |= FL_DOWNGRADE_PENDING;
			fl->fl_downgrade_time = break_time;
		}
		if (fl->fl_lmops->lm_break(fl))
			locks_delete_lock_ctx(fl, &dispose);
	}

	if (list_empty(&ctx->flc_lease))
		goto out;

	if (mode & O_NONBLOCK) {
		trace_break_lease_noblock(inode, new_fl);
		error = -EWOULDBLOCK;
		goto out;
	}

restart:
	fl = list_first_entry(&ctx->flc_lease, struct file_lock, fl_list);
	break_time = fl->fl_break_time;
	if (break_time != 0)
		break_time -= jiffies;
	if (break_time == 0)
		break_time++;
	locks_insert_block(fl, new_fl);
	trace_break_lease_block(inode, new_fl);
	spin_unlock(&ctx->flc_lock);
	locks_dispose_list(&dispose);
	error = wait_event_interruptible_timeout(new_fl->fl_wait,
						!new_fl->fl_next, break_time);
	spin_lock(&ctx->flc_lock);
	trace_break_lease_unblock(inode, new_fl);
	locks_delete_block(new_fl);
	if (error >= 0) {
		/*
		 * Wait for the next conflicting lease that has not been
		 * broken yet
		 */
		if (error == 0)
			time_out_leases(inode, &dispose);
		if (any_leases_conflict(inode, new_fl))
			goto restart;
		error = 0;
	}
out:
	spin_unlock(&ctx->flc_lock);
	locks_dispose_list(&dispose);
	locks_free_lock(new_fl);
	return error;
}

EXPORT_SYMBOL(__break_lease);

/**
 *	lease_get_mtime - get the last modified time of an inode
 *	@inode: the inode
 *      @time:  pointer to a timespec which will contain the last modified time
 *
 * This is to force NFS clients to flush their caches for files with
 * exclusive leases.  The justification is that if someone has an
 * exclusive lease, then they could be modifying it.
 */
void lease_get_mtime(struct inode *inode, struct timespec *time)
{
	bool has_lease = false;
	struct file_lock_context *ctx = inode->i_flctx;
	struct file_lock *fl;

	if (ctx && !list_empty_careful(&ctx->flc_lease)) {
		spin_lock(&ctx->flc_lock);
		if (!list_empty(&ctx->flc_lease)) {
			fl = list_first_entry(&ctx->flc_lease,
						struct file_lock, fl_list);
			if (fl->fl_type == F_WRLCK)
				has_lease = true;
		}
		spin_unlock(&ctx->flc_lock);
	}

	if (has_lease)
		*time = current_fs_time(inode->i_sb);
	else
		*time = inode->i_mtime;
}

EXPORT_SYMBOL(lease_get_mtime);

/**
 *	fcntl_getlease - Enquire what lease is currently active
 *	@filp: the file
 *
 *	The value returned by this function will be one of
 *	(if no lease break is pending):
 *
 *	%F_RDLCK to indicate a shared lease is held.
 *
 *	%F_WRLCK to indicate an exclusive lease is held.
 *
 *	%F_UNLCK to indicate no lease is held.
 *
 *	(if a lease break is pending):
 *
 *	%F_RDLCK to indicate an exclusive lease needs to be
 *		changed to a shared lease (or removed).
 *
 *	%F_UNLCK to indicate the lease needs to be removed.
 *
 *	XXX: sfr & willy disagree over whether F_INPROGRESS
 *	should be returned to userspace.
 */
int fcntl_getlease(struct file *filp)
{
	struct file_lock *fl;
	struct inode *inode = file_inode(filp);
	struct file_lock_context *ctx = inode->i_flctx;
	int type = F_UNLCK;
	LIST_HEAD(dispose);

	if (ctx && !list_empty_careful(&ctx->flc_lease)) {
		spin_lock(&ctx->flc_lock);
		time_out_leases(file_inode(filp), &dispose);
		list_for_each_entry(fl, &ctx->flc_lease, fl_list) {
			if (fl->fl_file != filp)
				continue;
			type = target_leasetype(fl);
			break;
		}
		spin_unlock(&ctx->flc_lock);
		locks_dispose_list(&dispose);
	}
	return type;
}

/**
 * check_conflicting_open - see if the given dentry points to a file that has
 * 			    an existing open that would conflict with the
 * 			    desired lease.
 * @dentry:	dentry to check
 * @arg:	type of lease that we're trying to acquire
 *
 * Check to see if there's an existing open fd on this file that would
 * conflict with the lease we're trying to set.
 */
static int
check_conflicting_open(const struct dentry *dentry, const long arg, int flags)
{
	int ret = 0;
	struct inode *inode = dentry->d_inode;

	if (flags & FL_LAYOUT)
		return 0;

	if ((arg == F_RDLCK) && (atomic_read(&inode->i_writecount) > 0))
		return -EAGAIN;

	if ((arg == F_WRLCK) && ((d_count(dentry) > 1) ||
	    (atomic_read(&inode->i_count) > 1)))
		ret = -EAGAIN;

	return ret;
}

static int
generic_add_lease(struct file *filp, long arg, struct file_lock **flp, void **priv)
{
	struct file_lock *fl, *my_fl = NULL, *lease;
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = file_inode(filp);
	struct file_lock_context *ctx;
	bool is_deleg = (*flp)->fl_flags & FL_DELEG;
	int error;
	LIST_HEAD(dispose);

	lease = *flp;
	trace_generic_add_lease(inode, lease);

	/* Note that arg is never F_UNLCK here */
	ctx = locks_get_lock_context(inode, arg);
	if (!ctx)
		return -ENOMEM;

	/*
	 * In the delegation case we need mutual exclusion with
	 * a number of operations that take the i_mutex.  We trylock
	 * because delegations are an optional optimization, and if
	 * there's some chance of a conflict--we'd rather not
	 * bother, maybe that's a sign this just isn't a good file to
	 * hand out a delegation on.
	 */
	if (is_deleg && !mutex_trylock(&inode->i_mutex))
		return -EAGAIN;

	if (is_deleg && arg == F_WRLCK) {
		/* Write delegations are not currently supported: */
		mutex_unlock(&inode->i_mutex);
		WARN_ON_ONCE(1);
		return -EINVAL;
	}

	spin_lock(&ctx->flc_lock);
	time_out_leases(inode, &dispose);
	error = check_conflicting_open(dentry, arg, lease->fl_flags);
	if (error)
		goto out;

	/*
	 * At this point, we know that if there is an exclusive
	 * lease on this file, then we hold it on this filp
	 * (otherwise our open of this file would have blocked).
	 * And if we are trying to acquire an exclusive lease,
	 * then the file is not open by anyone (including us)
	 * except for this filp.
	 */
	error = -EAGAIN;
	list_for_each_entry(fl, &ctx->flc_lease, fl_list) {
		if (fl->fl_file == filp &&
		    fl->fl_owner == lease->fl_owner) {
			my_fl = fl;
			continue;
		}

		/*
		 * No exclusive leases if someone else has a lease on
		 * this file:
		 */
		if (arg == F_WRLCK)
			goto out;
		/*
		 * Modifying our existing lease is OK, but no getting a
		 * new lease if someone else is opening for write:
		 */
		if (fl->fl_flags & FL_UNLOCK_PENDING)
			goto out;
	}

	if (my_fl != NULL) {
		lease = my_fl;
		error = lease->fl_lmops->lm_change(lease, arg, &dispose);
		if (error)
			goto out;
		goto out_setup;
	}

	error = -EINVAL;
	if (!leases_enable)
		goto out;

	locks_insert_lock_ctx(lease, &ctx->flc_lease);
	/*
	 * The check in break_lease() is lockless. It's possible for another
	 * open to race in after we did the earlier check for a conflicting
	 * open but before the lease was inserted. Check again for a
	 * conflicting open and cancel the lease if there is one.
	 *
	 * We also add a barrier here to ensure that the insertion of the lock
	 * precedes these checks.
	 */
	smp_mb();
	error = check_conflicting_open(dentry, arg, lease->fl_flags);
	if (error) {
		locks_unlink_lock_ctx(lease);
		goto out;
	}

out_setup:
	if (lease->fl_lmops->lm_setup)
		lease->fl_lmops->lm_setup(lease, priv);
out:
	spin_unlock(&ctx->flc_lock);
	locks_dispose_list(&dispose);
	if (is_deleg)
		mutex_unlock(&inode->i_mutex);
	if (!error && !my_fl)
		*flp = NULL;
	return error;
}

static int generic_delete_lease(struct file *filp, void *owner)
{
	int error = -EAGAIN;
	struct file_lock *fl, *victim = NULL;
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	struct file_lock_context *ctx = inode->i_flctx;
	LIST_HEAD(dispose);

	if (!ctx) {
		trace_generic_delete_lease(inode, NULL);
		return error;
	}

	spin_lock(&ctx->flc_lock);
	list_for_each_entry(fl, &ctx->flc_lease, fl_list) {
		if (fl->fl_file == filp &&
		    fl->fl_owner == owner) {
			victim = fl;
			break;
		}
	}
	trace_generic_delete_lease(inode, victim);
	if (victim)
		error = fl->fl_lmops->lm_change(victim, F_UNLCK, &dispose);
	spin_unlock(&ctx->flc_lock);
	locks_dispose_list(&dispose);
	return error;
}

/**
 *	generic_setlease	-	sets a lease on an open file
 *	@filp:	file pointer
 *	@arg:	type of lease to obtain
 *	@flp:	input - file_lock to use, output - file_lock inserted
 *	@priv:	private data for lm_setup (may be NULL if lm_setup
 *		doesn't require it)
 *
 *	The (input) flp->fl_lmops->lm_break function is required
 *	by break_lease().
 */
int generic_setlease(struct file *filp, long arg, struct file_lock **flp,
			void **priv)
{
	struct dentry *dentry = filp->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	int error;

	if ((!uid_eq(current_fsuid(), inode->i_uid)) && !capable(CAP_LEASE))
		return -EACCES;
	if (!S_ISREG(inode->i_mode))
		return -EINVAL;
	error = security_file_lock(filp, arg);
	if (error)
		return error;

	switch (arg) {
	case F_UNLCK:
		return generic_delete_lease(filp, *priv);
	case F_RDLCK:
	case F_WRLCK:
		if (!(*flp)->fl_lmops->lm_break) {
			WARN_ON_ONCE(1);
			return -ENOLCK;
		}

		return generic_add_lease(filp, arg, flp, priv);
	default:
		return -EINVAL;
	}
}
EXPORT_SYMBOL(generic_setlease);

/**
 * vfs_setlease        -       sets a lease on an open file
 * @filp:	file pointer
 * @arg:	type of lease to obtain
 * @lease:	file_lock to use when adding a lease
 * @priv:	private info for lm_setup when adding a lease (may be
 * 		NULL if lm_setup doesn't require it)
 *
 * Call this to establish a lease on the file. The "lease" argument is not
 * used for F_UNLCK requests and may be NULL. For commands that set or alter
 * an existing lease, the (*lease)->fl_lmops->lm_break operation must be set;
 * if not, this function will return -ENOLCK (and generate a scary-looking
 * stack trace).
 *
 * The "priv" pointer is passed directly to the lm_setup function as-is. It
 * may be NULL if the lm_setup operation doesn't require it.
 */
int
vfs_setlease(struct file *filp, long arg, struct file_lock **lease, void **priv)
{
	if (filp->f_op->setlease)
		return filp->f_op->setlease(filp, arg, lease, priv);
	else
		return generic_setlease(filp, arg, lease, priv);
}
EXPORT_SYMBOL_GPL(vfs_setlease);

static int do_fcntl_add_lease(unsigned int fd, struct file *filp, long arg)
{
	struct file_lock *fl;
	struct fasync_struct *new;
	int error;

	fl = lease_alloc(filp, arg);
	if (IS_ERR(fl))
		return PTR_ERR(fl);

	new = fasync_alloc();
	if (!new) {
		locks_free_lock(fl);
		return -ENOMEM;
	}
	new->fa_fd = fd;

	error = vfs_setlease(filp, arg, &fl, (void **)&new);
	if (fl)
		locks_free_lock(fl);
	if (new)
		fasync_free(new);
	return error;
}

/**
 *	fcntl_setlease	-	sets a lease on an open file
 *	@fd: open file descriptor
 *	@filp: file pointer
 *	@arg: type of lease to obtain
 *
 *	Call this fcntl to establish a lease on the file.
 *	Note that you also need to call %F_SETSIG to
 *	receive a signal when the lease is broken.
 */
int fcntl_setlease(unsigned int fd, struct file *filp, long arg)
{
	if (arg == F_UNLCK)
		return vfs_setlease(filp, F_UNLCK, NULL, (void **)&filp);
	return do_fcntl_add_lease(fd, filp, arg);
}

/**
 * flock_lock_inode_wait - Apply a FLOCK-style lock to a file
 * @inode: inode of the file to apply to
 * @fl: The lock to be applied
 *
 * Apply a FLOCK style lock request to an inode.
 */
int flock_lock_inode_wait(struct inode *inode, struct file_lock *fl)
{
	int error;
	might_sleep();
	for (;;) {
		error = flock_lock_inode(inode, fl);
		if (error != FILE_LOCK_DEFERRED)
			break;
		error = wait_event_interruptible(fl->fl_wait, !fl->fl_next);
		if (!error)
			continue;

		locks_delete_block(fl);
		break;
	}
	return error;
}
EXPORT_SYMBOL(flock_lock_inode_wait);

/**
 *	sys_flock: - flock() system call.
 *	@fd: the file descriptor to lock.
 *	@cmd: the type of lock to apply.
 *
 *	Apply a %FL_FLOCK style lock to an open file descriptor.
 *	The @cmd can be one of
 *
 *	%LOCK_SH -- a shared lock.
 *
 *	%LOCK_EX -- an exclusive lock.
 *
 *	%LOCK_UN -- remove an existing lock.
 *
 *	%LOCK_MAND -- a `mandatory' flock.  This exists to emulate Windows Share Modes.
 *
 *	%LOCK_MAND can be combined with %LOCK_READ or %LOCK_WRITE to allow other
 *	processes read and write access respectively.
 */
SYSCALL_DEFINE2(flock, unsigned int, fd, unsigned int, cmd)
{
	struct fd f = fdget(fd);
	struct file_lock *lock;
	int can_sleep, unlock;
	int error;

	error = -EBADF;
	if (!f.file)
		goto out;

	can_sleep = !(cmd & LOCK_NB);
	cmd &= ~LOCK_NB;
	unlock = (cmd == LOCK_UN);

	if (!unlock && !(cmd & LOCK_MAND) &&
	    !(f.file->f_mode & (FMODE_READ|FMODE_WRITE)))
		goto out_putf;

	lock = flock_make_lock(f.file, cmd);
	if (IS_ERR(lock)) {
		error = PTR_ERR(lock);
		goto out_putf;
	}

	if (can_sleep)
		lock->fl_flags |= FL_SLEEP;

	error = security_file_lock(f.file, lock->fl_type);
	if (error)
		goto out_free;

	if (f.file->f_op->flock)
		error = f.file->f_op->flock(f.file,
					  (can_sleep) ? F_SETLKW : F_SETLK,
					  lock);
	else
		error = flock_lock_file_wait(f.file, lock);

 out_free:
	locks_free_lock(lock);

 out_putf:
	fdput(f);
 out:
	return error;
}

/**
 * vfs_test_lock - test file byte range lock
 * @filp: The file to test lock for
 * @fl: The lock to test; also used to hold result
 *
 * Returns -ERRNO on failure.  Indicates presence of conflicting lock by
 * setting conf->fl_type to something other than F_UNLCK.
 */
int vfs_test_lock(struct file *filp, struct file_lock *fl)
{
	if (filp->f_op->lock)
		return filp->f_op->lock(filp, F_GETLK, fl);
	posix_test_lock(filp, fl);
	return 0;
}
EXPORT_SYMBOL_GPL(vfs_test_lock);

static int posix_lock_to_flock(struct flock *flock, struct file_lock *fl)
{
	flock->l_pid = IS_OFDLCK(fl) ? -1 : fl->fl_pid;
#if BITS_PER_LONG == 32
	/*
	 * Make sure we can represent the posix lock via
	 * legacy 32bit flock.
	 */
	if (fl->fl_start > OFFT_OFFSET_MAX)
		return -EOVERFLOW;
	if (fl->fl_end != OFFSET_MAX && fl->fl_end > OFFT_OFFSET_MAX)
		return -EOVERFLOW;
#endif
	flock->l_start = fl->fl_start;
	flock->l_len = fl->fl_end == OFFSET_MAX ? 0 :
		fl->fl_end - fl->fl_start + 1;
	flock->l_whence = 0;
	flock->l_type = fl->fl_type;
	return 0;
}

#if BITS_PER_LONG == 32
static void posix_lock_to_flock64(struct flock64 *flock, struct file_lock *fl)
{
	flock->l_pid = IS_OFDLCK(fl) ? -1 : fl->fl_pid;
	flock->l_start = fl->fl_start;
	flock->l_len = fl->fl_end == OFFSET_MAX ? 0 :
		fl->fl_end - fl->fl_start + 1;
	flock->l_whence = 0;
	flock->l_type = fl->fl_type;
}
#endif

/* Report the first existing lock that would conflict with l.
 * This implements the F_GETLK command of fcntl().
 */
int fcntl_getlk(struct file *filp, unsigned int cmd, struct flock __user *l)
{
	struct file_lock file_lock;
	struct flock flock;
	int error;

	error = -EFAULT;
	if (copy_from_user(&flock, l, sizeof(flock)))
		goto out;
	error = -EINVAL;
	if ((flock.l_type != F_RDLCK) && (flock.l_type != F_WRLCK))
		goto out;

	error = flock_to_posix_lock(filp, &file_lock, &flock);
	if (error)
		goto out;

	if (cmd == F_OFD_GETLK) {
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_GETLK;
		file_lock.fl_flags |= FL_OFDLCK;
		file_lock.fl_owner = filp;
	}

	error = vfs_test_lock(filp, &file_lock);
	if (error)
		goto out;
 
	flock.l_type = file_lock.fl_type;
	if (file_lock.fl_type != F_UNLCK) {
		error = posix_lock_to_flock(&flock, &file_lock);
		if (error)
			goto rel_priv;
	}
	error = -EFAULT;
	if (!copy_to_user(l, &flock, sizeof(flock)))
		error = 0;
rel_priv:
	locks_release_private(&file_lock);
out:
	return error;
}

/**
 * vfs_lock_file - file byte range lock
 * @filp: The file to apply the lock to
 * @cmd: type of locking operation (F_SETLK, F_GETLK, etc.)
 * @fl: The lock to be applied
 * @conf: Place to return a copy of the conflicting lock, if found.
 *
 * A caller that doesn't care about the conflicting lock may pass NULL
 * as the final argument.
 *
 * If the filesystem defines a private ->lock() method, then @conf will
 * be left unchanged; so a caller that cares should initialize it to
 * some acceptable default.
 *
 * To avoid blocking kernel daemons, such as lockd, that need to acquire POSIX
 * locks, the ->lock() interface may return asynchronously, before the lock has
 * been granted or denied by the underlying filesystem, if (and only if)
 * lm_grant is set. Callers expecting ->lock() to return asynchronously
 * will only use F_SETLK, not F_SETLKW; they will set FL_SLEEP if (and only if)
 * the request is for a blocking lock. When ->lock() does return asynchronously,
 * it must return FILE_LOCK_DEFERRED, and call ->lm_grant() when the lock
 * request completes.
 * If the request is for non-blocking lock the file system should return
 * FILE_LOCK_DEFERRED then try to get the lock and call the callback routine
 * with the result. If the request timed out the callback routine will return a
 * nonzero return code and the file system should release the lock. The file
 * system is also responsible to keep a corresponding posix lock when it
 * grants a lock so the VFS can find out which locks are locally held and do
 * the correct lock cleanup when required.
 * The underlying filesystem must not drop the kernel lock or call
 * ->lm_grant() before returning to the caller with a FILE_LOCK_DEFERRED
 * return code.
 */
int vfs_lock_file(struct file *filp, unsigned int cmd, struct file_lock *fl, struct file_lock *conf)
{
	if (filp->f_op->lock)
		return filp->f_op->lock(filp, cmd, fl);
	else
		return posix_lock_file(filp, fl, conf);
}
EXPORT_SYMBOL_GPL(vfs_lock_file);

static int do_lock_file_wait(struct file *filp, unsigned int cmd,
			     struct file_lock *fl)
{
	int error;

	error = security_file_lock(filp, fl->fl_type);
	if (error)
		return error;

	for (;;) {
		error = vfs_lock_file(filp, cmd, fl, NULL);
		if (error != FILE_LOCK_DEFERRED)
			break;
		error = wait_event_interruptible(fl->fl_wait, !fl->fl_next);
		if (!error)
			continue;

		locks_delete_block(fl);
		break;
	}

	return error;
}

/* Ensure that fl->fl_filp has compatible f_mode for F_SETLK calls */
static int
check_fmode_for_setlk(struct file_lock *fl)
{
	switch (fl->fl_type) {
	case F_RDLCK:
		if (!(fl->fl_file->f_mode & FMODE_READ))
			return -EBADF;
		break;
	case F_WRLCK:
		if (!(fl->fl_file->f_mode & FMODE_WRITE))
			return -EBADF;
	}
	return 0;
}

/* Apply the lock described by l to an open file descriptor.
 * This implements both the F_SETLK and F_SETLKW commands of fcntl().
 */
int fcntl_setlk(unsigned int fd, struct file *filp, unsigned int cmd,
		struct flock __user *l)
{
	struct file_lock *file_lock = locks_alloc_lock();
	struct flock flock;
	struct inode *inode;
	struct file *f;
	int error;

	if (file_lock == NULL)
		return -ENOLCK;

	/*
	 * This might block, so we do it before checking the inode.
	 */
	error = -EFAULT;
	if (copy_from_user(&flock, l, sizeof(flock)))
		goto out;

	inode = file_inode(filp);

	/* Don't allow mandatory locks on files that may be memory mapped
	 * and shared.
	 */
	if (mandatory_lock(inode) && mapping_writably_mapped(filp->f_mapping)) {
		error = -EAGAIN;
		goto out;
	}

	error = flock_to_posix_lock(filp, file_lock, &flock);
	if (error)
		goto out;

	error = check_fmode_for_setlk(file_lock);
	if (error)
		goto out;

	/*
	 * If the cmd is requesting file-private locks, then set the
	 * FL_OFDLCK flag and override the owner.
	 */
	switch (cmd) {
	case F_OFD_SETLK:
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_SETLK;
		file_lock->fl_flags |= FL_OFDLCK;
		file_lock->fl_owner = filp;
		break;
	case F_OFD_SETLKW:
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_SETLKW;
		file_lock->fl_flags |= FL_OFDLCK;
		file_lock->fl_owner = filp;
		/* Fallthrough */
	case F_SETLKW:
		file_lock->fl_flags |= FL_SLEEP;
	}

	error = do_lock_file_wait(filp, cmd, file_lock);

	/*
	 * Attempt to detect a close/fcntl race and recover by releasing the
	 * lock that was just acquired. There is no need to do that when we're
	 * unlocking though, or for OFD locks.
	 */
	if (!error && file_lock->fl_type != F_UNLCK &&
	    !(file_lock->fl_flags & FL_OFDLCK)) {
		/*
		 * We need that spin_lock here - it prevents reordering between
		 * update of i_flctx->flc_posix and check for it done in
		 * close(). rcu_read_lock() wouldn't do.
		 */
		spin_lock(&current->files->file_lock);
		f = fcheck(fd);
		spin_unlock(&current->files->file_lock);
		if (f != filp) {
			file_lock->fl_type = F_UNLCK;
			error = do_lock_file_wait(filp, cmd, file_lock);
			WARN_ON_ONCE(error);
			error = -EBADF;
		}
	}
out:
	locks_free_lock(file_lock);
	return error;
}

#if BITS_PER_LONG == 32
/* Report the first existing lock that would conflict with l.
 * This implements the F_GETLK command of fcntl().
 */
int fcntl_getlk64(struct file *filp, unsigned int cmd, struct flock64 __user *l)
{
	struct file_lock file_lock;
	struct flock64 flock;
	int error;

	error = -EFAULT;
	if (copy_from_user(&flock, l, sizeof(flock)))
		goto out;
	error = -EINVAL;
	if ((flock.l_type != F_RDLCK) && (flock.l_type != F_WRLCK))
		goto out;

	error = flock64_to_posix_lock(filp, &file_lock, &flock);
	if (error)
		goto out;

	if (cmd == F_OFD_GETLK) {
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_GETLK64;
		file_lock.fl_flags |= FL_OFDLCK;
		file_lock.fl_owner = filp;
	}

	error = vfs_test_lock(filp, &file_lock);
	if (error)
		goto out;

	flock.l_type = file_lock.fl_type;
	if (file_lock.fl_type != F_UNLCK)
		posix_lock_to_flock64(&flock, &file_lock);

	error = -EFAULT;
	if (!copy_to_user(l, &flock, sizeof(flock)))
		error = 0;

	locks_release_private(&file_lock);
out:
	return error;
}

/* Apply the lock described by l to an open file descriptor.
 * This implements both the F_SETLK and F_SETLKW commands of fcntl().
 */
int fcntl_setlk64(unsigned int fd, struct file *filp, unsigned int cmd,
		struct flock64 __user *l)
{
	struct file_lock *file_lock = locks_alloc_lock();
	struct flock64 flock;
	struct inode *inode;
	struct file *f;
	int error;

	if (file_lock == NULL)
		return -ENOLCK;

	/*
	 * This might block, so we do it before checking the inode.
	 */
	error = -EFAULT;
	if (copy_from_user(&flock, l, sizeof(flock)))
		goto out;

	inode = file_inode(filp);

	/* Don't allow mandatory locks on files that may be memory mapped
	 * and shared.
	 */
	if (mandatory_lock(inode) && mapping_writably_mapped(filp->f_mapping)) {
		error = -EAGAIN;
		goto out;
	}

	error = flock64_to_posix_lock(filp, file_lock, &flock);
	if (error)
		goto out;

	error = check_fmode_for_setlk(file_lock);
	if (error)
		goto out;

	/*
	 * If the cmd is requesting file-private locks, then set the
	 * FL_OFDLCK flag and override the owner.
	 */
	switch (cmd) {
	case F_OFD_SETLK:
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_SETLK64;
		file_lock->fl_flags |= FL_OFDLCK;
		file_lock->fl_owner = filp;
		break;
	case F_OFD_SETLKW:
		error = -EINVAL;
		if (flock.l_pid != 0)
			goto out;

		cmd = F_SETLKW64;
		file_lock->fl_flags |= FL_OFDLCK;
		file_lock->fl_owner = filp;
		/* Fallthrough */
	case F_SETLKW64:
		file_lock->fl_flags |= FL_SLEEP;
	}

	error = do_lock_file_wait(filp, cmd, file_lock);

	/*
	 * Attempt to detect a close/fcntl race and recover by releasing the
	 * lock that was just acquired. There is no need to do that when we're
	 * unlocking though, or for OFD locks.
	 */
	if (!error && file_lock->fl_type != F_UNLCK &&
	    !(file_lock->fl_flags & FL_OFDLCK)) {
		/*
		 * We need that spin_lock here - it prevents reordering between
		 * update of i_flctx->flc_posix and check for it done in
		 * close(). rcu_read_lock() wouldn't do.
		 */
		spin_lock(&current->files->file_lock);
		f = fcheck(fd);
		spin_unlock(&current->files->file_lock);
		if (f != filp) {
			file_lock->fl_type = F_UNLCK;
			error = do_lock_file_wait(filp, cmd, file_lock);
			WARN_ON_ONCE(error);
			error = -EBADF;
		}
	}
out:
	locks_free_lock(file_lock);
	return error;
}
#endif /* BITS_PER_LONG == 32 */

/*
 * This function is called when the file is being removed
 * from the task's fd array.  POSIX locks belonging to this task
 * are deleted at this time.
 */
void locks_remove_posix(struct file *filp, fl_owner_t owner)
{
	struct file_lock lock;
	struct file_lock_context *ctx = file_inode(filp)->i_flctx;

	/*
	 * If there are no locks held on this file, we don't need to call
	 * posix_lock_file().  Another process could be setting a lock on this
	 * file at the same time, but we wouldn't remove that lock anyway.
	 */
	if (!ctx || list_empty(&ctx->flc_posix))
		return;

	lock.fl_type = F_UNLCK;
	lock.fl_flags = FL_POSIX | FL_CLOSE;
	lock.fl_start = 0;
	lock.fl_end = OFFSET_MAX;
	lock.fl_owner = owner;
	lock.fl_pid = current->tgid;
	lock.fl_file = filp;
	lock.fl_ops = NULL;
	lock.fl_lmops = NULL;

	vfs_lock_file(filp, F_SETLK, &lock, NULL);

	if (lock.fl_ops && lock.fl_ops->fl_release_private)
		lock.fl_ops->fl_release_private(&lock);
}

EXPORT_SYMBOL(locks_remove_posix);

/* The i_flctx must be valid when calling into here */
static void
locks_remove_flock(struct file *filp)
{
	struct file_lock fl = {
		.fl_owner = filp,
		.fl_pid = current->tgid,
		.fl_file = filp,
		.fl_flags = FL_FLOCK,
		.fl_type = F_UNLCK,
		.fl_end = OFFSET_MAX,
	};
	struct inode *inode = file_inode(filp);
	struct file_lock_context *flctx = inode->i_flctx;

	if (list_empty(&flctx->flc_flock))
		return;

	if (filp->f_op->flock)
		filp->f_op->flock(filp, F_SETLKW, &fl);
	else
		flock_lock_inode(inode, &fl);

	if (fl.fl_ops && fl.fl_ops->fl_release_private)
		fl.fl_ops->fl_release_private(&fl);
}

/* The i_flctx must be valid when calling into here */
static void
locks_remove_lease(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	struct file_lock_context *ctx = inode->i_flctx;
	struct file_lock *fl, *tmp;
	LIST_HEAD(dispose);

	if (list_empty(&ctx->flc_lease))
		return;

	spin_lock(&ctx->flc_lock);
	list_for_each_entry_safe(fl, tmp, &ctx->flc_lease, fl_list)
		if (filp == fl->fl_file)
			lease_modify(fl, F_UNLCK, &dispose);
	spin_unlock(&ctx->flc_lock);
	locks_dispose_list(&dispose);
}

/*
 * This function is called on the last close of an open file.
 */
void locks_remove_file(struct file *filp)
{
	if (!file_inode(filp)->i_flctx)
		return;

	/* remove any OFD locks */
	locks_remove_posix(filp, filp);

	/* remove flock locks */
	locks_remove_flock(filp);

	/* remove any leases */
	locks_remove_lease(filp);
}

/**
 *	posix_unblock_lock - stop waiting for a file lock
 *	@waiter: the lock which was waiting
 *
 *	lockd needs to block waiting for locks.
 */
int
posix_unblock_lock(struct file_lock *waiter)
{
	int status = 0;

	spin_lock(&blocked_lock_lock);
	if (waiter->fl_next)
		__locks_delete_block(waiter);
	else
		status = -ENOENT;
	spin_unlock(&blocked_lock_lock);
	return status;
}
EXPORT_SYMBOL(posix_unblock_lock);

/**
 * vfs_cancel_lock - file byte range unblock lock
 * @filp: The file to apply the unblock to
 * @fl: The lock to be unblocked
 *
 * Used by lock managers to cancel blocked requests
 */
int vfs_cancel_lock(struct file *filp, struct file_lock *fl)
{
	if (filp->f_op->lock)
		return filp->f_op->lock(filp, F_CANCELLK, fl);
	return 0;
}

EXPORT_SYMBOL_GPL(vfs_cancel_lock);

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

struct locks_iterator {
	int	li_cpu;
	loff_t	li_pos;
};

static void lock_get_status(struct seq_file *f, struct file_lock *fl,
			    loff_t id, char *pfx)
{
	struct inode *inode = NULL;
	unsigned int fl_pid;

	if (fl->fl_nspid)
		fl_pid = pid_vnr(fl->fl_nspid);
	else
		fl_pid = fl->fl_pid;

	if (fl->fl_file != NULL)
		inode = file_inode(fl->fl_file);

	seq_printf(f, "%lld:%s ", id, pfx);
	if (IS_POSIX(fl)) {
		if (fl->fl_flags & FL_ACCESS)
			seq_puts(f, "ACCESS");
		else if (IS_OFDLCK(fl))
			seq_puts(f, "OFDLCK");
		else
			seq_puts(f, "POSIX ");

		seq_printf(f, " %s ",
			     (inode == NULL) ? "*NOINODE*" :
			     mandatory_lock(inode) ? "MANDATORY" : "ADVISORY ");
	} else if (IS_FLOCK(fl)) {
		if (fl->fl_type & LOCK_MAND) {
			seq_puts(f, "FLOCK  MSNFS     ");
		} else {
			seq_puts(f, "FLOCK  ADVISORY  ");
		}
	} else if (IS_LEASE(fl)) {
		if (fl->fl_flags & FL_DELEG)
			seq_puts(f, "DELEG  ");
		else
			seq_puts(f, "LEASE  ");

		if (lease_breaking(fl))
			seq_puts(f, "BREAKING  ");
		else if (fl->fl_file)
			seq_puts(f, "ACTIVE    ");
		else
			seq_puts(f, "BREAKER   ");
	} else {
		seq_puts(f, "UNKNOWN UNKNOWN  ");
	}
	if (fl->fl_type & LOCK_MAND) {
		seq_printf(f, "%s ",
			       (fl->fl_type & LOCK_READ)
			       ? (fl->fl_type & LOCK_WRITE) ? "RW   " : "READ "
			       : (fl->fl_type & LOCK_WRITE) ? "WRITE" : "NONE ");
	} else {
		seq_printf(f, "%s ",
			       (lease_breaking(fl))
			       ? (fl->fl_type == F_UNLCK) ? "UNLCK" : "READ "
			       : (fl->fl_type == F_WRLCK) ? "WRITE" : "READ ");
	}
	if (inode) {
		/* userspace relies on this representation of dev_t */
		seq_printf(f, "%d %02x:%02x:%ld ", fl_pid,
				MAJOR(inode->i_sb->s_dev),
				MINOR(inode->i_sb->s_dev), inode->i_ino);
	} else {
		seq_printf(f, "%d <none>:0 ", fl_pid);
	}
	if (IS_POSIX(fl)) {
		if (fl->fl_end == OFFSET_MAX)
			seq_printf(f, "%Ld EOF\n", fl->fl_start);
		else
			seq_printf(f, "%Ld %Ld\n", fl->fl_start, fl->fl_end);
	} else {
		seq_puts(f, "0 EOF\n");
	}
}

static int locks_show(struct seq_file *f, void *v)
{
	struct locks_iterator *iter = f->private;
	struct file_lock *fl, *bfl;

	fl = hlist_entry(v, struct file_lock, fl_link);

	lock_get_status(f, fl, iter->li_pos, "");

	list_for_each_entry(bfl, &fl->fl_block, fl_block)
		lock_get_status(f, bfl, iter->li_pos, " ->");

	return 0;
}

static void __show_fd_locks(struct seq_file *f,
			struct list_head *head, int *id,
			struct file *filp, struct files_struct *files)
{
	struct file_lock *fl;

	list_for_each_entry(fl, head, fl_list) {

		if (filp != fl->fl_file)
			continue;
		if (fl->fl_owner != files &&
		    fl->fl_owner != filp)
			continue;

		(*id)++;
		seq_puts(f, "lock:\t");
		lock_get_status(f, fl, *id, "");
	}
}

void show_fd_locks(struct seq_file *f,
		  struct file *filp, struct files_struct *files)
{
	struct inode *inode = file_inode(filp);
	struct file_lock_context *ctx;
	int id = 0;

	ctx = inode->i_flctx;
	if (!ctx)
		return;

	spin_lock(&ctx->flc_lock);
	__show_fd_locks(f, &ctx->flc_flock, &id, filp, files);
	__show_fd_locks(f, &ctx->flc_posix, &id, filp, files);
	__show_fd_locks(f, &ctx->flc_lease, &id, filp, files);
	spin_unlock(&ctx->flc_lock);
}

static void *locks_start(struct seq_file *f, loff_t *pos)
	__acquires(&blocked_lock_lock)
{
	struct locks_iterator *iter = f->private;

	iter->li_pos = *pos + 1;
	lg_global_lock(&file_lock_lglock);
	spin_lock(&blocked_lock_lock);
	return seq_hlist_start_percpu(&file_lock_list, &iter->li_cpu, *pos);
}

static void *locks_next(struct seq_file *f, void *v, loff_t *pos)
{
	struct locks_iterator *iter = f->private;

	++iter->li_pos;
	return seq_hlist_next_percpu(v, &file_lock_list, &iter->li_cpu, pos);
}

static void locks_stop(struct seq_file *f, void *v)
	__releases(&blocked_lock_lock)
{
	spin_unlock(&blocked_lock_lock);
	lg_global_unlock(&file_lock_lglock);
}

static const struct seq_operations locks_seq_operations = {
	.start	= locks_start,
	.next	= locks_next,
	.stop	= locks_stop,
	.show	= locks_show,
};

static int locks_open(struct inode *inode, struct file *filp)
{
	return seq_open_private(filp, &locks_seq_operations,
					sizeof(struct locks_iterator));
}

static const struct file_operations proc_locks_operations = {
	.open		= locks_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};

static int __init proc_locks_init(void)
{
	proc_create("locks", 0, NULL, &proc_locks_operations);
	return 0;
}
module_init(proc_locks_init);
#endif

static int __init filelock_init(void)
{
	int i;

	flctx_cache = kmem_cache_create("file_lock_ctx",
			sizeof(struct file_lock_context), 0, SLAB_PANIC, NULL);

	filelock_cache = kmem_cache_create("file_lock_cache",
			sizeof(struct file_lock), 0, SLAB_PANIC, NULL);

	lg_lock_init(&file_lock_lglock, "file_lock_lglock");

	for_each_possible_cpu(i)
		INIT_HLIST_HEAD(per_cpu_ptr(&file_lock_list, i));

	return 0;
}

core_initcall(filelock_init);
