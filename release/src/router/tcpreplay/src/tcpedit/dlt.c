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

#include "dlt.h"
#include "config.h"
#include "tcpedit.h"
#include <assert.h>

/**
 * takes in a libpcap DLT_ type and returns the length of the layer2 header
 * returns -1 for unsupported DLT
 */
int
dlt2layer2len(tcpedit_t *tcpedit, int dlt)
{
    assert(tcpedit);
    int len;
    switch (dlt) {
    /*
    case DLT_USER:
        len = tcpedit->l2.len;
        break;
        */
    case DLT_NULL:
        len = 2;
        break;

    case DLT_RAW:
        len = 0;
        break;

    case DLT_EN10MB:
        len = 12;
        break;
        /*
    case DLT_VLAN:
        len = 14;
        break;
        */
    case DLT_LINUX_SLL:
        len = 16;
        break;

    case DLT_PPP_SERIAL:
    case DLT_C_HDLC:
        len = 4;
        break;

    case DLT_JUNIPER_ETHER:
        len = 36;
        break;

    default:
        tcpedit_seterr(tcpedit, "Invalid DLT Type: %d", dlt);
        len = -1;
    }

    return len;
}

/**
 * each DLT type may require one or more user specified Layer 2 field
 * to be able to rewrite it as plain ethernet DLT_EN10MB
 * returns -1 on error or >= 0 on success
 */
int
dltrequires(tcpedit_t *tcpedit, int dlt)
{
    assert(tcpedit);
    int req = TCPEDIT_DLT_OK; // no change required by default

    switch (dlt) {
    case DLT_JUNIPER_ETHER:
    case DLT_EN10MB:
        /*        case DLT_USER:
                case DLT_VLAN: */
        /* we have everything we need in the original packet */
        break;

    case DLT_NULL:
    case DLT_RAW:
    case DLT_C_HDLC:
    case DLT_PPP_SERIAL:
        req = TCPEDIT_DLT_SRC + TCPEDIT_DLT_DST;
        /* we just have the proto */
        break;

    case DLT_LINUX_SLL:
        /* we have proto & SRC address */
        req = TCPEDIT_DLT_DST;
        break;

    default:
        tcpedit_seterr(tcpedit, "Invalid DLT Type: %d", dlt);
        req = -1;
    }

    return req;
}

/**
 * returns the default MTU size for the given DLT type.  Returns -1
 * for invalid DLT
 */
int
dlt2mtu(tcpedit_t *tcpedit, int dlt)
{
    int mtu;
    assert(tcpedit);
    switch (dlt) {
        /*        case DLT_VLAN:
                case DLT_USER: */
    case DLT_PPP_SERIAL:
    case DLT_EN10MB:
    case DLT_RAW:
    case DLT_C_HDLC:
    case DLT_JUNIPER_ETHER:
        mtu = 1500;
        break;

    case DLT_LINUX_SLL:
        mtu = 16436;
        break;

    case DLT_LOOP:
        mtu = 16384;
        break;

    default:
        tcpedit_seterr(tcpedit, "Invalid DLT Type: %d", dlt);
        mtu = -1;
        break;
    }

    return mtu;
}

/**
 * Returns the current layer 2 len based on the
 * DLT of the pcap or the --dlink value or -1 on error.
 * You need to call this function AFTER rewriting the layer 2 header
 * for it to be at all useful.
 */
int
layer2len(tcpedit_t *tcpedit, u_char *packet, uint32_t caplen)
{
    assert(tcpedit);
    assert(tcpedit->dlt_ctx);
    assert(tcpedit->dlt_ctx->encoder);

    return tcpedit->dlt_ctx->encoder->plugin_l2len(tcpedit->dlt_ctx, packet, caplen);
}
