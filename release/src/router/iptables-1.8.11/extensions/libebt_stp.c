/* ebt_stp
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2003
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ether.h>
#include <linux/netfilter_bridge/ebt_stp.h>
#include <xtables.h>

#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

/* These must correspond to the bit position in EBT_STP_* defines */
enum {
	O_TYPE = 0,
	O_FLAGS,
	O_RPRIO,
	O_RADDR,
	O_RCOST,
	O_SPRIO,
	O_SADDR,
	O_PORT,
	O_MSGAGE,
	O_MAXAGE,
	O_HTIME,
	O_FWDD,
};

static const struct xt_option_entry brstp_opts[] = {
#define ENTRY(n, i, t) { .name = n, .id = i, .type = t, .flags = XTOPT_INVERT }
	ENTRY("stp-type",          O_TYPE,   XTTYPE_STRING),
	ENTRY("stp-flags",         O_FLAGS,  XTTYPE_STRING),
	ENTRY("stp-root-prio",     O_RPRIO,  XTTYPE_UINT16RC),
	ENTRY("stp-root-addr",     O_RADDR,  XTTYPE_ETHERMACMASK),
	ENTRY("stp-root-cost",     O_RCOST,  XTTYPE_UINT32RC),
	ENTRY("stp-sender-prio",   O_SPRIO,  XTTYPE_UINT16RC),
	ENTRY("stp-sender-addr",   O_SADDR,  XTTYPE_ETHERMACMASK),
	ENTRY("stp-port",          O_PORT,   XTTYPE_UINT16RC),
	ENTRY("stp-msg-age",       O_MSGAGE, XTTYPE_UINT16RC),
	ENTRY("stp-max-age",       O_MAXAGE, XTTYPE_UINT16RC),
	ENTRY("stp-hello-time",    O_HTIME,  XTTYPE_UINT16RC),
	ENTRY("stp-forward-delay", O_FWDD,   XTTYPE_UINT16RC),
	XTOPT_TABLEEND,
};

#define BPDU_TYPE_CONFIG 0
#define BPDU_TYPE_TCN 0x80
#define BPDU_TYPE_CONFIG_STRING "config"
#define BPDU_TYPE_TCN_STRING "tcn"

#define FLAG_TC 0x01
#define FLAG_TC_ACK 0x80
#define FLAG_TC_STRING "topology-change"
#define FLAG_TC_ACK_STRING "topology-change-ack"

static void brstp_print_help(void)
{
	printf(
"stp options:\n"
"[!] --stp-type type                  : BPDU type\n"
"[!] --stp-flags flag                 : control flag\n"
"[!] --stp-root-prio prio[:prio]      : root priority (16-bit) range\n"
"[!] --stp-root-addr address[/mask]   : MAC address of root\n"
"[!] --stp-root-cost cost[:cost]      : root cost (32-bit) range\n"
"[!] --stp-sender-prio prio[:prio]    : sender priority (16-bit) range\n"
"[!] --stp-sender-addr address[/mask] : MAC address of sender\n"
"[!] --stp-port port[:port]           : port id (16-bit) range\n"
"[!] --stp-msg-age age[:age]          : message age timer (16-bit) range\n"
"[!] --stp-max-age age[:age]          : maximum age timer (16-bit) range\n"
"[!] --stp-hello-time time[:time]     : hello time timer (16-bit) range\n"
"[!] --stp-forward-delay delay[:delay]: forward delay timer (16-bit) range\n"
" Recognized BPDU type strings:\n"
"   \"config\": configuration BPDU (=0)\n"
"   \"tcn\"   : topology change notification BPDU (=0x80)\n"
" Recognized control flag strings:\n"
"   \"topology-change\"    : topology change flag (0x01)\n"
"   \"topology-change-ack\": topology change acknowledgement flag (0x80)");
}

static void print_range(unsigned int l, unsigned int u)
{
	if (l == u)
		printf("%u", l);
	else
		printf("%u:%u", l, u);
}

static void brstp_parse(struct xt_option_call *cb)
{
	struct ebt_stp_info *stpinfo = cb->data;
	char *end = NULL;
	long int i;

	xtables_option_parse(cb);

	stpinfo->bitmask |= 1 << cb->entry->id;
	if (cb->invert)
		stpinfo->invflags |= 1 << cb->entry->id;

	switch (cb->entry->id) {
	case O_TYPE:
		i = strtol(cb->arg, &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(cb->arg, BPDU_TYPE_CONFIG_STRING))
				stpinfo->type = BPDU_TYPE_CONFIG;
			else if (!strcasecmp(cb->arg, BPDU_TYPE_TCN_STRING))
				stpinfo->type = BPDU_TYPE_TCN;
			else
				xtables_error(PARAMETER_PROBLEM, "Bad --stp-type argument");
		} else
			stpinfo->type = i;
		break;
	case O_FLAGS:
		i = strtol(cb->arg, &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(cb->arg, FLAG_TC_STRING))
				stpinfo->config.flags = FLAG_TC;
			else if (!strcasecmp(cb->arg, FLAG_TC_ACK_STRING))
				stpinfo->config.flags = FLAG_TC_ACK;
			else
				xtables_error(PARAMETER_PROBLEM, "Bad --stp-flags argument");
		} else
			stpinfo->config.flags = i;
		break;
	case O_RADDR:
		memcpy(stpinfo->config.root_addr, cb->val.ethermac, ETH_ALEN);
		memcpy(stpinfo->config.root_addrmsk,
		       cb->val.ethermacmask, ETH_ALEN);
		break;
	case O_SADDR:
		memcpy(stpinfo->config.sender_addr, cb->val.ethermac, ETH_ALEN);
		memcpy(stpinfo->config.sender_addrmsk,
		       cb->val.ethermacmask, ETH_ALEN);
		break;

#define RANGE_ASSIGN(fname, val) {				    \
		stpinfo->config.fname##l = val[0];			    \
		stpinfo->config.fname##u = cb->nvals > 1 ? val[1] : val[0]; \
}
	case O_RPRIO:
		RANGE_ASSIGN(root_prio, cb->val.u16_range);
		break;
	case O_RCOST:
		RANGE_ASSIGN(root_cost, cb->val.u32_range);
		break;
	case O_SPRIO:
		RANGE_ASSIGN(sender_prio, cb->val.u16_range);
		break;
	case O_PORT:
		RANGE_ASSIGN(port, cb->val.u16_range);
		break;
	case O_MSGAGE:
		RANGE_ASSIGN(msg_age, cb->val.u16_range);
		break;
	case O_MAXAGE:
		RANGE_ASSIGN(max_age, cb->val.u16_range);
		break;
	case O_HTIME:
		RANGE_ASSIGN(hello_time, cb->val.u16_range);
		break;
	case O_FWDD:
		RANGE_ASSIGN(forward_delay, cb->val.u16_range);
		break;
#undef RANGE_ASSIGN
	}
}

static void brstp_print(const void *ip, const struct xt_entry_match *match,
			 int numeric)
{
	const struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)match->data;
	const struct ebt_stp_config_info *c = &(stpinfo->config);
	int i;

	for (i = 0; (1 << i) < EBT_STP_MASK; i++) {
		if (!(stpinfo->bitmask & (1 << i)))
			continue;
		printf("%s--%s ",
		       (stpinfo->invflags & (1 << i)) ? "! " : "",
		       brstp_opts[i].name);
		if (EBT_STP_TYPE == (1 << i)) {
			if (stpinfo->type == BPDU_TYPE_CONFIG)
				printf("%s", BPDU_TYPE_CONFIG_STRING);
			else if (stpinfo->type == BPDU_TYPE_TCN)
				printf("%s", BPDU_TYPE_TCN_STRING);
			else
				printf("%d", stpinfo->type);
		} else if (EBT_STP_FLAGS == (1 << i)) {
			if (c->flags == FLAG_TC)
				printf("%s", FLAG_TC_STRING);
			else if (c->flags == FLAG_TC_ACK)
				printf("%s", FLAG_TC_ACK_STRING);
			else
				printf("%d", c->flags);
		} else if (EBT_STP_ROOTPRIO == (1 << i))
			print_range(c->root_priol, c->root_priou);
		else if (EBT_STP_ROOTADDR == (1 << i))
			xtables_print_mac_and_mask((unsigned char *)c->root_addr,
			   (unsigned char*)c->root_addrmsk);
		else if (EBT_STP_ROOTCOST == (1 << i))
			print_range(c->root_costl, c->root_costu);
		else if (EBT_STP_SENDERPRIO == (1 << i))
			print_range(c->sender_priol, c->sender_priou);
		else if (EBT_STP_SENDERADDR == (1 << i))
			xtables_print_mac_and_mask((unsigned char *)c->sender_addr,
			   (unsigned char *)c->sender_addrmsk);
		else if (EBT_STP_PORT == (1 << i))
			print_range(c->portl, c->portu);
		else if (EBT_STP_MSGAGE == (1 << i))
			print_range(c->msg_agel, c->msg_ageu);
		else if (EBT_STP_MAXAGE == (1 << i))
			print_range(c->max_agel, c->max_ageu);
		else if (EBT_STP_HELLOTIME == (1 << i))
			print_range(c->hello_timel, c->hello_timeu);
		else if (EBT_STP_FWDD == (1 << i))
			print_range(c->forward_delayl, c->forward_delayu);
		printf(" ");
	}
}

static struct xtables_match brstp_match = {
	.name		= "stp",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= sizeof(struct ebt_stp_info),
	.help		= brstp_print_help,
	.x6_parse	= brstp_parse,
	.print		= brstp_print,
	.x6_options	= brstp_opts
};

void _init(void)
{
	xtables_register_match(&brstp_match);
}
