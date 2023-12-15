/* $Id$ */

/*
 *   Copyright (c) 2001-2010 Aaron Turner <aturner at synfin dot net>
 *   Copyright (c) 2013-2022 Fred Klassen <tcpreplay at appneta dot com> - AppNeta
 *
 *   The Tcpreplay Suite of tools is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, either version 3 of the
 *   License, or with the authors permission any later version.
 *
 *   The Tcpreplay Suite is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with the Tcpreplay Suite.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ieee80211_hdr.h"
#include "dlt_utils.h"
#include "tcpedit_stub.h"
#include <stdlib.h>
#include <string.h>

/*
 * Does the given 802.11 header have data?
 * returns 1 for true & 0 for false
 */
int
ieee80211_is_data(tcpeditdlt_t *ctx, const void *packet, int pktlen)
{
    uint16_t *frame_control, fc;
    struct tcpr_802_2snap_hdr *snap;
    int hdrlen = 0;

    assert(ctx);
    assert(packet);

    /* Ack, Auth, NULL packets often are very small (10-30 bytes) */
    if (pktlen <= (int)sizeof(ieee80211_hdr_t)) {
        dbgx(1, "**** packet " COUNTER_SPEC " is too small (%d)", ctx->tcpedit->runtime.packetnum, pktlen);
        return 0;
    }

    /*
     * Fields: Version|Type|Subtype|Flags
     * Bytes: 2|2|4|8
     * Types: 00 = Management, 01 = Control, 10 = Data
     * Data Subtypes (in binary):
     * 0000 - Data
     * 0001 - Data + Ack
     * 0010 - Data + Poll
     * 0011 - Data + Ack + Poll
     * 01?? - Data + Null (no data)
     * 1000 - QoS (w/ data)
     * 1100 - QoS (no data)
     * 1??? - Reserved (beacon, etc)
     * FIXME:
     * So right now, we only look for pure data frames, since I'm not sure what to do with ACK/Poll
     */

    frame_control = (uint16_t *)packet;
    fc = ntohs(*frame_control);

    /* reserved == no data */
    if ((fc & ieee80211_FC_SUBTYPE_MASK) == ieee80211_FC_SUBTYPE_NULL) {
        dbg(2, "packet is NULL");
        return 1;
    }

    /* check for data */
    if ((fc & ieee80211_FC_TYPE_MASK) == ieee80211_FC_TYPE_DATA) {
        dbg(2, "packet has data bit set");
        return 1;
    }

    /* QoS is set by the high bit, all the lower bits are QoS sub-types
       QoS seems to add 2 bytes of data at the end of the 802.11 hdr */
    if ((fc & ieee80211_FC_SUBTYPE_MASK) >= ieee80211_FC_SUBTYPE_QOS) {
        hdrlen += 2;
    }

    /* frame must also have a 802.2 SNAP header */
    if (ieee80211_USE_4(fc)) {
        hdrlen += sizeof(ieee80211_addr4_hdr_t);
    } else {
        hdrlen += sizeof(ieee80211_hdr_t);
    }

    if (pktlen < hdrlen + (int)sizeof(struct tcpr_802_2snap_hdr)) {
        return 0; /* not long enough for SNAP */
    }

    snap = (struct tcpr_802_2snap_hdr *)&((u_char *)packet)[hdrlen];

    /* verify the header is 802.2SNAP (8 bytes) not 802.2 (3 bytes) */
    if (snap->snap_dsap == 0xAA && snap->snap_ssap == 0xAA) {
        dbg(2, "packet is 802.2SNAP which I think always has data");
        return 1;
    }

    warnx("Packet " COUNTER_SPEC " is unknown reason for non-data", ctx->tcpedit->runtime.packetnum);

    return 0;
}

/*
 * returns 1 if WEP is enabled, 0 if not
 */
int
ieee80211_is_encrypted(tcpeditdlt_t *ctx, const void *packet, int pktlen)
{
    uint16_t *frame_control, fc;

    assert(ctx);
    assert(packet);

    if (pktlen < (int)sizeof(ieee80211_hdr_t))
        return 0;

    frame_control = (uint16_t *)packet;
    fc = ntohs(*frame_control);

    if ((fc & ieee80211_FC_WEP_MASK) == ieee80211_FC_WEP_MASK) {
        return 1;
    }
    return 0;
}

/*
 * 802.11 headers are variable length and the clients (non-AP's) have their
 * src & dst MAC addresses in different places in the header based on the
 * flags set in the first two bytes of the header (frame control)
 */

u_char *
ieee80211_get_src(const void *header)
{
    uint16_t *frame_control, fc;

    assert(header);
    frame_control = (uint16_t *)header;
    fc = ntohs(*frame_control);

    if (ieee80211_USE_4(fc)) {
        ieee80211_addr4_hdr_t *addr4 = (ieee80211_addr4_hdr_t *)header;
        return addr4->addr4;
    } else {
        ieee80211_hdr_t *addr3 = (ieee80211_hdr_t *)header;
        switch (fc & (ieee80211_FC_TO_DS_MASK + ieee80211_FC_FROM_DS_MASK)) {
        case ieee80211_FC_TO_DS_MASK:
            return addr3->addr2;
        case ieee80211_FC_FROM_DS_MASK:
            return addr3->addr3;
        case 0:
            return addr3->addr2;
        default:
            err(-1, "Whoops... we shouldn't of gotten here.");
        }
    }
}

u_char *
ieee80211_get_dst(const void *header)
{
    uint16_t *frame_control, fc;

    assert(header);
    frame_control = (uint16_t *)header;
    fc = ntohs(*frame_control);

    if (ieee80211_USE_4(fc)) {
        ieee80211_addr4_hdr_t *addr4 = (ieee80211_addr4_hdr_t *)header;
        return addr4->addr3;
    } else {
        ieee80211_hdr_t *addr3 = (ieee80211_hdr_t *)header;

        switch (fc & (ieee80211_FC_TO_DS_MASK + ieee80211_FC_FROM_DS_MASK)) {
        case ieee80211_FC_TO_DS_MASK:
            return addr3->addr3;
        case ieee80211_FC_FROM_DS_MASK:
            return addr3->addr1;
        case 0:
            return addr3->addr3;
        default:
            err(-1, "Whoops... we shouldn't of gotten here.");
        }
    }
}
