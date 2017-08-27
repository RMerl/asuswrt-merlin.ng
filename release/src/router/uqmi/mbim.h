/*
 * Copyright (C) 2014 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2016  Bjørn Mork <bjorn@mork.no>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef _MBIM_H__
#define _MBIM_H__

#include <libubox/utils.h>
#include <stdbool.h>
#include <stdint.h>

#define MBIM_MESSAGE_TYPE_COMMAND	0x00000003
#define MBIM_MESSAGE_TYPE_COMMAND_DONE	0x80000003
#define MBIM_MESSAGE_COMMAND_TYPE_SET	1
#define MBIM_CID_QMI_MSG		1

struct mbim_message_header {
	uint32_t type;
	uint32_t length;
	uint32_t transaction_id;
} __packed;

struct mbim_fragment_header {
	uint32_t total;
	uint32_t current;
} __packed;

struct mbim_command_message {
	struct mbim_message_header header;
	struct mbim_fragment_header fragment_header;
	uint8_t service_id[16];
	uint32_t command_id;
	uint32_t command_type;
	uint32_t buffer_length;
} __packed;

bool is_mbim_qmi(struct mbim_command_message *msg);
void mbim_qmi_cmd(struct mbim_command_message *msg, int len, uint16_t tid);

#endif
