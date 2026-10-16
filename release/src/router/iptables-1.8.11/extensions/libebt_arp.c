/* ebt_arp
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 * Tim Gardner <timg@tpi.com>
 *
 * April, 2002
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <netinet/ether.h>

#include <xtables.h>
#include <net/if_arp.h>
#include <linux/netfilter_bridge/ebt_arp.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

/* values must correspond with EBT_ARP_* bit positions */
enum {
	O_OPCODE = 0,
	O_HTYPE,
	O_PTYPE,
	O_SRC_IP,
	O_DST_IP,
	O_SRC_MAC,
	O_DST_MAC,
	O_GRAT,
};

static const struct xt_option_entry brarp_opts[] = {
#define ENTRY(n, i, t) { .name = n, .id = i, .type = t, .flags = XTOPT_INVERT }
	ENTRY("arp-opcode",     O_OPCODE,  XTTYPE_STRING),
	ENTRY("arp-op",         O_OPCODE,  XTTYPE_STRING),
	ENTRY("arp-htype",      O_HTYPE,   XTTYPE_STRING),
	ENTRY("arp-ptype",      O_PTYPE,   XTTYPE_STRING),
	ENTRY("arp-ip-src",     O_SRC_IP,  XTTYPE_HOSTMASK),
	ENTRY("arp-ip-dst",     O_DST_IP,  XTTYPE_HOSTMASK),
	ENTRY("arp-mac-src",    O_SRC_MAC, XTTYPE_ETHERMACMASK),
	ENTRY("arp-mac-dst",    O_DST_MAC, XTTYPE_ETHERMACMASK),
	ENTRY("arp-gratuitous", O_GRAT,    XTTYPE_NONE),
#undef ENTRY
	XTOPT_TABLEEND
};

/* a few names */
static char *opcodes[] =
{
	"Request",
	"Reply",
	"Request_Reverse",
	"Reply_Reverse",
	"DRARP_Request",
	"DRARP_Reply",
	"DRARP_Error",
	"InARP_Request",
	"ARP_NAK",
};

static void brarp_print_help(void)
{
	int i;

	printf(
"arp options:\n"
"[!] --arp-opcode  opcode        : ARP opcode (integer or string)\n"
"[!] --arp-htype   type          : ARP hardware type (integer or string)\n"
"[!] --arp-ptype   type          : ARP protocol type (hexadecimal or string)\n"
"[!] --arp-ip-src  address[/mask]: ARP IP source specification\n"
"[!] --arp-ip-dst  address[/mask]: ARP IP target specification\n"
"[!] --arp-mac-src address[/mask]: ARP MAC source specification\n"
"[!] --arp-mac-dst address[/mask]: ARP MAC target specification\n"
"[!] --arp-gratuitous            : ARP gratuitous packet\n"
" opcode strings: \n");
	for (i = 0; i < ARRAY_SIZE(opcodes); i++)
		printf(" %d = %s\n", i + 1, opcodes[i]);
	printf(
" hardware type string: 1 = Ethernet\n"
" protocol type string: see "XT_PATH_ETHERTYPES"\n");
}

static void brarp_parse(struct xt_option_call *cb)
{
	struct ebt_arp_info *arpinfo = cb->data;
	struct xt_ethertypeent *ent;
	long int i;
	char *end;


	xtables_option_parse(cb);

	arpinfo->bitmask |= 1 << cb->entry->id;
	if (cb->invert)
		arpinfo->invflags |= 1 << cb->entry->id;

	switch (cb->entry->id) {
	case O_OPCODE:
		i = strtol(cb->arg, &end, 10);
		if (i < 0 || i >= (0x1 << 16) || *end !='\0') {
			for (i = 0; i < ARRAY_SIZE(opcodes); i++)
				if (!strcasecmp(opcodes[i], cb->arg))
					break;
			if (i == ARRAY_SIZE(opcodes))
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with specified ARP opcode");
			i++;
		}
		arpinfo->opcode = htons(i);
		break;
	case O_HTYPE:
		i = strtol(cb->arg, &end, 10);
		if (i < 0 || i >= (0x1 << 16) || *end !='\0') {
			if (!strcasecmp("Ethernet", cb->arg))
				i = 1;
			else
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with specified ARP hardware type");
		}
		arpinfo->htype = htons(i);
		break;
	case O_PTYPE:
		i = strtol(cb->arg, &end, 16);
		if (i >= 0 && i < (0x1 << 16) && *end == '\0') {
			arpinfo->ptype = htons(i);
			break;
		}
		ent = xtables_getethertypebyname(cb->arg);
		if (!ent)
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with specified ARP protocol type");
		arpinfo->ptype = htons(ent->e_ethertype);
		break;
	case O_SRC_IP:
		arpinfo->saddr = cb->val.haddr.ip & cb->val.hmask.ip;
		arpinfo->smsk = cb->val.hmask.ip;
		break;
	case O_DST_IP:
		arpinfo->daddr = cb->val.haddr.ip & cb->val.hmask.ip;
		arpinfo->dmsk = cb->val.hmask.ip;
		break;
	case O_SRC_MAC:
		memcpy(arpinfo->smaddr, cb->val.ethermac, ETH_ALEN);
		memcpy(arpinfo->smmsk, cb->val.ethermacmask, ETH_ALEN);
		break;
	case O_DST_MAC:
		memcpy(arpinfo->dmaddr, cb->val.ethermac, ETH_ALEN);
		memcpy(arpinfo->dmmsk, cb->val.ethermacmask, ETH_ALEN);
		break;
	}
}

static void brarp_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct ebt_arp_info *arpinfo = (struct ebt_arp_info *)match->data;

	if (arpinfo->bitmask & EBT_ARP_OPCODE) {
		int opcode = ntohs(arpinfo->opcode);

		if (arpinfo->invflags & EBT_ARP_OPCODE)
			printf("! ");
		printf("--arp-op ");
		if (opcode > 0 && opcode <= ARRAY_SIZE(opcodes))
			printf("%s ", opcodes[opcode - 1]);
		else
			printf("%d ", opcode);
	}
	if (arpinfo->bitmask & EBT_ARP_HTYPE) {
		if (arpinfo->invflags & EBT_ARP_HTYPE)
			printf("! ");
		printf("--arp-htype %d ", ntohs(arpinfo->htype));
	}
	if (arpinfo->bitmask & EBT_ARP_PTYPE) {
		if (arpinfo->invflags & EBT_ARP_PTYPE)
			printf("! ");
		printf("--arp-ptype 0x%x ", ntohs(arpinfo->ptype));
	}
	if (arpinfo->bitmask & EBT_ARP_SRC_IP) {
		if (arpinfo->invflags & EBT_ARP_SRC_IP)
			printf("! ");
		printf("--arp-ip-src %s%s ",
		       xtables_ipaddr_to_numeric((void *)&arpinfo->saddr),
		       xtables_ipmask_to_numeric((void *)&arpinfo->smsk));
	}
	if (arpinfo->bitmask & EBT_ARP_DST_IP) {
		if (arpinfo->invflags & EBT_ARP_DST_IP)
			printf("! ");
		printf("--arp-ip-dst %s%s ",
		       xtables_ipaddr_to_numeric((void *)&arpinfo->daddr),
		       xtables_ipmask_to_numeric((void *)&arpinfo->dmsk));
	}
	if (arpinfo->bitmask & EBT_ARP_SRC_MAC) {
		if (arpinfo->invflags & EBT_ARP_SRC_MAC)
			printf("! ");
		printf("--arp-mac-src ");
		xtables_print_mac_and_mask(arpinfo->smaddr, arpinfo->smmsk);
		printf(" ");
	}
	if (arpinfo->bitmask & EBT_ARP_DST_MAC) {
		if (arpinfo->invflags & EBT_ARP_DST_MAC)
			printf("! ");
		printf("--arp-mac-dst ");
		xtables_print_mac_and_mask(arpinfo->dmaddr, arpinfo->dmmsk);
		printf(" ");
	}
	if (arpinfo->bitmask & EBT_ARP_GRAT) {
		if (arpinfo->invflags & EBT_ARP_GRAT)
			printf("! ");
		printf("--arp-gratuitous ");
	}
}

static struct xtables_match brarp_match = {
	.name		= "arp",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_arp_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_arp_info)),
	.help		= brarp_print_help,
	.x6_parse	= brarp_parse,
	.print		= brarp_print,
	.x6_options	= brarp_opts,
};

void _init(void)
{
	xtables_register_match(&brarp_match);
}
