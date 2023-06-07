/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */

#define OP_FRND_REQUEST			0x8040
#define OP_FRND_INQUIRY			0x8041
#define OP_FRND_CONFIRM			0x8042
#define OP_FRND_SUB_LIST_ADD		0x8043
#define OP_FRND_SUB_LIST_CONFIRM	0x8044
#define OP_FRND_SUB_LIST_REMOVE		0x8045
#define OP_FRND_NEGOTIATE		0x8046
#define OP_FRND_CLEAR			0x8047

void friend_poll(struct mesh_net *net, uint16_t src, bool seq,
						struct mesh_friend *frnd);
void friend_request(struct mesh_net *net, uint16_t net_idx, uint16_t src,
			uint8_t minReq, uint8_t delay, uint32_t timeout,
			uint16_t prev, uint8_t num_elements, uint16_t cntr,
			int8_t rssi);
void friend_clear_confirm(struct mesh_net *net, uint16_t src, uint16_t lpn,
							uint16_t lpnCounter);
void friend_clear(struct mesh_net *net, uint16_t src, uint16_t lpn,
			uint16_t lpnCounter, struct mesh_friend *frnd);
void friend_sub_add(struct mesh_net *net, struct mesh_friend *frnd,
					const uint8_t *pkt, uint8_t len);
void friend_sub_del(struct mesh_net *net, struct mesh_friend *frnd,
					const uint8_t *pkt, uint8_t len);
void mesh_friend_relay_init(struct mesh_net *net, uint16_t addr);

/* Low-Power-Node role */
void frnd_sub_add(struct mesh_net *net, uint32_t parms[7]);
void frnd_sub_del(struct mesh_net *net, uint32_t parms[7]);
void frnd_poll(struct mesh_net *net, bool retry);
void frnd_clear(struct mesh_net *net);
void frnd_ack_poll(struct mesh_net *net);
void frnd_poll_cancel(struct mesh_net *net);
void frnd_request_friend(struct mesh_net *net, uint8_t cache,
			uint8_t offer_delay, uint8_t delay, uint32_t timeout);
void frnd_offer(struct mesh_net *net, uint16_t src, uint8_t window,
			uint8_t cache, uint8_t sub_list_size,
			int8_t r_rssi, int8_t l_rssi, uint16_t fn_cnt);
void frnd_key_refresh(struct mesh_net *net, uint8_t phase);
uint32_t frnd_get_key(struct mesh_net *net);
