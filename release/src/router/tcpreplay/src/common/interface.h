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

typedef struct interface_list_s {
    char name[255];
    char alias[255];
    char description[255];
    u_int32_t flags;
    struct interface_list_s *next;
} interface_list_t;

#define INTERFACE_LIST_SIZE (80 * 80) /* 80 cols * 80 rows */

char *get_interface(interface_list_t *, const char *);
interface_list_t *get_interface_list(void);
void list_interfaces(interface_list_t *);
