// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/util.h"
#include "mesh/crypto.h"
#include "mesh/mesh-io.h"
#include "mesh/net.h"
#include "mesh/net-keys.h"

#define BEACON_TYPE_SNB		0x01
#define KEY_REFRESH		0x01
#define IV_INDEX_UPDATE		0x02

#define BEACON_INTERVAL_MIN	10
#define BEACON_INTERVAL_MAX	600

struct net_beacon {
	struct l_timeout *timeout;
	uint32_t ts;
	uint16_t observe_period;
	uint16_t observed;
	uint16_t expected;
	bool half_period;
	uint8_t beacon[23];
};

struct net_key {
	uint32_t id;
	struct net_beacon snb;
	uint16_t ref_cnt;
	uint16_t beacon_enables;
	uint8_t friend_key;
	uint8_t nid;
	uint8_t master[16];
	uint8_t encrypt[16];
	uint8_t privacy[16];
	uint8_t beacon[16];
	uint8_t network[8];
};

static struct l_queue *keys = NULL;
static uint32_t last_master_id = 0;

/* To avoid re-decrypting same packet for multiple nodes, cache and check */
static uint8_t cache_pkt[29];
static uint8_t cache_plain[29];
static size_t cache_len;
static size_t cache_plainlen;
static uint32_t cache_id;
static uint32_t cache_iv_index;

static bool match_master(const void *a, const void *b)
{
	const struct net_key *key = a;

	return (memcmp(key->master, b, sizeof(key->master)) == 0);
}

static bool match_id(const void *a, const void *b)
{
	const struct net_key *key = a;
	uint32_t id = L_PTR_TO_UINT(b);

	return id == key->id;
}

static bool match_network(const void *a, const void *b)
{
	const struct net_key *key = a;
	const uint8_t *network = b;

	return memcmp(key->network, network, sizeof(key->network)) == 0;
}

/* Key added from Provisioning, NetKey Add or NetKey update */
uint32_t net_key_add(const uint8_t master[16])
{
	struct net_key *key = l_queue_find(keys, match_master, master);
	uint8_t p[] = {0};
	bool result;

	if (key) {
		key->ref_cnt++;
		return key->id;
	}

	if (!keys)
		keys = l_queue_new();

	key = l_new(struct net_key, 1);
	memcpy(key->master, master, 16);
	key->ref_cnt++;
	result = mesh_crypto_k2(master, p, sizeof(p), &key->nid, key->encrypt,
								key->privacy);
	if (!result)
		goto fail;

	result = mesh_crypto_k3(master, key->network);
	if (!result)
		goto fail;

	result = mesh_crypto_nkbk(master, key->beacon);
	if (!result)
		goto fail;

	key->id = ++last_master_id;
	l_queue_push_tail(keys, key);
	return key->id;

fail:
	l_free(key);
	return 0;
}

uint32_t net_key_frnd_add(uint32_t master_id, uint16_t lpn, uint16_t frnd,
					uint16_t lp_cnt, uint16_t fn_cnt)
{
	const struct net_key *key = l_queue_find(keys, match_id,
						L_UINT_TO_PTR(master_id));
	struct net_key *frnd_key;
	uint8_t p[9] = {0x01};
	bool result;

	if (!key || key->friend_key)
		return 0;

	frnd_key = l_new(struct net_key, 1);

	l_put_be16(lpn, p + 1);
	l_put_be16(frnd, p + 3);
	l_put_be16(lp_cnt, p + 5);
	l_put_be16(fn_cnt, p + 7);

	result = mesh_crypto_k2(key->master, p, sizeof(p), &frnd_key->nid,
				frnd_key->encrypt, frnd_key->privacy);

	if (!result) {
		l_free(frnd_key);
		return 0;
	}

	frnd_key->friend_key = true;
	frnd_key->ref_cnt++;
	frnd_key->id = ++last_master_id;
	l_queue_push_head(keys, frnd_key);

	return frnd_key->id;
}

void net_key_unref(uint32_t id)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (key && key->ref_cnt) {
		if (--key->ref_cnt == 0) {
			l_timeout_remove(key->snb.timeout);
			l_queue_remove(keys, key);
			l_free(key);
		}
	}
}

bool net_key_confirm(uint32_t id, const uint8_t *master)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (key)
		return memcmp(key->master, master, sizeof(key->master)) == 0;

	return false;
}

bool net_key_retrieve(uint32_t id, uint8_t *master)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (key) {
		memcpy(master, key->master, sizeof(key->master));
		return true;
	}

	return false;
}

static void decrypt_net_pkt(void *a, void *b)
{
	const struct net_key *key = a;
	bool result;

	if (cache_id || !key->ref_cnt || (cache_pkt[0] & 0x7f) != key->nid)
		return;

	result = mesh_crypto_packet_decode(cache_pkt, cache_len, false,
						cache_plain, cache_iv_index,
						key->encrypt, key->privacy);

	if (result) {
		cache_id = key->id;
		if (cache_plain[1] & 0x80)
			cache_plainlen = cache_len - 8;
		else
			cache_plainlen = cache_len - 4;
	}
}

uint32_t net_key_decrypt(uint32_t iv_index, const uint8_t *pkt, size_t len,
					uint8_t **plain, size_t *plain_len)
{
	/* If we already successfully decrypted this packet, use cached data */
	if (cache_id && cache_len == len && !memcmp(pkt, cache_pkt, len)) {
		/* IV Index must match what was used to decrypt */
		if (cache_iv_index != iv_index)
			return 0;

		goto done;
	}

	cache_id = 0;
	memcpy(cache_pkt, pkt, len);
	cache_len = len;
	cache_iv_index = iv_index;

	/* Try all network keys known to us */
	l_queue_foreach(keys, decrypt_net_pkt, NULL);

done:
	if (cache_id) {
		*plain = cache_plain;
		*plain_len = cache_plainlen;
	}

	return cache_id;
}

bool net_key_encrypt(uint32_t id, uint32_t iv_index, uint8_t *pkt, size_t len)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));
	bool result;

	if (!key)
		return false;

	result = mesh_crypto_packet_encode(pkt, len, iv_index, key->encrypt,
							key->privacy);

	if (!result)
		return false;

	result = mesh_crypto_packet_label(pkt, len, iv_index, key->nid);

	return result;
}

uint32_t net_key_network_id(const uint8_t network[8])
{
	struct net_key *key = l_queue_find(keys, match_network, network);

	if (!key)
		return 0;

	return key->id;
}

bool net_key_snb_check(uint32_t id, uint32_t iv_index, bool kr, bool ivu,
								uint64_t cmac)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));
	uint64_t cmac_check;

	if (!key)
		return false;

	/* Any behavioral changes must pass CMAC test */
	if (!mesh_crypto_beacon_cmac(key->beacon, key->network, iv_index, kr,
							ivu, &cmac_check)) {
		l_error("mesh_crypto_beacon_cmac failed");
		return false;
	}

	if (cmac != cmac_check) {
		l_error("cmac compare failed 0x%16" PRIx64 " != 0x%16" PRIx64,
						cmac, cmac_check);
		return false;
	}

	return true;
}

bool net_key_snb_compose(uint32_t id, uint32_t iv_index, bool kr, bool ivu,
								uint8_t *snb)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));
	uint64_t cmac;

	if (!key)
		return false;

	/* Any behavioral changes must pass CMAC test */
	if (!mesh_crypto_beacon_cmac(key->beacon, key->network, iv_index, kr,
								ivu, &cmac)) {
		l_error("mesh_crypto_beacon_cmac failed");
		return false;
	}

	snb[0] = MESH_AD_TYPE_BEACON;
	snb[1] = BEACON_TYPE_SNB;
	snb[2] = 0;

	if (kr)
		snb[2] |= KEY_REFRESH;

	if (ivu)
		snb[2] |= IV_INDEX_UPDATE;

	memcpy(snb + 3, key->network, 8);
	l_put_be32(iv_index, snb + 11);
	l_put_be64(cmac, snb + 15);

	return true;
}

static void send_network_beacon(struct net_key *key)
{
	struct mesh_io_send_info info = {
		.type = MESH_IO_TIMING_TYPE_GENERAL,
		.u.gen.interval = 100,
		.u.gen.cnt = 1,
		.u.gen.min_delay = DEFAULT_MIN_DELAY,
		.u.gen.max_delay = DEFAULT_MAX_DELAY
	};

	mesh_io_send(NULL, &info, key->snb.beacon, sizeof(key->snb.beacon));
}

static void snb_timeout(struct l_timeout *timeout, void *user_data)
{
	struct net_key *key = user_data;
	uint32_t interval, scale_factor;

	/* Always send at least one beacon */
	send_network_beacon(key);

	/* Count our own beacons towards the vicinity total */
	key->snb.observed++;

	if (!key->snb.half_period) {

		l_debug("beacon %d for %d nodes, period %d, obs %d, exp %d",
						key->id,
						key->beacon_enables,
						key->snb.observe_period,
						key->snb.observed,
						key->snb.expected);


		interval = (key->snb.observe_period * key->snb.observed)
							/ key->snb.expected;

		/* Limit Increases and Decreases by 10 seconds Up and
		 * 20 seconds down each step, to avoid going nearly silent
		 * in highly populated environments.
		 */
		if (interval - 10 > key->snb.observe_period)
			interval = key->snb.observe_period + 10;
		else if (interval + 20 < key->snb.observe_period)
			interval = key->snb.observe_period - 20;

		/* Beaconing must be no *slower* than once every 10 minutes,
		 * and no *faster* than once every 10 seconds, per spec.
		 * Observation period is twice beaconing period.
		 */
		if (interval < BEACON_INTERVAL_MIN * 2)
			interval = BEACON_INTERVAL_MIN * 2;
		else if (interval > BEACON_INTERVAL_MAX * 2)
			interval = BEACON_INTERVAL_MAX * 2;

		key->snb.observe_period = interval;
		key->snb.observed = 0;

		/* To prevent "over slowing" of the beaconing frequency,
		 * require more significant "over observing" the slower
		 * our own beaconing frequency.
		 */
		key->snb.expected = interval / 10;
		scale_factor = interval / 60;
		key->snb.expected += scale_factor * 3;
	}

	interval = key->snb.observe_period / 2;
	key->snb.half_period = !key->snb.half_period;

	if (key->beacon_enables)
		l_timeout_modify(timeout, interval);
	else {
		l_timeout_remove(timeout);
		key->snb.timeout = NULL;
	}
}

void net_key_beacon_seen(uint32_t id)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (key) {
		key->snb.observed++;
		key->snb.ts = get_timestamp_secs();
	}
}

uint32_t net_key_beacon_last_seen(uint32_t id)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (key)
		return key->snb.ts;

	return 0;
}

void net_key_beacon_enable(uint32_t id)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));
	bool enabled;
	uint32_t rand_ms;

	if (!key)
		return;

	enabled = !!key->beacon_enables;
	key->beacon_enables++;

	/* If already Enabled, do nothing */
	if (enabled)
		return;

	/* Randomize first timeout to avoid bursts of beacons */
	l_getrandom(&rand_ms, sizeof(rand_ms));
	rand_ms %= (BEACON_INTERVAL_MIN * 1000);
	rand_ms++;

	/* Enable Periodic Beaconing on this key */
	key->snb.observe_period = BEACON_INTERVAL_MIN * 2;
	key->snb.expected = 2;
	key->snb.observed = 0;
	key->snb.half_period = true;
	l_timeout_remove(key->snb.timeout);
	key->snb.timeout = l_timeout_create_ms(rand_ms, snb_timeout, key, NULL);
}

bool net_key_beacon_refresh(uint32_t id, uint32_t iv_index, bool kr, bool ivu)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));
	uint8_t beacon[23];
	uint32_t rand_ms;

	if (!key)
		return false;

	if (!net_key_snb_compose(id, iv_index, kr, ivu, beacon))
		return false;

	if (memcmp(key->snb.beacon, beacon, sizeof(beacon)))
		memcpy(key->snb.beacon, beacon, sizeof(beacon));
	else
		return false;

	l_debug("Setting SNB: IVI: %8.8x, IVU: %d, KR: %d", iv_index, ivu, kr);
	print_packet("Set SNB Beacon to", beacon, sizeof(beacon));

	/* Propagate changes to all local nodes */
	net_local_beacon(id, beacon);

	/* Send one new SNB soon, after all nodes have seen it */
	l_getrandom(&rand_ms, sizeof(rand_ms));
	rand_ms %= 1000;
	key->snb.expected++;

	if (key->snb.timeout)
		l_timeout_modify_ms(key->snb.timeout, 500 + rand_ms);
	else
		key->snb.timeout = l_timeout_create_ms(500 + rand_ms,
						snb_timeout, key, NULL);

	return true;
}

void net_key_beacon_disable(uint32_t id)
{
	struct net_key *key = l_queue_find(keys, match_id, L_UINT_TO_PTR(id));

	if (!key || !key->beacon_enables)
		return;

	key->beacon_enables--;

	if (key->beacon_enables)
		return;

	/* Disable periodic Beaconing on this key */
	l_timeout_remove(key->snb.timeout);
	key->snb.timeout = NULL;
}

void net_key_cleanup(void)
{
	l_queue_destroy(keys, l_free);
	keys = NULL;
}
