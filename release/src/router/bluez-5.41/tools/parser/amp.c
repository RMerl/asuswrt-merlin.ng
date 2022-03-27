/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "parser.h"
#include "lib/amp.h"

static void amp_dump_chanlist(int level, struct amp_tlv *tlv, char *prefix)
{
	struct amp_chan_list *chan_list = (void *) tlv->val;
	struct amp_country_triplet *triplet;
	int i, num;

	num = (tlv->len - sizeof(*chan_list)) / sizeof(*triplet);

	printf("%s (number of triplets %d)\n", prefix, num);

	p_indent(level+2, 0);

	printf("Country code: %c%c%c\n", chan_list->country_code[0],
		chan_list->country_code[1], chan_list->country_code[2]);

	for (i = 0; i < num; i++) {
		triplet = &chan_list->triplets[i];

		p_indent(level+2, 0);

		if (triplet->chans.first_channel >= 201) {
			printf("Reg ext id %d reg class %d coverage class %d\n",
						triplet->ext.reg_extension_id,
						triplet->ext.reg_class,
						triplet->ext.coverage_class);
		} else {
			if (triplet->chans.num_channels == 1)
				printf("Channel %d max power %d\n",
						triplet->chans.first_channel,
						triplet->chans.max_power);
			else
				printf("Channels %d - %d max power %d\n",
						triplet->chans.first_channel,
						triplet->chans.first_channel +
						triplet->chans.num_channels,
						triplet->chans.max_power);
		}
	}
}

void amp_assoc_dump(int level, uint8_t *assoc, uint16_t len)
{
	struct amp_tlv *tlv = (void *) assoc;

	p_indent(level, 0);
	printf("Assoc data [len %d]:\n", len);

	while (len > sizeof(*tlv)) {
		uint16_t tlvlen = btohs(tlv->len);
		struct amp_pal_ver *ver;

		p_indent(level+1, 0);

		switch (tlv->type) {
		case A2MP_MAC_ADDR_TYPE:
			if (tlvlen != 6)
				break;
			printf("MAC: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
					tlv->val[0], tlv->val[1], tlv->val[2],
					tlv->val[3], tlv->val[4], tlv->val[5]);
			break;

		case A2MP_PREF_CHANLIST_TYPE:
			amp_dump_chanlist(level, tlv, "Preferred Chan List");
			break;

		case A2MP_CONNECTED_CHAN:
			amp_dump_chanlist(level, tlv, "Connected Chan List");
			break;

		case A2MP_PAL_CAP_TYPE:
			if (tlvlen != 4)
				break;
			printf("PAL CAP: %2.2x %2.2x %2.2x %2.2x\n",
					tlv->val[0], tlv->val[1], tlv->val[2],
					tlv->val[3]);
			break;

		case A2MP_PAL_VER_INFO:
			if (tlvlen != 5)
				break;
			ver = (struct amp_pal_ver *) tlv->val;
			printf("PAL VER: %2.2x Comp ID: %4.4x SubVer: %4.4x\n",
					ver->ver, btohs(ver->company_id),
					btohs(ver->sub_ver));
			break;

		default:
			printf("Unrecognized type %d\n", tlv->type);
			break;
		}

		len -= tlvlen + sizeof(*tlv);
		assoc += tlvlen + sizeof(*tlv);
		tlv = (struct amp_tlv *) assoc;
	}
}
