/*
 * net/tipc/name_table.h: Include file for TIPC name table code
 *
 * Copyright (c) 2000-2006, 2014-2015, Ericsson AB
 * Copyright (c) 2004-2005, 2010-2011, Wind River Systems
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TIPC_NAME_TABLE_H
#define _TIPC_NAME_TABLE_H

struct tipc_subscription;
struct tipc_plist;

/*
 * TIPC name types reserved for internal TIPC use (both current and planned)
 */
#define TIPC_ZM_SRV		3	/* zone master service name type */
#define TIPC_PUBL_SCOPE_NUM	(TIPC_NODE_SCOPE + 1)
#define TIPC_NAMETBL_SIZE	1024	/* must be a power of 2 */

/**
 * struct publication - info about a published (name or) name sequence
 * @type: name sequence type
 * @lower: name sequence lower bound
 * @upper: name sequence upper bound
 * @scope: scope of publication
 * @node: network address of publishing port's node
 * @ref: publishing port
 * @key: publication key
 * @nodesub_list: subscription to "node down" event (off-node publication only)
 * @local_list: adjacent entries in list of publications made by this node
 * @pport_list: adjacent entries in list of publications made by this port
 * @node_list: adjacent matching name seq publications with >= node scope
 * @cluster_list: adjacent matching name seq publications with >= cluster scope
 * @zone_list: adjacent matching name seq publications with >= zone scope
 * @rcu: RCU callback head used for deferred freeing
 *
 * Note that the node list, cluster list, and zone list are circular lists.
 */
struct publication {
	u32 type;
	u32 lower;
	u32 upper;
	u32 scope;
	u32 node;
	u32 ref;
	u32 key;
	struct list_head nodesub_list;
	struct list_head local_list;
	struct list_head pport_list;
	struct list_head node_list;
	struct list_head cluster_list;
	struct list_head zone_list;
	struct rcu_head rcu;
};

/**
 * struct name_table - table containing all existing port name publications
 * @seq_hlist: name sequence hash lists
 * @publ_list: pulication lists
 * @local_publ_count: number of publications issued by this node
 */
struct name_table {
	struct hlist_head seq_hlist[TIPC_NAMETBL_SIZE];
	struct list_head publ_list[TIPC_PUBL_SCOPE_NUM];
	u32 local_publ_count;
};

int tipc_nl_name_table_dump(struct sk_buff *skb, struct netlink_callback *cb);

u32 tipc_nametbl_translate(struct net *net, u32 type, u32 instance, u32 *node);
int tipc_nametbl_mc_translate(struct net *net, u32 type, u32 lower, u32 upper,
			      u32 limit, struct tipc_plist *dports);
struct publication *tipc_nametbl_publish(struct net *net, u32 type, u32 lower,
					 u32 upper, u32 scope, u32 port_ref,
					 u32 key);
int tipc_nametbl_withdraw(struct net *net, u32 type, u32 lower, u32 ref,
			  u32 key);
struct publication *tipc_nametbl_insert_publ(struct net *net, u32 type,
					     u32 lower, u32 upper, u32 scope,
					     u32 node, u32 ref, u32 key);
struct publication *tipc_nametbl_remove_publ(struct net *net, u32 type,
					     u32 lower, u32 node, u32 ref,
					     u32 key);
void tipc_nametbl_subscribe(struct tipc_subscription *s);
void tipc_nametbl_unsubscribe(struct tipc_subscription *s);
int tipc_nametbl_init(struct net *net);
void tipc_nametbl_stop(struct net *net);

struct tipc_plist {
	struct list_head list;
	u32 port;
};

static inline void tipc_plist_init(struct tipc_plist *pl)
{
	INIT_LIST_HEAD(&pl->list);
	pl->port = 0;
}

void tipc_plist_push(struct tipc_plist *pl, u32 port);
u32 tipc_plist_pop(struct tipc_plist *pl);

#endif
