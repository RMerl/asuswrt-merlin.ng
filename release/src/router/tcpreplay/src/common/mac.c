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

#include "mac.h"
#include "config.h"
#include "common.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/**
 * converts a string representation of a MAC address, based on
 * non-portable ether_aton()
 */
void
mac2hex(const char *mac, u_char *dst, int len)
{
    int i;
    char *pp;

    if (len < 6)
        return;

    while (isspace(*mac))
        mac++;

    /* expect 6 hex octets separated by ':' or space/NUL if last octet */
    for (i = 0; i < 6; i++) {
        long l = strtol(mac, &pp, 16);
        if (pp == mac || l > 0xFF || l < 0)
            return;
        if (!(*pp == ':' || (i == 5 && (isspace(*pp) || *pp == '\0'))))
            return;
        dst[i] = (u_char)l;
        mac = pp + 1;
    }
}

/**
 * converts a string representation of TWO MAC addresses, which
 * are comma deliminated into two hex values.  Either *first or *second
 * can be NULL if there is nothing before or after the comma.
 * returns:
 * 1 = first mac
 * 2 = second mac
 * 3 = both mac's
 * 0 = none
 */
int
dualmac2hex(const char *dualmac, u_char *first, u_char *second, int len)
{
    char *tok = NULL, *temp, *string;
    int ret = 0;

    string = safe_strdup(dualmac);

    /* if we've only got a comma, then return NULL's */
    if (len <= 1)
        goto done;

    temp = strtok_r(string, ",", &tok);
    if (strlen(temp)) {
        mac2hex(temp, first, len);
        ret = 1;
    }

    temp = strtok_r(NULL, ",", &tok);
    /* temp is null if no comma */
    if (temp != NULL) {
        if (strlen(temp)) {
            mac2hex(temp, second, len);
            ret += 2;
        }
    }

done:
    safe_free(string);
    return ret;
}

/**
 * Figures out if a MAC is listed in a comma delimited
 * string of MAC addresses.
 * returns TCPR_DIR_C2S if listed
 * returns TCPR_DIR_S2C if not listed
 */
tcpr_dir_t
macinstring(const char *macstring, const u_char *mac)
{
    char *tok = NULL, *tempstr, *ourstring;
    u_char tempmac[6];
    int len = 6, ret = TCPR_DIR_S2C;

    ourstring = safe_strdup(macstring);
    memset(&tempmac[0], 0, sizeof(tempmac));

    tempstr = strtok_r(ourstring, ",", &tok);
    if (tempstr != NULL && strlen(tempstr)) {
        mac2hex(tempstr, tempmac, len);
        if (memcmp(mac, tempmac, len) == 0) {
            dbgx(3, "Packet matches: " MAC_FORMAT " sending out primary.\n", MAC_STR(tempmac));
            ret = TCPR_DIR_C2S;
            goto EXIT_MACINSTRING;
        }
    } else {
        goto EXIT_MACINSTRING;
    }

    while ((tempstr = strtok_r(NULL, ",", &tok)) != NULL) {
        mac2hex(tempstr, tempmac, len);
        if (memcmp(mac, tempmac, len) == 0) {
            ret = TCPR_DIR_C2S;
            dbgx(3, "Packet matches: " MAC_FORMAT " sending out primary.\n", MAC_STR(tempmac));
            goto EXIT_MACINSTRING;
        }
    }

EXIT_MACINSTRING:
    safe_free(ourstring);
#ifdef DEBUG
    if (ret == TCPR_DIR_S2C)
        dbg(3, "Packet doesn't match any MAC addresses sending out secondary.\n");
#endif
    return ret;
}
