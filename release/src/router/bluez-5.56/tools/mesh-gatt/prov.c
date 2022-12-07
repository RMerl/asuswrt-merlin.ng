// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wordexp.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <glib.h>

#include "src/shared/util.h"
#include "src/shared/ecc.h"
#include "src/shared/shell.h"

#include "gdbus/gdbus.h"

#include "tools/mesh/agent.h"

#include "tools/mesh-gatt/node.h"
#include "tools/mesh-gatt/gatt.h"
#include "tools/mesh-gatt/crypto.h"
#include "tools/mesh-gatt/mesh-net.h"
#include "tools/mesh-gatt/util.h"
#include "tools/mesh-gatt/prov.h"
#include "tools/mesh-gatt/net.h"

/* Provisioning Security Levels */
#define MESH_PROV_SEC_HIGH	2
#define MESH_PROV_SEC_MED	1
#define MESH_PROV_SEC_LOW	0


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

#define PROV_NO_OOB	0
#define PROV_STATIC_OOB	1
#define PROV_OUTPUT_OOB	2
#define PROV_INPUT_OOB	3

#define PROV_ERR_INVALID_PDU		0x01
#define PROV_ERR_INVALID_FORMAT		0x02
#define PROV_ERR_UNEXPECTED_PDU		0x03
#define PROV_ERR_CONFIRM_FAILED		0x04
#define PROV_ERR_INSUF_RESOURCE		0x05
#define PROV_ERR_DECRYPT_FAILED		0x06
#define PROV_ERR_UNEXPECTED_ERR		0x07
#define PROV_ERR_CANT_ASSIGN_ADDR	0x08

/* For Deployment, Security levels below HIGH are *not* recomended */
static uint8_t prov_sec_level = MESH_PROV_SEC_MED;

/* Expected Provisioning PDU sizes */
static const uint16_t expected_pdu_size[] = {
	1 + 1,					/* PROV_INVITE */
	1 + 1 + 2 + 1 + 1 + 1 + 2 + 1 + 2,	/* PROV_CAPS */
	1 + 1 + 1 + 1 + 1 + 1,			/* PROV_START */
	1 + 64,					/* PROV_PUB_KEY */
	1,					/* PROV_INP_CMPLT */
	1 + 16,					/* PROV_CONFIRM */
	1 + 16,					/* PROV_RANDOM */
	1 + 16 + 2 + 1 + 4 + 2 + 8,		/* PROV_DATA */
	1,					/* PROV_COMPLETE */
	1 + 1,					/* PROV_FAILED */
};

typedef struct __packed {
	uint8_t attention;
} __attribute__ ((packed))	prov_invite;

typedef struct {
	uint8_t  num_ele;
	uint16_t algorithms;
	uint8_t  pub_type;
	uint8_t  static_type;
	uint8_t  output_size;
	uint16_t output_action;
	uint8_t  input_size;
	uint16_t input_action;
} __attribute__ ((packed))	prov_caps;

typedef struct {
	uint8_t algorithm;
	uint8_t pub_key;
	uint8_t auth_method;
	uint8_t auth_action;
	uint8_t auth_size;
} __attribute__ ((packed))	prov_start;

typedef struct {
	prov_invite	invite;
	prov_caps	caps;
	prov_start	start;
	uint8_t prv_pub_key[64];
	uint8_t dev_pub_key[64];
} __attribute__ ((packed))	conf_input;

struct prov_data {
	GDBusProxy		*prov_in;
	provision_done_cb	prov_done;
	void			*user_data;
	uint16_t		net_idx;
	uint16_t		new_addr;
	uint8_t			state;
	uint8_t			eph_priv_key[32];
	uint8_t			ecdh_secret[32];
	conf_input		conf_in;
	uint8_t			rand_auth[32];
	uint8_t			salt[16];
	uint8_t			conf_key[16];
	uint8_t			mesh_conf[16];
	uint8_t			dev_key[16];
};

static uint8_t u16_highest_bit(uint16_t mask)
{
	uint8_t cnt = 0;

	if (!mask) return 0xff;

	while (mask & 0xfffe) {
		cnt++;
		mask >>= 1;
	}

	return cnt;
}

bool prov_open(struct mesh_node *node, GDBusProxy *prov_in, uint16_t net_idx,
		provision_done_cb cb, void *user_data)
{
	uint8_t invite[] = { PROXY_PROVISIONING_PDU, PROV_INVITE, 0x10 };
	struct prov_data *prov = node_get_prov(node);

	if (prov) return false;

	prov = g_new0(struct prov_data, 1);
	prov->prov_in = prov_in;
	prov->net_idx = net_idx;
	prov->prov_done = cb;
	prov->user_data = user_data;
	node_set_prov(node, prov);
	prov->conf_in.invite.attention = invite[2];
	prov->state = PROV_INVITE;

	bt_shell_printf("Open-Node: %p\n", node);
	bt_shell_printf("Open-Prov: %p\n", prov);
	bt_shell_printf("Open-Prov: proxy %p\n", prov_in);

	return mesh_gatt_write(prov_in, invite, sizeof(invite), NULL, node);
}

static bool prov_send_prov_data(void *node)
{
	struct prov_data *prov = node_get_prov(node);
	uint8_t out[35] = { PROXY_PROVISIONING_PDU, PROV_DATA };
	uint8_t key[16];
	uint8_t nonce[13];
	uint64_t mic;

	if (prov == NULL) return false;

	mesh_crypto_session_key(prov->ecdh_secret, prov->salt, key);
	mesh_crypto_nonce(prov->ecdh_secret, prov->salt, nonce);
	mesh_crypto_device_key(prov->ecdh_secret, prov->salt, prov->dev_key);

	print_byte_array("S-Key\t", key, sizeof(key));
	print_byte_array("S-Nonce\t", nonce, sizeof(nonce));
	print_byte_array("DevKey\t", prov->dev_key, sizeof(prov->dev_key));

	if (!net_get_key(prov->net_idx, out + 2))
		return false;

	put_be16(prov->net_idx, out + 2 + 16);
	net_get_flags(prov->net_idx, out + 2 + 16 + 2);
	put_be32(net_get_iv_index(NULL), out + 2 + 16 + 2 + 1);
	put_be16(prov->new_addr, out + 2 + 16 + 2 + 1 + 4);

	print_byte_array("Data\t", out + 2, 16 + 2 + 1 + 4 + 2);

	mesh_crypto_aes_ccm_encrypt(nonce, key,
					NULL, 0,
					out + 2,
					sizeof(out) - 2 - sizeof(mic),
					out + 2,
					&mic, sizeof(mic));

	print_byte_array("DataEncrypted + mic\t", out + 2, sizeof(out) - 2);

	prov->state = PROV_DATA;
	return mesh_gatt_write(prov->prov_in, out, sizeof(out), NULL, node);
}

static bool prov_send_confirm(void *node)
{
	struct prov_data *prov = node_get_prov(node);
	uint8_t out[18] = { PROXY_PROVISIONING_PDU, PROV_CONFIRM };

	if (prov == NULL) return false;

	mesh_get_random_bytes(prov->rand_auth, 16);

	mesh_crypto_aes_cmac(prov->conf_key, prov->rand_auth,
				sizeof(prov->rand_auth), out + 2);

	prov->state = PROV_CONFIRM;
	return mesh_gatt_write(prov->prov_in, out, sizeof(out), NULL, node);
}

static void prov_out_oob_done(oob_type_t type, void *buf, uint16_t len,
		void *node)
{
	struct prov_data *prov = node_get_prov(node);

	if (prov == NULL) return;

	switch (type) {
		default:
		case NONE:
		case OUTPUT:
			prov_complete(node, PROV_ERR_INVALID_PDU);
			return;

		case ASCII:
		case HEXADECIMAL:
			if (len > 16)
				prov_complete(node, PROV_ERR_INVALID_PDU);

			memcpy(prov->rand_auth + 16, buf, len);
			break;

		case DECIMAL:
			if (len != 4)
				prov_complete(node, PROV_ERR_INVALID_PDU);

			memcpy(prov->rand_auth +
					sizeof(prov->rand_auth) -
					sizeof(uint32_t),
					buf, len);
			break;
	}

	prov_send_confirm(node);
}

static uint32_t power_ten(uint8_t power)
{
	uint32_t ret = 1;

	while (power--)
		ret *= 10;

	return ret;
}

char *in_action[3] = {
	"Push",
	"Twist",
	"Enter"
};

static void prov_calc_ecdh(DBusMessage *message, void *node)
{
	struct prov_data *prov = node_get_prov(node);
	uint8_t action = prov->conf_in.start.auth_action;
	uint8_t size = prov->conf_in.start.auth_size;
	char in_oob_display[100];
	uint8_t *tmp = (void *) in_oob_display;
	uint32_t in_oob;

	if (prov == NULL) return;

	/* Convert to Mesh byte order */
	memcpy(tmp, prov->conf_in.dev_pub_key, 64);
	swap_u256_bytes(tmp);
	swap_u256_bytes(tmp + 32);

	ecdh_shared_secret(tmp, prov->eph_priv_key, prov->ecdh_secret);

	/* Convert to Mesh byte order */
	swap_u256_bytes(prov->ecdh_secret);

	mesh_crypto_s1(&prov->conf_in,
			sizeof(prov->conf_in), prov->salt);

	mesh_crypto_prov_conf_key(prov->ecdh_secret,
			prov->salt, prov->conf_key);

	switch (prov->conf_in.start.auth_method) {
		default:
			prov_complete(node, PROV_ERR_INVALID_PDU);
			break;

		case 0: /* No OOB */
			prov_send_confirm(node);
			break;

		case 1: /* Static OOB */
			agent_input_request(HEXADECIMAL,
					16, NULL,
					prov_out_oob_done, node);
			break;

		case 2: /* Output OOB */
			if (action <= 3)
				agent_input_request(DECIMAL,
						size, NULL,
						prov_out_oob_done, node);
			else
				agent_input_request(ASCII,
						size, NULL,
						prov_out_oob_done, node);
			break;

		case 3: /* Input OOB */

			if (action <= 2) {
				mesh_get_random_bytes(&in_oob, sizeof(in_oob));
				in_oob %= power_ten(size);
				sprintf(in_oob_display, "%s %d on device\n",
					in_action[action], in_oob);
				put_be32(in_oob,
						prov->rand_auth +
						sizeof(prov->rand_auth) -
						sizeof(uint32_t));
			} else {
				uint8_t in_ascii[9];
				int i = size;

				mesh_get_random_bytes(in_ascii, i);

				while (i--) {
					in_ascii[i] =
						in_ascii[i] % ((26 * 2) + 10);
					if (in_ascii[i] >= 10 + 26)
						in_ascii[i] += 'a' - (10 + 26);
					else if (in_ascii[i] >= 10)
						in_ascii[i] += 'A' - 10;
					else
						in_ascii[i] += '0';
				}
				in_ascii[size] = '\0';
				memcpy(prov->rand_auth + 16, in_ascii, size);
				sprintf(in_oob_display,
						"Enter %s on device\n",
						in_ascii);
			}
			bt_shell_printf("Agent String: %s\n", in_oob_display);
			agent_output_request(in_oob_display);
			break;
	}
}

static void prov_send_pub_key(struct mesh_node *node)
{
	struct prov_data *prov = node_get_prov(node);
	uint8_t out[66] = { PROXY_PROVISIONING_PDU, PROV_PUB_KEY };
	GDBusReturnFunction cb = NULL;

	if (prov == NULL) return;

	if (prov->conf_in.start.pub_key)
		cb = prov_calc_ecdh;

	memcpy(out + 2, prov->conf_in.prv_pub_key, 64);
	prov->state = PROV_PUB_KEY;
	mesh_gatt_write(prov->prov_in, out, 66, cb, node);
}

static void prov_oob_pub_key(oob_type_t type, void *buf, uint16_t len,
		void *node)
{
	struct prov_data *prov = node_get_prov(node);

	if (prov == NULL) return;

	memcpy(prov->conf_in.dev_pub_key, buf, 64);
	prov_send_pub_key(node);
}

static void prov_start_cmplt(DBusMessage *message, void *node)
{
	struct prov_data *prov = node_get_prov(node);

	if (prov == NULL) return;

	if (prov->conf_in.start.pub_key)
		agent_input_request(HEXADECIMAL, 64, NULL, prov_oob_pub_key,
									node);
	else
		prov_send_pub_key(node);
}

bool prov_data_ready(struct mesh_node *node, uint8_t *buf, uint8_t len)
{
	struct prov_data *prov = node_get_prov(node);
	uint8_t sec_level = MESH_PROV_SEC_HIGH;
	uint8_t out[35] = { PROXY_PROVISIONING_PDU };

	if (prov == NULL || len < 2) return false;

	buf++;
	len--;

	bt_shell_printf("Got provisioning data (%d bytes)\n", len);

	if (buf[0] > PROV_FAILED || expected_pdu_size[buf[0]] != len)
		return prov_complete(node, PROV_ERR_INVALID_PDU);

	print_byte_array("\t", buf, len);

	if (buf[0] == PROV_FAILED)
		return prov_complete(node, buf[1]);

	/* Check provisioning state */
	switch (prov->state) {
		default:
			return prov_complete(node, PROV_ERR_INVALID_PDU);

		case PROV_INVITE:

			if (buf[0] != PROV_CAPS)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			/* Normalize to beginning of packed Param struct */
			buf++;
			len--;

			/* Save Capability values */
			memcpy(&prov->conf_in.caps, buf, len);

			sec_level = prov_get_sec_level();

			if (sec_level == MESH_PROV_SEC_HIGH) {

				/* Enforce High Security */
				if (prov->conf_in.caps.pub_type != 1 &&
					prov->conf_in.caps.static_type != 1)
					return prov_complete(node,
							PROV_ERR_INVALID_PDU);

			} else if (sec_level == MESH_PROV_SEC_MED) {

				/* Enforce Medium Security */
				if (prov->conf_in.caps.pub_type != 1 &&
					prov->conf_in.caps.static_type != 1 &&
					prov->conf_in.caps.input_size == 0 &&
					prov->conf_in.caps.output_size == 0)
					return prov_complete(node,
							PROV_ERR_INVALID_PDU);

			}

			/* Num Elements cannot be Zero */
			if (prov->conf_in.caps.num_ele == 0)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			/* All nodes must support Algorithm 0x0001 */
			if (!(get_be16(buf + 1) & 0x0001))
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			/* Pub Key and Static type may not be > 1 */
			if (prov->conf_in.caps.pub_type > 0x01 ||
					prov->conf_in.caps.static_type > 0x01)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			prov->new_addr =
				net_obtain_address(prov->conf_in.caps.num_ele);

			if (!prov->new_addr)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			out[1] = PROV_START;
			prov->conf_in.start.algorithm = 0;
			prov->conf_in.start.pub_key =
				prov->conf_in.caps.pub_type;

			/* Compose START based on most secure values */
			if (prov->conf_in.caps.static_type) {

				prov->conf_in.start.auth_method =
					PROV_STATIC_OOB;

			} else if (prov->conf_in.caps.output_size >
					prov->conf_in.caps.input_size) {

				prov->conf_in.start.auth_method =
					PROV_OUTPUT_OOB;
				prov->conf_in.start.auth_action =
					u16_highest_bit(get_be16(buf + 6));
				prov->conf_in.start.auth_size =
					prov->conf_in.caps.output_size;

			} else if (prov->conf_in.caps.input_size > 0) {

				prov->conf_in.start.auth_method =
					PROV_INPUT_OOB;
				prov->conf_in.start.auth_action =
					u16_highest_bit(get_be16(buf + 9));
				prov->conf_in.start.auth_size =
					prov->conf_in.caps.input_size;
			}

			/* Range Check START values */
			if (prov->conf_in.start.auth_size > 8)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			prov->state = PROV_START;

			memcpy(out + 2, &prov->conf_in.start, 5);

			ecc_make_key(prov->conf_in.prv_pub_key,
					prov->eph_priv_key);

			/* Swap public key to share into Mesh byte ordering */
			swap_u256_bytes(prov->conf_in.prv_pub_key);
			swap_u256_bytes(prov->conf_in.prv_pub_key + 32);

			return mesh_gatt_write(prov->prov_in, out, 7,
					prov_start_cmplt, node);


		case PROV_PUB_KEY:
			if (buf[0] == PROV_PUB_KEY &&
					!prov->conf_in.start.pub_key) {

				memcpy(prov->conf_in.dev_pub_key, buf + 1, 64);
				prov_calc_ecdh(NULL, node);
				return true;

			} else if (buf[0] == PROV_INP_CMPLT) {
				agent_output_request_cancel();
				return prov_send_confirm(node);
			} else
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

		case PROV_CONFIRM:
			if (buf[0] != PROV_CONFIRM)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			memcpy(prov->mesh_conf, buf + 1, 16);

			out[1] = PROV_RANDOM;
			memcpy(out + 2, prov->rand_auth, 16);

			prov->state = PROV_RANDOM;
			return mesh_gatt_write(prov->prov_in, out, 18,
					NULL, node);

		case PROV_RANDOM:
			if (buf[0] != PROV_RANDOM)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			/* Calculate New Salt while we still have
			 * both random values */
			mesh_crypto_prov_prov_salt(prov->salt,
							prov->rand_auth,
							buf + 1,
							prov->salt);

			/* Calculate meshs Conf Value */
			memcpy(prov->rand_auth, buf + 1, 16);
			mesh_crypto_aes_cmac(prov->conf_key, prov->rand_auth,
				sizeof(prov->rand_auth), out + 1);

			/* Validate Mesh confirmation */
			if (memcmp(out + 1, prov->mesh_conf, 16) != 0)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			bt_shell_printf("Confirmation Validated\n");

			prov_send_prov_data(node);

			return true;

		case PROV_DATA:
			if (buf[0] != PROV_COMPLETE)
				return prov_complete(node,
						PROV_ERR_INVALID_PDU);

			return prov_complete(node, 0);
	}



	/* Compose appropriate reply for the prov state message */
	/* Send reply via mesh_gatt_write() */
	/* If done, call prov_done calllback and free prov housekeeping data */
	bt_shell_printf("Got provisioning data (%d bytes)\n", len);
	print_byte_array("\t", buf, len);

	return true;
}

bool prov_complete(struct mesh_node *node, uint8_t status)
{
	struct prov_data *prov = node_get_prov(node);
	void *user_data;
	provision_done_cb cb;

	if (prov == NULL) return false;

	if (status && prov->new_addr && prov->conf_in.caps.num_ele) {
		net_release_address(prov->new_addr, prov->conf_in.caps.num_ele);
	}

	if (!status) {
		node_set_num_elements(node, prov->conf_in.caps.num_ele);
		node_set_primary(node, prov->new_addr);
		node_set_device_key(node, prov->dev_key);
		node_net_key_add(node, prov->net_idx);
	}

	user_data = prov->user_data;
	cb = prov->prov_done;
	g_free(prov);
	node_set_prov(node, NULL);
	if (cb) cb(user_data, status);

	return true;
}

bool prov_set_sec_level(uint8_t level)
{
	if (level > MESH_PROV_SEC_HIGH)
		return false;

	prov_sec_level = level;

	return true;
}

uint8_t prov_get_sec_level(void)
{
	return prov_sec_level;
}
