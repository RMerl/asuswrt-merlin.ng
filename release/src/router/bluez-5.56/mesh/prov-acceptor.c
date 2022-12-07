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
#include "mesh/prov.h"
#include "mesh/provision.h"
#include "mesh/pb-adv.h"
#include "mesh/mesh.h"
#include "mesh/agent.h"

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
static const uint8_t bec_filter[] = {MESH_AD_TYPE_BEACON,
						BEACON_TYPE_UNPROVISIONED};

enum acp_state {
	ACP_PROV_IDLE = 0,
	ACP_PROV_CAPS_SENT,
	ACP_PROV_CAPS_ACKED,
	ACP_PROV_KEY_SENT,
	ACP_PROV_KEY_ACKED,
	ACP_PROV_INP_CMPLT_SENT,
	ACP_PROV_INP_CMPLT_ACKED,
	ACP_PROV_CONF_SENT,
	ACP_PROV_CONF_ACKED,
	ACP_PROV_RAND_SENT,
	ACP_PROV_RAND_ACKED,
	ACP_PROV_CMPLT_SENT,
	ACP_PROV_FAIL_SENT,
};

#define MAT_REMOTE_PUBLIC	0x01
#define MAT_LOCAL_PRIVATE	0x02
#define MAT_RAND_AUTH		0x04
#define MAT_SECRET	(MAT_REMOTE_PUBLIC | MAT_LOCAL_PRIVATE)

struct mesh_prov_acceptor {
	mesh_prov_acceptor_complete_func_t cmplt;
	prov_trans_tx_t trans_tx;
	void *agent;
	void *caller_data;
	void *trans_data;
	struct l_timeout *timeout;
	uint32_t to_secs;
	enum acp_state	state;
	uint8_t transport;
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
};

static struct mesh_prov_acceptor *prov = NULL;

static void acceptor_free(void)
{
	if (!prov)
		return;

	l_timeout_remove(prov->timeout);

	mesh_send_cancel(bec_filter, sizeof(bec_filter));
	mesh_send_cancel(&pkt_filter, sizeof(pkt_filter));

	pb_adv_unreg(prov);

	l_free(prov);
	prov = NULL;
}

static void acp_prov_close(void *user_data, uint8_t reason)
{
	struct mesh_prov_acceptor *rx_prov = user_data;

	if (rx_prov != prov)
		return;

	if (reason == PROV_ERR_SUCCESS)
		reason = PROV_ERR_UNEXPECTED_ERR;

	if (prov->cmplt)
		prov->cmplt(prov->caller_data, reason, NULL);

	prov->cmplt = NULL;
	acceptor_free();
}

static void prov_to(struct l_timeout *timeout, void *user_data)
{
	struct mesh_prov_acceptor *rx_prov = user_data;
	uint8_t fail_code[2] = {PROV_FAILED, PROV_ERR_UNEXPECTED_ERR};

	if (rx_prov != prov)
		return;

	l_timeout_remove(prov->timeout);
	prov->timeout = NULL;

	if (prov->cmplt && prov->trans_tx) {
		prov->cmplt(prov->caller_data, PROV_ERR_TIMEOUT, NULL);
		prov->cmplt = NULL;
		prov->trans_tx(prov->trans_data, fail_code, 2);
		prov->timeout = l_timeout_create(1, prov_to, prov, NULL);
		return;
	}

	acceptor_free();
}

static void acp_prov_open(void *user_data, prov_trans_tx_t trans_tx,
				void *trans_data, uint8_t transport)
{
	struct mesh_prov_acceptor *rx_prov = user_data;

	/* Only one provisioning session may be open at a time */
	if (rx_prov != prov)
		return;

	/* Only one provisioning session may be open at a time */
	if (prov->trans_tx && prov->trans_tx != trans_tx &&
					prov->transport != transport)
		return;

	if (transport != PB_ADV)
		return;

	prov->trans_tx = trans_tx;
	prov->transport = transport;
	prov->trans_data = trans_data;
	prov->timeout = l_timeout_create(prov->to_secs, prov_to, prov, NULL);
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

static bool acp_credentials(struct mesh_prov_acceptor *prov)
{
	if (!prov_calc_secret(prov->conf_inputs.prv_pub_key,
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

static uint32_t digit_mod(uint8_t power)
{
	uint32_t ret = 1;

	while (power--)
		ret *= 10;

	return ret;
}

static void number_cb(void *user_data, int err, uint32_t number)
{
	struct mesh_prov_acceptor *rx_prov = user_data;
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
	msg.opcode = PROV_INP_CMPLT;
	prov->trans_tx(prov->trans_data, &msg.opcode, 1);
}

static void static_cb(void *user_data, int err, uint8_t *key, uint32_t len)
{
	struct mesh_prov_acceptor *rx_prov = user_data;
	struct prov_fail_msg msg;

	if (prov != rx_prov)
		return;

	if (err || !key || len != 16) {
		msg.opcode = PROV_FAILED;
		msg.reason = PROV_ERR_UNEXPECTED_ERR;
		prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		return;
	}

	/* Save two copies, to generate two confirmation values */
	memcpy(prov->rand_auth_workspace + 16, key, 16);
	memcpy(prov->rand_auth_workspace + 32, key, 16);
	prov->material |= MAT_RAND_AUTH;

	if (prov->conf_inputs.start.auth_action == PROV_ACTION_IN_ALPHA) {
		msg.opcode = PROV_INP_CMPLT;
		prov->trans_tx(prov->trans_data, &msg.opcode, 1);
	}
}

static void priv_key_cb(void *user_data, int err, uint8_t *key, uint32_t len)
{
	struct mesh_prov_acceptor *rx_prov = user_data;
	struct prov_fail_msg msg;

	if (prov != rx_prov)
		return;

	if (err || !key || len != 32) {
		msg.opcode = PROV_FAILED;
		msg.reason = PROV_ERR_UNEXPECTED_ERR;
		prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		return;
	}

	memcpy(prov->private_key, key, 32);
	ecc_make_public_key(prov->private_key,
			prov->conf_inputs.dev_pub_key);

	/* Convert to Mesh byte order */
	swap_u256_bytes(prov->conf_inputs.dev_pub_key);
	swap_u256_bytes(prov->conf_inputs.dev_pub_key + 32);

	prov->material |= MAT_LOCAL_PRIVATE;
	if ((prov->material & MAT_SECRET) == MAT_SECRET) {
		if (!acp_credentials(prov)) {
			msg.opcode = PROV_FAILED;
			msg.reason = PROV_ERR_UNEXPECTED_ERR;
			prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
		}
	}
}

static void send_caps(struct mesh_prov_acceptor *prov)
{
	struct prov_caps_msg msg;

	msg.opcode = PROV_CAPS;
	memcpy(&msg.caps, &prov->conf_inputs.caps,
			sizeof(prov->conf_inputs.caps));

	prov->state = ACP_PROV_CAPS_SENT;
	prov->expected = PROV_START;
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
}

static void send_pub_key(struct mesh_prov_acceptor *prov)
{
	struct prov_pub_key_msg msg;

	msg.opcode = PROV_PUB_KEY;
	memcpy(msg.pub_key, prov->conf_inputs.dev_pub_key, sizeof(msg.pub_key));
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
}

static void send_conf(struct mesh_prov_acceptor *prov)
{
	struct prov_conf_msg msg;

	msg.opcode = PROV_CONFIRM;
	mesh_crypto_aes_cmac(prov->calc_key, prov->rand_auth_workspace, 32,
								msg.conf);
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
}

static void send_rand(struct mesh_prov_acceptor *prov)
{
	struct prov_rand_msg msg;

	msg.opcode = PROV_RANDOM;
	memcpy(msg.rand, prov->rand_auth_workspace, sizeof(msg.rand));
	prov->trans_tx(prov->trans_data, &msg, sizeof(msg));
}

static void acp_prov_rx(void *user_data, const uint8_t *data, uint16_t len)
{
	struct mesh_prov_acceptor *rx_prov = user_data;
	struct mesh_prov_node_info *info;
	struct prov_fail_msg fail;
	uint8_t type = *data++;
	uint32_t oob_key;
	uint64_t decode_mic;
	bool result;

	if (rx_prov != prov || !prov->trans_tx)
		return;

	l_debug("Provisioning packet received type: %2.2x (%u octets)",
								type, len);

	if (type == prov->previous) {
		l_error("Ignore repeated %2.2x packet", type);
		return;
	} else if (type > prov->expected || type < prov->previous) {
		l_error("Expected %2.2x, Got:%2.2x", prov->expected, type);
		fail.reason = PROV_ERR_UNEXPECTED_PDU;
		goto failure;
	}

	if (type >= L_ARRAY_SIZE(expected_pdu_size) ||
					len != expected_pdu_size[type]) {
		l_error("Expected PDU size %d, Got %d (type: %2.2x)",
			len, expected_pdu_size[type], type);
		fail.reason = PROV_ERR_INVALID_FORMAT;
		goto failure;
	}

	switch (type){
	case PROV_INVITE: /* Prov Invite */
		prov->conf_inputs.invite.attention = data[0];
		send_caps(prov);
		break;

	case PROV_START: /* Prov Start */
		memcpy(&prov->conf_inputs.start, data,
				sizeof(prov->conf_inputs.start));

		if (prov->conf_inputs.start.algorithm ||
				prov->conf_inputs.start.pub_key > 1 ||
				prov->conf_inputs.start.auth_method > 3) {
			fail.reason = PROV_ERR_INVALID_FORMAT;
			goto failure;
		}

		if (prov->conf_inputs.start.pub_key) {
			if (prov->conf_inputs.caps.pub_type) {
				/* Prompt Agent for Private Key of OOB */
				mesh_agent_request_private_key(prov->agent,
							priv_key_cb, prov);
			} else {
				fail.reason = PROV_ERR_INVALID_PDU;
				goto failure;
			}
		} else {
			/* Ephemeral Public Key requested */
			ecc_make_key(prov->conf_inputs.dev_pub_key,
					prov->private_key);
			swap_u256_bytes(prov->conf_inputs.dev_pub_key);
			swap_u256_bytes(prov->conf_inputs.dev_pub_key + 32);
			prov->material |= MAT_LOCAL_PRIVATE;
		}

		prov->expected = PROV_PUB_KEY;
		break;

	case PROV_PUB_KEY: /* Public Key */
		/* Save Key */
		memcpy(prov->conf_inputs.prv_pub_key, data, 64);
		prov->material |= MAT_REMOTE_PUBLIC;
		prov->expected = PROV_CONFIRM;

		if ((prov->material & MAT_SECRET) != MAT_SECRET)
			return;

		if (!acp_credentials(prov)) {
			fail.reason = PROV_ERR_UNEXPECTED_ERR;
			goto failure;
		}

		if (!prov->conf_inputs.start.pub_key)
			send_pub_key(prov);

		/* Start Step 3 */
		switch (prov->conf_inputs.start.auth_method) {
		default:
		case 0:
			/* Auth Type 3c - No OOB */
			break;

		case 1:
			/* Auth Type 3c - Static OOB */
			/* Prompt Agent for Static OOB */
			fail.reason = mesh_agent_request_static(prov->agent,
					static_cb, prov);

			if (fail.reason)
				goto failure;

			break;

		case 2:
			/* Auth Type 3a - Output OOB */
			l_getrandom(&oob_key, sizeof(oob_key));
			oob_key %= digit_mod(prov->conf_inputs.start.auth_size);

			/* Save two copies, for two confirmation values */
			l_put_be32(oob_key, prov->rand_auth_workspace + 28);
			l_put_be32(oob_key, prov->rand_auth_workspace + 44);
			prov->material |= MAT_RAND_AUTH;

			if (prov->conf_inputs.start.auth_action ==
							PROV_ACTION_OUT_ALPHA) {
				/* TODO: Construst NUL-term string to pass */
				fail.reason = mesh_agent_display_string(
					prov->agent, NULL, NULL, prov);
			} else {
				/* Ask Agent to Display U32 */
				fail.reason = mesh_agent_display_number(
					prov->agent, false,
					prov->conf_inputs.start.auth_action,
					oob_key, NULL, prov);
			}

			if (fail.reason)
				goto failure;

			break;

		case 3:
			/* Auth Type 3b - input OOB */
			/* Prompt Agent for Input OOB */
			if (prov->conf_inputs.start.auth_action ==
							PROV_ACTION_IN_ALPHA) {
				fail.reason = mesh_agent_prompt_alpha(
					prov->agent, false,
					static_cb, prov);
			} else {
				fail.reason = mesh_agent_prompt_number(
					prov->agent, false,
					prov->conf_inputs.start.auth_action,
					number_cb, prov);
			}

			if (fail.reason)
				goto failure;

			break;
		}

		prov->expected = PROV_CONFIRM;
		break;

	case PROV_CONFIRM: /* Confirmation */
		/* Save Provisioners confirmation for later compare */
		memcpy(prov->confirm, data, 16);
		prov->expected = PROV_RANDOM;

		send_conf(prov);
		break;

	case PROV_RANDOM: /* Random Value */
		/* Calculate Session key (needed later) while data is fresh */
		mesh_crypto_prov_prov_salt(prov->salt, data,
						prov->rand_auth_workspace,
						prov->salt);
		mesh_crypto_session_key(prov->secret, prov->salt, prov->s_key);
		mesh_crypto_nonce(prov->secret, prov->salt, prov->s_nonce);

		/* Calculate expected Provisioner Confirm */
		memcpy(prov->rand_auth_workspace + 16, data, 16);
		mesh_crypto_aes_cmac(prov->calc_key,
					prov->rand_auth_workspace + 16, 32,
					prov->calc_key);

		/* Compare our calculation with Provisioners */
		if (memcmp(prov->calc_key, prov->confirm, 16)) {
			fail.reason = PROV_ERR_CONFIRM_FAILED;
			goto failure;
		}

		/* Send Random value we used */
		send_rand(prov);
		prov->expected = PROV_DATA;
		break;

	case PROV_DATA: /* Provisioning Data */

		/* Calculate our device key */
		mesh_crypto_device_key(prov->secret,
				prov->salt,
				prov->calc_key);

		/* Decrypt new node data into workspace */
		mesh_crypto_aes_ccm_decrypt(prov->s_nonce, prov->s_key,
				NULL, 0,
				data, len - 1, prov->rand_auth_workspace,
				&decode_mic, sizeof(decode_mic));

		/* Validate that the data hasn't been messed with in transit */
		if (l_get_be64(data + 25) != decode_mic) {
			l_error("Provisioning Failed-MIC compare");
			fail.reason = PROV_ERR_DECRYPT_FAILED;
			goto failure;
		}

		info = l_malloc(sizeof(struct mesh_prov_node_info));

		memcpy(info->device_key, prov->calc_key, 16);
		memcpy(info->net_key, prov->rand_auth_workspace, 16);
		info->net_index = l_get_be16(prov->rand_auth_workspace + 16);
		info->flags = prov->rand_auth_workspace[18];
		info->iv_index = l_get_be32(prov->rand_auth_workspace + 19);
		info->unicast = l_get_be16(prov->rand_auth_workspace + 23);

		result = prov->cmplt(prov->caller_data, PROV_ERR_SUCCESS, info);
		prov->cmplt = NULL;
		l_free(info);

		if (result) {
			prov->rand_auth_workspace[0] = PROV_COMPLETE;
			prov->trans_tx(prov->trans_data,
					prov->rand_auth_workspace, 1);
			goto cleanup;
		} else {
			fail.reason = PROV_ERR_UNEXPECTED_ERR;
			goto failure;
		}
		break;

	case PROV_FAILED: /* Provisioning Error -- abort */
		/* TODO: Call Complete Callback (Fail)*/
		prov->cmplt(prov->caller_data,
				data[0] ? data[0] : PROV_ERR_UNEXPECTED_ERR,
				NULL);
		prov->cmplt = NULL;
		goto cleanup;
	}

	if (prov)
		prov->previous = type;
	return;

failure:
	fail.opcode = PROV_FAILED;
	prov->trans_tx(prov->trans_data, &fail, sizeof(fail));
	if (prov->cmplt)
		prov->cmplt(prov->caller_data, fail.reason, NULL);
	prov->cmplt = NULL;

cleanup:
	l_timeout_remove(prov->timeout);

	/* Give PB Link 5 seconds to end session */
	prov->timeout = l_timeout_create(5, prov_to, prov, NULL);
}

static void acp_prov_ack(void *user_data, uint8_t msg_num)
{
	/* TODO: Handle PB-ADV Ack */
}


/* This starts unprovisioned device beacon */
bool acceptor_start(uint8_t num_ele, uint8_t uuid[16],
		uint16_t algorithms, uint32_t timeout,
		struct mesh_agent *agent,
		mesh_prov_acceptor_complete_func_t complete_cb,
		void *caller_data)
{
	struct mesh_agent_prov_caps *caps;
	uint8_t beacon[24] = {MESH_AD_TYPE_BEACON,
						BEACON_TYPE_UNPROVISIONED};
	uint8_t len = sizeof(beacon) - sizeof(uint32_t);
	bool result;

	/* Invoked from Join() method in mesh-api.txt, to join a
	 * remote mesh network.
	 */

	if (prov)
		return false;

	prov = l_new(struct mesh_prov_acceptor, 1);
	prov->to_secs = timeout;
	prov->agent = agent;
	prov->cmplt = complete_cb;
	prov->previous = -1;
	prov->caller_data = caller_data;

	caps = mesh_agent_get_caps(agent);

	/* TODO: Should we sanity check values here or elsewhere? */
	prov->conf_inputs.caps.num_ele = num_ele;
	prov->conf_inputs.caps.pub_type = caps->pub_type;
	prov->conf_inputs.caps.static_type = caps->static_type;
	prov->conf_inputs.caps.output_size = caps->output_size;
	prov->conf_inputs.caps.input_size = caps->input_size;

	/* Store UINT16 values in Over-the-Air order, in packed structure
	 * for crypto inputs
	 */
	l_put_be16(algorithms, &prov->conf_inputs.caps.algorithms);
	l_put_be16(caps->output_action, &prov->conf_inputs.caps.output_action);
	l_put_be16(caps->input_action, &prov->conf_inputs.caps.input_action);

	/* Compose Unprovisioned Beacon */
	memcpy(beacon + 2, uuid, 16);
	l_put_be16(caps->oob_info, beacon + 18);
	if (caps->oob_info & OOB_INFO_URI_HASH){
		l_put_be32(caps->uri_hash, beacon + 20);
		len += sizeof(uint32_t);
	}

	/* Infinitely Beacon until Canceled, or Provisioning Starts */
	result = mesh_send_pkt(0, 500, beacon, len);

	if (!result)
		goto error_fail;

	/* Always register for PB-ADV */
	result = pb_adv_reg(false, acp_prov_open, acp_prov_close, acp_prov_rx,
						acp_prov_ack, uuid, prov);

	if (result)
		return true;

error_fail:
	acceptor_free();
	return false;
}

void acceptor_cancel(void *user_data)
{
	acceptor_free();
}
