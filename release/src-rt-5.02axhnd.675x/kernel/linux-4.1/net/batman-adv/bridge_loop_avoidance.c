/* Copyright (C) 2011-2014 B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "hash.h"
#include "hard-interface.h"
#include "originator.h"
#include "bridge_loop_avoidance.h"
#include "translation-table.h"
#include "send.h"

#include <linux/etherdevice.h>
#include <linux/crc16.h>
#include <linux/if_arp.h>
#include <net/arp.h>
#include <linux/if_vlan.h>

static const uint8_t batadv_announce_mac[4] = {0x43, 0x05, 0x43, 0x05};

static void batadv_bla_periodic_work(struct work_struct *work);
static void
batadv_bla_send_announce(struct batadv_priv *bat_priv,
			 struct batadv_bla_backbone_gw *backbone_gw);

/* return the index of the claim */
static inline uint32_t batadv_choose_claim(const void *data, uint32_t size)
{
	struct batadv_bla_claim *claim = (struct batadv_bla_claim *)data;
	uint32_t hash = 0;

	hash = batadv_hash_bytes(hash, &claim->addr, sizeof(claim->addr));
	hash = batadv_hash_bytes(hash, &claim->vid, sizeof(claim->vid));

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % size;
}

/* return the index of the backbone gateway */
static inline uint32_t batadv_choose_backbone_gw(const void *data,
						 uint32_t size)
{
	const struct batadv_bla_claim *claim = (struct batadv_bla_claim *)data;
	uint32_t hash = 0;

	hash = batadv_hash_bytes(hash, &claim->addr, sizeof(claim->addr));
	hash = batadv_hash_bytes(hash, &claim->vid, sizeof(claim->vid));

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % size;
}

/* compares address and vid of two backbone gws */
static int batadv_compare_backbone_gw(const struct hlist_node *node,
				      const void *data2)
{
	const void *data1 = container_of(node, struct batadv_bla_backbone_gw,
					 hash_entry);
	const struct batadv_bla_backbone_gw *gw1 = data1, *gw2 = data2;

	if (!batadv_compare_eth(gw1->orig, gw2->orig))
		return 0;

	if (gw1->vid != gw2->vid)
		return 0;

	return 1;
}

/* compares address and vid of two claims */
static int batadv_compare_claim(const struct hlist_node *node,
				const void *data2)
{
	const void *data1 = container_of(node, struct batadv_bla_claim,
					 hash_entry);
	const struct batadv_bla_claim *cl1 = data1, *cl2 = data2;

	if (!batadv_compare_eth(cl1->addr, cl2->addr))
		return 0;

	if (cl1->vid != cl2->vid)
		return 0;

	return 1;
}

/* free a backbone gw */
static void
batadv_backbone_gw_free_ref(struct batadv_bla_backbone_gw *backbone_gw)
{
	if (atomic_dec_and_test(&backbone_gw->refcount))
		kfree_rcu(backbone_gw, rcu);
}

/* finally deinitialize the claim */
static void batadv_claim_release(struct batadv_bla_claim *claim)
{
	batadv_backbone_gw_free_ref(claim->backbone_gw);
	kfree_rcu(claim, rcu);
}

/* free a claim, call claim_free_rcu if its the last reference */
static void batadv_claim_free_ref(struct batadv_bla_claim *claim)
{
	if (atomic_dec_and_test(&claim->refcount))
		batadv_claim_release(claim);
}

/**
 * batadv_claim_hash_find
 * @bat_priv: the bat priv with all the soft interface information
 * @data: search data (may be local/static data)
 *
 * looks for a claim in the hash, and returns it if found
 * or NULL otherwise.
 */
static struct batadv_bla_claim
*batadv_claim_hash_find(struct batadv_priv *bat_priv,
			struct batadv_bla_claim *data)
{
	struct batadv_hashtable *hash = bat_priv->bla.claim_hash;
	struct hlist_head *head;
	struct batadv_bla_claim *claim;
	struct batadv_bla_claim *claim_tmp = NULL;
	int index;

	if (!hash)
		return NULL;

	index = batadv_choose_claim(data, hash->size);
	head = &hash->table[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(claim, head, hash_entry) {
		if (!batadv_compare_claim(&claim->hash_entry, data))
			continue;

		if (!atomic_inc_not_zero(&claim->refcount))
			continue;

		claim_tmp = claim;
		break;
	}
	rcu_read_unlock();

	return claim_tmp;
}

/**
 * batadv_backbone_hash_find - looks for a claim in the hash
 * @bat_priv: the bat priv with all the soft interface information
 * @addr: the address of the originator
 * @vid: the VLAN ID
 *
 * Returns claim if found or NULL otherwise.
 */
static struct batadv_bla_backbone_gw *
batadv_backbone_hash_find(struct batadv_priv *bat_priv,
			  uint8_t *addr, unsigned short vid)
{
	struct batadv_hashtable *hash = bat_priv->bla.backbone_hash;
	struct hlist_head *head;
	struct batadv_bla_backbone_gw search_entry, *backbone_gw;
	struct batadv_bla_backbone_gw *backbone_gw_tmp = NULL;
	int index;

	if (!hash)
		return NULL;

	ether_addr_copy(search_entry.orig, addr);
	search_entry.vid = vid;

	index = batadv_choose_backbone_gw(&search_entry, hash->size);
	head = &hash->table[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(backbone_gw, head, hash_entry) {
		if (!batadv_compare_backbone_gw(&backbone_gw->hash_entry,
						&search_entry))
			continue;

		if (!atomic_inc_not_zero(&backbone_gw->refcount))
			continue;

		backbone_gw_tmp = backbone_gw;
		break;
	}
	rcu_read_unlock();

	return backbone_gw_tmp;
}

/* delete all claims for a backbone */
static void
batadv_bla_del_backbone_claims(struct batadv_bla_backbone_gw *backbone_gw)
{
	struct batadv_hashtable *hash;
	struct hlist_node *node_tmp;
	struct hlist_head *head;
	struct batadv_bla_claim *claim;
	int i;
	spinlock_t *list_lock;	/* protects write access to the hash lists */

	hash = backbone_gw->bat_priv->bla.claim_hash;
	if (!hash)
		return;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];
		list_lock = &hash->list_locks[i];

		spin_lock_bh(list_lock);
		hlist_for_each_entry_safe(claim, node_tmp,
					  head, hash_entry) {
			if (claim->backbone_gw != backbone_gw)
				continue;

			batadv_claim_free_ref(claim);
			hlist_del_rcu(&claim->hash_entry);
		}
		spin_unlock_bh(list_lock);
	}

	/* all claims gone, initialize CRC */
	backbone_gw->crc = BATADV_BLA_CRC_INIT;
}

/**
 * batadv_bla_send_claim - sends a claim frame according to the provided info
 * @bat_priv: the bat priv with all the soft interface information
 * @mac: the mac address to be announced within the claim
 * @vid: the VLAN ID
 * @claimtype: the type of the claim (CLAIM, UNCLAIM, ANNOUNCE, ...)
 */
static void batadv_bla_send_claim(struct batadv_priv *bat_priv, uint8_t *mac,
				  unsigned short vid, int claimtype)
{
	struct sk_buff *skb;
	struct ethhdr *ethhdr;
	struct batadv_hard_iface *primary_if;
	struct net_device *soft_iface;
	uint8_t *hw_src;
	struct batadv_bla_claim_dst local_claim_dest;
	__be32 zeroip = 0;

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		return;

	memcpy(&local_claim_dest, &bat_priv->bla.claim_dest,
	       sizeof(local_claim_dest));
	local_claim_dest.type = claimtype;

	soft_iface = primary_if->soft_iface;

	skb = arp_create(ARPOP_REPLY, ETH_P_ARP,
			 /* IP DST: 0.0.0.0 */
			 zeroip,
			 primary_if->soft_iface,
			 /* IP SRC: 0.0.0.0 */
			 zeroip,
			 /* Ethernet DST: Broadcast */
			 NULL,
			 /* Ethernet SRC/HW SRC:  originator mac */
			 primary_if->net_dev->dev_addr,
			 /* HW DST: FF:43:05:XX:YY:YY
			  * with XX   = claim type
			  * and YY:YY = group id
			  */
			 (uint8_t *)&local_claim_dest);

	if (!skb)
		goto out;

	ethhdr = (struct ethhdr *)skb->data;
	hw_src = (uint8_t *)ethhdr + ETH_HLEN + sizeof(struct arphdr);

	/* now we pretend that the client would have sent this ... */
	switch (claimtype) {
	case BATADV_CLAIM_TYPE_CLAIM:
		/* normal claim frame
		 * set Ethernet SRC to the clients mac
		 */
		ether_addr_copy(ethhdr->h_source, mac);
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_send_claim(): CLAIM %pM on vid %d\n", mac,
			   BATADV_PRINT_VID(vid));
		break;
	case BATADV_CLAIM_TYPE_UNCLAIM:
		/* unclaim frame
		 * set HW SRC to the clients mac
		 */
		ether_addr_copy(hw_src, mac);
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_send_claim(): UNCLAIM %pM on vid %d\n", mac,
			   BATADV_PRINT_VID(vid));
		break;
	case BATADV_CLAIM_TYPE_ANNOUNCE:
		/* announcement frame
		 * set HW SRC to the special mac containg the crc
		 */
		ether_addr_copy(hw_src, mac);
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_send_claim(): ANNOUNCE of %pM on vid %d\n",
			   ethhdr->h_source, BATADV_PRINT_VID(vid));
		break;
	case BATADV_CLAIM_TYPE_REQUEST:
		/* request frame
		 * set HW SRC and header destination to the receiving backbone
		 * gws mac
		 */
		ether_addr_copy(hw_src, mac);
		ether_addr_copy(ethhdr->h_dest, mac);
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_send_claim(): REQUEST of %pM to %pM on vid %d\n",
			   ethhdr->h_source, ethhdr->h_dest,
			   BATADV_PRINT_VID(vid));
		break;
	}

	if (vid & BATADV_VLAN_HAS_TAG)
		skb = vlan_insert_tag(skb, htons(ETH_P_8021Q),
				      vid & VLAN_VID_MASK);

	skb_reset_mac_header(skb);
	skb->protocol = eth_type_trans(skb, soft_iface);
	batadv_inc_counter(bat_priv, BATADV_CNT_RX);
	batadv_add_counter(bat_priv, BATADV_CNT_RX_BYTES,
			   skb->len + ETH_HLEN);
	soft_iface->last_rx = jiffies;

	netif_rx(skb);
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
}

/**
 * batadv_bla_get_backbone_gw
 * @bat_priv: the bat priv with all the soft interface information
 * @orig: the mac address of the originator
 * @vid: the VLAN ID
 * @own_backbone: set if the requested backbone is local
 *
 * searches for the backbone gw or creates a new one if it could not
 * be found.
 */
static struct batadv_bla_backbone_gw *
batadv_bla_get_backbone_gw(struct batadv_priv *bat_priv, uint8_t *orig,
			   unsigned short vid, bool own_backbone)
{
	struct batadv_bla_backbone_gw *entry;
	struct batadv_orig_node *orig_node;
	int hash_added;

	entry = batadv_backbone_hash_find(bat_priv, orig, vid);

	if (entry)
		return entry;

	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "bla_get_backbone_gw(): not found (%pM, %d), creating new entry\n",
		   orig, BATADV_PRINT_VID(vid));

	entry = kzalloc(sizeof(*entry), GFP_ATOMIC);
	if (!entry)
		return NULL;

	entry->vid = vid;
	entry->lasttime = jiffies;
	entry->crc = BATADV_BLA_CRC_INIT;
	entry->bat_priv = bat_priv;
	atomic_set(&entry->request_sent, 0);
	atomic_set(&entry->wait_periods, 0);
	ether_addr_copy(entry->orig, orig);

	/* one for the hash, one for returning */
	atomic_set(&entry->refcount, 2);

	hash_added = batadv_hash_add(bat_priv->bla.backbone_hash,
				     batadv_compare_backbone_gw,
				     batadv_choose_backbone_gw, entry,
				     &entry->hash_entry);

	if (unlikely(hash_added != 0)) {
		/* hash failed, free the structure */
		kfree(entry);
		return NULL;
	}

	/* this is a gateway now, remove any TT entry on this VLAN */
	orig_node = batadv_orig_hash_find(bat_priv, orig);
	if (orig_node) {
		batadv_tt_global_del_orig(bat_priv, orig_node, vid,
					  "became a backbone gateway");
		batadv_orig_node_free_ref(orig_node);
	}

	if (own_backbone) {
		batadv_bla_send_announce(bat_priv, entry);

		/* this will be decreased in the worker thread */
		atomic_inc(&entry->request_sent);
		atomic_set(&entry->wait_periods, BATADV_BLA_WAIT_PERIODS);
		atomic_inc(&bat_priv->bla.num_requests);
	}

	return entry;
}

/* update or add the own backbone gw to make sure we announce
 * where we receive other backbone gws
 */
static void
batadv_bla_update_own_backbone_gw(struct batadv_priv *bat_priv,
				  struct batadv_hard_iface *primary_if,
				  unsigned short vid)
{
	struct batadv_bla_backbone_gw *backbone_gw;

	backbone_gw = batadv_bla_get_backbone_gw(bat_priv,
						 primary_if->net_dev->dev_addr,
						 vid, true);
	if (unlikely(!backbone_gw))
		return;

	backbone_gw->lasttime = jiffies;
	batadv_backbone_gw_free_ref(backbone_gw);
}

/**
 * batadv_bla_answer_request - answer a bla request by sending own claims
 * @bat_priv: the bat priv with all the soft interface information
 * @primary_if: interface where the request came on
 * @vid: the vid where the request came on
 *
 * Repeat all of our own claims, and finally send an ANNOUNCE frame
 * to allow the requester another check if the CRC is correct now.
 */
static void batadv_bla_answer_request(struct batadv_priv *bat_priv,
				      struct batadv_hard_iface *primary_if,
				      unsigned short vid)
{
	struct hlist_head *head;
	struct batadv_hashtable *hash;
	struct batadv_bla_claim *claim;
	struct batadv_bla_backbone_gw *backbone_gw;
	int i;

	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "bla_answer_request(): received a claim request, send all of our own claims again\n");

	backbone_gw = batadv_backbone_hash_find(bat_priv,
						primary_if->net_dev->dev_addr,
						vid);
	if (!backbone_gw)
		return;

	hash = bat_priv->bla.claim_hash;
	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(claim, head, hash_entry) {
			/* only own claims are interesting */
			if (claim->backbone_gw != backbone_gw)
				continue;

			batadv_bla_send_claim(bat_priv, claim->addr, claim->vid,
					      BATADV_CLAIM_TYPE_CLAIM);
		}
		rcu_read_unlock();
	}

	/* finally, send an announcement frame */
	batadv_bla_send_announce(bat_priv, backbone_gw);
	batadv_backbone_gw_free_ref(backbone_gw);
}

/**
 * batadv_bla_send_request - send a request to repeat claims
 * @backbone_gw: the backbone gateway from whom we are out of sync
 *
 * When the crc is wrong, ask the backbone gateway for a full table update.
 * After the request, it will repeat all of his own claims and finally
 * send an announcement claim with which we can check again.
 */
static void batadv_bla_send_request(struct batadv_bla_backbone_gw *backbone_gw)
{
	/* first, remove all old entries */
	batadv_bla_del_backbone_claims(backbone_gw);

	batadv_dbg(BATADV_DBG_BLA, backbone_gw->bat_priv,
		   "Sending REQUEST to %pM\n", backbone_gw->orig);

	/* send request */
	batadv_bla_send_claim(backbone_gw->bat_priv, backbone_gw->orig,
			      backbone_gw->vid, BATADV_CLAIM_TYPE_REQUEST);

	/* no local broadcasts should be sent or received, for now. */
	if (!atomic_read(&backbone_gw->request_sent)) {
		atomic_inc(&backbone_gw->bat_priv->bla.num_requests);
		atomic_set(&backbone_gw->request_sent, 1);
	}
}

/**
 * batadv_bla_send_announce
 * @bat_priv: the bat priv with all the soft interface information
 * @backbone_gw: our backbone gateway which should be announced
 *
 * This function sends an announcement. It is called from multiple
 * places.
 */
static void batadv_bla_send_announce(struct batadv_priv *bat_priv,
				     struct batadv_bla_backbone_gw *backbone_gw)
{
	uint8_t mac[ETH_ALEN];
	__be16 crc;

	memcpy(mac, batadv_announce_mac, 4);
	crc = htons(backbone_gw->crc);
	memcpy(&mac[4], &crc, 2);

	batadv_bla_send_claim(bat_priv, mac, backbone_gw->vid,
			      BATADV_CLAIM_TYPE_ANNOUNCE);
}

/**
 * batadv_bla_add_claim - Adds a claim in the claim hash
 * @bat_priv: the bat priv with all the soft interface information
 * @mac: the mac address of the claim
 * @vid: the VLAN ID of the frame
 * @backbone_gw: the backbone gateway which claims it
 */
static void batadv_bla_add_claim(struct batadv_priv *bat_priv,
				 const uint8_t *mac, const unsigned short vid,
				 struct batadv_bla_backbone_gw *backbone_gw)
{
	struct batadv_bla_claim *claim;
	struct batadv_bla_claim search_claim;
	int hash_added;

	ether_addr_copy(search_claim.addr, mac);
	search_claim.vid = vid;
	claim = batadv_claim_hash_find(bat_priv, &search_claim);

	/* create a new claim entry if it does not exist yet. */
	if (!claim) {
		claim = kzalloc(sizeof(*claim), GFP_ATOMIC);
		if (!claim)
			return;

		ether_addr_copy(claim->addr, mac);
		claim->vid = vid;
		claim->lasttime = jiffies;
		claim->backbone_gw = backbone_gw;

		atomic_set(&claim->refcount, 2);
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_add_claim(): adding new entry %pM, vid %d to hash ...\n",
			   mac, BATADV_PRINT_VID(vid));
		hash_added = batadv_hash_add(bat_priv->bla.claim_hash,
					     batadv_compare_claim,
					     batadv_choose_claim, claim,
					     &claim->hash_entry);

		if (unlikely(hash_added != 0)) {
			/* only local changes happened. */
			kfree(claim);
			return;
		}
	} else {
		claim->lasttime = jiffies;
		if (claim->backbone_gw == backbone_gw)
			/* no need to register a new backbone */
			goto claim_free_ref;

		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_add_claim(): changing ownership for %pM, vid %d\n",
			   mac, BATADV_PRINT_VID(vid));

		claim->backbone_gw->crc ^= crc16(0, claim->addr, ETH_ALEN);
		batadv_backbone_gw_free_ref(claim->backbone_gw);
	}
	/* set (new) backbone gw */
	atomic_inc(&backbone_gw->refcount);
	claim->backbone_gw = backbone_gw;

	backbone_gw->crc ^= crc16(0, claim->addr, ETH_ALEN);
	backbone_gw->lasttime = jiffies;

claim_free_ref:
	batadv_claim_free_ref(claim);
}

/* Delete a claim from the claim hash which has the
 * given mac address and vid.
 */
static void batadv_bla_del_claim(struct batadv_priv *bat_priv,
				 const uint8_t *mac, const unsigned short vid)
{
	struct batadv_bla_claim search_claim, *claim;

	ether_addr_copy(search_claim.addr, mac);
	search_claim.vid = vid;
	claim = batadv_claim_hash_find(bat_priv, &search_claim);
	if (!claim)
		return;

	batadv_dbg(BATADV_DBG_BLA, bat_priv, "bla_del_claim(): %pM, vid %d\n",
		   mac, BATADV_PRINT_VID(vid));

	batadv_hash_remove(bat_priv->bla.claim_hash, batadv_compare_claim,
			   batadv_choose_claim, claim);
	batadv_claim_free_ref(claim); /* reference from the hash is gone */

	claim->backbone_gw->crc ^= crc16(0, claim->addr, ETH_ALEN);

	/* don't need the reference from hash_find() anymore */
	batadv_claim_free_ref(claim);
}

/* check for ANNOUNCE frame, return 1 if handled */
static int batadv_handle_announce(struct batadv_priv *bat_priv,
				  uint8_t *an_addr, uint8_t *backbone_addr,
				  unsigned short vid)
{
	struct batadv_bla_backbone_gw *backbone_gw;
	uint16_t crc;

	if (memcmp(an_addr, batadv_announce_mac, 4) != 0)
		return 0;

	backbone_gw = batadv_bla_get_backbone_gw(bat_priv, backbone_addr, vid,
						 false);

	if (unlikely(!backbone_gw))
		return 1;

	/* handle as ANNOUNCE frame */
	backbone_gw->lasttime = jiffies;
	crc = ntohs(*((__be16 *)(&an_addr[4])));

	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "handle_announce(): ANNOUNCE vid %d (sent by %pM)... CRC = %#.4x\n",
		   BATADV_PRINT_VID(vid), backbone_gw->orig, crc);

	if (backbone_gw->crc != crc) {
		batadv_dbg(BATADV_DBG_BLA, backbone_gw->bat_priv,
			   "handle_announce(): CRC FAILED for %pM/%d (my = %#.4x, sent = %#.4x)\n",
			   backbone_gw->orig,
			   BATADV_PRINT_VID(backbone_gw->vid),
			   backbone_gw->crc, crc);

		batadv_bla_send_request(backbone_gw);
	} else {
		/* if we have sent a request and the crc was OK,
		 * we can allow traffic again.
		 */
		if (atomic_read(&backbone_gw->request_sent)) {
			atomic_dec(&backbone_gw->bat_priv->bla.num_requests);
			atomic_set(&backbone_gw->request_sent, 0);
		}
	}

	batadv_backbone_gw_free_ref(backbone_gw);
	return 1;
}

/* check for REQUEST frame, return 1 if handled */
static int batadv_handle_request(struct batadv_priv *bat_priv,
				 struct batadv_hard_iface *primary_if,
				 uint8_t *backbone_addr,
				 struct ethhdr *ethhdr, unsigned short vid)
{
	/* check for REQUEST frame */
	if (!batadv_compare_eth(backbone_addr, ethhdr->h_dest))
		return 0;

	/* sanity check, this should not happen on a normal switch,
	 * we ignore it in this case.
	 */
	if (!batadv_compare_eth(ethhdr->h_dest, primary_if->net_dev->dev_addr))
		return 1;

	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "handle_request(): REQUEST vid %d (sent by %pM)...\n",
		   BATADV_PRINT_VID(vid), ethhdr->h_source);

	batadv_bla_answer_request(bat_priv, primary_if, vid);
	return 1;
}

/* check for UNCLAIM frame, return 1 if handled */
static int batadv_handle_unclaim(struct batadv_priv *bat_priv,
				 struct batadv_hard_iface *primary_if,
				 uint8_t *backbone_addr,
				 uint8_t *claim_addr, unsigned short vid)
{
	struct batadv_bla_backbone_gw *backbone_gw;

	/* unclaim in any case if it is our own */
	if (primary_if && batadv_compare_eth(backbone_addr,
					     primary_if->net_dev->dev_addr))
		batadv_bla_send_claim(bat_priv, claim_addr, vid,
				      BATADV_CLAIM_TYPE_UNCLAIM);

	backbone_gw = batadv_backbone_hash_find(bat_priv, backbone_addr, vid);

	if (!backbone_gw)
		return 1;

	/* this must be an UNCLAIM frame */
	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "handle_unclaim(): UNCLAIM %pM on vid %d (sent by %pM)...\n",
		   claim_addr, BATADV_PRINT_VID(vid), backbone_gw->orig);

	batadv_bla_del_claim(bat_priv, claim_addr, vid);
	batadv_backbone_gw_free_ref(backbone_gw);
	return 1;
}

/* check for CLAIM frame, return 1 if handled */
static int batadv_handle_claim(struct batadv_priv *bat_priv,
			       struct batadv_hard_iface *primary_if,
			       uint8_t *backbone_addr, uint8_t *claim_addr,
			       unsigned short vid)
{
	struct batadv_bla_backbone_gw *backbone_gw;

	/* register the gateway if not yet available, and add the claim. */

	backbone_gw = batadv_bla_get_backbone_gw(bat_priv, backbone_addr, vid,
						 false);

	if (unlikely(!backbone_gw))
		return 1;

	/* this must be a CLAIM frame */
	batadv_bla_add_claim(bat_priv, claim_addr, vid, backbone_gw);
	if (batadv_compare_eth(backbone_addr, primary_if->net_dev->dev_addr))
		batadv_bla_send_claim(bat_priv, claim_addr, vid,
				      BATADV_CLAIM_TYPE_CLAIM);

	/* TODO: we could call something like tt_local_del() here. */

	batadv_backbone_gw_free_ref(backbone_gw);
	return 1;
}

/**
 * batadv_check_claim_group
 * @bat_priv: the bat priv with all the soft interface information
 * @primary_if: the primary interface of this batman interface
 * @hw_src: the Hardware source in the ARP Header
 * @hw_dst: the Hardware destination in the ARP Header
 * @ethhdr: pointer to the Ethernet header of the claim frame
 *
 * checks if it is a claim packet and if its on the same group.
 * This function also applies the group ID of the sender
 * if it is in the same mesh.
 *
 * returns:
 *	2  - if it is a claim packet and on the same group
 *	1  - if is a claim packet from another group
 *	0  - if it is not a claim packet
 */
static int batadv_check_claim_group(struct batadv_priv *bat_priv,
				    struct batadv_hard_iface *primary_if,
				    uint8_t *hw_src, uint8_t *hw_dst,
				    struct ethhdr *ethhdr)
{
	uint8_t *backbone_addr;
	struct batadv_orig_node *orig_node;
	struct batadv_bla_claim_dst *bla_dst, *bla_dst_own;

	bla_dst = (struct batadv_bla_claim_dst *)hw_dst;
	bla_dst_own = &bat_priv->bla.claim_dest;

	/* if announcement packet, use the source,
	 * otherwise assume it is in the hw_src
	 */
	switch (bla_dst->type) {
	case BATADV_CLAIM_TYPE_CLAIM:
		backbone_addr = hw_src;
		break;
	case BATADV_CLAIM_TYPE_REQUEST:
	case BATADV_CLAIM_TYPE_ANNOUNCE:
	case BATADV_CLAIM_TYPE_UNCLAIM:
		backbone_addr = ethhdr->h_source;
		break;
	default:
		return 0;
	}

	/* don't accept claim frames from ourselves */
	if (batadv_compare_eth(backbone_addr, primary_if->net_dev->dev_addr))
		return 0;

	/* if its already the same group, it is fine. */
	if (bla_dst->group == bla_dst_own->group)
		return 2;

	/* lets see if this originator is in our mesh */
	orig_node = batadv_orig_hash_find(bat_priv, backbone_addr);

	/* dont accept claims from gateways which are not in
	 * the same mesh or group.
	 */
	if (!orig_node)
		return 1;

	/* if our mesh friends mac is bigger, use it for ourselves. */
	if (ntohs(bla_dst->group) > ntohs(bla_dst_own->group)) {
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "taking other backbones claim group: %#.4x\n",
			   ntohs(bla_dst->group));
		bla_dst_own->group = bla_dst->group;
	}

	batadv_orig_node_free_ref(orig_node);

	return 2;
}

/**
 * batadv_bla_process_claim
 * @bat_priv: the bat priv with all the soft interface information
 * @primary_if: the primary hard interface of this batman soft interface
 * @skb: the frame to be checked
 *
 * Check if this is a claim frame, and process it accordingly.
 *
 * returns 1 if it was a claim frame, otherwise return 0 to
 * tell the callee that it can use the frame on its own.
 */
static int batadv_bla_process_claim(struct batadv_priv *bat_priv,
				    struct batadv_hard_iface *primary_if,
				    struct sk_buff *skb)
{
	struct batadv_bla_claim_dst *bla_dst, *bla_dst_own;
	uint8_t *hw_src, *hw_dst;
	struct vlan_hdr *vhdr, vhdr_buf;
	struct ethhdr *ethhdr;
	struct arphdr *arphdr;
	unsigned short vid;
	int vlan_depth = 0;
	__be16 proto;
	int headlen;
	int ret;

	vid = batadv_get_vid(skb, 0);
	ethhdr = eth_hdr(skb);

	proto = ethhdr->h_proto;
	headlen = ETH_HLEN;
	if (vid & BATADV_VLAN_HAS_TAG) {
		/* Traverse the VLAN/Ethertypes.
		 *
		 * At this point it is known that the first protocol is a VLAN
		 * header, so start checking at the encapsulated protocol.
		 *
		 * The depth of the VLAN headers is recorded to drop BLA claim
		 * frames encapsulated into multiple VLAN headers (QinQ).
		 */
		do {
			vhdr = skb_header_pointer(skb, headlen, VLAN_HLEN,
						  &vhdr_buf);
			if (!vhdr)
				return 0;

			proto = vhdr->h_vlan_encapsulated_proto;
			headlen += VLAN_HLEN;
			vlan_depth++;
		} while (proto == htons(ETH_P_8021Q));
	}

	if (proto != htons(ETH_P_ARP))
		return 0; /* not a claim frame */

	/* this must be a ARP frame. check if it is a claim. */

	if (unlikely(!pskb_may_pull(skb, headlen + arp_hdr_len(skb->dev))))
		return 0;

	/* pskb_may_pull() may have modified the pointers, get ethhdr again */
	ethhdr = eth_hdr(skb);
	arphdr = (struct arphdr *)((uint8_t *)ethhdr + headlen);

	/* Check whether the ARP frame carries a valid
	 * IP information
	 */
	if (arphdr->ar_hrd != htons(ARPHRD_ETHER))
		return 0;
	if (arphdr->ar_pro != htons(ETH_P_IP))
		return 0;
	if (arphdr->ar_hln != ETH_ALEN)
		return 0;
	if (arphdr->ar_pln != 4)
		return 0;

	hw_src = (uint8_t *)arphdr + sizeof(struct arphdr);
	hw_dst = hw_src + ETH_ALEN + 4;
	bla_dst = (struct batadv_bla_claim_dst *)hw_dst;
	bla_dst_own = &bat_priv->bla.claim_dest;

	/* check if it is a claim frame in general */
	if (memcmp(bla_dst->magic, bla_dst_own->magic,
		   sizeof(bla_dst->magic)) != 0)
		return 0;

	/* check if there is a claim frame encapsulated deeper in (QinQ) and
	 * drop that, as this is not supported by BLA but should also not be
	 * sent via the mesh.
	 */
	if (vlan_depth > 1)
		return 1;

	/* check if it is a claim frame. */
	ret = batadv_check_claim_group(bat_priv, primary_if, hw_src, hw_dst,
				       ethhdr);
	if (ret == 1)
		batadv_dbg(BATADV_DBG_BLA, bat_priv,
			   "bla_process_claim(): received a claim frame from another group. From: %pM on vid %d ...(hw_src %pM, hw_dst %pM)\n",
			   ethhdr->h_source, BATADV_PRINT_VID(vid), hw_src,
			   hw_dst);

	if (ret < 2)
		return ret;

	/* become a backbone gw ourselves on this vlan if not happened yet */
	batadv_bla_update_own_backbone_gw(bat_priv, primary_if, vid);

	/* check for the different types of claim frames ... */
	switch (bla_dst->type) {
	case BATADV_CLAIM_TYPE_CLAIM:
		if (batadv_handle_claim(bat_priv, primary_if, hw_src,
					ethhdr->h_source, vid))
			return 1;
		break;
	case BATADV_CLAIM_TYPE_UNCLAIM:
		if (batadv_handle_unclaim(bat_priv, primary_if,
					  ethhdr->h_source, hw_src, vid))
			return 1;
		break;

	case BATADV_CLAIM_TYPE_ANNOUNCE:
		if (batadv_handle_announce(bat_priv, hw_src, ethhdr->h_source,
					   vid))
			return 1;
		break;
	case BATADV_CLAIM_TYPE_REQUEST:
		if (batadv_handle_request(bat_priv, primary_if, hw_src, ethhdr,
					  vid))
			return 1;
		break;
	}

	batadv_dbg(BATADV_DBG_BLA, bat_priv,
		   "bla_process_claim(): ERROR - this looks like a claim frame, but is useless. eth src %pM on vid %d ...(hw_src %pM, hw_dst %pM)\n",
		   ethhdr->h_source, BATADV_PRINT_VID(vid), hw_src, hw_dst);
	return 1;
}

/* Check when we last heard from other nodes, and remove them in case of
 * a time out, or clean all backbone gws if now is set.
 */
static void batadv_bla_purge_backbone_gw(struct batadv_priv *bat_priv, int now)
{
	struct batadv_bla_backbone_gw *backbone_gw;
	struct hlist_node *node_tmp;
	struct hlist_head *head;
	struct batadv_hashtable *hash;
	spinlock_t *list_lock;	/* protects write access to the hash lists */
	int i;

	hash = bat_priv->bla.backbone_hash;
	if (!hash)
		return;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];
		list_lock = &hash->list_locks[i];

		spin_lock_bh(list_lock);
		hlist_for_each_entry_safe(backbone_gw, node_tmp,
					  head, hash_entry) {
			if (now)
				goto purge_now;
			if (!batadv_has_timed_out(backbone_gw->lasttime,
						  BATADV_BLA_BACKBONE_TIMEOUT))
				continue;

			batadv_dbg(BATADV_DBG_BLA, backbone_gw->bat_priv,
				   "bla_purge_backbone_gw(): backbone gw %pM timed out\n",
				   backbone_gw->orig);

purge_now:
			/* don't wait for the pending request anymore */
			if (atomic_read(&backbone_gw->request_sent))
				atomic_dec(&bat_priv->bla.num_requests);

			batadv_bla_del_backbone_claims(backbone_gw);

			hlist_del_rcu(&backbone_gw->hash_entry);
			batadv_backbone_gw_free_ref(backbone_gw);
		}
		spin_unlock_bh(list_lock);
	}
}

/**
 * batadv_bla_purge_claims
 * @bat_priv: the bat priv with all the soft interface information
 * @primary_if: the selected primary interface, may be NULL if now is set
 * @now: whether the whole hash shall be wiped now
 *
 * Check when we heard last time from our own claims, and remove them in case of
 * a time out, or clean all claims if now is set
 */
static void batadv_bla_purge_claims(struct batadv_priv *bat_priv,
				    struct batadv_hard_iface *primary_if,
				    int now)
{
	struct batadv_bla_claim *claim;
	struct hlist_head *head;
	struct batadv_hashtable *hash;
	int i;

	hash = bat_priv->bla.claim_hash;
	if (!hash)
		return;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(claim, head, hash_entry) {
			if (now)
				goto purge_now;
			if (!batadv_compare_eth(claim->backbone_gw->orig,
						primary_if->net_dev->dev_addr))
				continue;
			if (!batadv_has_timed_out(claim->lasttime,
						  BATADV_BLA_CLAIM_TIMEOUT))
				continue;

			batadv_dbg(BATADV_DBG_BLA, bat_priv,
				   "bla_purge_claims(): %pM, vid %d, time out\n",
				   claim->addr, claim->vid);

purge_now:
			batadv_handle_unclaim(bat_priv, primary_if,
					      claim->backbone_gw->orig,
					      claim->addr, claim->vid);
		}
		rcu_read_unlock();
	}
}

/**
 * batadv_bla_update_orig_address
 * @bat_priv: the bat priv with all the soft interface information
 * @primary_if: the new selected primary_if
 * @oldif: the old primary interface, may be NULL
 *
 * Update the backbone gateways when the own orig address changes.
 */
void batadv_bla_update_orig_address(struct batadv_priv *bat_priv,
				    struct batadv_hard_iface *primary_if,
				    struct batadv_hard_iface *oldif)
{
	struct batadv_bla_backbone_gw *backbone_gw;
	struct hlist_head *head;
	struct batadv_hashtable *hash;
	__be16 group;
	int i;

	/* reset bridge loop avoidance group id */
	group = htons(crc16(0, primary_if->net_dev->dev_addr, ETH_ALEN));
	bat_priv->bla.claim_dest.group = group;

	/* purge everything when bridge loop avoidance is turned off */
	if (!atomic_read(&bat_priv->bridge_loop_avoidance))
		oldif = NULL;

	if (!oldif) {
		batadv_bla_purge_claims(bat_priv, NULL, 1);
		batadv_bla_purge_backbone_gw(bat_priv, 1);
		return;
	}

	hash = bat_priv->bla.backbone_hash;
	if (!hash)
		return;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(backbone_gw, head, hash_entry) {
			/* own orig still holds the old value. */
			if (!batadv_compare_eth(backbone_gw->orig,
						oldif->net_dev->dev_addr))
				continue;

			ether_addr_copy(backbone_gw->orig,
					primary_if->net_dev->dev_addr);
			/* send an announce frame so others will ask for our
			 * claims and update their tables.
			 */
			batadv_bla_send_announce(bat_priv, backbone_gw);
		}
		rcu_read_unlock();
	}
}

/* periodic work to do:
 *  * purge structures when they are too old
 *  * send announcements
 */
static void batadv_bla_periodic_work(struct work_struct *work)
{
	struct delayed_work *delayed_work;
	struct batadv_priv *bat_priv;
	struct batadv_priv_bla *priv_bla;
	struct hlist_head *head;
	struct batadv_bla_backbone_gw *backbone_gw;
	struct batadv_hashtable *hash;
	struct batadv_hard_iface *primary_if;
	int i;

	delayed_work = container_of(work, struct delayed_work, work);
	priv_bla = container_of(delayed_work, struct batadv_priv_bla, work);
	bat_priv = container_of(priv_bla, struct batadv_priv, bla);
	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	batadv_bla_purge_claims(bat_priv, primary_if, 0);
	batadv_bla_purge_backbone_gw(bat_priv, 0);

	if (!atomic_read(&bat_priv->bridge_loop_avoidance))
		goto out;

	hash = bat_priv->bla.backbone_hash;
	if (!hash)
		goto out;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(backbone_gw, head, hash_entry) {
			if (!batadv_compare_eth(backbone_gw->orig,
						primary_if->net_dev->dev_addr))
				continue;

			backbone_gw->lasttime = jiffies;

			batadv_bla_send_announce(bat_priv, backbone_gw);

			/* request_sent is only set after creation to avoid
			 * problems when we are not yet known as backbone gw
			 * in the backbone.
			 *
			 * We can reset this now after we waited some periods
			 * to give bridge forward delays and bla group forming
			 * some grace time.
			 */

			if (atomic_read(&backbone_gw->request_sent) == 0)
				continue;

			if (!atomic_dec_and_test(&backbone_gw->wait_periods))
				continue;

			atomic_dec(&backbone_gw->bat_priv->bla.num_requests);
			atomic_set(&backbone_gw->request_sent, 0);
		}
		rcu_read_unlock();
	}
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);

	queue_delayed_work(batadv_event_workqueue, &bat_priv->bla.work,
			   msecs_to_jiffies(BATADV_BLA_PERIOD_LENGTH));
}

/* The hash for claim and backbone hash receive the same key because they
 * are getting initialized by hash_new with the same key. Reinitializing
 * them with to different keys to allow nested locking without generating
 * lockdep warnings
 */
static struct lock_class_key batadv_claim_hash_lock_class_key;
static struct lock_class_key batadv_backbone_hash_lock_class_key;

/* initialize all bla structures */
int batadv_bla_init(struct batadv_priv *bat_priv)
{
	int i;
	uint8_t claim_dest[ETH_ALEN] = {0xff, 0x43, 0x05, 0x00, 0x00, 0x00};
	struct batadv_hard_iface *primary_if;
	uint16_t crc;
	unsigned long entrytime;

	spin_lock_init(&bat_priv->bla.bcast_duplist_lock);

	batadv_dbg(BATADV_DBG_BLA, bat_priv, "bla hash registering\n");

	/* setting claim destination address */
	memcpy(&bat_priv->bla.claim_dest.magic, claim_dest, 3);
	bat_priv->bla.claim_dest.type = 0;
	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (primary_if) {
		crc = crc16(0, primary_if->net_dev->dev_addr, ETH_ALEN);
		bat_priv->bla.claim_dest.group = htons(crc);
		batadv_hardif_free_ref(primary_if);
	} else {
		bat_priv->bla.claim_dest.group = 0; /* will be set later */
	}

	/* initialize the duplicate list */
	entrytime = jiffies - msecs_to_jiffies(BATADV_DUPLIST_TIMEOUT);
	for (i = 0; i < BATADV_DUPLIST_SIZE; i++)
		bat_priv->bla.bcast_duplist[i].entrytime = entrytime;
	bat_priv->bla.bcast_duplist_curr = 0;

	if (bat_priv->bla.claim_hash)
		return 0;

	bat_priv->bla.claim_hash = batadv_hash_new(128);
	bat_priv->bla.backbone_hash = batadv_hash_new(32);

	if (!bat_priv->bla.claim_hash || !bat_priv->bla.backbone_hash)
		return -ENOMEM;

	batadv_hash_set_lock_class(bat_priv->bla.claim_hash,
				   &batadv_claim_hash_lock_class_key);
	batadv_hash_set_lock_class(bat_priv->bla.backbone_hash,
				   &batadv_backbone_hash_lock_class_key);

	batadv_dbg(BATADV_DBG_BLA, bat_priv, "bla hashes initialized\n");

	INIT_DELAYED_WORK(&bat_priv->bla.work, batadv_bla_periodic_work);

	queue_delayed_work(batadv_event_workqueue, &bat_priv->bla.work,
			   msecs_to_jiffies(BATADV_BLA_PERIOD_LENGTH));
	return 0;
}

/**
 * batadv_bla_check_bcast_duplist
 * @bat_priv: the bat priv with all the soft interface information
 * @skb: contains the bcast_packet to be checked
 *
 * check if it is on our broadcast list. Another gateway might
 * have sent the same packet because it is connected to the same backbone,
 * so we have to remove this duplicate.
 *
 * This is performed by checking the CRC, which will tell us
 * with a good chance that it is the same packet. If it is furthermore
 * sent by another host, drop it. We allow equal packets from
 * the same host however as this might be intended.
 */
int batadv_bla_check_bcast_duplist(struct batadv_priv *bat_priv,
				   struct sk_buff *skb)
{
	int i, curr, ret = 0;
	__be32 crc;
	struct batadv_bcast_packet *bcast_packet;
	struct batadv_bcast_duplist_entry *entry;

	bcast_packet = (struct batadv_bcast_packet *)skb->data;

	/* calculate the crc ... */
	crc = batadv_skb_crc32(skb, (u8 *)(bcast_packet + 1));

	spin_lock_bh(&bat_priv->bla.bcast_duplist_lock);

	for (i = 0; i < BATADV_DUPLIST_SIZE; i++) {
		curr = (bat_priv->bla.bcast_duplist_curr + i);
		curr %= BATADV_DUPLIST_SIZE;
		entry = &bat_priv->bla.bcast_duplist[curr];

		/* we can stop searching if the entry is too old ;
		 * later entries will be even older
		 */
		if (batadv_has_timed_out(entry->entrytime,
					 BATADV_DUPLIST_TIMEOUT))
			break;

		if (entry->crc != crc)
			continue;

		if (batadv_compare_eth(entry->orig, bcast_packet->orig))
			continue;

		/* this entry seems to match: same crc, not too old,
		 * and from another gw. therefore return 1 to forbid it.
		 */
		ret = 1;
		goto out;
	}
	/* not found, add a new entry (overwrite the oldest entry)
	 * and allow it, its the first occurrence.
	 */
	curr = (bat_priv->bla.bcast_duplist_curr + BATADV_DUPLIST_SIZE - 1);
	curr %= BATADV_DUPLIST_SIZE;
	entry = &bat_priv->bla.bcast_duplist[curr];
	entry->crc = crc;
	entry->entrytime = jiffies;
	ether_addr_copy(entry->orig, bcast_packet->orig);
	bat_priv->bla.bcast_duplist_curr = curr;

out:
	spin_unlock_bh(&bat_priv->bla.bcast_duplist_lock);

	return ret;
}

/**
 * batadv_bla_is_backbone_gw_orig
 * @bat_priv: the bat priv with all the soft interface information
 * @orig: originator mac address
 * @vid: VLAN identifier
 *
 * Check if the originator is a gateway for the VLAN identified by vid.
 *
 * Returns true if orig is a backbone for this vid, false otherwise.
 */
bool batadv_bla_is_backbone_gw_orig(struct batadv_priv *bat_priv, uint8_t *orig,
				    unsigned short vid)
{
	struct batadv_hashtable *hash = bat_priv->bla.backbone_hash;
	struct hlist_head *head;
	struct batadv_bla_backbone_gw *backbone_gw;
	int i;

	if (!atomic_read(&bat_priv->bridge_loop_avoidance))
		return false;

	if (!hash)
		return false;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(backbone_gw, head, hash_entry) {
			if (batadv_compare_eth(backbone_gw->orig, orig) &&
			    backbone_gw->vid == vid) {
				rcu_read_unlock();
				return true;
			}
		}
		rcu_read_unlock();
	}

	return false;
}

/**
 * batadv_bla_is_backbone_gw
 * @skb: the frame to be checked
 * @orig_node: the orig_node of the frame
 * @hdr_size: maximum length of the frame
 *
 * bla_is_backbone_gw inspects the skb for the VLAN ID and returns 1
 * if the orig_node is also a gateway on the soft interface, otherwise it
 * returns 0.
 */
int batadv_bla_is_backbone_gw(struct sk_buff *skb,
			      struct batadv_orig_node *orig_node, int hdr_size)
{
	struct batadv_bla_backbone_gw *backbone_gw;
	unsigned short vid;

	if (!atomic_read(&orig_node->bat_priv->bridge_loop_avoidance))
		return 0;

	/* first, find out the vid. */
	if (!pskb_may_pull(skb, hdr_size + ETH_HLEN))
		return 0;

	vid = batadv_get_vid(skb, hdr_size);

	/* see if this originator is a backbone gw for this VLAN */
	backbone_gw = batadv_backbone_hash_find(orig_node->bat_priv,
						orig_node->orig, vid);
	if (!backbone_gw)
		return 0;

	batadv_backbone_gw_free_ref(backbone_gw);
	return 1;
}

/* free all bla structures (for softinterface free or module unload) */
void batadv_bla_free(struct batadv_priv *bat_priv)
{
	struct batadv_hard_iface *primary_if;

	cancel_delayed_work_sync(&bat_priv->bla.work);
	primary_if = batadv_primary_if_get_selected(bat_priv);

	if (bat_priv->bla.claim_hash) {
		batadv_bla_purge_claims(bat_priv, primary_if, 1);
		batadv_hash_destroy(bat_priv->bla.claim_hash);
		bat_priv->bla.claim_hash = NULL;
	}
	if (bat_priv->bla.backbone_hash) {
		batadv_bla_purge_backbone_gw(bat_priv, 1);
		batadv_hash_destroy(bat_priv->bla.backbone_hash);
		bat_priv->bla.backbone_hash = NULL;
	}
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
}

/**
 * batadv_bla_rx
 * @bat_priv: the bat priv with all the soft interface information
 * @skb: the frame to be checked
 * @vid: the VLAN ID of the frame
 * @is_bcast: the packet came in a broadcast packet type.
 *
 * bla_rx avoidance checks if:
 *  * we have to race for a claim
 *  * if the frame is allowed on the LAN
 *
 * in these cases, the skb is further handled by this function and
 * returns 1, otherwise it returns 0 and the caller shall further
 * process the skb.
 */
int batadv_bla_rx(struct batadv_priv *bat_priv, struct sk_buff *skb,
		  unsigned short vid, bool is_bcast)
{
	struct ethhdr *ethhdr;
	struct batadv_bla_claim search_claim, *claim = NULL;
	struct batadv_hard_iface *primary_if;
	int ret;

	ethhdr = eth_hdr(skb);

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto handled;

	if (!atomic_read(&bat_priv->bridge_loop_avoidance))
		goto allow;

	if (unlikely(atomic_read(&bat_priv->bla.num_requests)))
		/* don't allow broadcasts while requests are in flight */
		if (is_multicast_ether_addr(ethhdr->h_dest) && is_bcast)
			goto handled;

	ether_addr_copy(search_claim.addr, ethhdr->h_source);
	search_claim.vid = vid;
	claim = batadv_claim_hash_find(bat_priv, &search_claim);

	if (!claim) {
		/* possible optimization: race for a claim */
		/* No claim exists yet, claim it for us!
		 */
		batadv_handle_claim(bat_priv, primary_if,
				    primary_if->net_dev->dev_addr,
				    ethhdr->h_source, vid);
		goto allow;
	}

	/* if it is our own claim ... */
	if (batadv_compare_eth(claim->backbone_gw->orig,
			       primary_if->net_dev->dev_addr)) {
		/* ... allow it in any case */
		claim->lasttime = jiffies;
		goto allow;
	}

	/* if it is a broadcast ... */
	if (is_multicast_ether_addr(ethhdr->h_dest) && is_bcast) {
		/* ... drop it. the responsible gateway is in charge.
		 *
		 * We need to check is_bcast because with the gateway
		 * feature, broadcasts (like DHCP requests) may be sent
		 * using a unicast packet type.
		 */
		goto handled;
	} else {
		/* seems the client considers us as its best gateway.
		 * send a claim and update the claim table
		 * immediately.
		 */
		batadv_handle_claim(bat_priv, primary_if,
				    primary_if->net_dev->dev_addr,
				    ethhdr->h_source, vid);
		goto allow;
	}
allow:
	batadv_bla_update_own_backbone_gw(bat_priv, primary_if, vid);
	ret = 0;
	goto out;

handled:
	kfree_skb(skb);
	ret = 1;

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	if (claim)
		batadv_claim_free_ref(claim);
	return ret;
}

/**
 * batadv_bla_tx
 * @bat_priv: the bat priv with all the soft interface information
 * @skb: the frame to be checked
 * @vid: the VLAN ID of the frame
 *
 * bla_tx checks if:
 *  * a claim was received which has to be processed
 *  * the frame is allowed on the mesh
 *
 * in these cases, the skb is further handled by this function and
 * returns 1, otherwise it returns 0 and the caller shall further
 * process the skb.
 *
 * This call might reallocate skb data.
 */
int batadv_bla_tx(struct batadv_priv *bat_priv, struct sk_buff *skb,
		  unsigned short vid)
{
	struct ethhdr *ethhdr;
	struct batadv_bla_claim search_claim, *claim = NULL;
	struct batadv_hard_iface *primary_if;
	int ret = 0;

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	if (!atomic_read(&bat_priv->bridge_loop_avoidance))
		goto allow;

	if (batadv_bla_process_claim(bat_priv, primary_if, skb))
		goto handled;

	ethhdr = eth_hdr(skb);

	if (unlikely(atomic_read(&bat_priv->bla.num_requests)))
		/* don't allow broadcasts while requests are in flight */
		if (is_multicast_ether_addr(ethhdr->h_dest))
			goto handled;

	ether_addr_copy(search_claim.addr, ethhdr->h_source);
	search_claim.vid = vid;

	claim = batadv_claim_hash_find(bat_priv, &search_claim);

	/* if no claim exists, allow it. */
	if (!claim)
		goto allow;

	/* check if we are responsible. */
	if (batadv_compare_eth(claim->backbone_gw->orig,
			       primary_if->net_dev->dev_addr)) {
		/* if yes, the client has roamed and we have
		 * to unclaim it.
		 */
		if (batadv_has_timed_out(claim->lasttime, 100)) {
			/* only unclaim if the last claim entry is
			 * older than 100 ms to make sure we really
			 * have a roaming client here.
			 */
			batadv_dbg(BATADV_DBG_BLA, bat_priv, "bla_tx(): Roaming client %pM detected. Unclaim it.\n",
				   ethhdr->h_source);
			batadv_handle_unclaim(bat_priv, primary_if,
					      primary_if->net_dev->dev_addr,
					      ethhdr->h_source, vid);
			goto allow;
		} else {
			batadv_dbg(BATADV_DBG_BLA, bat_priv, "bla_tx(): Race for claim %pM detected. Drop packet.\n",
				   ethhdr->h_source);
			goto handled;
		}
	}

	/* check if it is a multicast/broadcast frame */
	if (is_multicast_ether_addr(ethhdr->h_dest)) {
		/* drop it. the responsible gateway has forwarded it into
		 * the backbone network.
		 */
		goto handled;
	} else {
		/* we must allow it. at least if we are
		 * responsible for the DESTINATION.
		 */
		goto allow;
	}
allow:
	batadv_bla_update_own_backbone_gw(bat_priv, primary_if, vid);
	ret = 0;
	goto out;
handled:
	ret = 1;
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	if (claim)
		batadv_claim_free_ref(claim);
	return ret;
}

int batadv_bla_claim_table_seq_print_text(struct seq_file *seq, void *offset)
{
	struct net_device *net_dev = (struct net_device *)seq->private;
	struct batadv_priv *bat_priv = netdev_priv(net_dev);
	struct batadv_hashtable *hash = bat_priv->bla.claim_hash;
	struct batadv_bla_claim *claim;
	struct batadv_hard_iface *primary_if;
	struct hlist_head *head;
	uint32_t i;
	bool is_own;
	uint8_t *primary_addr;

	primary_if = batadv_seq_print_text_primary_if_get(seq);
	if (!primary_if)
		goto out;

	primary_addr = primary_if->net_dev->dev_addr;
	seq_printf(seq,
		   "Claims announced for the mesh %s (orig %pM, group id %#.4x)\n",
		   net_dev->name, primary_addr,
		   ntohs(bat_priv->bla.claim_dest.group));
	seq_printf(seq, "   %-17s    %-5s    %-17s [o] (%-6s)\n",
		   "Client", "VID", "Originator", "CRC");
	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(claim, head, hash_entry) {
			is_own = batadv_compare_eth(claim->backbone_gw->orig,
						    primary_addr);
			seq_printf(seq, " * %pM on %5d by %pM [%c] (%#.4x)\n",
				   claim->addr, BATADV_PRINT_VID(claim->vid),
				   claim->backbone_gw->orig,
				   (is_own ? 'x' : ' '),
				   claim->backbone_gw->crc);
		}
		rcu_read_unlock();
	}
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	return 0;
}

int batadv_bla_backbone_table_seq_print_text(struct seq_file *seq, void *offset)
{
	struct net_device *net_dev = (struct net_device *)seq->private;
	struct batadv_priv *bat_priv = netdev_priv(net_dev);
	struct batadv_hashtable *hash = bat_priv->bla.backbone_hash;
	struct batadv_bla_backbone_gw *backbone_gw;
	struct batadv_hard_iface *primary_if;
	struct hlist_head *head;
	int secs, msecs;
	uint32_t i;
	bool is_own;
	uint8_t *primary_addr;

	primary_if = batadv_seq_print_text_primary_if_get(seq);
	if (!primary_if)
		goto out;

	primary_addr = primary_if->net_dev->dev_addr;
	seq_printf(seq,
		   "Backbones announced for the mesh %s (orig %pM, group id %#.4x)\n",
		   net_dev->name, primary_addr,
		   ntohs(bat_priv->bla.claim_dest.group));
	seq_printf(seq, "   %-17s    %-5s %-9s (%-6s)\n",
		   "Originator", "VID", "last seen", "CRC");
	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(backbone_gw, head, hash_entry) {
			msecs = jiffies_to_msecs(jiffies -
						 backbone_gw->lasttime);
			secs = msecs / 1000;
			msecs = msecs % 1000;

			is_own = batadv_compare_eth(backbone_gw->orig,
						    primary_addr);
			if (is_own)
				continue;

			seq_printf(seq, " * %pM on %5d %4i.%03is (%#.4x)\n",
				   backbone_gw->orig,
				   BATADV_PRINT_VID(backbone_gw->vid), secs,
				   msecs, backbone_gw->crc);
		}
		rcu_read_unlock();
	}
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	return 0;
}
