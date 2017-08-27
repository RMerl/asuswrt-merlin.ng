#include <net/if.h>
#include <errno.h>
#include <string.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

SECTION(wowlan);

static int handle_wowlan_enable(struct nl80211_state *state, struct nl_cb *cb,
				struct nl_msg *msg, int argc, char **argv,
				enum id_input id)
{
	struct nlattr *wowlan, *pattern;
	struct nl_msg *patterns = NULL;
	enum {
		PS_REG,
		PS_PAT,
	} parse_state = PS_REG;
	int err = -ENOBUFS;
	unsigned char *pat, *mask;
	size_t patlen;
	int patnum = 0;

	wowlan = nla_nest_start(msg, NL80211_ATTR_WOWLAN_TRIGGERS);
	if (!wowlan)
		return -ENOBUFS;

	while (argc) {
		switch (parse_state) {
		case PS_REG:
			if (strcmp(argv[0], "any") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_ANY);
			else if (strcmp(argv[0], "disconnect") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_DISCONNECT);
			else if (strcmp(argv[0], "magic-packet") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_MAGIC_PKT);
			else if (strcmp(argv[0], "gtk-rekey-failure") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE);
			else if (strcmp(argv[0], "eap-identity-request") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST);
			else if (strcmp(argv[0], "4way-handshake") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE);
			else if (strcmp(argv[0], "rfkill-release") == 0)
				NLA_PUT_FLAG(msg, NL80211_WOWLAN_TRIG_RFKILL_RELEASE);
			else if (strcmp(argv[0], "patterns") == 0) {
				parse_state = PS_PAT;
				patterns = nlmsg_alloc();
				if (!patterns) {
					err = -ENOMEM;
					goto nla_put_failure;
				}
			} else {
				err = 1;
				goto nla_put_failure;
			}
			break;
		case PS_PAT:
			if (parse_hex_mask(argv[0], &pat, &patlen, &mask)) {
				err = 1;
				goto nla_put_failure;
			}
			pattern = nla_nest_start(patterns, ++patnum);
			NLA_PUT(patterns, NL80211_WOWLAN_PKTPAT_MASK,
				DIV_ROUND_UP(patlen, 8), mask);
			NLA_PUT(patterns, NL80211_WOWLAN_PKTPAT_PATTERN,
				patlen, pat);
			nla_nest_end(patterns, pattern);
			free(mask);
			free(pat);
			break;
		}
		argv++;
		argc--;
	}

	if (patterns)
		nla_put_nested(msg, NL80211_WOWLAN_TRIG_PKT_PATTERN,
				patterns);

	nla_nest_end(msg, wowlan);
	err = 0;
 nla_put_failure:
	nlmsg_free(patterns);
	return err;
}
COMMAND(wowlan, enable, "[any] [disconnect] [magic-packet] [gtk-rekey-failure] [eap-identity-request]"
	" [4way-handshake] [rfkill-release] [patterns <pattern>*]",
	NL80211_CMD_SET_WOWLAN, 0, CIB_PHY, handle_wowlan_enable,
	"Enable WoWLAN with the given triggers.\n"
	"Each pattern is given as a bytestring with '-' in places where any byte\n"
	"may be present, e.g. 00:11:22:-:44 will match 00:11:22:33:44 and\n"
	"00:11:22:33:ff:44 etc.");


static int handle_wowlan_disable(struct nl80211_state *state, struct nl_cb *cb,
				 struct nl_msg *msg, int argc, char **argv,
				 enum id_input id)
{
	/* just a set w/o wowlan attribute */
	return 0;
}
COMMAND(wowlan, disable, "", NL80211_CMD_SET_WOWLAN, 0, CIB_PHY, handle_wowlan_disable,
	"Disable WoWLAN.");


static int print_wowlan_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[NL80211_ATTR_MAX + 1];
	struct nlattr *trig[NUM_NL80211_WOWLAN_TRIG];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *pattern;
	int rem_pattern;

	nla_parse(attrs, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[NL80211_ATTR_WOWLAN_TRIGGERS]) {
		printf("WoWLAN is disabled.\n");
		return NL_SKIP;
	}

	/* XXX: use policy */
	nla_parse(trig, MAX_NL80211_WOWLAN_TRIG,
		  nla_data(attrs[NL80211_ATTR_WOWLAN_TRIGGERS]),
		  nla_len(attrs[NL80211_ATTR_WOWLAN_TRIGGERS]),
		  NULL);

	printf("WoWLAN is enabled:\n");
	if (trig[NL80211_WOWLAN_TRIG_ANY])
		printf(" * wake up on special any trigger\n");
	if (trig[NL80211_WOWLAN_TRIG_DISCONNECT])
		printf(" * wake up on disconnect\n");
	if (trig[NL80211_WOWLAN_TRIG_MAGIC_PKT])
		printf(" * wake up on magic packet\n");
	if (trig[NL80211_WOWLAN_TRIG_GTK_REKEY_FAILURE])
		printf(" * wake up on GTK rekeying failure\n");
	if (trig[NL80211_WOWLAN_TRIG_EAP_IDENT_REQUEST])
		printf(" * wake up on EAP identity request\n");
	if (trig[NL80211_WOWLAN_TRIG_4WAY_HANDSHAKE])
		printf(" * wake up on 4-way handshake\n");
	if (trig[NL80211_WOWLAN_TRIG_RFKILL_RELEASE])
		printf(" * wake up on RF-kill release\n");
	if (trig[NL80211_WOWLAN_TRIG_PKT_PATTERN]) {
		nla_for_each_nested(pattern,
				    trig[NL80211_WOWLAN_TRIG_PKT_PATTERN],
				    rem_pattern) {
			struct nlattr *patattr[NUM_NL80211_WOWLAN_PKTPAT];
			int i, patlen, masklen;
			uint8_t *mask, *pat;
			nla_parse(patattr, MAX_NL80211_WOWLAN_PKTPAT,
				  nla_data(pattern), nla_len(pattern),
				  NULL);
			if (!patattr[NL80211_WOWLAN_PKTPAT_MASK] ||
			    !patattr[NL80211_WOWLAN_PKTPAT_PATTERN]) {
				printf(" * (invalid pattern specification)\n");
				continue;
			}
			masklen = nla_len(patattr[NL80211_WOWLAN_PKTPAT_MASK]);
			patlen = nla_len(patattr[NL80211_WOWLAN_PKTPAT_PATTERN]);
			if (DIV_ROUND_UP(patlen, 8) != masklen) {
				printf(" * (invalid pattern specification)\n");
				continue;
			}
			printf(" * wake up on pattern: ");
			pat = nla_data(patattr[NL80211_WOWLAN_PKTPAT_PATTERN]);
			mask = nla_data(patattr[NL80211_WOWLAN_PKTPAT_MASK]);
			for (i = 0; i < patlen; i++) {
				if (mask[i / 8] & (1 << (i % 8)))
					printf("%.2x", pat[i]);
				else
					printf("--");
				if (i != patlen - 1)
					printf(":");
			}
			printf("\n");
		}
	}

	return NL_SKIP;
}

static int handle_wowlan_show(struct nl80211_state *state, struct nl_cb *cb,
			      struct nl_msg *msg, int argc, char **argv,
			      enum id_input id)
{
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
		  print_wowlan_handler, NULL);

	return 0;
}
COMMAND(wowlan, show, "", NL80211_CMD_GET_WOWLAN, 0, CIB_PHY, handle_wowlan_show,
	"Show WoWLAN status.");
