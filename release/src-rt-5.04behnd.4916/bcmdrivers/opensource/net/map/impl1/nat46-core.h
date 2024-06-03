/*
 * NAT46 core definitions
 *
 * Copyright (c) 2013-2014 Andrew Yourtchenko <ayourtch@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __NAT46_CORE_H__
#define __NAT46_CORE_H__

#include "nat46-glue.h"

// #define nat46debug(level, format, ...) debug(DBG_V6, level, format, __VA_ARGS__)
// #define nat46debug(level, format, ...)
#define nat46debug(level, format, ...) do { if(nat46->debug >= level) { printk(format "\n", ##__VA_ARGS__); } } while (0)

#define IPV6HDRSIZE 40
#define IPV4HDRSIZE 20
#define IPV6V4HDRDELTA (IPV6HDRSIZE - IPV4HDRSIZE)

/* 
 * A generic v4<->v6 translation structure.
 * The currently supported translation styles:
 */

typedef enum {
  NAT46_XLATE_NONE = 0,
  NAT46_XLATE_MAP,
  NAT46_XLATE_MAP0,
  NAT46_XLATE_RFC6052
} nat46_xlate_style_t;

#define NAT46_SIGNATURE 0x544e3634
#define FREED_NAT46_SIGNATURE 0xdead544e

typedef struct {
  nat46_xlate_style_t style;
  struct in6_addr v6_pref;
  int 		  v6_pref_len;
  u32		  v4_pref;
  int             v4_pref_len;
  int		  ea_len;
  int             psid_offset;
  int             fmr_flag;
} nat46_xlate_rule_t;

typedef struct {
  nat46_xlate_rule_t local;
  nat46_xlate_rule_t remote;
} nat46_xlate_rulepair_t;

typedef struct {
  u32 sig; /* nat46 signature */
  int refcount;
  int debug;

  int npairs;
  nat46_xlate_rulepair_t pairs[0]; /* npairs */
} nat46_instance_t;

int nat46_ipv6_input(struct sk_buff *old_skb);
int nat46_ipv4_input(struct sk_buff *old_skb);

int nat46_set_ipair_config(nat46_instance_t *nat46, int ipair, char *buf, int count);
int nat46_set_config(nat46_instance_t *nat46, char *buf, int count);

int nat46_get_ipair_config(nat46_instance_t *nat46, int ipair, char *buf, int count);
int nat46_get_config(nat46_instance_t *nat46, char *buf, int count);

char *get_next_arg(char **ptail);
nat46_instance_t *get_nat46_instance(struct sk_buff *sk);

nat46_instance_t *alloc_nat46_instance(int npairs, nat46_instance_t *old, int from_ipair, int to_ipair, int remove_ipair);
void release_nat46_instance(nat46_instance_t *nat46);

#define NAT46_HTABLE_SIZE 32
#define NAT46_GOLDEN_RATIO_16 0x9e37

static inline int ipid_hashfn(__u16 ipid) {
  unsigned int m = ntohs(ipid) * NAT46_GOLDEN_RATIO_16;
  return ((m & 0xf800) >> 11);
}

typedef struct {
  struct hlist_node node;
  __u32 v4saddr;
  __u16 sport;
  __u32 v4daddr;
  __u16 dport;
  __u16 proto;
  __u16 ipid;
  __u32 map_id;
  struct timespec64 timer;
} frag_tuple_t;

/* List to map fragmented IPv4 to NATed source port */
typedef struct {
  spinlock_t lock;
  struct hlist_head chain[NAT46_HTABLE_SIZE];
  int size;
  time64_t timeout;
} frag_list_t;

void init_frag_list(time64_t timeout);
void free_frag_list(void);

int backup_frag_l4_ports(__u32 v4saddr, __u16 sport, __u32 v4daddr, __u16 dport, __u16 proto, __u16 ipid, __u32 map_id);
int restore_frag_l4_ports(__u32 v4saddr, __u32 v4daddr, __u16 ipid, __u16 proto, __u16 *sport, __u16 *dport, __u32 *map_id, int last_frag);

#endif
