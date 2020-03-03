/*
 * Pid namespaces
 *
 * Authors:
 *    (C) 2007 Pavel Emelyanov <xemul@openvz.org>, OpenVZ, SWsoft Inc.
 *    (C) 2007 Sukadev Bhattiprolu <sukadev@us.ibm.com>, IBM
 *     Many thanks to Oleg Nesterov for comments and help
 *
 */

#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/user_namespace.h>
#include <linux/syscalls.h>
#include <linux/err.h>
#include <linux/acct.h>
#include <linux/slab.h>
#include <linux/proc_ns.h>
#include <linux/reboot.h>
#include <linux/export.h>

struct pid_cache {
	int nr_ids;
	char name[16];
	struct kmem_cache *cachep;
	struct list_head list;
};

static LIST_HEAD(pid_caches_lh);
static DEFINE_MUTEX(pid_caches_mutex);
static struct kmem_cache *pid_ns_cachep;

/*
 * creates the kmem cache to allocate pids from.
 * @nr_ids: the number of numerical ids this pid will have to carry
 */

static struct kmem_cache *create_pid_cachep(int nr_ids)
{
	struct pid_cache *pcache;
	struct kmem_cache *cachep;

	mutex_lock(&pid_caches_mutex);
	list_for_each_entry(pcache, &pid_caches_lh, list)
		if (pcache->nr_ids == nr_ids)
			goto out;

	pcache = kmalloc(sizeof(struct pid_cache), GFP_KERNEL);
	if (pcache == NULL)
		goto err_alloc;

	snprintf(pcache->name, sizeof(pcache->name), "pid_%d", nr_ids);
	cachep = kmem_cache_create(pcache->name,
			sizeof(struct pid) + (nr_ids - 1) * sizeof(struct upid),
			0, SLAB_HWCACHE_ALIGN, NULL);
	if (cachep == NULL)
		goto err_cachep;

	pcache->nr_ids = nr_ids;
	pcache->cachep = cachep;
	list_add(&pcache->list, &pid_caches_lh);
out:
	mutex_unlock(&pid_caches_mutex);
	return pcache->cachep;

err_cachep:
	kfree(pcache);
err_alloc:
	mutex_unlock(&pid_caches_mutex);
	return NULL;
}

static void proc_cleanup_work(struct work_struct *work)
{
	struct pid_namespace *ns = container_of(work, struct pid_namespace, proc_work);
	pid_ns_release_proc(ns);
}

/* MAX_PID_NS_LEVEL is needed for limiting size of 'struct pid' */
#define MAX_PID_NS_LEVEL 32

static struct pid_namespace *create_pid_namespace(struct user_namespace *user_ns,
	struct pid_namespace *parent_pid_ns)
{
	struct pid_namespace *ns;
	unsigned int level = parent_pid_ns->level + 1;
	int i;
	int err;

	if (level > MAX_PID_NS_LEVEL) {
		err = -EINVAL;
		goto out;
	}

	err = -ENOMEM;
	ns = kmem_cache_zalloc(pid_ns_cachep, GFP_KERNEL);
	if (ns == NULL)
		goto out;

	ns->pidmap[0].page = kzalloc(PAGE_SIZE, GFP_KERNEL);
	if (!ns->pidmap[0].page)
		goto out_free;

	ns->pid_cachep = create_pid_cachep(level + 1);
	if (ns->pid_cachep == NULL)
		goto out_free_map;

	err = ns_alloc_inum(&ns->ns);
	if (err)
		goto out_free_map;
	ns->ns.ops = &pidns_operations;

	kref_init(&ns->kref);
	ns->level = level;
	ns->parent = get_pid_ns(parent_pid_ns);
	ns->user_ns = get_user_ns(user_ns);
	ns->nr_hashed = PIDNS_HASH_ADDING;
	INIT_WORK(&ns->proc_work, proc_cleanup_work);

	set_bit(0, ns->pidmap[0].page);
	atomic_set(&ns->pidmap[0].nr_free, BITS_PER_PAGE - 1);

	for (i = 1; i < PIDMAP_ENTRIES; i++)
		atomic_set(&ns->pidmap[i].nr_free, BITS_PER_PAGE);

	return ns;

out_free_map:
	kfree(ns->pidmap[0].page);
out_free:
	kmem_cache_free(pid_ns_cachep, ns);
out:
	return ERR_PTR(err);
}

static void delayed_free_pidns(struct rcu_head *p)
{
	kmem_cache_free(pid_ns_cachep,
			container_of(p, struct pid_namespace, rcu));
}

static void destroy_pid_namespace(struct pid_namespace *ns)
{
	int i;

	ns_free_inum(&ns->ns);
	for (i = 0; i < PIDMAP_ENTRIES; i++)
		kfree(ns->pidmap[i].page);
	put_user_ns(ns->user_ns);
	call_rcu(&ns->rcu, delayed_free_pidns);
}

struct pid_namespace *copy_pid_ns(unsigned long flags,
	struct user_namespace *user_ns, struct pid_namespace *old_ns)
{
	if (!(flags & CLONE_NEWPID))
		return get_pid_ns(old_ns);
	if (task_active_pid_ns(current) != old_ns)
		return ERR_PTR(-EINVAL);
	return create_pid_namespace(user_ns, old_ns);
}

static void free_pid_ns(struct kref *kref)
{
	struct pid_namespace *ns;

	ns = container_of(kref, struct pid_namespace, kref);
	destroy_pid_namespace(ns);
}

void put_pid_ns(struct pid_namespace *ns)
{
	struct pid_namespace *parent;

	while (ns != &init_pid_ns) {
		parent = ns->parent;
		if (!kref_put(&ns->kref, free_pid_ns))
			break;
		ns = parent;
	}
}
EXPORT_SYMBOL_GPL(put_pid_ns);

void zap_pid_ns_processes(struct pid_namespace *pid_ns)
{
	int nr;
	int rc;
	struct task_struct *task, *me = current;
	int init_pids = thread_group_leader(me) ? 1 : 2;

	/* Don't allow any more processes into the pid namespace */
	disable_pid_allocation(pid_ns);

	/*
	 * Ignore SIGCHLD causing any terminated children to autoreap.
	 * This speeds up the namespace shutdown, plus see the comment
	 * below.
	 */
	spin_lock_irq(&me->sighand->siglock);
	me->sighand->action[SIGCHLD - 1].sa.sa_handler = SIG_IGN;
	spin_unlock_irq(&me->sighand->siglock);

	/*
	 * The last thread in the cgroup-init thread group is terminating.
	 * Find remaining pid_ts in the namespace, signal and wait for them
	 * to exit.
	 *
	 * Note:  This signals each threads in the namespace - even those that
	 * 	  belong to the same thread group, To avoid this, we would have
	 * 	  to walk the entire tasklist looking a processes in this
	 * 	  namespace, but that could be unnecessarily expensive if the
	 * 	  pid namespace has just a few processes. Or we need to
	 * 	  maintain a tasklist for each pid namespace.
	 *
	 */
	read_lock(&tasklist_lock);
	nr = next_pidmap(pid_ns, 1);
	while (nr > 0) {
		rcu_read_lock();

		task = pid_task(find_vpid(nr), PIDTYPE_PID);
		if (task && !__fatal_signal_pending(task))
			send_sig_info(SIGKILL, SEND_SIG_FORCED, task);

		rcu_read_unlock();

		nr = next_pidmap(pid_ns, nr);
	}
	read_unlock(&tasklist_lock);

	/*
	 * Reap the EXIT_ZOMBIE children we had before we ignored SIGCHLD.
	 * sys_wait4() will also block until our children traced from the
	 * parent namespace are detached and become EXIT_DEAD.
	 */
	do {
		clear_thread_flag(TIF_SIGPENDING);
		rc = sys_wait4(-1, NULL, __WALL, NULL);
	} while (rc != -ECHILD);

	/*
	 * sys_wait4() above can't reap the EXIT_DEAD children but we do not
	 * really care, we could reparent them to the global init. We could
	 * exit and reap ->child_reaper even if it is not the last thread in
	 * this pid_ns, free_pid(nr_hashed == 0) calls proc_cleanup_work(),
	 * pid_ns can not go away until proc_kill_sb() drops the reference.
	 *
	 * But this ns can also have other tasks injected by setns()+fork().
	 * Again, ignoring the user visible semantics we do not really need
	 * to wait until they are all reaped, but they can be reparented to
	 * us and thus we need to ensure that pid->child_reaper stays valid
	 * until they all go away. See free_pid()->wake_up_process().
	 *
	 * We rely on ignored SIGCHLD, an injected zombie must be autoreaped
	 * if reparented.
	 */
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (pid_ns->nr_hashed == init_pids)
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);

	if (pid_ns->reboot)
		current->signal->group_exit_code = pid_ns->reboot;

	acct_exit_ns(pid_ns);
	return;
}

#ifdef CONFIG_CHECKPOINT_RESTORE
static int pid_ns_ctl_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	struct pid_namespace *pid_ns = task_active_pid_ns(current);
	struct ctl_table tmp = *table;

	if (write && !ns_capable(pid_ns->user_ns, CAP_SYS_ADMIN))
		return -EPERM;

	/*
	 * Writing directly to ns' last_pid field is OK, since this field
	 * is volatile in a living namespace anyway and a code writing to
	 * it should synchronize its usage with external means.
	 */

	tmp.data = &pid_ns->last_pid;
	return proc_dointvec_minmax(&tmp, write, buffer, lenp, ppos);
}

extern int pid_max;
static int zero = 0;
static struct ctl_table pid_ns_ctl_table[] = {
	{
		.procname = "ns_last_pid",
		.maxlen = sizeof(int),
		.mode = 0666, /* permissions are checked in the handler */
		.proc_handler = pid_ns_ctl_handler,
		.extra1 = &zero,
		.extra2 = &pid_max,
	},
	{ }
};
static struct ctl_path kern_path[] = { { .procname = "kernel", }, { } };
#endif	/* CONFIG_CHECKPOINT_RESTORE */

int reboot_pid_ns(struct pid_namespace *pid_ns, int cmd)
{
	if (pid_ns == &init_pid_ns)
		return 0;

	switch (cmd) {
	case LINUX_REBOOT_CMD_RESTART2:
	case LINUX_REBOOT_CMD_RESTART:
		pid_ns->reboot = SIGHUP;
		break;

	case LINUX_REBOOT_CMD_POWER_OFF:
	case LINUX_REBOOT_CMD_HALT:
		pid_ns->reboot = SIGINT;
		break;
	default:
		return -EINVAL;
	}

	read_lock(&tasklist_lock);
	force_sig(SIGKILL, pid_ns->child_reaper);
	read_unlock(&tasklist_lock);

	do_exit(0);

	/* Not reached */
	return 0;
}

static inline struct pid_namespace *to_pid_ns(struct ns_common *ns)
{
	return container_of(ns, struct pid_namespace, ns);
}

static struct ns_common *pidns_get(struct task_struct *task)
{
	struct pid_namespace *ns;

	rcu_read_lock();
	ns = task_active_pid_ns(task);
	if (ns)
		get_pid_ns(ns);
	rcu_read_unlock();

	return ns ? &ns->ns : NULL;
}

static void pidns_put(struct ns_common *ns)
{
	put_pid_ns(to_pid_ns(ns));
}

static int pidns_install(struct nsproxy *nsproxy, struct ns_common *ns)
{
	struct pid_namespace *active = task_active_pid_ns(current);
	struct pid_namespace *ancestor, *new = to_pid_ns(ns);

	if (!ns_capable(new->user_ns, CAP_SYS_ADMIN) ||
	    !ns_capable(current_user_ns(), CAP_SYS_ADMIN))
		return -EPERM;

	/*
	 * Only allow entering the current active pid namespace
	 * or a child of the current active pid namespace.
	 *
	 * This is required for fork to return a usable pid value and
	 * this maintains the property that processes and their
	 * children can not escape their current pid namespace.
	 */
	if (new->level < active->level)
		return -EINVAL;

	ancestor = new;
	while (ancestor->level > active->level)
		ancestor = ancestor->parent;
	if (ancestor != active)
		return -EINVAL;

	put_pid_ns(nsproxy->pid_ns_for_children);
	nsproxy->pid_ns_for_children = get_pid_ns(new);
	return 0;
}

const struct proc_ns_operations pidns_operations = {
	.name		= "pid",
	.type		= CLONE_NEWPID,
	.get		= pidns_get,
	.put		= pidns_put,
	.install	= pidns_install,
};

static __init int pid_namespaces_init(void)
{
	pid_ns_cachep = KMEM_CACHE(pid_namespace, SLAB_PANIC);

#ifdef CONFIG_CHECKPOINT_RESTORE
	register_sysctl_paths(kern_path, pid_ns_ctl_table);
#endif
	return 0;
}

__initcall(pid_namespaces_init);
