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

/*
 *  This is very much incomplete! You have been warned.
 */

#include "chilli.h"

static int callback(int a, struct iphash_t *h, struct iphashm_t *m) {
    return 0;
}

int nat_init(net_interface *iface) {
  int size = 1024;
  int i;
  natm_t ** m = (natm_t **)calloc(sizeof(natm_t *), size);

  for (i=0; i < size; i++)
    m[i] = (natm_t *)calloc(sizeof(natm_t), 1);

  iphash_new(&iface->nat, m, size, callback);
}

static int num_ports = 20000;
static int min_port = 10000;

int nat_do(struct tun_t *this, int idx, uint8_t *pack, size_t len) {
  net_interface *iface = &tun(this, idx);
  struct pkt_ethhdr_t *ethh = ethhdr(pack);
  struct pkt_iphdr_t  *iph  = iphdr(pack);
  struct pkt_udphdr_t *udph = udphdr(pack);
  struct in_addr addr;

  natm_t *p=0;

  if (!iface->nat)
    nat_init(iface);

  addr.s_addr = iph->daddr;
  iphash_get(iface->nat, &p, &addr, udph->dst);

  if (!p) {
    iphash_add(iface->nat, &p, &addr, udph->dst);
  }

  if (!p) {
    return -1;
  }

  p->dst_ip = iph->daddr;
  p->src_ip = iph->saddr;
  p->dst_port = udph->dst;
  p->src_port = udph->src;

  iph->saddr = _options.natip.s_addr ? _options.natip.s_addr : iface->address.s_addr;
  udph->src = htons((ntohs(udph->src) % num_ports) + min_port);
  
  chksum(iph);

  return 0;
}

int nat_undo(struct tun_t *this, int idx, uint8_t *pack, size_t len) {
  net_interface *iface = &tun(this, idx);
  struct pkt_ethhdr_t *ethh = ethhdr(pack);
  struct pkt_iphdr_t  *iph  = iphdr(pack);
  struct pkt_udphdr_t *udph = udphdr(pack);
  struct in_addr addr;

  natm_t *p=0;

  if (!iface->nat)
    nat_init(iface);

  addr.s_addr = iph->saddr;
  iphash_get(iface->nat, &p, &addr, udph->src);

  if (!p) {
    return -1;
  }

  iph->daddr = p->src_ip;
  udph->dst = p->src_port;

  chksum(iph);

  return 0;
}
