// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2018-2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ell/ell.h>

#include "src/shared/ecc.h"

#include "mesh/mesh-defs.h"
#include "mesh/util.h"
#include "mesh/crypto.h"
#include "mesh/net.h"
#include "mesh/node.h"
#include "mesh/keyring.h"
#include "mesh/prov.h"
#include "mesh/provision.h"
#include "mesh/pb-adv.h"
#include "mesh/mesh.h"
#include "mesh/agent.h"
#include "mesh/error.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/* Quick size sanity check */
static const uint16_t expected_pdu_size[] = {
	2,	/* PROV_INVITE */
	12,	/* PROV_CAPS */
	6,	/* PROV_START */
	65,	/* PROV_PUB_KEY */
	1,	/* PROV_INP_CMPLT */
	17,	/* PROV_CONFIRM */
	17,	/* PROV_RANDOM */
	34,	/* PROV_DATA */
	1,	/* PROV_COMPLETE */
	2,	/* PROV_FAILED */
};

#define BEACON_TYPE_UNPROVISIONED		0x00

static const uint8_t pkt_filter = MESH_AD_TYPE_PROVISION;

enum int_state {
	INT_PROV_IDLE = 0,
	INT_PROV_INVITE_SENT,
	INT_PROV_INVITE_ACKED,
	INT_PROV_START_SENT,
	INT_PROV_START_ACKED,
	INT_PROV_KEY_SENT,
	INT_PROV_KEY_ACKED,
	INT_PROV_CONF_SENT,
	INT_PROV_CONF_ACKED,
	INT_PROV_RAND_SENT,
	INT_PROV_RAND_ACKED,
	INT_PROV_DATA_SENT,
	INT_PROV_DATA_ACKED,
};

#define MAT_REMOTE_PUBLIC	0x01
#define MAT_LOCAL_PRIVATE	0x02
#define MAT_RAND_AUTH		0x04
#define MAT_SECRET	(MAT_REMOTE_PUBLIC | MAT_LOCAL_PRIVATE)

struct mesh_prov_initiator {
	mesh_prov_initiator_start_func_t start_cb;
	mesh_prov_initiator_complete_func_t complete_cb;
	mesh_prov_initiator_data_req_func_t data_req_cb;
	prov_trans_tx_t trans_tx;
	struct mesh_agent *agent;
	void *caller_data;
	void *trans_data;
	struct mesh_node *node;
	struct l_timeout *timeout;
	uint32_t to_secs;
	enum int_state	state;
	enum trans_type transport;
	uint16_t net_idx;
	uint16_t unicast;
	uint8_t material;
	uint8_t expected;
	int8_t previous;
	struct conf_input conf_inputs;
	uint8_t calc_key[16];
	uint8_t salt[16];
	uint8_t confirm[16];
	uint8_t s_key[16];
	uint8_t s_nonce[13];
	uint8_t private_key[32];
	uint8_t secret[32];
	uint8_t rand_auth_workspace[48];
	uint8_t uuid[16];
};

static struct mesh_prov_initiator *prov = NULL;

static void initiator_free(void)
{
	if (prov)
		l_timeout_remove(prov->timeout);

	mesh_send_cancel(&pkt_filter, sizeof(pkt_filter));

	pb_adv_unreg(prov);

	l_free(prov);
	prov = NULL;
}

static void int_prov_close(void *user_data, uint8_t reason)
{
	struct mesh_prov_initiator *prov = user_data;
	struct mesh_prov_node_info info;

	if (reason != PROV_ERR_SUCCESS) {
		prov->complete_cb(prov->caller_data, reason, NULL);
		initiator_free();
		return;
	}

	memcpy(info.device_key, prov->calc_key, 16);
	info.net_index = prov->net_idx;
	info.unicast = prov->unicast;
	info.num_ele = prov->conf_inputs.caps.num_ele;

	prov->complete_cb(prov->caller_data, PROV_ERR_SUCCESS, &info);
	initiator_free();
}

static void swap_u256_bytes(uint8_t *u256)
{
	int i;

	/* End-to-End byte reflection of 32 octet buffer */
	for (i = 0; i < 16; i++) {
		u256[i] ^= u256[31 - i];
		u256[31 - i] ^= u256[i];
		u256[i] ^= u256[31 - i];
	}
}

static void int_prov_open(void *user_data, prov_trans_tx_t trans_tx,
				void *trans_data, uint8_t transport)
{
	struct mesh_prov_initiator *rx_prov = user_data;
	struct prov_invite_msg msg = { PROV_INVITE, { 30 }};

	/* Only one provisioning session may be open at a time */
	if (rx_prov != prov)
		return;

	/* Only one provisioning session may be open at a time */
	if (prov->trans_tx && prov->trans_tx != trans_tx &&
					prov->transport != transport)
		return;

	/* We only care here if transport does *not* match */
	if (transport != prov->transport)
		return;

	/* Always use an ephemeral key when Initiator */
	ecc_make_key(prov->conf_inputs.prv_pub_key, prov->private_key);
	swap_u256_bytes(prov->conf_inputs.prv_pub_key);
	swap_u256_bytes(prov->conf_inputs.prv_pub_key + 32);
	prov->material |= MAT_LOCAL_PRIVATE;

	prov->trans_tx = trans_tx;
	prov->trans_data = trans_data;
	prov->state = INT_PROV_INVITE_SENT;
	prov->expected = PROV_CAPS;

	prov->conf_inputs.invite.attention = msg.invite.attention;
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
	return;
}

static bool prov_calc_secret(const uint8_t *pub, const uint8_t *priv,
							uint8_t *secret)
{
	uint8_t tmp[64];

	/* Convert to ECC byte order */
	memcpy(tmp, pub, 64);
	swap_u256_bytes(tmp);
	swap_u256_bytes(tmp + 32);

	if (!ecdh_shared_secret(tmp, priv, secret))
		return false;

	/* Convert to Mesh byte order */
	swap_u256_bytes(secret);
	return true;
}

static bool int_credentials(struct mesh_prov_initiator *prov)
{
	if (!prov_calc_secret(prov->conf_inputs.dev_pub_key,
				prov->private_key, prov->secret))
		return false;

	if (!mesh_crypto_s1(&prov->conf_inputs,
				sizeof(prov->conf_inputs), prov->salt))
		return false;

	if (!mesh_crypto_prov_conf_key(prov->secret, prov->salt,
				prov->calc_key))
		return false;

	l_getrandom(prov->rand_auth_workspace, 16);

	print_packet("PublicKeyProv", prov->conf_inputs.prv_pub_key, 64);
	print_packet("PublicKeyDev", prov->conf_inputs.dev_pub_key, 64);
	print_packet("PrivateKeyLocal", prov->private_key, 32);
	print_packet("ConfirmationInputs", &prov->conf_inputs,
						sizeof(prov->conf_inputs));
	print_packet("ECDHSecret", prov->secret, 32);
	print_packet("LocalRandom", prov->rand_auth_workspace, 16);
	print_packet("ConfirmationSalt", prov->salt, 16);
	print_packet("ConfirmationKey", prov->calc_key, 16);

	return true;
}

static uint8_t u16_high_bit(uint16_t mask)
{
	uint8_t cnt = 0;

	if (!mask)
		return 0xff;

	while (mask & 0xfffe) {
		cnt++;
		mask >>= 1;
	}

	return cnt;
}

static uint32_t digit_mod(uint8_t power)
{
	uint32_t ret = 1;

	while (power--)
		ret *= 10;

	return ret;
}

static void calc_local_material(const uint8_t *random)
{
	/* Calculate SessionKey while the data is fresh */
	mesh_crypto_prov_prov_salt(prov->salt,
			prov->rand_auth_workspace, random,
			prov->salt);
	mesh_crypto_session_key(prov->secret, prov->salt,
			prov->s_key);
	mesh_crypto_nonce(prov->secret, prov->salt, prov->s_nonce);

	print_packet("SessionKey", prov->s_key, sizeof(prov->s_key));
	print_packet("Nonce", prov->s_nonce, sizeof(prov->s_nonce));
}

static void send_confirm(struct mesh_prov_initiator *prov)
{
	struct prov_conf_msg msg;

	msg.opcode = PROV_CONFIRM;
	mesh_crypto_aes_cmac(prov->calc_key, prov->rand_auth_workspace,
			32, msg.conf);
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
	prov->state = INT_PROV_CONF_SENT;
	prov->expected = PROV_CONFIRM;
}

static void number_cb(void *user_data, int err, uint32_t number)
{
	struct mesh_prov_initiator *rx_prov = user_data;
	struct prov_fail_msg msg;

	if (prov != rx_prov)
		return;

	if (err) {
		msg.opcode = PROV_FAILED;
		msg.reason = PROV_ERR_UNEXPECTED_ERR;
		prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		return;
	}

	/* Save two copies, to generate two confirmation values */
	l_put_be32(number, prov->rand_auth_workspace + 28);
	l_put_be32(number, prov->rand_auth_workspace + 44);
	prov->material |= MAT_RAND_AUTH;
	send_confirm(prov);
}

static void static_cb(void *user_data, int err, uint8_t *key, uint32_t len)
{
	struct mesh_prov_initiator *rx_prov = user_data;
	struct prov_fail_msg msg;

	if (prov != rx_prov)
		return;

	if (err || !key || len != 16) {
		msg.opcode = PROV_FAILED;
		msg.reason = PROV_ERR_UNEXPECTED_ERR;
		prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		return;
	}

	memcpy(prov->rand_auth_workspace + 16, key, 16);
	memcpy(prov->rand_auth_workspace + 32, key, 16);
	prov->material |= MAT_RAND_AUTH;
	send_confirm(prov);
}

static void send_pub_key(struct mesh_prov_initiator *prov)
{
	struct prov_pub_key_msg msg;

	msg.opcode = PROV_PUB_KEY;
	memcpy(msg.pub_key, prov->conf_inputs.prv_pub_key, 64);
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
	prov->state = INT_PROV_KEY_SENT;
}

static void pub_key_cb(void *user_data, int err, uint8_t *key, uint32_t len)
{
	struct mesh_prov_initiator *rx_prov = user_data;
	struct prov_fail_msg msg;
	uint8_t fail_code[2];

	if (prov != rx_prov)
		return;

	if (err || !key || len != 64) {
		msg.opcode = PROV_FAILED;
		msg.reason = PROV_ERR_UNEXPECTED_ERR;
		prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		return;
	}

	memcpy(prov->conf_inputs.dev_pub_key, key, 64);
	prov->material |= MAT_REMOTE_PUBLIC;

	if ((prov->material & MAT_SECRET) == MAT_SECRET) {
		if (!int_credentials(prov)) {
			fail_code[0] = PROV_FAILED;
			fail_code[1] = PROV_ERR_UNEXPECTED_ERR;
			prov->trans_tx(prov->trans_data, fail_code, 2);
			int_prov_close(prov, fail_code[1]);
			return;
		}
	}

	send_pub_key(prov);
}

static void send_random(struct mesh_prov_initiator *prov)
{
	struct prov_rand_msg msg;

	msg.opcode = PROV_RANDOM;
	memcpy(msg.rand, prov->rand_auth_workspace, sizeof(msg.rand));
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
	prov->state = INT_PROV_RAND_SENT;
	prov->expected = PROV_RANDOM;
}

void initiator_prov_data(uint16_t net_idx, uint16_t primary, void *caller_data)
{
	struct prov_data_msg prov_data;
	struct prov_fail_msg prov_fail;
	struct keyring_net_key key;
	struct mesh_net *net;
	uint32_t iv_index;
	uint8_t snb_flags;

	if (!prov || caller_data != prov->caller_data)
		return;

	if (prov->state != INT_PROV_RAND_ACKED)
		return;

	net = node_get_net(prov->node);
	prov->expected = PROV_COMPLETE;

	/* Calculate remote device key */
	mesh_crypto_device_key(prov->secret,
			prov->salt,
			prov->calc_key);

	print_packet("DevKey", prov->calc_key, 16);

	/* Fill Prov Data Structure */
	if (!keyring_get_net_key(prov->node, net_idx, &key)) {
		prov_fail.reason = PROV_ERR_UNEXPECTED_ERR;
		goto failure;
	}

	prov->unicast = primary;
	prov->net_idx = net_idx;
	mesh_net_get_snb_state(net, &snb_flags, &iv_index);

	prov_data.opcode = PROV_DATA;

	if (key.phase == KEY_REFRESH_PHASE_TWO) {
		memcpy(&prov_data.data.net_key, key.new_key, 16);
		snb_flags |= PROV_FLAG_KR;
	} else
		memcpy(&prov_data.data.net_key, key.old_key, 16);

	l_put_be16(net_idx, &prov_data.data.net_idx);
	l_put_u8(snb_flags, &prov_data.data.flags);
	l_put_be32(iv_index, &prov_data.data.iv_index);
	l_put_be16(primary, &prov_data.data.primary);

	print_packet("ProvData", &prov_data.data, sizeof(prov_data.data));
	/* Encrypt Prov Data */
	mesh_crypto_aes_ccm_encrypt(prov->s_nonce, prov->s_key,
			NULL, 0,
			&prov_data.data,
			sizeof(prov_data.data),
			&prov_data.data,
			NULL, sizeof(prov_data.mic));
	print_packet("EncdData", &prov_data.data, sizeof(prov_data) - 1);
	prov->trans_tx(prov->trans_data, &prov_data, sizeof(prov_data));
	prov->state = INT_PROV_DATA_SENT;
	return;

failure:
	l_debug("Failing... %d", prov_fail.reason);
	prov_fail.opcode = PROV_FAILED;
	prov->trans_tx(prov->trans_data, &prov_fail, sizeof(prov_fail));
	/* TODO: Call Complete Callback (Fail)*/
}

static void get_random_key(struct mesh_prov_initiator *prov, uint8_t action,
								uint8_t size)
{
	uint32_t oob_key;
	int i;

	if (action >= PROV_ACTION_IN_ALPHA) {
		uint8_t alpha;
		char tmp[17];

		memset(tmp, 0, sizeof(tmp));

		if (size > 16)
			size = 16;

		/* Create random alphanumeric string made of 0-9, a-z, A-Z */
		for (i = 0; i < size; i++) {
			l_getrandom(&alpha, sizeof(alpha));
			alpha %= (10 + 26 + 26);

			if (alpha < 10)
				alpha += '0';
			else if (alpha < 10 + 26)
				alpha += 'a' - 10;
			else
				alpha += 'A' - 10 - 26;

			tmp[i] = (char) alpha;
		}
		memcpy(prov->rand_auth_workspace + 16, tmp, size);
		memcpy(prov->rand_auth_workspace + 32, tmp, size);
		return;
	}

	l_getrandom(&oob_key, sizeof(oob_key));

	if (action <= PROV_ACTION_TWIST)
		oob_key %= size;
	else
		oob_key %= digit_mod(size);

	if (!oob_key)
		oob_key = size;

	/* Save two copies, for two confirmation values */
	l_put_be32(oob_key, prov->rand_auth_workspace + 28);
	l_put_be32(oob_key, prov->rand_auth_workspace + 44);
}

static void int_prov_auth(void)
{
	uint8_t fail_code[2];
	uint32_t oob_key;

	prov->state = INT_PROV_KEY_ACKED;

	l_debug("auth_method: %d", prov->conf_inputs.start.auth_method);
	memset(prov->rand_auth_workspace + 16, 0, 32);

	switch (prov->conf_inputs.start.auth_method) {
	default:
	case 0:
		/* Auth Type 3c - No OOB */
		prov->material |= MAT_RAND_AUTH;
		break;
	case 1:
		/* Auth Type 3c - Static OOB */
		/* Prompt Agent for Static OOB */
		fail_code[1] = mesh_agent_request_static(prov->agent,
				static_cb, prov);

		if (fail_code[1])
			goto failure;

		break;
	case 2:
		/* Auth Type 3a - Output OOB */
		/* Prompt Agent for Output OOB */
		if (prov->conf_inputs.start.auth_action ==
						PROV_ACTION_OUT_ALPHA) {
			fail_code[1] = mesh_agent_prompt_alpha(
				prov->agent, true,
				static_cb, prov);
		} else {
			fail_code[1] = mesh_agent_prompt_number(
				prov->agent, true,
				prov->conf_inputs.start.auth_action,
				number_cb, prov);
		}

		if (fail_code[1])
			goto failure;

		break;

	case 3:
		/* Auth Type 3b - input OOB */
		get_random_key(prov,
				prov->conf_inputs.start.auth_action,
				prov->conf_inputs.start.auth_size);
		oob_key = l_get_be32(prov->rand_auth_workspace + 28);

		/* Ask Agent to Display random key */
		if (prov->conf_inputs.start.auth_action ==
						PROV_ACTION_IN_ALPHA) {

			fail_code[1] = mesh_agent_display_string(
				prov->agent,
				(char *) prov->rand_auth_workspace + 16,
				NULL, prov);
		} else {
			fail_code[1] = mesh_agent_display_number(
				prov->agent, true,
				prov->conf_inputs.start.auth_action,
				oob_key, NULL, prov);
		}

		if (fail_code[1])
			goto failure;

		break;

	}

	if (prov->material & MAT_RAND_AUTH)
		send_confirm(prov);

	return;

failure:
	l_debug("Failing... %d", fail_code[1]);
	fail_code[0] = PROV_FAILED;
	prov->trans_tx(prov->trans_data, fail_code, 2);
	int_prov_close(prov, fail_code[1]);
}

static void int_prov_start_auth(const struct mesh_agent_prov_caps *prov_caps,
				const struct mesh_net_prov_caps *dev_caps,
				struct prov_start *start)
{
	uint8_t pub_type = prov_caps->pub_type & dev_caps->pub_type;
	uint8_t static_type = prov_caps->static_type & dev_caps->static_type;
	uint16_t output_action = prov_caps->output_action &
					L_BE16_TO_CPU(dev_caps->output_action);
	uint8_t output_size = MIN(prov_caps->output_size,
							dev_caps->output_size);
	uint16_t input_action = prov_caps->input_action &
					L_BE16_TO_CPU(dev_caps->input_action);
	uint8_t input_size = MIN(prov_caps->input_size, dev_caps->input_size);

	if (pub_type)
		start->pub_key = 0x01;

	/* Parse OOB Options, prefer static, then out, then in */
	if (static_type) {
		start->auth_method = 0x01;
		return;
	}

	if (output_size && output_action) {
		start->auth_method = 0x02;
		start->auth_action = u16_high_bit(output_action);
		start->auth_size = MIN(output_size, 8);
		return;
	}

	if (input_size && input_action) {
		start->auth_method = 0x03;
		start->auth_action = u16_high_bit(input_action);
		start->auth_size = MIN(input_size, 8);
		return;
	}
}

static void int_prov_rx(void *user_data, const uint8_t *data, uint16_t len)
{
	struct mesh_prov_initiator *rx_prov = user_data;
	uint8_t *out;
	uint8_t type = *data++;
	uint8_t fail_code[2];

	if (rx_prov != prov || !prov->trans_tx)
		return;

	l_debug("Provisioning packet received type: %2.2x (%u octets)",
								type, len);

	if (type == prov->previous) {
		l_error("Ignore repeated %2.2x packet", type);
		return;
	} else if (type > prov->expected || type < prov->previous) {
		l_error("Expected %2.2x, Got:%2.2x", prov->expected, type);
		fail_code[1] = PROV_ERR_UNEXPECTED_PDU;
		goto failure;
	}

	if (type >= L_ARRAY_SIZE(expected_pdu_size) ||
					len != expected_pdu_size[type]) {
		l_error("Expected PDU size %d, Got %d (type: %2.2x)",
			len, expected_pdu_size[type], type);
		fail_code[1] = PROV_ERR_INVALID_FORMAT;
		goto failure;
	}

	switch (type) {
	case PROV_CAPS: /* Capabilities */
		prov->state = INT_PROV_INVITE_ACKED;
		memcpy(&prov->conf_inputs.caps, data,
					sizeof(prov->conf_inputs.caps));

		l_debug("Got Num Ele %d", data[0]);
		l_debug("Got alg %4.4x", l_get_be16(data + 1));
		l_debug("Got pub_type %d", data[3]);
		l_debug("Got static_type %d", data[4]);
		l_debug("Got output_size %d", data[5]);
		l_debug("Got output_action %d", l_get_be16(data + 6));
		l_debug("Got input_size %d", data[8]);
		l_debug("Got input_action %d", l_get_be16(data + 9));

		if (!(l_get_be16(data + 1) & 0x0001)) {
			l_error("Unsupported Algorithm");
			fail_code[1] = PROV_ERR_INVALID_FORMAT;
			goto failure;
		}

		/*
		 * Select auth mechanism from methods supported by both
		 * parties.
		 */
		int_prov_start_auth(mesh_agent_get_caps(prov->agent),
						&prov->conf_inputs.caps,
						&prov->conf_inputs.start);

		if (prov->conf_inputs.start.pub_key == 0x01) {
			prov->expected = PROV_CONFIRM;
			/* Prompt Agent for remote Public Key */
			mesh_agent_request_public_key(prov->agent,
							pub_key_cb, prov);
			/* Nothing else for us to do now */
		} else
			prov->expected = PROV_PUB_KEY;

		out = l_malloc(1 + sizeof(prov->conf_inputs.start));
		out[0] = PROV_START;
		memcpy(out + 1, &prov->conf_inputs.start,
					sizeof(prov->conf_inputs.start));

		prov->state = INT_PROV_START_SENT;
		prov->trans_tx(prov->trans_data, out,
					sizeof(prov->conf_inputs.start) + 1);
		l_free(out);
		break;

	case PROV_PUB_KEY: /* Public Key */
		/* If we expected Pub Key Out-Of-Band, then fail */
		if (prov->conf_inputs.start.pub_key) {
			fail_code[1] = PROV_ERR_INVALID_PDU;
			goto failure;
		}

		memcpy(prov->conf_inputs.dev_pub_key, data, 64);
		prov->material |= MAT_REMOTE_PUBLIC;
		prov->expected = PROV_CONFIRM;

		if ((prov->material & MAT_SECRET) != MAT_SECRET)
			return;

		if (!int_credentials(prov)) {
			fail_code[1] = PROV_ERR_UNEXPECTED_ERR;
			goto failure;
		}

		int_prov_auth();
		break;

	case PROV_INP_CMPLT: /* Provisioning Input Complete */
		/* TODO: Cancel Agent prompt */
		prov->material |= MAT_RAND_AUTH;
		send_confirm(prov);
		break;

	case PROV_CONFIRM: /* Confirmation */
		prov->state = INT_PROV_CONF_ACKED;
		/* RXed Device Confirmation */
		memcpy(prov->confirm, data, 16);
		print_packet("ConfirmationDevice", prov->confirm, 16);
		send_random(prov);
		break;

	case PROV_RANDOM: /* Random */
		prov->state = INT_PROV_RAND_ACKED;

		/* RXed Device Confirmation */
		calc_local_material(data);
		memcpy(prov->rand_auth_workspace + 16, data, 16);
		print_packet("RandomDevice", data, 16);

		mesh_crypto_aes_cmac(prov->calc_key,
						prov->rand_auth_workspace + 16,
						32, prov->rand_auth_workspace);

		print_packet("Dev-Conf", prov->rand_auth_workspace, 16);
		if (memcmp(prov->rand_auth_workspace, prov->confirm, 16)) {
			l_error("Provisioning Failed-Confirm compare");
			fail_code[1] = PROV_ERR_CONFIRM_FAILED;
			goto failure;
		}

		if (!prov->data_req_cb(prov->caller_data,
					prov->conf_inputs.caps.num_ele)) {
			l_error("Provisioning Failed-Data Get");
			fail_code[1] = PROV_ERR_CANT_ASSIGN_ADDR;
			goto failure;
		}
		break;

	case PROV_COMPLETE: /* Complete */
		l_debug("Provisioning Complete");
		prov->state = INT_PROV_IDLE;
		int_prov_close(prov, PROV_ERR_SUCCESS);
		break;

	case PROV_FAILED: /* Failed */
		l_error("Provisioning Failed (reason: %d)", data[0]);
		prov->state = INT_PROV_IDLE;
		int_prov_close(prov, data[0]);
		break;

	default:
		l_error("Unknown Pkt %2.2x", type);
		fail_code[1] = PROV_ERR_UNEXPECTED_PDU;
		goto failure;
	}

	if (prov)
		prov->previous = type;

	return;

failure:
	l_debug("Failing... %d", fail_code[1]);
	fail_code[0] = PROV_FAILED;
	prov->trans_tx(prov->trans_data, fail_code, 2);
	int_prov_close(prov, fail_code[1]);
}

static void int_prov_ack(void *user_data, uint8_t msg_num)
{
	struct mesh_prov_initiator *rx_prov = user_data;

	if (rx_prov != prov || !prov->trans_tx)
		return;

	switch (prov->state) {
	case INT_PROV_START_SENT:
		prov->state = INT_PROV_START_ACKED;
		if (!prov->conf_inputs.start.pub_key)
			send_pub_key(prov);
		break;

	case INT_PROV_DATA_SENT:
		prov->state = INT_PROV_DATA_ACKED;
		break;

	case INT_PROV_KEY_SENT:
		if (prov->conf_inputs.start.pub_key)
			int_prov_auth();
		break;

	case INT_PROV_IDLE:
	case INT_PROV_INVITE_SENT:
	case INT_PROV_INVITE_ACKED:
	case INT_PROV_START_ACKED:
	case INT_PROV_KEY_ACKED:
	case INT_PROV_CONF_SENT:
	case INT_PROV_CONF_ACKED:
	case INT_PROV_RAND_SENT:
	case INT_PROV_RAND_ACKED:
	case INT_PROV_DATA_ACKED:
	default:
		break;
	}
}

static void initiator_open_cb(void *user_data, int err)
{
	bool result;

	if (!prov)
		return;

	if (err != MESH_ERROR_NONE)
		goto fail;

	/* Always register for PB-ADV */
	result = pb_adv_reg(true, int_prov_open, int_prov_close, int_prov_rx,
						int_prov_ack, prov->uuid, prov);

	if (!result) {
		err = MESH_ERROR_FAILED;
		goto fail;
	}

	if (!prov)
		return;

	prov->start_cb(prov->caller_data, MESH_ERROR_NONE);
	return;
fail:
	prov->start_cb(prov->caller_data, err);
	initiator_free();
}

bool initiator_start(enum trans_type transport,
		uint8_t uuid[16],
		uint16_t max_ele,
		uint32_t timeout, /* in seconds from mesh.conf */
		struct mesh_agent *agent,
		mesh_prov_initiator_start_func_t start_cb,
		mesh_prov_initiator_data_req_func_t data_req_cb,
		mesh_prov_initiator_complete_func_t complete_cb,
		void *node, void *caller_data)
{
	/* Invoked from Add() method in mesh-api.txt, to add a
	 * remote unprovisioned device network.
	 */

	if (prov)
		return false;

	prov = l_new(struct mesh_prov_initiator, 1);
	prov->to_secs = timeout;
	prov->node = node;
	prov->agent = agent;
	prov->complete_cb = complete_cb;
	prov->start_cb = start_cb;
	prov->data_req_cb = data_req_cb;
	prov->caller_data = caller_data;
	prov->previous = -1;
	memcpy(prov->uuid, uuid, 16);

	mesh_agent_refresh(prov->agent, initiator_open_cb, prov);

	return true;
}

void initiator_cancel(void *user_data)
{
	initiator_free();
}
