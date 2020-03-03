/*
 * Common code for control of lockd and nfsv4 grace periods.
 *
 * Transplanted from lockd code
 */

#include <linux/module.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <linux/fs.h>

static int grace_net_id;
static DEFINE_SPINLOCK(grace_lock);

/**
 * locks_start_grace
 * @net: net namespace that this lock manager belongs to
 * @lm: who this grace period is for
 *
 * A grace period is a period during which locks should not be given
 * out.  Currently grace periods are only enforced by the two lock
 * managers (lockd and nfsd), using the locks_in_grace() function to
 * check when they are in a grace period.
 *
 * This function is called to start a grace period.
 */
void
locks_start_grace(struct net *net, struct lock_manager *lm)
{
	struct list_head *grace_list = net_generic(net, grace_net_id);

	spin_lock(&grace_lock);
	list_add(&lm->list, grace_list);
	spin_unlock(&grace_lock);
}
EXPORT_SYMBOL_GPL(locks_start_grace);

/**
 * locks_end_grace
 * @net: net namespace that this lock manager belongs to
 * @lm: who this grace period is for
 *
 * Call this function to state that the given lock manager is ready to
 * resume regular locking.  The grace period will not end until all lock
 * managers that called locks_start_grace() also call locks_end_grace().
 * Note that callers count on it being safe to call this more than once,
 * and the second call should be a no-op.
 */
void
locks_end_grace(struct lock_manager *lm)
{
	spin_lock(&grace_lock);
	list_del_init(&lm->list);
	spin_unlock(&grace_lock);
}
EXPORT_SYMBOL_GPL(locks_end_grace);

/**
 * locks_in_grace
 *
 * Lock managers call this function to determine when it is OK for them
 * to answer ordinary lock requests, and when they should accept only
 * lock reclaims.
 */
int
locks_in_grace(struct net *net)
{
	struct list_head *grace_list = net_generic(net, grace_net_id);

	return !list_empty(grace_list);
}
EXPORT_SYMBOL_GPL(locks_in_grace);

static int __net_init
grace_init_net(struct net *net)
{
	struct list_head *grace_list = net_generic(net, grace_net_id);

	INIT_LIST_HEAD(grace_list);
	return 0;
}

static void __net_exit
grace_exit_net(struct net *net)
{
	struct list_head *grace_list = net_generic(net, grace_net_id);

	WARN_ONCE(!list_empty(grace_list),
		  "net %x %s: grace_list is not empty\n",
		  net->ns.inum, __func__);
}

static struct pernet_operations grace_net_ops = {
	.init = grace_init_net,
	.exit = grace_exit_net,
	.id   = &grace_net_id,
	.size = sizeof(struct list_head),
};

static int __init
init_grace(void)
{
	return register_pernet_subsys(&grace_net_ops);
}

static void __exit
exit_grace(void)
{
	unregister_pernet_subsys(&grace_net_ops);
}

MODULE_AUTHOR("Jeff Layton <jlayton@primarydata.com>");
MODULE_LICENSE("GPL");
module_init(init_grace)
module_exit(exit_grace)
