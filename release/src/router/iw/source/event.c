#include <stdint.h>
#include <stdbool.h>
#include <net/if.h>
#include <errno.h>
#include "iw.h"

static int no_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

struct ieee80211_beacon_channel {
	__u16 center_freq;
	bool passive_scan;
	bool no_ibss;
};

static int parse_beacon_hint_chan(struct nlattr *tb,
				  struct ieee80211_beacon_channel *chan)
{
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	static struct nla_policy beacon_freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
		[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
	};

	if (nla_parse_nested(tb_freq,
			     NL80211_FREQUENCY_ATTR_MAX,
			     tb,
			     beacon_freq_policy))
		return -EINVAL;

	chan->center_freq = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_FREQ]);

	if (tb_freq[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN])
		chan->passive_scan = true;
	if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IBSS])
		chan->no_ibss = true;

	return 0;
}

static void print_frame(struct print_event_args *args, struct nlattr *attr)
{
	uint8_t *frame;
	size_t len;
	int i;
	char macbuf[6*3];
	uint16_t tmp;

	if (!attr)
		printf(" [no frame]");

	frame = nla_data(attr);
	len = nla_len(attr);

	if (len < 26) {
		printf(" [invalid frame: ");
		goto print_frame;
	}

	mac_addr_n2a(macbuf, frame + 10);
	printf(" %s -> ", macbuf);
	mac_addr_n2a(macbuf, frame + 4);
	printf("%s", macbuf);

	switch (frame[0] & 0xfc) {
	case 0x10: /* assoc resp */
	case 0x30: /* reassoc resp */
		/* status */
		tmp = (frame[27] << 8) + frame[26];
		printf(" status: %d: %s", tmp, get_status_str(tmp));
		break;
	case 0x00: /* assoc req */
	case 0x20: /* reassoc req */
		break;
	case 0xb0: /* auth */
		/* status */
		tmp = (frame[29] << 8) + frame[28];
		printf(" status: %d: %s", tmp, get_status_str(tmp));
		break;
		break;
	case 0xa0: /* disassoc */
	case 0xc0: /* deauth */
		/* reason */
		tmp = (frame[25] << 8) + frame[24];
		printf(" reason %d: %s", tmp, get_reason_str(tmp));
		break;
	}

	if (!args->frame)
		return;

	printf(" [frame:");

 print_frame:
	for (i = 0; i < len; i++)
		printf(" %.02x", frame[i]);
	printf("]");
}

static void parse_cqm_event(struct nlattr **attrs)
{
	static struct nla_policy cqm_policy[NL80211_ATTR_CQM_MAX + 1] = {
		[NL80211_ATTR_CQM_RSSI_THOLD] = { .type = NLA_U32 },
		[NL80211_ATTR_CQM_RSSI_HYST] = { .type = NLA_U32 },
		[NL80211_ATTR_CQM_RSSI_THRESHOLD_EVENT] = { .type = NLA_U32 },
	};
	struct nlattr *cqm[NL80211_ATTR_CQM_MAX + 1];
	struct nlattr *cqm_attr = attrs[NL80211_ATTR_CQM];

	printf("connection quality monitor event: ");

	if (!cqm_attr ||
	    nla_parse_nested(cqm, NL80211_ATTR_CQM_MAX, cqm_attr, cqm_policy)) {
		printf("missing data!\n");
		return;
	}

	if (cqm[NL80211_ATTR_CQM_RSSI_THRESHOLD_EVENT]) {
		enum nl80211_cqm_rssi_threshold_event rssi_event;
		rssi_event = nla_get_u32(cqm[NL80211_ATTR_CQM_RSSI_THRESHOLD_EVENT]);
		if (rssi_event == NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH)
			printf("RSSI went above threshold\n");
		else
			printf("RSSI went below threshold\n");
	} else if (cqm[NL80211_ATTR_CQM_PKT_LOSS_EVENT] &&
		   attrs[NL80211_ATTR_MAC]) {
		uint32_t frames;
		char buf[3*6];

		frames = nla_get_u32(cqm[NL80211_ATTR_CQM_PKT_LOSS_EVENT]);
		mac_addr_n2a(buf, nla_data(attrs[NL80211_ATTR_MAC]));
		printf("peer %s didn't ACK %d packets\n", buf, frames);
	} else
		printf("unknown event\n");
}

static const char * key_type_str(enum nl80211_key_type key_type)
{
	static char buf[30];
	switch (key_type) {
	case NL80211_KEYTYPE_GROUP:
		return "Group";
	case NL80211_KEYTYPE_PAIRWISE:
		return "Pairwise";
	case NL80211_KEYTYPE_PEERKEY:
		return "PeerKey";
	default:
		snprintf(buf, sizeof(buf), "unknown(%d)", key_type);
		return buf;
	}
}

static void parse_mic_failure(struct nlattr **attrs)
{
	printf("Michael MIC failure event:");

	if (attrs[NL80211_ATTR_MAC]) {
		char addr[3 * ETH_ALEN];
		mac_addr_n2a(addr, nla_data(attrs[NL80211_ATTR_MAC]));
		printf(" source MAC address %s", addr);
	}

	if (attrs[NL80211_ATTR_KEY_SEQ] &&
	    nla_len(attrs[NL80211_ATTR_KEY_SEQ]) == 6) {
		unsigned char *seq = nla_data(attrs[NL80211_ATTR_KEY_SEQ]);
		printf(" seq=%02x%02x%02x%02x%02x%02x",
		       seq[0], seq[1], seq[2], seq[3], seq[4], seq[5]);
	}
	if (attrs[NL80211_ATTR_KEY_TYPE]) {
		enum nl80211_key_type key_type =
			nla_get_u32(attrs[NL80211_ATTR_KEY_TYPE]);
		printf(" Key Type %s", key_type_str(key_type));
	}

	if (attrs[NL80211_ATTR_KEY_IDX]) {
		__u8 key_id = nla_get_u8(attrs[NL80211_ATTR_KEY_IDX]);
		printf(" Key Id %d", key_id);
	}

	printf("\n");
}

static int print_event(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1], *nst;
	struct print_event_args *args = arg;
	char ifname[100];
	char macbuf[6*3];
	__u8 reg_type;
	struct ieee80211_beacon_channel chan_before_beacon,  chan_after_beacon;
	__u32 wiphy_idx = 0;
	int rem_nst;
	__u16 status;

	if (args->time || args->reltime) {
		unsigned long long usecs, previous;

		previous = 1000000ULL * args->ts.tv_sec + args->ts.tv_usec;
		gettimeofday(&args->ts, NULL);
		usecs = 1000000ULL * args->ts.tv_sec + args->ts.tv_usec;
		if (args->reltime) {
			if (!args->have_ts) {
				usecs = 0;
				args->have_ts = true;
			} else
				usecs -= previous;
		}
		printf("%llu.%06llu: ", usecs/1000000, usecs % 1000000);
	}

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (tb[NL80211_ATTR_IFINDEX] && tb[NL80211_ATTR_WIPHY]) {
		if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), ifname);
		printf("%s (phy #%d): ", ifname, nla_get_u32(tb[NL80211_ATTR_WIPHY]));
	} else if (tb[NL80211_ATTR_WDEV] && tb[NL80211_ATTR_WIPHY]) {
		printf("wdev 0x%llx (phy #%d): ",
			(unsigned long long)nla_get_u64(tb[NL80211_ATTR_WDEV]),
			nla_get_u32(tb[NL80211_ATTR_WIPHY]));
	} else if (tb[NL80211_ATTR_IFINDEX]) {
		if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), ifname);
		printf("%s: ", ifname);
	} else if (tb[NL80211_ATTR_WDEV]) {
		printf("wdev 0x%llx: ", (unsigned long long)nla_get_u64(tb[NL80211_ATTR_WDEV]));
	} else if (tb[NL80211_ATTR_WIPHY]) {
		printf("phy #%d: ", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
	}

	switch (gnlh->cmd) {
	case NL80211_CMD_NEW_WIPHY:
		printf("renamed to %s\n", nla_get_string(tb[NL80211_ATTR_WIPHY_NAME]));
		break;
	case NL80211_CMD_TRIGGER_SCAN:
		printf("scan started\n");
		break;
	case NL80211_CMD_NEW_SCAN_RESULTS:
		printf("scan finished:");
	case NL80211_CMD_SCAN_ABORTED:
		if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED)
			printf("scan aborted:");
		if (tb[NL80211_ATTR_SCAN_FREQUENCIES]) {
			nla_for_each_nested(nst, tb[NL80211_ATTR_SCAN_FREQUENCIES], rem_nst)
				printf(" %d", nla_get_u32(nst));
			printf(",");
		}
		if (tb[NL80211_ATTR_SCAN_SSIDS]) {
			nla_for_each_nested(nst, tb[NL80211_ATTR_SCAN_SSIDS], rem_nst) {
				printf(" \"");
				print_ssid_escaped(nla_len(nst), nla_data(nst));
				printf("\"");
			}
		}
		printf("\n");
		break;
	case NL80211_CMD_REG_CHANGE:
		printf("regulatory domain change: ");

		reg_type = nla_get_u8(tb[NL80211_ATTR_REG_TYPE]);

		switch (reg_type) {
		case NL80211_REGDOM_TYPE_COUNTRY:
			printf("set to %s by %s request",
			       nla_get_string(tb[NL80211_ATTR_REG_ALPHA2]),
			       reg_initiator_to_string(nla_get_u8(tb[NL80211_ATTR_REG_INITIATOR])));
			if (tb[NL80211_ATTR_WIPHY])
				printf(" on phy%d", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
			break;
		case NL80211_REGDOM_TYPE_WORLD:
			printf("set to world roaming by %s request",
			       reg_initiator_to_string(nla_get_u8(tb[NL80211_ATTR_REG_INITIATOR])));
			break;
		case NL80211_REGDOM_TYPE_CUSTOM_WORLD:
			printf("custom world roaming rules in place on phy%d by %s request",
			       nla_get_u32(tb[NL80211_ATTR_WIPHY]),
			       reg_initiator_to_string(nla_get_u32(tb[NL80211_ATTR_REG_INITIATOR])));
			break;
		case NL80211_REGDOM_TYPE_INTERSECTION:
			printf("intersection used due to a request made by %s",
			       reg_initiator_to_string(nla_get_u32(tb[NL80211_ATTR_REG_INITIATOR])));
			if (tb[NL80211_ATTR_WIPHY])
				printf(" on phy%d", nla_get_u32(tb[NL80211_ATTR_WIPHY]));
			break;
		default:
			printf("unknown source (upgrade this utility)");
			break;
		}

		printf("\n");
		break;
	case NL80211_CMD_REG_BEACON_HINT:

		wiphy_idx = nla_get_u32(tb[NL80211_ATTR_WIPHY]);

		memset(&chan_before_beacon, 0, sizeof(chan_before_beacon));
		memset(&chan_after_beacon, 0, sizeof(chan_after_beacon));

		if (parse_beacon_hint_chan(tb[NL80211_ATTR_FREQ_BEFORE],
					   &chan_before_beacon))
			break;
		if (parse_beacon_hint_chan(tb[NL80211_ATTR_FREQ_AFTER],
					   &chan_after_beacon))
			break;

		if (chan_before_beacon.center_freq != chan_after_beacon.center_freq)
			break;

		/* A beacon hint is sent _only_ if something _did_ change */
		printf("beacon hint:\n");

		printf("phy%d %d MHz [%d]:\n",
		       wiphy_idx,
		       chan_before_beacon.center_freq,
		       ieee80211_frequency_to_channel(chan_before_beacon.center_freq));

		if (chan_before_beacon.passive_scan && !chan_after_beacon.passive_scan)
			printf("\to active scanning enabled\n");
		if (chan_before_beacon.no_ibss && !chan_after_beacon.no_ibss)
			printf("\to beaconing enabled\n");

		break;
	case NL80211_CMD_NEW_STATION:
		mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
		printf("new station %s\n", macbuf);
		break;
	case NL80211_CMD_DEL_STATION:
		mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
		printf("del station %s\n", macbuf);
		break;
	case NL80211_CMD_JOIN_IBSS:
		mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
		printf("IBSS %s joined\n", macbuf);
		break;
	case NL80211_CMD_AUTHENTICATE:
		printf("auth");
		if (tb[NL80211_ATTR_FRAME])
			print_frame(args, tb[NL80211_ATTR_FRAME]);
		else if (tb[NL80211_ATTR_TIMED_OUT])
			printf(": timed out");
		else
			printf(": unknown event");
		printf("\n");
		break;
	case NL80211_CMD_ASSOCIATE:
		printf("assoc");
		if (tb[NL80211_ATTR_FRAME])
			print_frame(args, tb[NL80211_ATTR_FRAME]);
		else if (tb[NL80211_ATTR_TIMED_OUT])
			printf(": timed out");
		else
			printf(": unknown event");
		printf("\n");
		break;
	case NL80211_CMD_DEAUTHENTICATE:
		printf("deauth");
		print_frame(args, tb[NL80211_ATTR_FRAME]);
		printf("\n");
		break;
	case NL80211_CMD_DISASSOCIATE:
		printf("disassoc");
		print_frame(args, tb[NL80211_ATTR_FRAME]);
		printf("\n");
		break;
	case NL80211_CMD_UNPROT_DEAUTHENTICATE:
		printf("unprotected deauth");
		print_frame(args, tb[NL80211_ATTR_FRAME]);
		printf("\n");
		break;
	case NL80211_CMD_UNPROT_DISASSOCIATE:
		printf("unprotected disassoc");
		print_frame(args, tb[NL80211_ATTR_FRAME]);
		printf("\n");
		break;
	case NL80211_CMD_CONNECT:
		status = 0;
		if (!tb[NL80211_ATTR_STATUS_CODE])
			printf("unknown connect status");
		else if (nla_get_u16(tb[NL80211_ATTR_STATUS_CODE]) == 0)
			printf("connected");
		else {
			status = nla_get_u16(tb[NL80211_ATTR_STATUS_CODE]);
			printf("failed to connect");
		}
		if (tb[NL80211_ATTR_MAC]) {
			mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
			printf(" to %s", macbuf);
		}
		if (status)
			printf(", status: %d: %s", status, get_status_str(status));
		printf("\n");
		break;
	case NL80211_CMD_ROAM:
		printf("roamed");
		if (tb[NL80211_ATTR_MAC]) {
			mac_addr_n2a(macbuf, nla_data(tb[NL80211_ATTR_MAC]));
			printf(" to %s", macbuf);
		}
		printf("\n");
		break;
	case NL80211_CMD_DISCONNECT:
		printf("disconnected");
		if (tb[NL80211_ATTR_DISCONNECTED_BY_AP])
			printf(" (by AP)");
		else
			printf(" (local request)");
		if (tb[NL80211_ATTR_REASON_CODE])
			printf(" reason: %d: %s", nla_get_u16(tb[NL80211_ATTR_REASON_CODE]),
				get_reason_str(nla_get_u16(tb[NL80211_ATTR_REASON_CODE])));
		printf("\n");
		break;
	case NL80211_CMD_REMAIN_ON_CHANNEL:
		printf("remain on freq %d (%dms, cookie %llx)\n",
			nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]),
			nla_get_u32(tb[NL80211_ATTR_DURATION]),
			(unsigned long long)nla_get_u64(tb[NL80211_ATTR_COOKIE]));
		break;
	case NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL:
		printf("done with remain on freq %d (cookie %llx)\n",
			nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]),
			(unsigned long long)nla_get_u64(tb[NL80211_ATTR_COOKIE]));
		break;
	case NL80211_CMD_NOTIFY_CQM:
		parse_cqm_event(tb);
		break;
	case NL80211_CMD_MICHAEL_MIC_FAILURE:
		parse_mic_failure(tb);
		break;
	case NL80211_CMD_FRAME_TX_STATUS:
		printf("mgmt TX status (cookie %llx): %s\n",
			(unsigned long long)nla_get_u64(tb[NL80211_ATTR_COOKIE]),
			tb[NL80211_ATTR_ACK] ? "acked" : "no ack");
		break;
	case NL80211_ATTR_PMKSA_CANDIDATE:
		printf("PMKSA candidate found\n");
		break;
	default:
		printf("unknown event %d\n", gnlh->cmd);
		break;
	}

	fflush(stdout);
	return NL_SKIP;
}

struct wait_event {
	int n_cmds;
	const __u32 *cmds;
	__u32 cmd;
	struct print_event_args *pargs;
};

static int wait_event(struct nl_msg *msg, void *arg)
{
	struct wait_event *wait = arg;
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int i;

	for (i = 0; i < wait->n_cmds; i++) {
		if (gnlh->cmd == wait->cmds[i]) {
			wait->cmd = gnlh->cmd;
			if (wait->pargs)
				print_event(msg, wait->pargs);
		}
	}

	return NL_SKIP;
}

int __prepare_listen_events(struct nl80211_state *state)
{
	int mcid, ret;

	/* Configuration multicast group */
	mcid = nl_get_multicast_id(state->nl_sock, "nl80211", "config");
	if (mcid < 0)
		return mcid;

	ret = nl_socket_add_membership(state->nl_sock, mcid);
	if (ret)
		return ret;

	/* Scan multicast group */
	mcid = nl_get_multicast_id(state->nl_sock, "nl80211", "scan");
	if (mcid >= 0) {
		ret = nl_socket_add_membership(state->nl_sock, mcid);
		if (ret)
			return ret;
	}

	/* Regulatory multicast group */
	mcid = nl_get_multicast_id(state->nl_sock, "nl80211", "regulatory");
	if (mcid >= 0) {
		ret = nl_socket_add_membership(state->nl_sock, mcid);
		if (ret)
			return ret;
	}

	/* MLME multicast group */
	mcid = nl_get_multicast_id(state->nl_sock, "nl80211", "mlme");
	if (mcid >= 0) {
		ret = nl_socket_add_membership(state->nl_sock, mcid);
		if (ret)
			return ret;
	}

	return 0;
}

__u32 __do_listen_events(struct nl80211_state *state,
			 const int n_waits, const __u32 *waits,
			 struct print_event_args *args)
{
	struct nl_cb *cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	struct wait_event wait_ev;

	if (!cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		return -ENOMEM;
	}

	/* no sequence checking for multicast messages */
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);

	if (n_waits && waits) {
		wait_ev.cmds = waits;
		wait_ev.n_cmds = n_waits;
		wait_ev.pargs = args;
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, wait_event, &wait_ev);
	} else
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, print_event, args);

	wait_ev.cmd = 0;

	while (!wait_ev.cmd)
		nl_recvmsgs(state->nl_sock, cb);

	nl_cb_put(cb);

	return wait_ev.cmd;
}

__u32 listen_events(struct nl80211_state *state,
		    const int n_waits, const __u32 *waits)
{
	int ret;

	ret = __prepare_listen_events(state);
	if (ret)
		return ret;

	return __do_listen_events(state, n_waits, waits, NULL);
}

static int print_events(struct nl80211_state *state,
			struct nl_cb *cb,
			struct nl_msg *msg,
			int argc, char **argv,
			enum id_input id)
{
	struct print_event_args args;
	int ret;

	memset(&args, 0, sizeof(args));

	argc--;
	argv++;

	while (argc > 0) {
		if (strcmp(argv[0], "-f") == 0)
			args.frame = true;
		else if (strcmp(argv[0], "-t") == 0)
			args.time = true;
		else if (strcmp(argv[0], "-r") == 0)
			args.reltime = true;
		else
			return 1;
		argc--;
		argv++;
	}

	if (args.time && args.reltime)
		return 1;

	if (argc)
		return 1;

	ret = __prepare_listen_events(state);
	if (ret)
		return ret;

	return __do_listen_events(state, 0, NULL, &args);
}
TOPLEVEL(event, "[-t] [-r] [-f]", 0, 0, CIB_NONE, print_events,
	"Monitor events from the kernel.\n"
	"-t - print timestamp\n"
	"-r - print relative timstamp\n"
	"-f - print full frame for auth/assoc etc.");
