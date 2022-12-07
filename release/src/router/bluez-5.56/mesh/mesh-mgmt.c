// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  SILVAIR sp. z o.o. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "src/shared/mgmt.h"

#include "ell/queue.h"
#include "ell/log.h"
#include "ell/util.h"

#include "mesh/mesh-mgmt.h"

struct read_info_reg {
	mesh_mgmt_read_info_func_t cb;
	void *user_data;
};

struct read_info_req {
	int index;
	struct mesh_io *io;
};

static struct mgmt *mgmt_mesh;
static struct l_queue *read_info_regs;

static void process_read_info_req(void *data, void *user_data)
{
	struct read_info_reg *reg = data;
	int index = L_PTR_TO_UINT(user_data);

	reg->cb(index, reg->user_data);
}

static void read_info_cb(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	int index = L_PTR_TO_UINT(user_data);
	const struct mgmt_rp_read_info *rp = param;
	uint32_t current_settings, supported_settings;

	l_debug("hci %u status 0x%02x", index, status);

	if (status != MGMT_STATUS_SUCCESS) {
		l_error("Failed to read info for hci index %u: %s (0x%02x)",
				index, mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		l_error("Read info response too short");
		return;
	}

	current_settings = btohl(rp->current_settings);
	supported_settings = btohl(rp->supported_settings);

	l_debug("settings: supp %8.8x curr %8.8x",
					supported_settings, current_settings);

	if (current_settings & MGMT_SETTING_POWERED) {
		l_info("Controller hci %u is in use", index);
		return;
	}

	if (!(supported_settings & MGMT_SETTING_LE)) {
		l_info("Controller hci %u does not support LE", index);
		return;
	}

	l_queue_foreach(read_info_regs, process_read_info_req,
							L_UINT_TO_PTR(index));
}

static void index_added(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	mgmt_send(mgmt_mesh, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_cb, L_UINT_TO_PTR(index), NULL);
}

static void index_removed(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	l_warn("Hci dev %4.4x removed", index);
}

static void read_index_list_cb(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	uint16_t num;
	int i;

	if (status != MGMT_STATUS_SUCCESS) {
		l_error("Failed to read index list: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		l_error("Read index list response sixe too short");
		return;
	}

	num = btohs(rp->num_controllers);

	l_debug("Number of controllers: %u", num);

	if (num * sizeof(uint16_t) + sizeof(*rp) != length) {
		l_error("Incorrect packet size for index list response");
		return;
	}

	for (i = 0; i < num; i++) {
		uint16_t index;

		index = btohs(rp->index[i]);
		index_added(index, 0, NULL, user_data);
	}
}

static bool mesh_mgmt_init(void)
{
	if (!read_info_regs)
		read_info_regs = l_queue_new();

	if (!mgmt_mesh) {
		mgmt_mesh = mgmt_new_default();

		if (!mgmt_mesh) {
			l_error("Failed to initialize mesh management");
			return false;
		}

		mgmt_register(mgmt_mesh, MGMT_EV_INDEX_ADDED,
				MGMT_INDEX_NONE, index_added, NULL, NULL);
		mgmt_register(mgmt_mesh, MGMT_EV_INDEX_REMOVED,
				MGMT_INDEX_NONE, index_removed, NULL, NULL);
	}

	return true;
}

bool mesh_mgmt_list(mesh_mgmt_read_info_func_t cb, void *user_data)
{
	struct read_info_reg *reg;

	if (!mesh_mgmt_init())
		return false;

	reg = l_new(struct read_info_reg, 1);
	reg->cb = cb;
	reg->user_data = user_data;

	l_queue_push_tail(read_info_regs, reg);

	/* Use MGMT to find a candidate controller */
	l_debug("send read index_list");
	if (mgmt_send(mgmt_mesh, MGMT_OP_READ_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					read_index_list_cb, NULL, NULL) <= 0)
		return false;

	return true;
}
