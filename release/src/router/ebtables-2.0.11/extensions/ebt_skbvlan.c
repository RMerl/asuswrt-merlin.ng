/*
 * ebt_skbvlan
 * Description: EBTables skbvlan extension module for userspace.
 * Authors:  Jack Chang <jack.chang@broadcom.com>, ported from ebt_vlan.c
 *           The following is the original disclaimer.
 */
/* ebt_vlan
 * 
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 * Nick Fedchik <nick@fedchik.org.ua> 
 *
 * June, 2002
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "../include/ebtables_u.h"
#include "../include/ethernetdb.h"
#include <include/uapi/linux/netfilter_bridge/ebt_skbvlan_m.h>
#include <linux/if_ether.h>

#define NAME_VLAN_ID    "id"
#define NAME_VLAN_PRIO  "prio"
#define NAME_VLAN_ENCAP "encap"

#define VLAN_ID    '1'
#define VLAN_PRIO  '2'
#define VLAN_ENCAP '3'
#define VLAN_TAG_0 '4'
#define VLAN_TAG_1 '5'
#define VLAN_TAG_2 '6'
#define VLAN_TAG_3 '7'
#define VLAN_TPID_1 '8'
#define VLAN_TPID_0 '9'

#define MASK_VLAN_ALL 0xFFFFFFFF

static struct option opts[] = {
	{"skbvlan-id"   , required_argument, NULL, VLAN_ID},
	{"skbvlan-prio" , required_argument, NULL, VLAN_PRIO},
	{"skbvlan-encap", required_argument, NULL, VLAN_ENCAP},
	{"skbvlan-vlantag0", required_argument, NULL, VLAN_TAG_0},
	{"skbvlan-vlantag1", required_argument, NULL, VLAN_TAG_1},
	{"skbvlan-vlantag2", required_argument, NULL, VLAN_TAG_2},
	{"skbvlan-vlantag3", required_argument, NULL, VLAN_TAG_3},
	{"skbvlan-vlantpid0", required_argument, NULL, VLAN_TPID_0},
	{"skbvlan-vlantpid1", required_argument, NULL, VLAN_TPID_1},
	{ 0 }
};

/*
 * option inverse flags definition 
 */
#define OPT_VLAN_ID     0x0001
#define OPT_VLAN_PRIO   0x0002
#define OPT_VLAN_ENCAP  0x0004
#define OPT_VLAN_TAG_0  0x0008
#define OPT_VLAN_TAG_1  0x0010
#define OPT_VLAN_TAG_2  0x0020
#define OPT_VLAN_TAG_3  0x0040
#define OPT_VLAN_TPID_0  0x0080
#define OPT_VLAN_TPID_1  0x0100




#define OPT_VLAN_FLAGS	(OPT_VLAN_ID | OPT_VLAN_PRIO | OPT_VLAN_ENCAP | OPT_VLAN_TAG_0 \
	| OPT_VLAN_TAG_1 | OPT_VLAN_TAG_2 | OPT_VLAN_TAG_2 | OPT_VLAN_TPID_0 | \
	OPT_VLAN_TPID_1)

struct ethertypeent *ethent_skbvlan;

static uint32_t parse_vlantag(const char *vlanstr)
{
	char *end;
	uint32_t vlantag;

	vlantag = strtoul(vlanstr, &end, 16);
	if ((*end == '\0') || (*end == ':') || (*end == '/')) {
		return vlantag;
	}

	ebt_print_error("Problem with specified tos '%s'", vlantag);
	return 0;
}

static int parse_vlantag_mask(char *mask, uint32_t *mask2)
{
	char *end;

	*mask2 = (uint32_t)strtoul(mask, &end, 16);

	if (*end != '\0')
		ebt_print_error("Problem with specified vlantag mask 0x%08X", mask2);

	return 0;
}

static void
parse_vlantag_range_mask(const char *vlantagstring, uint32_t *vlantag, uint32_t *mask)
{
	char *buffer;
	char *cp;
	char *p;

	buffer = strdup(vlantagstring);
	p = strrchr(buffer, '/');
	cp = strchr(buffer, ':');

	if (cp == NULL) {
		vlantag[0] = vlantag[1] = parse_vlantag(buffer);
	} else {
		*cp = '\0';
		cp++;
		vlantag[0] = buffer[0] ? parse_vlantag(buffer) : 0;
        if (ebt_errormsg[0] != '\0')
			return;
		vlantag[1] = cp[0] ? parse_vlantag(cp):MASK_VLAN_ALL;
		if (ebt_errormsg[0] != '\0')
			return;
		
		if (vlantag[0] > vlantag[1])
			ebt_print_error("Invalid vlantag range (min > max)");
	}

	if (p != NULL) {
		parse_vlantag_mask(p + 1, (uint32_t *)mask);
	} else {
		*mask = MASK_VLAN_ALL;
	} 

	free(buffer);
}

static void print_vlantag_range_mask(uint32_t *vlantag, uint32_t mask)
{
	char str[128];
	int i;
	char * p = str;

	memset(str, 0, sizeof(str));
	if (vlantag[0] == vlantag[1])
		i = snprintf(str, sizeof(str), "0x%08X", vlantag[0]);
	else
		i = snprintf(str, sizeof(str),"0x%08X:0x%08X", vlantag[0], vlantag[1]);

	if (mask != MASK_VLAN_ALL)
		snprintf(p + i, sizeof(str) - i, "/0x%08X ", mask);
	else
		snprintf(p + i, sizeof(str) - i, " ");

	printf("%s", str);
}

/*********************************************************************
*  --skbvlan-vlantag0  find the first vlantag if the TPID is masked valid, if it has ! flag, it will search 
*  with the remained masked section. otherwise,  it will match the first vlantag according to the mask value
*
*  --skbvlan-vlantag1  find the second vlantag if the TPID is masked valid, if it has ! flag, it will search 
*  with the remained masked section. otherwise,  it will match the first vlantag according to the mask value
*
*  --skbvlan-vlantag2  find the first vlantag if the TPID is masked valid, if it has ! flag, it will search 
*  with the remained masked section. otherwise,  it will match the first vlantag according to the mask value
*
*  --skbvlan-vlantag3  find the second vlantag if the TPID is masked valid, if it has ! flag, it will search 
*  with the remained masked section. otherwise,  it will match the first vlantag according to the mask value
*
* --skbvlan-vlantpid0 find if the tpid exist in the vlantags.
*
* --skbvlan-vlantpid1 find if the tpid exist in the vlantags.
*
**********************************************************************/

static void print_help()
{
	printf(
"skbvlan options:\n"
"--skbvlan-id [!] id       : vlan-tagged frame identifier, 0,1-4096 (integer)\n"
"--skbvlan-prio [!] prio   : Priority-tagged frame's user priority, 0-7 (integer)\n"
"--skbvlan-encap [!] encap : Encapsulated frame protocol (hexadecimal or name)\n"
"--skbvlan-vlantag0 [!] vlantag[:vlantag][/mask] : first layer vlantag\n"
"--skbvlan-vlantag1 [!] vlantag[:vlantag][/mask] : second layer vlantag\n"
"--skbvlan-vlantag2 [!] vlantag[:vlantag][/mask] : first layer vlantag\n"
"--skbvlan-vlantag3 [!] vlantag[:vlantag][/mask] : second layer vlantag\n"
"--skbvlan-vlantpid0 [!] tpid : vlantag tpid\n"
"--skbvlan-vlantpid1 [!] tpid : vlantag tpid\n"
);
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_skbvlan_m_info *vlaninfo = (struct ebt_skbvlan_m_info *) match->data;

	memset(vlaninfo, 0, sizeof(struct ebt_skbvlan_m_info));
}


static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_skbvlan_m_info *vlaninfo = (struct ebt_skbvlan_m_info *) (*match)->data;
	char *end;
	struct ebt_skbvlan_m_info local;

	switch (c) {
	case VLAN_ID:
		ebt_check_option2(flags, OPT_VLAN_ID);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_ID;
		local.id = strtoul(optarg, &end, 10);
		if (local.id > 4094 || *end != '\0')
			ebt_print_error2("Invalid --skbvlan-id range ('%s')", optarg);
		vlaninfo->id = local.id;
		vlaninfo->bitmask |= EBT_SKBVLAN_ID;
		break;
	case VLAN_PRIO:
		ebt_check_option2(flags, OPT_VLAN_PRIO);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_PRIO;
		local.prio = strtoul(optarg, &end, 10);
		if (local.prio >= 8 || *end != '\0')
			ebt_print_error2("Invalid --skbvlan-prio range ('%s')", optarg);
		vlaninfo->prio = local.prio;
		vlaninfo->bitmask |= EBT_SKBVLAN_PRIO;
		break;
	case VLAN_ENCAP:
		ebt_check_option2(flags, OPT_VLAN_ENCAP);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_ENCAP;
		local.encap = strtoul(optarg, &end, 16);
		if (*end != '\0') {
			ethent_skbvlan = getethertypebyname(optarg);
			if (ethent_skbvlan == NULL)
				ebt_print_error("Unknown --skbvlan-encap value ('%s')", optarg);
			local.encap = ethent_skbvlan->e_ethertype;
		}
		if (local.encap < ETH_ZLEN)
			ebt_print_error2("Invalid --skbvlan-encap range ('%s')", optarg);
		vlaninfo->encap = htons(local.encap);
		vlaninfo->bitmask |= EBT_SKBVLAN_ENCAP;
		break;
	case VLAN_TAG_0:
		ebt_check_option2(flags, OPT_VLAN_TAG_0);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TAG_0;
		parse_vlantag_range_mask(optarg, vlaninfo->vlantag0, &vlaninfo->vlanmask0);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TAG_0;
		break;
	case VLAN_TAG_1:
		ebt_check_option2(flags, OPT_VLAN_TAG_1);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TAG_1;
		parse_vlantag_range_mask(optarg, vlaninfo->vlantag1, &vlaninfo->vlanmask1);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TAG_1;
		break;
	case VLAN_TAG_2:
		ebt_check_option2(flags, OPT_VLAN_TAG_2);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TAG_2;
		parse_vlantag_range_mask(optarg, vlaninfo->vlantag2, &vlaninfo->vlanmask2);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TAG_2;
		break;
	case VLAN_TAG_3:
		ebt_check_option2(flags, OPT_VLAN_TAG_3);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TAG_3;
		parse_vlantag_range_mask(optarg, vlaninfo->vlantag3, &vlaninfo->vlanmask3);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TAG_3;
		break;
	case VLAN_TPID_0:
		ebt_check_option2(flags, OPT_VLAN_TPID_0);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TPID_0;
		vlaninfo->vlantpid0 = strtoul(optarg, &end, 16);
		if (*end != '\0')
			ebt_print_error2("Invalid --skbvlan-vlantpid0 ('%s')", optarg);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TPID_0;
		break;
	case VLAN_TPID_1:
		ebt_check_option2(flags, OPT_VLAN_TPID_1);
		if (ebt_check_inverse2(optarg))
			vlaninfo->invflags |= EBT_SKBVLAN_VLAN_TPID_1;
		vlaninfo->vlantpid1 = strtoul(optarg, &end, 16);
		if (*end != '\0')
			ebt_print_error2("Invalid --skbvlan-vlantpid1 ('%s')", optarg);
		vlaninfo->bitmask |= EBT_SKBVLAN_VLAN_TPID_1;
		break;
	default:
		return 0;

	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match,
   const char *name, unsigned int hookmask, unsigned int time)
{
    return;
	//if (entry->ethproto != ETH_P_8021Q || entry->invflags & EBT_IPROTO)
		//ebt_print_error("For vlan filtering the protocol must be specified as 802_1Q");

	/* Check if specified vlan-id=0 (priority-tagged frame condition) 
	 * when vlan-prio was specified. */
	/* I see no reason why a user should be prohibited to match on a perhaps impossible situation <BDS>
	if (vlaninfo->bitmask & EBT_SKBVLAN_PRIO &&
	    vlaninfo->id && vlaninfo->bitmask & EBT_SKBVLAN_ID)
		ebt_print_error("When setting --vlan-prio the specified --vlan-id must be 0");*/
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_skbvlan_m_info *vlaninfo = (struct ebt_skbvlan_m_info *) match->data;

	if (vlaninfo->bitmask & EBT_SKBVLAN_ID) {
		printf("--skbvlan-id %s%d ", (vlaninfo->invflags & EBT_SKBVLAN_ID) ? "! " : "", vlaninfo->id);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_PRIO) {
		printf("--skbvlan-prio %s%d ", (vlaninfo->invflags & EBT_SKBVLAN_PRIO) ? "! " : "", vlaninfo->prio);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_ENCAP) {
		printf("--skbvlan-encap %s", (vlaninfo->invflags & EBT_SKBVLAN_ENCAP) ? "! " : "");
		ethent_skbvlan = getethertypebynumber(ntohs(vlaninfo->encap));
		if (ethent_skbvlan != NULL) {
			printf("%s ", ethent_skbvlan->e_name);
		} else {
			printf("%4.4X ", ntohs(vlaninfo->encap));
		}
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TAG_0) {
		printf("--skbvlan-vlantag0 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TAG_0)
			printf("! ");
		print_vlantag_range_mask(vlaninfo->vlantag0, vlaninfo->vlanmask0);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TAG_1) {
		printf("--skbvlan-vlantag1 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TAG_1)
			printf("! ");
		print_vlantag_range_mask(vlaninfo->vlantag1, vlaninfo->vlanmask1);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TAG_2) {
		printf("--skbvlan-vlantag2 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TAG_2)
			printf("! ");
		print_vlantag_range_mask(vlaninfo->vlantag2, vlaninfo->vlanmask2);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TAG_3) {
		printf("--skbvlan-vlantag3 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TAG_3)
			printf("! ");
		print_vlantag_range_mask(vlaninfo->vlantag3, vlaninfo->vlanmask3);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TPID_0) {
		printf("--skbvlan-vlantpid0 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TPID_0)
			printf("! ");
		printf("0x%04X ", vlaninfo->vlantpid0);
	}
	if (vlaninfo->bitmask & EBT_SKBVLAN_VLAN_TPID_1) {
		printf("--skbvlan-vlantpid1 ");
		if (vlaninfo->invflags & EBT_SKBVLAN_VLAN_TPID_1)
			printf("! ");
		printf("0x%04X ", vlaninfo->vlantpid1);
	}
}

static int compare(const struct ebt_entry_match *vlan1,
   const struct ebt_entry_match *vlan2)
{
	struct ebt_skbvlan_m_info *vlaninfo1 = (struct ebt_skbvlan_m_info *) vlan1->data;
	struct ebt_skbvlan_m_info *vlaninfo2 = (struct ebt_skbvlan_m_info *) vlan2->data;

	if (vlaninfo1->bitmask != vlaninfo2->bitmask)
		return 0;
	if (vlaninfo1->invflags != vlaninfo2->invflags)
		return 0;
	if (vlaninfo1->bitmask & EBT_SKBVLAN_ID &&
	    vlaninfo1->id != vlaninfo2->id)
		return 0;
	if (vlaninfo1->bitmask & EBT_SKBVLAN_PRIO &&
	    vlaninfo1->prio != vlaninfo2->prio)
		return 0;
	if (vlaninfo1->bitmask & EBT_SKBVLAN_ENCAP &&
	    vlaninfo1->encap != vlaninfo2->encap)
		return 0;
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TAG_0) {
		if (vlaninfo1->vlantag0[0] != vlaninfo2->vlantag0[0] ||
			vlaninfo1->vlantag0[1] != vlaninfo2->vlantag0[1] ||
			vlaninfo1->vlanmask0 != vlaninfo2->vlanmask0)
			return 0;
	}
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TAG_1) {
		if (vlaninfo1->vlantag1[0] != vlaninfo2->vlantag1[0] ||
			vlaninfo1->vlantag1[1] != vlaninfo2->vlantag1[1] ||
			vlaninfo1->vlanmask1 != vlaninfo2->vlanmask1)
			return 0;
	}
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TAG_2) {
		if (vlaninfo1->vlantag2[0] != vlaninfo2->vlantag2[0] ||
			vlaninfo1->vlantag2[1] != vlaninfo2->vlantag2[1] ||
			vlaninfo1->vlanmask2 != vlaninfo2->vlanmask2)
			return 0;
	}
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TAG_3) {
		if (vlaninfo1->vlantag3[0] != vlaninfo2->vlantag3[0] ||
			vlaninfo1->vlantag3[1] != vlaninfo2->vlantag3[1] ||
			vlaninfo1->vlanmask3 != vlaninfo2->vlanmask3)
			return 0;
	}
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TPID_0) {
	    if (vlaninfo1->vlantpid0 != vlaninfo2->vlantpid0)
			return 0;
	}
	if (vlaninfo1->bitmask & EBT_SKBVLAN_VLAN_TPID_1) {
	    if (vlaninfo1->vlantpid1 != vlaninfo2->vlantpid1)
			return 0;
	}
	// printf("[ebt_skbvlan]compare OK\n");
	return 1;
}

static struct ebt_u_match skbvlan_match = {
	.name		= "skbvlan",
	.size		= sizeof(struct ebt_skbvlan_m_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _INIT(void)
{
	ebt_register_match(&skbvlan_match);
}
