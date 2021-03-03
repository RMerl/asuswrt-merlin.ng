/*
 * p_ip6.c		packet editor: IPV6 header
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  Amir Vadai <amir@vadai.me>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"
#include "tc_util.h"
#include "m_pedit.h"

static int
parse_ip6(int *argc_p, char ***argv_p,
	  struct m_pedit_sel *sel, struct m_pedit_key *tkey)
{
	int res = -1;
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc < 2)
		return -1;

	if (!sel->extended)
		return -1;

	tkey->htype = TCA_PEDIT_KEY_EX_HDR_TYPE_IP6;

	if (strcmp(*argv, "src") == 0) {
		NEXT_ARG();
		tkey->off = 8;
		res = parse_cmd(&argc, &argv, 16, TIPV6, RU32, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "dst") == 0) {
		NEXT_ARG();
		tkey->off = 24;
		res = parse_cmd(&argc, &argv, 16, TIPV6, RU32, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "flow_lbl") == 0) {
		NEXT_ARG();
		tkey->off = 0;
		res = parse_cmd(&argc, &argv, 4, TU32, 0x0007ffff, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "payload_len") == 0) {
		NEXT_ARG();
		tkey->off = 4;
		res = parse_cmd(&argc, &argv, 2, TU32, RU16, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "nexthdr") == 0) {
		NEXT_ARG();
		tkey->off = 6;
		res = parse_cmd(&argc, &argv, 1, TU32, RU8, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "hoplimit") == 0) {
		NEXT_ARG();
		tkey->off = 7;
		res = parse_cmd(&argc, &argv, 1, TU32, RU8, sel, tkey);
		goto done;
	}
	if (strcmp(*argv, "traffic_class") == 0) {
		NEXT_ARG();
		tkey->off = 1;
		res = parse_cmd(&argc, &argv, 1, TU32, RU8, sel, tkey);

		/* Shift the field by 4 bits on success. */
		if (!res) {
			int nkeys = sel->sel.nkeys;
			struct tc_pedit_key *key = &sel->keys[nkeys - 1];

			key->mask = htonl(ntohl(key->mask) << 4 | 0xf);
			key->val = htonl(ntohl(key->val) << 4);
		}
		goto done;
	}

	return -1;

done:
	*argc_p = argc;
	*argv_p = argv;
	return res;
}

struct m_pedit_util p_pedit_ip6 = {
	.id = "ip6",
	.parse_peopt = parse_ip6,
};
