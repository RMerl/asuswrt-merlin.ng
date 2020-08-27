#include <stdio.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac.h>
#include <iptables/internal.h>

enum
{
	O_SRC_MAC = 0,
	O_SRC_MASK = 1,
	O_DST_MAC = 2,
	O_DST_MASK = 3,
};

static void mac_help(void)
{
	printf(
		"MAC v%s options:\n"
		"[!] --mac-source XX:XX:XX:XX:XX:XX\n"
		"				Match source MAC address\n"
		" --src-mask XX:XX:XX:XX:XX:XX\n"
		"				Source MAC mask\n"
		" [!] --mac-dst XX:XX:XX:XX:XX:XX\n"
		"				Match destination MAC address\n"
		" --dst-mask XX:XX:XX:XX:XX:XX\n"
		"				Destination MAC mask\n"
		"\n", IPTABLES_VERSION);
}

#define s struct xt_mac_info
static const struct xt_option_entry mac_opts[] =
{
	{
		.name = "mac-source", .id = O_SRC_MAC, .type = XTTYPE_ETHERMAC,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(s, srcaddr)
	},
	{
		.name = "src-mask", .id = O_SRC_MASK, .type = XTTYPE_ETHERMAC,
		.flags = XTOPT_PUT,
		XTOPT_POINTER(s, srcmask)
	},
	{
		.name = "mac-dst", .id = O_DST_MAC, .type = XTTYPE_ETHERMAC,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(s, dstaddr)
	},
	{
		.name = "dst-mask", .id = O_DST_MASK, .type = XTTYPE_ETHERMAC,
		.flags = XTOPT_PUT,
		XTOPT_POINTER(s, dstmask)
	},
	XTOPT_TABLEEND,
};
#undef s

static void mac_parse(struct xt_option_call *cb)
{
	struct xt_mac_info *macinfo = cb->data;

	if(cb->entry->id == 0)  	//--mac-source
	{
		xtables_option_parse(cb);
		if(cb->invert)
		{
			macinfo->flags |= MAC_SRC_INV;
		}
		macinfo->flags |= MAC_SRC;
	}
	else if(cb->entry->id == 1)   //--src-mask
	{
		xtables_option_parse(cb);
		if(cb->invert)  	//note this option cannot use invert
		{
			xtables_error(PARAMETER_PROBLEM, "--src-mask: cannot use invert!");
		}
		macinfo->flags |= SRC_MASK;
	}
	else if(cb->entry->id == 2)   //--mac-dst
	{
		xtables_option_parse(cb);
		if(cb->invert)
		{
			macinfo->flags |= MAC_DST_INV;
		}
		macinfo->flags |= MAC_DST;
	}
	else if(cb->entry->id == 3)   //--dst-mask
	{
		xtables_option_parse(cb);
		if(cb->invert)  	//note this option cannot use invert
		{
			xtables_error(PARAMETER_PROBLEM, "--dst-mask: cannot use invert!");
		}
		macinfo->flags |= DST_MASK;
	}
	else
	{
		xtables_error(PARAMETER_PROBLEM, "mac match: unknown option!");
	}
}

static void print_mac(const unsigned char *macaddress)
{
	unsigned int i;

	printf(" %02X", macaddress[0]);
	for(i = 1; i < ETH_ALEN; ++i)
		printf(":%02X", macaddress[i]);
}

static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mac_info *info = (void *)match->data;

	if(info->flags & MAC_SRC)
	{
		printf(" SRC MAC");
		if(info->flags & MAC_SRC_INV)
		{
			printf(" !");
		}
		print_mac(info->srcaddr);

		if(info->flags & SRC_MASK)
		{
			printf(" SRC MASK");
			print_mac(info->srcmask);
		}
	}
	if(info->flags & MAC_DST)
	{
		printf(" DST MAC");
		if(info->flags & MAC_DST_INV)
		{
			printf(" !");
		}
		print_mac(info->dstaddr);

		if(info->flags & DST_MASK)
		{
			printf(" DST MASK");
			print_mac(info->dstmask);
		}
	}

}

static void mac_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mac_info *info = (void *)match->data;

	if(info->flags & MAC_SRC)
	{
		if(info->flags & MAC_SRC_INV)
		{
			printf(" !");
		}
		printf(" --mac-source");
		print_mac(info->srcaddr);

		if(info->flags & SRC_MASK)
		{
			printf(" --src-mask");
			print_mac(info->srcmask);
		}
	}

	if(info->flags & MAC_DST)
	{
		if(info->flags & MAC_DST_INV)
		{
			printf(" !");
		}
		printf(" --mac-dst");
		print_mac(info->dstaddr);

		if(info->flags & DST_MASK)
		{
			printf(" --dst-mask");
			print_mac(info->dstmask);
		}
	}
}

/* Final check; must have specified --mac. */
/* Final check; must have specified --mac-source or --mac-dst. */
static void final_check(unsigned int flags)
{
	int check;

	check = flags & (MAC_SRC | MAC_DST);

	if(!check)
		xtables_error(PARAMETER_PROBLEM, "You must specify `--mac-source' or 'mac-dst'");
}

static struct xtables_match mac_match =
{
	.family		= NFPROTO_UNSPEC,
	.name		= "mac",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info)),
	.help		= mac_help,
	.x6_parse	= mac_parse,
	.final_check	= final_check,
	.print		= mac_print,
	.save		= mac_save,
	.x6_options	= mac_opts,
};

void _init(void)
{
	xtables_register_match(&mac_match);
}
