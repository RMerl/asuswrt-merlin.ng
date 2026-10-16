#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xshared.h"

#include "xtables-multi.h"

static const struct subcommand multi_subcommands[] = {
	{"iptables-xml",		iptables_xml_main},
	{"xml",				iptables_xml_main},
	{"iptables",			xtables_ip4_main},
	{"iptables-nft",		xtables_ip4_main},
	{"main4",			xtables_ip4_main},
	{"save4",			xtables_ip4_save_main},
	{"restore4",			xtables_ip4_restore_main},
	{"iptables-save",		xtables_ip4_save_main},
	{"iptables-restore",		xtables_ip4_restore_main},
	{"iptables-nft-save",	xtables_ip4_save_main},
	{"iptables-nft-restore",	xtables_ip4_restore_main},
	{"ip6tables",			xtables_ip6_main},
	{"ip6tables-nft",		xtables_ip6_main},
	{"main6",			xtables_ip6_main},
	{"save6",			xtables_ip6_save_main},
	{"restore6",			xtables_ip6_restore_main},
	{"ip6tables-save",		xtables_ip6_save_main},
	{"ip6tables-restore",		xtables_ip6_restore_main},
	{"ip6tables-nft-save",	xtables_ip6_save_main},
	{"ip6tables-nft-restore",	xtables_ip6_restore_main},
	{"iptables-translate",		xtables_ip4_xlate_main},
	{"ip6tables-translate",		xtables_ip6_xlate_main},
	{"iptables-restore-translate",	xtables_ip4_xlate_restore_main},
	{"ip6tables-restore-translate",	xtables_ip6_xlate_restore_main},
	{"arptables-translate",		xtables_arp_xlate_main},
	{"arptables",			xtables_arp_main},
	{"arptables-nft",		xtables_arp_main},
	{"arptables-restore",		xtables_arp_restore_main},
	{"arptables-nft-restore",	xtables_arp_restore_main},
	{"arptables-save",		xtables_arp_save_main},
	{"arptables-nft-save",		xtables_arp_save_main},
	{"ebtables-translate",		xtables_eb_xlate_main},
	{"ebtables",			xtables_eb_main},
	{"ebtables-restore",		xtables_eb_restore_main},
	{"ebtables-save",		xtables_eb_save_main},
	{"ebtables-nft",		xtables_eb_main},
	{"ebtables-nft-restore",	xtables_eb_restore_main},
	{"ebtables-nft-save",		xtables_eb_save_main},
	{"xtables-monitor",		xtables_monitor_main},
	{NULL},
};

int main(int argc, char **argv)
{
	return subcmd_main(argc, argv, multi_subcommands);
}
