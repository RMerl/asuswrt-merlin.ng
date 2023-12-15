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

#pragma once

#include "config.h"

/*
 * libpcap <= 0.5 don't have some DLT types.  Add them here
 */
#ifndef HAVE_DLT_LINUX_SLL
#define DLT_LINUX_SLL 113
#endif

#ifndef HAVE_DLT_C_HDLC
#define DLT_C_HDLC 104
#endif

/*
 * libpcap < 0.8 don't have pcap_datalink_val_to_description() 
 * and pcap_datalink_val_to_name()
 */
#ifndef HAVE_DLT_VAL_TO_DESC

const char *pcap_datalink_val_to_description(int dlt);
const char *pcap_datalink_val_to_name(int dlt);

#endif
