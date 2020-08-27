/*
 * Copyright (C) 2007-2016  B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#ifndef _BATCTL_TCPDUMP_H
#define _BATCTL_TCPDUMP_H

#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include "main.h"
#include "list-batman.h"

#ifndef ARPHRD_IEEE80211_PRISM
#define ARPHRD_IEEE80211_PRISM 802
#endif

#ifndef ARPHRD_IEEE80211_RADIOTAP
#define ARPHRD_IEEE80211_RADIOTAP 803
#endif

#define DUMP_TYPE_BATOGM 1
#define DUMP_TYPE_BATICMP 2
#define DUMP_TYPE_BATUCAST 4
#define DUMP_TYPE_BATBCAST 8
#define DUMP_TYPE_BATUTVLV 16
#define DUMP_TYPE_BATFRAG 32
#define DUMP_TYPE_NONBAT 128

#define IEEE80211_FCTL_FTYPE 0x0c00
#define IEEE80211_FCTL_TODS 0x0001
#define IEEE80211_FCTL_FROMDS 0x0002
#define IEEE80211_FCTL_PROTECTED 0x0040

#define IEEE80211_FTYPE_DATA 0x0800

#define IEEE80211_STYPE_QOS_DATA 0x8000

struct dump_if {
	struct list_head list;
	char *dev;
	int32_t raw_sock;
	struct sockaddr_ll addr;
	int32_t hw_type;
};

struct vlanhdr {
	unsigned short vid;
	u_int16_t ether_type;
} __attribute__ ((packed));

struct ieee80211_hdr {
	u_int16_t frame_control;
	u_int16_t duration_id;
	u_int8_t addr1[ETH_ALEN];
	u_int8_t addr2[ETH_ALEN];
	u_int8_t addr3[ETH_ALEN];
	u_int16_t seq_ctrl;
	u_int8_t addr4[ETH_ALEN];
} __attribute__ ((packed));

struct radiotap_header {
	u_int8_t it_version;
	u_int8_t it_pad;
	u_int16_t it_len;
	u_int32_t it_present;
} __attribute__((__packed__));

struct prism_item {
	u_int32_t did;
	u_int16_t status;
	u_int16_t len;
	u_int32_t data;
};

struct prism_header {
	u_int32_t msgcode;
	u_int32_t msglen;
	u_int8_t devname[16];
	struct prism_item hosttime;
	struct prism_item mactime;
	struct prism_item channel;
	struct prism_item rssi;
	struct prism_item sq;
	struct prism_item signal;
	struct prism_item noise;
	struct prism_item rate;
	struct prism_item istx;
	struct prism_item frmlen;
};

#define PRISM_HEADER_LEN sizeof(struct prism_header)
#define RADIOTAP_HEADER_LEN sizeof(struct radiotap_header)

int tcpdump(int argc, char **argv);

#endif
