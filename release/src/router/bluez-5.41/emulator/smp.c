/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/crypto.h"
#include "src/shared/ecc.h"
#include "monitor/bt.h"
#include "bthost.h"

#define SMP_CID		0x0006
#define SMP_BREDR_CID	0x0007

#define L2CAP_FC_SMP_BREDR	0x80

#define SMP_PASSKEY_ENTRY_FAILED	0x01
#define SMP_OOB_NOT_AVAIL		0x02
#define SMP_AUTH_REQUIREMENTS		0x03
#define SMP_CONFIRM_FAILED		0x04
#define SMP_PAIRING_NOTSUPP		0x05
#define SMP_ENC_KEY_SIZE		0x06
#define SMP_CMD_NOTSUPP			0x07
#define SMP_UNSPECIFIED			0x08
#define SMP_REPEATED_ATTEMPTS		0x09
#define SMP_INVALID_PARAMS		0x0a
#define SMP_DHKEY_CHECK_FAILED		0x0b
#define SMP_NUMERIC_COMP_FAILED		0x0c
#define SMP_BREDR_PAIRING_IN_PROGRESS	0x0d

#define DIST_ENC_KEY	0x01
#define DIST_ID_KEY	0x02
#define DIST_SIGN	0x04
#define DIST_LINK_KEY	0x08

#define SC_NO_DIST	(DIST_ENC_KEY | DIST_LINK_KEY)

#define MAX_IO_CAP	0x04

#define SMP_AUTH_NONE		0x00
#define SMP_AUTH_BONDING	0x01
#define SMP_AUTH_MITM		0x04
#define SMP_AUTH_SC		0x08
#define SMP_AUTH_KEYPRESS	0x10

struct smp {
	struct bthost *bthost;
	struct smp_conn *conn;
	struct bt_crypto *crypto;
};

struct smp_conn {
	struct smp *smp;
	uint16_t handle;
	uint8_t addr_type;
	bool out;
	bool sc;
	bool initiator;
	uint8_t method;
	uint8_t local_key_dist;
	uint8_t remote_key_dist;
	uint8_t ia[6];
	uint8_t ia_type;
	uint8_t ra[6];
	uint8_t ra_type;
	uint8_t tk[16];
	uint8_t prnd[16];
	uint8_t rrnd[16];
	uint8_t pcnf[16];
	uint8_t preq[7];
	uint8_t prsp[7];
	uint8_t ltk[16];

	uint8_t local_sk[32];
	uint8_t local_pk[64];
	uint8_t remote_pk[64];
	uint8_t dhkey[32];
	uint8_t mackey[16];

	uint8_t passkey_notify;
	uint8_t passkey_round;
};

enum {
	JUST_WORKS,
	JUST_CFM,
	REQ_PASSKEY,
	CFM_PASSKEY,
	REQ_OOB,
	DSP_PASSKEY,
	OVERLAP,
};

static const uint8_t gen_method[5][5] = {
	{ JUST_WORKS,  JUST_CFM,    REQ_PASSKEY, JUST_WORKS, REQ_PASSKEY },
	{ JUST_WORKS,  JUST_CFM,    REQ_PASSKEY, JUST_WORKS, REQ_PASSKEY },
	{ CFM_PASSKEY, CFM_PASSKEY, REQ_PASSKEY, JUST_WORKS, CFM_PASSKEY },
	{ JUST_WORKS,  JUST_CFM,    JUST_WORKS,  JUST_WORKS, JUST_CFM    },
	{ CFM_PASSKEY, CFM_PASSKEY, REQ_PASSKEY, JUST_WORKS, OVERLAP     },
};

static const uint8_t sc_method[5][5] = {
	{ JUST_WORKS,  JUST_CFM,    REQ_PASSKEY, JUST_WORKS, REQ_PASSKEY },
	{ JUST_WORKS,  CFM_PASSKEY, REQ_PASSKEY, JUST_WORKS, CFM_PASSKEY },
	{ DSP_PASSKEY, DSP_PASSKEY, REQ_PASSKEY, JUST_WORKS, DSP_PASSKEY },
	{ JUST_WORKS,  JUST_CFM,    JUST_WORKS,  JUST_WORKS, JUST_CFM    },
	{ DSP_PASSKEY, CFM_PASSKEY, REQ_PASSKEY, JUST_WORKS, CFM_PASSKEY },
};

static uint8_t get_auth_method(struct smp_conn *conn, uint8_t local_io,
							uint8_t remote_io)
{
	/* If either side has unknown io_caps, use JUST_CFM (which gets
	 * converted later to JUST_WORKS if we're initiators.
	 */
	if (local_io > MAX_IO_CAP || remote_io > MAX_IO_CAP)
		return JUST_CFM;

	if (conn->sc)
		return sc_method[remote_io][local_io];

	return gen_method[remote_io][local_io];
}

static uint8_t sc_select_method(struct smp_conn *conn)
{
	struct bt_l2cap_smp_pairing_request *local, *remote;
	uint8_t local_mitm, remote_mitm, local_io, remote_io, method;

	if (conn->out) {
		local = (void *) &conn->preq[1];
		remote = (void *) &conn->prsp[1];
	} else {
		local = (void *) &conn->prsp[1];
		remote = (void *) &conn->preq[1];
	}

	local_io = local->io_capa;
	remote_io = remote->io_capa;

	local_mitm = (local->auth_req & SMP_AUTH_MITM);
	remote_mitm = (remote->auth_req & SMP_AUTH_MITM);

	/* If either side wants MITM, look up the method from the table,
	 * otherwise use JUST WORKS.
	 */
	if (local_mitm || remote_mitm)
		method = get_auth_method(conn, local_io, remote_io);
	else
		method = JUST_WORKS;

	/* Don't confirm locally initiated pairing attempts */
	if (method == JUST_CFM && conn->initiator)
		method = JUST_WORKS;

	return method;
}

static uint8_t key_dist(struct bthost *host)
{
	if (!bthost_bredr_capable(host))
		return (DIST_ENC_KEY | DIST_ID_KEY | DIST_SIGN);

	return (DIST_ENC_KEY | DIST_ID_KEY | DIST_SIGN | DIST_LINK_KEY);
}

static void smp_send(struct smp_conn *conn, uint8_t smp_cmd, const void *data,
								uint8_t len)
{
	struct iovec iov[2];
	uint16_t cid;

	iov[0].iov_base = &smp_cmd;
	iov[0].iov_len = 1;

	iov[1].iov_base = (void *) data;
	iov[1].iov_len = len;

	if (conn->addr_type == BDADDR_BREDR)
		cid = SMP_BREDR_CID;
	else
		cid = SMP_CID;

	bthost_send_cid_v(conn->smp->bthost, conn->handle, cid, iov, 2);
}

static bool send_public_key(struct smp_conn *conn)
{
	if (!ecc_make_key(conn->local_pk, conn->local_sk))
		return false;

	smp_send(conn, BT_L2CAP_SMP_PUBLIC_KEY, conn->local_pk, 64);

	return true;
}

static void sc_dhkey_check(struct smp_conn *conn)
{
	uint8_t io_cap[3], r[16], a[7], b[7], *local_addr, *remote_addr;
	struct bt_l2cap_smp_dhkey_check check;

	memcpy(a, conn->ia, 6);
	memcpy(b, conn->ra, 6);
	a[6] = conn->ia_type;
	b[6] = conn->ra_type;

	if (conn->out) {
		local_addr = a;
		remote_addr = b;
		memcpy(io_cap, &conn->preq[1], 3);
	} else {
		local_addr = b;
		remote_addr = a;
		memcpy(io_cap, &conn->prsp[1], 3);
	}

	memset(r, 0, sizeof(r));

	bt_crypto_f6(conn->smp->crypto, conn->mackey, conn->prnd, conn->rrnd,
				r, io_cap, local_addr, remote_addr, check.e);

	smp_send(conn, BT_L2CAP_SMP_DHKEY_CHECK, &check, sizeof(check));
}

static void sc_mackey_and_ltk(struct smp_conn *conn)
{
	uint8_t *na, *nb, a[7], b[7];

	if (conn->out) {
		na = conn->prnd;
		nb = conn->rrnd;
	} else {
		na = conn->rrnd;
		nb = conn->prnd;
	}

	memcpy(a, conn->ia, 6);
	memcpy(b, conn->ra, 6);
	a[6] = conn->ia_type;
	b[6] = conn->ra_type;

	bt_crypto_f5(conn->smp->crypto, conn->dhkey, na, nb, a, b,
						conn->mackey, conn->ltk);
}

static uint8_t sc_passkey_send_confirm(struct smp_conn *conn)
{
	struct bt_l2cap_smp_pairing_confirm cfm;
	uint8_t r;

	r = ((conn->passkey_notify >> conn->passkey_round) & 0x01);
	r |= 0x80;

	if (!bt_crypto_f4(conn->smp->crypto, conn->local_pk, conn->remote_pk,
					conn->prnd, r, cfm.value))
		return SMP_UNSPECIFIED;

	smp_send(conn, BT_L2CAP_SMP_PAIRING_CONFIRM, &cfm, sizeof(cfm));

	return 0;
}

static uint8_t sc_passkey_round(struct smp_conn *conn, uint8_t smp_op)
{
	uint8_t cfm[16], r;

	/* Ignore the PDU if we've already done 20 rounds (0 - 19) */
	if (conn->passkey_round >= 20)
		return 0;

	switch (smp_op) {
	case BT_L2CAP_SMP_PAIRING_RANDOM:
		r = ((conn->passkey_notify >> conn->passkey_round) & 0x01);
		r |= 0x80;

		if (!bt_crypto_f4(conn->smp->crypto, conn->remote_pk,
					conn->local_pk, conn->rrnd, r, cfm))
			return SMP_UNSPECIFIED;

		if (memcmp(conn->pcnf, cfm, 16))
			return SMP_CONFIRM_FAILED;

		conn->passkey_round++;

		if (conn->passkey_round == 20) {
			/* Generate MacKey and LTK */
			sc_mackey_and_ltk(conn);
		}

		/* The round is only complete when the initiator
		 * receives pairing random.
		 */
		if (!conn->out) {
			smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM,
					conn->prnd, sizeof(conn->prnd));
			return 0;
		}

		/* Start the next round */
		if (conn->passkey_round != 20)
			return sc_passkey_round(conn, 0);

		/* Passkey rounds are complete - start DHKey Check */
		sc_dhkey_check(conn);

		break;

	case BT_L2CAP_SMP_PAIRING_CONFIRM:
		if (conn->out) {
			smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM,
					conn->prnd, sizeof(conn->prnd));
			return 0;
		}

		return sc_passkey_send_confirm(conn);

	case BT_L2CAP_SMP_PUBLIC_KEY:
	default:
		/* Initiating device starts the round */
		if (!conn->out)
			return 0;

		return sc_passkey_send_confirm(conn);
	}

	return 0;
}

static bool verify_random(struct smp_conn *conn, const uint8_t rnd[16])
{
	uint8_t confirm[16];

	if (!bt_crypto_c1(conn->smp->crypto, conn->tk, conn->rrnd, conn->prsp,
				conn->preq, conn->ia_type, conn->ia,
				conn->ra_type, conn->ra, confirm))
		return false;

	if (memcmp(conn->pcnf, confirm, sizeof(conn->pcnf) != 0)) {
		printf("Confirmation values don't match\n");
		return false;
	}

	if (conn->out) {
		bt_crypto_s1(conn->smp->crypto, conn->tk, conn->rrnd,
							conn->prnd, conn->ltk);
		bthost_le_start_encrypt(conn->smp->bthost, conn->handle,
								conn->ltk);
	} else {
		bt_crypto_s1(conn->smp->crypto, conn->tk, conn->prnd,
							conn->rrnd, conn->ltk);
	}

	return true;
}

static void distribute_keys(struct smp_conn *conn)
{
	uint8_t buf[16];

	if (conn->local_key_dist & DIST_ENC_KEY) {
		memset(buf, 0, sizeof(buf));
		smp_send(conn, BT_L2CAP_SMP_ENCRYPT_INFO, buf, sizeof(buf));
		smp_send(conn, BT_L2CAP_SMP_MASTER_IDENT, buf, 10);
	}

	if (conn->local_key_dist & DIST_ID_KEY) {
		memset(buf, 0, sizeof(buf));
		smp_send(conn, BT_L2CAP_SMP_IDENT_INFO, buf, sizeof(buf));

		memset(buf, 0, sizeof(buf));

		if (conn->out) {
			buf[0] = conn->ia_type;
			memcpy(&buf[1], conn->ia, 6);
		} else {
			buf[0] = conn->ra_type;
			memcpy(&buf[1], conn->ra, 6);
		}

		smp_send(conn, BT_L2CAP_SMP_IDENT_ADDR_INFO, buf, 7);
	}

	if (conn->local_key_dist & DIST_SIGN) {
		memset(buf, 0, sizeof(buf));
		smp_send(conn, BT_L2CAP_SMP_SIGNING_INFO, buf, sizeof(buf));
	}
}

static void pairing_req(struct smp_conn *conn, const void *data, uint16_t len)
{
	struct bthost *bthost = conn->smp->bthost;
	struct bt_l2cap_smp_pairing_response rsp;

	memcpy(conn->preq, data, sizeof(conn->preq));

	if (conn->addr_type == BDADDR_BREDR) {
		rsp.io_capa	= 0x00;
		rsp.oob_data	= 0x00;
		rsp.auth_req	= 0x00;
	} else {
		rsp.io_capa	= bthost_get_io_capability(bthost);
		rsp.oob_data	= 0x00;
		rsp.auth_req	= bthost_get_auth_req(bthost);
	}

	rsp.max_key_size	= 0x10;
	rsp.init_key_dist	= conn->preq[5] & key_dist(bthost);
	rsp.resp_key_dist	= conn->preq[6] & key_dist(bthost);

	conn->prsp[0] = BT_L2CAP_SMP_PAIRING_RESPONSE;
	memcpy(&conn->prsp[1], &rsp, sizeof(rsp));

	conn->local_key_dist	= rsp.resp_key_dist;
	conn->remote_key_dist	= rsp.init_key_dist;

	if (((conn->prsp[3] & 0x08) && (conn->preq[3] & 0x08)) ||
					conn->addr_type == BDADDR_BREDR) {
		conn->sc = true;
		conn->local_key_dist &= ~SC_NO_DIST;
		conn->remote_key_dist &= ~SC_NO_DIST;
	}

	smp_send(conn, BT_L2CAP_SMP_PAIRING_RESPONSE, &rsp, sizeof(rsp));

	if (conn->addr_type == BDADDR_BREDR)
		distribute_keys(conn);
}

static void pairing_rsp(struct smp_conn *conn, const void *data, uint16_t len)
{
	struct smp *smp = conn->smp;
	uint8_t cfm[16];

	memcpy(conn->prsp, data, sizeof(conn->prsp));

	conn->local_key_dist = conn->prsp[5];
	conn->remote_key_dist = conn->prsp[6];

	if (conn->addr_type == BDADDR_BREDR) {
		conn->local_key_dist &= ~SC_NO_DIST;
		conn->remote_key_dist &= ~SC_NO_DIST;
		distribute_keys(conn);
		return;
	}

	if (((conn->prsp[3] & 0x08) && (conn->preq[3] & 0x08)) ||
					conn->addr_type == BDADDR_BREDR) {
		conn->sc = true;
		conn->local_key_dist &= ~SC_NO_DIST;
		conn->remote_key_dist &= ~SC_NO_DIST;
		if (conn->addr_type == BDADDR_BREDR)
			distribute_keys(conn);
		else
			send_public_key(conn);
		return;
	}

	bt_crypto_c1(smp->crypto, conn->tk, conn->prnd, conn->prsp,
			conn->preq, conn->ia_type, conn->ia,
			conn->ra_type, conn->ra, cfm);

	smp_send(conn, BT_L2CAP_SMP_PAIRING_CONFIRM, cfm, sizeof(cfm));
}
static void sc_check_confirm(struct smp_conn *conn)
{
	if (conn->method == REQ_PASSKEY || conn->method == DSP_PASSKEY) {
		sc_passkey_round(conn, BT_L2CAP_SMP_PAIRING_CONFIRM);
		return;
	}

	if (conn->out)
		smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM, conn->prnd,
							sizeof(conn->prnd));
}

static void pairing_cfm(struct smp_conn *conn, const void *data, uint16_t len)
{
	uint8_t rsp[16];

	memcpy(conn->pcnf, data + 1, 16);

	if (conn->sc) {
		sc_check_confirm(conn);
		return;
	}

	if (conn->out) {
		memset(rsp, 0, sizeof(rsp));
		smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM, rsp, sizeof(rsp));
	} else {
		bt_crypto_c1(conn->smp->crypto, conn->tk, conn->prnd,
				conn->prsp, conn->preq, conn->ia_type,
				conn->ia, conn->ra_type, conn->ra, rsp);
		smp_send(conn, BT_L2CAP_SMP_PAIRING_CONFIRM, rsp, sizeof(rsp));
	}
}

static uint8_t sc_random(struct smp_conn *conn)
{
	/* Passkey entry has special treatment */
	if (conn->method == REQ_PASSKEY || conn->method == DSP_PASSKEY)
		return sc_passkey_round(conn, BT_L2CAP_SMP_PAIRING_RANDOM);

	if (conn->out) {
		uint8_t cfm[16];

		bt_crypto_f4(conn->smp->crypto, conn->remote_pk,
					conn->local_pk, conn->rrnd, 0, cfm);

		if (memcmp(conn->pcnf, cfm, 16))
			return 0x04; /* Confirm Value Failed */
	} else {
		smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM, conn->prnd, 16);
	}

	sc_mackey_and_ltk(conn);

	if (conn->out)
		sc_dhkey_check(conn);

	return 0;
}

static void pairing_rnd(struct smp_conn *conn, const void *data, uint16_t len)
{
	uint8_t rsp[16];

	memcpy(conn->rrnd, data + 1, 16);

	if (conn->sc) {
		uint8_t reason = sc_random(conn);
		if (reason)
			smp_send(conn, BT_L2CAP_SMP_PAIRING_FAILED, &reason,
							sizeof(reason));
		return;
	}

	if (!verify_random(conn, data + 1))
		return;

	if (conn->out)
		return;

	memset(rsp, 0, sizeof(rsp));
	smp_send(conn, BT_L2CAP_SMP_PAIRING_RANDOM, rsp, sizeof(rsp));
}

static void encrypt_info(struct smp_conn *conn, const void *data, uint16_t len)
{
}

static void master_ident(struct smp_conn *conn, const void *data, uint16_t len)
{
	conn->remote_key_dist &= ~DIST_ENC_KEY;

	if (conn->out && !conn->remote_key_dist)
		distribute_keys(conn);
}

static void ident_addr_info(struct smp_conn *conn, const void *data,
								uint16_t len)
{
}

static void ident_info(struct smp_conn *conn, const void *data, uint16_t len)
{
	conn->remote_key_dist &= ~DIST_ID_KEY;

	if (conn->out && !conn->remote_key_dist)
		distribute_keys(conn);
}

static void signing_info(struct smp_conn *conn, const void *data, uint16_t len)
{
	conn->remote_key_dist &= ~DIST_SIGN;

	if (conn->out && !conn->remote_key_dist)
		distribute_keys(conn);
}

static void public_key(struct smp_conn *conn, const void *data, uint16_t len)
{
	struct smp *smp = conn->smp;
	uint8_t buf[16];

	memcpy(conn->remote_pk, data + 1, 64);

	if (!conn->out) {
		if (!send_public_key(conn))
			return;
	}

	if (!ecdh_shared_secret(conn->remote_pk, conn->local_sk, conn->dhkey))
		return;

	conn->method = sc_select_method(conn);

	if (conn->method == DSP_PASSKEY || conn->method == REQ_PASSKEY) {
		sc_passkey_round(conn, BT_L2CAP_SMP_PUBLIC_KEY);
		return;
	}

	if (conn->out)
		return;

	if (!bt_crypto_f4(smp->crypto, conn->local_pk, conn->remote_pk,
							conn->prnd, 0, buf))
		return;

	smp_send(conn, BT_L2CAP_SMP_PAIRING_CONFIRM, buf, sizeof(buf));
}

static void dhkey_check(struct smp_conn *conn, const void *data, uint16_t len)
{
	const struct bt_l2cap_smp_dhkey_check *cmd = data + 1;
	uint8_t a[7], b[7], *local_addr, *remote_addr;
	uint8_t io_cap[3], r[16], e[16];

	memcpy(a, &conn->ia, 6);
	memcpy(b, &conn->ra, 6);
	a[6] = conn->ia_type;
	b[6] = conn->ra_type;

	if (conn->out) {
		local_addr = a;
		remote_addr = b;
		memcpy(io_cap, &conn->prsp[1], 3);
	} else {
		local_addr = b;
		remote_addr = a;
		memcpy(io_cap, &conn->preq[1], 3);
	}

	memset(r, 0, sizeof(r));

	if (conn->method == REQ_PASSKEY || conn->method == DSP_PASSKEY)
		put_le32(conn->passkey_notify, r);

	if (!bt_crypto_f6(conn->smp->crypto, conn->mackey, conn->rrnd,
			conn->prnd, r, io_cap, remote_addr, local_addr, e))
		return;

	if (memcmp(cmd->e, e, 16)) {
		uint8_t reason = 0x0b; /* DHKey Check Failed */
		smp_send(conn, BT_L2CAP_SMP_PAIRING_FAILED, &reason,
							sizeof(reason));
	}

	if (conn->out)
		bthost_le_start_encrypt(conn->smp->bthost, conn->handle,
								conn->ltk);
	else
		sc_dhkey_check(conn);
}

void smp_pair(void *conn_data, uint8_t io_cap, uint8_t auth_req)
{
	struct smp_conn *conn = conn_data;
	struct bt_l2cap_smp_pairing_request req;

	req.io_capa		= io_cap;
	req.oob_data		= 0x00;
	req.auth_req		= auth_req;
	req.max_key_size	= 0x10;
	req.init_key_dist	= key_dist(conn->smp->bthost);
	req.resp_key_dist	= key_dist(conn->smp->bthost);

	conn->preq[0] = BT_L2CAP_SMP_PAIRING_REQUEST;
	memcpy(&conn->preq[1], &req, sizeof(req));

	smp_send(conn, BT_L2CAP_SMP_PAIRING_REQUEST, &req, sizeof(req));
}

void smp_data(void *conn_data, const void *data, uint16_t len)
{
	struct smp_conn *conn = conn_data;
	uint8_t opcode;

	if (len < 1) {
		printf("Received too small SMP PDU\n");
		return;
	}

	if (conn->addr_type == BDADDR_BREDR) {
		printf("Received BR/EDR SMP data on LE link\n");
		return;
	}

	opcode = *((const uint8_t *) data);

	switch (opcode) {
	case BT_L2CAP_SMP_PAIRING_REQUEST:
		pairing_req(conn, data, len);
		break;
	case BT_L2CAP_SMP_PAIRING_RESPONSE:
		pairing_rsp(conn, data, len);
		break;
	case BT_L2CAP_SMP_PAIRING_CONFIRM:
		pairing_cfm(conn, data, len);
		break;
	case BT_L2CAP_SMP_PAIRING_RANDOM:
		pairing_rnd(conn, data, len);
		break;
	case BT_L2CAP_SMP_ENCRYPT_INFO:
		encrypt_info(conn, data, len);
		break;
	case BT_L2CAP_SMP_MASTER_IDENT:
		master_ident(conn, data, len);
		break;
	case BT_L2CAP_SMP_IDENT_ADDR_INFO:
		ident_addr_info(conn, data, len);
		break;
	case BT_L2CAP_SMP_IDENT_INFO:
		ident_info(conn, data, len);
		break;
	case BT_L2CAP_SMP_SIGNING_INFO:
		signing_info(conn, data, len);
		break;
	case BT_L2CAP_SMP_PUBLIC_KEY:
		public_key(conn, data, len);
		break;
	case BT_L2CAP_SMP_DHKEY_CHECK:
		dhkey_check(conn, data, len);
		break;
	default:
		break;
	}
}

void smp_bredr_data(void *conn_data, const void *data, uint16_t len)
{
	struct smp_conn *conn = conn_data;
	uint8_t opcode;

	if (len < 1) {
		printf("Received too small SMP PDU\n");
		return;
	}

	if (conn->addr_type != BDADDR_BREDR) {
		printf("Received LE SMP data on BR/EDR link\n");
		return;
	}

	opcode = *((const uint8_t *) data);

	switch (opcode) {
	case BT_L2CAP_SMP_PAIRING_REQUEST:
		pairing_req(conn, data, len);
		break;
	case BT_L2CAP_SMP_PAIRING_RESPONSE:
		pairing_rsp(conn, data, len);
		break;
	default:
		break;
	}
}

int smp_get_ltk(void *smp_data, uint64_t rand, uint16_t ediv, uint8_t *ltk)
{
	struct smp_conn *conn = smp_data;
	static const uint8_t no_ltk[16] = { 0 };

	if (!memcmp(conn->ltk, no_ltk, 16))
		return -ENOENT;

	memcpy(ltk, conn->ltk, 16);

	return 0;
}

static void smp_conn_bredr(struct smp_conn *conn, uint8_t encrypt)
{
	struct smp *smp = conn->smp;
	struct bt_l2cap_smp_pairing_request req;
	uint64_t fixed_chan;

	if (encrypt != 0x02)
		return;

	conn->sc = true;

	if (!conn->out)
		return;

	fixed_chan = bthost_conn_get_fixed_chan(smp->bthost, conn->handle);
	if (!(fixed_chan & L2CAP_FC_SMP_BREDR))
		return;

	memset(&req, 0, sizeof(req));
	req.max_key_size = 0x10;
	req.init_key_dist = key_dist(smp->bthost);
	req.resp_key_dist = key_dist(smp->bthost);

	smp_send(conn, BT_L2CAP_SMP_PAIRING_REQUEST, &req, sizeof(req));
}

void smp_conn_encrypted(void *conn_data, uint8_t encrypt)
{
	struct smp_conn *conn = conn_data;

	if (!encrypt)
		return;

	if (conn->addr_type == BDADDR_BREDR) {
		smp_conn_bredr(conn, encrypt);
		return;
	}

	if (conn->out && conn->remote_key_dist)
		return;

	distribute_keys(conn);
}

void *smp_conn_add(void *smp_data, uint16_t handle, const uint8_t *ia,
			const uint8_t *ra, uint8_t addr_type, bool conn_init)
{
	struct smp *smp = smp_data;
	struct smp_conn *conn;

	conn = malloc(sizeof(struct smp_conn));
	if (!conn)
		return NULL;

	memset(conn, 0, sizeof(*conn));

	conn->smp = smp;
	conn->handle = handle;
	conn->addr_type = addr_type;
	conn->out = conn_init;

	conn->ia_type = LE_PUBLIC_ADDRESS;
	conn->ra_type = LE_PUBLIC_ADDRESS;
	memcpy(conn->ia, ia, 6);
	memcpy(conn->ra, ra, 6);

	return conn;
}

void smp_conn_del(void *conn_data)
{
	struct smp_conn *conn = conn_data;

	free(conn);
}

void *smp_start(struct bthost *bthost)
{
	struct smp *smp;

	smp = malloc(sizeof(struct smp));
	if (!smp)
		return NULL;

	memset(smp, 0, sizeof(*smp));

	smp->crypto = bt_crypto_new();
	if (!smp->crypto) {
		free(smp);
		return NULL;
	}

	smp->bthost = bthost;

	return smp;
}

void smp_stop(void *smp_data)
{
	struct smp *smp = smp_data;

	bt_crypto_unref(smp->crypto);

	free(smp);
}
