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

/*
 * This file implements missing libpcap functions which only exist in really
 * recent versions of libpcap.  We assume the user has at least 0.6, so anything
 * after that needs to be re-implimented here unless we want to start
 * requiring a newer version
 */

#include "defines.h"
#include "config.h"
#include "common.h"
#include <stdlib.h>

#ifndef HAVE_DLT_VAL_TO_DESC

/**
 * replacement for libpcap's pcap_datalink_val_to_description()
 * which doesn't exist in all versions
 */
const char *
pcap_datalink_val_to_description(int dlt)
{
    if (dlt > DLT2DESC_LEN)
        return "Unknown";

    return dlt2desc[dlt];
}

/**
 * replacement for libpcap's pcap_datalink_val_to_name()
 * which doesn't exist in all versions
 */
const char *
pcap_datalink_val_to_name(int dlt)
{
    if (dlt > DLT2NAME_LEN)
        return "Unknown";

    return dlt2name[dlt];
}

#endif
