/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018  Intel Corporation. All rights reserved.
 *
 *
 */


/*
 * size: hard define (mesh.conf - OOB_NUMBEROOB_NUMBER)
 *      oob size - 8 if alpha or numeric
 *	else 1 if mask is non zero
 *	else 0
 */
struct bt_mesh;
struct mesh_prov;
struct mesh_agent;

/* Provisioner Agent Response Types */
#define OOB_CANCEL		0x00
#define OOB_PRIV_KEY		0x01
#define OOB_PUB_KEY		0x02
#define OOB_NUMBER		0x03
#define OOB_STATIC		0x04
#define OOB_NUMBER_DISPLAY	0x05

/* Spec defined Provisioning message types */
#define PROV_INVITE	0x00
#define PROV_CAPS	0x01
#define PROV_START	0x02
#define PROV_PUB_KEY	0x03
#define PROV_INP_CMPLT	0x04
#define PROV_CONFIRM	0x05
#define PROV_RANDOM	0x06
#define PROV_DATA	0x07
#define PROV_COMPLETE	0x08
#define PROV_FAILED	0x09

/* Spec defined Error Codes */
#define PROV_ERR_SUCCESS		0x00
#define PROV_ERR_INVALID_PDU		0x01
#define PROV_ERR_INVALID_FORMAT		0x02
#define PROV_ERR_UNEXPECTED_PDU		0x03
#define PROV_ERR_CONFIRM_FAILED		0x04
#define PROV_ERR_INSUF_RESOURCE		0x05
#define PROV_ERR_DECRYPT_FAILED		0x06
#define PROV_ERR_UNEXPECTED_ERR		0x07
#define PROV_ERR_CANT_ASSIGN_ADDR	0x08
/* Internally generated Error Codes */
#define PROV_ERR_TIMEOUT		0xFF

/* Provisioner Action values */
/* IN */
#define PROV_ACTION_PUSH		0x00
#define PROV_ACTION_TWIST		0x01
#define PROV_ACTION_IN_NUMERIC		0x02
#define PROV_ACTION_IN_ALPHA		0x03
/* OUT */
#define PROV_ACTION_BLINK		0x00
#define PROV_ACTION_BEEP		0x01
#define PROV_ACTION_VIBRATE		0x02
#define PROV_ACTION_OUT_NUMERIC		0x03
#define PROV_ACTION_OUT_ALPHA		0x04

/* OOB_Info defines from Table 3.54 of Mesh profile Specification v1.0 */
#define OOB_INFO_URI_HASH	0x0002

/* PB_REMOTE not supported from unprovisioned state */
enum trans_type {
	PB_ADV = 0,
	PB_GATT,
};

#define PROV_FLAG_KR	0x01
#define PROV_FLAG_IVU	0x02

struct mesh_prov_node_info {
	uint32_t iv_index;
	uint16_t unicast;
	uint16_t net_index;
	uint8_t num_ele;
	uint8_t net_key[16];
	uint8_t device_key[16];
	uint8_t flags; /* IVU and KR bits */
};

typedef bool (*mesh_prov_acceptor_complete_func_t)(void *user_data,
					uint8_t status,
					struct mesh_prov_node_info *info);

typedef void (*mesh_prov_initiator_start_func_t)(void *user_data, int err);

typedef bool (*mesh_prov_initiator_data_req_func_t)(void *user_data,
							uint8_t num_elem);

typedef bool (*mesh_prov_initiator_complete_func_t)(void *user_data,
					uint8_t status,
					struct mesh_prov_node_info *info);

/* This starts unprovisioned device beacon */
bool acceptor_start(uint8_t num_ele, uint8_t uuid[16],
			uint16_t algorithms, uint32_t timeout,
			struct mesh_agent *agent,
			mesh_prov_acceptor_complete_func_t complete_cb,
			void *caller_data);
void acceptor_cancel(void *user_data);

bool initiator_start(enum trans_type transport,
		uint8_t uuid[16],
		uint16_t max_ele,
		uint32_t timeout, /* in seconds from mesh.conf */
		struct mesh_agent *agent,
		mesh_prov_initiator_start_func_t start_cb,
		mesh_prov_initiator_data_req_func_t data_req_cb,
		mesh_prov_initiator_complete_func_t complete_cb,
		void *node, void *caller_data);
void initiator_prov_data(uint16_t net_idx, uint16_t primary, void *caller_data);
void initiator_cancel(void *caller_data);
