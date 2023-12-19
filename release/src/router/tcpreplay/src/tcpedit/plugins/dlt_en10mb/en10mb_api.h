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

#include "en10mb_types.h"
#include "plugins_types.h"
#include "tcpedit_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief EN10MB (Ethernet) Plugin API functions for modifying the 802.3 Ethernet header
 *
 * setters always return TCPEDIT_OK on success or TCPEDIT_ERROR
 * if there is a problem.  You can use tcpedit_geterr() to get the reason
 * for the failure
 */
int tcpedit_en10mb_set_mac(tcpedit_t *tcpedit, char *mac, tcpedit_mac_mask mask);
int tcpedit_en10mb_set_vlan_mode(tcpedit_t *tcpedit, tcpedit_vlan vlan);
int tcpedit_en10mb_set_vlan_tag(tcpedit_t *tcpedit, u_int16_t tag);
int tcpedit_en10mb_set_vlan_priority(tcpedit_t *tcpedit, u_int8_t priority);
int tcpedit_en10mb_set_vlan_cfi(tcpedit_t *tcpedit, u_int8_t cfi);

#ifdef __cplusplus
}
#endif
