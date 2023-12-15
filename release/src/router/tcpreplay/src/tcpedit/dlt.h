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

#include "defines.h"
#include "tcpedit.h"

#define TCPEDIT_DLT_OK 0
#define TCPEDIT_DLT_SRC 1
#define TCPEDIT_DLT_DST 2
#define TCPEDIT_DLT_PROTO 4

int dlt2layer2len(tcpedit_t *tcpedit, int dlt);
int dltrequires(tcpedit_t *tcpedit, int dlt);
int dlt2mtu(tcpedit_t *tcpedit, int dlt);
int layer2len(tcpedit_t *tcpedit, u_char *packet, uint32_t caplen);
