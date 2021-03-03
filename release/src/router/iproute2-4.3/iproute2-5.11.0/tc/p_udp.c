/*
 * m_pedit_udp.c	packet editor: UDP header
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (hadi@cyberus.ca)
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
parse_udp(int *argc_p, char ***argv_p,
	  struct m_pedit_sel *sel, struct m_pedit_key *tkey)
{
	int res = -1;
	int argc = *argc_p;
	char **argv = *argv_p;

	if (argc < 2)
		return -1;

	if (!sel->extended)
		return -1;

	tkey->htype = TCA_PEDIT_KEY_EX_HDR_TYPE_UDP;

	if (strcmp(*argv, "sport") == 0) {
		NEXT_ARG();
		tkey->off = 0;
		res = parse_cmd(&argc, &argv, 2, TU32, RU16, sel, tkey);
		goto done;
	}

	if (strcmp(*argv, "dport") == 0) {
		NEXT_ARG();
		tkey->off = 2;
		res = parse_cmd(&argc, &argv, 2, TU32, RU16, sel, tkey);
		goto done;
	}

	return -1;

done:
	*argc_p = argc;
	*argv_p = argv;
	return res;
}

struct m_pedit_util p_pedit_udp = {
	.id = "udp",
	.parse_peopt = parse_udp,
};
