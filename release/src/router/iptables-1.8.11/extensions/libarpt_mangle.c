/*
 * Arturo Borrero Gonzalez <arturo@debian.org> adapted
 * this code to libxtables for arptables-compat in 2015
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <netinet/ether.h>
#include <xtables.h>
#include <linux/netfilter_arp/arpt_mangle.h>
#include "iptables/nft.h"

static void arpmangle_print_help(void)
{
	printf(
	"mangle target options:\n"
	"--mangle-ip-s IP address\n"
	"--mangle-ip-d IP address\n"
	"--mangle-mac-s MAC address\n"
	"--mangle-mac-d MAC address\n"
	"--mangle-target target (DROP, CONTINUE or ACCEPT -- default is ACCEPT)\n");
}

/* internal use only, explicitly not covered by ARPT_MANGLE_MASK */
#define ARPT_MANGLE_TARGET	0x10

static const struct xt_option_entry arpmangle_opts[] = {
{ .name = "mangle-ip-s", .id = ARPT_MANGLE_SIP, .type = XTTYPE_HOSTMASK },
{ .name = "mangle-ip-d", .id = ARPT_MANGLE_TIP, .type = XTTYPE_HOSTMASK },
{ .name = "mangle-mac-s", .id = ARPT_MANGLE_SDEV, .type = XTTYPE_ETHERMAC },
{ .name = "mangle-mac-d", .id = ARPT_MANGLE_TDEV, .type = XTTYPE_ETHERMAC },
{ .name = "mangle-target", .id = ARPT_MANGLE_TARGET, .type = XTTYPE_STRING },
XTOPT_TABLEEND,
};

static void arpmangle_init(struct xt_entry_target *target)
{
	struct arpt_mangle *mangle = (struct arpt_mangle *)target->data;

	mangle->target = NF_ACCEPT;
}

static void assert_hopts(const struct arpt_entry *e, const char *optname)
{
	if (e->arp.arhln_mask == 0)
		xtables_error(PARAMETER_PROBLEM, "no --h-length defined");
	if (e->arp.invflags & IPT_INV_ARPHLN)
		xtables_error(PARAMETER_PROBLEM,
			      "! hln not allowed for --%s", optname);
	if (e->arp.arhln != 6)
		xtables_error(PARAMETER_PROBLEM, "only --h-length 6 supported");
}

static void arpmangle_parse(struct xt_option_call *cb)
{
	const struct arpt_entry *e = cb->xt_entry;
	struct arpt_mangle *mangle = cb->data;

	xtables_option_parse(cb);
	mangle->flags |= (cb->entry->id & ARPT_MANGLE_MASK);
	switch (cb->entry->id) {
	case ARPT_MANGLE_SIP:
		mangle->u_s.src_ip = cb->val.haddr.in;
		break;
	case ARPT_MANGLE_TIP:
		mangle->u_t.tgt_ip = cb->val.haddr.in;
		break;
	case ARPT_MANGLE_SDEV:
		assert_hopts(e, cb->entry->name);
		memcpy(mangle->src_devaddr, cb->val.ethermac, ETH_ALEN);
	case ARPT_MANGLE_TDEV:
		assert_hopts(e, cb->entry->name);
		memcpy(mangle->tgt_devaddr, cb->val.ethermac, ETH_ALEN);
		break;
	case ARPT_MANGLE_TARGET:
		if (!strcmp(cb->arg, "DROP"))
			mangle->target = NF_DROP;
		else if (!strcmp(cb->arg, "ACCEPT"))
			mangle->target = NF_ACCEPT;
		else if (!strcmp(cb->arg, "CONTINUE"))
			mangle->target = XT_CONTINUE;
		else
			xtables_error(PARAMETER_PROBLEM,
				      "bad target for --mangle-target");
		break;
	}
}

static const char *ipaddr_to(const struct in_addr *addrp, int numeric)
{
	if (numeric)
		return xtables_ipaddr_to_numeric(addrp);
	else
		return xtables_ipaddr_to_anyname(addrp);
}

static void
arpmangle_print(const void *ip, const struct xt_entry_target *target,
		int numeric)
{
	struct arpt_mangle *m = (struct arpt_mangle *)(target->data);

	if (m->flags & ARPT_MANGLE_SIP) {
		printf(" --mangle-ip-s %s",
		       ipaddr_to(&(m->u_s.src_ip), numeric));
	}
	if (m->flags & ARPT_MANGLE_SDEV) {
		printf(" --mangle-mac-s ");
		xtables_print_mac((unsigned char *)m->src_devaddr);
	}
	if (m->flags & ARPT_MANGLE_TIP) {
		printf(" --mangle-ip-d %s",
		       ipaddr_to(&(m->u_t.tgt_ip), numeric));
	}
	if (m->flags & ARPT_MANGLE_TDEV) {
		printf(" --mangle-mac-d ");
		xtables_print_mac((unsigned char *)m->tgt_devaddr);
	}
	if (m->target != NF_ACCEPT) {
		printf(" --mangle-target %s",
		       m->target == NF_DROP ? "DROP" : "CONTINUE");
	}
}

static void arpmangle_save(const void *ip, const struct xt_entry_target *target)
{
	arpmangle_print(ip, target, 0);
}

static void print_devaddr_xlate(const char *macaddress, struct xt_xlate *xl)
{
	unsigned int i;

	xt_xlate_add(xl, "%02x", macaddress[0]);
	for (i = 1; i < ETH_ALEN; ++i)
		xt_xlate_add(xl, ":%02x", macaddress[i]);
}

static int arpmangle_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct arpt_mangle *m = (const void *)params->target->data;

	if (m->flags & ARPT_MANGLE_SIP)
		xt_xlate_add(xl, "arp saddr ip set %s ",
			     xtables_ipaddr_to_numeric(&m->u_s.src_ip));

	if (m->flags & ARPT_MANGLE_SDEV) {
		xt_xlate_add(xl, "arp %caddr ether set ", 's');
		print_devaddr_xlate(m->src_devaddr, xl);
	}

	if (m->flags & ARPT_MANGLE_TIP)
		xt_xlate_add(xl, "arp daddr ip set %s ",
			     xtables_ipaddr_to_numeric(&m->u_t.tgt_ip));

	if (m->flags & ARPT_MANGLE_TDEV) {
		xt_xlate_add(xl, "arp %caddr ether set ", 'd');
		print_devaddr_xlate(m->tgt_devaddr, xl);
	}

	switch (m->target) {
	case NF_ACCEPT:
		xt_xlate_add(xl, "accept");
		break;
	case NF_DROP:
		xt_xlate_add(xl, "drop");
		break;
	default:
		break;
	}

	return 1;
}

static struct xtables_target arpmangle_target = {
	.name		= "mangle",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_ARP,
	.size		= XT_ALIGN(sizeof(struct arpt_mangle)),
	.userspacesize	= XT_ALIGN(sizeof(struct arpt_mangle)),
	.help		= arpmangle_print_help,
	.init		= arpmangle_init,
	.x6_parse	= arpmangle_parse,
	.print		= arpmangle_print,
	.save		= arpmangle_save,
	.x6_options	= arpmangle_opts,
	.xlate		= arpmangle_xlate,
};

void _init(void)
{
	xtables_register_target(&arpmangle_target);
}
