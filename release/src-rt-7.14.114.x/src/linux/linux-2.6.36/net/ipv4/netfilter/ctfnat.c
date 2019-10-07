/*
 * Packet matching code.
 */
#include <linux/cache.h>
#include <linux/skbuff.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <net/route.h>
#include <net/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>

enum {
	CTF_NAT_0 = 1,
	CTF_NAT_1 = 2
};

unsigned int ctfnat_act = 0;

static int ctfnat_ctrl(struct file *file, const char *buffer, unsigned long length, void *data)
{
	int tmp = 0;

	tmp = simple_strtol(buffer, NULL, 0);

	if(tmp == CTF_NAT_0 || CTF_NAT_1)
		ctfnat_act = tmp;
	
	printk("\n%s: reset ctf val=%d\n", __func__, ctfnat_act);

	return length;
}

static int __init init(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *p;

	p = create_proc_entry("ctfnat", 0200, init_net.proc_net);

	if(p) {
		p->write_proc = ctfnat_ctrl;
	}
#endif
	return 0;
}

static void __exit fini(void)
{
}

EXPORT_SYMBOL(ctfnat_act);

module_init(init);
module_exit(fini);
MODULE_LICENSE("Proprietary");

