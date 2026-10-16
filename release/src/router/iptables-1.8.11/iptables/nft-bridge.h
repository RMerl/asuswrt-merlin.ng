#ifndef _NFT_BRIDGE_H_
#define _NFT_BRIDGE_H_

#include <netinet/in.h>
//#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/nf_tables.h>
#include <net/ethernet.h>
#include <libiptc/libxtc.h>

extern unsigned char eb_mac_type_unicast[ETH_ALEN];
extern unsigned char eb_msk_type_unicast[ETH_ALEN];
extern unsigned char eb_mac_type_multicast[ETH_ALEN];
extern unsigned char eb_msk_type_multicast[ETH_ALEN];
extern unsigned char eb_mac_type_broadcast[ETH_ALEN];
extern unsigned char eb_msk_type_broadcast[ETH_ALEN];
extern unsigned char eb_mac_type_bridge_group[ETH_ALEN];
extern unsigned char eb_msk_type_bridge_group[ETH_ALEN];

int ebt_get_mac_and_mask(const char *from, unsigned char *to, unsigned char *mask);

/* From: include/linux/netfilter_bridge/ebtables.h
 *
 * Adapted for the need of the ebtables-compat.
 */

#define EBT_TABLE_MAXNAMELEN 32
#define EBT_FUNCTION_MAXNAMELEN EBT_TABLE_MAXNAMELEN

/* verdicts >0 are "branches" */
#define EBT_ACCEPT   -1
#define EBT_DROP     -2
#define EBT_CONTINUE -3
#define EBT_RETURN   -4
#define NUM_STANDARD_TARGETS   4

#define EBT_ENTRY_OR_ENTRIES 0x01
/* these are the normal masks */
#define EBT_NOPROTO 0x02
#define EBT_802_3 0x04
#define EBT_SOURCEMAC 0x08
#define EBT_DESTMAC 0x10
#define EBT_F_MASK (EBT_NOPROTO | EBT_802_3 | EBT_SOURCEMAC | EBT_DESTMAC \
   | EBT_ENTRY_OR_ENTRIES)

#define EBT_IPROTO 0x01
#define EBT_IIN 0x02
#define EBT_IOUT 0x04
#define EBT_ISOURCE 0x8
#define EBT_IDEST 0x10
#define EBT_ILOGICALIN 0x20
#define EBT_ILOGICALOUT 0x40
#define EBT_INV_MASK (EBT_IPROTO | EBT_IIN | EBT_IOUT | EBT_ILOGICALIN \
   | EBT_ILOGICALOUT | EBT_ISOURCE | EBT_IDEST)

/* ebtables target modules store the verdict inside an int. We can
 * reclaim a part of this int for backwards compatible extensions.
 * The 4 lsb are more than enough to store the verdict.
 */
#define EBT_VERDICT_BITS 0x0000000F

struct nftnl_rule;
struct iptables_command_state;

static const char *ebt_standard_targets[NUM_STANDARD_TARGETS] = {
	"ACCEPT",
	"DROP",
	"CONTINUE",
	"RETURN",
};

static inline const char *nft_ebt_standard_target(unsigned int num)
{
	if (num >= NUM_STANDARD_TARGETS)
		return NULL;

	return ebt_standard_targets[num];
}

static inline int ebt_fill_target(const char *str, unsigned int *verdict)
{
	int i, ret = 0;

	for (i = 0; i < NUM_STANDARD_TARGETS; i++) {
		if (!strcmp(str, nft_ebt_standard_target(i))) {
			*verdict = -i - 1;
			break;
		}
	}

	if (i == NUM_STANDARD_TARGETS)
		ret = 1;

	return ret;
}

static inline const char *ebt_target_name(unsigned int verdict)
{
	return nft_ebt_standard_target(-verdict - 1);
}

#define EBT_CHECK_OPTION(flags, mask) ({			\
	if (*flags & mask)					\
		xtables_error(PARAMETER_PROBLEM,		\
			      "Multiple use of same "		\
			      "option not allowed");		\
	*flags |= mask;						\
})								\

void ebt_cs_clean(struct iptables_command_state *cs);
struct xtables_match *ebt_add_match(struct xtables_match *m,
				    struct iptables_command_state *cs);
struct xtables_target *ebt_add_watcher(struct xtables_target *watcher,
				       struct iptables_command_state *cs);
int ebt_command_default(struct iptables_command_state *cs,
			struct xtables_globals *unused, bool ebt_invert);

struct nft_among_pair {
	struct ether_addr ether;
	struct in_addr in __attribute__((aligned (4)));
};

struct nft_among_data {
	struct {
		size_t cnt;
		bool inv;
		bool ip;
	} src, dst;
	/* first source, then dest pairs */
	struct nft_among_pair pairs[0];
};

/* initialize fields, return offset into pairs array to write pairs to */
static inline size_t
nft_among_prepare_data(struct nft_among_data *data, bool dst,
		       size_t cnt, bool inv, bool ip)
{
	size_t poff;

	if (dst) {
		data->dst.cnt = cnt;
		data->dst.inv = inv;
		data->dst.ip = ip;
		poff = data->src.cnt;
	} else {
		data->src.cnt = cnt;
		data->src.inv = inv;
		data->src.ip = ip;
		poff = 0;
		memmove(data->pairs + cnt, data->pairs,
			data->dst.cnt * sizeof(*data->pairs));
	}
	return poff;
}

static inline void
nft_among_insert_pair(struct nft_among_pair *pairs,
		      size_t *pcount, const struct nft_among_pair *new)
{
	int i;

	/* nftables automatically sorts set elements from smallest to largest,
	 * insert sorted so extension comparison works */

	for (i = 0; i < *pcount; i++) {
		if (memcmp(new, &pairs[i], sizeof(*new)) < 0)
			break;
	}
	memmove(&pairs[i + 1], &pairs[i], sizeof(*pairs) * (*pcount - i));
	memcpy(&pairs[i], new, sizeof(*new));
	(*pcount)++;
}

/* from xtables-eb.c */
void nft_bridge_print_help(struct iptables_command_state *cs);

#endif
