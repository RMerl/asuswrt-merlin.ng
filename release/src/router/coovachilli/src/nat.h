/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#ifndef _NAT_H
#define _NAT_H

#include "system.h"
#include "iphash.h"

typedef struct iphash_t nat_t;

typedef struct {
  struct iphashm_t hdr;
  uint32_t dst_ip;
#ifdef NETNAT_PORTS
  uint16_t dst_port;
#endif
  uint32_t src_ip;
#ifdef NETNAT_PORTS
  uint16_t src_port;
#endif
} natm_t;

#endif
