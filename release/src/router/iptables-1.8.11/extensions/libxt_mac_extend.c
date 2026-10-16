
/* extend from libxt_mac.c for MAC address extend match operations,
 * i.e, MAC/mask. 
 * BRCM, Jan, 31. 2019.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <xtables.h>
#include <linux/netfilter/xt_mac_extend.h>

enum {
	O_MAC = 0,
};

static void mac_help(void)
{
	printf(
"mac extend match options:\n"
"[!] --mac-source-extend[/mask] XX:XX:XX:XX:XX:XX/FF:FF:FF:00:00:00\n"
"				Match source MAC address/mask\n");
}

#define s struct xt_mac_info_extend
static const struct xt_option_entry mac_opts[] = {
	{.name = "mac-source-extend", .id = O_MAC, .type = XTTYPE_ETHERMAC,
	 .flags = XTOPT_MAND | XTOPT_INVERT | XTOPT_PUT,
	 XTOPT_POINTER(s, srcaddr)},
	XTOPT_TABLEEND,
};
#undef s

static int mac_mask(char *mask_in, unsigned char *mask_out)
{
	const char *arg = mask_in;
	unsigned int i;
	char *end;

	for (i = 0; i < ETH_ALEN; ++i) {
		mask_out[i] = strtoul(arg, &end, 16);
		if (((i < (ETH_ALEN - 1)) && (*end != ':')) ||
			((i == (ETH_ALEN - 1)) && (*end != '\0' )) || 
			(end - arg > 2))
			return -1;
		arg = end + 1;
	}
	return 0;
}

static void mac_parse(struct xt_option_call *cb)
{
	struct xt_mac_info_extend *macinfo = cb->data;
	char *p;

	/* first the mask */
	if ((p = strrchr(cb->arg, '/')) != NULL) {
		*p = '\0';
		if (mac_mask(p + 1, macinfo->msk)) {
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with the MAC mask '%s'", p + 1);
			return;
		}
	} else {
		/* default mask = FF:FF:FF:FF:FF:FF */
		int i;
		for (i = 0; i < ETH_ALEN; i++) {
			macinfo->msk[i] = 0xFF;
		}
	}
    
	xtables_option_parse(cb);
	if (cb->invert)
		macinfo->invert = 1;
}

static void print_mac(const unsigned char *macaddress)
{
	unsigned int i;

	printf(" %02X", macaddress[0]);
	for (i = 1; i < ETH_ALEN; ++i)
		printf(":%02X", macaddress[i]);
}

static void
mac_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_mac_info_extend *info = (void *)match->data;

	printf(" MAC");

	if (info->invert)
		printf(" !");

	print_mac(info->srcaddr);
	printf(" /");
	print_mac(info->msk);
}

static void mac_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_mac_info_extend *info = (void *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --mac-source");
	print_mac(info->srcaddr);
}

static void print_mac_xlate(const unsigned char *macaddress,
			    struct xt_xlate *xl)
{
	unsigned int i;

	xt_xlate_add(xl, "%02x", macaddress[0]);
	for (i = 1; i < ETH_ALEN; ++i)
		xt_xlate_add(xl, ":%02x", macaddress[i]);
}

static int mac_xlate(struct xt_xlate *xl,
		     const struct xt_xlate_mt_params *params)
{
	const struct xt_mac_info_extend *info = (void *)params->match->data;

	xt_xlate_add(xl, "ether saddr%s ", info->invert ? " !=" : "");
	print_mac_xlate(info->srcaddr, xl);

	return 1;
}

static struct xtables_match mac_match = {
	.family		= NFPROTO_UNSPEC,
	.name		= "mac-extend",
	.version	= XTABLES_VERSION,
	.size		= XT_ALIGN(sizeof(struct xt_mac_info_extend)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_mac_info_extend)),
	.help		= mac_help,
	.x6_parse	= mac_parse,
	.print		= mac_print,
	.save		= mac_save,
	.x6_options	= mac_opts,
	.xlate		= mac_xlate,
};

void _init(void)
{
	xtables_register_match(&mac_match);
}
