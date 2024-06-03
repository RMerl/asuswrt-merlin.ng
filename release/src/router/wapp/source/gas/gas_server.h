/*
 * Generic advertisement service (GAS) server
 * Copyright (c) 2017, Qualcomm Atheros, Inc.
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef GAS_SERVER_H
#define GAS_SERVER_H

struct gas_server;
struct wapp_dev;

struct gas_server * gas_server_init(void *ctx,
				    void (*tx)(void *ctx, struct wapp_dev *wdev, int freq,
					       const u8 *da,
					       struct wpabuf *buf,
					       unsigned int wait_time));
void gas_server_deinit(struct gas_server *gas);
int gas_server_register(struct gas_server *gas,
			const u8 *adv_proto_id, u8 adv_proto_id_len,
			struct wpabuf *
			(*req_cb)(void *ctx, const u8 *sa,
				  const u8 *query, size_t query_len, u8 *data, size_t data_len),
			void (*status_cb)(void *ctx, u8 *da,
					  int ok),
			void *ctx);
int gas_server_rx(struct gas_server *gas, struct wapp_dev *wdev, const u8 *da, const u8 *sa,
		  const u8 *bssid, u8 categ, const u8 *data, size_t len,
		  int freq);
void gas_server_tx_status(struct gas_server *gas, const u8 *dst, const u8 *data,
			  size_t data_len, int ack);
void gas_serv_req_dpp_processing(struct gas_server *gas,
				 struct wapp_dev *wdev,
       				 const u8 *sa, u8 dialog_token,
                                 int prot, struct wpabuf *buf);

#endif /* GAS_SERVER_H */
