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

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <ell/ell.h>

#include "monitor/bt.h"
#include "src/shared/hci.h"
#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "mesh/mesh-defs.h"
#include "mesh/mesh-mgmt.h"
#include "mesh/mesh-io.h"
#include "mesh/mesh-io-api.h"
#include "mesh/mesh-io-generic.h"

struct mesh_io_private {
	struct bt_hci *hci;
	void *user_data;
	mesh_io_ready_func_t ready_callback;
	struct l_timeout *tx_timeout;
	struct l_queue *rx_regs;
	struct l_queue *tx_pkts;
	struct tx_pkt *tx;
	uint16_t index;
	uint16_t interval;
	bool sending;
	bool active;
};

struct pvt_rx_reg {
	mesh_io_recv_func_t cb;
	void *user_data;
	uint8_t len;
	uint8_t filter[0];
};

struct process_data {
	struct mesh_io_private		*pvt;
	const uint8_t			*data;
	uint8_t				len;
	struct mesh_io_recv_info	info;
};

struct tx_pkt {
	struct mesh_io_send_info	info;
	bool				delete;
	uint8_t				len;
	uint8_t				pkt[30];
};

struct tx_pattern {
	const uint8_t			*data;
	uint8_t				len;
};

static uint32_t get_instant(void)
{
	struct timeval tm;
	uint32_t instant;

	gettimeofday(&tm, NULL);
	instant = tm.tv_sec * 1000;
	instant += tm.tv_usec / 1000;

	return instant;
}

static uint32_t instant_remaining_ms(uint32_t instant)
{
	instant -= get_instant();
	return instant;
}

static void process_rx_callbacks(void *v_reg, void *v_rx)
{
	struct pvt_rx_reg *rx_reg = v_reg;
	struct process_data *rx = v_rx;

	if (!memcmp(rx->data, rx_reg->filter, rx_reg->len))
		rx_reg->cb(rx_reg->user_data, &rx->info, rx->data, rx->len);
}

static void process_rx(struct mesh_io_private *pvt, int8_t rssi,
					uint32_t instant, const uint8_t *addr,
					const uint8_t *data, uint8_t len)
{
	struct process_data rx = {
		.pvt = pvt,
		.data = data,
		.len = len,
		.info.instant = instant,
		.info.addr = addr,
		.info.chan = 7,
		.info.rssi = rssi,
	};

	l_queue_foreach(pvt->rx_regs, process_rx_callbacks, &rx);
}

static void event_adv_report(struct mesh_io *io, const void *buf, uint8_t size)
{
	const struct bt_hci_evt_le_adv_report *evt = buf;
	const uint8_t *adv;
	const uint8_t *addr;
	uint32_t instant;
	uint8_t adv_len;
	uint16_t len = 0;
	int8_t rssi;

	if (evt->event_type != 0x03)
		return;

	instant = get_instant();
	adv = evt->data;
	adv_len = evt->data_len;
	addr = evt->addr;

	/* rssi is just beyond last byte of data */
	rssi = (int8_t) adv[adv_len];

	while (len < adv_len - 1) {
		uint8_t field_len = adv[0];

		/* Check for the end of advertising data */
		if (field_len == 0)
			break;

		len += field_len + 1;

		/* Do not continue data parsing if got incorrect length */
		if (len > adv_len)
			break;

		/* TODO: Create an Instant to use */
		process_rx(io->pvt, rssi, instant, addr, adv + 1, adv[0]);

		adv += field_len + 1;
	}
}

static void event_callback(const void *buf, uint8_t size, void *user_data)
{
	uint8_t event = l_get_u8(buf);
	struct mesh_io *io = user_data;

	switch (event) {
	case BT_HCI_EVT_LE_ADV_REPORT:
		event_adv_report(io, buf + 1, size - 1);
		break;

	default:
		l_debug("Other Meta Evt - %d", event);
	}
}

static void local_commands_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_commands *rsp = data;

	if (rsp->status)
		l_error("Failed to read local commands");
}

static void local_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;

	if (rsp->status)
		l_error("Failed to read local features");
}

static void hci_generic_callback(const void *data, uint8_t size,
								void *user_data)
{
	uint8_t status = l_get_u8(data);

	if (status)
		l_error("Failed to initialize HCI");
}

static void configure_hci(struct mesh_io_private *io)
{
	struct bt_hci_cmd_le_set_scan_parameters cmd;
	struct bt_hci_cmd_set_event_mask cmd_sem;
	struct bt_hci_cmd_le_set_event_mask cmd_slem;
	struct bt_hci_cmd_le_set_random_address cmd_raddr;

	/* Set scan parameters */
	cmd.type = 0x00; /* Passive Scanning. No scanning PDUs shall be sent */
	cmd.interval = 0x0030; /* Scan Interval = N * 0.625ms */
	cmd.window = 0x0030; /* Scan Window = N * 0.625ms */
	cmd.own_addr_type = 0x00; /* Public Device Address */
	/* Accept all advertising packets except directed advertising packets
	 * not addressed to this device (default).
	 */
	cmd.filter_policy = 0x00;

	/* Set event mask
	 *
	 * Mask: 0x2000800002008890
	 *   Disconnection Complete
	 *   Encryption Change
	 *   Read Remote Version Information Complete
	 *   Hardware Error
	 *   Data Buffer Overflow
	 *   Encryption Key Refresh Complete
	 *   LE Meta
	 */
	cmd_sem.mask[0] = 0x90;
	cmd_sem.mask[1] = 0x88;
	cmd_sem.mask[2] = 0x00;
	cmd_sem.mask[3] = 0x02;
	cmd_sem.mask[4] = 0x00;
	cmd_sem.mask[5] = 0x80;
	cmd_sem.mask[6] = 0x00;
	cmd_sem.mask[7] = 0x20;

	/* Set LE event mask
	 *
	 * Mask: 0x000000000000087f
	 *   LE Connection Complete
	 *   LE Advertising Report
	 *   LE Connection Update Complete
	 *   LE Read Remote Used Features Complete
	 *   LE Long Term Key Request
	 *   LE Remote Connection Parameter Request
	 *   LE Data Length Change
	 *   LE PHY Update Complete
	 */
	cmd_slem.mask[0] = 0x7f;
	cmd_slem.mask[1] = 0x08;
	cmd_slem.mask[2] = 0x00;
	cmd_slem.mask[3] = 0x00;
	cmd_slem.mask[4] = 0x00;
	cmd_slem.mask[5] = 0x00;
	cmd_slem.mask[6] = 0x00;
	cmd_slem.mask[7] = 0x00;

	/* Set LE random address */
	l_getrandom(cmd_raddr.addr, 6);
	cmd_raddr.addr[5] |= 0xc0;

	/* TODO: Move to suitable place. Set suitable masks */
	/* Reset Command */
	bt_hci_send(io->hci, BT_HCI_CMD_RESET, NULL, 0, hci_generic_callback,
								NULL, NULL);

	/* Read local supported commands */
	bt_hci_send(io->hci, BT_HCI_CMD_READ_LOCAL_COMMANDS, NULL, 0,
					local_commands_callback, NULL, NULL);

	/* Read local supported features */
	bt_hci_send(io->hci, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					local_features_callback, NULL, NULL);

	/* Set event mask */
	bt_hci_send(io->hci, BT_HCI_CMD_SET_EVENT_MASK, &cmd_sem,
			sizeof(cmd_sem), hci_generic_callback, NULL, NULL);

	/* Set LE event mask */
	bt_hci_send(io->hci, BT_HCI_CMD_LE_SET_EVENT_MASK, &cmd_slem,
			sizeof(cmd_slem), hci_generic_callback, NULL, NULL);

	/* Set LE random address */
	bt_hci_send(io->hci, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS, &cmd_raddr,
			sizeof(cmd_raddr), hci_generic_callback, NULL, NULL);

	/* Scan Params */
	bt_hci_send(io->hci, BT_HCI_CMD_LE_SET_SCAN_PARAMETERS, &cmd,
				sizeof(cmd), hci_generic_callback, NULL, NULL);
}

static void scan_enable_rsp(const void *buf, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) buf);

	if (status)
		l_error("LE Scan enable failed (0x%02x)", status);
}

static void set_recv_scan_enable(const void *buf, uint8_t size,
							void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct bt_hci_cmd_le_set_scan_enable cmd;

	cmd.enable = 0x01;	/* Enable scanning */
	cmd.filter_dup = 0x00;	/* Report duplicates */
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
			&cmd, sizeof(cmd), scan_enable_rsp, pvt, NULL);
}

static void scan_disable_rsp(const void *buf, uint8_t size,
							void *user_data)
{
	struct bt_hci_cmd_le_set_scan_parameters cmd;
	struct mesh_io_private *pvt = user_data;
	uint8_t status = *((uint8_t *) buf);

	if (status)
		l_error("LE Scan disable failed (0x%02x)", status);

	cmd.type = pvt->active ? 0x01 : 0x00;	/* Passive/Active scanning */
	cmd.interval = L_CPU_TO_LE16(0x0010);	/* 10 ms */
	cmd.window = L_CPU_TO_LE16(0x0010);	/* 10 ms */
	cmd.own_addr_type = 0x01;		/* ADDR_TYPE_RANDOM */
	cmd.filter_policy = 0x00;		/* Accept all */

	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_PARAMETERS,
			&cmd, sizeof(cmd),
			set_recv_scan_enable, pvt, NULL);
}

static bool simple_match(const void *a, const void *b)
{
	return a == b;
}

static bool find_by_ad_type(const void *a, const void *b)
{
	const struct tx_pkt *tx = a;
	uint8_t ad_type = L_PTR_TO_UINT(b);

	return !ad_type || ad_type == tx->pkt[0];
}

static bool find_by_pattern(const void *a, const void *b)
{
	const struct tx_pkt *tx = a;
	const struct tx_pattern *pattern = b;

	if (tx->len < pattern->len)
		return false;

	return (!memcmp(tx->pkt, pattern->data, pattern->len));
}

static bool find_active(const void *a, const void *b)
{
	const struct pvt_rx_reg *rx_reg = a;

	/* Mesh specific AD types do *not* require active scanning,
	 * so do not turn on Active Scanning on their account.
	 */
	if (rx_reg->filter[0] < MESH_AD_TYPE_PROVISION ||
			rx_reg->filter[0] > MESH_AD_TYPE_BEACON)
		return true;

	return false;
}

static void restart_scan(struct mesh_io_private *pvt)
{
	struct bt_hci_cmd_le_set_scan_enable cmd;

	if (l_queue_isempty(pvt->rx_regs))
		return;

	pvt->active = l_queue_find(pvt->rx_regs, find_active, NULL);
	cmd.enable = 0x00;	/* Disable scanning */
	cmd.filter_dup = 0x00;	/* Report duplicates */
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
				&cmd, sizeof(cmd), scan_disable_rsp, pvt, NULL);
}

static void hci_init(void *user_data)
{
	struct mesh_io *io = user_data;
	bool result = true;
	bool restarted = false;

	if (io->pvt->hci) {
		restarted = true;
		bt_hci_unref(io->pvt->hci);
	}

	io->pvt->hci = bt_hci_new_user_channel(io->pvt->index);
	if (!io->pvt->hci) {
		l_error("Failed to start mesh io (hci %u): %s", io->pvt->index,
							strerror(errno));
		result = false;
	}

	if (result) {
		configure_hci(io->pvt);

		bt_hci_register(io->pvt->hci, BT_HCI_EVT_LE_META_EVENT,
						event_callback, io, NULL);

		l_debug("Started mesh on hci %u", io->pvt->index);

		if (restarted)
			restart_scan(io->pvt);
	}

	if (io->pvt->ready_callback)
		io->pvt->ready_callback(io->pvt->user_data, result);
}

static void read_info(int index, void *user_data)
{
	struct mesh_io *io = user_data;

	if (io->pvt->index != MGMT_INDEX_NONE &&
					index != io->pvt->index) {
		l_debug("Ignore index %d", index);
		return;
	}

	io->pvt->index = index;
	hci_init(io);
}

static bool dev_init(struct mesh_io *io, void *opts,
				mesh_io_ready_func_t cb, void *user_data)
{
	if (!io || io->pvt)
		return false;

	io->pvt = l_new(struct mesh_io_private, 1);
	io->pvt->index = *(int *)opts;

	io->pvt->rx_regs = l_queue_new();
	io->pvt->tx_pkts = l_queue_new();

	io->pvt->ready_callback = cb;
	io->pvt->user_data = user_data;

	if (io->pvt->index == MGMT_INDEX_NONE)
		return mesh_mgmt_list(read_info, io);

	l_idle_oneshot(hci_init, io, NULL);

	return true;
}

static bool dev_destroy(struct mesh_io *io)
{
	struct mesh_io_private *pvt = io->pvt;

	if (!pvt)
		return true;

	bt_hci_unref(pvt->hci);
	l_timeout_remove(pvt->tx_timeout);
	l_queue_destroy(pvt->rx_regs, l_free);
	l_queue_destroy(pvt->tx_pkts, l_free);
	l_free(pvt);
	io->pvt = NULL;

	return true;
}

static bool dev_caps(struct mesh_io *io, struct mesh_io_caps *caps)
{
	struct mesh_io_private *pvt = io->pvt;

	if (!pvt || !caps)
		return false;

	caps->max_num_filters = 255;
	caps->window_accuracy = 50;

	return true;
}

static void send_cancel_done(const void *buf, uint8_t size,
							void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct bt_hci_cmd_le_set_random_address cmd;

	if (!pvt)
		return;

	pvt->sending = false;

	/* At end of any burst of ADVs, change random address */
	l_getrandom(cmd.addr, 6);
	cmd.addr[5] |= 0xc0;
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS,
				&cmd, sizeof(cmd), NULL, NULL, NULL);
}

static void send_cancel(struct mesh_io_private *pvt)
{
	struct bt_hci_cmd_le_set_adv_enable cmd;

	if (!pvt)
		return;

	if (!pvt->sending) {
		send_cancel_done(NULL, 0, pvt);
		return;
	}

	cmd.enable = 0x00;	/* Disable advertising */
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_ADV_ENABLE,
				&cmd, sizeof(cmd),
				send_cancel_done, pvt, NULL);
}

static void set_send_adv_enable(const void *buf, uint8_t size,
							void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct bt_hci_cmd_le_set_adv_enable cmd;

	if (!pvt)
		return;

	pvt->sending = true;
	cmd.enable = 0x01;	/* Enable advertising */
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_ADV_ENABLE,
				&cmd, sizeof(cmd), NULL, NULL, NULL);
}

static void set_send_adv_data(const void *buf, uint8_t size,
							void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct tx_pkt *tx;
	struct bt_hci_cmd_le_set_adv_data cmd;

	if (!pvt || !pvt->tx)
		return;

	tx = pvt->tx;
	if (tx->len >= sizeof(cmd.data))
		goto done;

	memset(&cmd, 0, sizeof(cmd));

	cmd.len = tx->len + 1;
	cmd.data[0] = tx->len;
	memcpy(cmd.data + 1, tx->pkt, tx->len);

	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_ADV_DATA,
					&cmd, sizeof(cmd),
					set_send_adv_enable, pvt, NULL);
done:
	if (tx->delete) {
		l_queue_remove_if(pvt->tx_pkts, simple_match, tx);
		l_free(tx);
	}

	pvt->tx = NULL;
}

static void set_send_adv_params(const void *buf, uint8_t size,
							void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct bt_hci_cmd_le_set_adv_parameters cmd;
	uint16_t hci_interval;

	if (!pvt)
		return;

	hci_interval = (pvt->interval * 16) / 10;
	cmd.min_interval = L_CPU_TO_LE16(hci_interval);
	cmd.max_interval = L_CPU_TO_LE16(hci_interval);
	cmd.type = 0x03; /* ADV_NONCONN_IND */
	cmd.own_addr_type = 0x01; /* ADDR_TYPE_RANDOM */
	cmd.direct_addr_type = 0x00;
	memset(cmd.direct_addr, 0, 6);
	cmd.channel_map = 0x07;
	cmd.filter_policy = 0x03;

	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
				&cmd, sizeof(cmd),
				set_send_adv_data, pvt, NULL);
}

static void send_pkt(struct mesh_io_private *pvt, struct tx_pkt *tx,
							uint16_t interval)
{
	struct bt_hci_cmd_le_set_adv_enable cmd;

	/* Delete superseded packet in favor of new packet */
	if (pvt->tx && pvt->tx != tx && pvt->tx->delete) {
		l_queue_remove_if(pvt->tx_pkts, simple_match, pvt->tx);
		l_free(pvt->tx);
	}

	pvt->tx = tx;
	pvt->interval = interval;

	if (!pvt->sending) {
		set_send_adv_params(NULL, 0, pvt);
		return;
	}

	cmd.enable = 0x00;	/* Disable advertising */
	bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_ADV_ENABLE,
				&cmd, sizeof(cmd),
				set_send_adv_params, pvt, NULL);
}

static void tx_to(struct l_timeout *timeout, void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct tx_pkt *tx;
	uint16_t ms;
	uint8_t count;

	if (!pvt)
		return;

	tx = l_queue_pop_head(pvt->tx_pkts);
	if (!tx) {
		l_timeout_remove(timeout);
		pvt->tx_timeout = NULL;
		send_cancel(pvt);
		return;
	}

	if (tx->info.type == MESH_IO_TIMING_TYPE_GENERAL) {
		ms = tx->info.u.gen.interval;
		count = tx->info.u.gen.cnt;
		if (count != MESH_IO_TX_COUNT_UNLIMITED)
			tx->info.u.gen.cnt--;
	} else {
		ms = 25;
		count = 1;
	}

	tx->delete = !!(count == 1);

	send_pkt(pvt, tx, ms);

	if (count == 1) {
		/* Recalculate wakeup if we are responding to POLL */
		tx = l_queue_peek_head(pvt->tx_pkts);

		if (tx && tx->info.type == MESH_IO_TIMING_TYPE_POLL_RSP) {
			ms = instant_remaining_ms(tx->info.u.poll_rsp.instant +
						tx->info.u.poll_rsp.delay);
		}
	} else
		l_queue_push_tail(pvt->tx_pkts, tx);

	if (timeout) {
		pvt->tx_timeout = timeout;
		l_timeout_modify_ms(timeout, ms);
	} else
		pvt->tx_timeout = l_timeout_create_ms(ms, tx_to, pvt, NULL);
}

static void tx_worker(void *user_data)
{
	struct mesh_io_private *pvt = user_data;
	struct tx_pkt *tx;
	uint32_t delay;

	tx = l_queue_peek_head(pvt->tx_pkts);
	if (!tx)
		return;

	switch (tx->info.type) {
	case MESH_IO_TIMING_TYPE_GENERAL:
		if (tx->info.u.gen.min_delay == tx->info.u.gen.max_delay)
			delay = tx->info.u.gen.min_delay;
		else {
			l_getrandom(&delay, sizeof(delay));
			delay %= tx->info.u.gen.max_delay -
						tx->info.u.gen.min_delay;
			delay += tx->info.u.gen.min_delay;
		}
		break;

	case MESH_IO_TIMING_TYPE_POLL:
		if (tx->info.u.poll.min_delay == tx->info.u.poll.max_delay)
			delay = tx->info.u.poll.min_delay;
		else {
			l_getrandom(&delay, sizeof(delay));
			delay %= tx->info.u.poll.max_delay -
						tx->info.u.poll.min_delay;
			delay += tx->info.u.poll.min_delay;
		}
		break;

	case MESH_IO_TIMING_TYPE_POLL_RSP:
		/* Delay until Instant + Delay */
		delay = instant_remaining_ms(tx->info.u.poll_rsp.instant +
						tx->info.u.poll_rsp.delay);
		if (delay > 255)
			delay = 0;
		break;

	default:
		return;
	}

	if (!delay)
		tx_to(pvt->tx_timeout, pvt);
	else if (pvt->tx_timeout)
		l_timeout_modify_ms(pvt->tx_timeout, delay);
	else
		pvt->tx_timeout = l_timeout_create_ms(delay, tx_to, pvt, NULL);
}

static bool send_tx(struct mesh_io *io, struct mesh_io_send_info *info,
					const uint8_t *data, uint16_t len)
{
	struct mesh_io_private *pvt = io->pvt;
	struct tx_pkt *tx;
	bool sending = false;

	if (!info || !data || !len || len > sizeof(tx->pkt))
		return false;

	tx = l_new(struct tx_pkt, 1);

	memcpy(&tx->info, info, sizeof(tx->info));
	memcpy(&tx->pkt, data, len);
	tx->len = len;

	if (info->type == MESH_IO_TIMING_TYPE_POLL_RSP)
		l_queue_push_head(pvt->tx_pkts, tx);
	else {
		if (pvt->tx)
			sending = true;
		else
			sending = !l_queue_isempty(pvt->tx_pkts);

		l_queue_push_tail(pvt->tx_pkts, tx);

		/*
		 * If transmitter is idle, send packets at least twice to
		 * guard against in-line cancelation of HCI command chain.
		 */
		if (info->type == MESH_IO_TIMING_TYPE_GENERAL && !sending &&
							tx->info.u.gen.cnt == 1)
			tx->info.u.gen.cnt++;
	}

	if (!sending) {
		l_timeout_remove(pvt->tx_timeout);
		pvt->tx_timeout = NULL;
		l_idle_oneshot(tx_worker, pvt, NULL);
	}

	return true;
}

static bool tx_cancel(struct mesh_io *io, const uint8_t *data, uint8_t len)
{
	struct mesh_io_private *pvt = io->pvt;
	struct tx_pkt *tx;

	if (!data)
		return false;

	if (len == 1) {
		do {
			tx = l_queue_remove_if(pvt->tx_pkts, find_by_ad_type,
							L_UINT_TO_PTR(data[0]));
			l_free(tx);

			if (tx == pvt->tx)
				pvt->tx = NULL;

		} while (tx);
	} else {
		struct tx_pattern pattern = {
			.data = data,
			.len = len
		};

		do {
			tx = l_queue_remove_if(pvt->tx_pkts, find_by_pattern,
								&pattern);
			l_free(tx);

			if (tx == pvt->tx)
				pvt->tx = NULL;

		} while (tx);
	}

	if (l_queue_isempty(pvt->tx_pkts)) {
		send_cancel(pvt);
		l_timeout_remove(pvt->tx_timeout);
		pvt->tx_timeout = NULL;
	}

	return true;
}

static bool find_by_filter(const void *a, const void *b)
{
	const struct pvt_rx_reg *rx_reg = a;
	const uint8_t *filter = b;

	return !memcmp(rx_reg->filter, filter, rx_reg->len);
}

static bool recv_register(struct mesh_io *io, const uint8_t *filter,
			uint8_t len, mesh_io_recv_func_t cb, void *user_data)
{
	struct bt_hci_cmd_le_set_scan_enable cmd;
	struct mesh_io_private *pvt = io->pvt;
	struct pvt_rx_reg *rx_reg;
	bool already_scanning;
	bool active = false;

	if (!cb || !filter || !len)
		return false;

	rx_reg = l_queue_remove_if(pvt->rx_regs, find_by_filter, filter);

	l_free(rx_reg);
	rx_reg = l_malloc(sizeof(*rx_reg) + len);

	memcpy(rx_reg->filter, filter, len);
	rx_reg->len = len;
	rx_reg->cb = cb;
	rx_reg->user_data = user_data;

	already_scanning = !l_queue_isempty(pvt->rx_regs);

	l_queue_push_head(pvt->rx_regs, rx_reg);

	/* Look for any AD types requiring Active Scanning */
	if (l_queue_find(pvt->rx_regs, find_active, NULL))
		active = true;

	if (!already_scanning || pvt->active != active) {
		pvt->active = active;
		cmd.enable = 0x00;	/* Disable scanning */
		cmd.filter_dup = 0x00;	/* Report duplicates */
		bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
				&cmd, sizeof(cmd), scan_disable_rsp, pvt, NULL);

	}

	return true;
}

static bool recv_deregister(struct mesh_io *io, const uint8_t *filter,
								uint8_t len)
{
	struct bt_hci_cmd_le_set_scan_enable cmd = {0, 0};
	struct mesh_io_private *pvt = io->pvt;
	struct pvt_rx_reg *rx_reg;
	bool active = false;

	rx_reg = l_queue_remove_if(pvt->rx_regs, find_by_filter, filter);

	if (rx_reg)
		l_free(rx_reg);

	/* Look for any AD types requiring Active Scanning */
	if (l_queue_find(pvt->rx_regs, find_active, NULL))
		active = true;

	if (l_queue_isempty(pvt->rx_regs)) {
		bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
					&cmd, sizeof(cmd), NULL, NULL, NULL);

	} else if (active != pvt->active) {
		pvt->active = active;
		bt_hci_send(pvt->hci, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
				&cmd, sizeof(cmd), scan_disable_rsp, pvt, NULL);
	}

	return true;
}

const struct mesh_io_api mesh_io_generic = {
	.init = dev_init,
	.destroy = dev_destroy,
	.caps = dev_caps,
	.send = send_tx,
	.reg = recv_register,
	.dereg = recv_deregister,
	.cancel = tx_cancel,
};
