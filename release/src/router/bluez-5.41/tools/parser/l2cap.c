/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2002  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2003-2011  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "parser.h"
#include "sdp.h"
#include "l2cap.h"
#include "lib/hci.h"
#include "lib/a2mp.h"
#include "lib/amp.h"

typedef struct {
	uint16_t handle;
	struct frame frm;
} handle_info;
#define HANDLE_TABLE_SIZE 10

static handle_info handle_table[HANDLE_TABLE_SIZE];

typedef struct {
	uint16_t handle;
	uint16_t cid;
	uint16_t psm;
	uint16_t num;
	uint8_t mode;
	uint8_t ext_ctrl;
} cid_info;
#define CID_TABLE_SIZE 20

static cid_info cid_table[2][CID_TABLE_SIZE];

#define SCID cid_table[0]
#define DCID cid_table[1]

/* Can we move this to l2cap.h? */
struct features {
	char	*name;
	int	flag;
};

static struct features l2cap_features[] = {
	{ "Flow control mode",			L2CAP_FEAT_FLOWCTL	},
	{ "Retransmission mode",		L2CAP_FEAT_RETRANS	},
	{ "Bi-directional QoS",			L2CAP_FEAT_BIDIR_QOS	},
	{ "Enhanced Retransmission mode",	L2CAP_FEAT_ERTM		},
	{ "Streaming mode",			L2CAP_FEAT_STREAMING	},
	{ "FCS Option",				L2CAP_FEAT_FCS		},
	{ "Extended Flow Specification",	L2CAP_FEAT_EXT_FLOW	},
	{ "Fixed Channels",			L2CAP_FEAT_FIXED_CHAN	},
	{ "Extended Window Size",		L2CAP_FEAT_EXT_WINDOW	},
	{ "Unicast Connectless Data Reception",	L2CAP_FEAT_UCD		},
	{ 0 }
};

static struct features l2cap_fix_chan[] = {
	{ "L2CAP Signalling Channel",		L2CAP_FC_L2CAP		},
	{ "L2CAP Connless",			L2CAP_FC_CONNLESS	},
	{ "AMP Manager Protocol",		L2CAP_FC_A2MP		},
	{ 0 }
};

static struct frame *add_handle(uint16_t handle)
{
	register handle_info *t = handle_table;
	register int i;

	for (i = 0; i < HANDLE_TABLE_SIZE; i++)
		if (!t[i].handle) {
			t[i].handle = handle;
			return &t[i].frm;
		}
	return NULL;
}

static struct frame *get_frame(uint16_t handle)
{
	register handle_info *t = handle_table;
	register int i;

	for (i = 0; i < HANDLE_TABLE_SIZE; i++)
		if (t[i].handle == handle)
			return &t[i].frm;

	return add_handle(handle);
}

static void add_cid(int in, uint16_t handle, uint16_t cid, uint16_t psm)
{
	register cid_info *table = cid_table[in];
	register int i, pos = -1;
	uint16_t num = 1;

	for (i = 0; i < CID_TABLE_SIZE; i++) {
		if ((pos < 0 && !table[i].cid) || table[i].cid == cid)
			pos = i;
		if (table[i].psm == psm)
			num++;
	}

	if (pos >= 0) {
		table[pos].handle = handle;
		table[pos].cid    = cid;
		table[pos].psm    = psm;
		table[pos].num    = num;
		table[pos].mode   = 0;
	}
}

static void del_cid(int in, uint16_t dcid, uint16_t scid)
{
	register int t, i;
	uint16_t cid[2];

	if (!in) {
		cid[0] = dcid;
		cid[1] = scid;
	} else {
		cid[0] = scid;
		cid[1] = dcid;
	}

	for (t = 0; t < 2; t++) {
		for (i = 0; i < CID_TABLE_SIZE; i++)
			if (cid_table[t][i].cid == cid[t]) {
				cid_table[t][i].handle = 0;
				cid_table[t][i].cid    = 0;
				cid_table[t][i].psm    = 0;
				cid_table[t][i].num    = 0;
				cid_table[t][i].mode   = 0;
				break;
			}
	}
}

static void del_handle(uint16_t handle)
{
	register int t, i;

	for (t = 0; t < 2; t++) {
		for (i = 0; i < CID_TABLE_SIZE; i++)
			if (cid_table[t][i].handle == handle) {
				cid_table[t][i].handle = 0;
				cid_table[t][i].cid    = 0;
				cid_table[t][i].psm    = 0;
				cid_table[t][i].num    = 0;
				cid_table[t][i].mode   = 0;
				break;
			}
	}
}
static uint16_t get_psm(int in, uint16_t handle, uint16_t cid)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			return table[i].psm;
	return parser.defpsm;
}

static uint16_t get_num(int in, uint16_t handle, uint16_t cid)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			return table[i].num;
	return 0;
}

static void set_mode(int in, uint16_t handle, uint16_t cid, uint8_t mode)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			table[i].mode = mode;
}

static uint8_t get_mode(int in, uint16_t handle, uint16_t cid)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			return table[i].mode;
	return 0;
}

static void set_ext_ctrl(int in, uint16_t handle, uint16_t cid,
							uint8_t ext_ctrl)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			table[i].ext_ctrl = ext_ctrl;
}

static uint8_t get_ext_ctrl(int in, uint16_t handle, uint16_t cid)
{
	register cid_info *table = cid_table[in];
	register int i;

	for (i = 0; i < CID_TABLE_SIZE; i++)
		if (table[i].handle == handle && table[i].cid == cid)
			return table[i].ext_ctrl;
	return 0;
}

static uint32_t get_val(uint8_t *ptr, uint8_t len)
{
	switch (len) {
	case 1:
		return *ptr;
	case 2:
		return get_le16(ptr);
	case 4:
		return get_le32(ptr);
	}
	return 0;
}

static char *reason2str(uint16_t reason)
{
	switch (reason) {
	case 0x0000:
		return "Command not understood";
	case 0x0001:
		return "Signalling MTU exceeded";
	case 0x0002:
		return "Invalid CID in request";
	default:
		return "Reserved";
	}
}

static char *a2mpreason2str(uint16_t reason)
{
	switch (reason) {
	case A2MP_COMMAND_NOT_RECOGNIZED:
		return "Command not recognized";
	default:
		return "Reserved";
	}
}

static char *connresult2str(uint16_t result)
{
	switch (result) {
	case 0x0000:
		return "Connection successful";
	case 0x0001:
		return "Connection pending";
	case 0x0002:
		return "Connection refused - PSM not supported";
	case 0x0003:
		return "Connection refused - security block";
	case 0x0004:
		return "Connection refused - no resources available";
	default:
		return "Reserved";
	}
}

static char *status2str(uint16_t status)
{
	switch (status) {
	case 0x0000:
		return "No futher information available";
	case 0x0001:
		return "Authentication pending";
	case 0x0002:
		return "Authorization pending";
	default:
		return "Reserved";
	}
}

static char *confresult2str(uint16_t result)
{
	switch (result) {
	case L2CAP_CONF_SUCCESS:
		return "Success";
	case L2CAP_CONF_UNACCEPT:
		return "Failure - unacceptable parameters";
	case L2CAP_CONF_REJECT:
		return "Failure - rejected (no reason provided)";
	case L2CAP_CONF_UNKNOWN:
		return "Failure - unknown options";
	case L2CAP_CONF_PENDING:
		return "Pending";
	case L2CAP_CONF_EFS_REJECT:
		return "Failure - flowspec reject";
	default:
		return "Reserved";
	}
}
static char *inforesult2str(uint16_t result)
{
	switch (result) {
	case 0x0000:
		return "Success";
	case 0x0001:
		return "Not supported";
	default:
		return "Reserved";
	}
}

static char *type2str(uint8_t type)
{
	switch (type) {
	case L2CAP_SERVTYPE_NOTRAFFIC:
		return "No traffic";
	case L2CAP_SERVTYPE_BESTEFFORT:
		return "Best Effort";
	case L2CAP_SERVTYPE_GUARANTEED:
		return "Guaranteed";
	default:
		return "Reserved";
	}
}

static char *mode2str(uint8_t mode)
{
	switch (mode) {
	case 0x00:
		return "Basic";
	case 0x01:
		return "Retransmission";
	case 0x02:
		return "Flow control";
	case 0x03:
		return "Enhanced Retransmission";
	case 0x04:
		return "Streaming";
	default:
		return "Reserved";
	}
}

static char *fcs2str(uint8_t fcs)
{
	switch (fcs) {
	case 0x00:
		return "No FCS";
	case 0x01:
		return "CRC16 Check";
	default:
		return "Reserved";
	}
}

static char *sar2str(uint8_t sar)
{
	switch (sar) {
	case L2CAP_SAR_UNSEGMENTED:
		return "Unsegmented";
	case L2CAP_SAR_START:
		return "Start";
	case L2CAP_SAR_END:
		return "End";
	case L2CAP_SAR_CONTINUE:
		return "Continuation";
	default:
		return "Bad SAR";

	}
}

static char *supervisory2str(uint8_t supervisory)
{
	switch (supervisory) {
	case L2CAP_SUPER_RR:
		return "Receiver Ready (RR)";
	case L2CAP_SUPER_REJ:
		return "Reject (REJ)";
	case L2CAP_SUPER_RNR:
		return "Receiver Not Ready (RNR)";
	case L2CAP_SUPER_SREJ:
		return "Select Reject (SREJ)";
	default:
		return "Bad Supervisory";
	}
}

static char *ampctrltype2str(uint8_t type)
{
	switch (type) {
	case HCI_BREDR:
		return "BR-EDR";
	case HCI_AMP:
		return "802.11 AMP";
	default:
		return "Reserved";
	}
}

static char *ampctrlstatus2str(uint8_t status)
{
	switch (status) {
	case AMP_CTRL_POWERED_DOWN:
		return "Powered down";
	case AMP_CTRL_BLUETOOTH_ONLY:
		return "Bluetooth only";
	case AMP_CTRL_NO_CAPACITY:
		return "No capacity";
	case AMP_CTRL_LOW_CAPACITY:
		return "Low capacity";
	case AMP_CTRL_MEDIUM_CAPACITY:
		return "Medium capacity";
	case AMP_CTRL_HIGH_CAPACITY:
		return "High capacity";
	case AMP_CTRL_FULL_CAPACITY:
		return "Full capacity";
	default:
		return "Reserved";

	}
}

static char *a2mpstatus2str(uint8_t status)
{
	switch (status) {
	case A2MP_STATUS_SUCCESS:
		return "Success";
	case A2MP_STATUS_INVALID_CTRL_ID:
		return "Invalid Controller ID";
	default:
		return "Reserved";
	}
}

static char *a2mpcplstatus2str(uint8_t status)
{
	switch (status) {
	case A2MP_STATUS_SUCCESS:
		return "Success";
	case A2MP_STATUS_INVALID_CTRL_ID:
		return "Invalid Controller ID";
	case A2MP_STATUS_UNABLE_START_LINK_CREATION:
		return "Failed - Unable to start link creation";
	case A2MP_STATUS_COLLISION_OCCURED:
		return "Failed - Collision occured";
	case A2MP_STATUS_DISCONN_REQ_RECVD:
		return "Failed - Disconnect physical link received";
	case A2MP_STATUS_PHYS_LINK_EXISTS:
		return "Failed - Physical link already exists";
	case A2MP_STATUS_SECURITY_VIOLATION:
		return "Failed - Security violation";
	default:
		return "Reserved";
	}
}

static char *a2mpdplstatus2str(uint8_t status)
{
	switch (status) {
	case A2MP_STATUS_SUCCESS:
		return "Success";
	case A2MP_STATUS_INVALID_CTRL_ID:
		return "Invalid Controller ID";
	case A2MP_STATUS_NO_PHYSICAL_LINK_EXISTS:
		return "Failed - No Physical Link exists";
	default:
		return "Reserved";
	}
}

static inline void command_rej(int level, struct frame *frm)
{
	l2cap_cmd_rej *h = frm->ptr;
	uint16_t reason = btohs(h->reason);
	uint32_t cid;

	printf("Command rej: reason %d", reason);

	switch (reason) {
	case 0x0001:
		printf(" mtu %d\n", get_val(frm->ptr + L2CAP_CMD_REJ_SIZE, 2));
		break;
	case 0x0002:
		cid = get_val(frm->ptr + L2CAP_CMD_REJ_SIZE, 4);
		printf(" dcid 0x%4.4x scid 0x%4.4x\n", cid & 0xffff, cid >> 16);
		break;
	default:
		printf("\n");
		break;
	}

	p_indent(level + 1, frm);
	printf("%s\n", reason2str(reason));
}

static inline void conn_req(int level, struct frame *frm)
{
	l2cap_conn_req *h = frm->ptr;
	uint16_t psm = btohs(h->psm);
	uint16_t scid = btohs(h->scid);

	add_cid(frm->in, frm->handle, scid, psm);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Connect req: psm %d scid 0x%4.4x\n", psm, scid);
}

static inline void conn_rsp(int level, struct frame *frm)
{
	l2cap_conn_rsp *h = frm->ptr;
	uint16_t scid = btohs(h->scid);
	uint16_t dcid = btohs(h->dcid);
	uint16_t result = btohs(h->result);
	uint16_t status = btohs(h->status);
	uint16_t psm;

	switch (h->result) {
	case L2CAP_CR_SUCCESS:
		if ((psm = get_psm(!frm->in, frm->handle, scid)))
			add_cid(frm->in, frm->handle, dcid, psm);
		break;

	case L2CAP_CR_PEND:
		break;

	default:
		del_cid(frm->in, dcid, scid);
		break;
	}

	if (p_filter(FILT_L2CAP))
		return;

	printf("Connect rsp: dcid 0x%4.4x scid 0x%4.4x result %d status %d\n",
		dcid, scid, result, status);

	p_indent(level + 1, frm);
	printf("%s", connresult2str(result));

	if (result == 0x0001)
		printf(" - %s\n", status2str(status));
	else
		printf("\n");
}

static void conf_rfc(void *ptr, int len, int in, uint16_t handle,
								uint16_t cid)
{
	uint8_t mode;

	mode = *((uint8_t *) ptr);
	set_mode(!in, handle, cid, mode);

	printf("RFC 0x%02x (%s", mode, mode2str(mode));
	if (mode >= 0x01 && mode <= 0x04) {
		uint8_t txwin, maxtrans;
		uint16_t rto, mto, mps;
		txwin = *((uint8_t *) (ptr + 1));
		maxtrans = *((uint8_t *) (ptr + 2));
		rto = get_le16(ptr + 3);
		mto = get_le16(ptr + 5);
		mps = get_le16(ptr + 7);
		printf(", TxWin %d, MaxTx %d, RTo %d, MTo %d, MPS %d",
					txwin, maxtrans, rto, mto, mps);
	}
	printf(")");
}

static void conf_efs(void *ptr)
{
	uint8_t id, ser_type;
	uint16_t max_sdu;
	uint32_t sdu_itime, access_lat, flush_to;

	id = get_val(ptr, sizeof(id));
	ser_type = get_val(ptr + 1, sizeof(ser_type));
	max_sdu = get_val(ptr + 2, sizeof(max_sdu));
	sdu_itime = get_val(ptr + 4, sizeof(sdu_itime));
	access_lat = get_val(ptr + 8, sizeof(access_lat));
	flush_to = get_val(ptr + 12, sizeof(flush_to));

	printf("EFS (Id 0x%02x, SerType %s, MaxSDU 0x%04x, SDUitime 0x%08x, "
			"AccLat 0x%08x, FlushTO 0x%08x)",
			id, type2str(ser_type), max_sdu, sdu_itime,
			access_lat, flush_to);
}

static void conf_fcs(void *ptr, int len)
{
	uint8_t fcs;

	fcs = *((uint8_t *) ptr);
	printf("FCS Option");
	if (len > 0)
		printf(" 0x%2.2x (%s)", fcs, fcs2str(fcs));
}

static void conf_opt(int level, void *ptr, int len, int in, uint16_t handle,
								uint16_t cid)
{
	int indent = 0;
	p_indent(level, 0);
	while (len > 0) {
		l2cap_conf_opt *h = ptr;

		ptr += L2CAP_CONF_OPT_SIZE + h->len;
		len -= L2CAP_CONF_OPT_SIZE + h->len;

		if (h->type & 0x80)
			printf("[");

		if (indent++) {
			printf("\n");
			p_indent(level, 0);
		}

		switch (h->type & 0x7f) {
		case L2CAP_CONF_MTU:
			set_mode(in, handle, cid, 0x00);
			printf("MTU");
			if (h->len > 0)
				printf(" %d", get_val(h->val, h->len));
			break;

		case L2CAP_CONF_FLUSH_TO:
			printf("FlushTO");
			if (h->len > 0)
				printf(" %d", get_val(h->val, h->len));
			break;

		case L2CAP_CONF_QOS:
			printf("QoS");
			if (h->len > 0)
				printf(" 0x%02x (%s)", *(h->val + 1), type2str(*(h->val + 1)));
			break;

		case L2CAP_CONF_RFC:
			conf_rfc(h->val, h->len, in, handle, cid);
			break;

		case L2CAP_CONF_FCS:
			conf_fcs(h->val, h->len);
			break;

		case L2CAP_CONF_EFS:
			conf_efs(h->val);
			break;

		case L2CAP_CONF_EWS:
			printf("EWS");
			if (h->len > 0)
				printf(" %d", get_val(h->val, h->len));
			set_ext_ctrl(in, handle, cid, 1);
			break;

		default:
			printf("Unknown (type %2.2x, len %d)", h->type & 0x7f, h->len);
			break;
		}

		if (h->type & 0x80)
			printf("] ");
		else
			printf(" ");
	}
	printf("\n");
}

static void conf_list(int level, uint8_t *list, int len)
{
	int i;

	p_indent(level, 0);
	for (i = 0; i < len; i++) {
		switch (list[i] & 0x7f) {
		case L2CAP_CONF_MTU:
			printf("MTU ");
			break;
		case L2CAP_CONF_FLUSH_TO:
			printf("FlushTo ");
			break;
		case L2CAP_CONF_QOS:
			printf("QoS ");
			break;
		case L2CAP_CONF_RFC:
			printf("RFC ");
			break;
		case L2CAP_CONF_FCS:
			printf("FCS ");
			break;
		case L2CAP_CONF_EFS:
			printf("EFS ");
			break;
		case L2CAP_CONF_EWS:
			printf("EWS ");
			break;
		default:
			printf("%2.2x ", list[i] & 0x7f);
			break;
		}
	}
	printf("\n");
}

static inline void conf_req(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_conf_req *h = frm->ptr;
	uint16_t dcid = btohs(h->dcid);
	int clen = btohs(cmd->len) - L2CAP_CONF_REQ_SIZE;

	if (p_filter(FILT_L2CAP))
		return;

	printf("Config req: dcid 0x%4.4x flags 0x%2.2x clen %d\n",
			dcid, btohs(h->flags), clen);

	if (clen > 0)
		conf_opt(level + 1, h->data, clen, frm->in, frm->handle,
									dcid);
}

static inline void conf_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_conf_rsp *h = frm->ptr;
	uint16_t scid = btohs(h->scid);
	uint16_t result = btohs(h->result);
	int clen = btohs(cmd->len) - L2CAP_CONF_RSP_SIZE;

	if (p_filter(FILT_L2CAP))
		return;

	printf("Config rsp: scid 0x%4.4x flags 0x%2.2x result %d clen %d\n",
			scid, btohs(h->flags), result, clen);

	if (clen > 0) {
		if (result) {
			p_indent(level + 1, frm);
			printf("%s\n", confresult2str(result));
		}
		if (result == 0x0003)
			conf_list(level + 1, h->data, clen);
		else
			conf_opt(level + 1, h->data, clen, frm->in,
							frm->handle, scid);
	} else {
		p_indent(level + 1, frm);
		printf("%s\n", confresult2str(result));
	}
}

static inline void disconn_req(int level, struct frame *frm)
{
	l2cap_disconn_req *h = frm->ptr;

	if (p_filter(FILT_L2CAP))
		return;

	printf("Disconn req: dcid 0x%4.4x scid 0x%4.4x\n",
			btohs(h->dcid), btohs(h->scid));
}

static inline void disconn_rsp(int level, struct frame *frm)
{
	l2cap_disconn_rsp *h = frm->ptr;
	uint16_t dcid = btohs(h->dcid);
	uint16_t scid = btohs(h->scid);

	del_cid(frm->in, dcid, scid);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Disconn rsp: dcid 0x%4.4x scid 0x%4.4x\n",
			btohs(h->dcid), btohs(h->scid));
}

static inline void echo_req(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	if (p_filter(FILT_L2CAP))
		return;

	printf("Echo req: dlen %d\n", btohs(cmd->len));
	raw_dump(level, frm);
}

static inline void echo_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	if (p_filter(FILT_L2CAP))
		return;

	printf("Echo rsp: dlen %d\n", btohs(cmd->len));
	raw_dump(level, frm);
}

static void info_opt(int level, int type, void *ptr, int len)
{
	uint32_t mask;
	uint64_t fc_mask;
	int i;

	p_indent(level, 0);

	switch (type) {
	case 0x0001:
		printf("Connectionless MTU %d\n", get_val(ptr, len));
		break;
	case 0x0002:
		mask = get_val(ptr, len);
		printf("Extended feature mask 0x%4.4x\n", mask);
		if (parser.flags & DUMP_VERBOSE)
			for (i=0; l2cap_features[i].name; i++)
				if (mask & l2cap_features[i].flag) {
					p_indent(level + 1, 0);
					printf("%s\n", l2cap_features[i].name);
				}
		break;
	case 0x0003:
		fc_mask = get_le64(ptr);
		printf("Fixed channel list 0x%8.8" PRIx64 "\n", fc_mask);
		if (parser.flags & DUMP_VERBOSE)
			for (i=0; l2cap_fix_chan[i].name; i++)
				if (fc_mask & l2cap_fix_chan[i].flag) {
					p_indent(level + 1, 0);
					printf("%s\n", l2cap_fix_chan[i].name);
				}
		break;
	default:
		printf("Unknown (len %d)\n", len);
		break;
	}
}

static inline void info_req(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_info_req *h = frm->ptr;

	if (p_filter(FILT_L2CAP))
		return;

	printf("Info req: type %d\n", btohs(h->type));
}

static inline void info_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_info_rsp *h = frm->ptr;
	uint16_t type = btohs(h->type);
	uint16_t result = btohs(h->result);
	int ilen = btohs(cmd->len) - L2CAP_INFO_RSP_SIZE;

	if (p_filter(FILT_L2CAP))
		return;

	printf("Info rsp: type %d result %d\n", type, result);

	if (ilen > 0) {
		info_opt(level + 1, type, h->data, ilen);
	} else {
		p_indent(level + 1, frm);
		printf("%s\n", inforesult2str(result));
	}
}

static void l2cap_ctrl_ext_parse(int level, struct frame *frm, uint32_t ctrl)
{
	p_indent(level, frm);

	printf("%s:", ctrl & L2CAP_EXT_CTRL_FRAME_TYPE ? "S-frame" : "I-frame");

	if (ctrl & L2CAP_EXT_CTRL_FRAME_TYPE) {
		printf(" %s", supervisory2str((ctrl & L2CAP_EXT_CTRL_SUPERVISE_MASK) >>
					L2CAP_EXT_CTRL_SUPER_SHIFT));

		if (ctrl & L2CAP_EXT_CTRL_POLL)
			printf(" P-bit");
	} else {
		uint8_t sar = (ctrl & L2CAP_EXT_CTRL_SAR_MASK) >>
			L2CAP_EXT_CTRL_SAR_SHIFT;
		printf(" %s", sar2str(sar));
		if (sar == L2CAP_SAR_START) {
			uint16_t len;
			len = get_le16(frm->ptr);
			frm->ptr += L2CAP_SDULEN_SIZE;
			frm->len -= L2CAP_SDULEN_SIZE;
			printf(" (len %d)", len);
		}
		printf(" TxSeq %d", (ctrl & L2CAP_EXT_CTRL_TXSEQ_MASK) >>
				L2CAP_EXT_CTRL_TXSEQ_SHIFT);
	}

	printf(" ReqSeq %d", (ctrl & L2CAP_EXT_CTRL_REQSEQ_MASK) >>
			L2CAP_EXT_CTRL_REQSEQ_SHIFT);

	if (ctrl & L2CAP_EXT_CTRL_FINAL)
		printf(" F-bit");
}

static void l2cap_ctrl_parse(int level, struct frame *frm, uint32_t ctrl)
{
	p_indent(level, frm);

	printf("%s:", ctrl & L2CAP_CTRL_FRAME_TYPE ? "S-frame" : "I-frame");

	if (ctrl & 0x01) {
		printf(" %s", supervisory2str((ctrl & L2CAP_CTRL_SUPERVISE_MASK) >>
					L2CAP_CTRL_SUPER_SHIFT));

		if (ctrl & L2CAP_CTRL_POLL)
			printf(" P-bit");
	} else {
		uint8_t sar = (ctrl & L2CAP_CTRL_SAR_MASK) >> L2CAP_CTRL_SAR_SHIFT;
		printf(" %s", sar2str(sar));
		if (sar == L2CAP_SAR_START) {
			uint16_t len;
			len = get_le16(frm->ptr);
			frm->ptr += L2CAP_SDULEN_SIZE;
			frm->len -= L2CAP_SDULEN_SIZE;
			printf(" (len %d)", len);
		}
		printf(" TxSeq %d", (ctrl & L2CAP_CTRL_TXSEQ_MASK) >> L2CAP_CTRL_TXSEQ_SHIFT);
	}

	printf(" ReqSeq %d", (ctrl & L2CAP_CTRL_REQSEQ_MASK) >> L2CAP_CTRL_REQSEQ_SHIFT);

	if (ctrl & L2CAP_CTRL_FINAL)
		printf(" F-bit");
}

static inline void create_req(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_create_req *h = frm->ptr;
	uint16_t psm = btohs(h->psm);
	uint16_t scid = btohs(h->scid);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Create chan req: psm 0x%4.4x scid 0x%4.4x ctrl id %d\n",
							psm, scid, h->id);
}

static inline void create_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_create_rsp *h = frm->ptr;
	uint16_t scid = btohs(h->scid);
	uint16_t dcid = btohs(h->dcid);
	uint16_t result = btohs(h->result);
	uint16_t status = btohs(h->status);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Create chan rsp: dcid 0x%4.4x scid 0x%4.4x result %d status %d\n",
						dcid, scid, result, status);
}

static inline void move_req(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_move_req *h = frm->ptr;
	uint16_t icid = btohs(h->icid);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Move chan req: icid 0x%4.4x ctrl id %d\n", icid, h->id);
}

static inline void move_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_move_rsp *h = frm->ptr;
	uint16_t icid = btohs(h->icid);
	uint16_t result = btohs(h->result);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Move chan rsp: icid 0x%4.4x result %d\n", icid, result);
}

static inline void move_cfm(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_move_cfm *h = frm->ptr;
	uint16_t icid = btohs(h->icid);
	uint16_t result = btohs(h->result);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Move chan cfm: icid 0x%4.4x result %d\n", icid, result);
}

static inline void move_cfm_rsp(int level, l2cap_cmd_hdr *cmd, struct frame *frm)
{
	l2cap_move_cfm_rsp *h = frm->ptr;
	uint16_t icid = btohs(h->icid);

	if (p_filter(FILT_L2CAP))
		return;

	printf("Move chan cfm rsp: icid 0x%4.4x\n", icid);
}

static inline void a2mp_command_rej(int level, struct frame *frm)
{
	struct a2mp_command_rej *h = frm->ptr;
	uint16_t reason = btohs(h->reason);

	printf("Command Reject: reason %d\n", reason);
	p_indent(level + 1, 0);
	printf("%s\n", a2mpreason2str(reason));
}

static inline void a2mp_discover_req(int level, struct frame *frm, uint16_t len)
{
	struct a2mp_discover_req *h = frm->ptr;
	uint16_t mtu = btohs(h->mtu);
	uint8_t	 *octet = (uint8_t *)&(h->mask);
	uint16_t mask;
	uint8_t  extension;

	printf("Discover req: mtu/mps %d ", mtu);
	len -= 2;

	printf("mask:");

	do {
		len -= 2;
		mask = get_le16(octet);
		printf(" 0x%4.4x", mask);

		extension = octet[1] & 0x80;
		octet += 2;
	} while ((extension != 0) && (len >= 2));

	printf("\n");
}

static inline void a2mp_ctrl_list_dump(int level, struct a2mp_ctrl *list, uint16_t len)
{
	p_indent(level, 0);
	printf("Controller list:\n");

	while (len >= 3) {
		p_indent(level + 1, 0);
		printf("id %d type %d (%s) status 0x%2.2x (%s)\n",
			   list->id, list->type, ampctrltype2str(list->type), list->status, ampctrlstatus2str(list->status));
		list++;
		len -= 3;
	}

}

static inline void a2mp_discover_rsp(int level, struct frame *frm, uint16_t len)
{
	struct a2mp_discover_rsp *h = frm->ptr;
	uint16_t mtu = btohs(h->mtu);
	uint8_t	 *octet = (uint8_t *)&(h->mask);
	uint16_t mask;
	uint8_t  extension;

	printf("Discover rsp: mtu/mps %d ", mtu);
	len -= 2;

	printf("mask:");

	do {
		len -= 2;
		mask = get_le16(octet);
		printf(" 0x%4.4x", mask);

		extension = octet[1] & 0x80;
		octet += 2;
	} while ((extension != 0) && (len >= 2));

	printf("\n");

	if (len >= 3) {
		a2mp_ctrl_list_dump(level + 1, (struct a2mp_ctrl *) octet, len);
	}
}

static inline void a2mp_change_notify(int level, struct frame *frm, uint16_t len)
{
	struct a2mp_ctrl *list = frm->ptr;

	printf("Change Notify\n");

	if (len >= 3) {
		a2mp_ctrl_list_dump(level + 1, list, len);
	}
}

static inline void a2mp_change_rsp(int level, struct frame *frm)
{
	printf("Change Response\n");
}

static inline void a2mp_info_req(int level, struct frame *frm)
{
	struct a2mp_info_req *h = frm->ptr;

	printf("Get Info req: id %d\n", h->id);
}

static inline void a2mp_info_rsp(int level, struct frame *frm)
{
	struct a2mp_info_rsp *h = frm->ptr;

	printf("Get Info rsp: id %d status %d (%s)\n",
		   h->id, h->status, a2mpstatus2str(h->status));

	p_indent(level + 1, 0);
	printf("Total bandwidth %d\n", btohl(h->total_bw));
	p_indent(level + 1, 0);
	printf("Max guaranteed bandwidth %d\n", btohl(h->max_bw));
	p_indent(level + 1, 0);
	printf("Min latency %d\n", btohl(h->min_latency));
	p_indent(level + 1, 0);
	printf("Pal capabilities 0x%4.4x\n", btohs(h->pal_caps));
	p_indent(level + 1, 0);
	printf("Assoc size %d\n", btohs(h->assoc_size));
}

static inline void a2mp_assoc_req(int level, struct frame *frm)
{
	struct a2mp_assoc_req *h = frm->ptr;

	printf("Get AMP Assoc req: id %d\n", h->id);
}

static inline void a2mp_assoc_rsp(int level, struct frame *frm, uint16_t len)
{
	struct a2mp_assoc_rsp *h = frm->ptr;

	printf("Get AMP Assoc rsp: id %d status (%d) %s\n",
			h->id, h->status, a2mpstatus2str(h->status));
	amp_assoc_dump(level + 1, h->assoc_data, len - sizeof(*h));
}

static inline void a2mp_create_req(int level, struct frame *frm, uint16_t len)
{
	struct a2mp_create_req *h = frm->ptr;

	printf("Create Physical Link req: local id %d remote id %d\n",
		   h->local_id, h->remote_id);
	amp_assoc_dump(level + 1, h->assoc_data, len - sizeof(*h));
}

static inline void a2mp_create_rsp(int level, struct frame *frm)
{
	struct a2mp_create_rsp *h = frm->ptr;

	printf("Create Physical Link rsp: local id %d remote id %d status %d\n",
		   h->local_id, h->remote_id, h->status);
	p_indent(level+1, 0);
	printf("%s\n", a2mpcplstatus2str(h->status));
}

static inline void a2mp_disconn_req(int level, struct frame *frm)
{
	struct a2mp_disconn_req *h = frm->ptr;

	printf("Disconnect Physical Link req: local id %d remote id %d\n",
		   h->local_id, h->remote_id);
}

static inline void a2mp_disconn_rsp(int level, struct frame *frm)
{
	struct a2mp_disconn_rsp *h = frm->ptr;

	printf("Disconnect Physical Link rsp: local id %d remote id %d status %d\n",
		   h->local_id, h->remote_id, h->status);
	p_indent(level+1, 0);
	printf("%s\n", a2mpdplstatus2str(h->status));
}

static void l2cap_parse(int level, struct frame *frm)
{
	l2cap_hdr *hdr = (void *)frm->ptr;
	uint16_t dlen = btohs(hdr->len);
	uint16_t cid  = btohs(hdr->cid);
	uint16_t psm;

	frm->ptr += L2CAP_HDR_SIZE;
	frm->len -= L2CAP_HDR_SIZE;

	if (cid == 0x1) {
		/* Signaling channel */

		while (frm->len >= L2CAP_CMD_HDR_SIZE) {
			l2cap_cmd_hdr *hdr = frm->ptr;

			frm->ptr += L2CAP_CMD_HDR_SIZE;
			frm->len -= L2CAP_CMD_HDR_SIZE;

			if (!p_filter(FILT_L2CAP)) {
				p_indent(level, frm);
				printf("L2CAP(s): ");
			}

			switch (hdr->code) {
			case L2CAP_COMMAND_REJ:
				command_rej(level, frm);
				break;

			case L2CAP_CONN_REQ:
				conn_req(level, frm);
				break;

			case L2CAP_CONN_RSP:
				conn_rsp(level, frm);
				break;

			case L2CAP_CONF_REQ:
				conf_req(level, hdr, frm);
				break;

			case L2CAP_CONF_RSP:
				conf_rsp(level, hdr, frm);
				break;

			case L2CAP_DISCONN_REQ:
				disconn_req(level, frm);
				break;

			case L2CAP_DISCONN_RSP:
				disconn_rsp(level, frm);
				break;

			case L2CAP_ECHO_REQ:
				echo_req(level, hdr, frm);
				break;

			case L2CAP_ECHO_RSP:
				echo_rsp(level, hdr, frm);
				break;

			case L2CAP_INFO_REQ:
				info_req(level, hdr, frm);
				break;

			case L2CAP_INFO_RSP:
				info_rsp(level, hdr, frm);
				break;

			case L2CAP_CREATE_REQ:
				create_req(level, hdr, frm);
				break;

			case L2CAP_CREATE_RSP:
				create_rsp(level, hdr, frm);
				break;

			case L2CAP_MOVE_REQ:
				move_req(level, hdr, frm);
				break;

			case L2CAP_MOVE_RSP:
				move_rsp(level, hdr, frm);
				break;

			case L2CAP_MOVE_CFM:
				move_cfm(level, hdr, frm);
				break;

			case L2CAP_MOVE_CFM_RSP:
				move_cfm_rsp(level, hdr, frm);
				break;

			default:
				if (p_filter(FILT_L2CAP))
					break;
				printf("code 0x%2.2x ident %d len %d\n", 
					hdr->code, hdr->ident, btohs(hdr->len));
				raw_dump(level, frm);
			}

			if (frm->len > btohs(hdr->len)) {
				frm->len -= btohs(hdr->len);
				frm->ptr += btohs(hdr->len);
			} else
				frm->len = 0;
		}
	} else if (cid == 0x2) {
		/* Connectionless channel */

		if (p_filter(FILT_L2CAP))
			return;

		psm = get_le16(frm->ptr);
		frm->ptr += 2;
		frm->len -= 2;

		p_indent(level, frm);
		printf("L2CAP(c): len %d psm %d\n", dlen, psm);
		raw_dump(level, frm);
	} else if (cid == 0x3) {
		/* AMP Manager channel */

		if (p_filter(FILT_A2MP))
			return;

		/* Adjust for ERTM control bytes */
		frm->ptr += 2;
		frm->len -= 2;

		while (frm->len >= A2MP_HDR_SIZE) {
			struct a2mp_hdr *hdr = frm->ptr;

			frm->ptr += A2MP_HDR_SIZE;
			frm->len -= A2MP_HDR_SIZE;

			p_indent(level, frm);
			printf("A2MP: ");

			switch (hdr->code) {
			case A2MP_COMMAND_REJ:
				a2mp_command_rej(level, frm);
				break;
			case A2MP_DISCOVER_REQ:
				a2mp_discover_req(level, frm, btohs(hdr->len));
				break;
			case A2MP_DISCOVER_RSP:
				a2mp_discover_rsp(level, frm, btohs(hdr->len));
				break;
			case A2MP_CHANGE_NOTIFY:
				a2mp_change_notify(level, frm, btohs(hdr->len));
				break;
			case A2MP_CHANGE_RSP:
				a2mp_change_rsp(level, frm);
				break;
			case A2MP_INFO_REQ:
				a2mp_info_req(level, frm);
				break;
			case A2MP_INFO_RSP:
				a2mp_info_rsp(level, frm);
				break;
			case A2MP_ASSOC_REQ:
				a2mp_assoc_req(level, frm);
				break;
			case A2MP_ASSOC_RSP:
				a2mp_assoc_rsp(level, frm, btohs(hdr->len));
				break;
			case A2MP_CREATE_REQ:
				a2mp_create_req(level, frm, btohs(hdr->len));
				break;
			case A2MP_CREATE_RSP:
				a2mp_create_rsp(level, frm);
				break;
			case A2MP_DISCONN_REQ:
				a2mp_disconn_req(level, frm);
				break;
			case A2MP_DISCONN_RSP:
				a2mp_disconn_rsp(level, frm);
				break;
			default:
				printf("code 0x%2.2x ident %d len %d\n",
					   hdr->code, hdr->ident, btohs(hdr->len));
				raw_dump(level, frm);
			}
			if (frm->len > btohs(hdr->len)) {
				frm->len -= btohs(hdr->len);
				frm->ptr += btohs(hdr->len);
			} else
				frm->len = 0;
		}
	} else if (cid == 0x04) {
		if (!p_filter(FILT_ATT))
			att_dump(level, frm);
		else
			raw_dump(level + 1, frm);
	} else if (cid == 0x06) {
		if (!p_filter(FILT_SMP))
			smp_dump(level, frm);
		else
			raw_dump(level + 1, frm);
	} else {
		/* Connection oriented channel */

		uint8_t mode = get_mode(!frm->in, frm->handle, cid);
		uint8_t ext_ctrl = get_ext_ctrl(!frm->in, frm->handle, cid);
		uint16_t psm = get_psm(!frm->in, frm->handle, cid);
		uint16_t fcs = 0;
		uint32_t proto, ctrl = 0;

		frm->cid = cid;
		frm->num = get_num(!frm->in, frm->handle, cid);

		if (mode > 0) {
			if (ext_ctrl) {
				ctrl = get_val(frm->ptr, 4);
				frm->ptr += 4;
				frm->len -= 6;
			} else {
				ctrl = get_val(frm->ptr, 2);
				frm->ptr += 2;
				frm->len -= 4;
			}
			fcs = get_le16(frm->ptr + frm->len);
		}

		if (!p_filter(FILT_L2CAP)) {
			p_indent(level, frm);
			printf("L2CAP(d): cid 0x%4.4x len %d", cid, dlen);
			if (mode > 0) {
				if (ext_ctrl)
					printf(" ext_ctrl 0x%8.8x fcs 0x%4.4x", ctrl, fcs);
				else
					printf(" ctrl 0x%4.4x fcs 0x%4.4x", ctrl, fcs);
			}

			printf(" [psm %d]\n", psm);
			level++;
			if (mode > 0) {
				if (ext_ctrl)
					l2cap_ctrl_ext_parse(level, frm, ctrl);
				else
					l2cap_ctrl_parse(level, frm, ctrl);

				printf("\n");
			}
		}

		switch (psm) {
		case 0x01:
			if (!p_filter(FILT_SDP))
				sdp_dump(level + 1, frm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x03:
			if (!p_filter(FILT_RFCOMM))
				rfcomm_dump(level, frm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x0f:
			if (!p_filter(FILT_BNEP))
				bnep_dump(level, frm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x11:
		case 0x13:
			if (!p_filter(FILT_HIDP))
				hidp_dump(level, frm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x17:
		case 0x1B:
			if (!p_filter(FILT_AVCTP))
				avctp_dump(level, frm, psm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x19:
			if (!p_filter(FILT_AVDTP))
				avdtp_dump(level, frm);
			else
				raw_dump(level + 1, frm);
			break;

		case 0x1f:
			if (!p_filter(FILT_ATT))
				att_dump(level, frm);
			else
				raw_dump(level + 1, frm);
			break;

		default:
			proto = get_proto(frm->handle, psm, 0);

			switch (proto) {
			case SDP_UUID_CMTP:
				if (!p_filter(FILT_CMTP))
					cmtp_dump(level, frm);
				else
					raw_dump(level + 1, frm);
				break;

			case SDP_UUID_HARDCOPY_CONTROL_CHANNEL:
				if (!p_filter(FILT_HCRP))
					hcrp_dump(level, frm);
				else
					raw_dump(level + 1, frm);
				break;

			case SDP_UUID_OBEX:
				if (!p_filter(FILT_OBEX))
					obex_dump(level, frm);
				else
					raw_dump(level + 1, frm);
				break;

			default:
				if (p_filter(FILT_L2CAP))
					break;

				raw_dump(level, frm);
				break;
			}
			break;
		}
	}
}

void l2cap_dump(int level, struct frame *frm)
{
	struct frame *fr;
	l2cap_hdr *hdr;
	uint16_t dlen;

	if ((frm->flags & ACL_START) || frm->flags == ACL_START_NO_FLUSH) {
		hdr  = frm->ptr;
		dlen = btohs(hdr->len);

		if (dlen + L2CAP_HDR_SIZE < (int) frm->len) {
			/* invalid frame */
			raw_dump(level,frm);
			return;
		}

		if ((int) frm->len == (dlen + L2CAP_HDR_SIZE)) {
			/* Complete frame */
			l2cap_parse(level, frm);
			return;
		}

		if (!(fr = get_frame(frm->handle))) {
			fprintf(stderr, "Not enough connection handles\n");
			raw_dump(level, frm);
			return;
		}

		if (fr->data)
			free(fr->data);

		if (!(fr->data = malloc(dlen + L2CAP_HDR_SIZE))) {
			perror("Can't allocate L2CAP reassembly buffer");
			return;
		}
		memcpy(fr->data, frm->ptr, frm->len);
		fr->data_len   = dlen + L2CAP_HDR_SIZE;
		fr->len        = frm->len;
		fr->ptr        = fr->data;
		fr->dev_id     = frm->dev_id;
		fr->in         = frm->in;
		fr->ts         = frm->ts;
		fr->handle     = frm->handle;
		fr->cid        = frm->cid;
		fr->num        = frm->num;
		fr->dlci       = frm->dlci;
		fr->channel    = frm->channel;
		fr->pppdump_fd = frm->pppdump_fd;
		fr->audio_fd   = frm->audio_fd;
	} else {
		if (!(fr = get_frame(frm->handle))) {
			fprintf(stderr, "Not enough connection handles\n");
			raw_dump(level, frm);
			return;
		}

		if (!fr->data) {
			/* Unexpected fragment */
			raw_dump(level, frm);
			return;
		}

		if (frm->len > (fr->data_len - fr->len)) {
			/* Bad fragment */
			raw_dump(level, frm);
			free(fr->data); fr->data = NULL;
			return;
		}

		memcpy(fr->data + fr->len, frm->ptr, frm->len);
		fr->len += frm->len;

		if (fr->len == fr->data_len) {
			/* Complete frame */
			l2cap_parse(level, fr);

			free(fr->data); fr->data = NULL;
			return;
		}
	}
}

void l2cap_clear(uint16_t handle)
{
	del_handle(handle);
}
