/*
 * m_pedit.h		generic packet editor actions module
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:  J Hadi Salim (hadi@cyberus.ca)
 *
 */

#ifndef _ACT_PEDIT_H_
#define _ACT_PEDIT_H_ 1

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
#include <linux/tc_act/tc_pedit.h>

#define MAX_OFFS 128

#define TIPV4 1
#define TIPV6 2
#define TINT 3
#define TU32 4
#define TMAC 5

#define RU32 0xFFFFFFFF
#define RU16 0xFFFF
#define RU8 0xFF

#define PEDITKINDSIZ 16

struct m_pedit_key {
	__u32           mask;  /* AND */
	__u32           val;   /*XOR */
	__u32           off;  /*offset */
	__u32           at;
	__u32           offmask;
	__u32           shift;

	enum pedit_header_type htype;
	enum pedit_cmd cmd;
};

struct m_pedit_key_ex {
	enum pedit_header_type htype;
	enum pedit_cmd cmd;
};

struct m_pedit_sel {
	struct tc_pedit_sel sel;
	struct tc_pedit_key keys[MAX_OFFS];
	struct m_pedit_key_ex keys_ex[MAX_OFFS];
	bool extended;
};

struct m_pedit_util {
	struct m_pedit_util *next;
	char    id[PEDITKINDSIZ];
	int     (*parse_peopt)(int *argc_p, char ***argv_p,
			       struct m_pedit_sel *sel,
			       struct m_pedit_key *tkey);
};

int parse_cmd(int *argc_p, char ***argv_p, __u32 len, int type,
	      __u32 retain,
	      struct m_pedit_sel *sel, struct m_pedit_key *tkey);
#endif
