#include <net/if.h>
#include <errno.h>
#include <string.h>

#include "nl80211.h"
#include "iw.h"

SECTION(ocb);

static int join_ocb(struct nl80211_state *state, struct nl_cb *cb,
		    struct nl_msg *msg, int argc, char **argv,
		    enum id_input id)
{
	unsigned long freq;
	char *end;
	int i;
	static const struct {
		const char *name;
		unsigned int width;
	} *chanmode_selected, chanmode[] = {
		{ .name = "5MHZ",
		  .width = NL80211_CHAN_WIDTH_5	},
		{ .name = "10MHZ",
		  .width = NL80211_CHAN_WIDTH_10 },
	};

	if (argc < 2)
		return 1;

	/* freq */
	freq = strtoul(argv[0], &end, 10);
	if (*end != '\0')
		return 1;

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	argv++;
	argc--;

	/* channel width */
	for (i = 0; i < ARRAY_SIZE(chanmode); i++) {
		if (strcasecmp(chanmode[i].name, argv[0]) == 0) {
			chanmode_selected = &chanmode[i];
			break;
		}
	}
	if (chanmode_selected) {
		NLA_PUT_U32(msg, NL80211_ATTR_CHANNEL_WIDTH,
			    chanmode_selected->width);
		NLA_PUT_U32(msg, NL80211_ATTR_CENTER_FREQ1, freq);

		argv++;
		argc--;
	} else {
		return 1;
	}

	return 0;

nla_put_failure:
	return -ENOBUFS;
}
COMMAND(ocb, join, "<freq in MHz> <5MHZ|10MHZ>",
	NL80211_CMD_JOIN_OCB, 0, CIB_NETDEV, join_ocb,
	"Join the OCB mode network.");

static int leave_ocb(struct nl80211_state *state, struct nl_cb *cb,
		     struct nl_msg *msg, int argc, char **argv,
		     enum id_input id)
{
	if (argc)
		return 1;

	return 0;
}
COMMAND(ocb, leave, NULL, NL80211_CMD_LEAVE_OCB, 0, CIB_NETDEV, leave_ocb,
	"Leave the OCB mode network.");
