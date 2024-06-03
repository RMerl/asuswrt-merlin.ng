/* ebt_inat
 *
 * Authors:
 * Grzegorz Borowiak <grzes@gnu.univ.gda.pl>
 *
 * August, 2003
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <getopt.h>
#include <ctype.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_inat.h>

static int s_sub_supplied, d_sub_supplied;

#define NAT_S '1'
#define NAT_D '1'
#define NAT_S_SUB '2'
#define NAT_D_SUB '2'
#define NAT_S_TARGET '3'
#define NAT_D_TARGET '3'
static const struct option opts_s[] =
{
	{ "isnat-list"     , required_argument, 0, NAT_S },
	{ "isnat-sub"      , required_argument, 0, NAT_S_SUB },
	{ "isnat-default-target"   , required_argument, 0, NAT_S_TARGET },
	{ 0 }
};

static const struct option opts_d[] =
{
	{ "idnat-list"     , required_argument, 0, NAT_D },
	{ "idnat-sub"      , required_argument, 0, NAT_D_SUB },
	{ "idnat-default-target"   , required_argument, 0, NAT_D_TARGET },
	{ 0 }
};

static void print_help_common(const char *cas)
{
	printf(
"isnat options:\n"
" --i%1.1snat-list                    : indexed list of MAC addresses\n"
" --i%1.1snat-sub                     : /24 subnet to which the rule apply\n"
" --i%1.1snat-default-target target   : ACCEPT, DROP, RETURN or CONTINUE\n"
"Indexed list of addresses is as follows:\n"
"\tlist := chunk\n"
"\tlist := list chunk\n"
"\tchunk := pair ','\n"
"\tpair := index '=' action\n"
"\taction := mac_addr\n"
"\taction := mac_addr '+'\n"
"\taction := '_'\n"
"where\n"
"\tindex -- an integer [0..255]\n"
"\tmac_addr -- a MAC address in format xx:xx:xx:xx:xx:xx\n"
"If '_' at some index is specified, packets with last %s IP address byte\n"
"equal to index are DROPped. If there is a MAC address, they are %1.1snatted\n"
"to this and the target is CONTINUE. If this MAC is followed by '+', the\n"
"target is ACCEPT.\n"
"For example,\n"
"--idnat-list 2=20:21:22:23:24:25,4=_,7=30:31:32:33:34:35,\n"
"is valid.\n"
"The subnet MUST be specified. Only packets with 3 first bytes of their\n"
"%s address are considered. --i%1.1snat-sub parameter must begin with\n"
"3 integers separated by dots. Only they are considered, the rest is ignored.\n"
"No matter if you write '192.168.42.', '192.168.42.0', '192.168.42.12',\n"
"'192.168.42.0/24', '192.168.42.0/23' or '192.168.42.i%1.1snat_sucks!!!',\n"
"The numbers 192, 168 and 42 are taken and it behaves like a /24 IP subnet.\n"
"--i%1.1snat-default-target affects only the packet not matching the subnet.\n",
		cas, cas, cas, cas, cas, cas, cas, cas,
		cas, cas, cas, cas, cas, cas, cas, cas
	);
}

static void print_help_s()
{
	print_help_common("src");
}

static void print_help_d()
{
	print_help_common("dest");
}

static void init_s(struct ebt_entry_target *target)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)target->data;

	s_sub_supplied = 0;
	memset(natinfo, 0, sizeof(struct ebt_inat_info));
	natinfo->target = EBT_CONTINUE;
	return;
}

static void init_d(struct ebt_entry_target *target)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)target->data;

	d_sub_supplied = 0;
	memset(natinfo, 0, sizeof(struct ebt_inat_info));
	natinfo->target = EBT_CONTINUE;
	return;
}

static void parse_list(const char *arg, struct ebt_inat_info *info)
{
	int i;
	char c;
	int count = 0;
	int now_index = 1;
	int index;
	char buf[4];
	unsigned char mac[6];
	int ibuf = 0;
	int imac = 0;
	int target;
	memset(buf, 0, 4);
	i = 0;
	while (1) {
		c = arg[i];
		if (now_index) {
			if (isdigit(c)) {
				buf[ibuf++] = c;
				if (ibuf > 3) {
					print_error("Index too long at position %d", i);
				}
				goto next;
			}
			if (c == '=') {
				if (ibuf == 0) {
					print_error("Integer index expected before '=' at position %d", i);
				}
				buf[ibuf] = 0;
				ibuf = 0;
				index = atoi(buf);
				if (index < 0 || 255 < index) {
					print_error("Index out of range [0..255], namely %d", index);
				}
				now_index = 0;
				memset(mac, 0, 6);
				imac = 0;
				target = EBT_CONTINUE;
				goto next;
			}
			if (c == '\0') {
				goto next;
			}
			print_error("Unexpected '%c' where integer or '=' expected", c);
		}
		else {
			if (isxdigit(c)) {
				buf[ibuf++] = c;
				if (ibuf > 2) {
					print_error("MAC address chunk too long at position %d", i);
				}
				goto next;
			}
			if (c == ':' || c == ',' || c == '\0') {
				buf[ibuf] = 0;
				ibuf = 0;
				mac[imac++] = strtol(buf, 0, 16);
				if (c == ',' || c == '\0') {
					info->a[index].enabled = 1;
					info->a[index].target = target;
					memcpy(info->a[index].mac, mac, 6);
					now_index = 1;
					count++;
					goto next;
				}
				if (c == ':' && imac >= 6) {
					print_error("Too many MAC address chunks at position %d", i);
				}
				goto next;
			}
			if (c == '_') {
				target = EBT_DROP;
				goto next;
			}
			if (c == '+') {
				target = EBT_ACCEPT;
				goto next;
			}
			print_error("Unexpected '%c' where hex digit, '_', '+', ',' or end of string expected", c);
		}
	next:
		if (!c) break;
		i++;
	}
	if (count == 0) {
		print_error("List empty");
	}
}

static uint32_t parse_ip(const char *s)
{
	int a0, a1, a2;
	char ip[4];
	sscanf(s, "%d.%d.%d", &a0, &a1, &a2);
	ip[0] = a0;
	ip[1] = a1;
	ip[2] = a2;
	ip[3] = 0;
	return *(uint32_t*)ip;
}

#define OPT_ISNAT         0x01
#define OPT_ISNAT_SUB     0x02
#define OPT_ISNAT_TARGET  0x04
static int parse_s(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)(*target)->data;

	switch (c) {
	case NAT_S:
		check_option(flags, OPT_ISNAT);
		parse_list(optarg, natinfo);
		break;
	case NAT_S_TARGET:
		check_option(flags, OPT_ISNAT_TARGET);
		if (FILL_TARGET(optarg, natinfo->target))
			print_error("Illegal --isnat-default-target target");
		break;
	case NAT_S_SUB:
		natinfo->ip_subnet = parse_ip(optarg);
		s_sub_supplied = 1;
		break;
	default:
		return 0;
	}
	return 1;
}

#define OPT_IDNAT        0x01
#define OPT_IDNAT_SUB    0x02
#define OPT_IDNAT_TARGET 0x04
static int parse_d(int c, char **argv, int argc,
   const struct ebt_u_entry *entry, unsigned int *flags,
   struct ebt_entry_target **target)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)(*target)->data;

	switch (c) {
	case NAT_D:
		check_option(flags, OPT_IDNAT);
		parse_list(optarg, natinfo);
		break;
	case NAT_D_TARGET:
		check_option(flags, OPT_IDNAT_TARGET);
		if (FILL_TARGET(optarg, natinfo->target))
			print_error("Illegal --idnat-default-target target");
		break;
	case NAT_D_SUB:
		natinfo->ip_subnet = parse_ip(optarg);
		d_sub_supplied = 1;
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check_s(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)target->data;

	if (BASE_CHAIN && natinfo->target == EBT_RETURN)
		print_error("--isnat-default-target RETURN not allowed on base chain");
	CLEAR_BASE_CHAIN_BIT;
	if ((hookmask & ~(1 << NF_BR_POST_ROUTING)) || strcmp(name, "nat"))
		print_error("Wrong chain for isnat");
	if (time == 0 && s_sub_supplied == 0)
		print_error("No isnat subnet supplied");
}

static void final_check_d(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_inat_info *natinfo = (struct ebt_inat_info *)target->data;

	if (BASE_CHAIN && natinfo->target == EBT_RETURN)
		print_error("--idnat-default-target RETURN not allowed on base chain");
	CLEAR_BASE_CHAIN_BIT;
	if (((hookmask & ~((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_OUT)))
	   || strcmp(name, "nat")) &&
	   ((hookmask & ~(1 << NF_BR_BROUTING)) || strcmp(name, "broute")))
		print_error("Wrong chain for idnat");
	if (time == 0 && d_sub_supplied == 0)
		print_error("No idnat subnet supplied");
}

static void print_list(const struct ebt_inat_info *info)
{
	int i;
	for (i = 0; i < 256; i++) {
		if (info->a[i].enabled) {
			printf("%d=", i);
			if (info->a[i].target == EBT_DROP) {
				printf("_");
			}
			else {
				if (info->a[i].target == EBT_ACCEPT) {
					printf("+");
				}
				print_mac(info->a[i].mac);
			}
			printf(",");
		}
	}
}

static void print_s(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_inat_info *info = (struct ebt_inat_info *)target->data;

	unsigned char sub[4];
	*(uint32_t*)sub = info->ip_subnet;
	printf("--isnat-sub %u.%u.%u.0/24", sub[0], sub[1], sub[2]);
	printf(" --isnat-list ");
	print_list(info);
	printf(" --isnat-default-target %s", TARGET_NAME(info->target));
}

static void print_d(const struct ebt_u_entry *entry,
   const struct ebt_entry_target *target)
{
	struct ebt_inat_info *info = (struct ebt_inat_info *)target->data;

	unsigned char sub[4];
	*(uint32_t*)sub = info->ip_subnet;
	printf("--idnat-sub %u.%u.%u.0/24", sub[0], sub[1], sub[2]);
	printf(" --idnat-list ");
	print_list(info);
	printf(" --idnat-default-target %s", TARGET_NAME(info->target));
}

static int compare(const struct ebt_entry_target *t1,
   const struct ebt_entry_target *t2)
{
	struct ebt_inat_info *natinfo1 = (struct ebt_inat_info *)t1->data;
	struct ebt_inat_info *natinfo2 = (struct ebt_inat_info *)t2->data;


	return !memcmp(natinfo1, natinfo2, sizeof(struct ebt_inat_info));
}

static struct ebt_u_target isnat_target =
{
	.name		= EBT_ISNAT_TARGET,
	.size		= sizeof(struct ebt_inat_info),
	.help		= print_help_s,
	.init		= init_s,
	.parse		= parse_s,
	.final_check	= final_check_s,
	.print		= print_s,
	.compare	= compare,
	.extra_ops	= opts_s,
};

static struct ebt_u_target idnat_target =
{
	.name		= EBT_IDNAT_TARGET,
	.size		= sizeof(struct ebt_inat_info),
	.help		= print_help_d,
	.init		= init_d,
	.parse		= parse_d,
	.final_check	= final_check_d,
	.print		= print_d,
	.compare	= compare,
	.extra_ops	= opts_d,
};

static void _INIT(void)
{
	register_target(&isnat_target);
	register_target(&idnat_target);
}
